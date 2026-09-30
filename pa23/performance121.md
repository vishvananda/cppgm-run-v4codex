# PA23/O0 performance — handoff 121

Final implementation: **6ae06461**. This evidence covers the completed ordinary nonvirtual view/dispatch/RTTI group; PA23 remains incomplete.

## Protocol

[`benchmark121.py`](../student.tests/pa23/benchmark121.py) freezes binary hashes, flags, full source inputs, platform and CPU affinity. Each phase has one warmup per lane, four A/A observations and four ABBA blocks. Compilation and checked execution are measured separately, with wall time and peak RSS. Telemetry/explicit-validation runs must preserve each LowIR hash. Native programs are built by the supplied backend from our IR and host linking; source output is never delegated to a host compiler.

Common A/B: stage entry `f33dd077` versus final. View A/B: correct implementation `b1aed715` versus final, since entry cannot execute the new semantics correctly. Runtime loops use volatile iteration inputs and live calls, memory, floating point or crosscasts, with checked answers. Tiny compiler/executable observations are marked startup-sensitive; no improvement claim relies on them.

All samples, paired ratios and spreads remain in [common](../student.tests/pa23/performance121-common.json) and [view](../student.tests/pa23/performance121-views.json) data. [Validation](../student.tests/pa23/validation121.json) separately inspects native text and checks relaxed equivalence where private object spellings changed. No required comparator or reference was modified.

## Final observations

Times are median milliseconds; RSS is peak KiB; native text is bytes. Compiler pair ratios show the median and full range of four ABBA blocks.

| Input | Compiler A → B ms | RSS A → B KiB | Compiler pair B/A [range] | Runtime A → B ms | Native text A → B |
|---|---:|---:|---:|---:|---:|
| auto-specializations-9600 | 712.31 → 706.43 | 106812 → 106952 | 0.998 [0.973–1.025] | 4.43 → 4.43 | 768286 → 768286 |
| runtime-calls | 6.79 → 6.57 | 6092 → 6240 | 0.979 [0.941–0.994] | 126.98 → 125.19 | 430 → 430 |
| runtime-memory | 6.63 → 6.47 | 6112 → 6316 | 0.979 [0.947–0.989] | 78.12 → 80.98 | 658 → 658 |
| runtime-floating | 6.48 → 6.23 | 6416 → 6532 | 0.965 [0.933–0.996] | 86.09 → 86.49 | 454 → 454 |
| runtime-member | 6.36 → 6.13 | 6132 → 6392 | 0.968 [0.949–0.999] | 359.50 → 371.61 | 488 → 488 |
| member-functions-2048 | 355.40 → 344.54 | 28216 → 27668 | 0.979 [0.948–0.986] | 6.82 → 7.21 | 184632 → 184632 |
| runtime-this-downcast | 12.87 → 12.11 | 6328 → 6356 | 0.952 [0.873–1.065] | 338.71 → 305.01 | 428 → 396 |
| primary-chain-512 | 23.73 → 22.45 | 7416 → 7700 | 0.940 [0.893–1.014] | 6.07 → 6.17 | 272 → 272 |
| primary-chain-2048 | 66.73 → 65.79 | 12040 → 12396 | 0.987 [0.957–1.036] | 7.19 → 7.13 | 272 → 272 |
| secondary-specializations-256 | 205.88 → 198.25 | 20476 → 20564 | 0.966 [0.798–0.997] | 10.99 → 10.51 | 78088 → 78088 |
| secondary-specializations-1024 | 663.53 → 641.39 | 63020 → 63292 | 1.021 [0.826–1.812] | 8.70 → 8.49 | 311560 → 311560 |
| runtime-secondary | 13.89 → 13.35 | 6360 → 6268 | 0.975 [0.955–0.998] | 425.79 → 393.80 | 632 → 632 |
| runtime-crosscast | 10.90 → 10.60 | 6160 → 6404 | 0.943 [0.918–1.002] | 588.17 → 583.66 | 634 → 634 |

## Acceptance and limits

Compiler `.text` is **2,286,342→2,308,614 bytes**, a **22,272-byte** required implementation cost. The largest observed RSS increase in the final comparisons is **356 KiB**. All six inherited common inputs retain identical LowIR; the new view workloads differ only in private object metadata where applicable, which the unchanged relaxed comparator checks. Program text size is unchanged except for the `this` input.

The `this` fact removes an unnecessary null branch: text falls **428→396 bytes**, with runtime paired ratios 0.931, 0.886, 0.823, 0.953. Earlier independent A/B blocks show the same direction. Its compiler cost is O(1), it adds no analysis and has **zero instruction-growth budget**. No iterative optimizer, body cloning or optional inlining was introduced.

Primary-chain growth **512→2048** produces **512→2048 slots**, **zero secondary views**, **1022→4094 slot-work visits**, and no RTTI demand. Secondary specializations **256→1024** produce **256→1024 views**, **1280→5120 slots**, **2304→9216 slot-work visits**, and **512→2048 RTTI edge visits**. These linear work counts support the scoped complexity claims; elapsed times alone do not.

Each table and distinct target/receiver/result/deleting-entry thunk emits once. Class view storage is a contiguous slot arena with slices, not per-view allocations. Branching RTTI flags require a reachable-base scan once per demanded class; single-base propagation is constant work. The scan has ABI purpose and never searches unrelated declarations. Cache and scratch lifetimes are explicit in the handoff detail.

Spec §9’s stage-scoped acceptance applies. Required semantic work and table/thunk sizes cannot be removed to satisfy an unsupported speed target. Inherited +15% latency, +16 MiB RSS and 5.5× scaling thresholds remain diagnostics, as already classified in PA22, not additional gates. Mandated correctness/work/growth limits and coverage are unchanged. Native optimizer/debug and self-host performance belong to their later stages.

## Noise and preserved observations

The earlier `72433afa` template run had A/A **1.120–1.451 s**, entry/final medians **1.536/1.701 s** (+10.8%) and a paired outlier of **1.230**. That input performs no virtual/RTTI work. An eight-block repeat with identical binaries/source measured **1.028/1.011 s** (−1.6%), RSS **106808/106860 KiB**, paired ratios **1.028, 0.893, 0.940, 0.964, 1.115, 1.057, 0.887, 0.915**. The possible regression did not repeat. The final metadata-corrected binary independently measures **0.712/0.706 s**, with A/A **0.701–0.738 s**. No template speedup is claimed and the slow observations remain available.

Noisy runtime differences on unchanged code are not presented as optimization wins or hidden regressions. The `this` improvement is supported by changed executable work, smaller code, checked output and repeated paired observations. Current measurements disclose all other spread and cost; there is no remaining demonstrated avoidable regression from this group.

- [Initial common pilot](../student.tests/pa23/performance121-common-pilot.json) overlapped earlier validation and is retained separately.
- [Common before the arena-identity seal](../student.tests/pa23/performance121-common-before-seal.json) and [views before that seal](../student.tests/pa23/performance121-views-before-seal.json) preserve the `55e2053a` binary measurements.
- [Common before native metadata correction](../student.tests/pa23/performance121-common-before-native.json), [views before correction](../student.tests/pa23/performance121-views-before-native.json), and [template repeat](../student.tests/pa23/performance121-template-repeat-before-native.json) preserve `72433afa` observations.
- All 13 workloads were rerun against the final `6ae06461` binary after private object-name/runtime-role corrections. Earlier observations were not relabeled as final results.
