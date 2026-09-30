# PA23/O0 performance — handoff 123

Implementation `6a762f25`, compared with entry `9688f9e5`. [All sealed observations](../student.tests/pa23/performance123.json) freeze source inputs, hashes, source flags, binaries, CPU affinity, platform, output hashes and telemetry. The binaries remain at `/tmp/pa23-123/{entry,sealed-final}`. The [pre-containment-cache series](../student.tests/pa23/performance123-before-containment-cache.json) and [pre-RTTI-hint series](../student.tests/pa23/performance123-before-rtti-hint.json), with their distinct binaries, are preserved; they are not relabeled as final.

[`benchmark123.py`](../student.tests/pa23/benchmark123.py) uses one warmup per lane, four A/A samples and four ABBA blocks. Compiler wall time and peak RSS are measured separately from checked native execution and text size. Runtime loops use volatile iteration counts and checked sums. Common templates, calls, memory, floating point and member workloads are frozen from PA22 evidence. Source compilation uses `--emit-lowir -O0`; the compiler build uses the existing `g++ -std=gnu++11 -Wall -O3` configuration. The supplied backend compiles our LowIR; the host linker links those objects. Telemetry and explicit validation preserve LowIR hashes.

## Comparable correct workloads

Times are median milliseconds, RSS is peak KiB, text is bytes. Each paired range includes every block. Small compiler TUs and startup-only executable checks support no speed claim.

| Input | Compiler A → B ms | RSS A → B KiB | Compiler paired B/A [range] | Runtime A → B ms | Runtime paired B/A [range] | Text A → B |
|---|---:|---:|---:|---:|---:|---:|
| auto-specializations-9600 | 720.02 → 692.27 | 107588 → 107624 | 0.952 [0.900–1.192] | 4.52 → 4.51 | 0.960 [0.750–1.061] | 768286 → 768286 |
| runtime-calls | 7.06 → 7.30 | 6192 → 6388 | 0.937 [0.863–1.138] | 146.58 → 155.78 | 1.032 [0.938–1.424] | 430 → 430 |
| runtime-memory | 8.25 → 7.81 | 6156 → 6336 | 0.979 [0.876–1.032] | 81.86 → 81.06 | 0.978 [0.956–1.017] | 658 → 658 |
| runtime-floating | 6.57 → 6.61 | 6372 → 6592 | 1.021 [0.751–1.139] | 89.63 → 90.97 | 1.006 [0.997–1.019] | 454 → 454 |
| runtime-member | 8.70 → 8.19 | 6256 → 6424 | 0.950 [0.885–1.030] | 254.56 → 267.35 | 1.088 [0.974–1.316] | 488 → 488 |
| member-functions-2048 | 227.88 → 238.46 | 28184 → 28216 | 1.101 [1.005–1.148] | 4.89 → 4.92 | 1.027 [0.814–1.239] | 184632 → 184632 |
| runtime-virtual-single | 9.64 → 8.17 | 6192 → 6324 | 0.864 [0.697–1.050] | 119.03 → 126.29 | 1.086 [0.946–1.156] | 550 → 570 |
| runtime-nonpoly-single | 8.23 → 8.86 | 6140 → 6404 | 1.004 [0.913–1.372] | 82.45 → 84.27 | 1.011 [0.963–1.069] | 372 → 434 |

## Noise and correctness costs

- **auto-specializations-9600, compiler:** A/A 709.51–750.59 ms; A range 684.27–798.43 ms; B range 681.38–856.38 ms. Paired ratios: 1.192, 0.986, 0.918, 0.900.
- **runtime-member, runtime:** A/A 236.53–252.02 ms; A range 245.74–279.49 ms; B range 252.53–339.96 ms. Paired ratios: 0.985, 0.974, 1.192, 1.316.
- **runtime-virtual-single, runtime:** A/A 90.36–104.45 ms; A range 86.18–139.04 ms; B range 85.87–157.49 ms. Paired ratios: 0.946, 1.048, 1.123, 1.156.
- **runtime-nonpoly-single, runtime:** A/A 77.74–84.92 ms; A range 75.06–105.01 ms; B range 77.45–103.81 ms. Paired ratios: 0.963, 1.069, 0.995, 1.026.

Common workloads retain byte-identical native text. The member-call timing varies despite identical code; paired ranges and the preserved series disclose this execution noise instead of attributing it to code changes. The template workload likewise supports no frontend speedup claim. Every observation is retained.

The affected standalone virtual-dispatch workload grows **550→570 text bytes** and its median runtime changes **119.03→126.29 ms**. It now recovers a virtual-base receiver through the object table, including when its most-derived layout differs. The nonpolymorphic virtual-base access workload grows **372→434 bytes**, with median runtime **82.45→84.27 ms**. It adds the layout pointer/initialization and dynamic field projection needed for correct references. Entry and final execute the same checked standalone programs; the entry representation cannot handle the separate shared-base reducer. These are required O0 semantic/contract costs, not optional optimizations. No inlining or devirtualization transform is proposed or accepted on these results.

Compiler text grows **2,313,478→2,328,838 bytes** (+15,360, 0.66%). Across comparable inputs, the largest peak-RSS increase is **264 KiB**. The new facts add compact virtual anchors, interned edge paths, layout extents and segment fields. Containment is completion-local and reused across virtual signatures.

## New-capability work and growth

Virtual-declaration scaling uses only the correct final compiler: the entry does not resolve the shared final-overrider graph correctly, so it is not an A/B profit baseline. Main executes successfully; its startup-dominated runtime is only an emission check.

| Independent diamonds | Compiler ms | RSS KiB | Runtime check ms | Native text | Slot work | Overrider work | LowIR instructions |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 64 | 30.94 | 9012 | 7.08 | 6512 | 1088 | 704 | 961 |
| 256 | 95.01 | 16912 | 6.52 | 25328 | 4352 | 2816 | 3841 |
| 1024 | 688.82 | 48856 | 7.25 | 100592 | 17408 | 11264 | 15361 |

64→256→1024 diamonds produce 192→768→3072 virtual-base entries, 128→512→2048 subobject identities and 6,144→24,576→98,304 bytes in the new identity/base arenas. Class/view storage is 112,640→450,560→1,802,240 bytes. Work and retained facts follow the demanded graph and emitted output. The containment cache reduces recorded overrider work from 832/3,328/13,312 in the preserved series to 704/2,816/11,264; no wall-time benefit is claimed from those separate series.

Budgets are structural at PA23/O0: one complete-object placement per virtual base, one recorded projection per canonical base path, bounded primary slots per required signature/ABI entry, segment-local rows, two candidate scans, one publication scan and cached receiver-path checks, and at most linear completion-local cache entries. Table construction is linear in emitted rows; dynamic projections and result thunks add a bounded number of operations per consumed fact. There is no optional optimizer, fixed-point pass, speculative cloning or code-growth transform. New implementation does not invalidate unrelated caches.

Spec §9’s stage-scoped acceptance applies. Historical +15% latency, +16 MiB RSS and 5.5× scaling diagnostics remain self-selected diagnostics, not additional gates. Their prior measurements remain preserved in performance 121/122. Correctness, coverage, mandated complexity/growth limits and evidence remain required. Native selection/allocation/debug and self-host performance belong to their later stages. This handoff accepts necessary bounded semantic costs and claims no optimization profit.

## Focused latency repeat

The sealed main series showed a 1.101 median paired compiler ratio on `member-functions-2048` (all four blocks above 1), so the same frozen binaries and input were repeated for eight ABBA blocks. [Every repeat observation](../student.tests/pa23/performance123-repeat.json) is retained. Compiler medians are 155.92→155.89 ms, peak RSS 28184→28248 KiB, A/A 157.15–189.45 ms; paired ratios are 1.091, 1.005, 0.950, 0.981, 0.936, 1.009, 1.005, 0.988. The paired median is 0.996. The repeat does not establish a repeatable regression or speedup; the initial slower observations remain visible.
