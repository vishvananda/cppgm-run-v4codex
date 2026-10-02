#!/usr/bin/env python3
"""Complete implementation-212 validation; record incomplete PA32 lanes."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
checks=[]
def check(name,command,cwd=root):
 log=out/(name+'.log')
 with log.open('w') as stream:r=subprocess.run(command,cwd=cwd,stdout=stream,stderr=subprocess.STDOUT)
 checks.append(dict(name=name,command=list(map(str,command)),cwd=str(cwd),exit_code=r.returncode,log=str(log),sha256=hashlib.sha256(log.read_bytes()).hexdigest()))
 (out/'checks.json').write_text(json.dumps(checks,indent=2)+'\n');print(name,r.returncode,flush=True);return r.returncode
assert check('prior',['make','test-report-through-pa31'])==0
check('stage',['make','test-pa32'])
check('through',['make','test-report-through-pa32'])
assert check('file',['perl','scripts/cppgm_file_audit.pl','--stage','pa32','--paths','dev/src'])==0
check('debug',['make','test-debuginfo','DEBUGINFO_TEST_PAS=pa32'])
for level in range(1,4):check(f'debug-driver-o{level}',['make','check',f'TEST=tests/debuginfo/driver/o{level}'],root/'pa32')
assert check('debug-objects',['scripts/run_object_lowir_roundtrip_tests.pl','--debuginfo','--app','../dev/cppgm++','--test-root','tests/object-roundtrip'],root/'pa32')==0
for name in ['loops','loop_trip_properties','objects','local','dataflow','calls','audit']:
 assert check(name,['python3',f'student.tests/pa32/{name}.py'])==0
assert check('loop-bounds',['python3','student.tests/pa32/loops_bounds.py',str(out/'bounds')])==0
assert check('trace',['python3','student.tests/pa32/audit_trace.py',str(out/'trace')])==0
assert check('loop-trace',['python3','student.tests/pa32/loops_trace.py',str(out/'loop-trace')])==0
