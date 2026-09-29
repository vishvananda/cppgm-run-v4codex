#!/usr/bin/env python3
"""Bind final PA18 audit checks, coverage, architecture trace and measurements: WORK."""
from pathlib import Path
import hashlib, json, subprocess, sys

ROOT = Path(__file__).resolve().parents[2]
W = Path(sys.argv[1]).resolve()
BASE = '94dcb8ad21664137e87d574e878c14a4a047348a'
REVIEW = 'f6eaf8ab213c90eefffd48f5c47b1ac9a75bce8d'

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT, text=True).strip()

checks = json.loads((W/'validation-complete/checks.json').read_text())
assert len(checks) == 65
assert all(c['exit'] == 0 and sha(c['path']) == c['sha256'] for c in checks.values())
progress = json.loads((W/'validation-complete/stage-progress.json').read_text())
assert progress['final_failures'] == 0 and progress['compiler_sha256'] == sha(ROOT/'dev/cppgm++')
assert not git('diff', 'HEAD', '--', 'dev')

personal = {}
for path in sorted((W/'validation-complete').rglob('results.json')):
    rows = json.loads(path.read_text())
    if isinstance(rows, list) and rows and all(isinstance(r, dict) and 'passed' in r for r in rows):
        assert all(r['passed'] for r in rows), path
        personal[str(path.parent.relative_to(W/'validation-complete'))] = dict(count=len(rows), sha256=sha(path))

entry = json.loads(Path('/tmp/pa18-loop88/controls-entry/results.json').read_text())
final = json.loads((W/'validation-complete/audit88_controls/results.json').read_text())
assert {r['name']: r['source_sha256'] for r in entry} == {r['name']: r['source_sha256'] for r in final}
assert all(r['passed'] for r in final)

suites = [f'pa{i}/tests' for i in range(1, 19)]
original = git('ls-tree', '-r', '--name-only', BASE, '--', *suites).splitlines()
current = git('ls-files', '--', *suites).splitlines()
assert original == current
changes = git('diff', '--name-only', BASE, '--', *suites).splitlines()
proofs = {
    'pa18/tests/spec/500-conversion-function-template-reference-conditional-auto-ref.ref': [65],
    'pa9/tests/abi/300-function-owner-member-pointer-nttp-data.ref': [67],
    'pa18/tests/spec/300-scalar-pseudo-destructor-noexcept.ref.exit_status': [69],
    'pa18/tests/general/300-function-template-result-first-lookup.ref.exit_status': [79],
}
for n in (83, 84, 85, 87):
    manifest = json.loads((ROOT/f'student.tests/pa18/reference{n}-revisions.json').read_text())
    for row in manifest['revisions']:
        proofs.setdefault(row['path'], []).append(n)
assert set(changes) == set(proofs), (changes, proofs)
assert len(changes) == 29
assert not git('diff', BASE, '--', 'scripts', 'Makefile', 'spec.md', 'AGENTS.md',
               *[f'pa{i}/Makefile' for i in range(1, 19)])
reference_checks = json.loads((W/'references/checks.json').read_text())
assert all(c['exit'] == 0 for c in reference_checks.values())
observations = ROOT/'student.tests/pa18/loop89-reference-complete.json'
assert all(r['passed'] for r in json.loads(observations.read_text())['observations'])

perf_path = ROOT/'student.tests/pa18/loop89-performance-complete.json'
perf = json.loads(perf_path.read_text())
assert perf.get('finished_utc') and perf['binaries'][1]['sha256'] == sha(ROOT/'dev/cppgm++')
assert perf['binaries'][0]['sha256'] == sha('/tmp/pa18-loop88/entry-cppgm')
observed = len(perf['startup']['warmups']) + len(perf['startup']['observations'])
for work in perf['workloads'].values():
    assert hashlib.sha256(work['source'].encode()).hexdigest() == work['source_sha256']
    for key in ('compiler', 'runtime'):
        if key not in work:
            continue
        measurement = work[key]
        observed += len(measurement['warmups']) + len(measurement['observations'])
        assert all(r['checked_exit'] == 0 for r in measurement['observations'])
        assert len(measurement['observations']) == (20 if len(measurement['warmups']) == 2 else 6)

historical = []
for filename in ('loop89-performance.json', 'loop89-performance-ordering-repeat.json', 'loop89-performance-final.json'):
    path = ROOT/'student.tests/pa18'/filename
    data = json.loads(path.read_text())
    assert data.get('finished_utc')
    historical.append(dict(path=str(path.relative_to(ROOT)), sha256=sha(path), binaries=data['binaries']))

historical_references = []
for filename in ('loop89-reference.json', 'loop89-reference-final.json'):
    path = ROOT/'student.tests/pa18'/filename
    data = json.loads(path.read_text())
    assert all(r['passed'] for r in data['observations'])
    historical_references.append(dict(path=str(path.relative_to(ROOT)), sha256=sha(path)))

trace = [json.loads(s) for s in (W/'validation-complete/audit88-trace.log').read_text().splitlines()]
record = dict(
    stage_base=BASE, previous_review=REVIEW, reviewed_code=git('log', '-1', '--format=%H', '--', 'dev'),
    compiler_sha256=sha(ROOT/'dev/cppgm++'), checks=checks, personal=personal,
    new_controls=dict(entry_passed=sum(r['passed'] for r in entry), final_passed=len(final),
                      entry_results_sha256=sha('/tmp/pa18-loop88/controls-entry/results.json'),
                      final_results_sha256=sha(W/'validation-complete/audit88_controls/results.json')),
    course=dict(stage='420/420', prior='2609/2609', through='3029/3029', stages=18,
                separate_properties=22, tracked_inputs=3053,
                excluded_from_default_roots=dict(pa8_later_debug=18, pa9_top_level=6),
                pa18_fixture_paths=progress['files']),
    fixture_paths=len(current), fixture_path_list_sha256=hashlib.sha256('\n'.join(current).encode()).hexdigest(),
    reference_corrections={p: dict(proofs=proofs[p], final_sha256=sha(ROOT/p)) for p in changes},
    reference_checks=reference_checks, reference_observations=dict(path=str(observations.relative_to(ROOT)), sha256=sha(observations)),
    architecture_trace=trace, historical_performance=historical, historical_reference_observations=historical_references,
    initial_checks=json.loads((W/'validation/checks.json').read_text()), performance=dict(path=str(perf_path.relative_to(ROOT)), sha256=sha(perf_path),
        observations=observed, workloads=len(perf['workloads']), binaries=perf['binaries']),
    reviewed_source_sha256={p: sha(ROOT/p) for p in git('diff', '--name-only', REVIEW, '--', 'dev').splitlines()},
    disposition='PA18 full-stage audit passes; PA19 was not started.')
out = ROOT/'student.tests/pa18/loop89-evidence.json'
out.write_text(json.dumps(record, indent=2)+'\n')
print('Bound', len(checks), 'checks;', sum(r['count'] for r in personal.values()),
      'personal cases;', observed, 'performance observations;', len(changes), 'proved oracle changes.')
