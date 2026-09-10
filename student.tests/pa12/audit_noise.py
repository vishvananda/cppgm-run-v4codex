#!/usr/bin/env python3
"""Retain a focused frozen repeat for the final audit's noisy compiler rows."""
from pathlib import Path
import json
import os
import statistics
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'student.tests/pa10'))
import benchmark as shared

campaign, work, output = map(Path, sys.argv[1:4])
original = json.loads(campaign.read_text())
os.sched_setaffinity(0, {original['cpu']})
work.mkdir(parents=True, exist_ok=True)
data = dict(protocol='same frozen inputs/binaries; warmups, four A/A then two ABBA blocks',
    cpu=original['cpu'], binaries=original['binaries'], harness_sha256=shared.sha(__file__),
    original_campaign_sha256=shared.sha(campaign), workloads={})
for binary in data['binaries']: assert shared.sha(binary['path']) == binary['sha256']
for name in ('template-semantics-1', 'template-semantics-4', 'calls-4'):
    previous = original['workloads'][name]
    source = Path(previous['source_path'])
    assert shared.sha(source) == previous['source_sha256']
    commands, outputs = [], []
    for label, binary in enumerate(data['binaries']):
        target = work/(name+'-'+str(label)+'.out')
        command = [binary['path'], previous['mode'], *(['-O0'] if previous['mode']=='--emit-lowir' else []),
                   '-o', target, source]
        shared.run(command)
        assert shared.sha(target) == previous['outputs'][label]['sha256']
        commands.append(command)
        outputs.append(dict(path=str(target), sha256=shared.sha(target)))
    def observe(label):
        usage = work/'usage.txt'; start = time.perf_counter_ns()
        shared.run(['/usr/bin/time', '-f', '%M %U %S %c %w', '-o', usage, *commands[label]])
        wall = (time.perf_counter_ns()-start)/1e9
        rss, user, system, involuntary, voluntary = usage.read_text().split()
        return dict(binary=label, wall_s=wall, rss_kib=int(rss), user_s=float(user), system_s=float(system),
                    involuntary=int(involuntary), voluntary=int(voluntary), checked_exit=0)
    warmups = [observe(label) for label in (0,1)]
    rows = [observe(label) for label in shared.ORDER]
    pairs = [statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==1)/
             statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary']==0) for k in (4,8)]
    data['workloads'][name] = dict(source_path=str(source), source_sha256=shared.sha(source),
        mode=previous['mode'], outputs=outputs, warmups=warmups, observations=rows, paired_b_over_a=pairs)
    output.write_text(json.dumps(data, indent=2)+'\n'); print(name, pairs, flush=True)
for binary in data['binaries']: assert shared.sha(binary['path']) == binary['sha256']
