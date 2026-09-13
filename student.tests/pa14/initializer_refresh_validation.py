#!/usr/bin/env python3
"""Recheck uniquely named controls; preserve all unchanged compiler checks."""
from pathlib import Path
import hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
OLD,OLD_CONTROL,WORK,OUT=map(lambda p:Path(p).resolve(),sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def file(p):return dict(path=str(p),sha256=sha(p))
v=json.loads(OLD.read_text());control=ROOT/'student.tests/pa14/check_initialization_facts.py'
prior=next(r for r in v['sources'] if r['path']==str(control))
assert prior['sha256']==sha(OLD_CONTROL)
for row in v['binaries']+v['sources']+v['coverage']:
 if row is not prior:assert sha(row['path'])==row['sha256']
v['refresh']=dict(harness=file(Path(__file__).resolve()),prior_manifest=file(OLD),prior_control=file(OLD_CONTROL),
 reason='Two distinct cases used function-pointer as an artifact name. Their outcomes passed but one saved source was overwritten. Only the positive artifact label changes to overload-pointer; recheck all 82 cases under both unchanged binaries.',rechecked=[])
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
for label,binary in [('release',v['binaries'][1]['path']),('sanitized',v['binaries'][2]['path'])]:
 command=[sys.executable,str(control),binary,str(WORK/label)]
 p=subprocess.run(command,capture_output=True,text=True,env=env,cwd=ROOT,timeout=600)
 log=WORK/(label+'.log');log.write_text(p.stdout+p.stderr);assert p.returncode==0,(label,p.returncode,p.stderr)
 index=next(i for i,row in enumerate(v['checks']) if row['name']==label+'-initializers')
 v['checks'][index]=dict(name=label+'-initializers',command=command,exit_code=0,log=str(log),log_sha256=sha(log))
 v['refresh']['rechecked'].append(label+'-initializers')
 print(label,'82 controls PASS',flush=True)
prior['sha256']=sha(control)
OUT.write_text(json.dumps(v,indent=2)+'\n')
