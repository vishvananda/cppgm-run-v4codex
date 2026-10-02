#!/usr/bin/env python3
"""Render all PA30 implementation200 observations, without speedup claims."""
import json,pathlib,statistics
root=pathlib.Path(__file__).resolve().parents[2];e=root/'student.tests/pa30/evidence200'
def read(n):return json.loads((e/(n+'.json')).read_text())
def span(x):return '%.4f [%.4f, %.4f]'%(x['median_s'],*x['range_s'])
def ratio(x):return '%.3f [%.3f, %.3f]'%(x['paired_ratio_median'],*x['paired_ratio_range'])
c=read('common-performance');o=read('owner-performance');h=read('hosted-performance')
parts=['''# PA30 implementation200 performance acceptance

Code tip: `37b6729d`. Frozen A is entry `377d92a0`; B is the final compiler.
[Source/binary binding](../student.tests/pa30/evidence200/source-binding.json)
and [ownership trace](design200.md) identify the measured code. These are
required semantic repairs at O0; no optional optimization or speedup is claimed.

## Protocol and stage scope

Flags: `-O0 -c --stats`, CPU 2 affinity. All compiler hashes, fixed input hashes,
wall times, RSS, phase counters, object/executable hashes and text sizes are
retained. Four common workloads use one AAAA calibration plus six ABBA blocks,
separately for compilation and execution. Runtime argc/argv and checked
checksums keep executable work observable. Host g++ only links emitted objects.
The final sequence began after builds, correctness reports and controls ended.

There are **312 observations plus 16 launch calibrations**:
[common](../student.tests/pa30/evidence200/common-performance.json) 224,
[owner scaling](../student.tests/pa30/evidence200/owner-performance.json) 48,
[repaired hosted fixtures](../student.tests/pa30/evidence200/hosted-performance.json)
40. Entry rejects the corrected owner/hosted workloads; those report absolute
final costs, never speedups against failed compilations. No sample is discarded.

The PA30 **45-second per-compile limit** remains mandatory. Historical blanket
15% latency/zero-growth diagnostic targets remain non-gates under spec §9;
performance195–199 and their raw observations are preserved. No numeric RSS
percentage is mandated. Existing generator (1,048,576 elements), evaluator,
inline and native expansion bounds are unchanged. New work is proportional to
selected subobject destruction edges and source bodies; query dependence uses
constant-time declaration flags. No optional code growth or optimization is
introduced. PA31 hosted runtime completion, PA32/33 optimization policies and
PA34 self-hosting remain later-stage work, not additional PA30 exit gates.

## Equivalent common programs

Every A/B object and executable is byte-identical. These fixed workloads include
2,400 demanded templates, loops, calls, memory, floating point, exception cleanup
and unused-function pruning. Runtime/text equivalence establishes that compiler
changes do not trade execution quality for lower compiler work on these inputs.

| Workload | Compile B/A [range] | Runtime B/A [range] | Compile RSS A/B KiB | Object / executable text bytes |
|---|---|---|---|---|''']
for name,s in c['summary'].items():
 a=s['compile'];t=s['runtime'];im=c['images'][name]['B']
 parts.append('| %s | %s | %s | %d / %d | %d / %d |'%(name,ratio(a),ratio(t),a['A']['peak_rss_kib'],a['B']['peak_rss_kib'],im['object_text_bytes'],im['executable_text_bytes']))
parts.append('\n| Workload | Compile A/B median ms | Runtime A/B median ms | Compile A/A range ms | Runtime A/A range ms |\n|---|---|---|---|---|')
for name,s in c['summary'].items():
 a=s['compile'];t=s['runtime'];parts.append('| %s | %.2f / %.2f | %.2f / %.2f | %.2f–%.2f | %.2f–%.2f |'%(name,1000*a['A']['median_s'],1000*a['B']['median_s'],1000*t['A']['median_s'],1000*t['B']['median_s'],*[1000*x for x in a['AA_range_s']],*[1000*x for x in t['AA_range_s']]))
parts.append('''
Compilation has isolated timing outliers, retained in the paired ranges. The
common paired medians and A/A observations do not establish a repeatable
avoidable regression. Generated code is identical, so runtime variation here
is measurement noise. Required new semantics are measured separately below;
no reduced-IR or blanket percentage claim is used to justify added work.

## Completed owner scaling

Each independent source family has an ordinary enclosing class, nested template
body, later-declared nested constructor and member alias sequence generator.
The body is checked in its enclosing complete-class context; a demanded
specialization uses both generated packs to select a delegating constructor.
The executable runs 3,000,000 argv-dependent checked transitions through that
constructor/call chain. Entry rejects the same input before code generation.

| N | Compile median [range] s | Peak RSS KiB | Runtime median [range] s | Object / executable text bytes |
|---:|---|---:|---|---|''')
for name,s in o['summary'].items():
 a=s['compile'];t=s['runtime'];im=o['images'][name]
 parts.append('| %d | %s | %d | %s | %d / %d |'%(o['inputs'][name]['N'],span(a),a['peak_rss_kib'],span(t),im['object_text_bytes'],im['executable_text_bytes']))
parts.append('''
Every repetition records lookup work `133N+49`, type-query work `11N+8`, class
completions `4N+1`, delimiter work `207N+97`, and maximum pending lookahead 132.
All output hashes match across repetitions and N. Increasing type-only families
does not emit additional code. This supports the indexed declaration/fact
ownership and linear work model without introducing broader invalidation.
''')
for mode in ['compile','runtime']:
 vals=[r['wall_s'] for r in o['launchers'] if r['mode']==mode]
 parts.append('%s launcher: median %.2f ms [%.2f, %.2f].'%(mode.capitalize(),statistics.median(vals)*1000,min(vals)*1000,max(vals)*1000))
parts.append('''
## Repaired hosted compile costs

| Fixture | Compile median [range] s | Peak RSS KiB | Object text bytes |
|---|---|---:|---:|''')
for path in h['inputs']:
 rows=[r for r in h['runs'] if r['path']==path];times=[r['wall_s'] for r in rows]
 parts.append('| %s | %.4f [%.4f, %.4f] | %d | %d |'%(pathlib.Path(path).stem,statistics.median(times),min(times),max(times),max(r['peak_rss_kib'] for r in rows),rows[0]['object_text_bytes']))
parts.append('\nMaximum hosted compile: **%.3f s**, **%d KiB** peak RSS; all four repetitions of each fixture emit identical objects. The 45-second limit is preserved.'%(max(r['wall_s'] for r in h['runs']),max(r['peak_rss_kib'] for r in h['runs'])))
parts.append('''
PA30 discards these hosted objects; their link/runtime completion is PA31 work.
Applicable cleanup/allocation/construction semantics are nevertheless checked
by direct object and serialized LowIR execution controls, including a throw
partway through aggregate union initialization. Whole-stage completion remains
blocked by five required fixture failures; performance evidence does not waive
them or the independent review of this implementation delta.
''')
(root/'pa30/performance200.md').write_text('\n'.join(parts))
