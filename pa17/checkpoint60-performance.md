# PA17 accumulated audit performance — loop 60

Reviewed tip: `2290a7bf8b56bc33e6ad975a8ae714a341f55ce5`. Both campaigns freeze compiler code `3ad8f2f429fa72f4f314931edacc30be488e6dcd`; the subsequent evidence-verifier correction changes no `dev/` content or measured binary. The final verifier checks source equivalence and frozen hashes. The [audit campaign](../student.tests/pa17/checkpoint60-performance.json) compares entry `119fa9fe` with final. The [cumulative campaign](../student.tests/pa17/checkpoint60-cumulative-performance.json) compares the previous reviewed code `e14b96fa` with final. Each covers **44 fixed workloads**: the union of all three handoff corpora, plus receiver, access-query, constant-query and deferred-emission scaling/runtime cases. Every source, binary/backend hash, preflight, warmup, observation and untimed counter is preserved.

Host build: g++ `-std=gnu++11 -Wall -O3`, course test runner enabled. Student flags: `--emit-lowir -O0`. CPU 31 affinity; serial compiler/executable timing, with builds and test campaigns completed beforehand. Each common case uses one warmup per binary, four A/A observations and four ABBA blocks. Final-only cases use a warmup and six observations. `--stats --validate-lowir`, supplied native translation and correctness execution run outside compiler timing. Wall/CPU time, peak RSS and context switches are retained. Paired results below are the median of four within-block B/A means, with the full block range. All observations, including outliers, remain included.

Native validation uses supplied O0 backend bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA-256 `c3bae4acf3243d5a2fd6a15ef00e2d75d11a7d82715b1e4771f55165d0542490`. Runtime loops use volatile bounds and check live results. Sectionless ELF code size is executable payload after its entry minus typed global bytes: **code plus alignment**, not an ELF `.text` section. JSON also preserves full payload, global data and file sizes. Own native generation and self-hosting remain later-stage obligations.

| Compiler | SHA-256 | .text bytes |
|---|---|---:|
| Previous reviewed | `8f8dfd898edc50a919518cda43968b08e09f1e4255d128d391cfdef15c2628d7` | 1,777,798 |
| Entry | `54985333e8eddcc0198cbf9584cfa7e554bbfa3f141b04bbd19c4d0543750dfa` | 1,788,934 |
| Final | `78e664489402136e18859f1eee29e51b3549222d6c1dc8dea62646f2372e3993` | 1,791,558 |

Compiler text grows **2,624 bytes (0.147%)** from audit entry and **13,760 bytes (0.774%)** across the reviewed implementation range. Entity and query records remain **120 and 48 bytes**. The earlier candidate flag packing and fully deduced fast path removed avoidable record growth and redundant default work; their earlier measurements are preserved.

## Audit compiler observations

Times are milliseconds; RSS is KiB. The roughly 6–7 ms compilations of runtime sources are startup-sensitive and remain in JSON without supporting latency claims. “Final only” excludes an entry rejection or incorrect executable, never treating missing behavior as a faster implementation.

| Input | A ms [range] | B ms [range] | Paired B/A [range] | A/A range ms | Peak RSS A/B |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 111.21 [109.39–115.24] | 111.21 [109.84–112.78] | 0.9960 [0.9818–1.0118] | 111.02–113.46 | 22,404/22,672 |
| common-partials-6000 | 454.55 [449.64–463.23] | 458.72 [450.58–489.21] | 1.0060 [0.9957–1.0502] | 450.64–463.48 | 72,740/72,944 |
| common-loop-float-1500 | 153.13 [151.68–156.21] | 152.68 [150.81–163.09] | 0.9993 [0.9895–1.0172] | 153.37–154.88 | 30,132/30,256 |
| common-loop-float-6000 | 603.62 [597.73–783.14] | 604.64 [598.84–617.08] | 0.9561 [0.8721–1.0084] | 597.47–601.60 | 104,000/104,144 |
| specialized-transfer-600 | 106.94 [105.62–204.29] | 106.83 [104.90–109.34] | 0.9912 [0.6946–1.0067] | 105.74–109.03 | 24,572/24,660 |
| qualified-path-600 | 66.19 [65.68–67.30] | 66.42 [66.00–68.20] | 1.0059 [0.9906–1.0188] | 66.20–67.17 | 16,100/16,064 |
| wide-array-600 | 36.29 [36.12–37.49] | 36.08 [35.86–36.25] | 0.9920 [0.9808–0.9968] | 36.37–37.20 | 10,672/10,708 |
| specialized-transfer-2400 | 435.49 [431.36–455.54] | 439.54 [428.78–505.31] | 1.0336 [0.9816–1.0757] | 430.77–444.78 | 81,324/81,072 |
| qualified-path-2400 | 261.29 [257.74–276.23] | 265.60 [262.05–308.79] | 1.0104 [1.0087–1.1019] | 262.66–268.13 | 46,832/47,420 |
| wide-array-2400 | 130.58 [129.18–131.60] | 129.74 [128.25–132.11] | 0.9954 [0.9904–0.9999] | 128.56–197.32 | 25,960/25,952 |
| common-candidate-defaults-1500 | 130.86 [129.10–148.34] | 129.86 [128.09–134.57] | 0.9929 [0.9451–1.0044] | 129.25–133.06 | 29,108/29,436 |
| common-candidate-defaults-6000 | 546.56 [542.30–570.47] | 544.71 [538.20–552.70] | 0.9922 [0.9738–1.0084] | 538.46–544.87 | 100,776/100,868 |
| common-candidate-deduced-1500 | 121.46 [119.68–128.57] | 120.94 [120.26–125.65] | 0.9992 [0.9723–1.0123] | 121.32–124.36 | 28,212/28,256 |
| common-candidate-deduced-6000 | 514.79 [501.93–551.68] | 510.27 [500.49–528.79] | 0.9967 [0.9559–1.0200] | 505.57–509.75 | 96,360/96,372 |
| recursive-candidate-200 | 53.83 [53.41–56.12] | 53.84 [53.16–54.91] | 0.9942 [0.9692–1.0110] | 54.09–55.39 | 13,180/13,448 |
| recursive-candidate-800 | 204.71 [201.82–227.04] | 202.71 [201.26–214.87] | 0.9813 [0.9363–1.0284] | 205.80–222.27 | 36,376/36,384 |
| failed-operator-query-600 | 44.53 [44.08–52.30] | 44.63 [44.27–45.34] | 0.9960 [0.9203–1.0071] | 44.58–46.48 | 12,096/12,192 |
| failed-operator-query-2400 | 173.43 [169.65–181.82] | 173.61 [169.71–204.77] | 0.9945 [0.9727–1.0973] | 170.01–176.65 | 31,076/31,364 |
| static-reads-1000 | 80.81 [78.45–82.36] | 81.55 [80.06–82.60] | 1.0102 [1.0037–1.0195] | 79.31–88.30 | 18,324/18,392 |
| static-reads-4000 | 326.17 [318.42–379.44] | 324.43 [320.66–327.43] | 0.9983 [0.9181–1.0075] | 320.95–329.73 | 55,800/55,804 |
| static-signatures-400 | 68.01 [66.82–70.15] | 68.90 [67.33–70.57] | 1.0117 [0.9938–1.0192] | 67.79–70.42 | 14,620/14,664 |
| static-signatures-1600 | 265.85 [264.08–310.47] | 264.91 [263.25–296.04] | 0.9969 [0.9753–1.0175] | 265.81–271.44 | 41,144/41,148 |
| local-address-600 | 68.69 [66.53–72.63] | 68.71 [66.70–74.07] | 1.0108 [0.9595–1.0297] | 66.68–68.58 | 15,836/15,820 |
| local-address-2400 | 274.14 [266.61–297.73] | 280.47 [270.36–313.93] | 1.0153 [1.0019–1.0439] | 266.63–275.84 | 45,420/45,644 |
| audit-receiver-600 | — | 87.39 [86.63–105.84] | final only | — | —/19,964 |
| audit-address-600 | 109.61 [108.09–162.00] | 108.37 [107.69–110.85] | 0.9721 [0.7986–0.9982] | 107.22–107.97 | 20,596/20,392 |
| audit-receiver-2400 | — | 338.89 [337.27–341.72] | final only | — | —/62,280 |
| audit-address-2400 | 434.40 [429.56–480.07] | 435.77 [431.71–471.60] | 1.0022 [0.9836–1.0119] | 425.95–438.05 | 65,364/65,264 |
| audit-deferred-use-600 | — | 100.84 [100.58–103.53] | final only | — | —/20,804 |
| audit-constant-query-600 | — | 104.98 [104.16–108.04] | final only | — | —/21,760 |
| audit-deferred-use-2400 | — | 417.29 [409.88–454.24] | final only | — | —/65,664 |
| audit-constant-query-2400 | — | 427.69 [414.72–521.69] | final only | — | —/68,696 |

All **36 common audit LowIR outputs** are byte-identical. The 26 nontrivial common compiler paired medians range **0.9561–1.0336**, with peak RSS changes **−252 to +588 KiB**. The 0.9561 loop/float ratio contains two slower A observations despite 603.62/604.64 ms separate medians, so it is not a speed claim. Specialized-transfer-2400 has a disclosed +3.36% paired cost (block range 0.9816–1.0757), unchanged work counts and two slower B observations; no transform or repeated semantic work was added to it.

Local-address-2400 has a +1.53% paired cost with all four pairs positive (1.0019–1.0439). The query now retains and checks its naming/access boundary, and function-use dispatch distinguishes body evaluation depth. Query/signature/body work counts do not grow; retained object-use/type counts add only one shared record each. Qualified-path-2400 adds two cached path hits and two shared object-use records. These constant owner checks and facts are required semantics, with no duplicated traversal or optional work found to remove. The cost is disclosed without claiming an improvement or relying on a numerical threshold.

## Audit executable observations

| Input | A ms [range] | B ms [range] | Paired B/A [range] | A/A range ms | Code+alignment A/B | Data A/B |
|---|---:|---:|---:|---:|---:|---:|
| runtime-calls | 359.03 [357.90–365.66] | 359.21 [358.21–360.80] | 1.0000 [0.9900–1.0019] | 358.15–363.66 | 206/206 | 0/0 |
| runtime-memory | 211.39 [209.17–213.80] | 210.44 [209.85–214.04] | 0.9968 [0.9933–1.0035] | 209.50–212.03 | 434/434 | 0/0 |
| runtime-floating | 249.89 [249.28–251.40] | 249.94 [248.85–250.68] | 0.9997 [0.9977–1.0005] | 249.95–250.20 | 230/230 | 0/0 |
| runtime-sparse-transfer | 161.24 [159.98–161.92] | 160.91 [159.98–162.02] | 1.0008 [0.9936–1.0048] | 158.97–161.02 | 234/234 | 0/0 |
| runtime-qualified-path | 161.72 [161.38–163.70] | 162.22 [161.31–163.03] | 1.0010 [0.9962–1.0020] | 161.59–162.97 | 230/230 | 0/0 |
| runtime-wide-array-2 | 120.65 [120.18–121.50] | 120.76 [120.13–121.29] | 1.0000 [0.9967–1.0022] | 120.49–121.15 | 225/225 | 0/0 |
| runtime-wide-array-8 | 123.09 [121.74–124.63] | 123.56 [121.81–129.84] | 1.0055 [0.9980–1.0253] | 120.54–122.70 | 273/273 | 0/0 |
| runtime-wide-array-9 | 631.78 [628.11–636.05] | 630.73 [628.75–866.83] | 0.9975 [0.9960–1.1846] | 628.18–631.35 | 264/264 | 72/72 |
| runtime-recursive-candidate | 227.74 [225.98–229.35] | 229.79 [226.03–244.02] | 1.0238 [0.9919–1.0423] | 225.97–250.06 | 184/184 | 0/0 |
| runtime-static-read | 237.12 [234.22–245.36] | 238.14 [235.53–247.23] | 1.0054 [0.9902–1.0098] | 238.77–245.67 | 196/196 | 4/4 |
| runtime-audit-receiver | — | 209.17 [207.93–232.54] | final only | — | —/228 | —/0 |
| runtime-audit-deferred-use | — | 128.25 [127.13–129.04] | final only | — | —/200 | —/0 |

The **ten common audit runtime executables are byte-identical**. Their timing differences, including +2.38% recursive-candidate and a retained 867 ms wide-array-9 outlier, do not indicate changed generated code. Native peak RSS is 256 KiB in these observations. The two final-only executables check the newly valid receiver/emission behavior; they are semantic costs, not speedup claims.
## Cumulative compiler observations

| Input | A ms [range] | B ms [range] | Paired B/A [range] | A/A range ms | Peak RSS A/B |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 255.25 [248.99–279.76] | 250.64 [241.02–267.13] | 0.9938 [0.9073–1.0276] | 189.13–250.74 | 22,384/22,672 |
| common-partials-6000 | 870.38 [781.53–1063.03] | 870.91 [779.60–1265.49] | 1.0031 [0.8793–1.1375] | 780.48–899.10 | 72,772/73,116 |
| common-loop-float-1500 | 402.68 [366.86–448.96] | 397.17 [365.30–429.48] | 0.9837 [0.9489–1.0261] | 379.92–413.62 | 29,836/30,200 |
| common-loop-float-6000 | 622.94 [600.74–914.90] | 612.60 [606.44–632.47] | 0.9761 [0.7935–1.0298] | 607.48–1581.28 | 104,076/104,036 |
| specialized-transfer-600 | 109.76 [106.96–330.61] | 109.36 [108.71–114.85] | 0.9973 [0.5045–1.0247] | 107.88–109.21 | 24,348/24,524 |
| qualified-path-600 | 67.35 [66.39–67.78] | 68.42 [67.14–70.06] | 1.0196 [1.0051–1.0286] | 66.90–67.62 | 15,564/16,116 |
| wide-array-600 | 37.47 [36.92–37.97] | 37.03 [36.58–37.52] | 0.9913 [0.9789–0.9946] | 36.87–37.61 | 10,816/10,676 |
| specialized-transfer-2400 | 441.51 [434.20–510.26] | 448.32 [441.60–482.57] | 0.9908 [0.9672–1.0381] | 441.01–446.77 | 80,040/81,004 |
| qualified-path-2400 | 261.25 [256.07–267.38] | 268.00 [265.15–282.31] | 1.0378 [1.0112–1.0593] | 259.86–267.62 | 44,540/46,864 |
| wide-array-2400 | 132.00 [130.72–133.68] | 130.01 [128.83–209.61] | 0.9851 [0.9797–1.2880] | 131.05–132.54 | 26,060/26,260 |
| common-candidate-defaults-1500 | 133.26 [132.05–135.34] | 134.09 [132.32–135.96] | 1.0027 [0.9997–1.0112] | 130.16–133.30 | 29,388/29,792 |
| common-candidate-defaults-6000 | 550.63 [543.00–586.25] | 558.81 [553.83–577.95] | 1.0201 [0.9968–1.0229] | 549.20–559.50 | 100,056/101,256 |
| common-candidate-deduced-1500 | 123.57 [122.10–125.78] | 124.27 [122.01–126.04] | 1.0048 [0.9984–1.0158] | 124.25–126.14 | 28,304/28,524 |
| common-candidate-deduced-6000 | 509.57 [502.21–515.95] | 511.19 [507.13–548.55] | 1.0089 [0.9943–1.0344] | 501.21–514.92 | 95,720/96,284 |
| recursive-candidate-200 | — | 53.14 [52.76–53.64] | final only | — | —/13,220 |
| recursive-candidate-800 | — | 202.35 [198.46–204.91] | final only | — | —/35,056 |
| failed-operator-query-600 | — | 44.49 [44.13–44.63] | final only | — | —/12,196 |
| failed-operator-query-2400 | — | 169.17 [167.33–170.20] | final only | — | —/31,248 |
| static-reads-1000 | 78.24 [77.06–79.66] | 78.69 [78.12–122.34] | 1.0077 [1.0047–1.2925] | 78.45–79.74 | 18,048/18,424 |
| static-reads-4000 | 310.86 [308.04–344.15] | 318.11 [314.91–320.74] | 1.0222 [0.9664–1.0263] | 307.79–316.74 | 54,668/55,892 |
| static-signatures-400 | 68.13 [67.46–119.79] | 69.22 [68.49–69.98] | 1.0084 [0.7369–1.0134] | 66.92–70.67 | 14,384/14,596 |
| static-signatures-1600 | 258.42 [255.74–261.46] | 265.71 [264.01–278.18] | 1.0288 [1.0200–1.0594] | 259.94–262.86 | 40,764/40,992 |
| local-address-600 | — | 67.69 [66.25–68.47] | final only | — | —/15,660 |
| local-address-2400 | — | 264.83 [263.00–312.32] | final only | — | —/45,584 |
| audit-receiver-600 | — | 89.60 [88.77–90.32] | final only | — | —/19,812 |
| audit-address-600 | — | 108.84 [107.15–109.51] | final only | — | —/20,580 |
| audit-receiver-2400 | — | 344.53 [342.67–346.92] | final only | — | —/62,284 |
| audit-address-2400 | — | 432.45 [426.22–493.40] | final only | — | —/65,488 |
| audit-deferred-use-600 | — | 100.02 [99.41–101.17] | final only | — | —/20,760 |
| audit-constant-query-600 | — | 104.53 [103.86–105.43] | final only | — | —/21,960 |
| audit-deferred-use-2400 | — | 399.47 [395.72–445.58] | final only | — | —/66,140 |
| audit-constant-query-2400 | — | 420.41 [415.74–426.83] | final only | — | —/68,696 |

The cumulative campaign has **27 correct common workloads** and **17 final-only workloads**. It records 12 entry compiler rejections and five accepted but incorrect executables (the two repeated-base receiver sizes, their runtime case, and two constant-query sizes). Incorrect or rejected implementations are excluded from speed comparisons. Changed common executable outputs are checked before timing; the exact comparison mode also requires identical LowIR and executable hashes. Comparison modes and all checked outputs remain explicit in JSON.

The 18 nontrivial common compiler paired medians range **0.9761–1.0378**; peak RSS changes range **−140 to +2,324 KiB**. Qualified-path-2400 has **+3.78%** paired time (all pairs positive) and +2,324 KiB for retained base paths and qualifier boundaries. Candidate-defaults-6000 has **+2.01%** time and +1,200 KiB for required active candidate/default-query ownership. Static-signatures-1600 has **+2.88%** time, all four pairs positive, for indexed source signature matching and normalization. These bounded semantic facts are necessary; no unrelated search, grammar replay or redundant default pass remains. The historical transfer wide-array-2400 +11% compiler observation remains preserved; this cumulative run measures 0.9851× [0.9797–1.2880] and does not convert the noisy old result into a permanent gate or claim a general compiler speedup.

## Cumulative executable observations

| Input | A ms [range] | B ms [range] | Paired B/A [range] | A/A range ms | Code+alignment A/B | Data A/B |
|---|---:|---:|---:|---:|---:|---:|
| runtime-calls | 358.98 [358.46–359.84] | 359.25 [358.06–361.63] | 1.0008 [0.9987–1.0045] | 358.06–359.60 | 206/206 | 0/0 |
| runtime-memory | 210.79 [209.69–214.81] | 211.37 [210.65–212.81] | 1.0032 [0.9911–1.0052] | 209.55–211.62 | 434/434 | 0/0 |
| runtime-floating | 249.78 [248.76–261.57] | 249.34 [248.98–251.10] | 0.9998 [0.9592–1.0018] | 248.93–250.24 | 230/230 | 0/0 |
| runtime-sparse-transfer | 435.04 [432.63–448.23] | 160.51 [159.84–165.73] | 0.3667 [0.3640–0.3711] | 432.64–448.35 | 236/234 | 0/0 |
| runtime-qualified-path | 162.07 [161.90–162.84] | 162.48 [161.74–163.08] | 1.0024 [1.0005–1.0030] | 161.50–161.97 | 230/230 | 0/0 |
| runtime-wide-array-2 | 122.47 [121.53–124.67] | 120.33 [119.75–122.71] | 0.9839 [0.9740–0.9944] | 122.06–123.68 | 240/225 | 16/0 |
| runtime-wide-array-8 | 129.57 [128.11–133.12] | 124.18 [123.26–127.99] | 0.9602 [0.9552–0.9795] | 129.24–145.79 | 264/273 | 64/0 |
| runtime-wide-array-9 | 630.06 [627.76–634.25] | 630.70 [629.09–634.63] | 0.9996 [0.9984–1.0024] | 627.75–632.98 | 264/264 | 72/72 |
| runtime-recursive-candidate | — | 225.09 [224.76–225.94] | final only | — | —/184 | —/0 |
| runtime-static-read | 229.67 [228.75–232.31] | 233.76 [232.76–235.44] | 1.0197 [1.0104–1.0214] | 229.06–229.71 | 188/196 | 4/4 |
| runtime-audit-receiver | — | 207.21 [206.89–207.67] | final only | — | —/228 | —/0 |
| runtime-audit-deferred-use | — | 127.65 [126.87–131.09] | final only | — | —/200 | —/0 |

Calls, memory, floating point, qualified-path and nine-element fallback binaries are unchanged. The sparse memberwise transfer repeats the handoff benefit: **0.3667× [0.3640–0.3711]** runtime, with two fewer code bytes (236→234). The eight-element wide array repeats its benefit: **0.9602× [0.9552–0.9795]**, nine added code bytes (264→273) and 64 removed data bytes. Both comparisons execute checked equivalent results. The two-element case is 0.9839× in this campaign, but its original campaign showed no repeatable runtime benefit; it preserves the course-required direct initializer shape while removing 15 code bytes and 16 data bytes. No broader speed claim follows from it.

The ordinary static-read boundary costs **1.0197× [1.0104–1.0214]** runtime and eight code bytes (188→196), repeating storage’s disclosed +2.06% cost. The source snapshot keeps ordinary O0 reads distinct from constants established later during specialization. Removing that load would violate the required source-use/LowIR contract. It is a necessary correctness/contract cost, not an optional optimization. Supplied-backend native layout/encoding remains outside PA17 ownership. Native peak RSS is 256 KiB throughout the final campaigns.

## Work, legality and growth bounds

Fourfold audit-owner inputs have the following final costs. These are demand-size scaling observations, not numerical pass/fail thresholds.

| Owner (600→2,400) | Compiler median ms | Latency ratio | Peak RSS KiB | Selected work counters |
|---|---:|---:|---:|---|
| Qualified receiver | 87.39→338.89 | 3.878× | 19,964→62,280 | `base_adjustment_work` 1,204→4,804; `base_adjustment_paths` 604→2,404 |
| Address query | 108.37→435.77 | 4.021× | 20,392→65,264 | `type_query_work` 602→2,402; `body_checks` 602→2,402 |
| Deferred emission | 100.84→417.29 | 4.138× | 20,804→65,664 | `deferred_function_uses` 600→2,400; `deferred_function_use_work` 600→2,400 |
| Constant subobject query | 104.98→427.69 | 4.074× | 21,760→68,696 | `base_adjustment_work` 2,404→9,604; `constant_execution_steps` 9,600→38,400; `constant_address_work` 5,400→21,600 |

Each deferred selected-callee edge is processed once: 600/600 and 2,400/2,400 retained edges/work. Unused bodies and deeper unevaluated operands do not activate edges. Graph queries use cached canonical pairs; constant evaluation consumes recorded subobject paths. Static-read definition applications/edges grow 1,000→4,000 while their shared source signature is checked once. Candidate cycle/default work follows actual candidate/head/tuple keys. The source-level controls cover declaration/default publication, cache contexts, repeated qualifiers, transitive/cyclic demand and dormant erroneous bodies. The [combined trace](../student.tests/pa17/checkpoint60-trace.json) retains source, LowIR, counters and decoded ELF for these owners.

Sparse transfer consumes a recorded legal memberwise plan, preserving empty subobjects, width/sign and ordered member actions; nontrivial, volatile, reference, union and array plans retain their own rules. Wide-array selection scans explicit source initializers rather than expanding declared bounds, and evaluates materializations that the constant recipe requires. Short wide arrays retain the **eight-lane / at-most-24 conversion-address-store instruction** bound; larger arrays preserve bulk/loop fallbacks. Nested expansion uses the existing shared budget. Constant execution retains its **1,000,000-work / 512-depth** limits. The native observations include calls, loops, memory and floating point; final encodings preserve their ABI and live result. No new pass, speculative growth, repeated IR scan, cache invalidation or debug-policy change was introduced.

## Acceptance and preservation

Acceptance follows spec.md §9 for **PA17/O0**, which mandates no numerical latency, RSS, runtime or text-size ceiling. The inherited **+15%, +16 MiB and 5.5×** targets remain diagnostic under this stage-scoped rule. Their original observations are retained; they do not impose new exit gates. Necessary semantic/contract costs and later-backend constraints are documented above. The measured transfer and eight-element array benefits repeat, while the implementation retains bounded work and growth, conservative fallbacks, correctness and coverage. No avoidable regression or unprofitable optional transform was identified by this accumulated review. The three remaining behavior failures remain implementation obligations.

The [transfer](transfer-performance.md), [query](query-performance.md), [storage](storage-performance.md) and [previous audit](checkpoint56-performance.md) reports and JSON remain unchanged, including their interrupted/noisy campaigns, pre-packing and pre-fast-path results. The complete [first audit campaign](../student.tests/pa17/checkpoint60-performance-before-constant-receiver.json) at `1f319dac` is preserved separately; it predates the final constant/emission fix and is not acceptance evidence. A record-size probe overlapped that earlier campaign, another reason not to pool it with the final serial campaigns. The initial no-entry-point preflight failure is retained in `checkpoint60/performance-preflight-no-entry.log` and supplied no accepted timing samples. No historical observations were overwritten or trimmed.
