# PA23/O0 performance — checkpoint audit 124

Reviewed code: `36e612a07bae32eb20d10ffc76bb03529555d4bb`.
[Main observations](../student.tests/pa23/performance124.json) and the
[long shared-diamond workload](../student.tests/pa23/performance124-forest.json)
retain every observation, input, binary hash, flag, output hash, A/A calibration,
paired result and spread. The binaries are frozen at
`$RALPH_ARTIFACT_DIR/pa23-audit124/{entry,final-v2}`. Their SHA-256 values are
`c2affdc75eda82241426b85c254d853027f987b2c4a4a866cf2580f417a6e9d5` and
`24688bccd2cc1b6926654b33d9aec6a8f92ddcdce419ef1f898fa2aca2022106`.
The JSON `implementation` field records the repository HEAD at measurement
(entry `2bba7273`, before committing the fixes); the frozen final binary and
[validation manifest](../student.tests/pa23/validation124.json) identify the
reviewed implementation and its committed `dev` tree.

The two explicit harnesses are [benchmark124.py](../student.tests/pa23/benchmark124.py)
and [benchmark124_forest.py](../student.tests/pa23/benchmark124_forest.py).
Each warms both lanes, records four A/A observations and four wall-time ABBA
blocks on one pinned CPU. Compilation and checked execution are measured
separately with `/usr/bin/time` peak RSS. Source flags are `--emit-lowir -O0`;
compiler build flags remain `g++ -std=gnu++11 -Wall -O3`. The supplied backend
accepts our LowIR; the host only links its objects. Telemetry plus explicit
validation leaves each output unchanged. No concurrent builds or test suites
ran during the final measurement series.

Both lanes are correct on every measured input, with identical native `.text`
bytes on all **19** workloads. The twelve ownership failures in the new audit
controls are correctness evidence, never an incorrect A/B profit baseline.
Runtime workloads use volatile iteration inputs and checked sums. The other
executable runs are startup-sensitive emission checks, not runtime-profit
measurements. The 512-diamond forest and large template/path inputs make both
compiler lanes run long enough to dominate startup.

Times below are median milliseconds, RSS is peak KiB, text is bytes. Ratios
are median paired B/A with the full block range, not a ratio of unpaired medians.

| Workload | Compiler A → B ms | RSS A → B | Compiler B/A [range] | Runtime A → B ms | Runtime B/A [range] | Text A = B |
|---|---:|---:|---:|---:|---:|---:|
| auto-specializations-9600 | 683.01 → 682.05 | 106820 → 106932 | 0.950 [0.894–1.001] | 4.38 → 4.40 | 0.966 [0.937–1.119] | 768286 |
| runtime-calls | 6.12 → 5.87 | 6272 → 6360 | 0.962 [0.954–0.968] | 123.02 → 122.81 | 0.986 [0.920–0.999] | 430 |
| runtime-memory | 6.15 → 6.05 | 6164 → 6340 | 0.980 [0.966–0.985] | 73.02 → 73.27 | 0.989 [0.951–1.006] | 658 |
| runtime-floating | 5.97 → 5.77 | 6388 → 6404 | 0.964 [0.928–0.983] | 86.26 → 86.38 | 1.003 [0.981–1.012] | 454 |
| runtime-member | 6.37 → 6.25 | 6312 → 6272 | 0.977 [0.969–0.985] | 211.58 → 210.58 | 1.010 [0.984–1.017] | 488 |
| member-functions-2048 | 153.47 → 152.35 | 27688 → 27644 | 0.984 [0.706–1.017] | 3.82 → 3.81 | 0.999 [0.985–1.007] | 184632 |
| runtime-virtual-single | 6.43 → 6.28 | 6296 → 6388 | 0.983 [0.964–1.006] | 67.05 → 66.90 | 0.999 [0.997–1.061] | 570 |
| runtime-nonpoly-single | 5.81 → 5.70 | 6300 → 6340 | 0.975 [0.958–0.986] | 66.17 → 65.95 | 0.998 [0.995–1.002] | 434 |
| runtime-this-downcast | 6.27 → 6.15 | 6176 → 6412 | 0.982 [0.977–1.016] | 171.61 → 172.35 | 1.005 [0.995–1.009] | 396 |
| runtime-secondary | 6.26 → 6.13 | 6176 → 6256 | 0.979 [0.962–0.987] | 267.60 → 271.50 | 1.007 [0.986–1.049] | 632 |
| runtime-crosscast | 6.39 → 6.26 | 6312 → 6324 | 0.971 [0.954–0.979] | 362.14 → 365.47 | 1.004 [0.999–1.059] | 634 |
| base-paths-256 | 21.08 → 20.74 | 8668 → 8700 | 0.983 [0.980–0.988] | 3.79 → 3.87 | 1.019 [0.997–1.050] | 4900 |
| base-paths-1024 | 93.94 → 92.31 | 15880 → 15972 | 1.046 [0.980–1.236] | 3.60 → 3.62 | 1.001 [0.966–1.023] | 18724 |
| base-paths-4096 | 1010.70 → 973.82 | 45652 → 45524 | 0.968 [0.943–0.990] | 3.71 → 3.67 | 0.992 [0.988–0.997] | 74020 |
| shared-depth-4 | 5.98 → 5.76 | 6332 → 6376 | 0.965 [0.941–0.969] | 3.62 → 3.63 | 0.995 [0.990–0.999] | 280 |
| shared-depth-8 | 7.31 → 6.23 | 6840 → 6324 | 0.851 [0.833–0.862] | 3.64 → 3.69 | 1.018 [1.000–1.058] | 280 |
| shared-depth-12 | 18.40 → 6.76 | 12024 → 6376 | 0.361 [0.257–0.369] | 3.59 → 3.61 | 0.988 [0.983–0.998] | 280 |
| shared-depth-16 | 195.43 → 6.98 | 98804 → 6568 | 0.035 [0.035–0.037] | 3.60 → 3.62 | 0.990 [0.977–1.009] | 280 |
| shared-forest-512-depth8 | 566.00 → 257.99 | 208624 → 54400 | 0.462 [0.409–0.493] | 3.85 → 3.84 | 1.001 [0.993–1.019] | 16632 |

The long forest gives repeatable compiler benefit: **566.00→257.99 ms**, paired
ratios **0.409, 0.493, 0.468, 0.457**, and RSS **208,624→54,400 KiB**.
A/A spans **551.26–914.84 ms**; A spans **547.70–706.85 ms**, B
**256.04–289.98 ms**. Every pair improves despite the observed noise. Retained
views fall **1,551,360→82,944**, slots **523,776→9,728**, class/view storage
**193,847,296→11,886,592 bytes**, and final-overrider work **532,480→26,624**.
This eliminates duplicate semantic facts; it does not optimize away program work.

At depths 4/8/12/16, entry views are **166/3,030/49,094/786,358** and final
views **50/162/338/578**. Entry slots are **63/1,023/16,383/262,143**, final
**11/19/27/35**. The final view count tracks the sum of distinct per-class
closures, rather than the exponentially many paths to shared subobjects.
Each incoming candidate still participates in final-overrider resolution before
physical views are coalesced; competing overrides remain diagnosed. Completion
scratch is linear in imported facts and releases after the class publishes.

The cached base-path change computes each demanded projection once. Controls
with 64/256/1,024 paths record exactly **64/256/1,024** layout computations,
replacing the old repeated full-path summation (2,080/32,896/524,800 edge visits).
The 4,096-path workload measures **1,010.70→973.82 ms**, with all four paired
ratios below one. The 1,024-path series has mixed paired ratios **0.980–1.236**;
its unpaired medians are **93.94→92.31 ms**. No speed claim is made for that
small difference or for the startup-sensitive inputs. Its spread is retained.

Compiler `.text` grows **2,328,838→2,333,190 bytes** (+4,352; 0.19%) for
program-owned ABI identities and bounded shared-view publication. The largest
RSS increase outside the improved forest/deep graphs is **236 KiB**. The common
native code is unchanged, including secondary calls/crosscasts whose timing
medians rise slightly. Those timing differences do not establish code regression
or runtime profit. No runtime speedup is claimed for these audit fixes.

[The earlier audit series](../student.tests/pa23/performance124-before-shared-views.json)
remains intact with its distinct frozen `final` binary. It predates shared-view
coalescing and briefly overlapped a small exploratory graph probe, so it is
not final acceptance evidence. All handoff-121/122/123 series and repeats remain
intact; their frozen binary hashes were independently rechecked. Their required
semantic costs (+20/+62 native text bytes for dynamic virtual-base access and
+146 bytes for the bounded two-action O0 deletion shape) remain disclosed.
The inherited `this` proof still produces 396 native text bytes; its original
428→396 byte reduction and repeated runtime evidence remain in
[performance121.md](performance121.md).

Acceptance follows spec §9 for **PA23/O0**. Historical +15% latency, +16 MiB RSS
and 5.5× scaling thresholds are unsupported diagnostic targets, not extra exit
gates. Their observations are preserved, not erased or weakened. Required
correctness, comparison coverage, linear work in consumed/produced facts and
bounded growth remain mandatory. There is no new optional optimizer, cloning,
inlining or fixed-point transform. Native selection/allocation/debug and
self-hosting benchmarks belong to PA24/PA33/PA34; they are not PA23 exit gates.
