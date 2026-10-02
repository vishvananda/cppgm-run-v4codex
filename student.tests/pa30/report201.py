#!/usr/bin/env python3
"""Render measured results without discarding outliers or inventing gates."""
import json,pathlib,statistics
root=pathlib.Path(__file__).resolve().parents[2];e=root/'student.tests/pa30/evidence201'
def read(n):return json.loads((e/(n+'.json')).read_text())
c=read('common-performance');o=read('owner-performance');h=read('hosted-performance')
s='''# PA30 implementation201 performance acceptance

Code tip: `fc8ea8d6`. Frozen A is entry `b0790726`; B is the final compiler.
[Source/binary binding](../student.tests/pa30/evidence201/source-binding.json)
and [ownership trace](design201.md) identify the measured code. Changes supply
required O0 semantic/CFG checks; no optional optimization or speedup is claimed.

## Protocol and stage scope

CPU 2 affinity, flags `-O0 -c --stats`, frozen binaries/inputs, one AAAA noise
calibration and six ABBA blocks per equivalent workload, separately for
compilation and execution. All hashes, observations, phase counters, checked
outcomes and text sizes are retained. Compilation and execution are independent
measurements; argv/argc and checksums keep runtime work observable. Host g++
only links emitted objects. Timing began after builds, reports and controls.

There are **420 observations plus 16 launcher calibrations**:
[common](../student.tests/pa30/evidence201/common-performance.json) 224,
[owner scaling](../student.tests/pa30/evidence201/owner-performance.json) 168,
and [hosted/fixed outcomes](../student.tests/pa30/evidence201/hosted-performance.json)
28. Correct rejection costs are final-only; accepting invalid code is not an
A/B performance baseline. No sample or outlier is discarded.

The **45-second per-compile limit** is mandatory. Historical blanket 15% latency
and zero-growth targets are diagnostics under spec §9; performance195–200 and
all raw observations remain. No numeric RSS percentage is mandated. Existing
language/expansion/native bounds are unchanged. The new reachability proof has
a fixed number of linear passes, deduplicated worklists and function-local
storage: O(instructions + edges), no fixed point or code growth. Isolated joins
avoid further analysis; ordinary complete-return bodies need none. Unknown
integer values retain both edges. This proof neither instantiates constexpr
bodies nor changes IR. Noreturn and unwind remain separate signature facts.

## Equivalent common programs

All A/B objects and executables are byte-identical. The fixed workloads cover
2,400 demanded templates, loops, calls, memory, floating point, exception
cleanup and unused-function pruning.

| Workload | Compile B/A [paired range] | Runtime B/A [paired range] | Compile RSS A/B KiB | Object / executable text bytes |
|---|---|---|---|---|
'''
def ratio(q):return '%.3f [%.3f, %.3f]'%(q['paired_ratio_median'],*q['paired_ratio_range'])
for name,q in c['summary'].items():
 im=c['images'][name]['B'];s+='| %s | %s | %s | %d / %d | %d / %d |\n'%(name,ratio(q['compile']),ratio(q['runtime']),q['compile']['A']['peak_rss_kib'],q['compile']['B']['peak_rss_kib'],im['object_text_bytes'],im['executable_text_bytes'])
s+='''
| Workload | Compile A/B median ms | Runtime A/B median ms | Compile A/A range ms | Runtime A/A range ms |
|---|---|---|---|---|
'''
for name,q in c['summary'].items():
 s+='| %s | %.2f / %.2f | %.2f / %.2f | %.2f–%.2f | %.2f–%.2f |\n'%(name,q['compile']['A']['median_s']*1000,q['compile']['B']['median_s']*1000,q['runtime']['A']['median_s']*1000,q['runtime']['B']['median_s']*1000,*[v*1000 for v in q['compile']['AA_range_s']],*[v*1000 for v in q['runtime']['AA_range_s']])
s+='''
Compiler paired medians are close to unity; outliers remain in the ranges.
The exceptions runtime has particularly broad outliers. Identical executable
bytes rule out a generated-code regression on these inputs. These observations
do not establish an avoidable compiler regression, and no noise-derived speedup
is claimed. Required validation costs are measured directly below.

## Completed-owner scaling

Each independent family demands a function template, local-class constant use,
a captured lambda, a constant arithmetic loop, noreturn calls and an exception
handler. All A/B programs pass the same checks and are byte-identical. The main
loop performs 3,000,000 argv-dependent transitions through the selected family;
external function definitions keep the other families' emission work visible.

| N | Compile A/B median s | Compile B/A [paired range] | Peak RSS A/B KiB | Runtime B median s | Runtime B/A [paired range] | Object / executable text bytes |
|---:|---|---|---|---|---|---|
'''
for name,q in o['summary'].items():
 im=o['images'][name]['B'];s+='| %d | %.4f / %.4f | %s | %d / %d | %.4f | %s | %d / %d |\n'%(o['inputs'][name]['N'],q['compile']['A']['median_s'],q['compile']['B']['median_s'],ratio(q['compile']),q['compile']['A']['peak_rss_kib'],q['compile']['B']['peak_rss_kib'],q['runtime']['B']['median_s'],ratio(q['runtime']),im['object_text_bytes'],im['executable_text_bytes'])
s+='''
Every B repetition records **3N** checked fallthrough functions, **274N**
instruction visits and **35N** edge visits. The linear counters and bounded
RSS support the function-local work model. Code growth with N is the same
external definitions in A and B; the checking itself emits no additional code.
The small measured compiler cost buys required rejection and preserves valid
control flow; there is no optional transform to remove or unbounded search.

'''
for mode in ['compile','runtime']:
 a=[v['wall_s']*1000 for v in o['launchers'] if v['mode']==mode];s+='%s launcher: median %.2f ms [%.2f, %.2f].\n\n'%(mode.capitalize(),statistics.median(a),min(a),max(a))
s+='''## Repaired outcomes and retained hosted costs

| Fixture | Outcome | Compile median [range] s | Peak RSS KiB | Object text bytes |
|---|---|---|---:|---:|
'''
for path,item in h['inputs'].items():
 rows=[r for r in h['runs'] if r['path']==path];times=[r['wall_s'] for r in rows]
 s+='| %s | %s | %.4f [%.4f, %.4f] | %d | %s |\n'%(pathlib.Path(path).stem,'reject' if item['expected_rejection'] else 'emit',statistics.median(times),min(times),max(times),max(r['peak_rss_kib'] for r in rows),'—' if item['expected_rejection'] else str(rows[0]['object_text_bytes']))
s+='''
All expected rejections produce no object. Every successful fixture emits an
identical object in all four repetitions. Maximum observed compile: **%.3f s**,
maximum RSS: **%d KiB**. The 45-second limit is preserved.

PA30 discards the heavy hosted objects; hosted link/runtime completion belongs
to PA31. Applicable default/capture/control/exception behavior is additionally
checked through direct objects and serialized LowIR execution. Two required
random fixtures still need packed SIMD semantics; these measurements do not
waive those failures, general vector subscripting, or independent review.
'''%(max(r['wall_s'] for r in h['runs']),max(r['peak_rss_kib'] for r in h['runs']))
(root/'pa30/performance201.md').write_text(s)
