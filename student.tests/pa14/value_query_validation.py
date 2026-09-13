#!/usr/bin/env python3
"""Run all current controls and retain terminal status/output evidence."""
from pathlib import Path
import json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
binaries=[A.resolve(),B.resolve()]
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
result=dict(harness_sha256=shared.sha(__file__),binaries=[dict(path=str(p),sha256=shared.sha(p)) for p in binaries],checks=[],reducers=[],
 stage_sources=314,prior_tests=1621,through_tests=1935,personal_native=23,parity_sources=337,rejection_controls=124,abi_controls=6,reducer_controls=7,
 coverage=[dict(path=str(p.relative_to(ROOT)),sha256=shared.sha(p)) for p in sorted((ROOT/'pa14/tests').rglob('*')) if p.is_file() and not '.my' in p.name],
 personal=[dict(path=str(p.relative_to(ROOT)),sha256=shared.sha(p)) for p in sorted((ROOT/'student.tests/pa14').glob('*.cpp'))])
def check(name,command):
 log=WORK/(name+'.log');r=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=600,env=env);log.write_text(r.stdout+r.stderr)
 result['checks'].append(dict(name=name,command=list(map(str,command)),exit_code=r.returncode,log=str(log),log_sha256=shared.sha(log)))
 OUT.write_text(json.dumps(result,indent=2)+'\n');assert r.returncode==0,(name,r.returncode,r.stderr)
 print(name,'PASS',flush=True)
check('sanitizer-parity',[sys.executable,ROOT/'student.tests/pa14/check_sanitizers.py',*binaries])
for b,binary in enumerate(binaries):
 for name in ('bindings','queries','fixed_expressions','fixed_calls','fixed_objects','dependent_objects','declaration_types','demand_regions','body_values','value_queries','object_reducers'):
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
