# PA18 implementation 85 performance evidence

Acceptance is **PA18/O0 LowIR**, spec §9. [The harness](../student.tests/pa18/benchmark85.py) freezes binary hashes, commits, flags, inputs, CPU affinity, telemetry and every observation in [the evidence](../student.tests/pa18/loop85-performance.json). Entry is `f953e42a`; final implementation is `076eccdd`. Correctness validation finished before timing began.

The [initial observations](../student.tests/pa18/loop85-performance-initial.json) are preserved. They exposed 151/601 copy selections and zero source-recipe reuse for 150/600 fixed uses. The final implementation retains the checked source recipe for casts, statements, commas, parentheses and conditionals; the scaling assertions below require one selection and one materialization per concrete use. Both complete runs use the same frozen entry binary, inputs, flags and harness. This repairs a spec defect rather than reclassifying repeated work as an acceptable timing cost.

Comparable cases use one warmup per binary, four A/A calibration samples and four ABBA blocks. Incorrect baseline behavior is preserved in the record and receives six final-only samples. Compilation and executable timing are separate; `/usr/bin/time` records peak RSS, CPU time and scheduling counters. Runtime bounds are volatile, and results are checked before timing and on every execution. The scalar discard loop performs 24 million calls and the class discard loop performs 12 million copies/destructions.

Build flags: `g++ -std=gnu++11 -Wall -O3`, with the course test runner. Compiler flags: `--emit-lowir -O0`. Separate preflights add `--stats --validate-lowir`. The explicit harness uses `lowir2native-ref -O0`, pinned bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, to build checked executables. This does not implement production compiler behavior. Native backend optimization and self-hosting are later-stage requirements.

## Compiler latency, peak RSS and output size

Times are median milliseconds. Ratios are medians of paired block means with their complete ranges. RSS is maximum observed KiB. The supplied backend emits sectionless ELF, so native size is its loadable payload proxy (code **and static data**), not isolated `.text`. Compiler `.text` is measured directly. Startup-sized rows are diagnostics. All A/A and warmup observations remain in the JSON.

| Workload | A / B ms | Paired B/A (range) | A / B KiB | A / B native payload bytes |
|---|---:|---:|---:|---:|
| ordering-600 | 56.81 / 56.01 | 0.986 (0.975–1.006) | 15292 / 15228 | 40842 / 40842 |
| ordering-2400 | 210.72 / 212.56 | 1.014 (0.999–1.184) | 43416 / 43396 | 163242 / 163242 |
| common-loop-float-1500 | 152.72 / 152.20 | 0.990 (0.636–1.006) | 30356 / 30328 | — / — |
| runtime-calls | 5.59 / 5.55 | 0.999 (0.993–1.005) | 6064 / 6072 | 206 / 206 |
| runtime-memory | 5.92 / 5.96 | 1.005 (0.994–1.014) | 6068 / 6044 | 434 / 434 |
| runtime-floating | 5.78 / 5.77 | 0.994 (0.981–1.018) | 6116 / 6180 | 230 / 230 |
| discard-scalar-150 | 18.07 / 18.09 | 1.000 (0.655–1.003) | 8136 / 8176 | 11212 / 11212 |
| discard-class-150 | — / 13.69 | final only | — / 7560 | — / 46992 |
| storage-dormant-150 | 15.41 / 15.69 | 1.024 (1.014–1.042) | 7400 / 7428 | 3105 / 3105 |
| discard-scalar-600 | 54.38 / 54.50 | 1.004 (0.996–1.006) | 15000 / 15000 | 44512 / 44512 |
| discard-class-600 | — / 36.97 | final only | — / 11976 | — / 185592 |
| storage-dormant-600 | 43.83 / 44.12 | 1.005 (1.000–1.011) | 12160 / 12100 | 12105 / 12105 |
| runtime-discard-scalar | 5.85 / 5.94 | 1.008 (1.000–1.018) | 5996 / 6052 | 249 / 249 |
| runtime-discard-class | — / 6.04 | final only | — / 6032 | — / 1240 |

Startup A/B medians: **5.46 / 5.49 ms**.

## Executable runtime

| Workload | A / B ms | Paired B/A (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 723.95 / 717.78 | 0.993 (0.983–1.007) | 206 / 206 |
| runtime-memory | 421.99 / 421.12 | 0.997 (0.989–1.002) | 434 / 434 |
| runtime-floating | 496.70 / 496.33 | 1.000 (0.998–1.005) | 230 / 230 |
| runtime-discard-scalar | 132.55 / 132.61 | 0.999 (0.963–1.003) | 249 / 249 |
| runtime-discard-class | — / 276.17 | final only | — / 1240 |

Compiler `.text`: **1,973,766 → 1,981,638 bytes**, **+7,872 (+0.399%)**.

## Required work and bounded costs

Discard source-form classification consumes only the selected expression and its immediate child facts: O(1) work per completed expression/query. The bit occupies existing packed flag storage; host record-layout inspection confirms `Expression` stays **36 bytes**. Lowering no longer allocates a second NodeId-sized syntax-classification cache. The sparse volatile-class conversion index belongs to the translation unit; complete contextual NodeId identity separates checked recipes from concrete temporary/lifetime records. Function-local lowering temporaries retain the existing release boundary.

| Class template scale | Copy selections | Shared recipe uses | Materializations | Expression work | LowIR instructions |
|---|---:|---:|---:|---:|---:|
| discard-class-150 | 1 | 150 | 150 | 466 | 2434 |
| discard-class-600 | 1 | 600 | 600 | 1816 | 9634 |

The class-discard baselines omit required copy/destructor effects and fail the native preflight. Their final costs are therefore necessary semantic work, not an optimization regression or speedup comparison. Invalid/deleted/inaccessible copies fail through compact query results; actual evaluated uses demand only the selected constructor/destructor and materialize once. Functional void queries consume the same conversion rules. No grammar replay, global retry, optional optimization pass, code cloning or growth allowance was added. Existing initializer and named-result limits are unchanged.

The comparable ordering, common loop/float, calls, memory, floating, scalar-discard and dormant-storage cases retain exact LowIR (and exact native bytes where emitted). Their runtime differences cannot establish generated-code improvement. Compiler ratios, A/A spread, CPU time and RSS remain visible for assessing the added required frontend work. No performance speedup is claimed.

PA18/O0 has no mandated numeric latency/RSS ceiling. Historical +15%, +16 MiB and 5.5× targets remain diagnostics, with old measurements preserved. Correctness, coverage and bounded work remain requirements. There is no new optional transform needing a profitability allowance; required volatile accesses and class copies cannot be removed to improve timing. Later native/runtime implementation constraints are not new PA18 exit gates.
