# Final source-obligation performance review

The frozen comparison is destination checkpoint A against final source-obligation
B. Both produce correct equivalent results on every timed input. Invalid and
newly accepted controls are correctness evidence, not timing baselines. The main
campaign has 47 compiler and 14 executable workloads; the repeat has 15 compiler
and four executable workloads: **1,120 observations, 19,726 cumulative**. All 37
parent compiler workloads remain. Raw data: [main](source-obligations-performance.json),
[repeat](source-obligations-noise.json), [layout](source-obligations-layout.json).

Builds use g++ C++11, -Wall -O3 and TEST_RUNNER_ENABLE; the tested compiler and
supplied backend use O0. Binaries, flags, inputs, outputs, backend and harnesses
are hash-frozen. Each campaign checks outputs before timing, warms A/B once,
records four A/A observations and two ABBA blocks on CPU 0. Agent builds, tests,
verification and compression did not overlap timing. Every sample is retained.
The main ran 2026-09-13 23:43:51–23:48:45 UTC; the repeat 23:48:57–23:51:27.

A SHA-256: `cd924522f01a357ba41c67ff7f349324be297e55b98f82dac12e86220cc3044c`.
B SHA-256: `f971551f64fd4cffe0f4effb5f5391ed818845c05a56ef26cb106e26c0baf115`.

## Acceptance and interpretation

The affected fixed-operator shape K=128/M=8 improves compiler latency in all
four blocks: main −5.18%/−5.00%, repeat −5.09%/−4.88%, against A/A ranges of
0.87% and 1.07%. Candidate visits fall 2,184→152 and conversion work 7,458→473.
This supports a workload-specific compiler benefit from source selection reuse.
It does not establish native runtime improvement or a general compiler speedup.
The N=64,000 and Q=4,000 operator shapes have mixed blocks across campaigns.

The new default shape retains M source recipes and KM concrete uses. The
operator shape retains 2M operator/cast recipes and 2KM uses, plus M initializer
recipes and KM uses. N unrelated declarations and Q repeated calls do not
multiply source work. Default candidate visits increase by one per shape for
the newly required declaration-property selection. The initial implementation
missed the declarator key and recorded zero default reuse; it was corrected
before any timing. At K=128, the source-default and source-operator shapes keep
the same entities and scopes as A; the prior KM destination saving remains.

Compiler .text grows 1,371,014→1,424,966 bytes (+53,952, 3.94%). MemberFacts grows
120→124 bytes for declaration properties and Analyzer 6,552→6,760 for typed
indexes/context/counters. The other 16 measured main records are unchanged;
ConversionObject/Conversion/ValueInitialization/UserConversion stay 52/28/8/80.
These are semantic ownership costs, with no optional transform or growth pass.

Peak RSS is not uniformly lower. Demand rises 346,224→346,910 KiB in the main
and 346,100→346,838 in the repeat. It now retains 128 required condition recipes
and 128 extra conversion records while conversion work falls by 872. Member
property record growth also applies to demanded declarations. The exact RSS
attribution is not isolated. The declaration workload's median RSS reverses
from −3,646 to +4,000 KiB across campaigns with almost unchanged semantic
counts (one extra fact, two queries, six lookup visits). This is not evidence
of a stable memory saving. Operator K=128 RSS also reverses: −126 then +326 KiB.
All spreads below remain part of acceptance; no measured increase is hidden.

Large compiler wall outliers remain: source-default Q=4,000 +75.55% in one main
block, source-operator Q=4,000 +13.46%, return K=1 −15.59%, and body −7.86%.
Their repeats, where present, do not reproduce these magnitudes. Declaration
repeat blocks are +1.72%/+3.69% with 9.83% A/A spread. The timing cause is not
isolated. Compiling the tiny runtime sources takes about 5–7 ms and is startup
sensitive; those rows support no fine latency claim.

All 14 native text payload sizes are unchanged; 12 complete executable images
are identical. Destructor-runtime and source-default-runtime have different
declaration/emission order, equal course-canonical LowIR and equal text size.
Their checked outcomes, calls and cleanup remain intact. No native speedup is
claimed. Source-operator runtime has +11.42%/−6.60% main and +9.10%/−0.48%
repeat blocks despite identical executable bytes; the main A/A spread is 13.48%.
Source-default runtime has +0.57%/−0.99%, then −1.97%/−6.52%, with 20.91% and
3.19% calibration spreads. These do not prove a repeatable generated-code
regression or improvement. All native outliers and RSS remain below and in JSON.

PA14/O0 acceptance requires correct bounded semantic work and removal of
avoidable costs. Fixed selections are reused; concrete preparation follows
required uses and retains effects, access/deletion, object identity and cleanup.
Queries/default properties do not demand bodies or create runtime objects.
Complete typed keys and terminal success/failure states avoid global invalidation.
No optional executable optimization is added, so there is no profitability
policy or speculative code expansion to justify. Equivalent typed IR retains
its provenance and ABI; missing facts fail at the owning boundary. Native
selection, register allocation, encoding, optimized levels and self-hosting
remain later-stage work. There is no mandated PA14 numerical latency/RSS/text
cap. Historical self-selected targets remain diagnostics; none overrides these
stage-scoped rules or weakens a language, coverage, comparison or work bound.

## All observations summarized

Medians and ranges below use the four observations per binary within the two
ABBA blocks, excluding warmups and calibration. Each percentage is a block's
mean B/A wall change. A/A shows all four calibration observations as min–max
milliseconds and range/min percent. RSS is each process's peak, in KiB. The
linked JSON also retains warmups, CPU time and context switches. Calibration
measures observed variation; it is not a bound on later wall stalls.

### Main compiler

| Workload | A / B median ms [range] | Paired change % | A/A ms (spread %) | A / B peak RSS KiB [range] |
|---|---|---|---|---|
| virtual-runtime | 6.03 [5.94–6.04] / 6.11 [6.07–6.21] | +1.41 / +2.33 | 6.08–6.36 (4.61) | 5460 [5380–5504] / 5350 [5308–5528] |
| destructor-runtime | 6.26 [6.13–6.36] / 6.14 [6.10–6.18] | -1.44 / -2.23 | 6.04–6.22 (2.89) | 5376 [5216–5380] / 5360 [5328–5500] |
| body-run-4000-128 | 600.49 [598.71–707.42] / 597.92 [596.17–605.52] | -0.58 / -7.86 | 595.79–612.34 (2.78) | 95232 [95088–95292] / 95222 [95144–95316] |
| declaration-instances-1000 | 562.53 [561.27–564.18] / 562.64 [554.17–608.86] | +4.24 / -0.89 | 558.49–572.50 (2.51) | 78966 [78944–79044] / 75320 [75144–75352] |
| demand-uses-1000-128-4 | 3075.15 [3060.10–3096.49] / 3064.06 [3052.11–3086.47] | -0.28 / -0.38 | 3031.90–3385.60 (11.67) | 346224 [346096–346268] / 346910 [346788–347052] |
| demand-runtime | 6.75 [6.67–7.01] / 6.73 [6.71–6.84] | -0.40 / -0.83 | 6.61–6.76 (2.36) | 5484 [5220–5648] / 5670 [5500–5748] |
| region-runtime | 7.40 [7.24–7.51] / 7.41 [7.33–8.43] | +7.39 / -0.28 | 7.35–7.60 (3.41) | 5670 [5668–5760] / 5626 [5504–5692] |
| calls-runtime | 5.39 [5.38–5.50] / 5.50 [5.48–5.53] | +1.93 / +1.20 | 5.45–5.57 (2.23) | 5354 [5228–5424] / 5336 [5272–5412] |
| memory-runtime | 5.76 [5.68–5.98] / 5.81 [5.72–5.95] | +0.45 / +0.37 | 5.91–6.16 (4.26) | 5290 [5216–5496] / 5304 [5268–5364] |
| floating-runtime | 6.06 [5.77–6.18] / 5.91 [5.74–6.29] | -2.46 / +0.65 | 5.86–6.12 (4.44) | 5394 [5364–5508] / 5482 [5400–5504] |
| default-16000-1-1-1 | 83.62 [82.78–85.01] / 81.82 [81.06–83.68] | -3.09 / -0.86 | 83.29–85.42 (2.56) | 15660 [15496–15760] / 15768 [15712–15808] |
| default-64000-1-1-1 | 328.94 [324.20–333.29] / 324.90 [322.49–344.28] | +2.94 / -2.70 | 324.95–331.63 (2.06) | 47250 [47176–47356] / 47288 [47184–47480] |
| default-16000-512-1-1 | 119.75 [118.30–120.44] / 118.96 [117.94–119.73] | -0.91 / -0.20 | 118.73–119.11 (0.32) | 22276 [22204–22356] / 22372 [22332–22436] |
| default-16000-1-32-1 | 82.99 [82.05–84.51] / 82.64 [82.40–82.84] | +0.32 / -1.51 | 83.17–84.92 (2.10) | 15634 [15544–15688] / 15786 [15592–15812] |
| default-16000-1-32-4000 | 141.03 [139.97–143.70] / 140.37 [140.01–144.09] | +1.34 / -1.62 | 139.35–146.17 (4.89) | 24978 [24896–25072] / 25316 [25224–25420] |
| default-16000-1-1-4000 | 139.36 [139.11–139.60] / 138.87 [138.32–139.07] | -0.56 / -0.26 | 139.03–141.03 (1.44) | 24916 [24880–24956] / 24976 [24904–25056] |
| default-runtime | 6.00 [5.89–6.17] / 6.02 [5.99–6.16] | -0.29 / +1.51 | 6.05–6.25 (3.33) | 5220 [5184–5472] / 5308 [5284–5376] |
| return-16000-1-8-1 | 83.91 [82.32–112.19] / 82.51 [82.02–85.34] | -15.59 / +0.21 | 82.31–83.99 (2.04) | 15692 [15664–15828] / 15686 [15656–15736] |
| return-64000-1-8-1 | 339.71 [332.47–358.52] / 338.47 [325.24–352.57] | -4.67 / +2.46 | 321.80–334.96 (4.09) | 47282 [47152–47404] / 47422 [47392–47524] |
| return-16000-128-8-1 | 110.10 [109.62–112.02] / 109.28 [108.24–115.67] | +1.13 / -0.86 | 110.93–120.42 (8.56) | 21158 [21052–21220] / 21262 [21104–21304] |
| return-16000-1-8-4000 | 170.61 [168.30–173.21] / 171.63 [169.21–174.68] | +1.12 / +0.18 | 171.14–188.70 (10.26) | 31042 [30972–31100] / 31184 [31056–31260] |
| return-runtime | 6.33 [6.15–6.58] / 6.34 [6.23–6.44] | -1.10 / +0.76 | 6.42–6.60 (2.86) | 5272 [5224–5380] / 5400 [5272–5504] |
| init-16000-1-8-1 | 83.79 [83.72–84.06] / 83.02 [82.21–85.27] | -0.84 / -0.26 | 83.09–84.77 (2.02) | 15654 [15540–15688] / 15624 [15596–15768] |
| init-64000-1-8-1 | 323.79 [323.21–328.23] / 321.88 [318.23–332.94] | -1.33 / +0.69 | 322.69–326.33 (1.13) | 47106 [47076–47268] / 47392 [47284–47540] |
| init-16000-128-8-1 | 131.75 [130.61–135.01] / 131.33 [131.04–132.53] | -1.27 / +0.18 | 130.82–135.59 (3.64) | 24130 [24044–24200] / 24264 [24164–24268] |
| init-16000-1-8-4000 | 156.98 [156.90–157.53] / 156.75 [155.84–159.84] | -0.34 / +0.59 | 155.41–156.61 (0.77) | 26660 [26564–26708] / 26716 [26620–26836] |
| init-runtime | 6.37 [6.32–6.37] / 6.37 [6.33–6.40] | -0.06 / +0.42 | 6.27–6.41 (2.26) | 5342 [5216–5380] / 5364 [5308–5424] |
| copy-16000-1-8-1 | 83.81 [83.10–84.07] / 83.71 [82.52–84.74] | +0.87 / -0.94 | 83.00–85.12 (2.54) | 15558 [15540–15768] / 15638 [15552–15652] |
| copy-64000-1-8-1 | 325.37 [323.01–326.24] / 323.49 [321.45–330.61] | +0.90 / -1.04 | 324.55–340.82 (5.01) | 47292 [47088–47384] / 47426 [47320–47536] |
| copy-16000-128-8-1 | 134.31 [133.45–147.83] / 134.40 [133.37–136.47] | -3.92 / -0.07 | 134.69–155.59 (15.52) | 24354 [24256–24448] / 24422 [24296–24492] |
| copy-16000-1-8-4000 | 159.26 [156.67–162.15] / 157.91 [157.71–158.82] | -1.00 / -0.56 | 157.64–158.51 (0.55) | 26684 [26608–26760] / 26788 [26680–26812] |
| copy-runtime | 6.18 [6.07–6.30] / 6.40 [6.32–6.43] | +2.84 / +3.60 | 6.03–6.29 (4.36) | 5320 [5148–5368] / 5388 [5340–5460] |
| user-copy-16000-1-8-1 | 83.57 [82.81–84.18] / 82.54 [81.94–82.95] | -1.69 / -0.81 | 83.94–87.06 (3.71) | 15898 [15756–15928] / 15868 [15776–16080] |
| user-copy-64000-1-8-1 | 327.65 [325.20–342.49] / 330.72 [319.70–335.17] | -0.40 / -0.61 | 326.41–354.81 (8.70) | 47430 [47344–47480] / 47560 [47440–47656] |
| user-copy-16000-128-8-1 | 137.45 [134.63–140.61] / 136.81 [135.26–140.97] | +0.82 / -0.91 | 135.86–140.55 (3.45) | 25710 [25672–25732] / 25184 [25052–25260] |
| user-copy-16000-1-8-4000 | 159.05 [156.87–164.40] / 157.47 [157.35–159.30] | +0.88 / -3.24 | 163.57–164.43 (0.53) | 27204 [27124–27280] / 27250 [27188–27440] |
| user-copy-runtime | 6.32 [6.06–6.36] / 6.36 [6.29–6.39] | +1.68 / +0.99 | 6.21–6.30 (1.41) | 5346 [5176–5464] / 5444 [5348–5528] |
| source-default-16000-1-8-1 | 84.01 [83.29–85.71] / 82.89 [82.39–83.37] | -1.35 / -1.90 | 83.15–84.29 (1.38) | 15624 [15560–15716] / 15820 [15672–15876] |
| source-default-64000-1-8-1 | 328.24 [325.93–337.53] / 324.44 [322.91–342.86] | +0.13 / -0.94 | 320.41–341.54 (6.59) | 47362 [47156–47460] / 47406 [47268–47604] |
| source-default-16000-128-8-1 | 114.60 [113.07–116.00] / 113.62 [113.51–114.80] | -0.91 / -0.28 | 114.20–115.29 (0.96) | 21478 [21240–21616] / 21342 [21252–21472] |
| source-default-16000-1-8-4000 | 159.29 [158.05–166.05] / 158.51 [157.45–410.87] | +75.55 / -0.76 | 162.29–167.16 (3.00) | 27004 [26976–27036] / 27098 [26996–27180] |
| source-operator-16000-1-8-1 | 84.03 [83.62–84.60] / 83.43 [82.93–83.87] | -0.45 / -1.10 | 83.15–86.19 (3.66) | 15686 [15600–15732] / 15770 [15692–15824] |
| source-operator-64000-1-8-1 | 337.49 [323.97–347.13] / 329.79 [324.40–358.42] | +2.58 / -3.14 | 326.02–342.59 (5.08) | 47466 [47436–47504] / 47588 [47440–47708] |
| source-operator-16000-128-8-1 | 132.69 [132.36–133.07] / 126.06 [125.20–126.49] | -5.18 / -5.00 | 132.09–133.24 (0.87) | 24356 [24224–24496] / 24230 [24124–24552] |
| source-operator-16000-1-8-4000 | 158.38 [155.94–158.86] / 157.19 [156.19–199.39] | +13.46 / -1.23 | 156.24–157.34 (0.70) | 26742 [26560–26784] / 26904 [26736–26964] |
| source-default-runtime | 6.27 [6.10–6.43] / 6.30 [6.21–6.45] | +0.59 / +0.82 | 6.24–6.74 (8.03) | 5396 [5348–5512] / 5396 [5308–5480] |
| source-operator-runtime | 6.67 [6.63–7.39] / 6.88 [6.69–7.20] | +1.08 / +1.07 | 6.49–6.78 (4.56) | 5378 [5216–5476] / 5376 [5216–5444] |

### Main runtime

| Workload | A / B median ms [range] | Paired change % | A/A ms (spread %) | A / B peak RSS KiB [range] |
|---|---|---|---|---|
| virtual-runtime | 297.76 [296.09–302.75] / 299.60 [296.52–301.66] | +0.99 / -0.48 | 297.60–303.75 (2.07) | 256 [256–256] / 256 [256–256] |
| destructor-runtime | 494.29 [489.91–500.30] / 495.72 [494.28–504.63] | +1.04 / +0.12 | 489.69–497.05 (1.50) | 320000 [320000–320000] / 320000 [320000–320000] |
| demand-runtime | 155.31 [155.12–155.68] / 155.64 [155.13–156.28] | +0.15 / +0.25 | 155.51–156.82 (0.84) | 256 [256–256] / 256 [256–256] |
| region-runtime | 59.08 [58.93–59.14] / 59.02 [58.99–59.76] | +0.51 / -0.04 | 58.97–59.33 (0.61) | 256 [256–256] / 256 [256–256] |
| calls-runtime | 478.91 [478.36–479.43] / 479.02 [477.70–480.71] | -0.05 / +0.14 | 477.68–480.45 (0.58) | 256 [256–256] / 256 [256–256] |
| memory-runtime | 279.58 [278.97–282.78] / 280.44 [280.05–290.81] | +2.09 / -0.16 | 279.61–282.34 (0.98) | 256 [256–256] / 256 [256–256] |
| floating-runtime | 331.75 [331.24–332.03] / 331.08 [330.75–331.55] | -0.27 / -0.08 | 330.88–331.67 (0.24) | 256 [256–256] / 256 [256–256] |
| default-runtime | 676.64 [658.54–680.83] / 678.48 [670.57–686.26] | +0.25 / +1.32 | 676.41–691.22 (2.19) | 256 [256–256] / 256 [256–256] |
| return-runtime | 1828.99 [1818.81–1863.33] / 1855.69 [1825.13–1879.21] | +1.46 / +0.60 | 1842.42–1874.43 (1.74) | 256 [256–256] / 256 [256–256] |
| init-runtime | 266.79 [265.87–267.66] / 267.67 [267.26–269.27] | +0.79 / +0.11 | 264.44–279.35 (5.64) | 256 [256–256] / 256 [256–256] |
| copy-runtime | 274.13 [271.99–290.98] / 271.33 [268.35–274.25] | -3.78 / -0.83 | 268.85–273.23 (1.63) | 256 [256–256] / 256 [256–256] |
| user-copy-runtime | 330.16 [325.86–336.11] / 329.03 [325.51–335.21] | +0.41 / -0.94 | 325.43–326.63 (0.37) | 256 [256–256] / 256 [256–256] |
| source-default-runtime | 912.57 [901.03–935.73] / 912.97 [910.27–917.68] | +0.57 / -0.99 | 903.37–1092.27 (20.91) | 256 [256–256] / 256 [256–256] |
| source-operator-runtime | 1968.10 [1840.14–2252.62] / 2105.65 [1727.71–2269.54] | +11.42 / -6.60 | 1721.59–1953.64 (13.48) | 256 [256–256] / 256 [256–256] |

### Repeat compiler

| Workload | A / B median ms [range] | Paired change % | A/A ms (spread %) | A / B peak RSS KiB [range] |
|---|---|---|---|---|
| body-run-4000-128 | 608.42 [601.97–618.88] / 612.43 [610.46–623.61] | +2.09 / -0.33 | 600.30–611.08 (1.80) | 95430 [95352–95560] / 95464 [95372–95600] |
| declaration-instances-1000 | 560.27 [555.56–568.10] / 569.76 [565.90–599.59] | +1.72 / +3.69 | 555.42–610.02 (9.83) | 74846 [74752–74940] / 78846 [78772–78888] |
| demand-uses-1000-128-4 | 3092.25 [3064.33–3112.74] / 3096.07 [3064.64–3154.86] | -0.35 / +1.16 | 3059.04–3080.90 (0.71) | 346100 [346032–346376] / 346838 [346828–346968] |
| copy-16000-128-8-1 | 132.43 [131.87–134.04] / 132.18 [130.69–132.96] | +0.33 / -1.38 | 131.14–133.11 (1.50) | 24162 [24128–24436] / 24298 [24224–24428] |
| user-copy-16000-128-8-1 | 136.45 [135.89–137.13] / 136.58 [136.30–136.71] | +0.40 / -0.30 | 135.41–137.78 (1.75) | 24892 [24832–24952] / 25252 [25236–25272] |
| source-default-64000-1-8-1 | 326.56 [323.99–331.40] / 326.99 [325.63–328.30] | +0.86 / -0.93 | 323.50–382.40 (18.21) | 47350 [47084–47484] / 47490 [47200–47544] |
| source-default-16000-128-8-1 | 117.21 [115.70–117.67] / 115.69 [115.32–117.11] | -1.52 / -0.18 | 114.96–116.08 (0.97) | 21470 [21456–21484] / 21612 [21488–21664] |
| source-default-16000-1-8-4000 | 156.57 [156.00–156.63] / 155.89 [155.13–156.68] | -0.12 / -0.57 | 156.91–157.59 (0.44) | 26184 [26096–26224] / 26296 [26172–26360] |
| source-operator-64000-1-8-1 | 331.17 [323.11–333.66] / 323.73 [320.46–324.46] | -2.60 / -1.44 | 327.37–330.31 (0.90) | 47390 [47340–47484] / 47504 [47436–47640] |
| source-operator-16000-128-8-1 | 132.59 [132.14–134.52] / 126.28 [125.75–127.02] | -5.09 / -4.88 | 133.23–134.65 (1.07) | 24356 [24308–24464] / 24682 [24532–24792] |
| source-operator-16000-1-8-4000 | 158.76 [157.43–164.01] / 159.82 [157.84–161.68] | +0.62 / -0.55 | 154.99–157.28 (1.47) | 26720 [26612–26752] / 26828 [26744–26884] |
| source-default-runtime | 6.58 [6.18–6.67] / 6.53 [6.42–6.77] | +0.59 / +1.32 | 6.34–6.82 (7.52) | 5322 [5216–5468] / 5412 [5348–5520] |
| source-operator-runtime | 6.09 [6.07–6.21] / 6.16 [6.01–6.20] | +1.38 / -0.83 | 6.18–6.80 (10.02) | 5294 [5216–5472] / 5320 [5248–5372] |
| copy-runtime | 6.39 [6.23–6.49] / 6.42 [6.31–6.73] | -0.18 / +3.19 | 6.32–6.56 (3.85) | 5326 [5188–5464] / 5320 [5248–5476] |
| user-copy-runtime | 6.47 [6.28–6.67] / 6.45 [6.27–6.56] | -1.81 / +0.63 | 6.35–6.45 (1.68) | 5208 [5152–5248] / 5414 [5288–5520] |

### Repeat runtime

| Workload | A / B median ms [range] | Paired change % | A/A ms (spread %) | A / B peak RSS KiB [range] |
|---|---|---|---|---|
| source-default-runtime | 912.53 [900.00–1032.14] / 900.29 [887.10–907.30] | -1.97 / -6.52 | 910.69–939.78 (3.19) | 256 [256–256] / 256 [256–256] |
| source-operator-runtime | 1807.91 [1768.01–1903.61] / 1938.29 [1749.05–1973.21] | +9.10 / -0.48 | 1718.47–1777.10 (3.41) | 256 [256–256] / 256 [256–256] |
| copy-runtime | 266.94 [265.18–267.51] / 267.05 [265.28–267.99] | -0.37 / +0.52 | 267.37–269.77 (0.90) | 256 [256–256] / 256 [256–256] |
| user-copy-runtime | 326.65 [326.43–429.84] / 326.94 [324.43–341.58] | -14.20 / +2.72 | 323.56–331.05 (2.31) | 256 [256–256] / 256 [256–256] |

### Native text and output identity

The supplied sectionless backend metric is payload after ELF entry, as in the
shared harness; compiler text uses the .text section. Loops, calls, memory,
floating point, virtual dispatch and lifecycle workloads check live results.
The new default runtime checks four million calls and checksum 18,000,000;
the new operator runtime checks sixteen million effects and checksum 40,000,000.

| Workload | A → B text bytes | Complete image equal |
|---|---:|---|
| virtual-runtime | 2360 → 2360 | yes |
| destructor-runtime | 2072 → 2072 | no; equivalent canonical LowIR |
| demand-runtime | 261 → 261 | yes |
| region-runtime | 206 → 206 | yes |
| calls-runtime | 206 → 206 | yes |
| memory-runtime | 434 → 434 | yes |
| floating-runtime | 230 → 230 | yes |
| default-runtime | 300 → 300 | yes |
| return-runtime | 444 → 444 | yes |
| init-runtime | 1456 → 1456 | yes |
| copy-runtime | 1456 → 1456 | yes |
| user-copy-runtime | 1704 → 1704 | yes |
| source-default-runtime | 416 → 416 | no; equivalent canonical LowIR |
| source-operator-runtime | 536 → 536 | yes |

The [validation](source-obligations-validation.json) contains 79 passing groups,
1935/1935 course tests, both builds’ 242 source controls and four branch programs,
349 entry/current and 349 release/sanitizer comparisons, all 35 existing PA14
native programs and inherited demand/lifetime/ABI probes. Repeated property
queries perform no body/action demand and concrete finish queries remain stable.
All 1,266 fixture/reference hashes are unchanged. The [proofs](source-obligations-proofs.json)
record 171 incorrect entry outcomes; the [handoff](source-obligations-handoff.json)
closes all five previously open cases under both builds. The [journal](source-obligations-journal.json)
retains preliminary failures, corrected evidence collisions and lossless probe
archives. No preliminary source-obligation timing was used or discarded.
