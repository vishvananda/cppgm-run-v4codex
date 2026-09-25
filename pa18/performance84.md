# PA18 implementation 84 performance evidence

Acceptance is **PA18/O0 LowIR**, spec §9. The [frozen harness](../student.tests/pa18/benchmark84.py) records source text/hashes, binary hashes, flags, CPU, warmups, telemetry and every sample in [the final observations](../student.tests/pa18/loop84-performance.json). [The initial observations](../student.tests/pa18/loop84-performance-initial.json) remain intact. Both use entry `09a77fca` and implementation `b4d66361` (measured repository tip `7a7ce959`, whose additional changes are proved oracles).

Each comparable workload has one warmup per binary, four A/A calibration samples and four ABBA blocks. Rejected/incorrect baseline behavior receives six final-only samples. Compiler wall time and peak RSS use a separate `/usr/bin/time` invocation. Native construction, correctness preflight and runtime timing are separate. Runtime inputs use volatile bounds and checked data-dependent sums; the empty-aggregate loop executes 24 million calls and the delegation loop executes 12 million calls.

Build flags are `g++ -std=gnu++11 -Wall -O3`; compiler flags are `--emit-lowir -O0`. Separate preflights use `--stats --validate-lowir`. Executables use the supplied `lowir2native-ref -O0`, bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`. Each emitted executable is checked before cost comparison. Own native optimization, debug and self-hosting remain later-stage work.

The initial run overlapped independent correctness validation and contains large compiler timing outliers (paired ranges include 0.656–0.987 for ordering-2400 and 0.773–1.005 for the common corpus). No compiler speedup is claimed from those values. The complete second run started after validation finished; neither run discards observations. The second run still shows large wall-time variation: several unchanged workloads take roughly twice the first-run time, and the 2400-function empty-aggregate paired range is 0.557–1.061. Neither set supports precise compiler speedup claims; instruction/work counts provide the complexity evidence. Both show the same substantial empty-helper runtime improvement.

## Compiler latency, RSS and executable size

Times are median milliseconds; ratio is median of paired block B/A means, with the full paired range. RSS is maximum observed KiB. Executable size is the sectionless ELF payload, including static data, **not isolated `.text`**; compiler size below is actual `.text`. Small source timings are startup-limited diagnostics.

| Workload | A / B ms | B/A (range) | A / B KiB | A / B native payload bytes |
|---|---:|---:|---:|---:|
| ordering-600 | 114.89 / 115.56 | 1.035 (0.921–1.258) | 15456 / 15576 | 40842 / 40842 |
| ordering-2400 | 465.52 / 456.70 | 0.936 (0.908–1.002) | 43608 / 43740 | 163242 / 163242 |
| common-loop-float-1500 | 394.68 / 411.49 | 1.005 (0.937–1.088) | 30296 / 30436 | — / — |
| runtime-calls | 10.19 / 9.69 | 0.949 (0.930–1.000) | 5980 / 5964 | 206 / 206 |
| runtime-memory | 11.23 / 10.37 | 0.937 (0.896–0.982) | 5844 / 6028 | 434 / 434 |
| runtime-floating | 10.97 / 10.53 | 1.001 (0.937–1.119) | 6052 / 6084 | 230 / 230 |
| empty-aggregate-600 | 64.96 / 61.42 | 0.930 (0.907–0.996) | 9108 / 8924 | 22956 / 18139 |
| empty-aggregate-2400 | 215.05 / 198.20 | 0.954 (0.557–1.061) | 19720 / 18248 | 91356 / 72139 |
| delegation-150 | 89.00 / 89.81 | 1.008 (0.895–1.166) | 9928 / 9960 | 12701 / 12701 |
| delegation-600 | 110.25 / 113.22 | 0.956 (0.815–1.027) | 21676 / 21488 | 50501 / 50501 |
| runtime-empty-aggregate | 5.93 / 5.70 | 0.948 (0.931–0.986) | 6016 / 6028 | 224 / 201 |
| runtime-delegation | 5.99 / 5.79 | 0.964 (0.954–0.985) | 5936 / 6040 | 256 / 256 |
| empty-lifetime-600 | — / 36.40 | final only | — / 11260 | — / 108768 |

Startup medians: **10.54 / 10.15 ms**.

## Executable runtime

| Workload | A / B ms | B/A (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 1408.02 / 1349.85 | 0.946 (0.922–1.005) | 206 / 206 |
| runtime-memory | 754.99 / 749.92 | 1.011 (0.978–1.035) | 434 / 434 |
| runtime-floating | 900.82 / 912.91 | 1.022 (0.998–1.099) | 230 / 230 |
| runtime-empty-aggregate | 168.89 / 119.70 | 0.712 (0.705–0.717) | 224 / 201 |
| runtime-delegation | 113.16 / 113.26 | 0.997 (0.987–1.006) | 256 / 256 |

Compiler `.text`: **1,972,422 → 1,973,766 bytes**, **+1,344 (0.068%)**.

## Work bounds and acceptance

The delegated-entry queue uses canonical entity IDs and an O(1)-average flat index. Every base/complete/polymorphic bit crosses its selected edge once at most. Scratch is released after semantic completion. There is one final pass over demanded members; insertion does not retry unrelated classes, candidates or bodies. For 150 → 600 specializations, measured delegation-entry work is **300 → 1200**, semantic entities **2111 → 8411**, and LowIR instructions **3313 → 13213**. Both binaries emit the same instruction counts in this family. No grammar replay, global invalidation or semantic reconstruction is introduced.

Empty aggregate helper omission checks only the group head and removes a no-effect call/body before either is emitted. Its work bound is **one O(1) check per group**, with **zero allowed code growth** and the ordinary action path as fallback. Empty classes with a declared destructor or transfer, or a volatile source, use ordinary checked transfers; that necessary work is not optional. One explicit widening instruction per affected operand preserves the required O0 conversion boundary. No optimizer pass or speculative code growth is added. Existing initializer expansion and named-result summary limits remain unchanged.

The empty-helper runtime workload drops from 224 to 201 payload bytes, removing one empty helper body and its call. It preserves the live destination, argument call and data-dependent work. The repeatable runtime improvement, smaller output and reduced compiler work justify this local omission. The initial run had a paired runtime median of **0.709** (range **0.704–0.711**); the final table supplies independent confirmation.

Unchanged ordering/common/call/memory/floating workloads retain exact LowIR; unchanged runtime probes retain identical executable bytes. Delegation runtime retains 256 payload bytes. The final identical-code runtime ratios are 0.946, 1.011 and 1.022 for calls, memory and floating work, with ranges spanning both directions; their initial ratios were 1.002, 1.000 and 1.001. These inconsistent shifts on identical bytes disclose timing variation rather than evidence of a generated-code improvement or regression. `empty-lifetime-600` is final-only because entry emits invalid by-value argument LowIR for an empty class with a nontrivial destructor. Its final executable checks two demanded calls and all four destructor effects; the rejected entry cannot be a fast correctness baseline.

PA18/O0 mandates no numeric compiler-latency or RSS ceiling. Historical +15%, +16 MiB and 5.5× targets remain diagnostic, with earlier measurements preserved. Correctness, unchanged coverage, bounded work and optional-transform profitability remain mandatory. Necessary ABI/value checks receive measured costs; later native optimization is not an additional PA18 exit gate. There is no avoidable regression established by these observations.
