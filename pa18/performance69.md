# PA18 loop 69 performance evidence

Frozen entry `e09162fa` and implementation `f2a9d7a3` (reference-only commit
`6073dbc0`), compiler SHA256s
`4a7204679e553cf8702b4dbfd32f306a0dd6077355b12bb19a56de3c6b674a32` and
`7979fa9128b01bee21d63a5840357b03ebc4396143c3ced0345aaa8154df196c`.
[Harness](../student.tests/pa18/benchmark69.py),
[all observations](../student.tests/pa18/loop69-performance.json).

Flags `--emit-lowir -O0`; host `g++ -std=gnu++11 -Wall -O3` with
`TEST_RUNNER_ENABLE`. CPU affinity 31. One warmup per binary, four A/A samples,
then four ABBA wall-time blocks for equivalent correct inputs. Six final-only
samples measure newly required behavior rejected at entry. Telemetry/full IR
validation run in preflights, separately from measurements. Inputs, binaries,
LowIR, backend and executables are hashed. Raw times, peak RSS, context switches
and every outlier are retained. Required tests completed before measurement.

| Compiler workload | Entry ms | Final ms | Paired B/A median (range) | Peak RSS A / B KiB |
|---|---:|---:|---:|---:|
| ordering-600 | 54.55 | 54.45 | 0.997 (0.993–1.013) | 15212 / 15160 |
| ordering-2400 | 212.72 | 210.76 | 0.993 (0.954–0.998) | 43196 / 43512 |
| member-head-600 | 96.67 | 97.35 | 1.006 (0.999–1.290) | 22656 / 22848 |
| member-head-2400 | 393.52 | 392.81 | 1.009 (0.988–1.026) | 73796 / 73996 |
| common-loop-float-1500 | 151.40 | 152.71 | 1.009 (0.931–1.028) | 30256 / 30388 |
| assignment-query-600 | — | 68.06 | new behavior | — / 17432 |
| assignment-query-2400 | — | 272.40 | new behavior | — / 52360 |
| destructor-query-600 | — | 32.99 | new behavior | — / 10484 |
| destructor-query-2400 | — | 119.13 | new behavior | — / 24840 |

Final startup median is **5.54 ms**. The larger fixed workloads dominate startup;
small runtime-source compilation samples remain in JSON as diagnostics. Compiler
A/A ranges include ordering-600 **54.29–55.08 ms**, ordering-2400
**206.71–212.55 ms**, member-head-2400 **390.31–399.18 ms** and common-loop-float
**150.03–151.66 ms**. The member-head-600 outlier (154.36 ms) is retained. Common
paired medians span 0.993–1.009; spreads/noise do not establish a repeatable
compiler slowdown or speedup. Peak RSS increases by at most **316 KiB** on these
equivalent scaled inputs. No optional transformation or avoidable global work
was introduced.

| Checked executable | Entry seconds | Final seconds | Paired B/A median (range) | Payload bytes A / B |
|---|---:|---:|---:|---:|
| runtime-calls | 0.71376 | 0.71439 | 1.002 (1.000–1.006) | 206 / 206 |
| runtime-memory | 0.41652 | 0.41677 | 1.000 (0.986–1.001) | 434 / 434 |
| runtime-floating | 0.49471 | 0.49496 | 1.001 (0.997–1.002) | 230 / 230 |
| runtime-new-reference | — | 0.13435 | new behavior | — / 215 |

Equivalent LowIR and executables are byte-identical: zero generated code growth.
Timing differences measure noise, not an optimizer benefit. Volatile runtime
bounds and checked results prevent constant-folded/dead loops. The new workload
checks 24 million calls/updates through a converted reference; entry rejects it,
so its cost is not a speed comparison. The supplied backend emits sectionless
ELF: payload after entry is a **text/alignment proxy**, not a `.text` section
measurement. Runtime inputs have no static data. Backend bundle remains
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.

Assignment-query 600→2400 scales **4.00×** in latency and **3.00×** in RSS;
destructor-query scales **3.61×** and **2.37×**. Existing observational telemetry:

| Work counter | Assignment 600→2400 | Destructor 600→2400 |
|---|---:|---:|
| type-query work | 2407→9607 | 3005→12005 |
| candidate work | 600→2400 | 2400→9600 |
| type-substitution work | 2404→9604 | 3005→12005 |
| substitution frames | 1200→4800 | 1800→7200 |

Compiler `.text` is **1,846,790 → 1,864,774 bytes**, +17,984 (**0.974%**),
for required semantic handling, typed lowering and ABI adapters. No optimizer or
own native backend is added. New facts use existing TU-owned vectors/indexes;
candidate scratch is local, destructor properties are memoized by declaration,
and queries/frames use canonical complete keys. Forward completion uses existing
precise reverse dependencies. Single-evaluation lowering adds constant work per
assignment; name encoding is linear in the retained expression/name.

PA18/O0 mandates no numerical latency/RSS/runtime/text ceiling. Historical PA17
**+15%, +16 MiB, 5.5×** self-selected targets remain diagnostics under spec §9;
all prior measurements remain. Correctness, coverage and graph-work bounds are
required. New semantic costs are measured against a rejecting baseline without
claiming an optimization win. No unprofitable optional pass exists to remove.
Stage-scoped acceptance passes; native optimization and self-hosting remain
PA24–PA34 obligations. Independent audit remains pending.
