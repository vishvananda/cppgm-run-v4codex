#!/usr/bin/env python3
"""Render the complete sealed PA23 performance observations without sample filtering."""
from pathlib import Path
import json,statistics
ROOT=Path(__file__).resolve().parents[2];x=json.loads((ROOT/'student.tests/pa23/performance123.json').read_text())
lines=['# PA23/O0 performance — handoff 123','',
'Implementation `6a762f25`, compared with entry `9688f9e5`. [All sealed observations](../student.tests/pa23/performance123.json) freeze source inputs, hashes, source flags, binaries, CPU affinity, platform, output hashes and telemetry. The binaries remain at `/tmp/pa23-123/{entry,sealed-final}`. The [pre-containment-cache series](../student.tests/pa23/performance123-before-containment-cache.json) and [pre-RTTI-hint series](../student.tests/pa23/performance123-before-rtti-hint.json), with their distinct binaries, are preserved; they are not relabeled as final.','',
'[`benchmark123.py`](../student.tests/pa23/benchmark123.py) uses one warmup per lane, four A/A samples and four ABBA blocks. Compiler wall time and peak RSS are measured separately from checked native execution and text size. Runtime loops use volatile iteration counts and checked sums. Common templates, calls, memory, floating point and member workloads are frozen from PA22 evidence. Source compilation uses `--emit-lowir -O0`; the compiler build uses the existing `g++ -std=gnu++11 -Wall -O3` configuration. The supplied backend compiles our LowIR; the host linker links those objects. Telemetry and explicit validation preserve LowIR hashes.','',
'## Comparable correct workloads','',
'Times are median milliseconds, RSS is peak KiB, text is bytes. Each paired range includes every block. Small compiler TUs and startup-only executable checks support no speed claim.','',
'| Input | Compiler A → B ms | RSS A → B KiB | Compiler paired B/A [range] | Runtime A → B ms | Runtime paired B/A [range] | Text A → B |',
'|---|---:|---:|---:|---:|---:|---:|']
def ratios(d):
 r=d['paired_b_over_a'];return '%.3f [%.3f–%.3f]'%(statistics.median(r),min(r),max(r))
for name,w in x['workloads'].items():
 if w['final_only']:continue
 c,r=w['compiler'],w['runtime'];a,b=w['outputs']
 lines.append('| %s | %.2f → %.2f | %d → %d | %s | %.2f → %.2f | %s | %d → %d |'%(name,c['0']['median_wall_s']*1000,c['1']['median_wall_s']*1000,c['0']['peak_rss_kib'],c['1']['peak_rss_kib'],ratios(c),r['0']['median_wall_s']*1000,r['1']['median_wall_s']*1000,ratios(r),a['text_bytes'],b['text_bytes']))
lines+=['','## Noise and correctness costs','']
for name in ('auto-specializations-9600','runtime-member','runtime-virtual-single','runtime-nonpoly-single'):
 w=x['workloads'][name];dim='compiler' if name.startswith('auto-') else 'runtime';d=w[dim]
 lines.append('- **%s, %s:** A/A %.2f–%.2f ms; A range %.2f–%.2f ms; B range %.2f–%.2f ms. Paired ratios: %s.'%(name,dim,*[t*1000 for t in d['aa_range_s']],*[t*1000 for t in d['0']['range_wall_s']],*[t*1000 for t in d['1']['range_wall_s']],', '.join('%.3f'%v for v in d['paired_b_over_a'])))
lines+=['',
'Common workloads retain byte-identical native text. The member-call timing varies despite identical code; paired ranges and the preserved series disclose this execution noise instead of attributing it to code changes. The template workload likewise supports no frontend speedup claim. Every observation is retained.', '',
('The affected standalone virtual-dispatch workload grows **550→570 text bytes** and its median runtime changes **%.2f→%.2f ms**. It now recovers a virtual-base receiver through the object table, including when its most-derived layout differs. The nonpolymorphic virtual-base access workload grows **372→434 bytes**, with median runtime **%.2f→%.2f ms**. It adds the layout pointer/initialization and dynamic field projection needed for correct references. Entry and final execute the same checked standalone programs; the entry representation cannot handle the separate shared-base reducer. These are required O0 semantic/contract costs, not optional optimizations. No inlining or devirtualization transform is proposed or accepted on these results.'%tuple(x['workloads'][n]['runtime'][str(i)]['median_wall_s']*1000 for n in ('runtime-virtual-single','runtime-nonpoly-single') for i in (0,1))), '',

'Compiler text grows **%s→%s bytes** (+%s, %.2f%%). Across comparable inputs, the largest peak-RSS increase is **%d KiB**. The new facts add compact virtual anchors, interned edge paths, layout extents and segment fields. Containment is completion-local and reused across virtual signatures.'%(format(x['binaries'][0]['text_bytes'],','),format(x['binaries'][1]['text_bytes'],','),format(x['binaries'][1]['text_bytes']-x['binaries'][0]['text_bytes'],','),100*(x['binaries'][1]['text_bytes']/x['binaries'][0]['text_bytes']-1),max(w['compiler']['1']['peak_rss_kib']-w['compiler']['0']['peak_rss_kib'] for w in x['workloads'].values() if not w['final_only'])), '',
'## New-capability work and growth','',
'Virtual-declaration scaling uses only the correct final compiler: the entry does not resolve the shared final-overrider graph correctly, so it is not an A/B profit baseline. Main executes successfully; its startup-dominated runtime is only an emission check.','',
'| Independent diamonds | Compiler ms | RSS KiB | Runtime check ms | Native text | Slot work | Overrider work | LowIR instructions |','|---:|---:|---:|---:|---:|---:|---:|---:|']
for name,w in x['workloads'].items():
 if not w['final_only']:continue
 stats={k:v for d in w['outputs'][0]['telemetry'] for k,v in d.items()}
 lines.append('| %s | %.2f | %d | %.2f | %d | %d | %d | %d |'%(name.rsplit('-',1)[1],w['compiler']['0']['median_wall_s']*1000,w['compiler']['0']['peak_rss_kib'],w['runtime']['0']['median_wall_s']*1000,w['outputs'][0]['text_bytes'],stats['semantic_virtual_slot_work'],stats['semantic_final_overrider_work'],stats['instructions']))
lines+=['',
'64→256→1024 diamonds produce 192→768→3072 virtual-base entries, 128→512→2048 subobject identities and 6,144→24,576→98,304 bytes in the new identity/base arenas. Class/view storage is 112,640→450,560→1,802,240 bytes. Work and retained facts follow the demanded graph and emitted output. The containment cache reduces recorded overrider work from 832/3,328/13,312 in the preserved series to 704/2,816/11,264; no wall-time benefit is claimed from those separate series.', '',
'Budgets are structural at PA23/O0: one complete-object placement per virtual base, one recorded projection per canonical base path, bounded primary slots per required signature/ABI entry, segment-local rows, two candidate scans, one publication scan and cached receiver-path checks, and at most linear completion-local cache entries. Table construction is linear in emitted rows; dynamic projections and result thunks add a bounded number of operations per consumed fact. There is no optional optimizer, fixed-point pass, speculative cloning or code-growth transform. New implementation does not invalidate unrelated caches.', '',
'Spec §9’s stage-scoped acceptance applies. Historical +15% latency, +16 MiB RSS and 5.5× scaling diagnostics remain self-selected diagnostics, not additional gates. Their prior measurements remain preserved in performance 121/122. Correctness, coverage, mandated complexity/growth limits and evidence remain required. Native selection/allocation/debug and self-host performance belong to their later stages. This handoff accepts necessary bounded semantic costs and claims no optimization profit.']
repeat=json.loads((ROOT/'student.tests/pa23/performance123-repeat.json').read_text())
w=repeat['workloads']['member-functions-2048'];c=w['compiler']
lines+=['', '## Focused latency repeat', '',
'The sealed main series showed a 1.101 median paired compiler ratio on `member-functions-2048` (all four blocks above 1), so the same frozen binaries and input were repeated for eight ABBA blocks. [Every repeat observation](../student.tests/pa23/performance123-repeat.json) is retained. Compiler medians are %.2f→%.2f ms, peak RSS %d→%d KiB, A/A %.2f–%.2f ms; paired ratios are %s. The paired median is %.3f. The repeat does not establish a repeatable regression or speedup; the initial slower observations remain visible.'%(c['0']['median_wall_s']*1000,c['1']['median_wall_s']*1000,c['0']['peak_rss_kib'],c['1']['peak_rss_kib'],*[v*1000 for v in c['aa_range_s']],', '.join('%.3f'%v for v in c['paired_b_over_a']),statistics.median(c['paired_b_over_a']))]
(ROOT/'pa23/performance123.md').write_text('\n'.join(lines)+'\n')
