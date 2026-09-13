#!/usr/bin/env python3
"""Declaration identities and sparse facts: complete parity and lifetime validation."""
from pathlib import Path
import json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
binaries=[A.resolve(),B.resolve()]
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
result=dict(harness_sha256=shared.sha(__file__),binaries=[dict(path=str(p),sha256=shared.sha(p)) for p in binaries],checks=[],reducers=[],
 stage_sources=314,prior_tests=1621,through_tests=1935,personal_native=31,parity_sources=345,rejection_controls=157,abi_controls=6,reducer_controls=7,
 coverage=[dict(path=str(p.relative_to(ROOT)),sha256=shared.sha(p)) for p in sorted((ROOT/'pa14/tests').rglob('*')) if p.is_file() and not '.my' in p.name],
 personal=[dict(path=str(p.relative_to(ROOT)),sha256=shared.sha(p)) for p in sorted((ROOT/'student.tests/pa14').glob('*.cpp'))])
def check(name,command):
 log=WORK/(name+'.log');r=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=600,env=env);log.write_text(r.stdout+r.stderr)
 result['checks'].append(dict(name=name,command=list(map(str,command)),exit_code=r.returncode,log=str(log),log_sha256=shared.sha(log)))
 OUT.write_text(json.dumps(result,indent=2)+'\n');assert r.returncode==0,(name,r.returncode,r.stderr)
 print(name,'PASS',flush=True)
check('sanitizer-parity',[sys.executable,ROOT/'student.tests/pa14/check_sanitizers.py',*binaries])
for b,binary in enumerate(binaries):
 for name in ('bindings','queries','fixed_expressions','fixed_calls','fixed_objects','dependent_objects','declaration_types','demand_regions','body_values','value_queries','object_reducers','definition_demands','special_signatures'):
  check(f'{b}-{name}',[sys.executable,ROOT/f'student.tests/pa14/check_{name}.py',binary])
for name in ('field-category','default-identity','parameter-shape','method-parameters','region-attributes','default-heads','value-conversion'):
 source=ROOT/f'student.tests/pa14/{name}.t';row=dict(source_path=str(source),source_sha256=shared.sha(source),outputs=[])
 for b,binary in enumerate(binaries):
  ir=WORK/f'{name}-{b}.lowir';exe=WORK/f'{name}-{b}'
  check(f'{name}-{b}-compile',[binary,'--emit-lowir','-O0','--validate-lowir','-o',ir,source])
  check(f'{name}-{b}-native',[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
  check(f'{name}-{b}-run',[exe])
  row['outputs'].append(dict(binary=b,path=str(ir),sha256=shared.sha(ir),native_path=str(exe),native_sha256=shared.sha(exe),native_exit=0))
 assert len(set(o['sha256'] for o in row['outputs']))==len(set(o['native_sha256'] for o in row['outputs']))==1
 result['reducers'].append(row)
OUT.write_text(json.dumps(result,indent=2)+'\n')

source=ROOT/'student.tests/pa14/expression-store.cc'
result['store_control']=dict(source_path=str(source),source_sha256=shared.sha(source),outputs=[])
for name,flags in (('release',['-O2']),('sanitized',['-O1','-g','-fno-omit-frame-pointer','-fsanitize=address,undefined','-fno-pie','-no-pie'])):
 exe=WORK/('store-'+name)
 command=['g++','-std=c++11',*flags,'-I'+str(ROOT/'dev/src'),source,ROOT/'dev/src/semantic/expression_store.cpp',ROOT/'dev/src/support/id_index.cpp','-o',exe]
 check('store-'+name+'-build',command);check('store-'+name+'-run',[exe])
 result['store_control']['outputs'].append(dict(path=str(exe),sha256=shared.sha(exe),exit_code=0))
source=ROOT/'student.tests/pa14/expression-owners.cpp'
result['owner_control']=dict(source_path=str(source),source_sha256=shared.sha(source),outputs=[])
for b,binary in enumerate((Path(sys.argv[5]).resolve(),*binaries)):
 ir=WORK/f'owner-{b}.lowir';exe=WORK/f'owner-{b}'
 check(f'owner-{b}-compile',[binary,'--emit-lowir','-O0','--validate-lowir','-o',ir,source])
 check(f'owner-{b}-native',[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);check(f'owner-{b}-run',[exe])
 result['owner_control']['outputs'].append(dict(binary_path=str(binary),binary_sha256=shared.sha(binary),path=str(ir),sha256=shared.sha(ir),native_path=str(exe),native_sha256=shared.sha(exe),native_exit=0))
assert len(set(o['sha256'] for o in result['owner_control']['outputs']))==len(set(o['native_sha256'] for o in result['owner_control']['outputs']))==1
OUT.write_text(json.dumps(result,indent=2)+'\n')

source=ROOT/'student.tests/pa14/fact-store.cc'
result['fact_store_control']=dict(source_path=str(source),source_sha256=shared.sha(source),
 implementation_path=str(ROOT/'dev/src/semantic/fact_store.cpp'),implementation_sha256=shared.sha(ROOT/'dev/src/semantic/fact_store.cpp'),outputs=[])
for name,flags in (('release',['-O2']),('sanitized',['-O1','-g','-fno-omit-frame-pointer','-fsanitize=address,undefined','-fno-pie','-no-pie'])):
 exe=WORK/('fact-store-'+name)
 command=['g++','-std=c++11',*flags,'-I'+str(ROOT/'dev/src'),source,ROOT/'dev/src/semantic/fact_store.cpp','-o',exe]
 check('fact-store-'+name+'-build',command);check('fact-store-'+name+'-run',[exe])
 result['fact_store_control']['outputs'].append(dict(path=str(exe),sha256=shared.sha(exe),exit_code=0))
source=ROOT/'student.tests/pa14/direct-initializer.t'
result['initializer_control']=dict(source_path=str(source),source_sha256=shared.sha(source),outputs=[])
for b,binary in enumerate(binaries):
 ir=WORK/f'initializer-{b}.lowir';exe=WORK/f'initializer-{b}'
 check(f'initializer-{b}-compile',[binary,'--emit-lowir','-O0','--validate-lowir','-o',ir,source])
 check(f'initializer-{b}-native',[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);check(f'initializer-{b}-run',[exe])
 result['initializer_control']['outputs'].append(dict(path=str(ir),sha256=shared.sha(ir),native_path=str(exe),native_sha256=shared.sha(exe),native_exit=0))
assert len(set(o['sha256'] for o in result['initializer_control']['outputs']))==len(set(o['native_sha256'] for o in result['initializer_control']['outputs']))==1
OUT.write_text(json.dumps(result,indent=2)+'\n')
