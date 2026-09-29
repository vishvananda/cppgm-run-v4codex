# PA18 final audit performance

Acceptance: **PA18/O0 LowIR**, spec §9. Frozen handoff `7000a4de` is compared
with reviewed code `f2558ff4`. The [final record](../student.tests/pa18/loop89-performance-complete.json)
contains **1398 observations across 54 workloads**. The
[initial record](../student.tests/pa18/loop89-performance.json),
[ordering repeat](../student.tests/pa18/loop89-performance-ordering-repeat.json) and
[scalar-frame repair run](../student.tests/pa18/loop89-performance-final.json) remain: **4260 observations**
in this audit. Earlier [performance 86](performance86.md) and
[performance 87](performance87.md), including their unsuccessful observations,
remain unchanged. [benchmark89.py](../student.tests/pa18/benchmark89.py) and
[report89.py](../student.tests/pa18/report89.py) reproduce the measurements and tables.

The corpus retains every audit-86 and boundary-87 source, plus six pack-prefix
scaling inputs and a checked runtime loop. Each comparable workload has a
warmup per binary, four A/A observations and four ABBA blocks. Invalid entry
implementations retain their failures and get six final-only observations.
Compiler and executable timings are separate wall times with peak RSS from
`/usr/bin/time`; all native results are checked. Flags, sources, hashes, CPU
affinity, observations, spread and preflight telemetry are retained. No test
suite ran during timing; telemetry is disabled in timed compiler invocations.

The native execution boundary is the explicitly invoked supplied
`lowir2native-ref -O0`, bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
Compiler size is actual `.text`; the sectionless native ELF metric is its
**loadable payload**, including data, not isolated native `.text`.
Times are median milliseconds; RSS is peak KiB. Startup-sized rows are
diagnostics. Startup A/B: **5.63 / 5.45 ms**.

| Workload | Compiler A / B ms | Paired B/A (range) | A / B KiB | A / B native payload bytes |
|---|---:|---:|---:|---:|
| ordering-600 | 56.03 / 55.71 | 0.991 (0.703–1.005) | 15416.00 / 15568.00 | 40842 / 40842 |
| ordering-2400 | 214.87 / 214.60 | 1.002 (0.991–1.059) | 43628.00 / 43812.00 | 163242 / 163242 |
| common-loop-float-1500 | 156.02 / 155.38 | 0.995 (0.977–1.006) | 30300.00 / 30468.00 | — / — |
| runtime-calls | 6.10 / 5.77 | 0.944 (0.939–0.949) | 6020.00 / 6076.00 | 206 / 206 |
| runtime-memory | 6.11 / 5.88 | 0.955 (0.951–0.977) | 5972.00 / 6076.00 | 434 / 434 |
| runtime-floating | 5.87 / 5.62 | 0.963 (0.952–0.970) | 6044.00 / 6232.00 | 230 / 230 |
| wide-array-600 | 54.74 / 54.34 | 1.002 (0.979–1.072) | 13600.00 / 13792.00 | 45712 / 45712 |
| converted-array-600 | 84.80 / 84.12 | 0.995 (0.985–1.002) | 18452.00 / 18712.00 | 45712 / 45712 |
| deduced-rows-600 | 69.04 / 68.31 | 0.989 (0.977–1.688) | 15908.00 / 16184.00 | 78104 / 78104 |
| pack-bound-600 | 65.07 / 65.02 | 1.000 (0.984–1.016) | 14568.00 / 15056.00 | 37308 / 37308 |
| wide-array-2400 | 212.03 / 214.11 | 1.007 (1.000–1.013) | 37396.00 / 37624.00 | 182512 / 182512 |
| converted-array-2400 | 344.93 / 345.59 | 0.997 (0.970–1.010) | 56128.00 / 56432.00 | 182512 / 182512 |
| deduced-rows-2400 | 269.80 / 269.36 | 0.996 (0.986–1.002) | 46516.00 / 46748.00 | 312104 / 312104 |
| pack-bound-2400 | 259.92 / 259.62 | 0.997 (0.991–1.081) | 41220.00 / 40912.00 | 148908 / 148908 |
| runtime-array-wide | 6.36 / 6.14 | 0.967 (0.950–0.970) | 6020.00 / 6108.00 | 320 / 320 |
| runtime-array-conversion | 6.56 / 6.32 | 0.964 (0.956–0.973) | 5984.00 / 6076.00 | 320 / 320 |
| empty-aggregate-600 | 24.92 / 24.62 | 0.988 (0.980–0.995) | 8816.00 / 9008.00 | 18139 / 18139 |
| empty-aggregate-2400 | 81.90 / 81.38 | 0.993 (0.988–1.001) | 18184.00 / 18320.00 | 72139 / 72139 |
| delegation-150 | 30.84 / 30.52 | 0.980 (0.976–0.991) | 9844.00 / 10136.00 | 12701 / 12701 |
| delegation-600 | 108.65 / 107.53 | 0.991 (0.987–0.999) | 21460.00 / 21736.00 | 50501 / 50501 |
| runtime-empty-aggregate | 6.42 / 6.13 | 0.964 (0.931–0.997) | 6004.00 / 6072.00 | 201 / 201 |
| runtime-delegation | 6.17 / 5.81 | 0.946 (0.928–0.955) | 5988.00 / 6124.00 | 256 / 256 |
| empty-lifetime-600 | 36.66 / 35.96 | 0.979 (0.961–1.418) | 11244.00 / 11472.00 | 108768 / 108768 |
| discard-scalar-150 | 18.88 / 18.51 | 0.980 (0.974–0.983) | 8068.00 / 8248.00 | 11212 / 11212 |
| discard-class-150 | 14.40 / 14.05 | 0.978 (0.963–1.004) | 7292.00 / 7576.00 | 46992 / 46992 |
| storage-dormant-150 | 15.77 / 15.43 | 0.974 (0.946–0.982) | 7292.00 / 7544.00 | 3105 / 3105 |
| discard-scalar-600 | 55.92 / 55.82 | 0.994 (0.952–1.004) | 14920.00 / 15140.00 | 44512 / 44512 |
| discard-class-600 | 37.70 / 37.26 | 0.998 (0.981–3.890) | 11900.00 / 12088.00 | 185592 / 185592 |
| storage-dormant-600 | 44.70 / 44.42 | 0.993 (0.992–1.004) | 12092.00 / 12252.00 | 12105 / 12105 |
| runtime-discard-scalar | 6.04 / 5.81 | 0.964 (0.949–0.965) | 5996.00 / 6184.00 | 249 / 249 |
| runtime-discard-class | 6.19 / 5.97 | 0.965 (0.956–0.974) | 6012.00 / 6188.00 | 1240 / 1240 |
| audit-effects-150 | 14.96 / 14.66 | 0.979 (0.955–0.993) | 7272.00 / 7512.00 | 5201 / 5201 |
| audit-lifetimes-150 | 25.80 / 25.51 | 0.991 (0.979–0.994) | 8824.00 / 9016.00 | 55328 / 55328 |
| audit-effects-600 | 39.87 / 39.15 | 0.976 (0.682–1.003) | 11644.00 / 11864.00 | 20501 / 20501 |
| audit-lifetimes-600 | 85.14 / 83.90 | 0.984 (0.981–1.007) | 18544.00 / 18828.00 | 219128 / 219128 |
| runtime-audit-lifetimes | 6.16 / 5.84 | 0.957 (0.947–0.975) | 6020.00 / 6124.00 | 1112 / 1112 |
| ellipsis-lvalue-150 | 17.93 / 17.65 | 0.982 (0.980–0.985) | 7836.00 / 7928.00 | 8515 / 8515 |
| ellipsis-fixed-150 | 14.71 / 14.31 | 0.973 (0.968–0.994) | 7280.00 / 7472.00 | 10324 / 10324 |
| ellipsis-query-150 | 13.90 / 13.43 | 0.972 (0.962–0.975) | 7032.00 / 7260.00 | 5797 / 5797 |
| alias-result-150 | 37.74 / 37.32 | 0.986 (0.984–1.030) | 10876.00 / 11056.00 | 13299 / 13299 |
| ellipsis-lvalue-600 | 52.30 / 52.39 | 1.001 (0.993–1.019) | 13448.00 / 13632.00 | 33715 / 33715 |
| ellipsis-fixed-600 | 37.91 / 37.44 | 0.985 (0.293–0.994) | 11384.00 / 11640.00 | 40924 / 40924 |
| ellipsis-query-600 | 36.04 / 35.14 | 0.978 (0.969–0.988) | 10864.00 / 11268.00 | 22897 / 22897 |
| alias-result-600 | 135.28 / 136.49 | 1.011 (0.995–1.019) | 25984.00 / 26144.00 | 52899 / 52899 |
| runtime-ellipsis-prvalue | 6.05 / 5.76 | 0.968 (0.948–1.007) | 6024.00 / 6016.00 | 208 / 208 |
| runtime-ellipsis-copy | 6.48 / 6.15 | 0.946 (0.936–0.980) | 6004.00 / 6168.00 | 1232 / 1232 |
| runtime-alias-result | 6.25 / 6.01 | 0.967 (0.950–0.970) | 6012.00 / 6188.00 | 212 / 212 |
| prefix-address-150 | — / 26.71 | final only | — / 9560.00 | — / 12105 |
| prefix-nested-150 | — / 28.97 | final only | — / 10340.00 | — / 12105 |
| prefix-address-600 | — / 89.84 | final only | — / 20456.00 | — / 48105 |
| prefix-nested-600 | — / 104.33 | final only | — / 22704.00 | — / 48105 |
| prefix-address-2400 | — / 371.07 | final only | — / 63392.00 | — / 192105 |
| prefix-nested-2400 | — / 431.48 | final only | — / 72480.00 | — / 192105 |
| runtime-prefix | — / 6.13 | final only | — / 6180.00 | — / 197 |

Compiler `.text`: **1,991,942 → 1,992,710 bytes** (+768, +0.039%).

| Workload | Runtime A / B ms | Paired B/A (range) | A / B native payload bytes |
|---|---:|---:|---:|
| runtime-calls | 714.76 / 718.04 | 1.004 (1.002–1.016) | 206 / 206 |
| runtime-memory | 416.98 / 417.01 | 0.999 (0.998–1.003) | 434 / 434 |
| runtime-floating | 495.08 / 494.98 | 1.000 (0.996–1.004) | 230 / 230 |
| runtime-array-wide | 85.83 / 85.51 | 0.993 (0.984–1.013) | 320 / 320 |
| runtime-array-conversion | 86.17 / 85.70 | 1.002 (0.987–1.022) | 320 / 320 |
| runtime-empty-aggregate | 117.67 / 117.95 | 1.002 (0.996–1.011) | 201 / 201 |
| runtime-delegation | 112.42 / 112.15 | 1.002 (0.995–1.004) | 256 / 256 |
| runtime-discard-scalar | 132.35 / 133.09 | 1.003 (1.001–1.017) | 249 / 249 |
| runtime-discard-class | 277.14 / 276.52 | 0.996 (0.993–1.001) | 1240 / 1240 |
| runtime-audit-lifetimes | 259.10 / 258.76 | 1.013 (0.990–1.027) | 1112 / 1112 |
| runtime-ellipsis-prvalue | 50.31 / 50.50 | 1.018 (0.987–1.024) | 208 / 208 |
| runtime-ellipsis-copy | 64.57 / 64.36 | 0.998 (0.977–1.026) | 1232 / 1232 |
| runtime-alias-result | 47.02 / 46.66 | 0.988 (0.879–0.991) | 212 / 212 |
| runtime-prefix | — / 40.63 | final only | — / 197 |

## Findings, work bounds and disposition

All 47 comparable workloads have byte-identical LowIR. Their native outputs
are also identical wherever the source provides an executable entry point;
the common-loop-float-1500 input measures compilation only.
Seven prefix workloads were rejected by the entry compiler, so their final
costs are reported without a speedup claim. The executable loop uses a volatile
bound and checks a nonconstant sum through the deduced function pointer.
Existing calls, memory, floating-point, array, lifetime and class-result runtime
workloads retain the same checked code and sizes.

The first measurements exposed one avoidable cost: forming an explicit-prefix
frame for scalar explicit arguments even when no pack prefix existed. The final
repair creates a frame only when an explicitly supplied pack needs it, in both
ordinary and target deduction; calls without arguments need no deduction frame.
The final pack-bound, audit-effects and
ellipsis-query counters remove the extra frames and argument tuples recorded
by the initial run. This is a demand/ownership repair, not an optional optimizer.

The initial ordering-600 compiler ratio was 1.135 (0.989–1.176); its 2400 row
was 1.003. The retained targeted repeat on the same initial binaries gives
0.997 (0.986–1.005) and 1.005 (0.984–1.006), respectively. This resolves the
smaller-row timing concern without deleting any observations. The final tables
retain all spread and disclose the final costs; no precise speedup is claimed.

| Family | Size | Candidates | Expansion lanes | Body transitions | Instructions |
|---|---:|---:|---:|---:|---:|
| prefix-address | 150 | 752 | 300 | 150 | 1663 |
| prefix-address | 600 | 3002 | 1200 | 600 | 6613 |
| prefix-address | 2400 | 12002 | 4800 | 2400 | 26413 |
| prefix-nested | 150 | 1352 | 300 | 150 | 1513 |
| prefix-nested | 600 | 5402 | 1200 | 600 | 6013 |
| prefix-nested | 2400 | 21602 | 4800 | 2400 | 24013 |

Work tracks actual specializations, lanes and emitted instructions. Expansion
parameter discovery is cached once per pattern; each demanded function body
transitions once. Frames and canonical arguments are TU-owned, with no syntax
replay, whole-program retry or new growth allowance. Existing O0 named-result
summaries retain their eight-wrapper bound, empty-helper omission its one-head
check and zero-growth policy, and constant evaluation its work/depth budgets.
Their earlier profitability evidence and required array-copy costs remain in
performance 82 and 86; this audit preserves their output exactly.

PA18 mandates no numerical compiler latency/RSS ceiling. Historical +15%,
+16 MiB and 5.5× targets remain diagnostics, not exit gates. Required prefix
deduction costs have bounded work; the identified unnecessary frame work is
removed. No unprofitable optional transform, runtime regression hidden by IR
size, or relaxed correctness/comparison requirement is accepted. Native
optimization/debug, hosted aggregate-varargs retrieval and self-hosting remain
later-stage ownership. Stage-scoped performance acceptance is satisfied.
