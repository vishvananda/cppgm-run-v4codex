#!/usr/bin/env python3
"""Sequential implementation-217 checks with complete logs and exit statuses."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
checks=[]
def check(name,command,cwd=root,required=True):
    log=out/(name+'.log')
    with log.open('w') as stream:
        r=subprocess.run(list(map(str,command)),cwd=cwd,stdout=stream,stderr=subprocess.STDOUT)
    checks.append(dict(name=name,command=list(map(str,command)),cwd=str(cwd),exit_code=r.returncode,
                       log=str(log),sha256=hashlib.sha256(log.read_bytes()).hexdigest()))
    (out/'checks.json').write_text(json.dumps(checks,indent=2)+'\n');print(name,r.returncode,flush=True)
    if required:assert r.returncode==0,name
check('prior',['make','test-report-through-pa31'])
check('stage',['make','test-pa32'])
check('through',['make','test-report-through-pa32'])
check('file',['perl','scripts/cppgm_file_audit.pl','--stage','pa32','--paths','dev/src'])
check('debug',['make','test-debuginfo','DEBUGINFO_TEST_PAS=pa32'],required=False)
for level in range(1,4):
    check(f'debug-driver-o{level}',['make','check',f'TEST=tests/debuginfo/driver/o{level}'],root/'pa32',required=False)
for mode in ['normal','debug']:
    check(mode+'-objects',['scripts/run_object_lowir_roundtrip_tests.pl',*(['--debuginfo'] if mode=='debug' else []),
                          '--app','../dev/cppgm++','--test-root','tests/object-roundtrip'],root/'pa32')
for name in ['source_identity','pointer_congruence','floating_facts','context_guards','contextual_calls','calls',
             'control_closure','range_fills','pointer_loops','memory','memory_native','loops','loop_trip_properties',
             'objects','local','dataflow','audit']:
    check(name,['python3',f'student.tests/pa32/{name}.py'])
check('context-bounds',['python3','student.tests/pa32/context_bounds.py',out/'bounds.json'])
check('memory-bounds',['python3','student.tests/pa32/memory_bounds.py',out/'memory-bounds.json'])
check('loop-bounds',['python3','student.tests/pa32/loops_bounds.py',out/'loop-bounds'])
for name,script in [('trace','audit_trace'),('loop-trace','loops_trace'),('memory-trace','memory_trace'),('range-trace','range_trace')]:
    check(name,['python3',f'student.tests/pa32/{script}.py',out/name])
check('audit214',['python3','student.tests/pa32/audit214.py',out/'audit214'])
check('range-bounds',['python3','student.tests/pa32/range_bounds.py',out/'range-bounds.json'])
check('range-native',['python3','student.tests/pa32/range_native.py'])
