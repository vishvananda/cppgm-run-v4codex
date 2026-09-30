#!/usr/bin/env python3
"""Recheck the inherited fixed template workload through our native backend."""
import hashlib
import json
import os
import pathlib
import re
import statistics
import subprocess
import sys
import time

root = pathlib.Path(__file__).resolve().parents[2]
a, b, backend, out = [pathlib.Path(p).resolve() for p in sys.argv[1:5]]
out.mkdir(parents=True, exist_ok=True)
cpu = min(os.sched_getaffinity(0))
os.sched_setaffinity(0, {cpu})


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


source = out/'templates.cpp'
source.write_text(json.loads((root/'student.tests/pa23/performance126.json').read_text())
                  ['workloads']['auto-specializations-9600']['source'])
data = dict(cpu=cpu, input_sha256=digest(source), flags=['-O0', '--emit-lowir'],
            binaries={k:dict(path=str(p), sha256=digest(p)) for k, p in [('A', a), ('B', b), ('backend', backend)]},
            checks=[], observations=[])
commands = {}
for label, cc in [('A', a), ('B', b)]:
    ir = out/(label+'.lowir')
    commands[label] = [str(cc), '-O0', '--emit-lowir', '-o', str(ir), str(source)]
    subprocess.run(commands[label], check=True, timeout=60)
    checked = subprocess.run(commands[label] + ['--stats', '--validate-lowir'], capture_output=True, text=True, check=True, timeout=60)
    (out/(label+'-frontend.stats')).write_text(checked.stderr)
    exe = out/label
    checked = subprocess.run([str(backend), '--stats', '-o', str(exe), str(ir)], capture_output=True, text=True, check=True, timeout=60)
    (out/(label+'-native.stats')).write_text(checked.stderr)
    run = subprocess.run([str(exe)], check=True, timeout=10)
    data['checks'].append(dict(lane=label, lowir_sha256=digest(ir), executable_sha256=digest(exe),
                               text_bytes=int(re.search(r'text_bytes=(\d+)', checked.stderr)[1]), exit_code=run.returncode))
assert data['checks'][0]['lowir_sha256'] == data['checks'][1]['lowir_sha256']
assert data['checks'][0]['executable_sha256'] == data['checks'][1]['executable_sha256']
for block, order in [('AA', 'AAAA')] + [(str(i), 'ABBA') for i in range(6)]:
    for lane in order:
        start = time.perf_counter()
        run = subprocess.run(['/usr/bin/time', '-f', 'RSS=%M', *commands[lane]], capture_output=True, text=True, check=True, timeout=60)
        data['observations'].append(dict(block=block, lane=lane, wall_s=time.perf_counter()-start,
                                         rss_kib=int(re.search(r'RSS=(\d+)', run.stderr)[1])))
rows = data['observations']
data['paired_B_over_A'] = [statistics.mean(r['wall_s'] for r in rows if r['block'] == str(i) and r['lane'] == 'B') /
                           statistics.mean(r['wall_s'] for r in rows if r['block'] == str(i) and r['lane'] == 'A') for i in range(6)]
data['summary'] = {lane:dict(wall_median=statistics.median(r['wall_s'] for r in rows if r['block'] != 'AA' and r['lane'] == lane),
                           rss_max=max(r['rss_kib'] for r in rows if r['lane'] == lane)) for lane in ('A', 'B')}
data['AA_range'] = [min(r['wall_s'] for r in rows[:4]), max(r['wall_s'] for r in rows[:4])]
(out/'observations.json').write_text(json.dumps(data, indent=2)+'\n')
print(json.dumps(data['summary']))
