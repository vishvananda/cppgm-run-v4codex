#!/usr/bin/env python3
"""Recompute every PA9 performance gate from retained observations and artifacts."""
import hashlib
import json
from pathlib import Path
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[2]
r = json.loads((ROOT / 'student.tests/pa9/performance.json').read_text())
def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
assert r['generated_program_runtime'] is None and r['generated_program_text_size'] is None
assert sha(ROOT / 'dev/abimangle') == r['binaries']['B']['sha256']
for binary in r['binaries'].values():
    assert sha(binary['path']) == binary['sha256']
    text = int(subprocess.check_output(['size', binary['path']], text=True).splitlines()[1].split()[0])
    assert text == binary['compiler_text_bytes']
# Documentation/evidence commits may follow the frozen implementation commit.
assert not subprocess.check_output(['git', 'diff', r['candidate_commit'], 'HEAD', '--', 'dev'], cwd=ROOT)
assert len(r['observations']) == 96
b = r['budgets']
assert b == dict(wall_ratio=1.25, rss_ratio=1.20, rss_slack_kib=16384,
                compiler_text_growth_bytes=102400, scale_wall_ratio=5.5,
                scale_work_ratio=4.5, scale_rss_ratio=5.0)
medians = {}
for name, workload in r['workloads'].items():
    assert sha(workload['path']) == workload['sha256']
    for label in 'AB':
        output = Path(workload['path']).with_suffix('.' + label + '.names')
        assert sha(output) == workload['output_sha256']
    rows = [x for x in r['observations'] if x['workload'] == name]
    assert len(rows) == 12 and all(x['exit_code'] == 0 and x['wall_seconds'] > 0 for x in rows)
    ratios = []
    for block, sequence in enumerate(('AAAA', 'ABBA', 'ABBA')):
        current = [x for x in rows if x['block'] == block]
        assert ''.join(x['binary'] for x in sorted(current, key=lambda x: x['position'])) == sequence
        if block:
            ratios.append(statistics.mean(x['wall_seconds'] for x in current if x['binary'] == 'B') /
                          statistics.mean(x['wall_seconds'] for x in current if x['binary'] == 'A'))
    assert ratios == r['summary'][name]['paired_wall_ratios']
    assert statistics.mean(ratios) <= b['wall_ratio']
    rss = {label: statistics.median(x['peak_rss_kib'] for x in rows if x['block'] and x['binary'] == label) for label in 'AB'}
    assert rss['B'] <= rss['A'] * b['rss_ratio'] + b['rss_slack_kib']
    medians[name] = (statistics.median(x['wall_seconds'] for x in rows if x['block'] and x['binary'] == 'B'), rss['B'])
for kind in ('templates', 'expressions', 'modifiers', 'batch'):
    small, large = medians[kind + '-1'], medians[kind + '-4']
    assert large[0] / small[0] <= b['scale_wall_ratio']
    assert large[1] / small[1] <= b['scale_rss_ratio']
    for counter in ('intern_requests', 'intern_probes', 'substitution_lookups', 'emitted_nodes'):
        c1, c4 = (r['workloads'][kind + '-' + str(scale)]['counters'][counter] for scale in (1, 4))
        assert c4 <= c1 * b['scale_work_ratio']
assert r['binaries']['B']['compiler_text_bytes'] - r['binaries']['A']['compiler_text_bytes'] <= b['compiler_text_growth_bytes']
assert all(r['budget_checks'].values())
print('frozen binaries, inputs, output equivalence, all 96 observations and unchanged budgets verified')
