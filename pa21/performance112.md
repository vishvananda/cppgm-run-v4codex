# PA21/O0 performance evidence 112

[Driver](../student.tests/pa21/benchmark112.py) and [all observations](../student.tests/pa21/performance112.json). Entry `9457e2fc`; implementation `e32e9f2f`.
Frozen binaries are `/tmp/pa21-112/compiler-A` and `compiler-B`; full hashes, input sources/hashes, flags, backend hashes and host identity are in the evidence. Each workload has one warmup per binary, four A/A observations and four ABBA blocks, pinned to one available CPU. Compilation and checked executable execution are timed separately with wall time and peak RSS. Instrumented/validated output is checked against timed output.

| Workload | Compiler median ms A → B | Peak RSS KiB A → B | Runtime median ms A → B | Native text bytes A → B |
|---|---:|---:|---:|---:|
| startup | 16.061 → 16.968 | 5760 → 6052 | 6.469 → 7.063 | 24 → 24 |
| auto-specializations-9600 | 857.981 → 949.733 | 108104 → 108232 | 3.718 → 3.776 | 768062 → 768062 |
| runtime-calls | 8.540 → 8.153 | 6216 → 6288 | 192.756 → 193.306 | 206 → 206 |
| runtime-memory | 9.677 → 9.461 | 5996 → 6272 | 115.452 → 112.700 | 434 → 434 |
| runtime-floating | 9.932 → 9.577 | 6276 → 6232 | 107.367 → 151.617 | 230 → 230 |
| catch-functions-512 | 132.969 → 133.010 | 18760 → 18916 | 14.480 → 13.871 | 212400 → 212400 |
| catch-functions-2048 | 342.066 → 312.234 | 56456 → 56828 | 5.334 → 5.285 | 848304 → 848304 |
| runtime-handlers | 6.794 → 6.620 | 6024 → 6208 | 308.275 → 308.698 | 832 → 832 |

The five common workloads have byte-identical LowIR **and executable binaries**. The apparent floating runtime increase (107.367 → 151.617 ms) is therefore timing interference, not changed machine work; it is retained, not discarded. The 9600-specialization compiler medians differ by +10.7%, but their ranges overlap (A 715–1203 ms, B 717–1204 ms) and this path never constructs a source exception context. There is no general compiler or runtime speedup claim.

For the affected 512-function workload, compiler paired B/A ratios are 0.995, 1.005, 0.957 and 1.222; wall ranges are 131–275 ms and 122–264 ms. At 2048 functions they are 0.906, 1.444, 0.796 and 0.923; wall ranges are 302–551 ms and 301–697 ms, with an A/A range of 519–1057 ms. These observations are too noisy to establish a latency change. Peak RSS differs by +156 and +372 KiB, respectively.

The runtime-handler benchmark executes 120000 iterations using a volatile bound. Each iteration throws through two nested handlers and verifies the result and destruction trace. Runtime paired B/A ratios are 1.0083, 0.9969, 0.9989 and 1.0000; ranges are A 307.08–310.75 ms and B 307.01–313.10 ms, against A/A 310.73–335.79 ms. It demonstrates preserved execution, not a speedup. The larger compiler workloads execute only two functions as correctness controls; their short runtimes and the startup probe are not profitability evidence.

Native sizes for hosted handler workloads are ELF `.text`, excluding the shared exception runtime. The unchanged freestanding common workloads use the established sectionless ELF payload proxy. All checked exits are zero; output hashes and every timing observation are retained.

## Work, storage and stage acceptance

Compiler `.text` increases **704 bytes**, from 2223942 to 2224646 (**0.0317%**). The implementation adds one boolean dispatch summary to each function-owned exception-context record. It computes that summary once from the immediate parent and scans each checked source handler once to establish exhaustiveness. Additional work is O(handlers + contexts); there is no ancestor rescan, candidate search, syntax replay, cache invalidation, new ownership graph, code cloning or optional optimization pass. Context storage is reset by the existing function-boundary owner.

At 512/2048 functions, typed LowIR instruction counts change from 48676/194596 to 48164/192548: exactly **one dead instruction per function**. Source/semantic/full-expression work and protected-region counts are unchanged. The one-function runtime case changes 132 → 131 instructions. Native text does not change. This is the required O0 dispatch convention, not a runtime optimization justified by smaller IR. The automatic-array reference correction changes no compiler work or materialization policy; the inherited PA16 owner already implements it.

PA21 sets no numeric latency, RSS or text ceiling. Apply spec §9 to PA21/O0: preserve semantic/LowIR correctness, linear added work and bounded storage, and remove avoidable optional costs. No optional executable transform was added. Historical +15%, +16 MiB and 5.5× self-selected diagnostics remain evidence, not extra exit gates; all mandated limits and coverage remain binding. [Performance 111](performance111.md), [110](performance110.md) and earlier reports are preserved. Student native optimization, debug encoding and self-hosting remain later-stage owners.
