# PA19 performance evidence 92

Frozen A is entry `065d6783`; B is implementation `5763cf6c` plus oracle commit `946c651b`. Both use `g++ -std=gnu++11 -Wall -O3`, the course test runner, and `--emit-lowir -O0`. Inputs, binary/backend hashes, CPU affinity, platform, complete harness source, checked exits, telemetry, warmups and every observation are in [performance92.json](../student.tests/pa19/performance92.json). The [runner](../student.tests/pa19/benchmark92.py) uses the inherited PA10 timing/ELF helpers. No production phase calls the supplied backend; it executes outputs only.

A SHA-256 `d65826dd89e27b465ac9cf8bd61db6de565c5913b24856c5740345fbb73c6b02`; B `1a181c95ee97d646d41c1009780200049e776d67e1d715840d5526b3d10e1d40`. Compiler `.text` grows 2,004,358 → 2,005,958 bytes: **+1,600 bytes (0.080%)**.

One warmup per binary, four A/A observations, then four wall-time ABBA blocks per comparable workload. Newly supported inputs use six B observations; entry rejections are preserved and excluded from speed comparisons. Compilation and native execution are measured separately. Every output passes LowIR validation and its native result check before measurement. All nine comparable workloads have exact LowIR and native-byte equality. Seven workloads have newly correct final-only behavior.

The completed run contains **16 workloads, 464 observations and 52 warmups**, including startup calibration. The [initial attempt](../student.tests/pa19/performance92-initial.json) retains 60 observations and 6 warmups plus its original harness: generated `>>==` tokenized as `>>=` and `=`, so the final compiler correctly rejected the malformed input. The repaired generator inserts whitespace. No old measurement was replaced; the first attempt's timing overlapped final validation and is not used for acceptance.

## Compiler latency and memory

Times are milliseconds; RSS is peak KiB. Paired entries give median B/A and the full block-ratio range. Absolute medians can differ from paired ratios when the machine changes speed within a run. All individual observations remain available; no outlier was removed.

| Workload | Compile A → B ms | B range | A/A range | Paired B/A (range) | RSS A → B KiB |
|---|---:|---:|---:|---:|---:|
| ordinary-constant-600 | 146.3 → 136.3 | 122.5–184.2 | 139.2–192.1 | 0.934 (0.904–0.945) | 12840 → 13036 |
| namespace-variable-600 | 147.5 → 147.8 | 138.5–203.2 | 100.7–157.2 | 1.040 (0.953–1.151) | 10308 → 10428 |
| member-variable-600 | — → 265.4 | 209.7–363.5 | — | final only | — → 21396 |
| member-object-600 | — → 181.0 | 157.1–263.1 | — | final only | — → 16276 |
| ordinary-constant-2400 | 579.2 → 487.5 | 268.3–678.2 | 469.8–588.9 | 0.811 (0.647–0.952) | 33748 → 33872 |
| namespace-variable-2400 | 156.7 → 162.7 | 153.9–181.3 | 176.4–224.1 | 0.996 (0.936–1.169) | 24468 → 24852 |
| member-variable-2400 | — → 451.9 | 422.1–513.2 | — | final only | — → 67208 |
| member-object-2400 | — → 313.5 | 302.6–342.2 | — | final only | — → 46560 |
| ordinary-constant-9600 | 916.3 → 903.0 | 849.0–1165.9 | 939.4–1169.3 | 0.976 (0.921–1.124) | 115192 → 115384 |
| namespace-variable-9600 | 781.1 → 898.1 | 559.0–997.2 | 581.4–828.1 | 1.055 (1.029–1.183) | 80168 → 81804 |
| member-variable-9600 | — → 1787.6 | 1667.9–1884.6 | — | final only | — → 254784 |
| member-object-9600 | — → 1169.5 | 1151.4–1184.0 | — | final only | — → 170064 |
| runtime-calls | 6.6 → 6.3 | 6.1–6.5 | 7.1–7.2 | 0.953 (0.946–0.990) | 6028 → 6124 |
| runtime-memory | 6.7 → 6.5 | 6.3–6.9 | 6.7–7.2 | 0.966 (0.955–0.987) | 5960 → 6172 |
| runtime-floating | 6.5 → 6.3 | 6.2–6.5 | 6.4–6.6 | 0.973 (0.955–0.975) | 6116 → 6252 |
| runtime-member-object | — → 6.4 | 6.3–6.6 | — | final only | — → 6104 |

Ordinary and namespace controls vary substantially with machine speed: the 2,400 ordinary case spans 268–678 ms in B. No speedup is claimed. The largest namespace-variable workload has a 1.055 paired median, with all four blocks 1.029–1.183. Its measured semantic counters differ by exactly 9,600 additional type probes, initialization conversions and constant-conversion operations for 9,600 specializations. These are the new checked initialization path, required to reject invalid/deleted/explicit-only conversions and validate persistent literal values. Parsed nodes, query work, frame counts and output are unchanged. Peak RSS grows 1,636 KiB (2.04%). This is disclosed correctness work, with no new search, retry, optional transform or code-growth policy. The source-level bound is one additional check per initializer, not a claim inferred solely from timing.

## Generated execution and size

The supplied backend emits sectionless ELF. Payload below is the inherited executable-payload proxy, which may include static data; it is **not** a falsely claimed exact native `.text` section. Frontend stress programs execute an empty main after compile-time assertions: their runtime is startup-dominated and supplies no useful runtime speed comparison. The four loop workloads use volatile runtime bounds and checked results; member-object executes 8,000,000 iterations and checks 32,256.

| Workload | Runtime A → B ms | B range | A/A range | Paired B/A (range) | Payload bytes A → B |
|---|---:|---:|---:|---:|---:|
| ordinary-constant-600 | 5.8 → 5.6 | 5.2–11.1 | 5.9–9.6 | 1.149 (1.022–1.196) | 24 → 24 |
| namespace-variable-600 | 5.5 → 6.1 | 5.2–7.5 | 5.0–5.9 | 1.114 (0.994–1.192) | 24 → 24 |
| member-variable-600 | — → 7.8 | 7.1–8.4 | — | final only | — → 24 |
| member-object-600 | — → 5.8 | 4.8–7.4 | — | final only | — → 40 |
| ordinary-constant-2400 | 4.0 → 3.8 | 3.5–6.5 | 3.7–5.4 | 1.060 (0.626–1.274) | 24 → 24 |
| namespace-variable-2400 | 3.5 → 3.6 | 3.3–4.7 | 3.3–3.6 | 1.068 (0.975–1.157) | 24 → 24 |
| member-variable-2400 | — → 4.3 | 3.6–7.5 | — | final only | — → 24 |
| member-object-2400 | — → 4.1 | 3.6–7.7 | — | final only | — → 40 |
| ordinary-constant-9600 | 3.4 → 3.4 | 3.2–3.7 | 3.4–3.6 | 0.994 (0.913–1.006) | 24 → 24 |
| namespace-variable-9600 | 3.4 → 3.3 | 3.2–3.6 | 3.3–3.5 | 0.947 (0.891–1.012) | 24 → 24 |
| member-variable-9600 | — → 3.5 | 3.3–3.9 | — | final only | — → 24 |
| member-object-9600 | — → 3.2 | 3.1–3.5 | — | final only | — → 40 |
| runtime-calls | 124.9 → 124.9 | 123.4–197.8 | 124.1–188.4 | 1.007 (0.960–1.033) | 206 → 206 |
| runtime-memory | 73.1 → 73.2 | 72.5–73.9 | 72.8–79.7 | 0.998 (0.991–1.002) | 434 → 434 |
| runtime-floating | 86.4 → 86.4 | 85.8–90.6 | 85.8–86.8 | 0.998 (0.994–1.027) | 230 → 230 |
| runtime-member-object | — → 42.7 | 42.5–44.7 | — | final only | — → 192 |

Calls, memory and floating-loop paired runtime medians are 1.007, 0.998 and 0.998, with identical emitted bytes. There is no generated-code optimization claim. New member-object execution is final-only because A rejects its valid declaration.

## Complexity and stage acceptance

Member-variable stress has two assertions per specialization and one dormant invalid member initializer. At 600 → 2,400 → 9,600 keys, initializer work is exactly 600 → 2,400 → 9,600; type-query work 2,405 → 9,605 → 38,405; query edges 2,404 → 9,604 → 38,404; frames 6,000 → 24,000 → 96,000; nodes 97,287 → 388,887 → 1,555,287. The second use never reevaluates an initializer and the dormant member never does. B latency at 2,400 → 9,600 is 451.9 → 1,787.6 ms (3.96×), RSS 67,208 → 254,784 KiB (3.79×). Member-object initializer count likewise equals key count; latency 313.5 → 1,169.5 ms and RSS 46,560 → 170,064 KiB. These counters and the actual owner loops support linear added work in demanded facts and query edges, not all combinations of declarations.

PA19/O0 spec §9 acceptance passes: there is no mandated numerical latency/RSS or code-growth ceiling, no optional optimization, and no avoidable pass or global retry added by this group. The measured necessary conversion-validation cost is retained. The inherited +15%, +16 MiB and 5.5× targets remain diagnostics under the stage-scoped rule; historical measurements and misses are preserved in earlier reports. No mandated limit, correctness rule, architecture requirement or coverage is weakened. Native backend optimization, debug/toolchain and self-hosting evidence belongs to later assignments. Independent whole-stage audit still reviews this evidence and the reference corrections.
