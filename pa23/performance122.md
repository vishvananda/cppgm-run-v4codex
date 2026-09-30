# PA23/O0 performance — handoff 122

Implementation **a04a8cae**, compared with turn entry **26b96f27**. Both binaries, all source inputs, hashes, flags, CPU affinity, platform, observations and checked results are frozen in [initial observations](../student.tests/pa23/performance122.json) and [longer repeats](../student.tests/pa23/performance122-repeat.json). The binaries remain at `/tmp/pa23-122/{entry,sealed}`; their SHA-256 values are also in the committed data.

## Protocol

[`benchmark122.py`](../student.tests/pa23/benchmark122.py) uses one warmup per lane, four A/A samples and four wall-time ABBA blocks, with compilation and executable execution measured separately. The repeat uses eight ABBA blocks, the same binaries and template source, and 12 million deletion iterations instead of one million. The original short measurements are preserved. Runtime inputs are volatile and results are checked; no workload times a constant or dead loop. The compiler is built with the existing dev Makefile defaults (`g++ -std=gnu++11 -Wall -O3`); measured source compilation uses `--emit-lowir -O0`.

All lanes execute correct equivalent programs. The supplied backend compiles our LowIR to native objects and the host linker links those objects. The source frontend never delegates implementation. Telemetry plus explicit validation preserves each LowIR hash. The [validation record](../student.tests/pa23/validation122.json) also hashes native `.text` sections. Small source TUs and execution-only smoke checks are startup-sensitive; no speed claim relies on them.

## Initial observations

Times are median milliseconds; compiler memory is peak KiB; native text is bytes. Pair ranges include every ABBA block.

| Input | Compiler A → B ms | RSS A → B KiB | Compiler paired B/A [range] | Runtime A → B ms | Text A → B |
|---|---:|---:|---:|---:|---:|
| auto-specializations-9600 | 1422.24 → 1380.33 | 108332 → 108404 | 0.950 [0.906–1.001] | 5.91 → 6.00 | 768286 → 768286 |
| runtime-calls | 11.31 → 10.25 | 6188 → 6304 | 0.904 [0.891–0.935] | 190.00 → 199.19 | 430 → 430 |
| runtime-memory | 13.54 → 12.35 | 6148 → 6280 | 0.951 [0.352–0.953] | 122.66 → 122.97 | 658 → 658 |
| runtime-floating | 16.46 → 15.73 | 6416 → 6428 | 0.909 [0.888–0.993] | 152.43 → 147.88 | 454 → 454 |
| runtime-member | 11.06 → 10.51 | 6228 → 6348 | 0.937 [0.728–0.972] | 366.16 → 418.77 | 488 → 488 |
| member-functions-2048 | 288.84 → 296.84 | 28180 → 28300 | 1.029 [0.999–1.165] | 5.82 → 5.83 | 184632 → 184632 |
| runtime-delete-2 | 10.23 → 9.59 | 6124 → 6340 | 0.955 [0.841–1.037] | 62.17 → 53.78 | 1266 → 1412 |
| runtime-delete-3 | 11.85 → 11.46 | 6140 → 6364 | 0.958 [0.901–1.199] | 76.96 → 76.24 | 1686 → 1686 |
| lifecycle-specializations-128 | 199.65 → 206.75 | 13920 → 13952 | 1.029 [0.961–1.154] | 13.86 → 14.07 | 61692 → 61692 |
| lifecycle-specializations-512 | 351.85 → 339.22 | 38756 → 38664 | 0.981 [0.896–1.059] | 7.85 → 7.99 | 246012 → 246012 |
| destructor-width-4 | 11.69 → 11.34 | 6168 → 6368 | 0.983 [0.873–1.030] | 7.51 → 7.59 | 2090 → 2090 |
| destructor-width-16 | 15.77 → 15.42 | 6656 → 6880 | 0.966 [0.925–1.002] | 10.74 → 10.80 | 7032 → 7032 |
| destructor-width-64 | 25.79 → 25.39 | 8404 → 8532 | 0.977 [0.928–1.000] | 7.52 → 7.34 | 28287 → 28287 |

## Longer repeats and acceptance

- **auto-specializations-9600:** compiler medians 1904.97 → 2244.89 ms; eight paired B/A ratios 0.964, 0.905, 1.078, 0.988, 0.981, 1.083, 1.153, 0.984. Compiler peak RSS 108272 → 108360 KiB.
- **runtime-delete-2:** runtime medians 501.14 → 571.08 ms; eight paired B/A ratios 0.805, 1.002, 1.164, 1.112, 1.157, 0.835, 0.995, 0.999. Compiler peak RSS 6164 → 6384 KiB.
- **runtime-delete-3:** runtime medians 828.92 → 900.73 ms; eight paired B/A ratios 1.113, 1.227, 1.015, 1.006, 1.029, 0.986, 0.978, 1.187. Compiler peak RSS 6172 → 6384 KiB.

The pre-seal template observations had A/A 0.728–0.889 s and a 1.233 paired outlier; a repeat measured +0.6% median difference. Those observations are retained below. The sealed run had A/A 1.393–1.738 s, and substantially slower absolute times across both lanes, with mixed paired outliers. No frontend speedup is claimed. Unchanged runtime inputs retain identical native text: their differing timings, including the noisy member-call case, do not identify a generated-code regression. All paired results and spread remain visible.

Two-base deletion removes the extra complete-destructor call on the normal path. The preserved pre-seal repeat showed about 4% improvement, but the sealed repeat does not establish a repeatable speedup: its eight paired ratios span **0.805–1.164**, median **1.0004**. Unpaired medians rise **501.14→571.08 ms**, while A/A ranges **665.95–904.43 ms** and both lanes range widely. No runtime benefit is claimed from these observations. Native text grows **1266→1412 bytes** (+146 bytes). This is required by the small O0 lifecycle comparison shape, with a fixed **two nontrivial action** expansion limit; it is not an optional transform accepted on a speed claim. Source bodies are not cloned; larger suffixes call the complete entry once. Consequently expansion is at most five subobject-call sites across normal/cleanup paths and four deallocation sites, independent of total class width. There is no iterative optimizer or unbounded inlining/growth policy.

The sealed template repeat also varies widely: A spans **1.317–3.509 s**, B **1.115–3.480 s**, with paired ratios **0.905–1.153**, median **0.986**. Its unpaired median increase is **17.8%**; the paired observations do not establish a frontend regression or gain. Three-base deletion retains byte-identical native text, yet its unpaired runtime median rises **8.7%**, confirming substantial execution noise. These uncertainty limits are retained rather than treated as optimization evidence.

Compiler `.text` grows **2,308,614→2,313,478 bytes** (+4,864 bytes) for required table/lifecycle behavior. The largest peak-RSS increase in the sealed comparisons is **224 KiB**. The added layout field is one 64-bit group address point per view; references/definitions use existing typed identities and bounded-lifetime storage. No unrelated cache is invalidated.

## Work and stage scope

- **128→512 lifecycle specializations:** 1,280→5,120 slot-work visits, 256→1,024 destruction actions, 384→1,536 defined tables, 79,872→319,488 bytes of class/view storage and 12,290→49,154 LowIR instructions. Work follows demanded facts and output.
- **4→16→64 destructor bases:** 32→128→512 slot-work visits and 344→1,234→4,834 LowIR instructions. Text is unchanged from entry on these larger-width cases (2,090 / 7,032 / 28,287 bytes). Their shared suffix path remains linear; the two-action change does not reintroduce quadratic cleanup expansion.
- Key-owned groups and declarations are emitted once. The controls check both TU publication orders and expose external references separately from local definitions in telemetry. External-table correctness is not presented as an A/B optimization against the previously incorrect ownership behavior.

Spec §9’s **PA23/O0 stage-scoped acceptance** applies. Inherited +15% latency, +16 MiB RSS and 5.5× scaling diagnostics remain self-selected targets, not mandated exit gates; this preserves their prior reclassification and all handoff-121 observations. Correctness, comparison coverage, bounded work and growth remain mandatory. Later native optimization, debug and self-host performance stay with their owning stages. Acceptance here rests on required O0 representation, bounded work and documented storage/text costs. No optional optimization or speed claim is accepted on the noisy observations; unresolved timing uncertainty remains visible for review.

## Preserved pre-seal observations

[Before entry-identity correction](../student.tests/pa23/performance122-before-entry-identity.json), [its longer repeats](../student.tests/pa23/performance122-repeat-before-entry-identity.json), and [before alias deduplication](../student.tests/pa23/performance122-before-alias-identity.json) retain every earlier sample and binary hash. Their original binary paths remain frozen; they are not relabeled as final. All workloads were rerun against the sealed `a04a8cae` binary.
