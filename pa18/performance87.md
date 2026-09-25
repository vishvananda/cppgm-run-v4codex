# PA18 implementation 87 performance evidence

Acceptance: **PA18/O0 LowIR**, spec §9. Frozen entry `01b47c20` is compared with
final committed implementation `7000a4de`. [Raw measurements](../student.tests/pa18/loop87-performance-complete.json)
retain **438 observations across 17 workloads**, all sources,
binary/input/backend hashes, flags, affinity, preflight telemetry, checked
execution results, warmups and timing observations. [The harness](../student.tests/pa18/benchmark87.py)
and [table renderer](../student.tests/pa18/report87.py) reproduce this record.
Historical evidence, including [audit 86](performance86.md), the first 438 observations in [the initial run](../student.tests/pa18/loop87-performance.json), and 438 observations in [the query/default repair run](../student.tests/pa18/loop87-performance-final.json), remains unchanged. This checkpoint retains **1,314 observations** in total.

Each comparable workload uses one warmup per binary, four A/A observations,
then four wall-time ABBA blocks. Rejecting or incorrect entry implementations
receive six final-only samples. Timing uses `/usr/bin/time` peak RSS and separate
compiler/executable invocations. Telemetry is collected during preflight only.
No throughput tests ran during this benchmark. The native harness boundary is
`lowir2native-ref -O0`, bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
Every native preflight and timed execution checks its result. Runtime loops use
volatile bounds and checked sums/copy/destruction counts. No optional optimizer
was added, and no runtime speedup is claimed.

Times are median milliseconds, paired ratios include the full block spread,
RSS is maximum KiB. Compiler `.text` is measured directly; the sectionless
reference ELF's **loadable payload** includes code/data and is not isolated
`.text`. Startup-sized compiler rows are diagnostics. Compiler startup medians:
**10.28 / 10.42 ms**.

| Workload | Compiler A / B ms | Paired B/A (range) | A / B KiB | A / B payload bytes |
|---|---:|---:|---:|---:|
| ordering-600 | 136.59 / 138.33 | 0.950 (0.565–1.054) | 15232.00 / 15240.00 | 40842 / 40842 |
| ordering-2400 | 484.37 / 484.34 | 1.023 (0.893–1.033) | 43388.00 / 43460.00 | 163242 / 163242 |
| common-loop-float-1500 | 339.03 / 331.40 | 0.987 (0.918–1.033) | 30364.00 / 30500.00 | — / — |
| runtime-calls | 10.43 / 10.69 | 1.086 (0.990–1.473) | 5960.00 / 6036.00 | 206 / 206 |
| runtime-memory | 6.71 / 6.62 | 0.982 (0.974–1.146) | 5884.00 / 6052.00 | 434 / 434 |
| runtime-floating | 6.66 / 6.48 | 0.969 (0.801–1.001) | 6072.00 / 6108.00 | 230 / 230 |
| ellipsis-lvalue-150 | — / 18.95 | final only | — / 7960.00 | — / 8515 |
| ellipsis-fixed-150 | 14.48 / 15.02 | 1.045 (0.953–1.086) | 7196.00 / 7576.00 | 7324 / 10324 |
| ellipsis-query-150 | — / 14.75 | final only | — / 7164.00 | — / 5797 |
| alias-result-150 | 40.23 / 40.14 | 1.006 (0.925–1.316) | 10916.00 / 11044.00 | 13299 / 13299 |
| ellipsis-lvalue-600 | — / 57.70 | final only | — / 13528.00 | — / 33715 |
| ellipsis-fixed-600 | 38.29 / 41.71 | 1.061 (0.751–1.133) | 10988.00 / 11808.00 | 28924 / 40924 |
| ellipsis-query-600 | — / 49.66 | final only | — / 11032.00 | — / 22897 |
| alias-result-600 | 148.50 / 146.32 | 0.974 (0.866–0.994) | 26060.00 / 26228.00 | 52899 / 52899 |
| runtime-ellipsis-prvalue | 7.21 / 6.54 | 0.971 (0.823–1.090) | 6024.00 / 6148.00 | 208 / 208 |
| runtime-ellipsis-copy | — / 6.77 | final only | — / 6056.00 | — / 1232 |
| runtime-alias-result | 6.64 / 6.64 | 0.992 (0.932–1.023) | 5984.00 / 6152.00 | 212 / 212 |

Compiler `.text`: **1,984,518 → 1,991,942 bytes**, **+7,424 (0.374%)**.

| Workload | Runtime A / B ms | Paired B/A (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 1337.65 / 1354.73 | 1.018 (0.973–1.093) | 206 / 206 |
| runtime-memory | 452.73 / 475.97 | 1.020 (0.919–1.030) | 434 / 434 |
| runtime-floating | 509.42 / 509.41 | 1.002 (0.998–1.113) | 230 / 230 |
| runtime-ellipsis-prvalue | 56.10 / 51.96 | 0.912 (0.864–0.986) | 208 / 208 |
| runtime-ellipsis-copy | — / 72.95 | final only | — / 1232 |
| runtime-alias-result | 52.75 / 50.62 | 0.968 (0.869–1.012) | 212 / 212 |

## Work, regressions and stage acceptance

All unaffected ordering, loop/floating, calls, memory, alias-result and prvalue
ellipsis workloads retain byte-identical LowIR and native bytes. The raw record
retains scheduler outliers and the full paired ranges. There is no supported
speedup claim from these noisy ratios. Their semantic work counters and output
sizes are unchanged; alias result classification introduces no extra work.

Fixed class ellipsis now makes the source-required private argument copy. At
600 specializations its compiler ratio, memory and payload growth appear in the
table above. This is the selected transfer/materialization cost, not
an optional optimization. Passing the source object's address happened to work
for the old benchmark's unused argument, but does not provide a general value
boundary. The final result is separately checked. Per-use storage and `copyobj`
growth follow actual arguments, without helper/body cloning. New lvalue and
nontrivial-copy workloads have no correct general entry comparison; the query
baseline computes a wrong `noexcept` value. Their rejection/wrong-result records
are retained and their final costs are reported rather than called speedups.

The affected executable timings are about 51–73 ms on checked multi-million-iteration
loops; unaffected call/memory/floating loops run 0.45–1.36 seconds. Final-only copy
timing measures both construction and destruction. Native try/catch, hosted
aggregate varargs retrieval, native optimization, debug encoding and self-hosting
remain later-stage obligations, not additional PA18 exit gates.

| Family, 150 → 600 | Final work evidence |
|---|---|
| ellipsis-lvalue | candidates 607 → 2407; conversion objects 151 → 601; instructions 1666 → 6616 |
| ellipsis-fixed | candidates 160 → 610; conversion objects 152 → 602; instructions 1214 → 4814 |
| ellipsis-query | candidates 154 → 604; conversion objects 1 → 1; instructions 613 → 2413 |
| alias-result | candidates 3152 → 12602; conversion objects 300 → 1200; instructions 3313 → 13213 |

The fixed recipe keeps one selected call and adds a materialization per evaluated
use; the query shares its checked conversion recipe without a body demand.
Type, expression and substitution identities remain TU-owned and interned.
Existing constant-evaluation work/depth limits, bounded default and transfer
queries, memoized branch analysis and function-local emission remain unchanged.
No new pass, code cloning, global rescan or optional growth budget is introduced.

PA18 mandates no numeric compiler latency/RSS ceiling. Historical +15%, +16 MiB
and 5.5× targets remain diagnostic, as established by audit 86. This record
preserves necessary costs, all observations and correctness while showing work
proportional to consumed/emitted facts. There is no unprofitable optional
transform to remove. Stage-scoped performance acceptance is satisfied; the
independent full-stage audit remains required before advancement.
