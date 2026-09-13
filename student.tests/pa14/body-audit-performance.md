# PA14 body/statement audit performance

Frozen comparison: audit entry `78c2f13e` against the body-publication and statement-fact changes. The compiler hashes, flags, CPU, input hashes, telemetry, warmups and every observation are in [body-audit-performance.json](body-audit-performance.json) and [body-audit-noise.json](body-audit-noise.json). The second campaign repeats the six principal frontend workloads and the new executable. No build, test or verifier ran during timing. Each workload has one warmup per binary, four A/A observations and two ABBA blocks; 532 new observations, 16,380 cumulative.

The 17 inherited inputs plus five return inputs are equivalent under the course comparison. All nine executable payloads have identical sizes and checked exit status zero; complete executable hashes are preserved. No generated runtime improvement is claimed. The supplied PA8 native backend is an external observation tool; production ends at typed LowIR in PA14. Compiler .text is 1,337,734 → 1,348,742 bytes (+11,008, +0.823%).

The table reports both paired compiler changes, A/A wall range and median RSS for the ABBA samples. Short runtime-input compilations measure startup-scale overhead and support no latency claim.

| Campaign / workload | Paired B/A change (%) | A/A seconds | RSS A → B (KiB) |
|---|---:|---:|---:|
| main / virtual-runtime | +0.16 / -0.72 | 0.0065–0.0068 | 5216 → 5220 |
| main / destructor-runtime | -0.64 / -1.39 | 0.0061–0.0063 | 5246 → 5240 |
| main / body-run-4000-128 | +10.00 / -0.59 | 0.6054–0.6104 | 95308 → 94676 |
| main / declaration-instances-1000 | -0.08 / +6.14 | 0.5597–0.6204 | 75600 → 75430 |
| main / demand-uses-1000-128-4 | -0.28 / -0.79 | 3.0406–3.0814 | 346192 → 346244 |
| main / demand-runtime | +1.24 / +0.34 | 0.0069–0.0087 | 5546 → 5452 |
| main / region-runtime | +0.37 / -1.31 | 0.0074–0.0075 | 5576 → 5496 |
| main / calls-runtime | -0.50 / +6.47 | 0.0053–0.0056 | 5258 → 5288 |
| main / memory-runtime | +1.03 / +0.80 | 0.0058–0.0067 | 5274 → 5278 |
| main / floating-runtime | +0.67 / +0.52 | 0.0059–0.0068 | 5404 → 5544 |
| main / default-16000-1-1-1 | -1.49 / +0.64 | 0.0822–0.0855 | 15682 → 15678 |
| main / default-64000-1-1-1 | +0.55 / +0.89 | 0.3223–0.3261 | 47336 → 47310 |
| main / default-16000-512-1-1 | +0.76 / -0.36 | 0.1172–0.1207 | 22244 → 22382 |
| main / default-16000-1-32-1 | +0.12 / +0.67 | 0.0820–0.0841 | 15624 → 15664 |
| main / default-16000-1-32-4000 | -0.23 / +0.81 | 0.1419–0.1436 | 25034 → 24742 |
| main / default-16000-1-1-4000 | -0.16 / +1.39 | 0.1380–0.1424 | 24894 → 24826 |
| main / default-runtime | -0.22 / +0.57 | 0.0059–0.0061 | 5288 → 5332 |
| main / return-16000-1-8-1 | +0.46 / -0.48 | 0.0825–0.0835 | 15684 → 15612 |
| main / return-64000-1-8-1 | +0.20 / +1.16 | 0.3209–0.3289 | 47270 → 47246 |
| main / return-16000-128-8-1 | -1.94 / -1.41 | 0.1094–0.1105 | 21112 → 21088 |
| main / return-16000-1-8-4000 | -0.08 / -0.08 | 0.1689–0.1901 | 30354 → 31050 |
| main / return-runtime | +1.24 / +2.41 | 0.0059–0.0061 | 5360 → 5294 |
| repeat / body-run-4000-128 | +0.88 / -1.36 | 0.6055–0.6241 | 95042 → 94300 |
| repeat / declaration-instances-1000 | -0.10 / +1.05 | 0.5611–0.5741 | 75586 → 75578 |
| repeat / demand-uses-1000-128-4 | -0.16 / -1.10 | 3.1111–3.1271 | 346176 → 346192 |
| repeat / return-16000-128-8-1 | -1.38 / -0.12 | 0.1094–0.1149 | 21212 → 21158 |
| repeat / return-16000-1-8-4000 | -0.26 / +0.25 | 0.1692–0.1926 | 30264 → 31106 |
| repeat / return-runtime | +1.32 / +0.34 | 0.0063–0.0065 | 5288 → 5382 |

The body workload’s initial +10.00% pair and declaration workload’s +6.14% pair do not repeat (+0.88/−1.36% and −0.10/+1.05%). All observations remain available. Return conversion candidate visits fall 1,281 → 138 for 128 specializations; the latency reduction is small compared with the repeated A/A range, so this establishes bounded shared work, not a broad timing gain. The 4,000-use return input costs +696/+842 KiB in the two median RSS comparisons. No broad memory reduction is claimed.

Nine source return recipes are checked once independently of unrelated declaration count N, specialization count K and repeated call count Q; concrete applications are 9K. Body and lifetime checks agree with demanded function count. No generated code growth or optional optimizer is introduced. PA14/O0 has no mandated numeric frontend latency, RSS or compiler text ceiling; historical self-selected targets remain diagnostic, with measurements preserved. Necessary definition-time validation and terminal states are correctness costs, not optional runtime transforms.

Entity remains 112 bytes; all other measured records except Analyzer are unchanged. Analyzer is 6,312 → 6,376 bytes: one TU-owned flat source index and four counters. Source conversion recipes are immutable; per-use conversion objects and lifetimes are copied/materialized at their concrete owner. See [body-layout.json](body-layout.json).

| Campaign / executable | Paired runtime change (%) | Native payload bytes A = B |
|---|---:|---:|
| main / virtual-runtime | +0.31 / -0.00 | 2360 |
| main / destructor-runtime | -0.29 / +0.03 | 2072 |
| main / demand-runtime | -0.04 / +0.18 | 261 |
| main / region-runtime | +0.39 / -2.51 | 206 |
| main / calls-runtime | -0.27 / +0.34 | 206 |
| main / memory-runtime | -0.17 / -0.39 | 434 |
| main / floating-runtime | -0.10 / +0.68 | 230 |
| main / default-runtime | +1.59 / -0.56 | 300 |
| main / return-runtime | -1.60 / +2.28 | 444 |
| repeat / return-runtime | +0.36 / +1.75 | 444 |

Runtime variation includes −2.51% through +2.28% pairs on unchanged payloads. Volatile bounds, observable checksums/call counts, object cleanup, memory and floating computations prevent timing dead workloads. Self-hosting is PA34-owned and unavailable in PA14; no claim is made for it.

The 51-check [validation manifest](body-audit-validation.json) freezes 223 implementation/contract source files, 1,266 unchanged fixtures/references, all logs, both release/sanitizer binaries, 349 output comparisons and 64 statement controls per build. The corrected personal Perl comparator adapter avoids lexical `$a`/`$b` shadowing of Perl sort variables. The course comparison code and rules are unchanged; one alias test differs only in allowed top-level presentation order. Exact byte identity is an unsupported personal gate for that case.
