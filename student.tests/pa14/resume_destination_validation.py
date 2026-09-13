#!/usr/bin/env python3
"""Resume identical validation after an inspection linker exhausted scratch space.
Completed commands/logs remain frozen; rerun the failed check and all unrun checks.
"""
from pathlib import Path
import hashlib,json,os,sys
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'student.tests/pa14/destination-validation.json'
WORK=Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa14-final-audit/destination-validation-final'
RECOVERY=WORK.parent/'destination-validation-resume';RECOVERY.mkdir(exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def binary(p):return dict(path=str(p),sha256=sha(p))
prior=json.loads(OUT.read_text());harness=ROOT/'student.tests/pa14/destination_audit_validation.py'
assert sha(harness)==prior['harness_sha256']
assert prior['checks'][-1]['name']=='sanitized-body-publication' and prior['checks'][-1]['exit_code']==1
assert 'No space left on device' in Path(prior['checks'][-1]['log']).read_text()
archive=RECOVERY/'prior-manifest.json';assert not archive.exists();archive.write_bytes(OUT.read_bytes())
completed={r['name']:r for r in prior['checks'] if r['exit_code']==0}
for row in prior['checks']:assert sha(row['log'])==row['log_sha256']
for row in prior['binaries']:assert sha(row['path'])==row['sha256']
recovery=dict(harness=binary(__file__),prior_manifest=binary(archive),failed_check=prior['checks'][-1],reason='inspection probe link exhausted shared storage; completed untimed probes archived losslessly',reused=[],rerun=[])
code=harness.read_text();a=code.index('def check(name,command):');b=code.index("assert sha(ROOT/'dev/cppgm++')",a)
replacement="""def check(name,command):
 command=list(map(str,command))
 if name in completed:
  row=completed[name]
  assert row['command']==command and sha(row['log'])==row['log_sha256']
  result['checks'].append(row);recovery['reused'].append(name)
  print(name,'reused PASS',flush=True)
 else:
  p=subprocess.run(command,cwd=ROOT,env=env,capture_output=True,text=True,timeout=600)
  log=RECOVERY/(name+'.log');log.write_text(p.stdout+p.stderr)
  result['checks'].append(dict(name=name,command=command,exit_code=p.returncode,log=str(log),log_sha256=sha(log)))
  recovery['rerun'].append(name)
  result['recovery']=recovery;OUT.write_text(json.dumps(result,indent=2)+'\\n')
  assert p.returncode==0,(name,p.returncode,p.stderr)
  print(name,'PASS',flush=True)
 result['recovery']=recovery
 OUT.write_text(json.dumps(result,indent=2)+'\\n')
"""
code=code[:a]+replacement+code[b:]
sys.argv=[str(harness),*[r['path'] for r in prior['binaries']],str(WORK),str(OUT)]
exec(compile(code,str(harness),'exec'),dict(__name__='__main__',__file__=str(harness),completed=completed,recovery=recovery,RECOVERY=RECOVERY))
