#!/usr/bin/env python3
"""Record PA22 implementation handoff gates and unchanged course coverage."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2];WORK=Path('/tmp/pa22-116')
BASE='a8482d768bd2dcede42ea63ef39e39cf3245c380';ENTRY='7b3685fc18c6e392465f733b40a9aca7f4aaad74'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def fail(log):return set(re.findall(r'(pa22/tests/[^:]+\.t): ERROR:',log))
entry=(WORK/'entry.log').read_text();stage=(WORK/'stage-final.log').read_text();prior=(WORK/'prior-final.log').read_text()
old,new=fail(entry),fail(stage)
assert len(old)==8 and len(new)==5 and new<=old
assert '94 / 99 TESTS PASSED' in stage and '(3712 / 3712)' in prior
initial_prior=(WORK/'prior-initial.log').read_text()
recheck=(WORK/'pa3-recheck.log').read_text()
assert '3711 / 3712' in initial_prior and 'pa3/tests/300-triple.t: ERROR: tool timed out' in initial_prior
assert 'PASS (1/1)' in recheck
baseline=json.loads((ROOT/'student.tests/pa22/validation114.json').read_text())
manifest=baseline['contract_manifest']
paths=subprocess.check_output(['git','ls-files','pa22/tests'],cwd=ROOT,text=True).splitlines()
assert set(paths)=={p['path'] for p in manifest}
for item in manifest:assert sha(ROOT/item['path'])==item['sha256'],item['path']
assert sum(p.endswith('.t') for p in paths)==99
assert not subprocess.check_output(['git','diff','--name-only',BASE,'--','Makefile','scripts','shared','pa22/tests','pa22/Makefile','pa22/scripts'],cwd=ROOT,text=True).strip()
audit=subprocess.run(['perl','scripts/cppgm_file_audit.pl','--stage','pa22','--paths','dev/src'],cwd=ROOT,capture_output=True,text=True);assert audit.returncode==0
personal=json.loads((ROOT/'student.tests/pa22/controls116.json').read_text())
initial=json.loads((ROOT/'student.tests/pa22/controls116-entry.json').read_text())
inherited=json.loads((ROOT/'student.tests/pa22/controls116-inherited114.json').read_text())
assert len(personal)==len(initial)==8 and all(x['passed'] for x in personal)
assert len(inherited)==16 and all(x['passed'] for x in inherited)
roundtrips=json.loads((ROOT/'student.tests/pa22/roundtrips116.json').read_text())
assert {x['source'] for x in roundtrips}==old-new and all(x['stable_roundtrip'] for x in roundtrips)
assert sum('execution_note' not in x for x in roundtrips)==2
assert sha(ROOT/'dev/cppgm++')==sha(WORK/'final')
assert not subprocess.check_output(['git','diff','HEAD','--','dev'],cwd=ROOT,text=True).strip()
implementation=subprocess.check_output(['git','log','-1','--format=%H','--','dev/src'],cwd=ROOT,text=True).strip()
inherited115=json.loads((ROOT/'student.tests/pa22/controls116-inherited115.json').read_text())
assert len(inherited115)==12 and all(x['passed'] for x in inherited115)
perf=[]
for name in ['performance116-common.json','performance116-conversions.json']:
 path=ROOT/'student.tests/pa22'/name;data=json.loads(path.read_text())
 assert all(o['checked_exit']==0 for w in data['workloads'].values() for o in w['outputs'])
 assert all(len(w[k]['observations'])==20 for w in data['workloads'].values() for k in ['compiler','runtime'])
 assert all(w['identical_lowir'] and w['outputs'][0]['text_bytes']==w['outputs'][1]['text_bytes'] for w in data['workloads'].values())
 assert data['binaries'][0]['sha256']==sha(WORK/'entry')
 assert data['binaries'][1]['sha256']==sha(WORK/'final')
 perf.append(dict(path=str(path.relative_to(ROOT)),sha256=sha(path)))
plan=(ROOT/'pa22/plan.md').read_text()
assert f'Stage base commit: `{BASE}`' in plan and f'Last reviewed commit: `{BASE}`' in plan
result=dict(stage_base=BASE,entry_commit=ENTRY,implementation_commit=implementation,compiler_sha256=sha(WORK/'final'),
 previous_turn='Progress: prior type/query group committed; clean entry inspected and no live validation handle at turn start',
 priorThroughTests=dict(command='make test-report-through-pa21',exit=0,passed=3712,total=3712,log_sha256=sha(WORK/'prior-final.log')),
 initialPriorReport=dict(exit=2,passed=3711,total=3712,failure='PA3 300-triple timed out',log_sha256=sha(WORK/'prior-initial.log'),recheck_exit=0,recheck_log_sha256=sha(WORK/'pa3-recheck.log'),harness_and_timeouts_unchanged=True),
 stageTests=dict(command='make test-pa22',exit=2,passed=94,total=99,log_sha256=sha(WORK/'stage-final.log')),
 stageProgress=dict(entry_passed=91,entry_failed=8,final_passed=94,final_failed=5,unchanged_cases=99,improved=sorted(old-new),regressed=sorted(new-old),criterion_met=True),
 fileAudit=dict(command='perl scripts/cppgm_file_audit.pl --stage pa22 --paths dev/src',exit=audit.returncode,output=audit.stdout+audit.stderr),
 personal=dict(entry_passed=sum(x['passed'] for x in initial),final_passed=8,total=8,inherited_passed=28,inherited_total=28),
 supplemental=dict(validated_roundtrips=3,matching_executions=2,declaration_only=1),
 performance=perf,remaining_failures=sorted(new),contract_manifest=manifest,
 review='Independent review pending; stage/review markers preserved; five remaining failures are implementation obligations')
(ROOT/'student.tests/pa22/validation116.json').write_text(json.dumps(result,indent=2)+'\n')
print('PA22 94/99; 3 repaired, zero regressions; PA1–PA21 3712/3712; file audit pass; unchanged 99-case contract')
