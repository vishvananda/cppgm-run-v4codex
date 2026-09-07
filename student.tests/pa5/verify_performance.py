#!/usr/bin/env python3
"""Recompute every performance gate from the retained raw observations."""
import hashlib
import json
import pathlib
import statistics as st
import sys
from bench_inputs import workloads

report = pathlib.Path(sys.argv[1])
data = json.loads(report.read_text())
final = pathlib.Path(sys.argv[2] if len(sys.argv)>2 else 'dev/cppgm++')
assert hashlib.sha256(final.read_bytes()).hexdigest() == data['binaries']['B']['sha256']
inputs = {f'{name}-{scale}.cpp': source for scale in (1,4) for name,source in workloads(scale).items()}
assert set(inputs) == set(data['inputs'])
for name,source in inputs.items():
    assert hashlib.sha256(source.encode()).hexdigest() == data['inputs'][name]['sha256']
assert len(data['observations']) == 14*len(inputs)
assert len(data['startup']) == 8
host_growth = data['binaries']['B']['text_bytes']/data['binaries']['A']['text_bytes']-1
assert host_growth <= .15
summary = {}
for name in inputs:
    rows = [r for r in data['observations'] if r['input']==name]
    assert len({r['output_sha256'] for r in rows})==1
    blocks = {b:[r for r in rows if r['block']==b] for b in ('AA1','AA2','BB','ABBA1','ABBA2')}
    assert [r['variant'] for r in blocks['AA1']] == ['A','A']
    assert [r['variant'] for r in blocks['AA2']] == ['A','A']
    assert [r['variant'] for r in blocks['BB']] == ['B','B']
    noise = max(abs(v[0]['wall_s']-v[1]['wall_s'])/st.mean(r['wall_s'] for r in v)
                for b,v in blocks.items() if b.startswith('AA'))
    gains = []
    for block in ('ABBA1','ABBA2'):
        rows_block = blocks[block]
        assert [r['variant'] for r in rows_block] == list('ABBA')
        a = [r for r in rows_block if r['variant']=='A']
        b = [r for r in rows_block if r['variant']=='B']
        at,bt = (st.mean(r['wall_s'] for r in v) for v in (a,b))
        gains.append(1-bt/at)
        assert bt <= at*(1.10+noise),(name,block,'wall budget',at,bt,noise)
        ar,br = (st.mean(r['rss_kib'] for r in v) for v in (a,b))
        assert br <= ar*1.15+1024,(name,block,'RSS budget',ar,br)
    ab = [r for r in rows if r['block'].startswith('ABBA')]
    variants = [[r for r in ab if r['variant']==v] for v in ('A','B')]
    medians = [[st.median(r[key] for r in v) for key in ('wall_s','rss_kib')] for v in variants]
    bstats = variants[1][0]['stats']
    assert bstats['delimiter_work']==bstats['tokens']
    assert bstats['node_growths'] <= bstats['node_capacity'].bit_length()
    if name.startswith('nested'):
        assert bstats['angle_work'] < bstats['tokens']*2
        assert min(gains)>noise,(name,'benefit must exceed noise in both blocks')
    summary[name] = {'A_wall':medians[0][0], 'B_wall':medians[1][0],
                     'A_rss':medians[0][1], 'B_rss':medians[1][1],
                     'gains_percent':[x*100 for x in gains], 'AA_noise_percent':noise*100,
                     'B_wall_spread':[min(r['wall_s'] for r in variants[1]),max(r['wall_s'] for r in variants[1])],
                     'stats':bstats}
for name in workloads():
    small,large=summary[f'{name}-1.cpp'],summary[f'{name}-4.cpp']
    assert large['B_wall'] < small['B_wall']*6,(name,'4x wall scaling')
    assert large['B_rss'] < small['B_rss']*5+1024,(name,'4x RSS scaling')
    if name=='nested':
        assert large['stats']['angle_work'] < small['stats']['angle_work']*5
        assert large['stats']['nodes'] < small['stats']['nodes']*5
print(json.dumps({'host_text_growth_percent':host_growth*100,'workloads':summary},indent=2))
print('PASS: frozen identities, exact outputs, protocol, all budgets and nested-work bounds',file=sys.stderr)
