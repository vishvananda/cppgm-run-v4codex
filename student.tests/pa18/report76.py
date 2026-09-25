#!/usr/bin/env python3
"""Refresh the numerical tables in the handoff report from frozen observations."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[2]
p=json.loads((ROOT/'student.tests/pa18/loop76-performance-final.json').read_text())
assert p['finished_utc']
path=ROOT/'pa18/performance76.md';s=path.read_text()
s=s.replace('final `bff709db`','final `c8a2aad8`').replace('[loop76-performance.json](../student.tests/pa18/loop76-performance.json)','[loop76-performance-final.json](../student.tests/pa18/loop76-performance-final.json)')
if '[the first batch]' not in s:s=s.replace('Each equivalent workload has one warmup per binary, four A/A observations and','The earlier `bff709db` measurements remain unchanged in\n[the first batch](../student.tests/pa18/loop76-performance.json); the final batch\nfollows the omitted-type demand fix.\nEach equivalent workload has one warmup per binary, four A/A observations and')
for kind,header in [('compiler','| Workload | A / B median ms'),('runtime','| Workload | A / B median seconds')]:
 start=s.index(header);end=s.index('\n\n',start)
 table=s[start:end].splitlines()[:2]
 for name,w in p['workloads'].items():
  if kind not in w:continue
  q=w[kind];a=q.get('0');b=q['1'];ratios=q.get('paired_b_over_a')
  factor=1000 if kind=='compiler' else 1;dec=2 if kind=='compiler' else 6
  times=(f'{a["median_wall_s"]*factor:.{dec}f}' if a else '—')+f' / {b["median_wall_s"]*factor:.{dec}f}'
  ratio=f'{q["median_b_over_a"]:.3f} ({min(ratios):.3f}–{max(ratios):.3f})' if ratios else 'new behavior'
  if kind=='compiler':sizes=(str(a['peak_rss_kib']) if a else '—')+' / '+str(b['peak_rss_kib'])
  else:sizes=('' if a else '— / ')+' / '.join(str(o['native']['payload_bytes']) for o in w['outputs'])
  table.append(f'| {name} | {times} | {ratio} | {sizes} |')
 s=s[:start]+'\n'.join(table)+s[end:]
a,b=p['binaries'];growth=b['text_bytes']-a['text_bytes']
start=s.index('Compiler `.text`:');end=s.index('\nGenerated equivalent',start)
s=s[:start]+f'Compiler `.text`: **{a["text_bytes"]:,} → {b["text_bytes"]:,} bytes**, **+{growth:,} ({100*growth/a["text_bytes"]:.3f}%)**.'+s[end:]
start=s.index('Startup medians:');end=s.index('\n\nCompiler `.text`:',start)
lat=[p['startup'][str(i)]['median_wall_s']*1000 for i in range(2)]
equivalent=[(name,w['compiler']['1']['peak_rss_kib']-w['compiler']['0']['peak_rss_kib']) for name,w in p['workloads'].items() if w['comparison']=='exact']
name,growth=max(equivalent,key=lambda x:x[1])
s=s[:start]+f'''Startup medians: **{lat[0]:.2f} / {lat[1]:.2f} ms**. Small runtime-source compilations are
startup-limited; the scaled frontend workloads dominate startup. All observations
and A/A spreads remain in the raw record. No general compiler speedup is claimed.
The largest equivalent peak-RSS increase is **{growth} KiB**, on {name}.
Timing variation is visible in the paired ranges; equivalent measured workloads
show no repeatable material regression requiring a new optimization.'''+s[end:]
start=s.index('At 600 →');end=s.index(' The twelve independent',start)
r=[]
for family,a,b in [('inherited-declaration',600,2400),('inherited-query',600,2400),('inherited-body',160,640)]:
 r.append(p['workloads'][f'{family}-{b}']['compiler']['1']['median_wall_s']/p['workloads'][f'{family}-{a}']['compiler']['1']['median_wall_s'])
s=s[:start]+f'''At 600 → 2400 classes, inherited declaration/query compiler medians grow
**{r[0]:.2f}× / {r[1]:.2f}×** for fourfold input. At 160 → 640 demanded bodies, time grows
**{r[2]:.2f}×**.'''+s[end:]
path.write_text(s)
