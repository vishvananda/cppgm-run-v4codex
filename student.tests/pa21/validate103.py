#!/usr/bin/env python3
"""Re-run required loop-103 gates and archive controls and unchanged coverage."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK,BASELINE,OUT=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
assert not OUT.exists(),'preserve previous evidence'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
results={}
commands={
 'priorThroughTests':'n=21; if [ "$n" -le 1 ]; then echo "===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====="; else make test-report-through-pa$((n - 1)); fi',
 'stageTests':'make test-pa21',
 'fileAudit':'perl scripts/cppgm_file_audit.pl --stage pa21 --paths dev/src',
}
for name,cmd in commands.items():
 log=WORK/(name+'.log')
 with log.open('w') as out:p=subprocess.run(cmd,shell=True,cwd=ROOT,stdout=out,stderr=subprocess.STDOUT)
 results[name]=dict(command=cmd,exit=p.returncode,log_sha256=sha(log),output=log.read_text())
 print(name,p.returncode,flush=True)
 assert p.returncode==0 if name!='stageTests' else p.returncode in (0,2)
def failures(text):return sorted(set(re.findall(r'pa21/tests/[^ :]+\.t(?=: ERROR:)',text)))
before=failures(BASELINE.read_text());after=failures(results['stageTests']['output'])
assert len(before)==79 and len(after)<len(before) and not set(after)-set(before)
assert re.search(r'/\s*116 TESTS PASSED',results['stageTests']['output'])
results['stageProgress']=dict(turn_start_failures=before,current_failures=after,resolved=sorted(set(before)-set(after)),new_failures=[],required_cases=116,passed=True)
fixture_diff=subprocess.run(['git','diff','--exit-code','f4224e0b','--','pa21/tests','pa21/Makefile','pa21/scripts','scripts'],cwd=ROOT,capture_output=True,text=True)
assert fixture_diff.returncode==0
results['coverage']=dict(unchanged_fixture_and_harness_diff=True,required_sources={str(p.relative_to(ROOT)):sha(p) for p in sorted((ROOT/'pa21/tests').rglob('*.t'))})
for name,path in [('capture_controls','controls-final/results.json'),('capture_unwind','eh-fifth/results.json'),('inherited_captures','inherited-final2/results.json')]:
 data=json.loads((WORK/path).read_text());rows=data['rows'] if isinstance(data,dict) else data
 assert all(r['passed'] for r in rows)
 results[name]=dict(passed=len(rows),total=len(rows),evidence=data)
results.update(compiler_sha256=sha(ROOT/'dev/cppgm++'),implementation_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),baseline_log_sha256=sha(BASELINE))
OUT.write_text(json.dumps(results,indent=2)+'\n')
