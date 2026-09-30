#!/usr/bin/env python3
"""Validate the full PA22 handoff inventory, gates, reducers and frozen evidence."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa22-119')
ENTRY='17603c8a69e76820274b7bf849091d528f6ff261'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(cmd):
 p=subprocess.run([str(x) for x in cmd],cwd=ROOT,capture_output=True,text=True)
 assert p.returncode==0,(cmd,p.stderr);return p.stdout
def git(*args):return run(['git',*args]).strip()
manifest=json.loads((ROOT/'student.tests/pa22/validation116.json').read_text())['contract_manifest']
assert set(git('ls-files','pa22/tests').splitlines())=={x['path'] for x in manifest}
reference=json.loads((ROOT/'student.tests/pa22/reference119.json').read_text())
assert reference['passed']
revised={r['path']:r for r in reference['fixtures']}
assert len(revised)==4
changes=[]
for item in manifest:
 path=item['path'];current=sha(ROOT/path)
 if current!=item['sha256']:
  assert path in revised and current==revised[path]['corrected_sha256']
  assert item['sha256']==revised[path]['original_sha256']
  changes.append(path)
assert set(changes)==set(revised)
assert set(git('diff','--name-only',ENTRY,'--','pa22/tests').splitlines())==set(revised)
assert not git('diff',ENTRY,'--','Makefile','scripts','shared',*[f'pa{i}/tests' for i in range(1,22)],'pa22/Makefile','pa22/scripts')
plan=(ROOT/'pa22/plan.md').read_text()
assert 'Stage base commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`' in plan
assert 'Last reviewed commit: `e90fa3fa514e2990bbe5ea4716252d8c42ae782a`' in plan
checks=json.loads((WORK/'checks-final.json').read_text());assert len(checks)==4
expected={'stage':'(99 / 99)','prior':'(3712 / 3712)','through22':'(3811 / 3811)','file-audit':'File audit passed for pa22 with 3 warning(s).'}
for check in checks:
 log=Path(check['log']);assert check['exit']==0 and expected[check['name']] in log.read_text()
 check['log_sha256']=sha(log)
assert sha(ROOT/'dev/cppgm++')==sha(WORK/'final')==reference['compiler_sha256']
assert sha(ROOT/'dev/lowir')==reference['lowir_sha256']
controls={}
for name,count in [('',32),('-inherited114',16),('-inherited115',12),('-inherited116',8),('-audit117',18),('-constants117',4),('-inherited118',36)]:
 p=ROOT/f'student.tests/pa22/controls119{name}.json';d=json.loads(p.read_text());rows=d['cases'] if isinstance(d,dict) else d
 assert len(rows)==count and all(r['passed'] for r in rows)
 if isinstance(d,dict): assert d['compiler_sha256']==sha(WORK/'final')
 controls[name or 'new']=dict(passed=count,total=count,sha256=sha(p))
roundtrips=[];(WORK/'roundtrips').mkdir(exist_ok=True)
for src in sorted((ROOT/'pa22/tests').rglob('*.t')):
 status=src.with_suffix('.my.exit_status').read_text().strip()
 assert status==src.with_suffix('.ref.exit_status').read_text().strip()
 row=dict(source=str(src.relative_to(ROOT)),rejected=status!='EXIT_SUCCESS');roundtrips.append(row)
 if row['rejected']: continue
 first=WORK/'roundtrips'/(src.stem+'.first.lowir');second=first.with_suffix('.second.lowir')
 run([ROOT/'dev/lowir','-o',first,src.with_suffix('.my')]);run([ROOT/'dev/lowir','-o',second,first])
 assert first.read_bytes()==second.read_bytes()
 row.update(stable_roundtrip=True,sha256=sha(second))
assert len(roundtrips)==99 and sum(x['rejected'] for x in roundtrips)==4
# Retain the inherited three-form target-word ABI probe as well as overlay reducers.
ir=WORK/'truth.lowir';merged=WORK/'truth-merged.lowir';exe=WORK/'truth'
commands=[[ROOT/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',ir,ROOT/'student.tests/pa22/member-truth-word.source'],
 [ROOT/'dev/lowir','-o',merged,ir,ROOT/'student.tests/pa22/member-truth-word.lowir'],
 [ROOT/'dev/lowir2native-ref','-O0','-o',exe,merged],[exe]]
for c in commands:run(c)
performance=[]
for mode,count in [('common',6),('affected',4)]:
 p=ROOT/f'student.tests/pa22/performance119-{mode}.json';d=json.loads(p.read_text())
 assert len(d['workloads'])==count
 assert [b['sha256'] for b in d['binaries']]==[sha(WORK/'entry'),sha(WORK/'final')]
 for name,w in d['workloads'].items():
  assert all(len(w[k]['observations'])==20 for k in ('compiler','runtime'))
  assert all(r['checked_exit']==0 for k in ('compiler','runtime') for r in w[k]['observations'])
  if mode=='common':assert w['identical_lowir']
 performance.append(dict(path=str(p.relative_to(ROOT)),sha256=sha(p)))
placement=ROOT/'student.tests/pa22/placement119.json';d=json.loads(placement.read_text())
assert len(d['placements'])==4
assert all(len(r['observations'])==20 and all(x['checked_exit']==0 for x in r['observations']) for r in d['placements'])
assert len(d['original_binaries'])==4
performance.append(dict(path=str(placement.relative_to(ROOT)),sha256=sha(placement),runtime_only_diagnostic=True))
old=json.loads((ROOT/'student.tests/pa22/validation118.json').read_text())['stageProgress']['final_failures']
assert len(old)==4
sources={p:sha(ROOT/p) for p in git('diff','--name-only',ENTRY,'--','dev').splitlines()}
result=dict(entry=ENTRY,implementation=git('rev-parse','HEAD'),compiler_sha256=sha(WORK/'final'),
 checks=checks,stageProgress=dict(passed=True,entry_passed=95,final_passed=99,total=99,entry_failures=old,final_failures=[],
 source_fixtures_unchanged=True,status_sidecars_unchanged=True,comparison_rules_unchanged=True,reference_overlay='119',references_changed=changes),
 controls=controls,roundtrips=roundtrips,performance=performance,
 reference_proof=dict(path='pa22/reference-corrections119.md',sha256=sha(ROOT/'pa22/reference-corrections119.md'),evidence_sha256=sha(ROOT/'student.tests/pa22/reference119.json')),
 abi_truth_probe=dict(passed=True,commands=[[str(x) for x in c] for c in commands]),
 implementation_sources=sources,source_contract_inventory_sha256=hashlib.sha256(json.dumps(manifest,sort_keys=True).encode()).hexdigest(),
 independent_review_pending=True)
(ROOT/'student.tests/pa22/validation119.json').write_text(json.dumps(result,indent=2)+'\n')
print('PA22 95->99/99; prior 3712/3712; through22 3811/3811; file audit pass; 126 controls + ABI truth; 95 roundtrips + 4 rejections; four proven oracle corrections')
