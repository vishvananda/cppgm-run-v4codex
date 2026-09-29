#!/usr/bin/env python3
"""Record PA22 handoff gates, unchanged coverage, controls and frozen evidence."""
from pathlib import Path
import hashlib, json, re, subprocess
ROOT = Path(__file__).resolve().parents[2]
WORK = Path('/tmp/pa22-118')
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def run(command):
    result = subprocess.run([str(x) for x in command],cwd=ROOT,capture_output=True,text=True)
    assert result.returncode == 0, (command,result.stderr)
    return result
def git(*args): return run(['git',*args]).stdout.strip()
entry = '0c710f745f1341536992f43b24ba325578a2fad3'
manifest = json.loads((ROOT/'student.tests/pa22/validation116.json').read_text())['contract_manifest']
assert set(git('ls-files','pa22/tests').splitlines()) == {x['path'] for x in manifest}
for item in manifest: assert sha(ROOT/item['path']) == item['sha256']
assert not git('diff',entry,'--','Makefile','scripts','shared',*[f'pa{i}/tests' for i in range(1,23)],'pa22/Makefile','pa22/scripts')
plan = (ROOT/'pa22/plan.md').read_text()
assert 'Stage base commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`' in plan
assert 'Last reviewed commit: `e90fa3fa514e2990bbe5ea4716252d8c42ae782a`' in plan
stage = (WORK/'stage-sealed.log').read_text()
prior = (WORK/'prior-sealed.log').read_text()
audit = (WORK/'file-audit-sealed.log').read_text()
assert '95 / 99 TESTS PASSED' in stage and '(3712 / 3712)' in prior
assert 'File audit passed for pa22 with 3 warning(s).' in audit
through = (WORK/'through22-sealed.log').read_text()
assert '3807 / 3811' in through
checks = json.loads((WORK/'checks-sealed.json').read_text())
assert [x['exit'] for x in checks] == [2,0,0]
for check in checks: check['log_sha256'] = sha(Path(check['log']))
old = json.loads((ROOT/'student.tests/pa22/audit117-final-validation.json').read_text())['stageProgressPreserved']['entry_failures']
failures = sorted(set(re.findall(r'(pa22/tests/[^:]+\.t): ERROR:',stage)))
assert len(failures)==4 and set(failures)<set(old)
assert sha(ROOT/'dev/cppgm++') == sha(WORK/'sealed')
controls = {}
for name,total in [('',36),('-inherited114',16),('-inherited115',12),('-inherited116',8),('-audit117',18),('-constants117',4)]:
    path = ROOT/f'student.tests/pa22/controls118{name}.json'
    data = json.loads(path.read_text()); rows = data['cases'] if isinstance(data,dict) else data
    assert len(rows)==total and all(r['passed'] for r in rows)
    if isinstance(data,dict): assert data['compiler_sha256']==sha(WORK/'sealed')
    controls[name or 'new'] = dict(passed=total,total=total,sha256=sha(path))
# All 95 accepted outputs must still be valid and stable through the IR adapter.
roundtrips = []; (WORK/'roundtrips').mkdir(exist_ok=True)
for src in sorted((ROOT/'pa22/tests').rglob('*.t')):
    status = src.with_suffix('.my.exit_status').read_text().strip()
    if status != 'EXIT_SUCCESS':
        assert status == src.with_suffix('.ref.exit_status').read_text().strip()
        roundtrips.append(dict(source=str(src.relative_to(ROOT)),rejected=True)); continue
    first = WORK/'roundtrips'/(src.stem+'.first.lowir'); second = first.with_suffix('.second.lowir')
    run([ROOT/'dev/lowir','-o',first,src.with_suffix('.my')]); run([ROOT/'dev/lowir','-o',second,first])
    assert first.read_bytes()==second.read_bytes()
    roundtrips.append(dict(source=str(src.relative_to(ROOT)),rejected=False,stable_roundtrip=True,sha256=sha(second)))
assert len(roundtrips)==99 and sum(r['rejected'] for r in roundtrips)==4
# The ABI truth probe covers a null target with a nonzero adjustment word.
ir=WORK/'truth.lowir'; merged=WORK/'truth-merged.lowir'; exe=WORK/'truth'
commands=[[ROOT/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',ir,ROOT/'student.tests/pa22/member-truth-word.source'],
          [ROOT/'dev/lowir','-o',merged,ir,ROOT/'student.tests/pa22/member-truth-word.lowir'],
          [ROOT/'dev/lowir2native-ref','-O0','-o',exe,merged],[exe]]
for command in commands: run(command)
performance = []
for mode,count in [('common',6),('fields',4)]:
    path = ROOT/f'student.tests/pa22/performance118-{mode}-sealed.json'; data=json.loads(path.read_text())
    assert len(data['workloads'])==count
    assert [b['sha256'] for b in data['binaries']] == [sha(WORK/'entry'),sha(WORK/'sealed')]
    for workload in data['workloads'].values():
        assert all(len(workload[k]['observations'])==20 for k in ('compiler','runtime'))
        assert all(r['checked_exit']==0 for k in ('compiler','runtime') for r in workload[k]['observations'])
        if mode=='common': assert workload['identical_lowir']
    performance.append(dict(path=str(path.relative_to(ROOT)),sha256=sha(path)))
result = dict(entry=entry,implementation=git('rev-parse','HEAD'),compiler_sha256=sha(WORK/'sealed'),
              checks=checks,stageProgress=dict(passed=True,entry_failures=old,final_failures=failures,
              fixed=sorted(set(old)-set(failures)),coverage=99,contract_unchanged=True),
              controls=controls,abi_truth_probe=dict(passed=True,commands=[[str(x) for x in c] for c in commands]),
              roundtrips=roundtrips,performance=performance,
              through_pa22=dict(command='make test-report-through-pa22',passed=False,
              tests_passed=3807,tests_total=3811,log_sha256=sha(WORK/'through22-sealed.log')),
              implementation_sources={p:sha(ROOT/p) for p in git('diff','--name-only',entry,'--','dev').splitlines()},
              references_changed=False,comparison_rules_changed=False,independent_review_pending=True)
(ROOT/'student.tests/pa22/validation118.json').write_text(json.dumps(result,indent=2)+'\n')
print('PA22 94->95/99; prior 3712/3712; file audit pass; 94/94 controls + ABI truth; 95 roundtrips + 4 rejections')
