# PA15 execution, body validation and storage evidence

Frozen implementation: `5452ad74`; entry: `d705aafc`. Full observations, binary/input/output hashes, flags, CPU, phase/work telemetry and timestamps are retained in [execution-performance.json](../student.tests/pa15/execution-performance.json). The [harness](../student.tests/pa15/execution_benchmark.py) uses one warmup per binary, four A/A samples and two ABBA blocks; newly supported workloads use six B samples because A rejects or omits required storage. There are 224 checked invocations including warmups. No preliminary observations were discarded.

At PA15/O0 these changes establish required semantic facts; they introduce no optional optimization. There is no mandated numeric compiler-latency, RSS or text ceiling. The evaluator permits 512 active calls and 1,000,000 expression visits per root. Limit exhaustion and missing definitions leave pending results, while completed scalar facts remain reusable. The 512-call bound matches the recommended minimum in N3485 Annex B (doc/n3485.txt, line 66685); the expression-work cap is an explicit implementation limit, not a performance gate. The inherited 32-byte local-array backing policy remains in force.

Compiler text grows from 1,512,070 to 1,528,070 bytes (+16,000; 1.06%). New caches use translation-unit flat indexes and geometrically grown vectors; expression values are keyed by activation and node, activations by checked body, canonical scalar arguments and receiver identity. All release with the translation unit. No process-global cache, source replay, IR roundtrip or optional code-growth transform was added.

## Compiler measurements

Medians include wall time; peak RSS is the maximum of measured samples. Paired ratios use only ABBA blocks. A dash means the entry compiler was not a correct implementation of this workload. Small runtime-source compilations are startup dominated and are not used to claim frontend speedups.

| Workload | A wall ms | B wall ms | A/B peak RSS KiB | Paired B/A |
|---|---:|---:|---:|---|
| ordinary-1000 | 94.95 | 95.22 | 20164 / 20044 | 0.999, 0.990 |
| dormant-1000 | 41.15 | 46.20 | 10676 / 12028 | 1.128, 1.118 |
| template-1000 | 60.79 | 61.11 | 15352 / 15284 | 1.002, 1.008 |
| calls-1000 | — | 35.86 | — / 9200 | — |
| receivers-1000 | — | 76.12 | — / 16100 | — |
| storage-1000 | — | 72.56 | — / 15728 | — |
| ordinary-4000 | 376.14 | 374.88 | 64344 / 64672 | 0.983, 0.999 |
| dormant-4000 | 149.32 | 171.41 | 26636 / 31640 | 1.156, 0.938 |
| template-4000 | 237.17 | 237.41 | 45140 / 45092 | 0.989, 1.014 |
| calls-4000 | — | 125.81 | — / 20764 | — |
| receivers-4000 | — | 309.15 | — / 48756 | — |
| storage-4000 | — | 291.71 | — / 46284 | — |
| runtime-calls | 5.86 | 5.92 | 5484 / 5544 | 1.002, 1.008 |
| runtime-memory | 6.15 | 6.00 | 5472 / 5492 | 1.008, 1.001 |
| runtime-floating | 6.00 | 6.04 | 5704 / 5660 | 1.006, 1.022 |
| runtime-constant | — | 6.30 | — / 5452 | — |

The common ordinary/template corpora produce byte-identical LowIR. At N=4,000 their paired compiler ratios are 0.983–0.999 and 0.989–1.014. A/A wall ranges are 371.3–378.0 ms and 235.1–239.2 ms. These observations support no material regression or speedup claim.

Dormant ordinary bodies now undergo mandatory complete-class and static-assert checking: N=4,000 adds exactly 4,000 checked bodies, about 22.1 ms median and 5,004 KiB peak RSS. The first paired ratio is 1.156; the second is 0.938 because one A observation took 216.2 ms, outside its 147.3–149.9 ms A/A range. That outlier is retained. The median and first block show the necessary validation cost; the second block is not evidence of a speedup. Bodies are checked once and emitted only under the existing demand policy. There is no avoidable optional transform to remove.

For newly correct paths, N=1,000 -> 4,000 scales call evaluation from 35.9 -> 125.8 ms, receivers from 76.1 -> 309.2 ms, and storage from 72.6 -> 291.7 ms. Scalar calls have one checked body, N activations, 5N expression visits and N memo hits. Receiver specializations have N checked bodies, N activations and N visits. Storage has zero checked member bodies and emits exactly 2N globals versus the entry compiler’s incomplete N. These counters establish work proportional to actual declarations, activations and produced storage.

## Executable measurements

Native controls use the pinned supplied backend at O0, as permitted for PA15. Volatile iteration counts and checked sums prevent dead or folded workloads; each sample exits zero. The common executables are byte-identical. Text counts the sectionless ELF payload after entry; these native workloads have no static data.

| Workload | A/B median runtime ms | A/B text bytes | Paired B/A |
|---|---:|---:|---|
| runtime-calls | 300.99 / 301.61 | 206 / 206 | 0.996, 1.011 |
| runtime-memory | 177.25 / 177.16 | 434 / 434 | 0.999, 0.998 |
| runtime-floating | 210.65 / 208.95 | 230 / 230 | 0.981, 0.993 |
| runtime-constant | — / 419.31 | — / 242 | — |

All runtime samples report 256 KiB peak RSS. Common runtime ratios span 0.981–1.011 with identical machine bytes; there is no claimed runtime optimization. The new constant-initializer executable is measured only for B because A rejects it. Native allocation/encoding, optimized code and self-hosting remain later-stage responsibilities. Historical [initialization](initialization-performance.md), [matching](matching-performance.md) and [audit](audit-performance.md) evidence stays preserved; their diagnostic targets do not override current-stage acceptance.
