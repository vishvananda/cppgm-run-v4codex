#!/usr/bin/env python3
"""Independent host-pipeline trace, cross-object properties and repeatable ledger.
Usage: python3 student.tests/pa27/audit150.py ARTIFACT_DIRECTORY
Run after performance collection; links an explicit inspection adapter to the
current compiler's implementation objects, never to a reference implementation.
"""
import hashlib, json, pathlib, re, subprocess, sys

root = pathlib.Path(__file__).resolve().parents[2]
here = root/'student.tests/pa27'
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
records = []

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

result = dict(binary_sha256=sha(root/'dev/cppgm++'), commands=records)

def save():
    (out/'trace.json').write_text(json.dumps(result, indent=2)+'\n')

def run(args, expected=0):
    args = list(map(str,args))
    p = subprocess.run(args, cwd=root, capture_output=True, text=True, timeout=120)
    records.append(dict(command=args, status=p.returncode, stdout=p.stdout, stderr=p.stderr))
    save()
    assert p.returncode == expected, records[-1]
    return p.stdout

objects = run(['make','-s','--no-print-directory','-C','dev','--eval',
               'audit150-objects:;@echo $(call frontend_objs,cppgm++)','audit150-objects']).split()
objects = [(root/'dev'/p).resolve() for p in objects]
run(['g++','-std=c++11','-O2','-I',root/'dev/src',here/'audit150-view.cpp',*objects,'-o',out/'view'])
source = here/'audit150-a.cpp'
run([out/'view',source,out/'view-a'])
phase_counters = [json.loads(line) for line in records[-1]['stderr'].splitlines() if line.startswith('{')]
assert phase_counters[0]['template_body_transitions'] == 1
assert phase_counters[0]['semantic_member_demands'] == phase_counters[0]['semantic_demand_processed']
result['phase_counters'] = phase_counters
result['native_counters'] = json.loads(records[-1]['stdout'])
for name in ('a','b'):
    run([root/'dev/cppgm++','--stats','-c',here/f'audit150-{name}.cpp','-o',out/f'{name}.o'])
assert sha(out/'view-a.o') == sha(out/'a.o'), 'inspection adapter differs from production object'
run(['g++','-std=c++11','-c',here/'audit150-host.cpp','-o',out/'host.o'])
for order in ('ab','ba'):
    run(['g++',out/'host.o',*[out/f'{n}.o' for n in order],'-Wl,-z,text','-o',out/order])
    run([out/order])
run(['g++','-std=c++11',here/'audit150-a.cpp',here/'audit150-b.cpp',here/'audit150-host.cpp','-o',out/'host-control'])
run([out/'host-control'])

views = {}
for name, args in {
    'symbols':['readelf','-sW'], 'sections':['readelf','-SW'],
    'relocations':['readelf','-rW'], 'groups':['readelf','-gW'],
    'frames':['readelf','--debug-dump=frames'], 'disassembly':['objdump','-dr'],
}.items():
    views[name] = run([*args,out/'a.o'])
    (out/(name+'.txt')).write_text(views[name])
assert 'unused150' not in views['symbols']
assert re.search(r'FUNC\s+LOCAL.*retained150', views['symbols'])
assert re.search(r'FUNC\s+WEAK.*_Z10measure150ILi11EEli', views['symbols'])
assert 'unusedI' not in views['symbols']
assert '.rela.audit150' in views['relocations'] and 'imported + 8' in views['relocations']
assert re.search(r'\.audit150\s+PROGBITS.*\s32$',views['sections'], re.M)
for symbol in ('destroyed','imported','_ZTH6ticket'):
    assert any('GOTPCREL' in line and symbol in line for line in views['relocations'].splitlines())
assert 'R_X86_64_TPOFF32' in views['relocations']
assert 'COMDAT group' in views['groups'] and '.rela.text._Z10measure150' in views['groups']
assert 'FDE' in views['frames'] and 'DW_CFA' in views['frames']

# Same source with/without telemetry must emit precisely the same bytes.
run([root/'dev/cppgm++','-c',source,'-o',out/'no-stats.o'])
assert sha(out/'no-stats.o') == sha(out/'a.o')
result['source_hashes'] = {str(p.relative_to(root)):sha(p) for p in sorted(here.glob('audit150*')) if p.is_file()}
result['artifacts'] = {p.name:dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(out.iterdir())
                       if p.is_file() and p.name not in ('trace.json','view')}
result['properties'] = dict(host_links_in_both_orders=True, host_control=True, validated_typed_lowir=True,
    actual_host_mir_image_equals_production=True, telemetry_neutral=True,
    demanded_template_body_once=True, unrelated_member_body_absent=True,
    local_function_pruning_and_retention=True, section_alignment=32,
    relocated_import_addend=8, tls_import=True, comdat_and_unwind=True)
save()
print('PA27 audit150 trace PASS:',len(records),'commands')
