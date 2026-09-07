#!/usr/bin/env python3
"""Recompute campaign budgets from every saved observation (no timing rerun)."""
import hashlib
import json
from pathlib import Path
import re
import statistics
import sys

report = Path(sys.argv[1])
text = report.read_text()
manifests = json.loads(text.split('```json\n')[1].split('\n```')[0])
rows = []
for line in text.split('## Every observation')[1].splitlines():
    cells = [x.strip() for x in line.split('|')[1:-1]]
    if len(cells) == 6 and cells[2] in ('A', 'B'):
        rows.append(dict(workload=cells[0], block=cells[1], binary=cells[2], mode=cells[3],
                         seconds=float(cells[4]), rss=int(cells[5])))
assert len(rows) == len(manifests)*15
medians = {}
for manifest in manifests:
    name = manifest['workload']
    group = [r for r in rows if r['workload'] == name]
    assert len(group) == 15
    assert ''.join(r['binary'] for r in group[:6]) == 'AAAABB'
    assert ''.join(r['binary'] for r in group[6:14]) == 'ABBAABBA'
    assert group[14]['mode'] == 'stats'
    noise = max(abs(group[i+1]['seconds']/group[i]['seconds']-1) for i in (0,2,4))
    paired = []
    for i in (6,10):
        paired.append((group[i+1]['seconds']+group[i+2]['seconds']) /
                      (group[i]['seconds']+group[i+3]['seconds'])-1)
    assert max(paired) <= 0.10 + noise, (name, paired, noise)
    m = {}
    for kind in ('A', 'B'):
        samples = [r for r in group if r['binary'] == kind and r['mode'] == 'ordinary']
        m[kind] = {key: statistics.median(r[key] for r in samples) for key in ('seconds','rss')}
    assert m['B']['rss'] <= m['A']['rss']*1.15 + 1024, (name,m)
    medians[name] = m
    print(name, 'paired %:', [round(x*100,3) for x in paired], 'noise %:', round(noise*100,3))
for key, limit in [('seconds',6), ('rss',5)]:
    ratio = medians['flat-16MiB']['B'][key]/medians['flat-4MiB']['B'][key]
    assert ratio < limit
    print('4x input', key, 'ratio:', round(ratio,4))
by_name = {m['workload']:m for m in manifests}
nested = by_name['nested-arguments']['stats']
assert nested['captured_tokens'] == 600*128*3
assert nested['borrowed_arguments'] == 599*128
assert nested['max_prescan_depth'] == 600
assert nested['argument_prescans'] == 600*128
assert by_name['counter-spellings']['stats']['arena_bytes'] <= 131072
sizes = re.search(r'data\): (\d+) → (\d+) bytes', text)
assert sizes and int(sizes[2]) <= int(sizes[1])*1.15
if len(sys.argv) > 2:
    expected = re.search(r'; B: `([0-9a-f]{64})`', text)[1]
    assert hashlib.sha256(Path(sys.argv[2]).read_bytes()).hexdigest() == expected
assert 'N/A at PA4' in text
print('Verified all compiler work/growth budgets; generated runtime/text N/A.')
