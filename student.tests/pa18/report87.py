#!/usr/bin/env python3
"""Render complete frozen measurement tables without discarding observations."""
from pathlib import Path
import json,statistics
ROOT=Path(__file__).resolve().parents[2]
d=json.loads((ROOT/'student.tests/pa18/loop87-performance-complete.json').read_text())
def samples(m):return len(m['observations'])+len(m['warmups'])
count=samples(d['startup'])+sum(samples(w[k]) for w in d['workloads'].values() for k in ('compiler','runtime') if k in w)
def number(v):return f'{v:.2f}'
def pair(m,key,scale=1):return ' / '.join(number(m[str(i)][key]*scale) if str(i) in m else '—' for i in (0,1))
def ratio(m):
 r=m.get('paired_b_over_a')
 return f'{statistics.median(r):.3f} ({min(r):.3f}–{max(r):.3f})' if r else 'final only'
def size(w):
 out={str(o['binary']):o.get('native',{}).get('payload_bytes') for o in w['outputs']}
 return ' / '.join(str(out[str(i)]) if out.get(str(i)) is not None else '—' for i in (0,1))
text=f'''# PA18 implementation 87 performance evidence

Acceptance: **PA18/O0 LowIR**, spec §9. Frozen entry `01b47c20` is compared with
final committed implementation `{d['commits'][1][:8]}`. [Raw measurements](../student.tests/pa18/loop87-performance-complete.json)
retain **{count} observations across {len(d['workloads'])} workloads**, all sources,
binary/input/backend hashes, flags, affinity, preflight telemetry, checked
execution results, warmups and timing observations. [The harness](../student.tests/pa18/benchmark87.py)
and [table renderer](../student.tests/pa18/report87.py) reproduce this record.
Historical evidence, including [audit 86](performance86.md), the first 438 observations in [the initial run](../student.tests/pa18/loop87-performance.json), and 438 observations in [the query/default repair run](../student.tests/pa18/loop87-performance-final.json), remains unchanged. This checkpoint retains **1,314 observations** in total.

Each comparable workload uses one warmup per binary, four A/A observations,
then four wall-time ABBA blocks. Rejecting or incorrect entry implementations
receive six final-only samples. Timing uses `/usr/bin/time` peak RSS and separate
compiler/executable invocations. Telemetry is collected during preflight only.
No throughput tests ran during this benchmark. The native harness boundary is
`lowir2native-ref -O0`, bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
Every native preflight and timed execution checks its result. Runtime loops use
volatile bounds and checked sums/copy/destruction counts. No optional optimizer
was added, and no runtime speedup is claimed.

Times are median milliseconds, paired ratios include the full block spread,
RSS is maximum KiB. Compiler `.text` is measured directly; the sectionless
reference ELF's **loadable payload** includes code/data and is not isolated
`.text`. Startup-sized compiler rows are diagnostics. Compiler startup medians:
**{pair(d['startup'],'median_wall_s',1000)} ms**.

| Workload | Compiler A / B ms | Paired B/A (range) | A / B KiB | A / B payload bytes |
|---|---:|---:|---:|---:|
'''
for name,w in d['workloads'].items():
 m=w['compiler'];text+=f"| {name} | {pair(m,'median_wall_s',1000)} | {ratio(m)} | {pair(m,'peak_rss_kib')} | {size(w)} |\n"
a,b=(x['text_bytes'] for x in d['binaries'])
text+=f'\nCompiler `.text`: **{a:,} → {b:,} bytes**, **+{b-a:,} ({100*(b-a)/a:.3f}%)**.\n\n'
text+='| Workload | Runtime A / B ms | Paired B/A (range) | A / B payload bytes |\n|---|---:|---:|---:|\n'
for name,w in d['workloads'].items():
 if 'runtime' in w:text+=f"| {name} | {pair(w['runtime'],'median_wall_s',1000)} | {ratio(w['runtime'])} | {size(w)} |\n"
text+='''
## Work, regressions and stage acceptance

All unaffected ordering, loop/floating, calls, memory, alias-result and prvalue
ellipsis workloads retain byte-identical LowIR and native bytes. The raw record
retains scheduler outliers and the full paired ranges. There is no supported
speedup claim from these noisy ratios. Their semantic work counters and output
sizes are unchanged; alias result classification introduces no extra work.

Fixed class ellipsis now makes the source-required private argument copy. At
600 specializations its compiler ratio, memory and payload growth appear in the
table above. This is the selected transfer/materialization cost, not
an optional optimization. Passing the source object's address happened to work
for the old benchmark's unused argument, but does not provide a general value
boundary. The final result is separately checked. Per-use storage and `copyobj`
growth follow actual arguments, without helper/body cloning. New lvalue and
nontrivial-copy workloads have no correct general entry comparison; the query
baseline computes a wrong `noexcept` value. Their rejection/wrong-result records
are retained and their final costs are reported rather than called speedups.

The affected executable timings are about 51–73 ms on checked multi-million-iteration
loops; unaffected call/memory/floating loops run 0.45–1.36 seconds. Final-only copy
timing measures both construction and destruction. Native try/catch, hosted
aggregate varargs retrieval, native optimization, debug encoding and self-hosting
remain later-stage obligations, not additional PA18 exit gates.

| Family, 150 → 600 | Final work evidence |
|---|---|
'''
for prefix in ('ellipsis-lvalue','ellipsis-fixed','ellipsis-query','alias-result'):
 rows=[]
 for n in (150,600):
  w=d['workloads'][prefix+'-'+str(n)];o=next(o for o in w['outputs'] if o['binary']==1)
  rows.append((o['telemetry'][0],o['telemetry'][1]))
 a,b=rows
 text+=f"| {prefix} | candidates {a[0]['semantic_candidate_work']} → {b[0]['semantic_candidate_work']}; conversion objects {a[0]['semantic_conversion_objects']} → {b[0]['semantic_conversion_objects']}; instructions {a[1]['instructions']} → {b[1]['instructions']} |\n"
text+='''
The fixed recipe keeps one selected call and adds a materialization per evaluated
use; the query shares its checked conversion recipe without a body demand.
Type, expression and substitution identities remain TU-owned and interned.
Existing constant-evaluation work/depth limits, bounded default and transfer
queries, memoized branch analysis and function-local emission remain unchanged.
No new pass, code cloning, global rescan or optional growth budget is introduced.

PA18 mandates no numeric compiler latency/RSS ceiling. Historical +15%, +16 MiB
and 5.5× targets remain diagnostic, as established by audit 86. This record
preserves necessary costs, all observations and correctness while showing work
proportional to consumed/emitted facts. There is no unprofitable optional
transform to remove. Stage-scoped performance acceptance is satisfied; the
independent full-stage audit remains required before advancement.
'''
(ROOT/'pa18/performance87.md').write_text(text)
print('Rendered',count,'observations')
