#!/usr/bin/env python3
"""Render measured PA30 implementation costs without inferring speedups."""
import json,pathlib,statistics
root=pathlib.Path(__file__).resolve().parents[2];e=root/'student.tests/pa30/evidence199'
def read(name):return json.loads((e/(name+'.json')).read_text())
def spread(v):return '%.4f [%.4f, %.4f]'%(v['median_s'],*v['range_s'])
def ratio(v):return '%.3f [%.3f, %.3f]'%(v['paired_ratio_median'],*v['paired_ratio_range'])
c=read('common-performance');o=read('owner-performance');h=read('hosted-performance');v=read('vector-performance')
parts=['''# PA30 implementation199 performance acceptance

Code tip: `d6408429`. Frozen A is entry `d9a62584`; B is the final
implementation binary. [Source/binary binding](../student.tests/pa30/evidence199/source-binding.json)
and [design trace](design199.md) identify the measured implementation.
These are necessary semantic/emission repairs at O0. No optional optimization
or speedup is claimed.

## Protocol and budgets

Flags are `-O0 -c --stats`, affinity CPU 2. Fixed inputs, compiler hashes,
all wall times, peak RSS, phase counters, object/executable hashes and text
sizes are retained. Common comparisons use one AAAA calibration and six ABBA
blocks for compilation and execution separately. Every executable run checks
its result from runtime argc/argv inputs; host g++ only links compiler-emitted
objects. Tests/builds finish before the final measurement sequence.

Final evidence contains **312 observations plus 16 launch calibrations**:

- [Common](../student.tests/pa30/evidence199/common-performance.json): 224, with
  templates, loops, calls, memory, floating point, exceptions and unused emission.
- [Allocation/access scaling](../student.tests/pa30/evidence199/owner-performance.json):
  48 plus 16 launchers, at N=64,256,1024 independent declaration families.
- [Repaired hosted fixtures](../student.tests/pa30/evidence199/hosted-performance.json):
  24, four compiles of each of the six newly passing course inputs.
- [Vector runtime](../student.tests/pa30/evidence199/vector-performance.json):
  16 compile/runtime samples covering initialization, extraction, representation
  conversion, volatile whole-vector reads and assignment.

The last three workloads have no valid entry timing baseline: A rejects the
new semantics. They report final absolute costs, not a comparison to failed
compilation. The small vector source's compile time is startup-dominated;
compiler scaling uses the larger owner/common inputs instead. Runtime loops
perform 3,000,000 checked transitions and dominate process startup.

The mandated **45-second per-compile limit** remains unchanged. Inherited
blanket 15% latency/zero-growth diagnostics are not extra gates under spec §9;
performance195–198 and all observations remain preserved. No numeric RSS
percentage is mandated by PA30. Existing evaluator, native, inline and
per-function reclamation bounds are unchanged. Required initialization stores,
volatile reads and representation copies are measured correctness costs; no
optional transform is retained without profit. Fixed builtins emit at most
eight lane initializations; larger existing vector operations use bounded
loop lowering; static zero vectors use one byte-span data item. These repairs
add no unbounded search or optional code expansion.

## Equivalent common workloads

All A/B object and executable pairs are byte-identical, including text. Paired
B/A medians and complete six-block ranges are below; noise observations are
retained rather than discarded. No generated-work reduction is inferred.

| Workload | Compile B/A [range] | Runtime B/A [range] | Compile RSS A/B KiB | Object text bytes |
|---|---|---|---|---:|''']
for name,s in c['summary'].items():
 a=s['compile'];t=s['runtime'];parts.append('| %s | %s | %s | %d / %d | %d |'%(name,ratio(a),ratio(t),a['A']['peak_rss_kib'],a['B']['peak_rss_kib'],c['images'][name]['B']['object_text_bytes']))
parts.append('\n| Workload | Compile A/B median ms | Runtime A/B median ms | Compile A/A range ms | Runtime A/A range ms |\n|---|---|---|---|---|')
for name,s in c['summary'].items():
 a=s['compile'];t=s['runtime'];parts.append('| %s | %.2f / %.2f | %.2f / %.2f | %.2f–%.2f | %.2f–%.2f |'%(name,1000*a['A']['median_s'],1000*a['B']['median_s'],1000*t['A']['median_s'],1000*t['B']['median_s'],*[1000*x for x in a['AA_range_s']],*[1000*x for x in t['AA_range_s']]))
parts.append('''
## Corrected owner scaling

Each family contains dependent new-array declarations, current-instantiation
friendship, fixed-base signature aliases and one demanded nested class. The
emitted helper consumes those facts and the runtime allocation/access loop
checks an independently computed final state and checksum.

| N | Compile median [range] s | Peak RSS KiB | Runtime median [range] s | Object / executable text bytes |
|---:|---|---:|---|---|''')
for name,s in o['summary'].items():
 a=s['compile'];t=s['runtime'];i=o['images'][name];parts.append('| %d | %s | %d | %s | %d / %d |'%(o['inputs'][name]['N'],spread(a),a['peak_rss_kib'],spread(t),i['object_text_bytes'],i['executable_text_bytes']))
parts.append('''
Every repetition records lookup work `179N+88`, type-query work `12N+3`,
class completions `N`, delimiter work `311N+170` and maximum lookahead 47.
All output hashes match across repetitions and N: growing type-only families
adds no emitted text. Indexed facts and language-required scope/base edges
account for the linear work, without a new global cache or retry policy.
''')
for mode in ['compile','runtime']:
 values=[r['wall_s'] for r in o['launchers'] if r['mode']==mode]
 parts.append('%s launch calibration: median %.2f ms [%.2f, %.2f].'%(mode.capitalize(),statistics.median(values)*1000,min(values)*1000,max(values)*1000))
parts.append('''
## Vector and hosted costs

The vector runtime uses an argv-dependent seed, vector construction, two
representation casts and two full volatile operand reads each iteration. Its
checksum/final state are computed independently. The early benchmark exposed
invalid zero-vector static data; that defect is fixed in the measured B.
The reduced zero-initialization and volatile controls also pass object and
external LowIR roundtrip execution.
''')
parts.append('Vector compile median [range]: **%s s**, peak RSS **%d KiB**. Runtime: **%s s**. Object/executable text: **%d / %d bytes**. All repeated objects match.'%(spread(v['summary']['compile']),v['summary']['compile']['peak_rss_kib'],spread(v['summary']['runtime']),v['image']['object_text_bytes'],v['image']['executable_text_bytes']))
parts.append('\n| Repaired fixture | Compile median [range] s | Peak RSS KiB | Object text bytes |\n|---|---|---:|---:|')
for path in h['inputs']:
 rows=[r for r in h['runs'] if r['path']==path];times=[r['wall_s'] for r in rows]
 parts.append('| %s | %.4f [%.4f, %.4f] | %d | %d |'%(pathlib.Path(path).stem,statistics.median(times),min(times),max(times),max(r['peak_rss_kib'] for r in rows),rows[0]['object_text_bytes']))
parts.append('\nAll repaired hosted compiles succeeded with deterministic repeated objects. Maximum measured hosted time **%.3f s**, RSS **%d KiB**, below the unchanged timeout. PA30 discards these objects; hosted runtime completion remains PA31 work, while the applicable generated-code surfaces above have checked runtime/text measurements.'%(max(r['wall_s'] for r in h['runs']),max(r['peak_rss_kib'] for r in h['runs'])))
parts.append('''
## Preserved intermediate observations

Two earlier complete common/owner/hosted series (296 observations and 16
launchers each) are retained in `evidence199/preliminary-*-performance.json`
and `pre-zero-*-performance.json`. Their B hashes identify the earlier code;
neither is relabeled final. The first predates volatile snapshots; the second
predates zero-vector storage repair. The later correctness discoveries required
fresh final bindings and measurements. Some early samples overlapped follow-up
work and exhibit large outliers. The final sequence runs without concurrent
compiler builds or reports; all intermediate outliers are still preserved.

Common images are identical in both intermediate series too. Their compilation
paired medians span 0.983–1.017 and runtime medians 0.998–1.009. These observations
and A/A spreads do not establish a repeatable avoidable regression or an
optimization benefit. The current code adds constant-time dispatch checks and
required semantic work; no unrelated scans, optimizer expansions or growing
cache keys were added. The final raw data preserves any timing variance rather
than claiming zero overhead. Whole-stage acceptance remains incomplete because
15 required correctness fixtures still fail, independently of these measured
costs; the performance record does not waive them.
''')
(root/'pa30/performance199.md').write_text('\n'.join(parts))
