# PA22 final audit 120 performance

The two audit corrections establish physical empty-subobject identity and valid
access through every relevant nonvirtual base. They add necessary semantic
work, not an optional executable optimization. Incorrect entry reducers are
excluded from timed comparisons; all measured sources are correct in both lanes.

## Frozen protocol and identity

The [harness](../student.tests/pa22/benchmark120.py) freezes inputs, flags,
binaries and supplied object backend, fixes CPU affinity, checks results and
separately measures compilation and execution. Each workload has a warmup per
lane, four A/A observations and four ABBA blocks. `/usr/bin/time` records peak
compiler RSS. Telemetry and full LowIR validation are outside timing and must
preserve output bytes. Builds and test suites do not overlap these campaigns.
All warmups, observations, paired ratios, spread, sources and hashes remain in
[common](../student.tests/pa22/performance120-common.json) and
[affected](../student.tests/pa22/performance120-affected.json) evidence.

| Compiler | Commit | Text bytes | SHA-256 |
|---|---|---:|---|
| A entry | `17bc7a06` | 2281222 | `d2cac90074d5f8c4c9a73201e6b3bd1f6274b925146ec8879fe5491b7eda9b65` |
| B final | `2b1a02c9` | 2286342 | `f637045b13aad2a1dd7d4995e86d97eea558decb644780105486f1a25f0510bb` |

Compiler text increases **5120 bytes (0.224%)**. Frozen artifacts are under
`/tmp/pa22-120/{entry,final}`. Flags are `--emit-lowir -O0`, with the same
supplied object backend and `g++ -no-pie` link in both lanes. Runtime loops use
volatile iteration bounds, live calls/memory/floating operations and checked
results. Small compiler inputs and trivial mains do not support speed claims.
Template/self-host-shaped frontend specialization stress is included; compiling
this compiler's complete implementation with itself belongs to PA34.

## Measurements

Median times are milliseconds; RSS is peak compiler KiB. Paired ranges contain
all four ABBA B/A ratios. Text includes the same hosted startup in both lanes.

| Workload | Compile A/B | RSS A/B | Runtime A/B | Text A/B | Compile paired B/A | Runtime paired B/A |
|---|---:|---:|---:|---:|---:|---:|
| auto-specializations-9600 | 1352.70/1296.73 | 106664/106824 | 7.46/9.56 | 768286/768286 | 0.868–1.084 | 0.925–1.335 |
| runtime-calls | 20.46/20.08 | 6204/6320 | 203.63/204.32 | 430/430 | 0.957–1.014 | 0.972–1.076 |
| runtime-memory | 16.13/15.83 | 6168/6328 | 219.24/213.75 | 658/658 | 0.910–1.161 | 0.959–0.994 |
| runtime-floating | 14.92/14.83 | 6260/6388 | 163.08/167.57 | 454/454 | 0.965–1.048 | 0.975–1.072 |
| runtime-member | 15.99/16.76 | 6236/6324 | 360.82/339.79 | 488/488 | 0.986–1.299 | 0.901–0.965 |
| member-functions-2048 | 583.66/584.50 | 28068/28288 | 7.14/7.07 | 184632/184632 | 0.970–1.024 | 0.960–1.035 |
| empty-layouts-512 | 57.89/59.75 | 11340/11248 | 4.80/4.87 | 6419/6419 | 1.007–1.029 | 0.998–1.018 |
| access-functions-512 | 55.62/56.33 | 10892/11128 | 4.84/4.86 | 25903/25903 | 0.922–1.018 | 1.000–1.011 |
| empty-layouts-2048 | 211.67/215.10 | 27332/27064 | 4.71/4.71 | 24851/24851 | 1.000–1.026 | 0.990–1.041 |
| access-functions-2048 | 196.60/201.71 | 23680/25880 | 4.91/4.94 | 102703/102703 | 1.021–1.815 | 0.978–1.022 |
| runtime-inherited | 8.53/8.62 | 6216/6164 | 232.97/233.48 | 379/379 | 1.007–1.105 | 0.991–1.013 |
| runtime-field | 8.64/9.34 | 6244/6172 | 356.62/293.85 | 462/462 | 1.016–1.493 | 0.794–0.971 |
| runtime-conditional | 8.15/8.16 | 6208/6180 | 171.32/170.53 | 345/345 | 0.982–1.151 | 0.992–1.015 |
| runtime-member-selection | 8.05/8.23 | 6200/6124 | 363.76/355.07 | 523/523 | 1.004–1.042 | 0.641–1.202 |

All **14** inputs retain byte-identical LowIR and identical executable `.text`
bytes, verified by extracted section hashes in the [native record](../student.tests/pa22/audit120-native.json).
Whole ELF hashes differ because separate lane artifacts carry different file
identities; the executed text is identical. Thus the apparent runtime changes,
including the field loop's improved median, establish no new execution benefit.
The previously validated proof outcomes/loop work are preserved.

The template-heavy common compilation has paired ratios **0.971, 1.084, 0.870,
0.868**; its smaller median is not claimed as a compiler speedup. The common
2048-member-function input is essentially unchanged at the median, with paired
ratios **1.010, 1.002, 1.024, 0.970**. Common peak RSS increases at most **220 KiB**.
The member-loop compilation's small-input median regresses **4.8%**, with an
outlying **1.299** pair; this observation is retained, not silently discarded.

Empty-layout compiler medians increase **3.2%** at 512 and **1.6%** at 2048.
The large input's paired ratios are **1.000, 1.020, 1.018, 1.026**. A/A spans
206.77–220.05 ms; A and B span 209.29–214.60 / 213.24–216.74 ms.
These are small measured costs of establishing the previously missing layout
facts. Summary visits scale **1024→4096**; retained identities **2049→8193**.
Peak RSS decreases 92/268 KiB; no memory improvement is claimed from that
allocation/measurement variation.

Inherited-access compiler medians increase **1.3% / 2.6%** at 512/2048 functions.
At 2048, A/A is 195.99–202.25 ms; A/B are 192.73–200.26 / 198.90–525.40 ms.
The final paired ratio **1.815** includes the retained 525 ms observation; the
other pairs are **1.021, 1.047, 1.025**. Peak RSS rises **2200 KiB** in this
campaign. The semantic graph/IR sizes and other work counters are unchanged;
completed base-path hits rise **6144→30725**, reflecting additional related
access checks that previously skipped the later base. No body, class-layout or
specialization is repeatedly instantiated and no unrelated cache is invalidated.
These necessary access checks retain O(related graph) traversal per access,
with flat visited sets and canonical completed path/miss lookup. No optional
transform can be removed to restore the invalid first-base-only behavior.

## Budgets and disposition

Empty-layout summaries consume at most 64 identities per direct class-valued
edge and retain at most 64 per completed class. Overflow means unknown; disjoint
storage is the bounded conservative fallback. Arrays never expand their element
count for this analysis. The billion-element and 70-type controls pass with
bounded work, as do constexpr addresses and member-function displacements.
The new facts have TU lifetimes; traversal temporaries end at each query/layout.
The fix adds no function, branch, call, loop iteration or text growth on correct
common inputs. Incorrect overlapping objects receive the storage needed for
their required distinct identities; that is a semantic cost.

The inherited 64-node local value proof, 4096-visit/depth-64 function proof,
eight-wrapper conversion/receiver proofs and eight-element initializer expansion
cap remain unchanged. Their zero code-growth guarantees, invalidations and
conservative fallbacks are independently reviewed in [audit120](audit.md).
The live-loop profitability and native stack/shift/store inspections in
[117](performance117.md), [118](performance118.md) and [119](performance119.md)
remain evidence, including every regression and the 119 placement diagnostic.
Fewer IR nodes alone are never treated as a runtime result.

Spec §9's PA22/O0 acceptance is satisfied: required semantic work is bounded,
measured regressions are disclosed, and generated work is unchanged on equivalent
correct inputs. There is no mandated numeric latency/RSS threshold at this stage.
Historical +15%, +16 MiB and 5.5× diagnostics remain non-gating, with all old
observations preserved. No mandated limit, timeout, required behavior, comparison
rule or fixture coverage is weakened. Native layout/allocation/optimization and
full self-hosting remain explicitly later-stage work.
