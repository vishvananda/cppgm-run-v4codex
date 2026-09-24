# PA18/O0 performance evidence — handoff 64

Frozen entry `30a61006` and final tree `3f49ce3a` (implementation `e01b8763`).
[All observations](../student.tests/pa18/loop64-performance.json) retain compiler
hashes, flags, affinity, exact generated sources, telemetry and native hashes.
[The harness](../student.tests/pa18/benchmark64.py) uses one warmup per binary,
four A/A samples and four wall-time ABBA blocks. Newly accepted programs have
six final-only samples: the entry compiler rejects them. Compilation and
execution are separate; `/usr/bin/time` records compiler peak RSS. Validation
and telemetry occur outside timing. No competing build/test ran during timing.

| Compiler workload | Entry ms | Final ms | Paired B/A median (range) | Peak RSS KiB A / B |
|---|---:|---:|---:|---:|
| ordering-600 | 54.61 | 54.96 | 1.004 (0.998–1.008) | 15132 / 15208 |
| explicit-discard-600 | — | 47.17 | new behavior | — / 13948 |
| incomplete-size-600 | — | 41.66 | new behavior | — / 12848 |
| ordering-2400 | 211.23 | 213.50 | 1.010 (1.000–1.013) | 43320 / 43428 |
| explicit-discard-2400 | — | 180.70 | new behavior | — / 38228 |
| incomplete-size-2400 | — | 154.78 | new behavior | — / 33492 |
| common-loop-float-1500 | 154.04 | 155.07 | 1.014 (1.005–1.316) | 30188 / 30240 |
| runtime-calls | 6.27 | 6.00 | 0.964 (0.958–0.970) | 5824 / 5948 |
| runtime-memory | 6.29 | 6.15 | 0.966 (0.961–0.984) | 5716 / 5760 |
| runtime-floating | 6.12 | 5.94 | 0.966 (0.931–0.969) | 5932 / 6076 |
| runtime-new-substitution | — | 6.21 | new behavior | — / 5816 |

Final startup median is 5.37 ms. Scaled template and loop/float workloads
dominate startup; tiny runtime-source compilation timings are diagnostic.
The larger common workloads show small paired increases (0.4–1.4%), not a
compiler-speedup claim. The retained loop/float B sample at 267 ms produces a
1.316 block ratio; ordering-2400 A/A includes a 237 ms outlier. A/A ranges are
54.73–55.07 ms (ordering-600), 211.05–237.06 ms (ordering-2400), and
151.59–153.89 ms (loop/float). Required probing, type validity and dependency
state add work; there is no optional transform to justify by a runtime benefit.
The common workloads create zero completion edges; completed lookup does not
traverse a dependency graph. No avoidable global retry/cache clear is added.

| Executable workload | Entry seconds | Final seconds | Paired B/A median (range) | Payload bytes A / B |
|---|---:|---:|---:|---:|
| runtime-calls | 0.7165 | 0.7179 | 1.006 (1.001–1.021) | 206 / 206 |
| runtime-memory | 0.4191 | 0.4184 | 0.998 (0.990–1.001) | 434 / 434 |
| runtime-floating | 0.4950 | 0.4950 | 1.000 (1.000–1.001) | 230 / 230 |
| runtime-new-substitution | — | 0.2253 | new behavior | — / 177 |

Common LowIR and executables are byte-identical, including size. These runtime
variations are measurement spread, not changed generated code. Runtime A/A
ranges are 0.7142–0.7195 s (calls), 0.4189–0.4237 s (memory), and
0.4938–0.4946 s (floating). Volatile iteration bounds and checked results ensure
loops, calls, memory and floating-point work execute. New substitution runtime
is 0.2253 s median (0.2244–0.2257 s), 177 payload bytes. The supplied backend
writes sectionless ELF; payload after entry is the available text/alignment
proxy for these programs with no static data, not native-backend optimization.

600→2400 scaling is 3.89× time / 2.86× RSS for common ordering, 3.83× / 2.74×
for explicit discard, and 3.72× / 2.61× for incomplete-size substitution.
Query work grows 2404→9604 (discard) and 1204→4804 (size). Incomplete-size
completion edges grow exactly 600→2400. Separate
[completion scaling controls](../student.tests/pa18/completion_scaling.py)
record 32/128/512 pending classes and exactly one invalidation when one class
completes. A nested `sizeof` result records two edges/two invalidations;
these profiles and source inputs are in [validation evidence](../student.tests/pa18/loop64-evidence.json).

Work/storage bounds: visit required candidate/type/query edges, intern typed
keys, and retain only observed incomplete-dependency edges. Completed lookups
are O(1) average. Completion traverses affected reverse edges with NotStarted
as the deduplication state, then consumers retry only on demand. No ordinary
query without incomplete dependencies allocates reverse edges. Canonical facts,
flat indexes and edge vectors belong to the TU; candidate guards and worklists
release on return. No optimization iterations, speculative code growth or
function-body demand are introduced for candidate probing.

Compiler `.text` grows 1,826,630→1,831,302 bytes (+4,672, 0.26%). Common emitted
code growth is zero. PA18/O0 mandates no numerical latency/RSS/runtime ceiling;
this is necessary semantic work, not an optional optimization. Prior PA17
self-selected thresholds remain diagnostics under spec §9. All prior PA18
measurements remain in [performance.md](performance.md); no mandated limit,
correctness check, or coverage was weakened. Native optimization and self-host
benchmarks remain owned by PA24–PA34.
