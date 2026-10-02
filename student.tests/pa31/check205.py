#!/usr/bin/env python3
"""Explicit PA31 controls: source semantics, hosted ownership, and LowIR views."""
import hashlib, json, pathlib, subprocess, sys, time
root = pathlib.Path(__file__).resolve().parents[2]
sources = root / 'student.tests/pa31/source205'
out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
cc = pathlib.Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else root / 'dev/cppgm++'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
r = dict(compiler_sha256=sha(cc), sources={p.name: sha(p) for p in sources.iterdir() if p.is_file()}, commands=[])
def save(): (out / 'controls.json').write_text(json.dumps(r, indent=2) + '\n')
def run(args, reject=False):
    args = list(map(str, args)); start = time.perf_counter()
    p = subprocess.run(args, cwd=root, capture_output=True, text=True, timeout=45)
    r['commands'].append(dict(args=args, status=p.returncode, wall_s=time.perf_counter()-start,
                              stdout=p.stdout, stderr=p.stderr, expected_rejection=reject))
    save(); assert (p.returncode != 0) == reject, r['commands'][-1]
    return p.stdout
objs = {}
for source in sorted(sources.glob('*.cpp')):
    reject = '.reject.' in source.name
    obj = out / (source.stem + '.o')
    # The replacement allocator is a separate host-owned library implementation.
    compiler = 'g++' if source.stem == 'allocation-provider' else cc
    run([compiler, '-std=c++11', '-O0', '-c', source, '-o', obj], reject)
    if reject: continue
    objs[source.stem] = obj
    if compiler == cc:
        run([cc, '-c', '--emit-lowir', '--validate-lowir', source, '-o', out / (source.stem + '.lowir')])
for name, stems in [('global-defined', ['global-defined']),
                    ('global-import', ['global-import','global-provider']),
                    ('allocation', ['allocation-client','allocation-second','allocation-provider']),
                    ('vbase', ['vbase-complete','vbase-derived']),
                    ('list-exceptions', ['list-exceptions']),
                    ('inherited-default', ['inherited-default']),
                    ('inherited-traits', ['inherited-traits'])]:
    exe = out / name
    run(['g++', *(objs[s] for s in stems), '-o', exe]); run([exe])
for stem, kind in [('global-defined','R_X86_64_PC32'), ('global-import','R_X86_64_GOTPCREL')]:
    reloc = run(['readelf','-rW',objs[stem]])
    assert any(kind in line and ' g ' in line for line in reloc.splitlines())
    assert '_Z1g' not in reloc
for stem in ['allocation-client','allocation-second']:
    nm = run(['nm','--defined-only',objs[stem]])
    assert '_Znam' not in nm and '_ZdaPv' not in nm
    ir = (out / (stem + '.lowir')).read_text()
    assert 'cppgm_builtin_operator' not in ir
nm = run(['nm','--defined-only',objs['vbase-complete']])
assert '_ZN1BC1Ev' in nm and '_ZN1BC2Ev' not in nm
assert '_ZN1BD1Ev' in nm
# Vtable/thunk demand can require D2 even in this TU. It must be a distinct
# definition, not an alias of the complete object's destructor.
symbols = run(['readelf','-Ws',objs['vbase-complete']])
entries = {line.split()[-1]: line.split()[6] for line in symbols.splitlines()
           if line.split() and line.split()[-1] in ['_ZN1BD1Ev','_ZN1BD2Ev']}
assert entries['_ZN1BD1Ev'] != entries.get('_ZN1BD2Ev')
nm = run(['nm','--defined-only',objs['vbase-derived']])
assert '_ZN1BC2Ev' in nm
assert sha(cc) == r['compiler_sha256']
r['passed'] = True; save()
print(len(r['commands']), 'explicit controls passed')
