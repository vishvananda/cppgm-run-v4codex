#!/usr/bin/env python3
"""Preserved follow-up measurements for noisy common cases and changed zeroing."""
from pathlib import Path
import hashlib, json, os, statistics, subprocess, sys, time

campaign, output, work = map(Path, sys.argv[1:4])
assert not output.exists()
p = json.loads(campaign.read_text()); work.mkdir(parents=True, exist_ok=True)
os.sched_setaffinity(0, {p['cpu']})
def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
for b in p['binaries']: assert sha(b['path']) == b['sha256']
result = dict(reason='Initial common compiler samples show large bidirectional wall-time excursions; repeat with identical binaries/inputs. Confirm the changed union-zero runtime separately.',
              campaign_sha256=sha(campaign), harness_sha256=sha(__file__), cpu=p['cpu'], binaries=p['binaries'],
              protocol=p['protocol'], started_utc=time.strftime('%Y-%m-%d %H:%M:%S', time.gmtime()), workloads={})
def observe(command):
    usage = work/'usage.txt'; start = time.perf_counter_ns()
    subprocess.run(['/usr/bin/time', '-f', '%M %U %S %c %w', '-o', usage, *map(str, command)], check=True)
    rss, user, system, iv, v = usage.read_text().split()
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9, rss_kib=int(rss), user_s=float(user), system_s=float(system),
                involuntary=int(iv), voluntary=int(v), checked_exit=0)
def measure(commands):
    warmups = [dict(binary=i, **observe(c)) for i, c in enumerate(commands)]
    rows = [dict(binary=i, **observe(commands[i])) for i in [0]*4+[0, 1, 1, 0]*4]
    ratios = [statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary'] == 1)/
              statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary'] == 0) for j in range(4, 20, 4)]
    m = dict(warmups=warmups, observations=rows, paired_b_over_a=ratios, median_b_over_a=statistics.median(ratios),
             aa_range_s=[min(r['wall_s'] for r in rows[:4]), max(r['wall_s'] for r in rows[:4])])
    for i in (0, 1):
        sample = [r for r in rows[4:] if r['binary'] == i]
        m[str(i)] = dict(median_wall_s=statistics.median(r['wall_s'] for r in sample),
                        wall_range_s=[min(r['wall_s'] for r in sample), max(r['wall_s'] for r in sample)],
                        peak_rss_kib=max(r['rss_kib'] for r in sample))
    return m
for name in ['common-partials-1500', 'common-loop-float-1500', 'common-loop-float-6000',
             'member-qualified-1500', 'member-qualified-6000', 'union-zero-runtime']:
    w = p['workloads'][name]; src = work/(name+'.cpp'); src.write_text(w['source'])
    assert sha(src) == w['source_sha256']
    commands = [[b['path'], *p['flags'], '-o', work/(name+f'-{i}.lowir'), src] for i, b in enumerate(p['binaries'])]
    item = dict(source_sha256=w['source_sha256'], compiler=measure(commands))
    for i in (0, 1): assert sha(work/(name+f'-{i}.lowir')) == w['outputs'][i]['sha256']
    if 'runtime' in w:
        for o in w['outputs']: assert sha(o['native']['path']) == o['native']['sha256']
        item['runtime'] = measure([[o['native']['path']] for o in w['outputs']])
    result['workloads'][name] = item
    output.write_text(json.dumps(result, indent=2)+'\n'); print(name, 'measured', flush=True)
result['finished_utc'] = time.strftime('%Y-%m-%d %H:%M:%S', time.gmtime())
output.write_text(json.dumps(result, indent=2)+'\n')
