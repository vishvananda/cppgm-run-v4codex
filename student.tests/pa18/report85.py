#!/usr/bin/env python3
"""Render all frozen PA18 demand/discard cost observations."""
from pathlib import Path
import json,statistics
R=Path(__file__).resolve().parents[2];p=json.loads((R/'student.tests/pa18/loop85-performance.json').read_text());assert p.get('finished_utc')
lines=['# PA18 implementation 85 performance evidence','',
 'Acceptance is **PA18/O0 LowIR**, spec §9. [The harness](../student.tests/pa18/benchmark85.py) freezes binary hashes, commits, flags, inputs, CPU affinity, telemetry and every observation in [the evidence](../student.tests/pa18/loop85-performance.json). Entry is `f953e42a`; final implementation is `076eccdd`. Correctness validation finished before timing began.','',
 'The [initial observations](../student.tests/pa18/loop85-performance-initial.json) are preserved. They exposed 151/601 copy selections and zero source-recipe reuse for 150/600 fixed uses. The final implementation retains the checked source recipe for casts, statements, commas, parentheses and conditionals; the scaling assertions below require one selection and one materialization per concrete use. Both complete runs use the same frozen entry binary, inputs, flags and harness. This repairs a spec defect rather than reclassifying repeated work as an acceptable timing cost.','',
 'Comparable cases use one warmup per binary, four A/A calibration samples and four ABBA blocks. Incorrect baseline behavior is preserved in the record and receives six final-only samples. Compilation and executable timing are separate; `/usr/bin/time` records peak RSS, CPU time and scheduling counters. Runtime bounds are volatile, and results are checked before timing and on every execution. The scalar discard loop performs 24 million calls and the class discard loop performs 12 million copies/destructions.','',
 'Build flags: `g++ -std=gnu++11 -Wall -O3`, with the course test runner. Compiler flags: `--emit-lowir -O0`. Separate preflights add `--stats --validate-lowir`. The explicit harness uses `lowir2native-ref -O0`, pinned bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, to build checked executables. This does not implement production compiler behavior. Native backend optimization and self-hosting are later-stage requirements.','',
 '## Compiler latency, peak RSS and output size','',
 'Times are median milliseconds. Ratios are medians of paired block means with their complete ranges. RSS is maximum observed KiB. The supplied backend emits sectionless ELF, so native size is its loadable payload proxy (code **and static data**), not isolated `.text`. Compiler `.text` is measured directly. Startup-sized rows are diagnostics. All A/A and warmup observations remain in the JSON.','',
 '| Workload | A / B ms | Paired B/A (range) | A / B KiB | A / B native payload bytes |','|---|---:|---:|---:|---:|']
def values(m,k,f=lambda x:str(x)):
 return ' / '.join(f(m[str(i)][k]) if str(i) in m else '—' for i in (0,1))
def ratio(m):
 a=m.get('paired_b_over_a',[])
 return f'{statistics.median(a):.3f} ({min(a):.3f}–{max(a):.3f})' if a else 'final only'
def sizes(v):
 d={r['binary']:r for r in v['outputs']}
 return ' / '.join(str(d[i]['native']['payload_bytes']) if i in d and 'native' in d[i] else '—' for i in (0,1))
for name,v in p['workloads'].items():
 m=v['compiler'];lines.append(f"| {name} | {values(m,'median_wall_s',lambda x:f'{1000*x:.2f}')} | {ratio(m)} | {values(m,'peak_rss_kib')} | {sizes(v)} |")
lines+=['',f"Startup A/B medians: **{values(p['startup'],'median_wall_s',lambda x:f'{x*1000:.2f}')} ms**.",'','## Executable runtime','',
 '| Workload | A / B ms | Paired B/A (range) | A / B payload bytes |','|---|---:|---:|---:|']
for name,v in p['workloads'].items():
 if 'runtime' in v:
  m=v['runtime'];lines.append(f"| {name} | {values(m,'median_wall_s',lambda x:f'{1000*x:.2f}')} | {ratio(m)} | {sizes(v)} |")
a,b=[r['text_bytes'] for r in p['binaries']]
lines+=['',f'Compiler `.text`: **{a:,} → {b:,} bytes**, **{b-a:+,} ({(b/a-1)*100:+.3f}%)**.','',
 '## Required work and bounded costs','',
 'Discard source-form classification consumes only the selected expression and its immediate child facts: O(1) work per completed expression/query. The bit occupies existing packed flag storage; host record-layout inspection confirms `Expression` stays **36 bytes**. Lowering no longer allocates a second NodeId-sized syntax-classification cache. The sparse volatile-class conversion index belongs to the translation unit; complete contextual NodeId identity separates checked recipes from concrete temporary/lifetime records. Function-local lowering temporaries retain the existing release boundary.','',
 '| Class template scale | Copy selections | Shared recipe uses | Materializations | Expression work | LowIR instructions |','|---|---:|---:|---:|---:|---:|']
for name in ('discard-class-150','discard-class-600'):
 output=next(x for x in p['workloads'][name]['outputs'] if x['binary']==1)
 stats={k:v for d in output['telemetry'] for k,v in d.items()}
 keys=('semantic_discard_selections','semantic_discard_recipe_uses','semantic_discard_materializations','semantic_expression_work','instructions')
 lines.append('| '+name+' | '+' | '.join(str(stats.get(k,'not emitted')) for k in keys)+' |')
lines+=['',
 'The class-discard baselines omit required copy/destructor effects and fail the native preflight. Their final costs are therefore necessary semantic work, not an optimization regression or speedup comparison. Invalid/deleted/inaccessible copies fail through compact query results; actual evaluated uses demand only the selected constructor/destructor and materialize once. Functional void queries consume the same conversion rules. No grammar replay, global retry, optional optimization pass, code cloning or growth allowance was added. Existing initializer and named-result limits are unchanged.','',
 'The comparable ordering, common loop/float, calls, memory, floating, scalar-discard and dormant-storage cases retain exact LowIR (and exact native bytes where emitted). Their runtime differences cannot establish generated-code improvement. Compiler ratios, A/A spread, CPU time and RSS remain visible for assessing the added required frontend work. No performance speedup is claimed.','',
 'PA18/O0 has no mandated numeric latency/RSS ceiling. Historical +15%, +16 MiB and 5.5× targets remain diagnostics, with old measurements preserved. Correctness, coverage and bounded work remain requirements. There is no new optional transform needing a profitability allowance; required volatile accesses and class copies cannot be removed to improve timing. Later native/runtime implementation constraints are not new PA18 exit gates.','']
(R/'pa18/performance85.md').write_text('\n'.join(lines))
