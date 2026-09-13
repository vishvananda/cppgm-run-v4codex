#!/usr/bin/env python3
"""Validate explicit virtual dependencies and compact emission owners."""
from pathlib import Path
import hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
A,B,SAN,WORK,OUT=map(lambda p:Path(p).resolve(),sys.argv[1:6]);WORK.mkdir(parents=True,exist_ok=True)
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
parent=ROOT/'student.tests/pa14/demand-failure-validation.json';old=json.loads(parent.read_text())
result=dict(harness_sha256=sha(__file__),control_sha256=sha(ROOT/'student.tests/pa14/check_demand_failures.py'),probe_sha256=sha(ROOT/'student.tests/pa14/demand-failures.cc'),parent_path=str(parent),parent_sha256=sha(parent),binaries=[dict(path=str(p),sha256=sha(p)) for p in (A,B,SAN)],checks=[],reducers=[])
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
def check(name,command):
 p=subprocess.run(list(map(str,command)),cwd=ROOT,env=env,capture_output=True,text=True,timeout=600)
 log=WORK/(name+'.log');log.write_text(p.stdout+p.stderr)
 result['checks'].append(dict(name=name,command=list(map(str,command)),exit_code=p.returncode,log=str(log),log_sha256=sha(log)))
 OUT.write_text(json.dumps(result,indent=2)+'\n');assert p.returncode==0,(name,p.returncode,p.stderr)
 print(name,'PASS',flush=True)
assert sha(ROOT/'dev/cppgm++')==sha(B)
for name,command in [('stage',['make','test-pa14']),('prior',['make','test-report-through-pa13']),('through',['make','test-report-through-pa14']),('file-audit',['perl','scripts/cppgm_file_audit.pl','--stage','pa14','--paths','dev/src'])]:check(name,command)
check('sanitizer-parity',[sys.executable,ROOT/'student.tests/pa14/check_sanitizers.py',B,SAN])
check('baseline-parity',[sys.executable,ROOT/'student.tests/pa14/check_sanitizers.py',A,B])
for label,binary in [('release',B),('sanitized',SAN)]:
 for name in ('bindings','queries','fixed_expressions','fixed_calls','fixed_objects','dependent_objects','declaration_types','demand_regions','body_values','value_queries','object_reducers','definition_demands','special_signatures','signature_publications'):
  check(f'{label}-{name}',[sys.executable,ROOT/f'student.tests/pa14/check_{name}.py',binary])
 objects=ROOT/'obj/dev' if label=='release' else WORK.parent.parent/'pa14-asan-obj/dev'
 check(label+'-virtual-demands',[sys.executable,ROOT/'student.tests/pa14/check_virtual_demands.py',WORK/('virtual-'+label),objects,'current' if label=='release' else 'sanitized'])
 check(label+'-demand-failures',[sys.executable,ROOT/'student.tests/pa14/check_demand_failures.py',WORK/('demands-'+label),objects,'current' if label=='release' else 'sanitized'])
for row in old['reducers']+[old['initializer_control']]:
 source=Path(row['source_path']);name=source.stem
 ir=WORK/(name+'.lowir');exe=WORK/(name+'.exe')
 check(name+'-compile',[B,'--emit-lowir','-O0','--validate-lowir','-o',ir,source])
 check(name+'-native',[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);check(name+'-run',[exe])
 assert sha(ir)==row['outputs'][0]['sha256'] and sha(exe)==row['outputs'][0]['native_sha256']
 result['reducers'].append(dict(source_path=str(source),source_sha256=sha(source),path=str(ir),sha256=sha(ir),native_path=str(exe),native_sha256=sha(exe)))
check('all-native',[sys.executable,ROOT/'student.tests/pa14/check_functions.py'])
assert sha(ROOT/'dev/cppgm++')==sha(B)
OUT.write_text(json.dumps(result,indent=2)+'\n')
