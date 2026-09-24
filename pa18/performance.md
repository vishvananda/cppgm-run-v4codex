# PA18/O0 performance evidence — handoff 63

Frozen entry `94dcb8ad` and final implementation `4a49ea10`; compiler hashes,
flags, CPU affinity, complete generated inputs, all observations, checked
outcomes and telemetry are in [final observations](../student.tests/pa18/loop63-performance.json).
The harness is [benchmark.py](../student.tests/pa18/benchmark.py). One warmup
per binary precedes four A/A samples and four ABBA blocks. Newly accepted
behavior has six final-only samples because the entry compiler rejects it.
Compilation and execution are measured separately; external `/usr/bin/time`
records peak RSS. Telemetry/validation runs are outside the timed samples.
Every common LowIR output and executable is byte-identical across A/B.

| Compiler workload | Entry ms | Final ms | Paired B/A median (range) | Peak RSS KiB A / B |
|---|---:|---:|---:|---:|
| ordering-600 | 55.13 | 54.32 | 0.989 (0.980–1.013) | 15040 / 15160 |
| nested-cv-600 | — | 55.95 | new behavior | — / 15256 |
| pack-result-600 | — | 127.27 | new behavior | — / 25416 |
| ordering-2400 | 218.31 | 214.26 | 0.984 (0.973–0.991) | 43200 / 43356 |
| nested-cv-2400 | — | 217.30 | new behavior | — / 43012 |
| pack-result-2400 | — | 519.64 | new behavior | — / 85040 |
| common-loop-float-1500 | 154.64 | 154.33 | 0.995 (0.991–1.003) | 30200 / 30192 |
| runtime-calls | 6.55 | 6.52 | 0.983 (0.965–1.018) | 5888 / 5872 |
| runtime-memory | 6.20 | 6.15 | 0.980 (0.955–1.027) | 5944 / 5980 |
| runtime-floating | 5.62 | 5.62 | 0.998 (0.997–1.009) | 5912 / 6060 |
| runtime-new-ordering | — | 6.13 | new behavior | — / 5956 |

Final startup median is 5.48 ms. The six scaled
template workloads and the loop/float compiler control dominate startup;
compilation timings for the tiny runtime sources are diagnostic only.
A/A compiler ranges (ms) are ordering-600: 55.39–55.88, ordering-2400: 211.26–219.09, common-loop-float-1500: 152.58–164.18.
The final common compiler samples show no repeatable regression. These are
semantic correctness changes, not generated-code speedup claims.

| Executable workload | Entry seconds | Final seconds | Paired B/A median (range) | Final payload bytes |
|---|---:|---:|---:|---:|
| runtime-calls | 0.7152 | 0.7165 | 1.000 (1.000–1.005) | 206 |
| runtime-memory | 0.4227 | 0.4203 | 0.986 (0.958–1.007) | 434 |
| runtime-floating | 0.4957 | 0.4958 | 1.002 (0.989–1.006) | 230 |
| runtime-new-ordering | — | 0.2291 | new behavior | 173 |

Runtime inputs use volatile iteration counts and checked results; loops,
calls, memory and floating-point work execute. Common executables have
identical hashes and payload sizes: runtime differences reflect measurement
noise, not changed code. The memory run includes a 0.636 s final outlier
(median 0.420 s); it is retained. A/A runtime ranges are 0.713–0.747 s
(calls), 0.418–0.434 s (memory), and 0.494–0.505 s (floating).
The new ordering program runs in 0.2291 s median (0.2251–0.2394 s) with
173 payload bytes; the entry rejection prevents an equivalent A/B comparison.
The supplied backend writes sectionless ELF: payload after entry includes
code/alignment; these inputs have no static data. This is explicitly the
available text-size proxy, not a native-backend optimization claim.

600→2400 specialization scaling is 3.95× compiler time / 2.86× RSS for
common ordering, 3.88× / 2.82× for new cv ordering, and 4.08× / 3.35×
for pack results. Ordering telemetry reports one computed comparison and
1199/4799 cache hits; pack query work grows from 2409 to 9609. Completed
cache probes precede shape construction and have O(1) average cost. Misses
visit compared type occurrences/used-parameter edges; expansion visits each
required lane. The cache is bounded by demanded comparison keys and the TU
lifetime. No optimization iterations or speculative code growth are added.

Compiler `.text` grows from 1,814,470 to 1,826,630 bytes
(+12,160, 0.67%). This is required semantic/ABI machinery;
common generated code has zero growth. PA18 is O0 LowIR and mandates no
numerical latency/RSS/runtime ceiling. The inherited PA17 numeric targets
remain diagnostics under spec §9; no mandated limit, work bound or coverage
has been weakened. Native selection/allocation/ELF optimization and self-host
evaluation belong to PA24–PA34.

Historical evidence is preserved: [attempt 1](../student.tests/pa18/loop63-performance-attempt1.json)
stopped on the pack-result scaling failure; the final-only preflight error
led to the EntityId/TypeId lane fix. [Before cache placement](../student.tests/pa18/loop63-performance-before-cache.json)
contains a complete earlier run. The final rerun follows the completed-fact
lookup correction, using the same frozen inputs and protocol. Historical
measurements are retained without making an invalid entry implementation
the performance standard for newly required behavior.
