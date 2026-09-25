#!/usr/bin/env python3
"""Bind final code, all checks, coverage, reducers and frozen performance: WORK."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
entry='01b47c20b41f68d3142df6a8691fc905c20341a8'
progress=json.loads((W/'validation-complete/stage-progress.json').read_text())
assert progress['entry_failures']==3 and progress['final_failures']==0
assert progress['compiler_sha256']==sha(ROOT/'dev/cppgm++')
checks=progress['checks'];assert all(c['exit']==0 and sha(c['path'])==c['sha256'] for c in checks.values())
plan=(ROOT/'pa18/plan.md').read_text()
assert 'Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`' in plan
assert 'Last reviewed commit: `f6eaf8ab213c90eefffd48f5c47b1ac9a75bce8d`' in plan
assert not git('diff','HEAD','--','dev')
perf=[]
for filename in ('loop87-performance.json','loop87-performance-final.json','loop87-performance-complete.json'):
 p=ROOT/'student.tests/pa18'/filename;data=json.loads(p.read_text());assert data.get('finished_utc')
 count=len(data['startup']['warmups'])+len(data['startup']['observations'])
 for work in data['workloads'].values():
  assert hashlib.sha256(work['source'].encode()).hexdigest()==work['source_sha256']
  for key in ('compiler','runtime'):
   if key not in work:continue
   m=work[key];count+=len(m['warmups'])+len(m['observations'])
   assert all(r['checked_exit']==0 for r in m['observations'])
   assert len(m['observations'])==(20 if len(m['warmups'])==2 else 6)
 perf.append(dict(path=str(p.relative_to(ROOT)),sha256=sha(p),observations=count,workloads=len(data['workloads']),binaries=data['binaries'],commits=data['commits']))
assert perf[-1]['binaries'][1]['sha256']==sha(ROOT/'dev/cppgm++')
personal={}
for p in sorted((W/'validation-complete').rglob('results.json')):
 rows=json.loads(p.read_text())
 if isinstance(rows,list) and rows and all(isinstance(r,dict) and 'passed' in r for r in rows):
  assert all(r['passed'] for r in rows),p
  personal[str(p.parent.relative_to(W/'validation-complete'))]=dict(count=len(rows),sha256=sha(p),path=str(p))
baseline=json.loads((W/'controls-entry-complete/results.json').read_text())
final=json.loads((W/'validation-complete/boundary87_controls/results.json').read_text())
assert {r['name']:r['source_sha256'] for r in baseline}=={r['name']:r['source_sha256'] for r in final}
assert all(r['passed'] for r in final)
proof=ROOT/'student.tests/pa18/loop87-reference-complete.json'
assert all(r['passed'] for r in json.loads(proof.read_text())['observations'])
changed=git('diff','--name-only',entry,'--','dev').splitlines()
record=dict(entry=entry,validated_code=git('log','-1','--format=%H','--','dev'),head_at_evidence=git('rev-parse','HEAD'),
 stage_base='94dcb8ad21664137e87d574e878c14a4a047348a',last_reviewed='f6eaf8ab213c90eefffd48f5c47b1ac9a75bce8d',
 stage_progress=progress,personal=personal,new_controls=dict(entry_passed=sum(r['passed'] for r in baseline),final_passed=len(final),count=len(final),entry_sha256=sha(W/'controls-entry-complete/results.json'),final_sha256=sha(W/'validation-complete/boundary87_controls/results.json')),
 changed_code_sha256={p:sha(ROOT/p) for p in changed},performance=perf,reference_observations=dict(path=str(proof.relative_to(ROOT)),sha256=sha(proof)),
 handoff='Full-stage implementation: all required checks pass; independent whole-stage audit including reference-boundary proof remains required before advancement.')
out=ROOT/'student.tests/pa18/loop87-evidence.json';out.write_text(json.dumps(record,indent=2)+'\n')
print('Bound final evidence:',len(checks),'checks;',len(final),'new controls;',sum(r['passed'] for r in baseline),'entry passes.')
