#!/usr/bin/env python3
"""Verify checkpoint audit gates, frozen artifacts, history and course inventory."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2]; WORK=Path('/tmp/pa22-117')
BASE='a8482d768bd2dcede42ea63ef39e39cf3245c380'
ENTRY='853c4493'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
entry=git('rev-parse',ENTRY)
prior=(WORK/'prior-final.log').read_text();stage=(WORK/'stage-final.log').read_text()
assert '(3712 / 3712)' in prior and 'ALL TESTS PASSED SUCCESSFULLY!' in prior
assert '94 / 99 TESTS PASSED' in stage
failures=sorted(set(re.findall(r'(pa22/tests/[^:]+\.t): ERROR:',stage)))
previous=json.loads((ROOT/'student.tests/pa22/validation116.json').read_text())
assert len(failures)==5 and failures==previous['remaining_failures']
manifest=previous['contract_manifest']
assert set(git('ls-files','pa22/tests').splitlines())=={r['path'] for r in manifest}
for item in manifest:assert sha(ROOT/item['path'])==item['sha256']
# No course fixture, reference, status, comparison or build rule is changed.
assert not git('diff',BASE,'--','Makefile','scripts','shared',*[f'pa{i}/tests' for i in range(1,23)],'pa22/Makefile','pa22/scripts')
audit=(WORK/'file-audit.log').read_text();assert 'File audit passed for pa22 with 3 warning(s).' in audit
controls={}
for name,total in [('entry',18),('controls',18),('generic',18),('inherited114',16),('inherited115',12),('inherited116',8)]:
 p=ROOT/f'student.tests/pa22/audit117-{name}.json'; data=json.loads(p.read_text())
 rows=data['cases'] if isinstance(data,dict) else data
 assert len(rows)==total and (name=='entry' or all(r['passed'] for r in rows))
 controls[name]=dict(passed=sum(r['passed'] for r in rows),total=total,sha256=sha(p))
assert controls['entry']['passed']==8
roundtrips=json.loads((ROOT/'student.tests/pa22/audit117-roundtrips.json').read_text())
assert len(roundtrips)==99 and sum(r['rejected'] for r in roundtrips)==4
assert all(r['rejected'] or r['stable_roundtrip'] for r in roundtrips)
assert sha(ROOT/'dev/cppgm++')==sha(WORK/'final')
performance=[]
for mode,count in [('common',6),('hierarchy',4),('proof',2)]:
 p=ROOT/f'student.tests/pa22/performance117-{mode}.json'; d=json.loads(p.read_text())
 assert len(d['workloads'])==count
 assert d['binaries'][1]['sha256']==sha(WORK/'final')
 assert d['binaries'][0]['sha256']==sha(WORK/('proof-variant/compiler-generic' if mode=='proof' else 'entry'))
 for w in d['workloads'].values():
  assert all(len(w[k]['observations'])==20 for k in ('compiler','runtime'))
  assert all(r['checked_exit']==0 for k in ('compiler','runtime') for r in w[k]['observations'])
  if mode!='proof':assert w['identical_lowir'] and w['outputs'][0]['text_bytes']==w['outputs'][1]['text_bytes']
 performance.append(dict(path=str(p.relative_to(ROOT)),sha256=sha(p)))
review=[]
for commit in git('rev-list','--reverse',f'{BASE}..{entry}').splitlines():
 review.append(dict(commit=commit,subject=git('show','-s','--format=%s',commit),paths=git('diff-tree','--no-commit-id','--name-only','-r',commit).splitlines()))
assert len(review)==15
sources={p:sha(ROOT/p) for p in git('diff','--name-only',BASE,'--','dev').splitlines()}
result=dict(stage_base=BASE,audit_entry=entry,previous_turn='Progress: handoff 116 committed conversion/initialization fixes and evidence; clean entry and no live check process verified.',
 review_commits=review,reviewed_sources=sources,compiler_sha256=sha(WORK/'final'),entry_compiler_sha256=sha(WORK/'entry'),
 priorThroughTests=dict(command='make test-report-through-pa21',exit=0,passed=3712,total=3712,log_sha256=sha(WORK/'prior-final.log')),
 stageTests=dict(command='make test-pa22',exit=2,passed=94,total=99,log_sha256=sha(WORK/'stage-final.log')),
 stageProgressPreserved=dict(passed=True,entry_failures=failures,final_failures=failures,new_failures=[],coverage_cases=99,contract_manifest_source='student.tests/pa22/validation116.json',manifest_sha256=sha(ROOT/'student.tests/pa22/validation116.json')),
 fileAudit=dict(command='perl scripts/cppgm_file_audit.pl --stage pa22 --paths dev/src',exit=0,output=audit),controls=controls,performance=performance,
 roundtrips=dict(accepted=95,rejected=4,sha256=sha(ROOT/'student.tests/pa22/audit117-roundtrips.json')),
 references_changed=False,comparison_rules_changed=False,architecture='PA22 source-to-typed-LowIR traced through supplied test backend to ELF; student native emission is PA24.')
(ROOT/'student.tests/pa22/audit117-validation.json').write_text(json.dumps(result,indent=2)+'\n')
print('PA1–21 3712/3712; PA22 same 5/99 failures; file audit pass; audit controls 8/18 -> 18/18; inherited 36/36; course inventory unchanged')
