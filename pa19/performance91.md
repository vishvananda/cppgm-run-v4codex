# PA19 implementation 91 performance evidence

The completed type-composition changes add required semantics at PA19/O0.
Nine workloads supported correctly by both binaries retain byte-identical LowIR
and native output. Thirteen newly supported workloads have final-only costs;
the entry compiler rejects them or executes incorrectly. No optimization or
precise compiler/runtime speedup is claimed. Stage-scoped performance acceptance
for this behavior group passes; whole-stage review remains pending.

## Frozen experiment

Entry source: `e5f4c3ed78972c8d161671d145bf525cb99033f4`;
final implementation: `57e27df26d081379580a7b2c08cc30fd39c0378c`.
Compiler SHA-256 values:
`0f422d8c85e5da91c8f89188718cfb6394bb880a1cde06cf5183d35ca3d026ca` and
`d65826dd89e27b465ac9cf8bd61db6de565c5913b24856c5740345fbb73c6b02`.
Both use `g++ -std=gnu++11 -Wall -O3`, `TEST_RUNNER_ENABLE`; measured invocations
use `--emit-lowir -O0`. Compiler `.text` is 2,000,006 → 2,004,358 bytes
(+4,352, 0.218%). The unchanged supplied native backend uses `-O0`; its hash and
bundle revision are in every primary evidence file. Production compilation
never invokes this backend; the validation harness does, per the course.

[The harness](../student.tests/pa19/benchmark91.py) pins CPU 31 on Linux x86-64.
Each comparable measurement has one warmup per binary, four A/A observations,
and four ABBA blocks. New behavior has one warmup and six final observations.
Compilation and execution are separate. Instrumented `--stats --validate-lowir`
preflights collect counters separately from timed compiles. Every produced
executable is checked for exit zero before accepting timing. Runtime sources
use volatile loop bounds and result checks; calls, memory and floating-point
benchmarks run for 124–221 ms, well above process startup.

Raw records preserve every source, source/output hash, timing, RSS, counter,
warmup and interruption: [initial](../student.tests/pa19/performance91-initial.json),
[reference binding](../student.tests/pa19/performance91-reference.json),
[ordinary member comparison](../student.tests/pa19/performance91-member.json),
and [frontend executions](../student.tests/pa19/performance91-frontend-runtime.json).
There are 22 completed distinct workloads, 576 measured observations and 68
warmups, including startup calibration. The initial last runtime source was
invalid C++11 (assignment to an xvalue scalar member); its record and exact
harness source are retained. The follow-up binds a named reference before
assignment. It supplies the missing completed measurement, without altering any
prior observation. Harness hashes and retained snapshots distinguish all three versions.

## Compiler and generated output

Times are milliseconds, RSS is peak KiB. Paired columns report median B/A and
full block-ratio range; individual timing ranges include all measured samples.
The compiler's empty-program medians were 6.0–11.4 ms across sessions. Tiny
runtime-source compiles are startup-dominated and provide no speedup evidence.
Frontend native programs below only check endpoints; their measured runtime
medians are startup-dominated too (no useful runtime comparison).

| Shared workload | Compile A → B | B range | A/A range | Paired B/A (range) | RSS A → B | Native payload bytes | Native B ms |
|---|---:|---:|---:|---:|---:|---:|---:|
| ordinary-ordering-600 | 50.6 → 50.3 | 49.0–51.3 | 49.5–50.9 | 0.994 (0.973–1.004) | 14488 → 14636 | 33705 | 1.805 |
| ordinary-ordering-2400 | 198.0 → 197.5 | 193.8–200.9 | 195.9–198.1 | 0.982 (0.941–1.007) | 40400 → 40468 | 134505 | 1.683 |
| ordinary-ordering-9600 | 905.5 → 886.1 | 822.1–2161.1 | 811.5–859.2 | 1.026 (0.673–1.133) | 142756 → 145292 | 537705 | 1.615 |
| ordinary-member-600 | 160.1 → 154.3 | 147.4–185.1 | 143.6–177.0 | 0.976 (0.932–1.058) | 17512 → 17640 | 624 | 1.525 |
| ordinary-member-2400 | 653.0 → 634.0 | 614.3–657.0 | 631.6–694.2 | 0.991 (0.962–1.025) | 52808 → 52968 | 2424 | 1.541 |
| ordinary-member-9600 | 1191.6 → 1188.6 | 1181.2–1240.2 | 1207.4–1353.4 | 1.009 (0.985–1.019) | 193612 → 193728 | 9624 | 1.729 |

Shared frontend compiler paired medians range 0.976–1.026 with no repeatable slowdown.
The largest ordering input has unusually broad individual timings (both
binaries reach 2.16 s); all samples remain. Its RSS changes by +2,536 KiB
(1.78%). The ordinary-member control exercises the same class/alias density
without unsupported template-template forwarding; its largest paired median
is 1.009 and peak RSS changes by +116 KiB. Noise and different session speed
preclude a precise speedup or cross-session scaling claim.

| Newly correct workload | Compile B | B range | RSS B | Native payload bytes | Native B ms |
|---|---:|---:|---:|---:|---:|
| member-alias-600 | 70.3 | 68.8–71.5 | 18280 | 624 | 1.855 |
| member-alias-2400 | 286.8 | 281.7–291.6 | 55296 | 2424 | 1.663 |
| member-alias-9600 | 2470.1 | 2262.0–2653.3 | 202836 | 9624 | 1.595 |
| function-result-600 | 153.2 | 151.2–158.2 | 29616 | 27705 | 1.774 |
| function-result-2400 | 661.0 | 657.5–667.3 | 100184 | 110505 | 1.740 |
| function-result-9600 | 6102.4 | 5862.4–6621.6 | 383140 | 441705 | 1.677 |
| fixed-head-pack-600 | 68.1 | 66.1–107.8 | 18480 | 39705 | 1.674 |
| fixed-head-pack-2400 | 265.2 | 260.9–291.2 | 54824 | 158505 | 1.725 |
| fixed-head-pack-9600 | 2242.8 | 2050.9–2481.8 | 203112 | 633705 | 1.691 |
| empty-tail-ordering-600 | 66.5 | 65.5–71.2 | 17696 | 33705 | 1.696 |
| empty-tail-ordering-2400 | 263.2 | 260.9–270.1 | 52940 | 134505 | 1.711 |
| empty-tail-ordering-9600 | 2032.3 | 2018.0–2509.3 | 190672 | 537705 | 1.686 |

The backend emits sectionless ELF. “Payload” is the existing harness's executable
payload proxy, which can include static data; it is not falsely reported as
an exact native `.text` section. Shared outputs have identical payload bytes.
Full native timing ranges and A/A/ABBA records for the short frontend executions
are in the raw appendix, produced by
[its hash-checking runner](../student.tests/pa19/frontend_runtime91.py).

| Executed loop workload | Compile A → B ms | RSS A → B KiB | Runtime A → B ms | Runtime B range | Runtime A/A range | Paired runtime B/A (range) | Payload bytes |
|---|---:|---:|---:|---:|---:|---:|---:|
| runtime-calls | 11.3 → 11.0 | 6024 → 6068 | 220.6 → 215.6 | 181.3–227.3 | 205.0–231.5 | 0.982 (0.938–1.025) | 206 |
| runtime-memory | 12.1 → 11.3 | 5976 → 6028 | 129.0 → 123.7 | 118.0–162.6 | 104.7–114.9 | 0.967 (0.902–1.092) | 434 |
| runtime-floating | 12.7 → 11.5 | 6056 → 6196 | 136.4 → 137.5 | 123.3–145.8 | 123.5–135.7 | 1.002 (0.948–1.048) | 230 |
| runtime-reference-cast | 13.3 | 6028 | 77.8 | 74.7–79.6 | final only | final only | 216 |

Reference-cast runtime validates identity while a conversion operator must stay
uninvoked; its 8,000,000 iterations end with the checked value 4,608. Entry
behavior is incorrect, so it is not included in speed comparisons. The three
shared runtime paired medians 0.982, 0.967 and 1.002 have overlapping spread and
identical native bytes: there is no claimed generated-code improvement.

## Work bounds, scaling and acceptance

Canonical indexed name/type identities and complete substitution/ordering keys
remain the owners. The new class-pack and ordering projections visit the actual
argument sequences; elaborated lookup visits lexical parents; ADL visits its
associated edges. There is no new whole-program scan, retry, persistent cache,
optional transformation, or higher-order optimization growth allowance.

The largest new cases show 7.7–9.2× latency for 4× declarations; this is recorded,
not treated as proof of linear wall time. Function-result counters at 2,400 →
9,600 declarations are: nodes 549,810 → 2,198,610; class completions 7,200 →
28,800; body transitions 2,400 → 9,600; expansion work 7,211 → 28,811;
lookup work 48,069 → 192,069. Peak RSS grows 3.82×. Member-alias nodes and class
completions similarly grow 4×. These observed work counts and the changed loops
show no Cartesian-product work introduced by this group; they do not identify
all causes of the latency change.

A task-clock profile of the largest function-result input (29,316 samples)
placed 23.59% in existing `IdIndex::get`, 6.34% in `put`, 4.29% in
`Ast::project_view`, 3.81% in type interning and 2.70% in argument interning.
The existing index uses a mixed hash, half-full capacity and geometric growth.
This diagnostic motivates the shared ordinary-member control above; it does not
establish an asymptotic bound or waive a defect. Kernel symbols were restricted.
Hardware profiling is not a required test dependency.

PA19/O0 has no mandated numerical compiler latency/RSS or text-growth ceiling.
The inherited [PA18 plan](../pa18/plan.md) already classifies its +15%, +16 MiB
and 5.5× thresholds as diagnostic targets under spec §9. Retain that
classification and all historical observations: none becomes an invented PA19
exit gate. Required semantics explain the new final-only workloads, while the
shared workloads provide no evidence of an avoidable regression attributable
to these edits. There are no optional optimizations whose benefit needs to buy
additional work or growth. Native optimization and self-hosting remain owned by
their later assignments. Coverage, correctness, all mandated limits and the
architecture requirements remain in force. The remaining implementation and
oracle-review groups in the plan are not excused by this performance result.
