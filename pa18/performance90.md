# PA18 audit 90 performance

Acceptance is **PA18/O0 LowIR**, spec §9. Frozen entry `f5c942ff`
is compared with repair `fe4a11f0` over **99 workloads**,
with **2563 observations** in [the full record](../student.tests/pa18/loop90-performance.json).
[benchmark90.py](../student.tests/pa18/benchmark90.py) and
[report90.py](../student.tests/pa18/report90.py) reproduce this evidence.
All 54 audit-89 sources and 34 distinct audit-82 sources remain unchanged;
eleven nonfinal-pack workloads are added. The added inherited corpus directly
checks the named-result optimization, retained calls and signature/query costs.
Prior [performance 89](performance89.md), including its unsuccessful observations,
and inherited profitability evidence remain unchanged.

Binaries, source hashes, flags, CPU affinity and all observations are retained.
Each comparable workload has one warmup per binary, four A/A calibration samples,
and four wall-time ABBA blocks. Entry rejections receive six final-only samples.
Compilation and execution are measured separately with telemetry disabled;
preflights validate LowIR and check native results. No validation or build runs
concurrently with timing. Startup A/B: **6.24 / 6.18 ms**.
Startup-sized compiler rows are diagnostic; they do not establish a speedup.

The supplied `lowir2native-ref -O0` is used only by the explicit harness,
bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`. Compiler size is `.text`.
The sectionless native ELF exposes **loadable payload including data**; this is
a disclosed code-size proxy, not an isolated native `.text` measurement.
Times are milliseconds; RSS is peak KiB; ranges show all paired block ratios.

| Workload | Compiler A / B ms | Paired B/A (range) | A / B KiB | A / B native payload bytes |
|---|---:|---:|---:|---:|
| ordering-600 | 57.06 / 57.60 | 1.014 (0.797–1.028) | 15148.00 / 15308.00 | 40842 / 40842 |
| ordering-2400 | 218.84 / 217.11 | 0.995 (0.973–1.071) | 43084.00 / 43276.00 | 163242 / 163242 |
| member-head-600 | 94.71 / 94.10 | 0.991 (0.975–1.006) | 21296.00 / 21556.00 | 39642 / 39642 |
| member-head-2400 | 377.77 / 378.05 | 1.004 (0.991–1.047) | 68764.00 / 68876.00 | 158442 / 158442 |
| common-loop-float-1500 | 160.22 / 155.38 | 0.982 (0.676–0.988) | 30328.00 / 30404.00 | — / — |
| runtime-calls | 6.11 / 5.95 | 0.971 (0.956–0.984) | 6004.00 / 6120.00 | 206 / 206 |
| runtime-memory | 6.25 / 6.20 | 0.985 (0.973–1.010) | 6036.00 / 5984.00 | 434 / 434 |
| runtime-floating | 6.27 / 6.16 | 1.001 (0.968–1.024) | 6044.00 / 6172.00 | 230 / 230 |
| first-signature-600 | 66.83 / 68.28 | 0.996 (0.975–1.051) | 13576.00 / 13708.00 | 24 / 24 |
| first-member-600 | 138.17 / 139.50 | 1.030 (0.992–1.052) | 28172.00 / 28360.00 | 28905 / 28905 |
| qualified-alias-600 | 66.71 / 67.15 | 1.002 (0.998–1.007) | 16112.00 / 16340.00 | 20451 / 20451 |
| first-signature-2400 | 258.55 / 257.40 | 0.982 (0.957–1.012) | 38380.00 / 38600.00 | 24 / 24 |
| first-member-2400 | 579.53 / 579.47 | 1.008 (0.993–1.195) | 95408.00 / 95568.00 | 115305 / 115305 |
| qualified-alias-2400 | 267.72 / 268.50 | 1.003 (0.991–1.016) | 46208.00 / 46440.00 | 81651 / 81651 |
| runtime-first-signature | 6.15 / 6.11 | 0.993 (0.972–1.007) | 6004.00 / 6100.00 | 180 / 180 |
| member-arguments-600 | 110.14 / 105.89 | 1.031 (0.850–1.149) | 19792.00 / 19980.00 | 28951 / 28951 |
| member-arguments-2400 | 409.69 / 393.98 | 0.967 (0.922–1.027) | 61296.00 / 61460.00 | 115351 / 115351 |
| member-arguments-runtime | 7.21 / 7.00 | 0.964 (0.917–0.985) | 6028.00 / 6200.00 | 219 / 219 |
| pointer-temporaries-600 | 48.13 / 43.17 | 0.989 (0.872–1.000) | 9552.00 / 9840.00 | 20585 / 20585 |
| cast-reference-600 | 70.80 / 76.46 | 1.000 (0.973–1.182) | 16548.00 / 16808.00 | 34915 / 34915 |
| constant-reference-600 | 79.09 / 83.64 | 1.067 (0.972–1.209) | 16156.00 / 16376.00 | 24 / 24 |
| pointer-temporaries-2400 | 112.28 / 112.40 | 0.989 (0.941–1.074) | 20680.00 / 20904.00 | 81785 / 81785 |
| cast-reference-2400 | 505.05 / 519.68 | 1.074 (0.980–1.160) | 48320.00 / 48616.00 | 139315 / 139315 |
| constant-reference-2400 | 717.93 / 677.33 | 0.920 (0.902–1.264) | 47216.00 / 47380.00 | 24 / 24 |
| runtime-pointer-temporary | 12.59 / 12.30 | 0.984 (0.908–0.995) | 6012.00 / 6192.00 | 239 / 239 |
| runtime-cast-reference | 10.62 / 10.12 | 1.008 (0.909–1.153) | 5988.00 / 6076.00 | 215 / 215 |
| named-result-600 | 99.94 / 97.14 | 1.009 (0.950–1.028) | 13084.00 / 13272.00 | 12097 / 12097 |
| retained-result-600 | 135.04 / 136.85 | 1.049 (0.778–1.183) | 14948.00 / 15132.00 | 26497 / 26497 |
| named-result-2400 | 497.03 / 500.65 | 0.992 (0.950–1.060) | 35392.00 / 35688.00 | 48097 / 48097 |
| retained-result-2400 | 520.41 / 492.12 | 0.966 (0.842–1.134) | 42052.00 / 42348.00 | 105697 / 105697 |
| runtime-named-result | 9.78 / 9.86 | 1.002 (0.976–1.041) | 5988.00 / 6088.00 | 196 / 196 |
| runtime-known-branch | 17.26 / 14.38 | 0.693 (0.591–1.170) | 6004.00 / 6152.00 | 192 / 192 |
| runtime-retained-result | 16.49 / 15.96 | 0.975 (0.941–1.012) | 5988.00 / 6192.00 | 222 / 222 |
| audit-conversion-lookup-600 | 538.03 / 429.68 | 0.832 (0.651–1.093) | 19792.00 / 19992.00 | 48097 / 48097 |
| audit-base-query-600 | 262.53 / 256.49 | 1.018 (0.907–1.060) | 15868.00 / 15864.00 | 33725 / 33725 |
| audit-template-query-600 | 364.72 / 357.70 | 0.959 (0.695–1.036) | 20348.00 / 20404.00 | 33697 / 33697 |
| audit-conversion-lookup-2400 | 830.21 / 865.64 | 1.048 (0.981–1.177) | 62472.00 / 62636.00 | 192097 / 192097 |
| audit-base-query-2400 | 713.39 / 658.68 | 0.953 (0.924–1.021) | 46316.00 / 46448.00 | 134525 / 134525 |
| audit-template-query-2400 | 833.87 / 869.12 | 1.021 (1.006–1.043) | 63108.00 / 63116.00 | 134497 / 134497 |
| runtime-audit-lookup | 11.35 / 11.09 | 1.032 (0.961–1.064) | 5996.00 / 6056.00 | 200 / 200 |
| wide-array-600 | 100.06 / 108.68 | 1.090 (0.928–1.174) | 13608.00 / 13720.00 | 45712 / 45712 |
| converted-array-600 | 206.64 / 205.99 | 0.986 (0.794–1.038) | 18372.00 / 18372.00 | 45712 / 45712 |
| deduced-rows-600 | 128.70 / 130.05 | 1.028 (0.938–1.047) | 16016.00 / 15948.00 | 78104 / 78104 |
| pack-bound-600 | 123.63 / 130.34 | 0.978 (0.879–1.312) | 14680.00 / 14764.00 | 37308 / 37308 |
| wide-array-2400 | 439.85 / 444.66 | 1.024 (0.951–1.050) | 37312.00 / 37460.00 | 182512 / 182512 |
| converted-array-2400 | 843.53 / 872.91 | 1.013 (0.982–1.074) | 56032.00 / 56048.00 | 182512 / 182512 |
| deduced-rows-2400 | 606.71 / 603.32 | 0.927 (0.824–0.988) | 45708.00 / 45820.00 | 312104 / 312104 |
| pack-bound-2400 | 391.40 / 433.86 | 1.029 (0.949–1.266) | 40592.00 / 40736.00 | 148908 / 148908 |
| runtime-array-wide | 6.90 / 6.67 | 0.971 (0.948–0.995) | 6012.00 / 6000.00 | 320 / 320 |
| runtime-array-conversion | 6.82 / 6.67 | 0.987 (0.974–1.007) | 6012.00 / 6036.00 | 320 / 320 |
| empty-aggregate-600 | 26.12 / 25.62 | 0.981 (0.969–0.993) | 8816.00 / 8984.00 | 18139 / 18139 |
| empty-aggregate-2400 | 82.83 / 84.24 | 1.010 (0.997–1.030) | 18280.00 / 18444.00 | 72139 / 72139 |
| delegation-150 | 31.10 / 31.17 | 0.995 (0.974–1.011) | 9840.00 / 9984.00 | 12701 / 12701 |
| delegation-600 | 109.41 / 109.37 | 0.998 (0.988–1.079) | 21444.00 / 21416.00 | 50501 / 50501 |
| runtime-empty-aggregate | 6.49 / 6.41 | 0.982 (0.962–1.007) | 5988.00 / 6024.00 | 201 / 201 |
| runtime-delegation | 6.65 / 6.42 | 0.968 (0.963–0.993) | 5996.00 / 5988.00 | 256 / 256 |
| empty-lifetime-600 | 37.67 / 37.91 | 1.020 (0.986–1.039) | 11532.00 / 11652.00 | 108768 / 108768 |
| discard-scalar-150 | 19.21 / 19.01 | 0.987 (0.982–0.995) | 8092.00 / 8192.00 | 11212 / 11212 |
| discard-class-150 | 14.97 / 14.78 | 0.986 (0.956–1.006) | 7280.00 / 7356.00 | 46992 / 46992 |
| storage-dormant-150 | 16.26 / 16.08 | 0.981 (0.755–0.995) | 7284.00 / 7396.00 | 3105 / 3105 |
| discard-scalar-600 | 57.06 / 57.44 | 1.002 (0.998–1.065) | 14920.00 / 14860.00 | 44512 / 44512 |
| discard-class-600 | 38.35 / 38.35 | 0.997 (0.987–1.004) | 11884.00 / 12020.00 | 185592 / 185592 |
| storage-dormant-600 | 45.25 / 45.46 | 1.005 (0.989–1.013) | 12116.00 / 12248.00 | 12105 / 12105 |
| runtime-discard-scalar | 6.50 / 6.32 | 0.983 (0.969–0.999) | 6028.00 / 6012.00 | 249 / 249 |
| runtime-discard-class | 6.58 / 6.49 | 0.978 (0.977–0.994) | 6004.00 / 5924.00 | 1240 / 1240 |
| audit-effects-150 | 15.37 / 15.16 | 0.986 (0.968–0.994) | 7292.00 / 7464.00 | 5201 / 5201 |
| audit-lifetimes-150 | 26.16 / 25.98 | 0.993 (0.987–1.003) | 8816.00 / 9000.00 | 55328 / 55328 |
| audit-effects-600 | 41.28 / 41.22 | 0.997 (0.992–1.000) | 11628.00 / 11784.00 | 20501 / 20501 |
| audit-lifetimes-600 | 84.76 / 84.84 | 0.999 (0.994–1.002) | 17784.00 / 17904.00 | 219128 / 219128 |
| runtime-audit-lifetimes | 6.81 / 6.69 | 0.986 (0.967–1.021) | 6004.00 / 6044.00 | 1112 / 1112 |
| ellipsis-lvalue-150 | 18.47 / 18.26 | 0.984 (0.960–1.018) | 7744.00 / 7712.00 | 8515 / 8515 |
| ellipsis-fixed-150 | 15.14 / 14.83 | 0.991 (0.940–0.999) | 7280.00 / 7336.00 | 10324 / 10324 |
| ellipsis-query-150 | 14.16 / 14.01 | 0.984 (0.971–0.998) | 7028.00 / 7068.00 | 5797 / 5797 |
| alias-result-150 | 37.95 / 38.07 | 0.998 (0.953–1.076) | 10872.00 / 11064.00 | 13299 / 13299 |
| ellipsis-lvalue-600 | 55.03 / 54.73 | 0.990 (0.979–1.033) | 13500.00 / 13604.00 | 33715 / 33715 |
| ellipsis-fixed-600 | 38.58 / 38.37 | 0.992 (0.989–1.007) | 11612.00 / 11636.00 | 40924 / 40924 |
| ellipsis-query-600 | 35.85 / 35.97 | 1.002 (0.993–1.013) | 11128.00 / 11292.00 | 22897 / 22897 |
| alias-result-600 | 140.58 / 139.58 | 0.995 (0.511–1.019) | 25776.00 / 25700.00 | 52899 / 52899 |
| runtime-ellipsis-prvalue | 6.55 / 6.40 | 0.985 (0.969–1.009) | 6012.00 / 5980.00 | 208 / 208 |
| runtime-ellipsis-copy | 6.62 / 6.47 | 0.977 (0.974–0.984) | 6004.00 / 6004.00 | 1232 / 1232 |
| runtime-alias-result | 7.00 / 6.92 | 0.997 (0.956–0.999) | 5996.00 / 6008.00 | 212 / 212 |
| prefix-address-150 | 27.11 / 27.29 | 1.004 (0.994–1.007) | 9328.00 / 9324.00 | 12105 / 12105 |
| prefix-nested-150 | 29.76 / 30.18 | 1.046 (1.007–1.091) | 10060.00 / 10048.00 | 12105 / 12105 |
| prefix-address-600 | 96.02 / 95.48 | 0.995 (0.989–0.996) | 20260.00 / 20244.00 | 48105 / 48105 |
| prefix-nested-600 | 105.71 / 105.25 | 0.994 (0.982–1.010) | 22384.00 / 22520.00 | 48105 / 48105 |
| prefix-address-2400 | 371.89 / 377.43 | 1.012 (1.000–1.031) | 63020.00 / 63200.00 | 192105 / 192105 |
| prefix-nested-2400 | 468.88 / 453.16 | 0.992 (0.955–1.042) | 72220.00 / 72352.00 | 192105 / 192105 |
| runtime-prefix | 6.98 / 7.06 | 0.994 (0.980–1.027) | 5996.00 / 6028.00 | 197 / 197 |
| nonfinal-150 | — / 26.83 | final only | — / 9760.00 | — / 12405 |
| nonfinal-target-150 | — / 31.52 | final only | — / 10528.00 | — / 14805 |
| nonfinal-600 | — / 88.94 | final only | — / 20308.00 | — / 49305 |
| nonfinal-target-600 | — / 109.79 | final only | — / 23756.00 | — / 58905 |
| nonfinal-2400 | — / 373.46 | final only | — / 63472.00 | — / 196905 |
| nonfinal-target-2400 | — / 488.05 | final only | — / 78360.00 | — / 235305 |
| nonfinal-list-150 | — / 24.95 | final only | — / 9448.00 | — / 10905 |
| nonfinal-list-600 | — / 80.09 | final only | — / 20140.00 | — / 43305 |
| nonfinal-list-2400 | — / 325.67 | final only | — / 61868.00 | — / 172905 |
| runtime-nonfinal-list | — / 6.54 | final only | — / 5952.00 | — / 196 |
| runtime-nonfinal | 6.61 / 6.55 | 0.994 (0.984–0.997) | 6016.00 / 5968.00 | 183 / 183 |

Compiler `.text`: **1,992,710 → 2,000,006 bytes** (+7,296, +0.366%).

| Workload | Runtime A / B ms | Paired B/A (range) | A / B native payload bytes |
|---|---:|---:|---:|
| runtime-calls | 718.36 / 718.10 | 1.003 (0.998–1.005) | 206 / 206 |
| runtime-memory | 418.83 / 418.47 | 0.994 (0.983–1.001) | 434 / 434 |
| runtime-floating | 495.68 / 496.03 | 1.002 (1.000–1.005) | 230 / 230 |
| runtime-first-signature | 102.01 / 83.54 | 0.986 (0.754–1.225) | 180 / 180 |
| member-arguments-runtime | 78.84 / 78.94 | 1.022 (0.973–1.499) | 219 / 219 |
| runtime-pointer-temporary | 172.97 / 175.44 | 1.010 (0.992–1.019) | 239 / 239 |
| runtime-cast-reference | 102.28 / 101.28 | 1.009 (0.893–1.063) | 215 / 215 |
| runtime-named-result | 278.72 / 279.65 | 1.010 (0.905–1.096) | 196 / 196 |
| runtime-known-branch | 291.94 / 304.00 | 1.029 (0.971–1.187) | 192 / 192 |
| runtime-retained-result | 313.49 / 306.13 | 1.000 (0.911–1.031) | 222 / 222 |
| runtime-audit-lookup | 225.49 / 233.09 | 1.047 (1.002–1.145) | 200 / 200 |
| runtime-array-wide | 86.63 / 86.38 | 0.994 (0.988–1.000) | 320 / 320 |
| runtime-array-conversion | 86.48 / 87.35 | 1.011 (0.999–1.020) | 320 / 320 |
| runtime-empty-aggregate | 119.31 / 119.08 | 1.002 (0.994–1.020) | 201 / 201 |
| runtime-delegation | 114.35 / 113.95 | 0.999 (0.991–1.984) | 256 / 256 |
| runtime-discard-scalar | 134.53 / 134.87 | 1.005 (0.992–1.029) | 249 / 249 |
| runtime-discard-class | 277.80 / 278.47 | 1.001 (0.997–1.005) | 1240 / 1240 |
| runtime-audit-lifetimes | 263.54 / 262.06 | 0.988 (0.909–0.997) | 1112 / 1112 |
| runtime-ellipsis-prvalue | 49.92 / 50.34 | 1.006 (1.002–1.014) | 208 / 208 |
| runtime-ellipsis-copy | 66.95 / 66.94 | 0.971 (0.780–0.999) | 1232 / 1232 |
| runtime-alias-result | 47.79 / 47.71 | 0.992 (0.779–1.009) | 212 / 212 |
| runtime-prefix | 41.38 / 41.28 | 0.991 (0.968–1.005) | 197 / 197 |
| runtime-nonfinal-list | — / 49.19 | final only | — / 196 |
| runtime-nonfinal | 41.36 / 41.15 | 0.891 (0.768–2.467) | 183 / 183 |

A/A calibration is retained per workload in the full record. Representative compiler noise:

| Workload | A/A max/min | Paired B/A (range) |
|---|---:|---:|
| ordering-2400 | 1.156 | 0.995 (0.973–1.071) |
| constant-reference-600 | 1.185 | 1.067 (0.972–1.209) |
| cast-reference-2400 | 1.296 | 1.074 (0.980–1.160) |
| retained-result-600 | 1.139 | 1.049 (0.778–1.183) |
| audit-conversion-lookup-2400 | 1.088 | 1.048 (0.981–1.177) |
| wide-array-600 | 1.085 | 1.090 (0.928–1.174) |

Targeted repeats retain the same binaries, flags and source hashes; every observation is kept.

| Repeat record / workload | A / B ms | Paired B/A (range) | A/A max/min |
|---|---:|---:|---:|
| [loop90-repeat-audit-conversion-lookup-2400](../student.tests/pa18/loop90-repeat-audit-conversion-lookup-2400.json) / audit-conversion-lookup-2400 | 454.61 / 455.21 | 1.001 (0.995–1.008) | 1.130 |
| [loop90-repeat-cast-confirm](../student.tests/pa18/loop90-repeat-cast-confirm.json) / cast-reference-2400 | 269.10 / 263.21 | 0.975 (0.971–0.996) | 1.010 |
| [loop90-repeat-cast-reference-2400](../student.tests/pa18/loop90-repeat-cast-reference-2400.json) / cast-reference-2400 | 271.48 / 285.70 | 1.115 (0.768–1.249) | 1.071 |
| [loop90-repeat-constant-reference-600](../student.tests/pa18/loop90-repeat-constant-reference-600.json) / constant-reference-600 | 74.37 / 75.05 | 0.996 (0.947–1.013) | 1.038 |
| [loop90-repeat-prefix-nested-150](../student.tests/pa18/loop90-repeat-prefix-nested-150.json) / prefix-nested-150 | 29.42 / 29.73 | 1.014 (0.919–1.028) | 1.043 |
| [loop90-repeat-retained-result-600](../student.tests/pa18/loop90-repeat-retained-result-600.json) / retained-result-600 | 68.33 / 67.60 | 0.984 (0.954–1.502) | 1.028 |
| [loop90-repeat-wide-array-600](../student.tests/pa18/loop90-repeat-wide-array-600.json) / wide-array-600 | 56.69 / 55.98 | 0.974 (0.904–1.012) | 1.049 |

The full run retains 2563 observations, the repeats 308, and the interrupted initial record 330 completed observations.

## Findings and budgets

All 89 comparable inputs retain exact LowIR and executable bytes wherever a
native entry exists. Thus the earlier loops, calls, memory, floating point,
array, lifetime and named-result optimization workloads retain their code.
The one-element default runtime control already worked at entry; the other
ten newly supported inputs have final-only costs; comparing their time with
an entry rejection would not be a meaningful speedup. The new runtime loop
uses a volatile bound and checks its sum. Runtime observations on identical
executables are noise diagnostics, not claims that deduction speeds up code.

| Family | Size | Candidates | Expansion lanes | Body transitions | Instructions |
|---|---:|---:|---:|---:|---:|
| nonfinal | 150 | 752 | 300 | 150 | 1813 |
| nonfinal | 600 | 3002 | 1200 | 600 | 7213 |
| nonfinal | 2400 | 12002 | 4800 | 2400 | 28813 |
| nonfinal-target | 150 | 752 | 300 | 150 | 2263 |
| nonfinal-target | 600 | 3002 | 1200 | 600 | 9013 |
| nonfinal-target | 2400 | 12002 | 4800 | 2400 | 36013 |
| nonfinal-list | 150 | 752 | 150 | 150 | 1663 |
| nonfinal-list | 600 | 3002 | 600 | 600 | 6613 |
| nonfinal-list | 2400 | 12002 | 2400 | 2400 | 26413 |

The initial run on `34b49cab` was intentionally interrupted after the related
nonfinal-template-list defect was confirmed. Its completed observation groups
remain in [the initial record](../student.tests/pa18/loop90-performance-initial.json);
the unfinished group is not used for a claim. Final acceptance uses the complete
run on `fe4a11f0`, with the broader corrected corpus.

The apparent compiler slowdowns do not persist consistently. Constant-reference,
retained-result, conversion-lookup and wide-array repeats have paired medians
0.996, 0.984, 1.001 and 0.974. The cast-reference repeat remains noisy
(0.768–1.249); its further same-binary confirmation has A/A max/min 1.010 and
paired ratios 0.971–0.996. The primary cast run also shows the same A binary
moving from roughly 260 ms during calibration to 346–673 ms in ABBA blocks.
Prefix-nested-150 repeats at 1.014 (0.919–1.028), with A/A max/min 1.043.
These observations support no precise compiler speedup or consistent regression;
all initial/repeat samples remain visible. Source review finds no added global
work or optional transform: the required parameter/default work stays local and
proportional. The implementation growth is 7,296 compiler text bytes (0.366%);
comparable generated programs do not grow. Stage-scoped acceptance is satisfied.

Candidate-local parameter views are allocated only for nonfinal packs; work is
linear in source parameters plus explicitly supplied lanes. A completed function
owns one default-position slice, and defaults retain their existing lazy fact
states. There is no new optional optimization, retry, syntax replay or output
growth allowance. Existing O0 named-result summaries inspect at most eight
wrappers, empty-helper omission checks one action head and adds no code,
constant evaluation retains its million-step/512-depth limits, and required
array expansion retains its bounded policy. Original profitability measurements
remain in performance 82/86; this run verifies their emitted code is preserved.

PA18 has no mandated numeric compiler latency/RSS ceiling. Historical +15%,
+16 MiB and 5.5× diagnostic targets remain preserved measurements, not exit gates.
Necessary costs of newly accepted semantics are disclosed, with proportional
work and conservative rejection of failed deductions. Native selection,
allocation, optimization/debug, hosted varargs and self-hosting are later-stage
ownership, not additional PA18 exit gates.
