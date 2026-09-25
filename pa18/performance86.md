# PA18 accumulated audit 86 performance evidence

Acceptance is **PA18/O0 LowIR**, spec §9. The [frozen harness](../student.tests/pa18/benchmark86.py)
compares last reviewed `ecc308bc` against reviewed `f6eaf8ab` across the full union
of handoffs 83–85, plus audit exception/lifetime workloads. The
[raw record](../student.tests/pa18/loop86-performance.json) preserves source text,
input/binary/backend hashes, CPU affinity, flags, warmups, telemetry and every
observation. There are **36 workloads and 869 observations**, including startup,
warmups and native timing. Correctness validation finished before timing began.

Comparable cases use one warmup per binary, four A/A calibration observations
and four wall-time ABBA blocks. Rejected or incorrect baselines receive six
final-only observations. Compiler execution and native execution are timed
separately with `/usr/bin/time` peak RSS and scheduling counters. Preflights use
`--stats --validate-lowir`; timed compiler invocations use `--emit-lowir -O0`.
Build flags are `g++ -std=gnu++11 -Wall -O3`, with the course test runner.

The supplied `lowir2native-ref -O0` is an explicit harness boundary, bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`. Native results are checked before
measurement and on every timed execution. Volatile runtime bounds and checked
sums/effect counts keep loops, calls, memory, floating operations and lifetimes
live. Own native optimization, debug encoding and self-hosting belong to later
stages. No host/reference tool implements the student's production frontend or
LowIR output.

All five historical measurement sets from handoffs 83–85 are retained and their
binary identities and sample protocols independently verified: **1990** prior
observations. Both initial noisy/redundant-work runs remain evidence. Their
historical misses are not permanent exit gates for the corrected implementation.

## Compiler latency, peak RSS and executable size

Times are median milliseconds; ratios are paired block B/A medians with their
full ranges. RSS is maximum observed KiB. Native size is the supplied sectionless
ELF's loadable payload, including static data; it is **not isolated `.text`**.
Compiler `.text` is measured directly. Startup-sized compiler rows are diagnostics.

| Workload | A / B ms | Paired B/A (range) | A / B KiB | A / B payload bytes |
|---|---:|---:|---:|---:|
| ordering-600 | 60.11 / 56.43 | 0.996 (0.970–1.003) | 15192 / 15304 | 40842 / 40842 |
| ordering-2400 | 221.59 / 228.68 | 1.029 (1.005–1.061) | 43932 / 43984 | 163242 / 163242 |
| common-loop-float-1500 | 158.62 / 161.03 | 0.997 (0.971–1.051) | 30260 / 30316 | — / — |
| runtime-calls | 6.08 / 5.88 | 0.982 (0.970–1.002) | 5896 / 6064 | 206 / 206 |
| runtime-memory | 6.11 / 6.01 | 0.990 (0.970–1.014) | 5848 / 6064 | 434 / 434 |
| runtime-floating | 5.95 / 5.93 | 0.985 (0.973–1.016) | 6088 / 6152 | 230 / 230 |
| wide-array-600 | 52.73 / 55.45 | 1.047 (0.935–1.070) | 13284 / 13760 | 32505 / 45712 |
| converted-array-600 | 79.88 / 84.86 | 1.067 (0.984–1.160) | 18296 / 18500 | 36105 / 45712 |
| deduced-rows-600 | — / 68.29 | final only | — / 16080 | — / 78104 |
| pack-bound-600 | — / 65.33 | final only | — / 14712 | — / 37308 |
| wide-array-2400 | 200.81 / 212.66 | 1.071 (1.017–1.115) | 38084 / 37468 | 129705 / 182512 |
| converted-array-2400 | 321.26 / 349.64 | 1.087 (1.025–1.153) | 54268 / 56108 | 144105 / 182512 |
| deduced-rows-2400 | — / 272.52 | final only | — / 46376 | — / 312104 |
| pack-bound-2400 | — / 251.82 | final only | — / 40808 | — / 148908 |
| runtime-array-wide | 6.02 / 5.99 | 0.997 (0.979–1.000) | 5968 / 6024 | 284 / 320 |
| runtime-array-conversion | 6.73 / 6.65 | 0.992 (0.970–3.311) | 5972 / 6080 | 314 / 320 |
| empty-aggregate-600 | 25.24 / 24.72 | 0.977 (0.971–0.989) | 9128 / 8976 | 22956 / 18139 |
| empty-aggregate-2400 | 83.91 / 81.08 | 0.963 (0.694–1.097) | 19468 / 18284 | 91356 / 72139 |
| delegation-150 | 30.92 / 30.68 | 0.995 (0.971–1.003) | 9864 / 9880 | 12701 / 12701 |
| delegation-600 | 108.83 / 108.87 | 0.995 (0.946–1.003) | 22332 / 22404 | 50501 / 50501 |
| runtime-empty-aggregate | 6.18 / 6.15 | 0.998 (0.898–1.076) | 5984 / 6064 | 224 / 201 |
| runtime-delegation | 6.51 / 6.34 | 0.957 (0.932–1.039) | 5964 / 6096 | 256 / 256 |
| empty-lifetime-600 | — / 37.01 | final only | — / 11604 | — / 108768 |
| discard-scalar-150 | 18.55 / 18.53 | 0.997 (0.990–1.767) | 8132 / 8204 | 11212 / 11212 |
| discard-class-150 | — / 13.96 | final only | — / 7432 | — / 46992 |
| storage-dormant-150 | 15.31 / 15.39 | 0.999 (0.998–1.013) | 7364 / 7432 | 3105 / 3105 |
| discard-scalar-600 | 54.73 / 55.33 | 1.009 (1.008–1.017) | 14608 / 15036 | 44512 / 44512 |
| discard-class-600 | — / 36.46 | final only | — / 11928 | — / 185592 |
| storage-dormant-600 | 44.26 / 44.38 | 1.004 (0.998–1.010) | 12140 / 12184 | 12105 / 12105 |
| runtime-discard-scalar | 5.90 / 5.82 | 0.991 (0.985–1.001) | 5932 / 5984 | 249 / 249 |
| runtime-discard-class | — / 5.78 | final only | — / 5828 | — / 1240 |
| audit-effects-150 | — / 14.37 | final only | — / 7440 | — / 5201 |
| audit-lifetimes-150 | 26.34 / 25.79 | 0.985 (0.974–1.447) | 9152 / 9228 | 55328 / 55328 |
| audit-effects-600 | — / 39.52 | final only | — / 11716 | — / 20501 |
| audit-lifetimes-600 | 87.68 / 86.37 | 0.986 (0.972–0.998) | 18812 / 18900 | 219128 / 219128 |
| runtime-audit-lifetimes | 6.28 / 6.29 | 0.986 (0.913–1.033) | 5932 / 6064 | 1112 / 1112 |

Startup A/B medians: **5.46 / 5.40 ms**.
Compiler `.text`: **1,969,094 → 1,984,518 bytes**, **+15,424 (0.783%)**.

## Executable runtime

| Workload | A / B ms | Paired B/A (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 725.96 / 732.91 | 1.004 (0.985–1.026) | 206 / 206 |
| runtime-memory | 417.49 / 419.99 | 1.011 (0.959–1.025) | 434 / 434 |
| runtime-floating | 499.58 / 496.76 | 0.995 (0.993–0.999) | 230 / 230 |
| runtime-array-wide | 80.38 / 85.78 | 1.070 (0.958–1.082) | 284 / 320 |
| runtime-array-conversion | 85.41 / 85.62 | 0.987 (0.896–1.010) | 314 / 320 |
| runtime-empty-aggregate | 179.84 / 129.81 | 0.719 (0.713–0.726) | 224 / 201 |
| runtime-delegation | 113.26 / 113.43 | 1.000 (0.994–1.008) | 256 / 256 |
| runtime-discard-scalar | 132.38 / 132.53 | 1.001 (1.000–1.290) | 249 / 249 |
| runtime-discard-class | — / 276.48 | final only | — / 1240 |
| runtime-audit-lifetimes | 270.51 / 273.22 | 0.997 (0.981–1.010) | 1112 / 1112 |

## Legality, bounded work and acceptance

The empty-aggregate omission is the optional transform in this range. A checked
empty action group has no initialization work; its destination and any required
value-initialization zeroing remain. The profitability test removes one empty
helper call/body. Work is one O(1) head check, code-growth allowance is zero,
and nonempty/effectful groups use the normal path. Runtime paired B/A is
**0.719 (0.713–0.726)**, with payload **224 → 201 bytes**. All four blocks improve,
confirming the two prior handoff-84 runs. At 2400 functions compiler paired B/A
is **0.963**, peak RSS **19468 → 18284 KiB**, payload **91356 → 72139 bytes**.
The omission is profitable without added compiler work or growth.

The PA16 array image/copy requirement has a measured necessary cost. At 2400
functions, wide arrays have compiler paired B/A **1.071** and converted arrays
**1.087**; RSS is **38084 → 37468** and **54268 → 56108 KiB**, respectively.
The old compiler skipped required proof/image work for these cases. Work remains
linear in explicit actions/output; compressed omitted tails and the eight-lane
expansion limit remain. The audit's additional literal-type checks reuse the
class fact rather than scanning initializer syntax or rejecting every construction.

The wide-array runtime paired median is **1.070**, with range **0.958–1.082**;
three blocks regress and one has a slower A observation. Payload is **284 → 320**
bytes, including the required 32-byte readonly image. The earlier handoff-83
four-block result was **1.065**. This disclosed regression comes from the
contractually required image/copy, not an optional pass that can be removed.
The converted-array runtime paired median is **0.987 (0.896–1.010)**, payload
**314 → 320**; noisy observations support no precise speedup claim. Native
implementation of the required copy belongs to the supplied/later backend.

The ordering-2400 compiler paired ratio is **1.029** on identical LowIR, while
ordering-600 and common-loop/float ratios are **0.996** and **0.997**. Peak RSS
changes on those workloads are **+52, +112 and +56 KiB**, respectively. The new
source-form bit, required conversion checks and bounded literal-type lookup add
semantic work; no unrelated scan, eager body or optional transform explains a
removable regression. All observations and A/A spread are preserved; no compiler
speedup is claimed. Unchanged runtime call/memory/floating/scalar-discard probes
retain identical native bytes, so their timing shifts—including a **1.290**
scalar-discard outlier block—are measurement variation rather than code changes.

| Scaling family | 150 → 600 | Work evidence |
|---|---|---|
| Delegation | 300 → 1200 entry-work items; 3313 → 13213 instructions | Each of three monotonic entry bits traverses only its selected edge; local scratch dies after completion. |
| Fixed class discard | 1 → 1 selections; 150 → 600 recipe uses/materializations; 2434 → 9634 instructions | One shared fixed recipe, distinct contextual lifetime for every evaluated use. |
| Audit exception query | 1 → 1 selections; 8 → 8 exception-work items; zero materializations; 613 → 2413 instructions | Complete canonical query identity shares the checked copy/default effects across specializations; no body demand. |
| Audit array lifetimes | 453 → 1803 constant-work items; 4827 → 19227 instructions | Memoized literal-type proof preserves ordinary temporary construction/destruction; no source rescan. |

The previous reviewed compiler predates the faulty removal of the temporary
syntax exclusion. Consequently it is correct on the audit lifetime benchmarks:
reviewed output is **byte-identical**, with the same **1112-byte** runtime payload.
The entry handoff-85 compiler fails the corresponding audit controls; that
incorrect implementation is not used as a faster comparator. The repair restores
the required behavior through canonical semantic facts. Audit effect workloads
are final-only because the previous review computed wrong `noexcept` values.
Class-discard, nontrivial empty-copy, and dependent array-bound baselines likewise
fail preflight or reject valid input; their complete failures and final-only
costs remain in the raw record.

No new optimization level, fixed-point pass, inlining, code cloning, allocator
or pipeline growth budget was introduced. Existing named-result summaries retain
their eight-wrapper proof bound and pass the inherited legality/emission checks.
The constant evaluator's existing work/depth limits, conservative failure path,
ABI entries, source positions and all observable lifetimes remain.

PA18/O0 has no mandated numerical compiler latency/RSS ceiling. Historical
**+15%, +16 MiB and 5.5×** targets remain diagnostic. The evidence establishes
bounded required semantic work, a repeatable benefit for the optional empty-helper
omission, and disclosed required array-copy costs. Correctness, coverage, mandated
limits and work/growth bounds are preserved. This cumulative range meets the
spec's stage-scoped performance acceptance; PA18's three remaining correctness
mismatches remain implementation work.
