# PA20 final audit 101 — performance evidence

Scope: PA20 `--emit-lowir -O0`. Reviewed production commit **806b38fb**.
[Harness](../student.tests/pa20/audit101_benchmark.py),
[whole-stage data](../student.tests/pa20/audit101-performance-stage.json),
[entry/fix data](../student.tests/pa20/audit101-performance-fix.json).
All handoff 94–100 measurements and [checkpoint 97 evidence](performance97.md)
remain intact, including earlier misses, noisy samples and corrected personal
benchmark expectations. This audit adds **1180 observations / 130 warmups**:
784/88 over 28 whole-stage inputs and 396/42 over 12 entry/fix inputs.

## Frozen protocol

| Snapshot | Commit | Binary SHA256 | Compiler .text bytes |
|---|---|---|---:|
| Stage base | `a9b24ab6` | `1a181c95ee97d646d41c1009780200049e776d67e1d715840d5526b3d10e1d40` | 2005958 |
| Audit entry | `8c86c298` | `02c84de4898458446d52bc2cdead7d9e0e845a808209d922fb2a0ed1fa629177` | 2090694 |
| Final | `806b38fb` | `a8a4702950de9694b4e9e269dc6f84c834d9acfded8232a0a10bebdbf1619faf` | 2090694 |

All snapshots use the same host build (`g++ -std=gnu++11 -Wall -O3`, course
runner enabled). The unchanged supplied backend is used only for measurement.
The records freeze binary/backend/harness/source/input hashes, flags and CPU
affinity. Every timed executable passes its live result check. Runtime loops
use volatile limits and data-dependent sums; startup/template execution rows
are startup-dominated controls, never runtime-profit evidence. Compiler sources
are large enough to measure frontend work beyond startup.

Comparable inputs have one warmup per binary, four A/A calibration observations
and four wall-time ABBA blocks, separately for compilation and execution.
New/incorrect baseline cases have six final-only samples after warmup. Every
sample, paired ratio, range, peak RSS and user/system/context-switch observation
is retained. No build/test job ran during either series; the two series ran
sequentially. Instrumentation/validation is separate from timed compilation.
The trace and prior controls verify output equality with telemetry on/off.

Compiler size is actual `.text`. The supplied sectionless native ELF's measured
payload includes code and static data, so it is explicitly a code-size proxy,
not a purported section size. Student native encoding/debug/allocation and
self-hosting do not exist at PA20; no benchmark result is claimed for those
later-stage implementations.

## Whole-stage results

A is stage base; B is final. Entries are medians in ms and maximum sampled RSS.
Both sides of every A/B row compute the same checked result; the old aggregate
paths still lack the required O0 comparison shape. Baseline rejection or wrong
execution is preserved and excluded from paired timing, not treated as a win.

| Workload | Compiler A/B ms | Peak RSS A/B KiB | Native A/B ms | Native payload A/B bytes | Compiler paired B/A range |
|---|---:|---:|---:|---:|---:|
| startup | 5.73 / 5.59 | 5644 / 5956 | 3.06 / 3.04 | 24 / 24 | 0.977–1.016 |
| auto-specializations-9600 | 658.48 / 682.42 | 106936 / 106576 | 3.70 / 3.60 | 768062 / 768062 | 0.909–1.181 |
| namespace-variable-9600 | 547.49 / 565.17 | 82052 / 82332 | 3.96 / 4.03 | 24 / 24 | 0.959–1.077 |
| runtime-calls | 6.26 / 6.12 | 5968 / 6080 | 123.58 / 123.51 | 206 / 206 | 0.971–0.993 |
| runtime-memory | 6.90 / 7.27 | 5964 / 6228 | 72.71 / 72.66 | 434 / 434 | 0.981–1.181 |
| runtime-floating | 6.03 / 5.95 | 6160 / 6276 | 85.99 / 85.60 | 230 / 230 | 0.980–0.994 |
| new-array-pack | — / 6.64 | — / 6148 | — / 75.20 | — / 263 | B only |
| new-array-ranges-3200 | — / 488.15 | — / 70108 | — / 4.00 | — / 563261 | B only |
| new-member-ranges-3200 | — / 415.03 | — / 72380 | — / 4.03 | — / 544160 | B only |
| new-runtime-range-array | — / 6.49 | — / 6144 | — / 117.19 | — / 260 | B only |
| new-runtime-range-class | — / 7.50 | — / 6140 | — / 154.99 | — / 464 | B only |
| array-members-3200 | 641.87 / 658.18 | 105484 / 98484 | 3.56 / 3.69 | 614461 / 704095 | 0.860–1.034 |
| closure-entries-3200 | 750.07 / 725.60 | 108432 / 106156 | 3.68 / 3.62 | 697662 / 633662 | 0.897–1.116 |
| runtime-pointer-closure | 6.72 / 6.59 | 6080 / 6204 | 52.83 / 42.27 | 258 / 208 | 0.945–1.008 |
| runtime-immediate-closure | 6.25 / 6.15 | 6012 / 6160 | 42.60 / 42.53 | 192 / 200 | 0.963–0.992 |
| runtime-array-members | 6.28 / 6.25 | 5952 / 6164 | 34.59 / 106.28 | 285 / 343 | 0.982–1.015 |
| new-capture-specializations-3200 | — / 886.95 | — / 125744 | — / 3.56 | — / 633662 | B only |
| new-runtime-capture | — / 6.40 | — / 6020 | — / 53.41 | — / 196 | B only |
| new-runtime-nested-this | — / 6.88 | — / 6116 | — / 59.80 | — / 282 | B only |
| aggregate-specializations-3200 | 453.60 / 498.68 | 85712 / 90412 | 3.56 / 3.68 | 300802 / 537601 | 0.948–1.116 |
| return-specializations-3200 | 619.18 / 621.77 | 97080 / 101264 | 3.55 / 3.52 | 345494 / 294294 | 0.941–1.071 |
| runtime-aggregate | 7.02 / 6.85 | 6064 / 6236 | 36.62 / 43.42 | 208 / 281 | 0.974–0.989 |
| runtime-return | 11.87 / 11.39 | 6036 / 6156 | 35.88 / 25.25 | 232 / 201 | 0.936–0.981 |
| qualified-specializations-3200 | 711.79 / 737.82 | 61724 / 62172 | 5.25 / 4.48 | 89479 / 89479 | 1.018–1.084 |
| new-retained-specializations-3200 | — / 443.01 | — / 34872 | — / 5.72 | — / 99300 | B only |
| new-captured-ranges-800 | — / 346.23 | — / 34372 | — / 5.30 | — / 166464 | B only |
| new-captured-ranges-3200 | — / 1572.40 | — / 118724 | — / 6.67 | — / 665664 | B only |
| new-runtime-captured-range | — / 11.12 | — / 6220 | — / 68.34 | — / 308 | B only |

Compiler text grows **84,736 bytes (4.224%)** over the stage base and **zero
bytes** for the final audit fix. The binary hash changes for the fix; equal
section length does not mean identical code. Seven stage-control native files
are byte-identical: startup, auto/namespace specializations, calls, memory,
floating point and qualified specializations. Their timing variation cannot
establish a generated-code improvement or regression.

The auto-9600 compiler median increases 3.6%; paired B/A is **1.044, 0.909,
1.029, 1.181**, A/A **622.72–989.41 ms**, final **661.51–851.62 ms**.
Current semantics establishes 9600 return deductions, checks each of 9601
bodies once, and performs no aggregate-independence work. This is measured
required deduction cost with substantial scheduling noise, not optional work.
Namespace-9600 adds no such work; its 3.2% median increase has mixed pairs
**0.959, 1.077, 1.051, 1.046** (A/A 531.92–542.96, B 543.34–599.65 ms).
Qualified-3200 adds one path classification per occurrence: 3200 names/6400
parts, no body transition. Its 3.7% median increase has pairs **1.021, 1.018,
1.084, 1.048**; A/A 713.86–843.78 and B 669.04–757.24 ms. The retained
interpretation is canonical and shared, as trace 100 verifies. These signals
remain disclosed; no broad compiler speedup is claimed.

Pointer-closure runtime benefit repeats: **52.83→42.27 ms**, pairs **0.807,
0.800, 0.780, 0.878**, A/A **51.44–53.17**, B **41.99–49.51 ms**, payload
**258→208 bytes**. This agrees with handoff 96 and audit 97. The useful change
removes a wrapper call while preserving two ABI entries, shared statics and
one semantic body; growth is bounded by demanded entries. This has executable
profit evidence independent of IR node count.

Required array-member helpers cost **34.59→106.28 ms**, pairs **3.094, 3.084,
3.049, 3.067**, A/A 34.25–35.99 and B 105.87–107.19 ms, payload **285→343**.
Required class-member helpers cost **36.62→43.42 ms**, pairs **1.167, 1.189,
1.182, 1.190**, A/A 36.48–37.35 and B 42.56–44.03 ms, payload **208→281**.
These are real regressions from mandatory O0 helper/temporary/copy shapes,
already disclosed in handoffs 96/99 and required by the unchanged oracles.
They are not optional losing optimizations to retain under a profit claim.
Helper duplication was removed by the earlier ownership fix; relaxing the
comparison or introducing an optimizer to eliminate its required shape would
change this stage's contract. Native optimization is a later owner.

At 3200 aggregate specializations, compiler median grows 9.9%, RSS 4700 KiB
and payload 236,799 bytes, with **3202** independence work items, **3199** hits
and unchanged **3200** body transitions/**3202** checked bodies. Paired compiler
B/A is **1.116, 0.948, 1.075, 1.094**, A/A 449.67–526.50 and B 485.81–509.30 ms.
These costs follow distinct demanded types/helpers, without duplicate semantic
work. The array-member compiler's paired spread is 0.860–1.034, with lower
sampled peak RSS than baseline; that RSS difference is recorded, not attributed
to a new memory optimization.

Caller-owned result ABI reduces runtime-return payload **232→201 bytes** and
current runtime **35.88→25.25 ms**, pairs **0.716, 0.734, 0.698, 0.707** (A/A
34.38–36.55, B 24.91–26.06 ms). Handoff 99's earlier mixed/noisy measurements
remain. This is evidence for the current native work reduction, not a claim
that the required ABI change is an optional optimization justified solely by
profit. The 3200-result specialization compiler pairs span 0.941–1.071.

## Audit-entry versus repair

A is the fully passing audit-entry compiler; B includes the captured-range fix.
**Every one of the nine semantically correct A/B native files is byte-identical.**
The three new capture/range baselines compile but fail their result checks;
all are excluded from A/B timing. No faster-incorrect-program comparison is used.

| Workload | Compiler A/B ms | Peak RSS A/B KiB | Native A/B ms | Native payload A/B bytes | Compiler paired B/A range |
|---|---:|---:|---:|---:|---:|
| auto-specializations-9600 | 1579.31 / 1459.07 | 106480 / 106528 | 5.66 / 5.71 | 768062 / 768062 | 0.854–1.075 |
| new-array-ranges-3200 | 1040.20 / 988.99 | 70076 / 70172 | 6.05 / 6.07 | 563261 / 563261 | 0.925–1.065 |
| new-member-ranges-3200 | 416.63 / 415.57 | 73224 / 73172 | 3.96 / 3.83 | 544160 / 544160 | 0.525–2.025 |
| new-capture-specializations-3200 | 920.51 / 896.49 | 125556 / 125624 | 3.53 / 3.54 | 633662 / 633662 | 0.900–1.059 |
| runtime-calls | 6.52 / 6.48 | 6072 / 5948 | 123.45 / 123.31 | 206 / 206 | 0.957–1.047 |
| runtime-memory | 6.43 / 6.24 | 6048 / 6092 | 73.21 / 73.04 | 434 / 434 | 0.964–1.013 |
| runtime-floating | 6.24 / 6.34 | 6200 / 6148 | 86.36 / 86.72 | 230 / 230 | 0.979–1.043 |
| new-runtime-range-array | 6.24 / 6.20 | 6076 / 6172 | 115.50 / 115.13 | 260 / 260 | 0.949–1.007 |
| new-runtime-range-class | 6.68 / 6.66 | 6076 / 6168 | 151.10 / 149.92 | 464 / 464 | 0.974–1.024 |
| new-captured-ranges-800 | — / 194.53 | — / 34484 | — / 3.59 | — / 166464 | B only |
| new-captured-ranges-3200 | — / 781.13 | — / 119168 | — / 3.79 | — / 665664 | B only |
| new-runtime-captured-range | — / 6.54 | — / 6104 | — / 43.97 | — / 308 | B only |

No repeatable compiler regression is established by this fix. Array-range
compiler pairs are **0.938, 0.933, 1.065, 0.925**; peak RSS adds **96 KiB**.
Member-range pairs are **0.525, 0.967, 1.014, 2.025**, A/A **405.31–416.53 ms**,
A samples **407.33–1156.83**, B **405.02–1272.44 ms**. Both directions contain
large scheduling outliers; the 416.63/415.57 medians cannot erase that spread.
Capture-specialization pairs are **0.900, 0.987, 1.059, 1.000**; RSS adds 68 KiB.
The auto-specialization A/A interval **1434.53–2068.24 ms** is very different
from the whole-stage series, further ruling out comparisons of separate-run
medians as speedup evidence. All timing observations are preserved.

The repaired 800/3200 workloads have **800/3200 body transitions, 1601/6401
checked bodies, 800/3200 closures/capture edges/range plans**, one fixed recipe
and 800/3200 recipe uses. Repeated calls do not recheck bodies. Range metadata
is **409600/1638400 bytes**, exactly 4×. Final-only compiler time is
194.53/781.13 ms (4.02×), peak RSS 34484/119168 KiB (3.46×) and native payload
166464/665664 bytes (4.00×). The earlier whole-stage samples report 346.23/
1572.40 ms and remain intact. The repaired live loop is **43.97 ms**
(43.79–44.87), **308-byte** payload; its correctness fix has no relative profit
claim. Source inspection shows only constant work per direct range binding and
one ordinary hidden reference for a captured object.

## Budgets and final acceptance

Legality, sharing keys, completed-fact validity, conservative fallbacks and
release boundaries are independently reviewed in [audit.md](audit.md). There
is no new optimizer, global rescan, fixed point, inlining or unrolling. Required
work follows source nodes, demanded semantic facts/dependency edges and emitted
IR. Initialization expansion product **8**, one range plan per occurrence,
constant implicit-operation count, one capture per closure/object and bounded
callable/helper emission remain unchanged. Unknown independence retains ordered
IR; unknown capture storage cannot bypass source-expression binding.

**Stage-scoped performance acceptance: pass.** The PA20/O0 handout mandates
behavior, LowIR shape and required test/inspection rules, not a numeric latency,
RSS, native-runtime or text ceiling. The inherited **+15%, +16 MiB and 5.5×**
values are self-selected diagnostics, as documented by PA18 and PA19; they
remain diagnostics here under spec §9. No measurement, historical miss,
mandatory bound, correctness condition or coverage is removed. Necessary
semantic/contract costs are measured and explained; optional pointer-wrapper
removal has repeatable native benefit. The audit repair introduces no native
regression on equivalent correct inputs, and adds bounded required work on
previously wrong programs. No unsupported optimization-profit or later-stage
performance claim is used to close PA20.
