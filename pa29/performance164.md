# PA29 implementation164 performance evidence

Code tip: `e16108af`. PA29/O0 required-capability acceptance. No optional optimization or speedup claim.

The [manifest](../student.tests/pa29/evidence164/performance-manifest.json) freezes binaries, scripts, flags, CPU and environment; observation files include every input hash. Compilation and checked execution were measured separately with CPU affinity 0, wall clocks and `/usr/bin/time` peak RSS. No compiler build or test suite overlapped measurements; light documentation/evidence work and external scheduling were uncontrolled.

Common workloads retain four A/A calibration samples and six ABBA blocks per mode/workload: **224 observations**. Affected demand shapes retain eight samples per mode: **96 observations**, plus six launcher calibrations. No sample was discarded.

## Equivalent common workloads

[All common observations](../student.tests/pa29/evidence164/common-performance.json). Medians are seconds; paired ratios are B/A block means, with the full block range. RSS is maximum KiB. Each input demands 2,400 templates and executes checked loops driven by runtime input. All four A/B object and executable pairs are **byte-identical**.

| Workload | Compile A / B s | Paired compile median [range] | Compiler RSS A / B KiB | Runtime A / B s | Text A = B bytes |
|---|---:|---:|---:|---:|---:|
| memory | 0.16233 / 0.16151 | 0.9931 [0.8315, 0.9998] | 29732 / 30100 | 0.05263 / 0.05259 | 151633 |
| floating | 0.16234 / 0.16236 | 0.9961 [0.6965, 1.0315] | 29488 / 29696 | 0.04947 / 0.04946 | 151474 |
| exceptions | 0.25894 / 0.21283 | 0.9985 [0.7338, 1.0115] | 29272 / 29632 | 0.25060 / 0.25150 | 151781 |
| pruning | 0.20162 / 0.19899 | 0.9921 [0.8795, 1.1044] | 35024 / 35284 | 0.05246 / 0.05256 | 151633 |

| Workload | A/A compile range s | A/A runtime range s | Paired runtime median [range] | Runtime RSS A / B KiB |
|---|---:|---:|---:|---:|
| memory | 0.16107–0.19387 | 0.05237–0.05278 | 0.9993 [0.9856, 1.0108] | 1756 / 1756 |
| floating | 0.16120–0.16551 | 0.04909–0.04958 | 0.9939 [0.9647, 1.0618] | 1760 / 1760 |
| exceptions | 0.15984–0.16329 | 0.31661–0.44730 | 1.0030 [0.9958, 1.1030] | 3964 / 3932 |
| pruning | 0.19941–0.21181 | 0.05228–0.05285 | 1.0023 [0.9989, 1.0087] | 1760 / 1760 |

Paired compiler medians range from 0.9921 to 0.9985. Broad calibration/block spreads, especially exceptions, prevent interpreting small changes as speedups. These samples show no repeatable avoidable regression. Identical emitted bytes exclude a generated-code explanation for runtime timing differences.

## Required function-context costs

The [entry affected results](../student.tests/pa29/evidence164/affected-baseline.json) show why these are not equivalent A/B workloads: the entry compiler emits incorrect bare names for every string workload (the initial checked demand checksum fails), and rejects all dependent-query workloads. [Final affected observations](../student.tests/pa29/evidence164/affected-performance.json) record correctness cost/scaling only.

String workloads demand 600/1,200/2,400 distinct function strings, then perform thirty million calls with runtime-derived character indices. Query workloads demand the same number of functions with constexpr array-reference deduction through pretty-function strings, then execute thirty million checked runtime-input calls. All demanded functions contribute to an independent checksum.

| Workload / demands | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| strings600 | 0.06041 [0.05954, 0.06277] | 16304 | 0.17400 [0.17325, 0.28584] | 1760 | 47933 |
| strings1200 | 0.11115 [0.11034, 0.11259] | 25520 | 0.17421 [0.17325, 0.17913] | 1756 | 95333 |
| strings2400 | 0.22568 [0.22243, 0.26414] | 43980 | 0.17428 [0.17390, 0.17512] | 1756 | 190133 |
| queries600 | 0.06621 [0.06473, 0.06805] | 17284 | 0.18243 [0.18127, 0.19249] | 1760 | 40731 |
| queries1200 | 0.17494 [0.13115, 0.21577] | 27280 | 0.18395 [0.18272, 0.18445] | 1760 | 80931 |
| queries2400 | 0.24829 [0.24508, 0.25609] | 47500 | 0.18307 [0.18239, 0.18425] | 1756 | 161331 |

| Workload | String objects | Body transitions | LowIR instructions | Native instructions |
|---|---:|---:|---:|---:|
| strings600 | 600 | 600 | 9644 | 10256 |
| strings1200 | 1200 | 1200 | 19244 | 20456 |
| strings2400 | 2400 | 2400 | 38444 | 40856 |
| queries600 | 600 | 603 | 9644 | 10256 |
| queries1200 | 1200 | 1204 | 19244 | 20456 |
| queries2400 | 2400 | 2404 | 38444 | 40856 |

Launcher median is 0.00437 s [0.00432, 0.00502]. Runtime workloads exceed launcher time by over 30 times. Work counters, string objects, text and incremental memory grow proportionally to demanded functions. Query timings include a higher-cost 1,200-demand sample; all observations are retained, and no exact scaling exponent or speedup is inferred.

## Acceptance and limits

Compiler binary size grows from 4,048,384 to 4,057,576 bytes (+9,192). Common generated text is unchanged. New string data/typed queries are required semantics, with linear bounds described in [function-context164](function-context164.md). Sparse string counts add O(1) reporting work and no analysis solely for telemetry.

New optional work and growth budgets are **zero**. Existing constexpr step/depth bounds, native limits and assignment timeouts remain. Historical blanket 15% latency/RSS and zero-growth diagnostic targets retain their spec §9 classification; prior evidence is preserved and no mandated gate is weakened. No PA29 correctness, fixture or comparison requirement changed. Broader hosted runtime, optimization/allocation and self-hosting remain owned by PA30–34.
