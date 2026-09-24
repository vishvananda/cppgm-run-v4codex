# PA17 storage performance — loop 59

Frozen entry `7cc89281` versus implementation `6a1a51a8` (campaign commit
`3c7eeea2`). The [complete observations](../student.tests/pa17/storage-performance.json)
retain every source, hash, warmup, sample, A/A range, paired spread and telemetry.

Host compiler flags: g++ `-std=gnu++11 -Wall -O3`, course runner enabled.
Student flags: `--emit-lowir -O0`; CPU affinity 31. Serial measurements ran after
builds and test campaigns completed. One warmup per binary, four A/A samples,
then four ABBA blocks; new accepted inputs have six final-only samples. Entry
rejection is recorded and is never used as a speed baseline. Validator, telemetry,
supplied native backend and correctness execution run outside compilation timing.

Common sources have identical LowIR/native hashes except `static-reads` and
`runtime-static-read`: those explicitly compare equivalent checked outputs with
the changed O0 load policy. Both scaled static-read/signature executables check
the sum of every generated function. Runtime loops retain volatile bounds and
checked live sums. Tiny runtime-source compiler timings remain startup-dominated
cost observations, not speed claims. The eight common frontend workloads have
67–604 ms final medians; the two newly accepted workloads take 67/268 ms.

Compiler `.text` grows **2,752 bytes**, 1,786,182 → 1,788,934. Entity records stay
120 bytes. Maximum observed additional peak RSS is **624 KiB**. Common paired
compiler medians range **0.9696–1.0246**; the eight frontend medians range
**0.9867–1.0236**. No optimization benefit is claimed.

| Workload | Final compiler median, s | Paired B/A median (block range) | Final peak RSS, KiB |
|---|---:|---:|---:|
| common-partials-1500 | 0.1131 | 0.9867 (0.9569–1.0186) | 22,652 |
| common-partials-6000 | 0.4518 | 1.0080 (0.9949–1.0560) | 73,104 |
| common-loop-float-1500 | 0.1529 | 1.0026 (0.9974–1.0055) | 30,332 |
| common-loop-float-6000 | 0.6040 | 1.0198 (0.9968–1.1646) | 104,032 |
| static-reads-1000 | 0.0799 | 1.0167 (1.0000–1.0338) | 18,308 |
| static-reads-4000 | 0.3122 | 1.0168 (0.9852–1.0187) | 55,848 |
| static-signatures-400 | 0.0673 | 1.0213 (1.0124–1.0275) | 14,640 |
| static-signatures-1600 | 0.2659 | 1.0236 (0.9799–1.0274) | 40,936 |
| local-address-600 | 0.0671 | final only (—) | 15,604 |
| local-address-2400 | 0.2679 | final only (—) | 45,580 |
| runtime-calls | 0.0116 | 1.0246 (0.3501–1.0649) | 5,768 |
| runtime-memory | 0.0104 | 0.9696 (0.8029–1.0215) | 5,768 |
| runtime-floating | 0.0099 | 0.9857 (0.9415–1.0128) | 6,004 |
| runtime-static-read | 0.0105 | 0.9793 (0.9375–1.0712) | 5,844 |

| Executable workload | Paired runtime B/A median (block range) | Code + alignment A → B, bytes |
|---|---:|---:|
| runtime-calls | 1.0061 (0.9753–1.0425) | 206 → 206 |
| runtime-memory | 1.0072 (0.9797–1.0204) | 434 → 434 |
| runtime-floating | 0.9859 (0.9682–1.0100) | 230 → 230 |
| runtime-static-read | 1.0206 (1.0120–1.0454) | 188 → 196 |

The supplied O0 backend emits sectionless ELF. Code size is executable payload
minus typed global data, including alignment; JSON preserves all component/file
sizes. The three unchanged runtime binaries are byte-identical. The static-read
runtime adds one eight-byte load encoding in `read`, with a **1.0206** paired
median and 1.0120–1.0454 block range. This is the disclosed cost of preserving the
course's ordinary O0 read boundary while retaining instantiated constant reads;
it is not an optional optimization or a profit claim. Its result is unchanged.

At 4× static-read scale, definition applications/edges and initialization facts
increase 1000 → 4000; the single shared source signature is checked exactly once.
At 4× signature scale, signature requests/work increase 400 → 1600, applications
400 → 1600, and query computations 1602 → 6402. At 4× new local-table scale,
initialization facts increase 600 → 2400 and plan facts 1200 → 4800. Latency
scales 3.91×, 3.95× and 3.99× respectively. Source, fact and output growth track
demanded declarations; no global retry, grammar replay or optional transform is
introduced. Signature-context normalization is memoized over typed query edges.

PA17/O0 has no mandated numeric performance ceiling. Historical +15%, +16 MiB
and 5.5× self-selected targets remain diagnostic under spec.md §9, with their
original observations preserved. The acceptance budget is linear demanded work,
one fact application per complete owner/key, no optional generated-code growth,
and disclosed semantic/contract costs. These observations meet that stage scope.
Self-hosting and the compiler's own native backend remain later-stage evidence;
the external backend here is only the authorized validation adapter.
