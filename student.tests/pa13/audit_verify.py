#!/usr/bin/env python3
"""Verify final frozen identities, every observation and checked execution."""
from pathlib import Path
import hashlib, json, statistics, subprocess

root = Path(__file__).resolve().parents[2]
def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def verify(record):
    assert sha(record['path']) == record['sha256'], record['path']

data = json.loads((root/'student.tests/pa13/final-audit-performance.json').read_text())
prior = root/'student.tests/pa13/performance.json'
assert sha(prior) == data['prior_report_sha256']
assert sha(root/'student.tests/pa13/audit_benchmark.py') == data['harness_sha256']
assert sha(root/'student.tests/pa10/benchmark.py') == data['shared_harness_sha256']
assert sha(root/'reference-binaries/lowir2native') == data['backend_sha256']
for binary in data['binaries']:
    verify(binary)
assert sha(root/'dev/cppgm++') == data['binaries'][1]['sha256']
expected = set(json.loads(prior.read_text())['workloads'])
expected.update(name+'-'+scale for name in ('array','destructor','global-delete','conversion','operator')
                for scale in ('400','1600','runtime'))
assert set(data['workloads']) == expected
count = 0
for name, workload in data['workloads'].items():
    assert sha(workload['source_path']) == workload['source_sha256'], name
    outputs = workload['outputs']
    for output in outputs:
        verify(output)
        if 'native' in output:
            native = output['native']; verify(native)
            assert native['text_bytes'] > 0 and native['checked_exit'] == 0
            run = subprocess.run([native['path']], capture_output=True, timeout=60)
            assert run.returncode == 0, (name, run.returncode)
    if len(outputs) == 2:
        assert outputs[0]['sha256'] == outputs[1]['sha256'], name
        if 'native' in outputs[0]:
            assert outputs[0]['native']['sha256'] == outputs[1]['native']['sha256'], name
    for key in ('compiler', 'runtime'):
        if key not in workload: continue
        campaign = workload[key]; rows = campaign['observations']
        order = [0,0,0,0,0,1,1,0,0,1,1,0] if len(outputs) == 2 else [1]*6
        assert [r['binary'] for r in rows] == order, (name, key)
        for row in rows+campaign['warmups']:
            assert row['wall_s'] > 0 and row['rss_kib'] > 0 and row['checked_exit'] == 0
        if len(outputs) == 2:
            pairs = [statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 1)/
                     statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 0) for k in (4,8)]
            assert pairs == campaign['paired_b_over_a']
            assert campaign['aa_range_s'] == [min(r['wall_s'] for r in rows[:4]), max(r['wall_s'] for r in rows[:4])]
        count += len(rows)+len(campaign['warmups'])
for filename, harness, scaling in (
        ('final-noise-performance.json','audit_noise.py',False),
        ('final-scaling-performance.json','audit_scaling.py',True)):
    repeat = json.loads((root/'student.tests/pa13'/filename).read_text())
    assert repeat['source_report_sha256'] == sha(root/'student.tests/pa13/final-audit-performance.json')
    assert repeat['harness_sha256'] == sha(root/'student.tests/pa13'/harness)
    for name, workload in repeat['workloads'].items():
        rows = workload['observations']
        if scaling:
            assert [r['scale'] for r in rows] == [400,1600]*4
        else:
            assert sha(workload['source_path']) == workload['source_sha256']
            assert workload['output_hashes'] == [o['sha256'] for o in data['workloads'][name]['outputs']]
            assert [r['binary'] for r in rows] == [0]*4+[0,1,1,0]*4
            pairs = [statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 1)/
                     statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 0) for k in (4,8,12,16)]
            assert pairs == workload['paired_b_over_a']
        for row in rows+workload['warmups']:
            assert row['wall_s'] > 0 and row['rss_kib'] > 0 and row['checked_exit'] == 0
        count += len(rows)+len(workload['warmups'])
assert count == 550, count
print(len(expected), 'workloads,', count, 'observations; frozen hashes, pairs and native results passed')
