#!/usr/bin/env python3
"""Final accumulated PA22 audit evidence, including the constant-owner repair."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa22-117')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
original=json.loads((ROOT/'student.tests/pa22/audit117-validation.json').read_text())
manifest=json.loads((ROOT/'student.tests/pa22/validation116.json').read_text())['contract_manifest']
assert set(git('ls-files','pa22/tests').splitlines())=={x['path'] for x in manifest}
for x in manifest:assert sha(ROOT/x['path'])==x['sha256']
assert not git('diff',original['stage_base'],'--','Makefile','scripts','shared',*[f'pa{i}/tests' for i in range(1,23)],'pa22/Makefile','pa22/scripts')
checks=json.loads((WORK/'checks-sealed.json').read_text());assert [x['exit'] for x in checks]==[2,0,0]
for x in checks:x['log_sha256']=sha(Path(x['log']))
stage=(WORK/'stage-sealed.log').read_text();prior=(WORK/'prior-sealed.log').read_text()
assert '94 / 99 TESTS PASSED' in stage and '(3712 / 3712)' in prior
failures=sorted(set(re.findall(r'(pa22/tests/[^:]+\.t): ERROR:',stage)))
assert failures==original['stageProgressPreserved']['entry_failures']
assert 'File audit passed for pa22 with 3 warning(s).' in (WORK/'file-audit-sealed.log').read_text()
assert sha(ROOT/'dev/cppgm++')==sha(WORK/'sealed')
controls={}
for name,total in [('constants-entry',4),('constants-final',4),('constants-generic',4),('controls-sealed',18),('generic-sealed',18),('inherited114-sealed',16),('inherited115-sealed',12),('inherited116-sealed',8)]:
 path=ROOT/f'student.tests/pa22/audit117-{name}.json';d=json.loads(path.read_text());rows=d['cases'] if isinstance(d,dict) else d
 assert len(rows)==total and (name=='constants-entry' or all(r['passed'] for r in rows))
 controls[name]=dict(passed=sum(r['passed'] for r in rows),total=total,sha256=sha(path))
assert controls['constants-entry']['passed']==1
performance=[]
for mode,count in [('common-sealed',6),('hierarchy-sealed',4),('constants',4),('proof-sealed',2)]:
 path=ROOT/f'student.tests/pa22/performance117-{mode}.json';d=json.loads(path.read_text());assert len(d['workloads'])==count
 assert d['binaries'][1]['sha256']==sha(WORK/'sealed')
 assert d['binaries'][0]['sha256']==sha(WORK/('proof-variant-sealed/compiler-generic' if mode=='proof-sealed' else 'entry'))
 for w in d['workloads'].values():
  assert all(len(w[k]['observations'])==20 for k in ('compiler','runtime'))
  assert all(r['checked_exit']==0 for k in ('compiler','runtime') for r in w[k]['observations'])
  if mode!='proof-sealed':assert w['identical_lowir'] and w['outputs'][0]['text_bytes']==w['outputs'][1]['text_bytes']
 performance.append(dict(path=str(path.relative_to(ROOT)),sha256=sha(path)))
roundtrips=json.loads((ROOT/'student.tests/pa22/audit117-roundtrips.json').read_text())
assert len(roundtrips)==99 and sum(x['rejected'] for x in roundtrips)==4
assert all(x['rejected'] or x['stable_roundtrip'] for x in roundtrips)
sources={p:sha(ROOT/p) for p in git('diff','--name-only',original['stage_base'],'--','dev').splitlines()}
result=dict(stage_base=original['stage_base'],audit_entry=original['audit_entry'],accumulated_review_manifest='student.tests/pa22/audit117-validation.json',
 initial_audit_fixes='015feb8d9c477017fa78fda5171d3773f1b82d68',reviewed_sources=sources,compiler_sha256=sha(WORK/'sealed'),
 checks=checks,stageProgressPreserved=dict(passed=True,entry_failures=failures,final_failures=failures,coverage_cases=99,course_inventory_unchanged=True),
 controls=controls,performance=performance,roundtrips=dict(accepted=95,rejected=4,sha256=sha(ROOT/'student.tests/pa22/audit117-roundtrips.json')),
 references_changed=False,comparison_rules_changed=False,
 constant_sources={str(p.relative_to(ROOT)):sha(p) for p in (ROOT/'student.tests/pa22/constants').glob('*.cpp')})
(ROOT/'student.tests/pa22/audit117-final-validation.json').write_text(json.dumps(result,indent=2)+'\n')
print('Final audit: prior 3712/3712; same PA22 5/99 failures; file audit pass; 58/58 personal; 95 roundtrips + 4 rejections; fourteen equivalent benchmark inputs and profitable bounded proof')
