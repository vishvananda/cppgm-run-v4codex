#!/usr/bin/env python3
"""Verify the frozen PA17 implementation handoff without replacing its tests."""
from pathlib import Path
import hashlib, json, statistics, subprocess

ROOT = Path(__file__).resolve().parents[2]
def read(name): return json.loads((ROOT/'student.tests/pa17'/name).read_text())
def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def git(*args): return subprocess.check_output(['git', *args], cwd=ROOT, text=True).strip()

e = read('handoff61-evidence.json')
assert git('rev-parse', 'HEAD:dev') == e['implementation_tree']
assert git('rev-parse', e['implementation_tip']+':dev') == e['implementation_tree']
for folder, tree in e['course_trees'].items():
    assert git('rev-parse', 'HEAD:'+folder) == tree
    assert git('rev-parse', e['entry_commit']+':'+folder) == tree
for marker in e['review_markers']:
    assert marker in (ROOT/'pa17/plan.md').read_text()
for record in e['artifacts']:
    assert sha(ROOT/record['path']) == record['sha256'], record['path']
for check in e['checks']:
    assert check['exit_code'] == 0
controls = read('handoff61-controls.json')
assert sum(len(rows) for rows in controls['inherited'].values()) == 610
assert all(row['passed'] for rows in controls['inherited'].values() for row in rows)
assert len(controls['closures']['final']) == 71
assert all(row['passed'] for row in controls['closures']['final'])
assert len(controls['regions']['final']) == 10
assert all(row['passed'] for row in controls['regions']['final'])
assert len(controls['regions']['structure_final']) == 5
assert all(controls['regions']['structure_final'].values())
assert len(controls['abi']) == 8 and all(row['passed'] for row in controls['abi'])
for family in ('closures', 'regions'):
    for point in ('entry', 'final'):
        for row in controls[family][point]:
            assert hashlib.sha256(row['source'].encode()).hexdigest() == row['source_sha256']
for report in ('handoff61-prelexical-performance.json', 'handoff61-performance.json'):
    p = read(report)
    assert not p['source_diff'] and p['finished_utc']
    assert sha(ROOT/'student.tests/pa17/handoff61_benchmark.py') == p['harness_sha256']
    for binary in p['binaries']:
        path = Path(binary['path'])
        if path.exists(): assert sha(path) == binary['sha256']
    for w in p['workloads'].values():
        assert hashlib.sha256(w['source'].encode()).hexdigest() == w['source_sha256']
        for out in w['outputs']:
            if 'native' in out: assert out['native']['checked_exit'] == 0
        if w['comparison'] == 'exact':
            assert w['outputs'][0]['sha256'] == w['outputs'][1]['sha256']
        for metric in ('compiler', 'runtime'):
            if metric not in w: continue
            data = w[metric]; rows = data['observations']
            if '0' in data:
                assert [r['binary'] for r in rows] == [0]*4 + [0,1,1,0]*4
                ratios = [statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1) /
                          statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0)
                          for j in range(4, len(rows), 4)]
                assert ratios == data['paired_b_over_a']
                assert statistics.median(ratios) == data['median_b_over_a']
            else: assert [r['binary'] for r in rows] == [1]*6
assert read('handoff61-performance.json')['source_commit'] == e['implementation_tip']
assert e['stage'] == dict(entry_passed=340, final_passed=343, total=343, original_failures_remaining=0)
assert not e['unfinished_implementation'] and e['independent_audit']
print('PA17 handoff: unchanged course trees; 704 controls; complete frozen performance evidence; review markers preserved')
