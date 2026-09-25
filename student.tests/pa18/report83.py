#!/usr/bin/env python3
"""Render all observations without dropping rows: PERFORMANCE OUTPUT."""
from pathlib import Path
import json,sys
p=json.loads(Path(sys.argv[1]).read_text());out=Path(sys.argv[2]);w=p['workloads'];a,b=p['binaries']
def ms(x):return f'{x*1000:.2f}'
def ratio(m):
 if 'paired_b_over_a' not in m:return 'final only'
 r=m['paired_b_over_a'];return f"{m['median_b_over_a']:.3f} ({min(r):.3f}–{max(r):.3f})"
def row(name,item,phase):
 m=item[phase];a=m.get('0');b=m['1']
 times=(ms(a['median_wall_s']) if a else '—')+' / '+ms(b['median_wall_s'])
 if phase=='compiler':size=(str(a['peak_rss_kib']) if a else '—')+' / '+str(b['peak_rss_kib'])
 else:
  sizes={o['binary']:o['native']['payload_bytes'] for o in item['outputs']};size=str(sizes.get(0,'—'))+' / '+str(sizes[1])
 return f'| {name} | {times} | {ratio(m)} | {size} |'
text=f'''# PA18 implementation 83 performance evidence

Acceptance is **PA18/O0 LowIR**, spec §9. Frozen entry `{p['commits'][0][:8]}`
and final `{p['commits'][1][:8]}` binaries, source text/hashes, flags, backend
hash, telemetry, warmups and every sample are in
[the raw observations](../student.tests/pa18/loop83-performance.json).
[The harness](../student.tests/pa18/benchmark83.py) pins one CPU, records four
A/A observations and four ABBA blocks, and checks each executable result.
Incorrect/rejected baseline cases receive six final-only observations; they are
never treated as faster correct implementations. Compilation, supplied-backend
construction and executable timing are separate. No optional optimizer is added.

Compiler flags are `--emit-lowir -O0`; separate preflights add `--stats
--validate-lowir`. Build flags are `g++ -std=gnu++11 -Wall -O3`. The backend is
bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, invoked only by the harness.
The runtime sources retain volatile bounds and checked data-dependent loop,
call, memory and floating work. The array loops perform 12 million calls,
initialize distinct mutable arrays and select/mutate elements from runtime input.
Native optimization, own object emission and self-hosting remain later stages.

## Compiler latency and peak RSS

Times are median milliseconds; ratios are paired B/A medians and complete ranges.
RSS is maximum observed KiB. All noise/outlier samples remain in the JSON.

| Workload | A / B ms | Paired ratio (range) | A / B peak KiB |
|---|---:|---:|---:|
'''
text+='\n'.join(row(n,i,'compiler') for n,i in w.items())+'\n'
s=p['startup'];text+=f"\nStartup medians are **{ms(s['0']['median_wall_s'])} / {ms(s['1']['median_wall_s'])} ms**.\nSmall compiler inputs are startup-limited diagnostics, not speedup evidence.\n"
text+='''
## Executable runtime and size

Times are median milliseconds. The supplied native executables have no section
headers, so payload bytes (including data) are a size proxy, not a claim to have
isolated executable `.text`. The compiler sizes below use actual ELF `.text`.

| Workload | A / B ms | Paired ratio (range) | A / B payload bytes |
|---|---:|---:|---:|
'''
text+='\n'.join(row(n,i,'runtime') for n,i in w.items() if 'runtime' in i)+'\n'
text+=f"\nCompiler `.text`: **{a['text_bytes']:,} → {b['text_bytes']:,} bytes**, "
text+=f"**{b['text_bytes']-a['text_bytes']:+,} ({(b['text_bytes']/a['text_bytes']-1)*100:+.3f}%)**.\n"
text+='''
## Work bounds and acceptance

| Source family, 600 → 2400 | Initializer actions | Constant-array checks | Query work | LowIR instructions | Median time scaling |
|---|---:|---:|---:|---:|---:|
'''
for family in ('wide-array','converted-array','deduced-rows','pack-bound'):
 small=w[family+'-600'];large=w[family+'-2400']
 def stats(item):return {k:v for t in item['outputs'][-1]['telemetry'] for k,v in t.items()}
 x,y=stats(small),stats(large)
 nums=[f"{x[k]} → {y[k]}" for k in ('semantic_initializer_actions','semantic_constant_array_work','semantic_type_query_work','instructions')]
 scaling=large['compiler']['1']['median_wall_s']/small['compiler']['1']['median_wall_s']
 text+=f"| {family} | "+' | '.join(nums)+f' | {scaling:.2f}× |\n'
text+='''
Bound inference reuses the checked plan, walks explicit element actions once,
and rejects a non-consuming clause. Source shape checks retain fixed conversions
and defer expansion counts; query substitution reads the completed entity by
identity. There is no token replay, expanded-bound walk, global invalidation or
whole-program candidate retry. Initializer plans, source facts and query caches
are TU owned; per-function lowering uses the existing typed action/data path.
The existing eight-lane omitted-element expansion limit and compact zero-data
fallback remain unchanged. Explicit emitted data costs O(output bytes).

The removed small-wide and construction exclusions were contrary to the inherited
PA16 copy requirement. Their cost/size comparison is disclosed as a required
contract repair; it does not justify an optional optimization or relax correctness.
The constant plan checker already proves the initializer before emitting its
image. Removal also eliminates the additional initializer syntax scan. Volatile,
effectful and runtime-address cases retain ordinary execution.

PA18/O0 has no mandated numerical compiler latency/RSS ceiling. Historical +15%,
+16 MiB and 5.5× diagnostic targets remain non-gates; prior measurements are
preserved. Required correctness, coverage, bounded work, and profitability of
optional transforms remain requirements. Existing named-result summary limits
and their previously measured benefit are unchanged; their inspection controls
pass. No new optional transformation needs a growth or profitability allowance.
'''
text+='''
The unchanged ordering and common compiler workloads retain exact LowIR; call,
memory and floating probes retain identical native bytes. Their paired runtime
ratios are 1.001, 0.996 and 1.001. No speedup or executable regression is inferred
from those noise-scale differences. Common compiler RSS rises by at most 216 KiB.

At 2400 functions, required wide-array materialization costs **201.20 → 208.87 ms**
(paired **1.037**), while converted arrays cost **322.06 → 341.18 ms** (paired
**1.077**). Wide-array RSS falls **38212 → 37448 KiB**; converted-array RSS rises
**54272 → 55536 KiB**, **1264 KiB**. The old wide-scalar shortcut skipped constant
proof/image work completely. Final constant-array work is 7200 actions for both
families, exactly four times the 600-function workload. The additional work
establishes required images; no secondary full initializer scan remains.

The wide-array runtime cost is **80.84 → 86.05 ms**, paired **1.065** in all four
blocks, with **284 → 320 bytes** of payload. Converted-array runtime is
**85.06 → 85.82 ms**, paired **1.008**, payload **314 → 320 bytes**. These are
observed regressions, not gains. The wide-array LowIR differs by replacing four
immediate stores with the required 32-byte readonly image and one 32-byte copy;
the following mutation, indexing, calls and loop remain the same. The image
accounts for most of the 36-byte payload growth. This is necessary representation
work under PA16's inherited rule, performed by the supplied backend, not a new
optional transform that could be removed. Improving native implementation of
that required copy belongs to the later backend stage. No filename/type-width
exception or coverage reduction is used to recover the old numbers.

Four bound/query workloads are final-only: the entry compiler either computes
an incorrect deduced extent or rejects a valid pack-dependent query. Their costs
are measured without treating the wrong result as a fast baseline. All 15
workloads with a main function pass native preflights; the common compiler corpus
retains exact LowIR. On these observations, explicit work bounds and unchanged
fallback semantics, this behavior group meets the stage-scoped acceptance rule.
'''
out.write_text(text)
