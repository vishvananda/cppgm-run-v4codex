#!/usr/bin/env python3
"""Record PA22 implementation handoff gates and unchanged course coverage."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa22-115')
BASE='a8482d768bd2dcede42ea63ef39e39cf3245c380';ENTRY='5a21daff2c280c8b3a55000e253c50773fe6a6da'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def fail(log):return set(re.findall(r'(pa22/tests/[^:]+\.t): ERROR:',log))
entry=(WORK/'entry.log').read_text();stage=(WORK/'stage-final.log').read_text();prior=(WORK/'prior-final.log').read_text()
old,new=fail(entry),fail(stage)
assert len(old)==52 and len(new)==8 and new<=old
assert '91 / 99 TESTS PASSED' in stage and '(3712 / 3712)' in prior
baseline=json.loads((ROOT/'student.tests/pa22/validation114.json').read_text())
manifest=baseline['contract_manifest']
paths=subprocess.check_output(['git','ls-files','pa22/tests'],cwd=ROOT,text=True).splitlines()
assert set(paths)=={p['path'] for p in manifest}
for item in manifest:assert sha(ROOT/item['path'])==item['sha256'],item['path']
assert sum(p.endswith('.t') for p in paths)==99
assert not subprocess.check_output(['git','diff','--name-only',BASE,'--','Makefile','scripts','shared','pa22/tests','pa22/Makefile','pa22/scripts'],cwd=ROOT,text=True).strip()
audit=subprocess.run(['perl','scripts/cppgm_file_audit.pl','--stage','pa22','--paths','dev/src'],cwd=ROOT,capture_output=True,text=True);assert audit.returncode==0
personal=json.loads((ROOT/'student.tests/pa22/controls115.json').read_text())
initial=json.loads((ROOT/'student.tests/pa22/controls115-entry.json').read_text())
inherited=json.loads((ROOT/'student.tests/pa22/controls115-inherited.json').read_text())
assert len(personal)==len(initial)==12 and all(x['passed'] for x in personal)
assert len(inherited)==16 and all(x['passed'] for x in inherited)
roundtrips=json.loads((ROOT/'student.tests/pa22/roundtrips115.json').read_text())
assert {x['source'] for x in roundtrips}==old-new and all(x['stable_roundtrip'] for x in roundtrips)
assert sum('execution_note' not in x for x in roundtrips)==38
assert sha(ROOT/'dev/cppgm++')==sha(WORK/'compiler-final')
assert not subprocess.check_output(['git','diff','--name-only','2fd83db0','--','dev'],cwd=ROOT,text=True).strip()
perf=[]
for name in ['performance115-common.json','performance115-target.json','performance115-common-final.json','performance115-target-final.json']:
 path=ROOT/'student.tests/pa22'/name;data=json.loads(path.read_text())
 assert all(o['checked_exit']==0 for w in data['workloads'].values() for o in w['outputs'])
 assert all(len(w[k]['observations'])==20 for w in data['workloads'].values() for k in ['compiler','runtime'])
 if '-final' in name:assert data['binaries'][1]['sha256']==sha(WORK/'compiler-final')
 perf.append(dict(path=str(path.relative_to(ROOT)),sha256=sha(path)))
plan=(ROOT/'pa22/plan.md').read_text()
assert f'Stage base commit: `{BASE}`' in plan and f'Last reviewed commit: `{BASE}`' in plan
result=dict(stage_base=BASE,entry_commit=ENTRY,implementation_commit='2fd83db0',compiler_sha256=sha(WORK/'compiler-final'),
 previous_turn='Progress: 44 original failures repaired, implementation commits preserved; interrupted validation handles missing and no live processes before restart',
 priorThroughTests=dict(command='make test-report-through-pa21',exit=0,passed=3712,total=3712,log_sha256=sha(WORK/'prior-final.log')),
 stageTests=dict(command='make test-pa22',exit=2,passed=91,total=99,log_sha256=sha(WORK/'stage-final.log')),
 stageProgress=dict(entry_passed=47,entry_failed=52,final_passed=91,final_failed=8,unchanged_cases=99,improved=sorted(old-new),regressed=sorted(new-old),criterion_met=True),
 fileAudit=dict(command='perl scripts/cppgm_file_audit.pl --stage pa22 --paths dev/src',exit=audit.returncode,output=audit.stdout+audit.stderr),
 personal=dict(entry_passed=sum(x['passed'] for x in initial),final_passed=12,total=12,inherited_passed=16,inherited_total=16),
 supplemental=dict(validated_roundtrips=44,matching_executions=38,declaration_only=5,unresolved_external_both_lanes=1),
 performance=perf,remaining_failures=sorted(new),contract_manifest=manifest,
 review='Independent review pending; stage/review markers preserved; eight remaining failures are implementation obligations')
(ROOT/'student.tests/pa22/validation115.json').write_text(json.dumps(result,indent=2)+'\n')
print('PA22 91/99; 44 repaired, zero regressions; PA1–PA21 3712/3712; file audit pass; unchanged 99-case contract')
