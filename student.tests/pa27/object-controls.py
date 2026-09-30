#!/usr/bin/env python3
"""Explicit PA27 object, relocation, COMDAT/unwind and section controls."""
import hashlib, json, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
here = pathlib.Path(__file__).resolve().parent
out = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '/tmp/pa27-145/controls').resolve()
out.mkdir(parents=True, exist_ok=True)
compiler = root/'dev/cppgm++'
records = []
def run(args, success=True):
    p = subprocess.run(list(map(str, args)), cwd=root, capture_output=True, timeout=60)
    records.append(dict(command=list(map(str,args)), status=p.returncode,
                        stdout=p.stdout.decode(errors='replace'), stderr=p.stderr.decode(errors='replace')))
    (out/'commands.json').write_text(json.dumps(records, indent=2)+'\n')
    assert (p.returncode == 0) == success, records[-1]
    return p.stdout.decode()
def compile(source, name, host=False):
    obj = out/(name+'.o')
    run(['g++' if host else compiler, '-std=c++11', '-O0', '-c', here/source, '-o', obj])
    return obj
def link(objects, name, own=False, extra=()):
    exe = out/name
    run([compiler if own else 'g++', *objects, *extra, '-o', exe])
    run([exe])
    return exe
# Reducer corroboration in both cross-compiler directions, preserving exact
# unmangled variable identity and the host PIE/GOT boundary.
for definition_host in (False, True):
    tag = str(int(definition_host))
    definition = compile('global-name.cpp', 'global-definition'+tag, definition_host)
    consumer = compile('global-name-user.cpp', 'global-user'+tag, not definition_host)
    names = run(['nm', definition]); imports = run(['nm', '-u', consumer])
    assert ' D g\n' in names and '_Z1g' not in names
    assert ' U g\n' in imports and '_Z1g' not in imports
    if not definition_host:
        rel = run(['readelf', '-rW', definition])
        assert any('R_X86_64_PC32' in line and ' g ' in line for line in rel.splitlines())
    else:
        rel = run(['readelf', '-rW', consumer])
        assert any('GOTPCREL' in line and ' g ' in line for line in rel.splitlines())
    link([definition, consumer], 'global'+tag, extra=['-Wl,-z,text'])
# Both chosen COMDAT copies contain throwing code and live call relocations.
a = compile('inline-eh-a.cpp', 'eh-a')
b = compile('inline-eh-b.cpp', 'eh-b')
for obj in (a,b):
    groups = run(['readelf', '--section-groups', obj])
    assert '_Z13checked_valuei' in groups and '.rela.text.' in groups
    frames = run(['readelf', '-wf', obj]); assert 'zPLR' in frames
link([a,b], 'eh-ab'); link([b,a], 'eh-ba')
link([a,b], 'eh-own', own=True)
link([here/'inline-eh-a.cpp',here/'inline-eh-b.cpp'], 'eh-direct', own=True)
# DSO data requires actual GOT loads; static or non-PIE copies cannot mask it.
dso = out/'libinput.so'
run(['g++','-std=c++11','-shared','-fPIC',here/'imported-data-host.cpp','-o',dso])
obj = compile('imported-data.cpp', 'import')
rel = run(['readelf','-rW',obj])
for name in ('incoming','imported_fp','imported_count'):
    assert any('GOTPCREL' in line and ' '+name+' ' in line for line in rel.splitlines())
    assert not any('R_X86_64_PC32' in line and ' '+name+' ' in line for line in rel.splitlines())
link([obj,dso], 'import', extra=['-Wl,-z,text','-Wl,-rpath,'+str(out)])
# Multiple definitions in one named section retain alignment and pointer fixes.
obj = compile('sections.cpp', 'sections')
assert 'records' in run(['readelf','-SW',obj])
assert '.relarecords' in run(['readelf','-rW',obj])
link([obj], 'sections'); link([obj], 'sections-own', own=True)
weak = compile('weak-data.cpp', 'weak')
strong = compile('weak-data-host.cpp', 'strong', host=True)
rel = run(['readelf','-rW',weak])
assert any('GOTPCREL' in line and ' replaceable ' in line for line in rel.splitlines())
assert not any('R_X86_64_PC32' in line and ' replaceable ' in line for line in rel.splitlines())
link([weak,strong], 'weak-override')
for index,source in enumerate([
    '__attribute__((section())) int x;',
    '__attribute__((section(1))) int x;',
    '__attribute__((section(""))) int x;',
    '__attribute__((section("a","b"))) int x;',
    '__attribute__((section("a-b"))) int x;',
    '__attribute__((section("abc"))) void f() {}',
    '__attribute__((section("one"))) extern int x; __attribute__((section("two"))) int x;',
    'int main() { __attribute__((section("local"))) int x = 0; return x; }',
]):
    path = out/('invalid'+str(index)+'.cpp'); path.write_text(source)
    run([compiler,'-c',path,'-o',out/'invalid.o'], success=False)
manifest = dict(status='pass', commands=len(records), compiler_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),
                source_sha256={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in here.glob('*.cpp')})
(out/'result.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('PA27 personal object controls: PASS ('+str(len(records))+' commands)')
