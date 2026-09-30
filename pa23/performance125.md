# PA23/O0 performance — implementation 125

Implementation: `d9179e848e8b6a5b9ca378a0ee20152404ce166a`.
[Main observations](../student.tests/pa23/performance125.json) and
[the inherited forest](../student.tests/pa23/performance125-forest.json)
retain every input, observation, flag, compiler/backend/output hash, checked
exit, A/A calibration, paired result and spread. Frozen binaries are
`/tmp/pa23-125/{entry,final}-cppgm++`, SHA-256:

- A: `24688bccd2cc1b6926654b33d9aec6a8f92ddcdce419ef1f898fa2aca2022106`
- B: `979a332fdeb3133af19b09d93bfbf4cb061236b59573e682b2c4a747fda5acaa`

[benchmark125.py](../student.tests/pa23/benchmark125.py) and the unchanged
[benchmark124_forest.py](../student.tests/pa23/benchmark124_forest.py) warm
both lanes, record four A/A observations and four ABBA blocks on one pinned
CPU. Compilation and checked execution are separate; `/usr/bin/time` measures
peak RSS. Flags are `--emit-lowir -O0`; compiler build remains
`g++ -std=gnu++11 -Wall -O3`. The supplied backend translates our LowIR and the
host links its objects. Telemetry plus validation preserves the LowIR hash;
its separate observation is not substituted for plain compilation RSS.
No builds, correctness suites or other benchmark runs overlapped these series.

All **19** inherited workloads have equivalent correct executable behavior and
identical native `.text` bytes. **Six** additional workloads measure newly
implemented behavior using B alone; A is an invalid baseline for those required
semantics/ABI. Volatile iteration counts and checked sums keep the runtime
workloads live. Other execution timings are startup-sensitive emission checks,
not runtime-profit evidence. Large template, inheritance-path and forest inputs
dominate compiler startup. No optimization or runtime speedup is claimed.

## Measurements

Times are median milliseconds, RSS is peak KiB, native text is bytes. Ratios
are median paired B/A and full block range, not ratios of unpaired medians.
Final-only lanes show B's measurements and have no A/B ratio.

| Workload | Compile A → B ms | RSS A → B | Compile B/A [range] | Runtime A → B ms | Runtime B/A [range] | Native text A / B |
|---|---:|---:|---:|---:|---:|---:|
| auto-specializations-9600 | 710.11 → 721.01 | 106804 → 106856 | 1.030 [0.888–1.070] | 4.11 → 4.11 | 0.994 [0.983–1.032] | 768286 / 768286 |
| runtime-calls | 6.30 → 6.27 | 6148 → 6320 | 0.993 [0.978–6.230] | 122.94 → 122.35 | 0.986 [0.932–1.002] | 430 / 430 |
| runtime-memory | 6.51 → 6.58 | 6260 → 6292 | 1.011 [0.978–1.037] | 73.29 → 73.29 | 1.001 [0.992–1.009] | 658 / 658 |
| runtime-floating | 5.87 → 5.88 | 6360 → 6416 | 0.996 [0.987–1.008] | 86.14 → 85.89 | 0.996 [0.992–0.999] | 454 / 454 |
| runtime-member | 5.90 → 5.96 | 6308 → 6428 | 1.018 [1.001–1.049] | 210.36 → 211.00 | 1.010 [0.996–1.091] | 488 / 488 |
| member-functions-2048 | 153.28 → 153.65 | 27684 → 28632 | 1.005 [0.978–1.051] | 4.11 → 4.15 | 1.007 [0.994–1.020] | 184632 / 184632 |
| runtime-virtual-single | 6.26 → 6.31 | 6264 → 6428 | 1.011 [0.985–1.020] | 67.65 → 67.53 | 0.995 [0.988–1.000] | 570 / 570 |
| runtime-nonpoly-single | 6.39 → 6.43 | 6312 → 6444 | 1.005 [0.999–1.028] | 66.20 → 66.36 | 1.003 [0.999–1.018] | 434 / 434 |
| runtime-this-downcast | 6.33 → 6.26 | 6304 → 6352 | 0.991 [0.987–1.001] | 171.70 → 171.43 | 1.004 [0.990–1.100] | 396 / 396 |
| runtime-secondary | 6.75 → 7.01 | 6332 → 6328 | 1.031 [0.999–1.090] | 586.05 → 574.50 | 0.988 [0.908–1.036] | 632 / 632 |
| runtime-crosscast | 6.75 → 6.83 | 6352 → 6300 | 1.006 [0.999–1.014] | 365.18 → 366.03 | 1.000 [0.987–1.021] | 634 / 634 |
| base-paths-256 | 20.79 → 20.91 | 8708 → 8836 | 0.999 [0.990–1.015] | 3.88 → 3.81 | 1.000 [0.956–1.054] | 4900 / 4900 |
| base-paths-1024 | 92.57 → 92.72 | 15808 → 16012 | 0.999 [0.963–1.012] | 3.95 → 4.01 | 1.004 [0.954–1.050] | 18724 / 18724 |
| base-paths-4096 | 961.71 → 977.71 | 45244 → 45748 | 1.016 [1.006–1.028] | 3.76 → 3.76 | 1.004 [0.993–1.031] | 74020 / 74020 |
| shared-depth-4 | 6.04 → 6.03 | 6316 → 6288 | 1.000 [0.997–1.007] | 3.72 → 3.72 | 1.003 [0.997–1.027] | 280 / 280 |
| shared-depth-8 | 6.45 → 6.37 | 6280 → 6340 | 0.986 [0.981–1.046] | 3.74 → 3.78 | 1.007 [0.975–1.008] | 280 / 280 |
| shared-depth-12 | 6.69 → 6.69 | 6332 → 6340 | 1.011 [0.941–1.021] | 3.81 → 3.82 | 0.998 [0.976–1.032] | 280 / 280 |
| shared-depth-16 | 8.83 → 9.19 | 6564 → 6560 | 1.038 [0.895–1.074] | 4.33 → 4.34 | 1.009 [0.989–1.023] | 280 / 280 |
| new-runtime-member-pointer | 7.84 | 6428 | — | 151.36 | — | 1014 |
| new-runtime-lifecycle | 6.77 | 6276 | — | 40.79 | — | 1739 |
| new-runtime-value-abi | 6.16 | 6312 | — | 76.38 | — | 547 |
| new-construct-depth-4 | 7.53 | 6524 | — | 4.99 | — | 1910 |
| new-construct-depth-8 | 12.56 | 7768 | — | 5.05 | — | 5744 |
| new-construct-depth-12 | 25.29 | 10476 | — | 5.26 | — | 12801 |
| shared-forest-512-depth8 | 262.32 → 270.32 | 54632 → 58016 | 1.037 [1.006–1.045] | 3.79 → 3.79 | 1.003 [0.991–1.119] | 16632 / 16632 |

A/A compiler ranges are **677.90–728.90 ms** for templates,
**956.95–970.12 ms** for 4096 paths and **259.01–266.55 ms** for the forest.
The forest's A range is **255.10–270.57 ms**, B **267.63–279.09 ms**;
paired ratios **1.045, 1.006, 1.041, 1.034** show a small consistent cost.
The path workload has paired ratios **1.006–1.028** and A/B ranges
**953.95–969.67 / 963.14–992.80 ms**. Template ranges overlap broadly:
**681.38–823.43 / 702.59–796.30 ms**. These increases are disclosed, not
relabelled as improvements. Tiny compiler inputs are startup-sensitive:
`runtime-calls` includes a retained **74.71 ms** B outlier, which causes its
**6.230** block ratio despite a **6.27 ms** median. No sample was discarded.

New runtime-member-pointer observations range **129.33–241.84 ms**, with
calibration **218.09–242.64 ms**; lifecycle observations **40.58–41.11 ms**,
calibration **40.37–82.70 ms**; value-ABI observations **75.36–78.69 ms**,
calibration **78.71–80.49 ms**. They check behavior and record absolute cost;
the observed noise and lack of a correct entry lane preclude speed claims.
Their compile ranges are **7.18–8.22**, **6.64–7.01**, **6.05–6.37 ms**.
All other calibration ranges and raw observations remain in the linked JSON.

Compiler `.text` grows **2,333,190→2,368,262 bytes**, **+35,072 (1.50%)**, for
required lifecycle, parameter ABI and dispatch implementation. The forest's
**+3,384 KiB** is the largest common-input RSS increase. It now retains **74,241**
lifecycle records in **3,145,728 bytes** of capacity, explaining most of that
increase; its **82,944** virtual views, **6,657** instructions and **371,560**
LowIR bytes are unchanged. The 4096-path input adds **4,097** lifecycle records
(**196,608** capacity bytes), has **+504 KiB** RSS, and unchanged output. Ordinary
member-heavy input adds **948 KiB** peak RSS; template input adds **52 KiB**.
There is no new optional transform to justify these costs with runtime profit:
they implement the completed class and call contracts with bounded ownership.

## Required semantic work and explicit bounds

- Each completed class publishes one contiguous lifecycle fact per shared
  virtual base and direct nonvirtual base. Store-order sorting is
  O(views log views); construction dependencies publish once per canonical
  class. TU vectors have geometric capacity and release with the TU.
- Each physical construction segment has one program identity and contains
  its required vcall rows, virtual-base offsets, offset-to-top, RTTI and slots.
  Each complete VTT contains its direct-base slices, secondary pointers and
  each virtual sub-VTT once. Nested slices omit virtual tails. Work and storage
  track emitted rows and distinct layout facts, without traversing duplicate
  shared inheritance paths or retaining alternate bodies. Rooted VTT shape is
  preserved for explicit LowIR and separate/merged-TU consistency, including
  view-only/key-definition output. Native table pruning belongs downstream.
- Each program signature stores at most one hidden fact per by-value parameter
  and its shared virtual base. A call adds one pointer argument and index
  instruction per fact, plus an address instruction only when needed. Existing
  visible arguments are copied linearly. References/pointers add zero facts.
  Bindings and argument scratch release with function lowering.
- Each addressed virtual member declaration has at most one program-owned
  dispatch callable. Its body has at most five logical instructions: load,
  optional slot index, load, call, return. It forwards prepared arguments in
  O(argument count) and never clones the source body. Ordinary adjustor identity
  remains program-owned. No new inlining, unrolling or fixed-point transform is
  introduced; inherited cleanup/array growth caps remain unchanged.
- Lifecycle scheduling scans reserved emission identities once. Construction,
  destruction and transfer entries visit their published actions; complete and
  base identities implement distinct required behavior. There is no retry pass
  over unrelated declarations. Later native, debug and host ABI requirements
  remain explicit stage boundaries.

Actual construction controls show the cost of required emitted structures:

| Shared graph depth | Lifecycle records | Views | Instructions | LowIR bytes | IR capacity bytes | Compile B ms | Peak RSS KiB | Native text bytes |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 4 | 42 | 50 | 401 | 49,511 | 274,568 | 7.53 | 6,524 | 1,910 |
| 8 | 146 | 162 | 1,129 | 250,318 | 1,110,280 | 12.56 | 7,768 | 5,744 |
| 12 | 314 | 338 | 2,241 | 771,958 | 3,526,920 | 25.29 | 10,476 | 12,801 |

These sources have overlapping class closures; their total table rows need not
be linear in source depth. Each distinct complete class needs its own physical
layout and construction view. The counts show bounded growth in those produced
facts, retaining the prior audit's shared-path coalescing. All three construct
an object and verify virtual dispatch, rather than merely declaring a graph.
Compile observation ranges are **7.25–8.38 / 12.36–13.98 / 24.73–26.43 ms**.

Acceptance follows spec §9 for **PA23/O0**. Required correctness and emitted-IR
complexity/growth bounds are preserved. Prior **+15% latency**, **+16 MiB RSS**
and **5.5× scaling** targets were self-selected diagnostics, not stage-mandated
limits; their classification and rationale from [performance124](performance124.md)
continue to apply. Every earlier measurement remains intact. There are no new
optional optimizations or claims that smaller IR proves runtime benefit.
Native selection/allocation, later debug and self-hosting performance remain
with PA24/PA33/PA34, without waiving their eventual requirements.
