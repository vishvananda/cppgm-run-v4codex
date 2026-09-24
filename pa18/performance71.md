# PA18 loop 71 performance evidence

Frozen entry `2eb83de5` and implementation `b6287928`, built with `g++ -std=gnu++11 -Wall -O3` and the test runner enabled. [Harness](../student.tests/pa18/benchmark71.py) and [all observations](../student.tests/pa18/loop71-performance.json) retain binary/input/output hashes, flags, CPU affinity, warmups, timings, RSS and work counters. No rebuild occurs during measurement.

One warmup per binary; four A/A observations; four ABBA blocks for equivalent correct outputs. Newly supported inputs have six final-only observations after entry rejection. Telemetry and full LowIR validation run separately from measurement. Compilation and checked executable execution are measured separately. Sources retain volatile runtime inputs and checked results. The supplied native backend is pinned to bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.

## Compiler latency and peak RSS

| Workload | A ms | B ms | Paired B/A median (range) | Peak RSS A / B KiB |
|---|---:|---:|---:|---:|
| ordering-600 | 56.66 | 55.51 | 0.984 (0.966–1.114) | 15180 / 15220 |
| ordering-2400 | 219.73 | 218.91 | 1.000 (0.968–1.115) | 43004 / 43164 |
| member-head-600 | 174.20 | 174.08 | 0.956 (0.783–1.029) | 22836 / 22896 |
| member-head-2400 | 395.71 | 392.90 | 0.996 (0.946–1.012) | 74212 / 72020 |
| common-loop-float-1500 | 153.53 | 153.95 | 0.999 (0.791–1.005) | 30284 / 30392 |
| runtime-calls | 5.70 | 5.58 | 0.983 (0.959–0.988) | 5892 / 5840 |
| runtime-memory | 18.20 | 18.63 | 0.988 (0.810–1.119) | 5844 / 5980 |
| runtime-floating | 6.47 | 6.39 | 0.992 (0.936–1.012) | 6024 / 5968 |
| member-context-600 | 150.66 | 155.50 | 1.026 (1.016–1.053) | 30376 / 30752 |
| lexical-alias-600 | — | 166.20 | new behavior | — / 31780 |
| member-definition-600 | — | 199.49 | new behavior | — / 39796 |
| member-context-2400 | 613.79 | 623.97 | 1.017 (1.014–1.025) | 103892 / 105928 |
| lexical-alias-2400 | — | 656.94 | new behavior | — / 111136 |
| member-definition-2400 | — | 836.95 | new behavior | — / 141780 |
| runtime-new-member | — | 6.81 | new behavior | — / 5936 |

Empty-input startup medians are 6.06 / 5.89 ms. The larger frontend workloads dominate startup; compile timings of the small runtime sources are startup-limited and do not establish a compiler speedup. All equivalent inputs have identical LowIR; all equivalent native outputs have identical hashes.

Member-context overhead is repeatable (2.6% at 600 and 1.7% at 2400), with peak RSS growth 376 / 2036 KiB. This is required semantic work: complete enclosing context and structural declaration comparison. The cache computes each consumed immutable shape once; no optional transform was added. The owner is bounded by actual declaration/type/query edges, as shown below. Other common measurements do not show a repeatable material regression. Outliers are retained: member-head-600 paired range 0.783–1.029 and the initial run’s much shorter wall times demonstrate scheduling noise, not an optimization benefit.

## Checked executable runtime and size

| Workload | A / B median seconds | Paired B/A (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 0.765914 / 0.844204 | 0.995 (0.919–1.156) | 206 / 206 |
| runtime-memory | 0.420245 / 0.422487 | 1.002 (0.973–1.022) | 434 / 434 |
| runtime-floating | 0.495668 / 0.496086 | 0.998 (0.987–1.003) | 230 / 230 |
| runtime-new-member | — / 0.114549 | new behavior | — / 179 |

All native runs return zero. The backend emits sectionless ELF; payload bytes are the executable-code proxy for these sources without static data, not a claimed ELF `.text` section. Runtime variation in the byte-identical calls executable (0.716–1.172 s for A; 0.714–1.154 s for B) is environmental. No runtime gain or regression is attributed to this frontend change. Newly required member-definition execution has no correct entry executable for A/B comparison.

Compiler `.text`: **1,866,374 → 1,871,494 bytes**, +5,120 bytes (0.274%). This is compiler implementation growth; generated equivalent code is unchanged.

## Graph work and acceptance

| Family, 600 → 2400 | Type substitution work | Signature shapes computed | Shape cache hits |
|---|---:|---:|---:|
| member-context | 10212 → 40812 | 4209 → 16809 | 3002 → 12002 |
| lexical-alias | 12616 → 50416 | 4 → 4 | 1801 → 7201 |
| member-definition | 15626 → 62426 | 6619 → 26419 | 6009 → 24009 |

Fourfold input growth yields approximately fourfold type/shape work and final compiler time (member context 4.01×, aliases 3.95×, definitions 4.20×). Alias structural shape work is constant where all instantiations share the same declaration shape. Existing completion-scaling controls separately prove that one completed class invalidates one consumer among 32/128/512 pending classes. No global cache epoch or retry was introduced.

Acceptance is **PA18/O0 LowIR**, spec §9. There is no mandated numerical compiler/runtime ceiling at this stage and no optional optimization whose benefit must justify its cost. Correctness, coverage and bounded graph work remain gates. The historical +15%, +16 MiB and 5.5× diagnostic targets remain diagnostics, with all inherited measurements preserved. Native optimization/allocation/ELF ownership and self-hosting are later-stage requirements, not waived requirements.

The [initial observations](../student.tests/pa18/loop71-performance-attempt1.json) are preserved. That run completed the eight inherited workloads, then correctly stopped because `member-context-600`, initially classified as new behavior, already compiled at entry. The final protocol compares it as an equivalent workload and adds actual failing lexical-alias workloads. Neither binary changed. The original harness is frozen at `/tmp/pa18-loop71/benchmark71-attempt1.py`; its hash is recorded in the initial JSON. No measurement or outlier was discarded.
