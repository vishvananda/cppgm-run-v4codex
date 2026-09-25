# PA18 handoff 77 performance evidence

Acceptance: **PA18/O0 LowIR**, spec §9. Nested-definition demand is a language
requirement; no optional optimization, native runtime improvement or mandated
numeric ceiling is introduced. Compiler work follows demanded semantic facts.

## Protocol

[`benchmark77.py`](../student.tests/pa18/benchmark77.py) freezes entry `2ce99c6a`
and final `975e6162` binaries, flags and all source texts. One warmup per binary,
four A/A samples and four ABBA blocks are retained for equivalent correct inputs.
New behavior rejected at entry has six final-only samples. Failed compilation
is not compared with successful compilation as a performance improvement.

[Final observations](../student.tests/pa18/loop77-performance-final.json) include
all samples, paired ratios/spread, RSS, A/A noise, platform/CPU, binary/backend
hashes and output checks. The [first batch](../student.tests/pa18/loop77-performance.json)
uses `b6294394` before the explicit nested-class extension and remains unchanged.
The final batch refreshes the complete corpus after all implementation changes.

Compiler flags: `--emit-lowir -O0`; build: `g++ -std=gnu++11 -Wall -O3`, test runner
enabled. Separate preflight uses `--validate-lowir --stats`. Checked execution uses
the supplied backend at `-O0`, pinned bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
Every executable exits zero. All equivalent LowIR and executable hashes match exactly.

## Compiler latency and peak RSS

| Workload | A / B median ms | Paired B/A median (range) | A / B peak RSS KiB |
|---|---:|---:|---:|
| ordering-600 | 54.52 / 55.15 | 1.011 (1.003–1.028) | 15256 / 15272 |
| ordering-2400 | 212.34 / 212.07 | 0.997 (0.990–1.003) | 43920 / 43800 |
| member-head-600 | 97.25 / 98.42 | 1.016 (0.986–1.021) | 22964 / 22732 |
| member-head-2400 | 397.19 / 400.52 | 1.008 (0.891–1.286) | 72996 / 72452 |
| common-loop-float-1500 | 154.88 / 155.79 | 1.001 (0.478–1.013) | 30468 / 30404 |
| runtime-calls | 5.97 / 6.00 | 1.003 (0.994–1.017) | 6096 / 5984 |
| runtime-memory | 5.83 / 5.85 | 0.998 (0.995–1.030) | 6032 / 5952 |
| runtime-floating | 5.83 / 5.93 | 1.014 (0.994–1.024) | 6044 / 6132 |
| nested-dormant-600 | 100.01 / 23.52 | 0.235 (0.234–0.237) | 18452 / 9160 |
| nested-layout-600 | 101.26 / 99.99 | 0.998 (0.956–0.999) | 18484 / 18296 |
| nested-invalid-dormant-600 | — / 23.14 | new behavior | — / 9108 |
| ambiguous-type-600 | — / 38.50 | new behavior | — / 11916 |
| nested-dormant-2400 | 407.52 / 78.44 | 0.192 (0.192–0.214) | 56444 / 18892 |
| nested-layout-2400 | 409.94 / 408.72 | 0.998 (0.994–1.013) | 55672 / 55700 |
| nested-invalid-dormant-2400 | — / 76.89 | new behavior | — / 18764 |
| ambiguous-type-2400 | — / 142.80 | new behavior | — / 28744 |
| runtime-nested | 5.95 / 5.98 | 1.008 (0.985–1.020) | 5908 / 6060 |

Startup medians are **5.43 / 5.46 ms**. Small runtime-source compiler timings
are startup-limited; scaled frontend cases dominate startup. Paired ranges retain
visible environmental outliers, including the common-loop and large member-head
cases. No general compiler speedup is inferred from those samples.

Dormant nested definitions improve reproducibly in both batches: final compiler
latency is about **4.26× / 5.20× faster** at 600 / 2400 classes, with peak RSS
**18,452 → 9,160 KiB / 56,444 → 18,892 KiB**. Both implementations produce exactly
the same valid output on these sources. The work counters explain the reduction:
the final compiler creates zero concrete nested definitions or member occurrences.
Layout-demand workloads retain approximately equal latency and memory; the two
batches show no repeatable material regression on the unchanged corpus.

Compiler `.text`: **1,941,318 → 1,944,070 bytes**, **+2,752 (0.142%)**. This covers
new required semantic paths and state checks. Equivalent generated executables
do not grow. No optimization work/growth policy is added.

## Checked runtime and generated size

Volatile bounds and checked data-dependent results prevent dead/folded workloads.
The supplied backend emits sectionless ELF; the code-size metric is executable
payload bytes, preserving the prior evidence convention rather than inventing a
`.text` section measurement. These runtime sources have no static payload data.

| Workload | A / B median seconds | Paired B/A median (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 0.715275 / 0.715285 | 1.001 (0.997–1.005) | 206 / 206 |
| runtime-memory | 0.417930 / 0.417321 | 1.000 (0.997–1.001) | 434 / 434 |
| runtime-floating | 0.495860 / 0.495796 | 1.001 (0.999–1.008) | 230 / 230 |
| runtime-nested | 0.151297 / 0.152157 | 1.007 (0.998–1.008) | 205 / 205 |

Executable bytes are identical. Runtime differences are environmental observations;
this change claims frontend savings from correct demand, not a native runtime gain.

## Work bounds and stage acceptance

The 18 independent graph controls at 32/128 classes and 4/64/256 members establish:

| Demand | Definition transitions | Concrete occurrences |
|---|---:|---:|
| Dormant nested definition | 0 | 8N, independent of member count |
| Layout of N distinct nested classes | N | (8 + 9 × members)N |
| Repeated layout of one nested class | 1 | 8 + 9 × members, independent of use count |

Three nested completion controls at 32/128/512 distinct owners invalidate exactly
one query when one nested definition is supplied. All three inherited completion
controls likewise preserve local invalidation. Source-region IDs, canonical class
identity, immutable substitution frames and indexed base/access edges bound the
work; dormant source bodies are parsed and bound once and never replayed.

For fourfold class input, final dormant/layout medians grow **3.34× / 4.09×**.
Newly required dormant-invalid and ambiguous-type behavior grows **3.32× / 3.71×**.
These are final-only semantic costs with no correct entry implementation to compare.
There is no unbounded transformation, global rescan, or retained optimization body.

Historical **+15%, +16 MiB, 5.5×** targets remain non-mandated diagnostics, with all
measurements in records 63–76 preserved. Correctness, unchanged coverage and
stage-owned graph bounds remain gates. Native optimization and self-hosting remain
PA24–PA34 work; this acceptance does not waive the 32 PA18 failures or independent
review of handoffs 75–77.
