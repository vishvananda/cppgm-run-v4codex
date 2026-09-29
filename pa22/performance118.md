# PA22/O0 handoff 118 performance

The completed local member-storage proof removes known-zero receiver adjustment;
it never assumes that an arbitrary parameter or field declaration is unadjusted.
Qualified repeated-base field access is a required semantic correction, not an
optimization claim. Its previously rejected controls are excluded from A/B timing.

The [harness](../student.tests/pa22/benchmark118.py) freezes sources, binaries,
flags (`--emit-lowir -O0`), backend and hosted link command. Each lane warms up,
then four A/A observations and four ABBA blocks measure compilation and execution
separately. `/usr/bin/time` records peak RSS; every generated executable checks
its answer. Forty-million-iteration volatile bounds keep affected loops live.
CPU affinity is fixed to CPU 0. Course checks/builds did not overlap timing.
Small compiler inputs and trivial mains are startup-sensitive; they support no
standalone speed claim. Telemetry/validation is outside timing and preserves IR.

| Compiler | Commit | `.text` bytes | SHA-256 |
|---|---|---:|---|
| Entry A | `0c710f74` (audit117 code) | 2267526 | `b0c10c42b6532a3ff00072a1c3ec02c768af51944246c81ffd82ad3f2e365c13` |
| First B | `d649b4b5` | 2279238 | `947a254d3eb6a5650d3b1fdadfdb4a4c42d31516b4b87fd929e4b4112a007b09` |
| Final B | `e10bdd7f` | 2279302 | `2c4d610ea33c14d3dcd82a67cee412edb9f73ec558451c0e838b5b31ded8a968` |

Artifacts are `/tmp/pa22-118/{entry,final,sealed}`; `sealed` is final B.
Final compiler text grows **11776 bytes (0.519%)**. The second code change adds
an effect boundary for overloaded-arrow receivers. All ten measured programs
have byte-identical LowIR between first B and final B, and equal generated text
sizes. No incorrect baseline is used for the proof's benefit.

## Final campaign

Raw [common](../student.tests/pa22/performance118-common-sealed.json) and
[affected](../student.tests/pa22/performance118-fields-sealed.json) records retain
every warmup, observation, hash, work counter, A/A range and paired result.
Times below are median milliseconds. RSS is peak compiler KiB; text is hosted
executable `.text`, including unchanged startup. Paired columns give the full
range of four ABBA B/A ratios, not independent unpaired medians.

| Workload | Compile ms A/B | RSS KiB A/B | Runtime ms A/B | Native text A/B | Compile paired B/A | Runtime paired B/A |
|---|---:|---:|---:|---:|---:|---:|
| auto-specializations-9600 | 1827.89/2267.95 | 108040/108108 | 7.74/7.78 | 768286/768286 | 0.952–1.176 | 0.903–1.134 |
| runtime-calls | 10.38/10.45 | 6112/6352 | 198.30/199.51 | 430/430 | 0.930–1.004 | 0.996–1.008 |
| runtime-memory | 9.39/9.45 | 6112/6368 | 132.86/132.60 | 658/658 | 0.915–1.074 | 0.862–1.072 |
| runtime-floating | 8.84/9.01 | 6292/6448 | 148.39/145.51 | 454/454 | 0.935–1.164 | 0.934–1.079 |
| runtime-member | 10.53/10.69 | 6244/6376 | 406.51/393.36 | 488/488 | 0.975–1.087 | 0.951–1.200 |
| member-functions-2048 | 350.02/344.49 | 27880/27960 | 7.54/7.66 | 184632/184632 | 0.872–1.019 | 0.970–1.018 |
| runtime-field | 15.17/15.00 | 6144/6364 | 595.99/610.14 | 538/462 | 0.910–1.064 | 0.639–1.091 |
| runtime-nested-field | 19.59/18.51 | 6236/6392 | 489.94/344.35 | 538/462 | 0.905–0.981 | 0.682–0.828 |
| field-functions-512 | 103.34/98.13 | 12960/13016 | 6.49/6.53 | 84280/46392 | 0.913–1.010 | 0.930–1.064 |
| field-functions-2048 | 351.17/372.38 | 33272/33664 | 7.29/7.00 | 336184/184632 | 0.906–1.245 | 0.936–1.021 |

The six common inputs preserve byte-identical LowIR and native text, so their
runtime differences are noise, not benefit. The template-heavy compile median
rises **24.1%**, with paired ratios **0.952, 1.176, 1.098, 1.141**. This regression
is disclosed: A/B ranges overlap at **1394–2782 / 1414–2870 ms**, versus A/A
**1343–1526 ms**. All non-time/non-RSS work counters are identical on that input;
new flow work/read counters are both zero. The first campaign's same input was
**697.19/707.53 ms**, paired **0.973, 1.052, 1.007, 1.116**. The data supports no
compiler-speed claim, nor a stable 24% implementation cost. There is no added
analysis of this workload. Common-input RSS grows by at most **256 KiB** in the
final campaign. The large affected compile median rises **6.0%**; A/B ranges
are **304–775 / 296–673 ms**, with A/A **275–696 ms**. Its largest final RSS
increase is **392 KiB**. All samples, including outliers, remain recorded.

The final flat field loop has a **2.4% slower unpaired median**, despite three
improving paired blocks: **0.708, 0.891, 1.091, 0.639**. Its A/A runtime is
**766–1004 ms**, A runtime **501–985 ms**, B **336–737 ms**. The nested-field loop
improves **29.7%** at the median and every pair improves:
**0.828, 0.710, 0.682, 0.683**. A/A is **912–1064 ms**, A **463–915 ms**, B
**327–646 ms**. These scheduling swings are retained, not filtered.

## First campaign and profitability

The first [affected](../student.tests/pa22/performance118-fields.json) and
[common](../student.tests/pa22/performance118-common.json) campaigns are preserved.
The flat loop runs **311.16/216.37 ms**, paired **0.694, 0.690, 0.690, 0.694**;
A/B ranges are **309.61–318.98 / 213.54–218.68 ms**. The nested loop runs
**320.84/218.15 ms**, paired **0.700, 0.584, 0.692, 0.567**. Both remove **76 text
bytes (538→462)**. Final B produces exactly the same loop LowIR/native work, so
the first campaign remains direct evidence of its benefit. Across both campaigns
15 of 16 affected runtime pairs improve. The final flat-loop regression above
remains part of that evidence.

Initial compiler medians for the two loops are **6.55/6.32** and **6.55/6.40 ms**;
RSS **6144/6320** and **6164/6376 KiB**. At 512 functions compiler time is
**47.02/46.50 ms**, RSS **13208/13076 KiB**; at 2048, **174.99/177.07 ms**,
RSS **32972/34172 KiB**. The latter's paired compile ratios are
**1.001, 1.012, 1.040, 1.009**: a small measured cost, not a compiler gain.
Native text shrinks **84280→46392** and **336184→184632** bytes. Their trivial
mains only check behavior; no runtime benefit is claimed for those programs.

Disassembly of the final hosted flat loop confirms the eliminated work:
A extracts the high word via a shift sequence, stores/reloads a temporary and
adds the receiver displacement. B omits that sequence, retains the indirect
call and live loop, and reduces stack reservation **136→120 bytes**. This is
execution and machine-code evidence, not only a smaller IR count.

## Owners, legality and budgets

Semantic registration uses the current function ID and deduplicates requests.
Only completed requested bodies are traversed. Source occurrences own immutable
read proofs; direct projections use local entity identity plus canonical 64-bit
byte offset from recorded layout facts. This distinguishes repeated bases and
coalesces equivalent paths. No name-based recovery or hierarchy search occurs
in the proof or lowering consumer. Qualified-field semantics uses the existing
canonical base-path and receiver owners; constant evaluation follows that path.

The analysis visits at most **4096 nodes/function**, with depth at most **64**.
Each attempted address/value proof retains the inherited **64-node** limit;
thus the conservative pipeline upper bound is 64 value-proof visits per flow
visit, plus one bounded flow visit. Each location/fact is a flat-index entry;
function-local maps/vectors release on return. Persistent memory is proportional
to requested functions and proven source occurrences. Calls, unknown/alias writes,
overloaded arrows, control/lifetime boundaries and budget exhaustion invalidate
the current region; no unrelated TU cache is cleared. Unsupported forms retain
generic adjustment. There is no iterative analysis, callee/body demand or parse.

Measured visits are **6656/26624** and recorded field facts **512/2048** for
512/2048 functions: exactly linear. The live loops use **31/33** visits and one
fact each. Personal controls exercise both function-budget and depth fallback,
aliases, assignments, repeated/equivalent paths, reference/volatile/union storage,
loops, temporary destruction and overloaded-arrow effects. The latter's missing
call-receiver boundary was found, reduced and repaired before final measurement.

A proven member call removes three IR operations; there is **zero output-growth
budget** and no duplication. Unknown facts produce the existing constant-size
member operation. The inherited 64-node local proof and eight-element initializer
expansion limit remain unchanged. Profitability is supported by repeated checked
loop benefit, lower machine traffic/text and bounded compiler work/memory.

Spec §9 applies at PA22/O0. There is no mandated numeric latency/RSS limit here.
Inherited +15%, +16 MiB and 5.5× diagnostic targets stay non-gating; historical
measurements from 114–117 are preserved. This does not waive correctness, required
LowIR shape, comparisons or coverage. Native optimization/debug/self-host gates
belong to later stages. The four remaining PA22 fixture failures remain explicit
implementation obligations, separate from this completed ownership group and
its pending independent review.
