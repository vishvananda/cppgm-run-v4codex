# PA18 checkpoint audit 70 performance evidence

Reviewed code: `f59e8f67cd8c832361130aef9af1a0337b25c45d`.
The [harness](../student.tests/pa18/audit70_benchmark.py) freezes binaries,
`--emit-lowir -O0`, and the union of all loop 67–69 fixed workloads, plus new
volatile-assignment and arrow-query controls. Host flags are
`g++ -std=gnu++11 -Wall -O3`, test runner enabled; CPU affinity 31.

Two full protocols each cover **28 workloads**:
[checkpoint observations](../student.tests/pa18/loop70-performance.json) compare
entry `15b34993` with the audited tip; [cumulative observations](../student.tests/pa18/loop70-cumulative-performance.json)
compare previous reviewed `3a883d10` with that tip. The cumulative A binary is
loop 67's frozen entry; `3a883d10`→`06211ad0` changes records only, and its hash
matches the preserved audit 66 implementation. No compiler is rebuilt during
measurement. Binary/input/LowIR/executable hashes, flags, backend and all raw
observations are retained, including outliers and earlier failed implementations.

One warmup each, four A/A samples, then four wall-time ABBA blocks compare
semantically equivalent correct outputs. New behavior rejected by A has six
final-only samples. Telemetry and full LowIR validation run in separate preflights;
measurement omits both. Compilation and execution are timed separately. Runtime
sources use volatile bounds, runtime work and checked results. The supplied
backend bundle is `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.

## Compiler latency and memory

Checkpoint comparison (A = entry, B = audited implementation):

| Workload | A ms | B ms | Paired B/A median (range) | Peak RSS A / B KiB |
|---|---:|---:|---:|---:|
| ordering-600 | 70.51 | 86.07 | 1.260 (0.771–1.548) | 15204 / 15296 |
| ordering-2400 | 268.16 | 250.21 | 0.962 (0.929–1.512) | 43612 / 43700 |
| member-head-600 | 103.63 | 110.56 | 1.060 (1.001–1.139) | 22904 / 22872 |
| member-head-2400 | 441.81 | 458.61 | 1.007 (0.968–1.439) | 73980 / 73932 |
| common-loop-float-1500 | 204.10 | 182.13 | 0.871 (0.668–0.979) | 30280 / 30176 |
| address-600 | 93.82 | 93.30 | 0.995 (0.985–1.006) | 21288 / 21384 |
| function-address-600 | 64.03 | 64.44 | 1.007 (0.993–1.017) | 16876 / 17020 |
| address-2400 | 416.11 | 383.09 | 0.935 (0.878–0.995) | 67100 / 67344 |
| function-address-2400 | 246.51 | 245.44 | 0.993 (0.897–1.034) | 49456 / 49476 |
| partial-head-600 | 46.36 | 46.76 | 1.006 (0.976–1.065) | 13184 / 13272 |
| base-fallback-600 | 135.77 | 120.66 | 0.950 (0.872–1.026) | 19784 / 19852 |
| partial-head-2400 | 172.15 | 174.52 | 1.017 (0.997–1.153) | 35624 / 35528 |
| base-fallback-2400 | 335.26 | 340.94 | 1.028 (1.003–1.087) | 61468 / 61532 |
| assignment-query-600 | 68.97 | 68.80 | 0.993 (0.980–1.005) | 17372 / 17356 |
| destructor-query-600 | 32.79 | 32.65 | 0.997 (0.991–1.021) | 10420 / 10488 |
| assignment-query-2400 | 272.73 | 276.63 | 1.012 (1.004–1.021) | 52464 / 52360 |
| destructor-query-2400 | 119.30 | 119.45 | 1.003 (0.998–1.021) | 24772 / 24876 |
| volatile-assignment-600 | — | 89.11 | new behavior | — / 19076 |
| arrow-query-600 | — | 41.51 | new behavior | — / 11344 |
| volatile-assignment-2400 | — | 345.24 | new behavior | — / 57924 |
| arrow-query-2400 | — | 157.95 | new behavior | — / 27720 |

Full reviewed-range comparison, equivalent common inputs:

| Workload | Previous reviewed ms | Final ms | Paired B/A median (range) | Peak RSS A / B KiB |
|---|---:|---:|---:|---:|
| ordering-600 | 55.11 | 55.41 | 0.990 (0.937–1.013) | 15144 / 15260 |
| ordering-2400 | 235.10 | 221.34 | 0.993 (0.807–1.074) | 43324 / 43044 |
| member-head-600 | 97.92 | 101.51 | 1.113 (0.896–1.247) | 22604 / 22920 |
| member-head-2400 | 424.25 | 408.49 | 1.030 (0.910–1.112) | 72940 / 73444 |
| common-loop-float-1500 | 158.78 | 172.36 | 1.057 (1.000–1.465) | 30184 / 30204 |

The initial checkpoint ordering-600 paired range 0.771–1.548 and A/A range
62.95–77.12 ms do not support a 26% code-cost claim. Other raw samples also have
large outliers. Work counters are exactly unchanged on ordering-600,
member-head-600 and base-fallback-2400 between checkpoint binaries. Therefore
[focused observations](../student.tests/pa18/loop70-focused-performance.json)
repeat those inputs with five complete compilations per observation, four A/A
batches and six ABBA blocks, retaining every individual invocation:

| Follow-up workload | A ms | B ms | Paired B/A median (range) | A/A batch mean range ms |
|---|---:|---:|---:|---:|
| ordering-600 | 55.98 | 55.65 | 0.997 (0.896–1.022) | 53.59–62.37 |
| member-head-600 | 105.76 | 102.54 | 0.994 (0.936–1.017) | 95.48–111.47 |
| base-fallback-2400 | 341.21 | 340.10 | 1.007 (0.945–1.029) | 331.77–357.98 |

The cumulative comparison's [focused repeat](../student.tests/pa18/loop70-focused-cumulative.json)
uses the same frozen binaries and sources, five invocations per observation,
four A/A batches and four ABBA blocks:

| Cumulative follow-up | Previous reviewed ms | Final ms | Paired B/A median (range) | A/A batch mean range ms |
|---|---:|---:|---:|---:|
| member-head-600 | 97.25 | 99.01 | 1.020 (0.962–1.098) | 97.01–97.91 |
| member-head-2400 | 399.46 | 418.52 | 1.051 (0.993–1.347) | 399.02–420.06 |
| common-loop-float-1500 | 340.75 | 349.02 | 1.023 (0.976–1.050) | 314.17–352.59 |

The cumulative follow-ups retain modest positive paired estimates (2.0%, 5.1%,
2.3%), with mixed blocks and the 1.347 member-head-2400 outlier. They do not
support a compiler speedup. The accumulated member-head implementation adds
required outer lexical/prefix frames: at 600 instances, substitution frames
4200→5400 and argument packs 5404→6604, while type-substitution work falls
5402→4802 through 600 cache hits. The loop/float workload adds one final compound
assignment conversion check per function: conversion work 19500→21000. These
are bounded semantic costs of complete context and operand validity, with no
extra global traversal or optional optimization to remove. Common cumulative
peak RSS increases by at most 504 KiB in the full protocol. No stage-mandated
numeric limit is missed, and these costs do not become a new exit gate.


## Executable runtime and size

Checkpoint A/B outputs on all 23 equivalent workloads are byte-identical LowIR;
every executable built from them is also byte-identical. Zero generated-code
growth is established for those equivalent outputs. Runtime differences below
measure noise, not an optimizer benefit. Cumulative common outputs are also
byte-identical. The other cumulative inputs are required new behavior, so their
cost is not a speed comparison with the rejecting earlier implementation.

| Checked workload | Entry seconds | Final seconds | Paired B/A median (range) | Payload A / B bytes |
|---|---:|---:|---:|---:|
| runtime-calls | 0.73438 | 0.73514 | 0.998 (0.974–1.011) | 206 / 206 |
| runtime-memory | 0.42566 | 0.42393 | 0.996 (0.979–1.013) | 434 / 434 |
| runtime-floating | 0.50306 | 0.50456 | 1.004 (0.985–1.018) | 230 / 230 |
| runtime-new-address | 0.23340 | 0.23340 | 0.999 (0.997–1.005) | 195 / 195 |
| runtime-new-partial | 0.11490 | 0.11475 | 0.997 (0.988–1.007) | 173 / 173 |
| runtime-new-reference | 0.13470 | 0.13537 | 1.005 (1.000–1.020) | 215 / 215 |
| runtime-audit-volatile | — | 0.13674 | new behavior | — / 217 |

The separate arrow-effects workload checks **24 million** calls. The entry
executable returns **1** (incorrectly omits the required arrow action); final
returns **0**. Six checked final runs have median **6.03348 s**,
range **5.61534–8.46290 s**. Its ELF file grows **300→308 bytes**,
payload **180→188 bytes**, for required observable work. This is a necessary
correctness cost, not an optimization comparison. The supplied sectionless ELF
payload is a text/alignment proxy; this arrow case also includes the global
counter's data, so it is explicitly not an exact `.text` measurement. Other
runtime benchmark inputs have no static data.

Compiler `.text`: checkpoint **1,864,774→1,866,374 bytes**
(+1,600); complete range
**1,837,766→1,866,374 bytes**
(+28,608).

## Scaling, ownership and acceptance

- volatile-assignment, 600→2400: **3.87×** latency, **3.04×** RSS.
- arrow-query, 600→2400: **3.80×** latency, **2.44×** RSS.

Compiler work follows demanded query/candidate facts and explicit arrow/base
edges. Modifying builtin candidates keep cv and correct ranking types; lowering
consumes those facts once. Arrow queries use class completion without needless
layout, detect cycles, and return ordinary rejection state. Destructor properties
are computed in their owner's context once, regardless of the caller's privileges.
No optional transform, speculative growth, global retry, extra phase representation
or process-global cache was added. Scratch candidate/worklist storage is local;
canonical facts are TU-owned. Existing array expansion/work limits are unchanged.

PA18 requires **O0 LowIR**, with no mandated numerical latency/RSS/runtime/text
ceiling. Historical PA17 **+15%, +16 MiB, 5.5×** targets remain diagnostics under
spec §9, as already classified by audit 66; every historical observation remains.
No mandatory limit, correctness obligation, failure or comparison is relaxed.
New semantic costs and supplied-backend constraints do not create extra exit gates.
No runtime improvement or compiler speedup is claimed. Native allocation, native
optimization and self-hosting remain PA24–PA34 obligations. Final acceptance is
based on the full observations, resolved correctness/ownership defects, bounded
required work and repeated comparisons, not a self-selected percentage target.
