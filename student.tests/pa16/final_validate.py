#!/usr/bin/env python3
"""Run the explicit PA16 audit controls and required root exits, retaining logs."""
from pathlib import Path
import concurrent.futures, hashlib, json, subprocess, sys, time
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
compiler=ROOT/'dev/cppgm++';before=sha(compiler)
suites=['scalar_execution','floating','storage','validity','noexcept','audit_controls','objects','initialization','member_constants','result_lifetimes','final_audit']
def check(name,command):
 log=WORK/(name+'.log');start=time.time()
 with log.open('w') as f:r=subprocess.run(command,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=900)
 row=dict(name=name,command=[str(x) for x in command],exit_code=r.returncode,elapsed_s=time.time()-start,log=str(log),sha256=sha(log))
 print(name,r.returncode,flush=True);return row
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
 controls=list(pool.map(lambda name:check(name,['python3',ROOT/'student.tests/pa16'/f'{name}.py',compiler,WORK/name]),suites))
checks=[]
for name,command in [('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa16','--paths','dev/src']),('stageTests',['make','test-pa16']),('throughTests',['make','test-report-through-pa16'])]:
 checks.append(check(name,command))
record=dict(compiler_sha256=before,controls=controls,checks=checks)
(WORK/'results.json').write_text(json.dumps(record,indent=2)+'\n')
assert sha(compiler)==before
assert all(r['exit_code']==0 for r in controls+checks)
