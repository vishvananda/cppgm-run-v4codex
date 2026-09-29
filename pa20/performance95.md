# PA20 handoff 95: range performance and architecture evidence

This handoff completes the range-statement owner at PA20/O0. It adds required
language behavior, with no optional optimization or claimed generated-code
speedup. The remaining closure, aggregate-helper and retained-declaration
failures remain implementation work in [the plan](plan.md).

## Frozen evidence and protocol

[Harness](../student.tests/pa20/benchmark95.py) and
[all observations](../student.tests/pa20/performance95.json) record source and
binary hashes, flags, backend hash, telemetry, compiler wall time/peak RSS,
checked executable runtime and payload sizes. The two binaries are:

| Binary | Commit | SHA256 | Compiler .text bytes |
|---|---|---|---:|
| Entry | `ef897177` | `29cfe4f4ced25eabdc3d652a91b275c74455f855a090029d3497207ed88bf5bc` | 2,012,294 |
| Final | `fe0c1722` | `3971efde7636140fa157dacc70c54d9142d4fbfeb5e67e927534743ca162c1b9` | 2,060,870 |

Compiler text grows 48,576 bytes (2.414%) for range facts, checked implicit
operations, lowering and lifetime handling. Build flags are
`g++ -std=gnu++11 -Wall -O3`, `TEST_RUNNER_ENABLE`; source compilation uses
`--emit-lowir -O0`. The supplied native backend uses `-O0` only in the test
harness. Production output is entirely student-generated typed LowIR.

Measurements ran on CPU 1, with no concurrent build or test. Nine common
workloads each have a warmup per binary, four A/A samples, and four ABBA blocks
for both compilation and execution. Six newly supported range workloads have
a warmup and six final-only samples for each dimension. Total: **432 measured
observations and 48 warmups**, all retained. The earlier complete runs against
[`14fff139`](../student.tests/pa20/performance95-initial.json) and
[`58c89851`](../student.tests/pa20/performance95-before-destination.json) are
also preserved; the tables below use the final `fe0c1722` binary. Validation
and stats are separate from timed compilation. All executions check results. Native .text is unavailable
in the supplied sectionless ELF, so its payload size includes code and data;
it is explicitly a proxy, not an exact text section measurement.

## Comparable compiler and executable costs

Medians are milliseconds; RSS is maximum measured KiB. Each ratio is the
paired final/entry compiler wall-time ratio for one ABBA block. Raw timing
spread and A/A calibration are preserved in the JSON.

| Common workload | Compile A / B ms | RSS A / B KiB | Paired compile ratios | Runtime A / B ms | Payload A = B bytes |
|---|---:|---:|---|---:|---:|
| startup | 8.01 / 7.81 | 5,756 / 6,044 | 0.991, 0.964, 0.983, 0.976 | 3.90 / 3.90 | 24 |
| auto specializations 2,400 | 181.13 / 171.91 | 31,592 / 31,196 | 1.037, 1.022, 1.001, 0.932 | 3.42 / 3.46 | 192,062 |
| namespace variables 2,400 | 135.76 / 136.32 | 25,084 / 25,296 | 0.997, 1.003, 0.993, 1.029 | 3.27 / 3.35 | 24 |
| auto specializations 9,600 | 834.15 / 713.17 | 105,728 / 105,876 | 0.861, 0.867, 0.813, 0.974 | 3.75 / 3.81 | 768,062 |
| namespace variables 9,600 | 545.40 / 542.56 | 81,284 / 81,520 | 0.999, 0.977, 0.983, 0.595 | 6.00 / 5.84 | 24 |
| runtime calls | 15.13 / 14.40 | 6,040 / 5,984 | 0.926, 0.933, 0.973, 0.846 | 208.77 / 215.10 | 206 |
| runtime memory | 12.09 / 11.32 | 6,012 / 6,120 | 0.969, 0.927, 0.953, 0.950 | 137.66 / 135.83 | 434 |
| runtime floating | 14.32 / 13.90 | 6,020 / 6,168 | 0.952, 0.878, 0.953, 0.967 | 151.73 / 151.32 | 230 |
| array pack (supported at entry) | 15.87 / 15.26 | 6,020 / 6,252 | 0.963, 0.987, 0.947, 0.989 | 135.13 / 137.42 | 263 |

All nine common native executables are **byte-identical** between entry and
final. Their runtime differences therefore reflect measurement noise. The
calls/memory/floating/pack loops run long enough to dominate startup and use
volatile limits plus checked dependent sums. Other runtime rows are controls,
not profit evidence. Timing varies substantially within this final run: the
2,400-specialization A/A calibration is 268.67–269.78 ms, while later final
samples span 163.45–282.40 ms. The 9,600-specialization A/A calibration is
681.06–909.31 ms; final samples span 653.90–1,081.46 ms. The namespace-variable
0.595 pair includes an entry stall. The identical native array-pack output has
a 1.453 runtime pair with a final 225.88 ms sample versus its 137.42 ms median.
No observation is removed or interpreted as a stable speedup/regression.

Earlier complete runs are less noisy: the initial common compiler medians were
within 4%, including 716.96/714.72 ms for 9,600 specializations. The final
calls/memory/floating executable times are roughly 1.5–2 times their initial
values for both binaries, although those native bytes never changed. This
limits timing conclusions. There is no supported general speedup claim or
repeatable substantial common-workload regression established by these runs;
compiler size, peak RSS, semantic work and native bytes remain separately
reported.

## New behavior, work and memory bounds

Entry rejects these sources, so there is no correct A/B range implementation
and no claimed comparative benefit. Compiler times are medians and complete
six-sample ranges. Template runtime is startup-dominated; runtime loops below
are the executable-cost evidence.

| New workload | Compile median (range) ms | Peak KiB | Runtime median ms | Native payload bytes |
|---|---:|---:|---:|---:|
| array ranges 800 | 192.52 (185.89–205.34) | 21,944 | 5.69 | 140,861 |
| array ranges 3,200 | 859.39 (796.31–977.89) | 73,904 | 6.38 | 563,261 |
| member ranges 800 | 179.61 (170.62–221.50) | 22,852 | 6.34 | 136,160 |
| member ranges 3,200 | 693.51 (662.95–796.36) | 73,084 | 6.62 | 544,160 |
| live array loop | 13.50 (11.51–14.30) | 6,132 | 188.97 | 260 |
| live class-iterator loop | 12.23 (11.85–12.68) | 6,248 | 223.49 | 464 |

The array loop runs 8 million outer iterations, the class-iterator loop
4 million; both vary an element from the induction variable and verify the
masked sum. Runtime ranges are 183.68–191.65 and 211.07–232.58 ms respectively.
These are necessary new executable costs, not an optimization comparison.

For n=800/3,200, each template workload records exactly n range plans and n
body transitions, with one retained range pattern and n pattern uses. The
array cases check n+1 bodies; member cases n+3. Candidate work is **2n** for
arrays and **2n+2** for member ranges: the two endpoint selections are checked
once at definition and reused across specializations. Parsed nodes are
20,913/83,313 and 24,175/96,175. A second call to each specialization does not
repeat its body or range plan. Fourfold input growth produces approximately
fourfold compile time, candidate work, semantic plans and executable payload.

A concrete range plan is 400 bytes; geometrically grown plan-arena capacities
are 409,600/1,638,400 bytes. One nondependent source recipe is shared by all
specializations. Runtime identities, materializations and lexical prefixes are
per occurrence/context. The analyzer owns these flat indices and arenas until
translation-unit teardown; call/conversion candidate vectors are bounded by
the language-required candidate/argument set and are released after checking.
There is no process-global cache, token replay, fake syntax, rendered-name key,
textual phase transport, or lowering-time overload selection in this path.

## Source-to-native inspection and stage acceptance

[Trace source](../student.tests/pa20/trace95.cpp),
[reproducer](../student.tests/pa20/trace95.py), and
[results](../student.tests/pa20/trace95.json) connect source through native output:

1. `fixed<Bias>` binds one concrete member-range recipe at definition. Two
   demanded specializations consume it; the repeated `fixed<3>` call reuses
   its completed body. Iterator results, transfers and operator declarations
   flow by canonical identities into the plan.
2. `temporary<Range>` checks the dependent range on demand and constructs the
   range prvalue in hidden storage. Its begin/end iterator objects share the
   ordinary class ABI. The range and both iterators stay alive through the
   loop; `continue` preserves them, while the statement exit destroys them.
3. The source variable is initialized from one recorded dereference/conversion
   each iteration. Lexical lifetime prefixes also cover break, goto, return,
   iterator destructors and conversion-created reference temporaries, tested
   by the personal controls.
4. Lowering consumes those records directly. The supplied backend encodes the
   produced LowIR for validation and execution. The trace exits zero;
   stats-on/validated and stats-off LowIR are byte-identical. It has three
   concrete range plans, one fixed recipe, and two recipe uses.

Explicit budgets: one plan per demanded occurrence/context; one nondependent
shape/selection per source recipe; five implicit operations and at most two
iterator objects per user range; one index per array range; linear lifetime
walks and bounded control-flow construction. Conversion/default costs track
actual selected arguments; there is no optional unrolling, inlining or growth
policy added. Existing array expansion limits remain unchanged.

Spec §9 applies at PA20/O0. No wall-time/RSS percentage gate is mandated, and
historical self-selected diagnostics do not become extra gates. All historical
measurements remain preserved. Required semantic work and measured code growth
are disclosed; no correctness/coverage/complexity bound is waived. Student
native encoding, debug and optimization policies belong to their later stages;
self-hosting remains PA34 work. This evidence covers the completed range group,
not the pending independent whole-stage audit.
