#!/usr/bin/env python3
"""Time checked native frontend controls; these short runs measure startup.

Run with the initial evidence's executable directory, the member evidence's
executable directory, and a new output JSON path. Reject any changed executable.
"""
from pathlib import Path
import hashlib
import json
import os
import statistics
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent
initial, member, out = map(Path, sys.argv[1:])
assert not out.exists(), 'Preserve earlier observations'
cpu = max(os.sched_getaffinity(0))
os.sched_setaffinity(0, {cpu})
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
result = dict(cpu=cpu, harness_sha256=sha(Path(__file__)),
    protocol='one warmup each, four A/A samples, four ABBA blocks; six final-only samples',
    limitation='Startup-dominated checked executions; no speedup or scaling claims.',
    workloads={})
for filename, directory in [('performance91-initial.json', initial),
                            ('performance91-member.json', member)]:
    evidence = HERE / filename
    for name, item in json.loads(evidence.read_text())['workloads'].items():
        if name.startswith('runtime-') or 'compiler' not in item:
            continue
        executables = {}
        for output in item['outputs']:
            i = output['binary']
            path = directory / (name + '-' + str(i))
            assert sha(path) == output['native']['sha256']
            executables[i] = path
        def observe(i):
            start = time.perf_counter_ns()
            subprocess.run([executables[i]], check=True, capture_output=True, timeout=30)
            return dict(binary=i, wall_s=(time.perf_counter_ns()-start)/1e9, checked_exit=0)
        warmups = [observe(i) for i in executables]
        both = len(executables) == 2
        rows = [observe(i) for i in ([0]*4 + [0,1,1,0]*4 if both else [1]*6)]
        data = dict(evidence=filename, evidence_sha256=sha(evidence),
            native=[o['native'] for o in item['outputs']], warmups=warmups, observations=rows)
        if both:
            aa = [r['wall_s'] for r in rows[:4]]
            data['aa_range_s'] = [min(aa), max(aa)]
            data['paired_b_over_a'] = [
                statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary'] == 1) /
                statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary'] == 0)
                for j in range(4, len(rows), 4)]
        for i in executables:
            samples = [r['wall_s'] for r in rows[4 if both else 0:] if r['binary'] == i]
            data[str(i)] = dict(median_wall_s=statistics.median(samples),
                               wall_range_s=[min(samples), max(samples)])
        result['workloads'][name] = data
out.write_text(json.dumps(result, indent=2) + '\n')
print('PASS:', len(result['workloads']), 'startup-only native controls')
