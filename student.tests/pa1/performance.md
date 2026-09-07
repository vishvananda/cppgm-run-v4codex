# PA1 final audit performance evidence

Final implementation: `29e95571f`; measured 2026-09-07 UTC. The original campaign
is retained below as historical evidence. This audit adds **360 observations**
over ten fixed inputs; none were discarded. All three campaigns are complete.

## Scope and controls

PA1 produces preprocessing tokens. **Generated-program runtime and generated
text size are N/A for every workload.** Template syntax, loops, calls, memory
indexing, floating-point spellings and repeated implementation sources exercise
lexical work; they do not establish semantic or executable performance.

Host: Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43; Intel(R) Xeon(R) CPU @ 2.20GHz.
Compiler: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0.
Flags: `-std=gnu++11 -Wall -O3`; supplied test wrapper enabled
(`-Dmain=test_runner_real_main` on the entry point). Ordinary invocations
call the implementation directly. No concurrent builds or tests ran during
measurements. CPU affinity is explicit below; frequency and host scheduling
were not controlled, so the A/A calibration and all spreads are disclosed.

Before binary SHA-256: `bdb50c54f12d6ea1b58204edcaed46aeb298cfd5a987eae169c8238532ec03d5`.
Final binary SHA-256: `0875fb4f617aacf0f7d1275af5167b52bad21c8408fc66dc72cd311ca5ae8839`.

- `audit-abba`: A = completed pre-audit binary, B = final binary, both ordinary CLI;
  affinity 0–31. This measures the table-growth fix and its telemetry support code.
- `audit-abba-pinned`: same A/B binaries and byte-identical inputs, pinned to CPU 0.
  Repeated because the first run had a noisy long-token result.
- `audit-telemetry`: A = final ordinary CLI, B = the same binary with `--stats`,
  pinned to CPU 0. This isolates enabled telemetry overhead.

Each workload has two A/A calibration pairs and two wall-time ABBA blocks.
Before timing, complete output hashes agree for A, B and final `--stats`.
Wall time includes process startup, reading, scanning and required formatting
to `/dev/null`; `/usr/bin/time` measures the fresh executable child peak RSS.
Output-hashing warmups use a pipe and are excluded from latency/RSS comparisons.
Every invocation has a 60-second timeout. Frozen binaries, inputs, build/source
hashes, harness copies, manifests and all JSON rows remain under the three
`obj/student-pa1/` directories above. Generated artifacts are not committed.

## Result and tradeoffs

The completed-name boundary keeps 262144 canonical IDs and then repeats one of
them 1.5 million times. Before the fix, this needlessly doubled the slot table:
**13467510 → 11370358 retained identifier bytes (2 MiB saved)** and 55 → 54
table storage growth events. Peak RSS fell **44604 → 40508 KiB** in the first
comparison and **44608 → 40508 KiB** when pinned. The direct API regression
failed before the fix and passes afterward, including under ASan/UBSan.
This repeatable memory/work reduction justifies the change. Its latency pairs
(-0.82/-0.71% and -2.56/-0.96%) are within the calibration spread; no speedup
is claimed. IDs, spellings, output and insertion behavior are preserved.

Regressions/noise are not hidden: the first long-token comparison had a +7.1%
median change and a +7.87% second ABBA block; the pinned repeat instead had
-0.36/-2.89% pairs, with roughly 6.2% A/A variation in each campaign. The first
800k-name comparison had a +9.04% block, followed by +0.48%; pinned pairs were
-0.42/+0.82%. The pinned 3.2M-name median was about +2.9% (+3.44/+1.70% pairs),
within its 6.13% calibration change; the first campaign pairs were +0.23/+0.07%.
Repeated-32 pairs ranged from +0.13% to +1.62% across comparisons. These results
do not establish a repeatable large latency regression or a small speed gain.
Small costs remain possible and are acceptable for the proven storage/work
reduction, bounded scaling, unchanged output and modest host-tool code growth.

GNU `size` reports host-tool text totals 57085 → 57573 bytes (+488, 0.85%);
data/BSS remain 1712/1272 bytes. These are compiler-binary measurements, not
generated-code text size or an executable optimization claim. No optimization
level, IR/code growth, ABI, debug, loop or spill policy is active in PA1.

Final telemetry-enabled pairs are reported below, including positive costs.
Telemetry adds no semantic analysis. Timing spread prevents a subpercent overhead
claim; disabling it preserves output and skips per-work counters.

## Whole-stage budgets

All modes/campaigns pass the [ownership/work bounds](../../pa1/audit.md#ownership-validity-and-work-budgets).
Both A and B fourfold scaling ratios were independently recomputed from all
observations; the harness now gates both variants. Decoding is at most `2*B+64`,
rehash probes at most `8*U+64` on these inputs, unique-name storage at most
`64*U+2*B+64`, source capacity at most `2*B+65536`, and scratch capacity at most
`2*B+15`, where B is source bytes and U is unique IDs. RSS stays within twice
retained source/name/scratch capacity plus 32 MiB. The original five inputs and
new raw/long-token/boundary inputs also satisfy `4*B+32 MiB`.
The denser 800k/3.2M-name inputs use the ownership bound: a byte-only RSS limit
would omit required canonical-name metadata. This corrects the scope of the
old memory envelope; it does not assert that arbitrary source fits that limit.

Final ordinary-CLI medians from the telemetry campaign are 1.514611/6.015027 s
for repeated 8/32 MiB (3.9713x, limit 6x), with maximum RSS 12092/36672 KiB.
The final compiler in the two A/B campaigns scaled 3.9930x and 3.9956x.
Unique-name 200k→800k→3.2M ordinary-CLI growth is 4.1871x/4.3258x in the final
campaign (each limit 6x). The 3.2M-name workload takes 3.922724 s and reaches
245556 KiB, including 181925660 bytes of retained identifier storage.
Repeated-name storage stays 618 bytes at both input sizes. Long/raw/splice
boundaries retain one scratch buffer or borrowed source and bounded lookahead.

## audit-abba: paired compiler results

Seconds show median [min, max] of ABBA samples. RSS maxima include calibration.
Percentages use second/first for A/A and mean(B)/mean(A) for each ABBA block.

| Workload | A seconds | B seconds | Max RSS A/B KiB | A/A % | ABBA B/A % |
| --- | --- | --- | --- | --- | --- |
| repeated-8 | 1.497012 [1.490465, 1.505423] | 1.512869 [1.498794, 1.561386] | 12096/12092 | +0.259, -0.752 | +1.059, +2.146 |
| repeated-32 | 5.987238 [5.977764, 5.993005] | 6.040821 [6.014380, 6.065074] | 36672/36668 | -0.068, +0.426 | +0.498, +1.305 |
| translations-8 | 0.650350 [0.646194, 0.660187] | 0.650805 [0.640928, 0.669036] | 12092/12072 | -0.924, -0.466 | +0.510, -0.161 |
| self-source-8 | 1.042319 [1.035283, 1.048796] | 1.046651 [1.037216, 1.054159] | 12096/12092 | -1.534, -0.735 | +0.241, +0.524 |
| unique-200000 | 0.217721 [0.216999, 0.219249] | 0.212027 [0.210878, 0.213414] | 19836/19840 | +2.194, +1.583 | -2.911, -2.444 |
| unique-800000 | 0.881569 [0.878865, 0.906182] | 0.906366 [0.889392, 1.013668] | 68852/68852 | -1.648, -2.877 | +9.039, +0.481 |
| unique-3200000 | 3.869471 [3.753145, 3.916158] | 3.877110 [3.780234, 3.896251] | 245560/245552 | -3.029, -2.502 | +0.227, +0.066 |
| table-hit-boundary | 1.657598 [1.630892, 1.682413] | 1.643107 [1.612589, 1.678993] | 44604/40508 | +4.125, +4.216 | -0.818, -0.712 |
| raw-near-8 | 0.360115 [0.358622, 0.362792] | 0.359319 [0.356654, 0.360304] | 20284/20252 | +0.908, -1.437 | -0.342, -0.497 |
| long-token-8 | 0.389581 [0.388538, 0.421289] | 0.417213 [0.390715, 0.421015] | 28156/28132 | +0.239, -6.264 | -0.511, +7.868 |

## audit-abba-pinned: paired compiler results

Seconds show median [min, max] of ABBA samples. RSS maxima include calibration.
Percentages use second/first for A/A and mean(B)/mean(A) for each ABBA block.

| Workload | A seconds | B seconds | Max RSS A/B KiB | A/A % | ABBA B/A % |
| --- | --- | --- | --- | --- | --- |
| repeated-8 | 1.511262 [1.497503, 1.520976] | 1.510002 [1.505424, 1.514571] | 12096/12092 | -1.014, -0.280 | -0.439, +0.409 |
| repeated-32 | 6.012367 [5.997854, 6.039117] | 6.033380 [6.011789, 6.193945] | 36672/36668 | +0.239, -0.635 | +1.622, +0.132 |
| translations-8 | 0.643794 [0.641521, 0.647705] | 0.647103 [0.643807, 0.651212] | 12096/12088 | -0.035, -0.161 | -0.292, +1.259 |
| self-source-8 | 1.048028 [1.033202, 1.059729] | 1.041395 [1.038503, 1.052232] | 12096/12092 | +0.333, -0.417 | -0.007, -0.732 |
| unique-200000 | 0.215708 [0.214776, 0.247286] | 0.217924 [0.213310, 0.221139] | 19836/19836 | +12.104, -2.727 | -6.097, +1.167 |
| unique-800000 | 0.904284 [0.892033, 0.922194] | 0.904633 [0.897701, 0.922954] | 68856/68852 | -1.505, +0.531 | -0.421, +0.818 |
| unique-3200000 | 3.831068 [3.798243, 3.853888] | 3.943496 [3.851558, 3.968603] | 245560/245552 | +6.125, -2.748 | +3.442, +1.697 |
| table-hit-boundary | 1.691426 [1.645667, 1.711562] | 1.653714 [1.633253, 1.680844] | 44608/40508 | +3.516, -0.643 | -2.561, -0.963 |
| raw-near-8 | 0.359135 [0.357625, 0.361293] | 0.360648 [0.357347, 0.364853] | 20288/20284 | -2.165, +1.346 | +0.993, -0.114 |
| long-token-8 | 0.417997 [0.394429, 0.421281] | 0.406868 [0.392163, 0.418654] | 28160/28136 | +0.739, +6.235 | -0.358, -2.893 |

## audit-telemetry: paired compiler results

Seconds show median [min, max] of ABBA samples. RSS maxima include calibration.
Percentages use second/first for A/A and mean(B)/mean(A) for each ABBA block.

| Workload | A seconds | B seconds | Max RSS A/B KiB | A/A % | ABBA B/A % |
| --- | --- | --- | --- | --- | --- |
| repeated-8 | 1.514611 [1.503521, 1.557087] | 1.511706 [1.496204, 1.524789] | 12092/12092 | -0.677, +0.607 | -1.857, +0.382 |
| repeated-32 | 6.015027 [5.991834, 6.038376] | 6.041228 [6.030904, 6.121234] | 36672/36668 | -0.882, +0.004 | +1.015, +0.434 |
| translations-8 | 0.645008 [0.642210, 0.650481] | 0.647295 [0.642128, 0.652102] | 12092/12092 | +1.256, -0.440 | +0.738, -0.265 |
| self-source-8 | 1.052687 [1.042600, 1.053517] | 1.041710 [1.037881, 1.054957] | 12092/12096 | -0.245, -0.564 | +0.153, -1.350 |
| unique-200000 | 0.216572 [0.213609, 0.220119] | 0.215697 [0.215249, 0.221431] | 19840/19836 | -3.031, -2.354 | -0.576, +0.852 |
| unique-800000 | 0.906816 [0.895759, 1.032687] | 0.919354 [0.908147, 0.924054] | 68852/68984 | -0.835, +2.124 | +1.393, -5.001 |
| unique-3200000 | 3.922724 [3.825003, 3.949799] | 3.884329 [3.832545, 3.941885] | 245556/245548 | -0.829, -0.096 | +0.144, -1.124 |
| table-hit-boundary | 1.655444 [1.627949, 1.685319] | 1.683857 [1.655571, 1.686290] | 40508/40580 | +3.252, -0.243 | +1.927, +0.654 |
| raw-near-8 | 0.359200 [0.358046, 0.362174] | 0.363529 [0.360328, 0.365126] | 20284/20284 | -5.515, -0.210 | +0.433, +1.503 |
| long-token-8 | 0.395622 [0.392964, 0.398405] | 0.398922 [0.395505, 0.416478] | 28156/28280 | +6.897, +2.631 | +3.447, -0.008 |

## Final phase and work observations

Phase medians use the four timed B (`--stats`) runs from `audit-telemetry`.
These are the observed reading and scanning/formatting phases, not warmups.
Work/capacity counters are invariant across those four runs. Growth events
cover identifier storage, not every host allocation. Probe definitions and
the inherited-RSS caveat are in the architecture audit.

| Workload | Read / scan+emit ms | Tokens incl. EOF | Decode units | Copied spelling bytes | Unique IDs | Rehash probes | Growth events |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| repeated-8 | 17.262 / 1486.625 | 4767572 | 8388511 | 0 | 14 | 10 | 11 |
| repeated-32 | 72.078 / 5957.575 | 19070443 | 33554322 | 0 | 14 | 10 | 11 |
| translations-8 | 16.788 / 622.548 | 1417789 | 7679685 | 827043 | 3 | 0 | 5 |
| self-source-8 | 16.722 / 1017.220 | 2843425 | 8374464 | 0 | 240 | 287 | 24 |
| unique-200000 | 6.611 / 200.553 | 400002 | 2288891 | 0 | 200000 | 308127 | 54 |
| unique-800000 | 27.000 / 878.674 | 1600002 | 9488891 | 0 | 800000 | 1216336 | 60 |
| unique-3200000 | 113.941 / 3738.985 | 6400002 | 40488891 | 0 | 3200000 | 4846466 | 66 |
| table-hit-boundary | 58.189 / 1614.485 | 3524290 | 21034619 | 0 | 262144 | 308127 | 54 |
| raw-near-8 | 26.944 / 328.510 | 3 | 8912934 | 0 | 0 | 0 | 0 |
| long-token-8 | 25.796 / 363.563 | 3 | 8388612 | 8388609 | 1 | 0 | 2 |

| Workload | Source bytes | Source capacity | Identifier capacity | Scratch capacity |
| --- | ---: | ---: | ---: | ---: |
| repeated-8 | 8388511 | 8388608 | 618 | 15 |
| repeated-32 | 33554322 | 33554432 | 618 | 15 |
| translations-8 | 8388579 | 8388608 | 170 | 15 |
| self-source-8 | 8374464 | 8388608 | 11170 | 15 |
| unique-200000 | 2288891 | 4194304 | 11370358 | 15 |
| unique-800000 | 9488891 | 16777216 | 45481426 | 15 |
| unique-3200000 | 40488891 | 67108864 | 181925660 | 15 |
| table-hit-boundary | 21034619 | 33554432 | 11370358 | 15 |
| raw-near-8 | 8912934 | 16777216 | 64 | 15 |
| long-token-8 | 8388612 | 16777216 | 8388697 | 16777216 |

## Frozen input and output identities

These inputs are identical across all three audit campaigns. Self-source was
refrozen for this audit and then reused; it differs from the historical input.

| Workload | Input SHA-256 | A/B/stats output SHA-256 |
| --- | --- | --- |
| repeated-8 | `459d7af709027a678fa0c2ed27b86ba51abb340ecbdf366cf5bf4266de65697a` | `450ef8eb52b9098983995289322fa7a3bd6e55e94745e660ad524acf74be282b` |
| repeated-32 | `7719804e57fa7d3127ad47fc3cfeb98ae91eb55d9e3c8e716ce16ba1f4299e9a` | `230812a1ab8149a665a388c1ef9a657460112016560f0217848c3920d460f854` |
| translations-8 | `b7481e1da8429d9a50c8f98d5d9a80644c2032b44277ec66bcd789ed188c5b45` | `3bcb87f0de344e0dc80044a825976c6c29004b4e4034faeb288991c090caada8` |
| self-source-8 | `fdd9b13605153e2b32073aad7460998640473549fa1bb4729aa832f13aed5588` | `6321e2713911c562f930d1283bdddc49fbeeadebeb22e7aedeb21c5dd7fd049b` |
| unique-200000 | `d95badc2ff7c09100d90c66097e21d7731a2088ab0a739d28c54a04735af07d1` | `2fcc157f41436c7eda2815513c3a52b1cbe85a46e6c9a9574f48926fef14d61b` |
| unique-800000 | `62940cd03c665d57129829e9231dd137d3719e2e1c154d7393597ad79cdc2af1` | `8ede690ec9237ade3412816ced80d9d8c7730e6b5fe9212c5f510f11c7ccfb16` |
| unique-3200000 | `035ad16bcaf000ca87e5b96bfbbb9aea1e3ae22dd3dd34b1a504f38ad573f852` | `82685027ff286bdb421b273dc843413d42ac616d47167da924d8f02ad42586a0` |
| table-hit-boundary | `54e4cd416f44e1793d7c7bcb12fac934a2d7cf5d55d9d40d073ef6f01c7f07a1` | `0141f68885fd074e95eacf06fb2c9d394a5ac3a776534a97bf89e3f32de4000f` |
| raw-near-8 | `f6d2eb3f5cfa2af9476aca0bc6809fcfd104a9cf2045eb730ee52f47cb02ece1` | `04d1c0b30dda2a240a48ca0713874ca075c96f6ee130601c35de3f3a319175c6` |
| long-token-8 | `5eb8d6af7b343fba4ed5cf783cebfe6b33afc4593d135885ca0f4c00ac24dcd5` | `a38c521688da8a21816e786c7bbe0a48e758784ef88415179614c64586ec38e5` |

## All 360 wall-time/RSS observations

Each cell is `seconds / peak KiB`. Rows 0–3 are A/A calibration; rows 4–7
and 8–11 are ABBA blocks. No rows were filtered, trimmed or replaced.

### audit-abba

| Sample / mode | repeated-8 | repeated-32 | translations-8 | self-source-8 | unique-200000 | unique-800000 | unique-3200000 | table-hit-boundary | raw-near-8 | long-token-8 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0 / A | 1.490130599 / 12068 | 5.962885972 / 36492 | 0.643965955 / 12092 | 1.052141071 / 12092 | 0.210779047 / 19820 | 0.906464307 / 68824 | 3.898680463 / 245476 | 1.625732568 / 44424 | 0.355054824 / 20280 | 0.414206059 / 28016 |
| 1 / A | 1.493983738 / 12080 | 5.958802827 / 36488 | 0.638017040 / 12092 | 1.035998738 / 12092 | 0.215403702 / 19712 | 0.891528033 / 68848 | 3.780586427 / 245540 | 1.692790172 / 44588 | 0.358278905 / 20284 | 0.415195868 / 28152 |
| 2 / A | 1.505713214 / 12096 | 5.970746970 / 36500 | 0.641434170 / 11816 | 1.056018403 / 12080 | 0.216811006 / 19656 | 0.907106430 / 68852 | 3.852864164 / 245416 | 1.626825710 / 44428 | 0.365047478 / 20280 | 0.416030123 / 28148 |
| 3 / A | 1.494387401 / 12072 | 5.996190711 / 36668 | 0.638447699 / 11968 | 1.048254631 / 12096 | 0.220242800 / 19832 | 0.881011784 / 68720 | 3.756483186 / 245560 | 1.695406810 / 44604 | 0.359803465 / 20144 | 0.389970262 / 28128 |
| 4 / A | 1.493339451 / 12096 | 5.983743740 / 36672 | 0.648811104 / 12092 | 1.045799626 / 12088 | 0.219249153 / 19836 | 0.878864856 / 68824 | 3.753145452 / 245496 | 1.630892080 / 44436 | 0.362792327 / 20116 | 0.388537869 / 27988 |
| 5 / B | 1.511237196 / 12092 | 6.014379732 / 36668 | 0.649729126 / 12072 | 1.052454406 / 11820 | 0.212340783 / 19836 | 0.904336803 / 68824 | 3.873815561 / 245404 | 1.670233027 / 40508 | 0.358642284 / 20252 | 0.414971777 / 28132 |
| 6 / B | 1.514500164 / 12092 | 6.022031900 / 36520 | 0.651881565 / 11912 | 1.037216320 / 12092 | 0.211713233 / 19840 | 1.013667762 / 68852 | 3.780234031 / 245536 | 1.615981004 / 40500 | 0.360304334 / 19852 | 0.390714631 / 27948 |
| 7 / A | 1.500683641 / 11952 | 5.993004625 / 36668 | 0.646193645 / 11932 | 1.038838243 / 12092 | 0.217518524 / 19832 | 0.880137787 / 68852 | 3.883573565 / 245376 | 1.682412716 / 44600 | 0.358622386 / 20156 | 0.421288654 / 28024 |
| 8 / A | 1.490464639 / 11912 | 5.990732911 / 36668 | 0.660187454 / 12076 | 1.048796178 / 11924 | 0.216998713 / 19836 | 0.883000131 / 68728 | 3.916157555 / 245532 | 1.635863527 / 44424 | 0.358795936 / 20268 | 0.389455735 / 27976 |
| 9 / B | 1.561386035 / 12048 | 6.065074394 / 36652 | 0.640927712 / 11952 | 1.040848141 / 11640 | 0.213413910 / 19672 | 0.908395138 / 68852 | 3.896251346 / 245272 | 1.678992544 / 40324 | 0.356653831 / 20104 | 0.419454224 / 28128 |
| 10 / B | 1.498794225 / 11912 | 6.059610467 / 36484 | 0.669036231 / 11960 | 1.054158995 / 12080 | 0.210878426 / 19680 | 0.889392301 / 68676 | 3.880404867 / 245552 | 1.612588612 / 40480 | 0.359995741 / 20028 | 0.421014658 / 28128 |
| 11 / A | 1.505422883 / 11912 | 5.977764362 / 36668 | 0.651889386 / 11912 | 1.035283381 / 11924 | 0.217924379 / 19692 | 0.906182025 / 68848 | 3.855367607 / 245552 | 1.679331975 / 44480 | 0.361433426 / 20160 | 0.389705623 / 28156 |

### audit-abba-pinned

| Sample / mode | repeated-8 | repeated-32 | translations-8 | self-source-8 | unique-200000 | unique-800000 | unique-3200000 | table-hit-boundary | raw-near-8 | long-token-8 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0 / A | 1.517110229 / 12088 | 5.970517782 / 36668 | 0.646235947 / 11968 | 1.050209259 / 12076 | 0.219597456 / 19604 | 0.916510230 / 68852 | 3.922628965 / 245424 | 1.635790994 / 44428 | 0.367852232 / 20264 | 0.416214721 / 28124 |
| 1 / A | 1.501721691 / 12092 | 5.984806961 / 36648 | 0.646011237 / 12080 | 1.053708826 / 11912 | 0.246177983 / 19704 | 0.902720508 / 68824 | 4.162893913 / 245556 | 1.693311546 / 44560 | 0.359890064 / 20072 | 0.419291648 / 28020 |
| 2 / A | 1.508623495 / 12092 | 6.013926004 / 36652 | 0.641994775 / 12096 | 1.038076823 / 12064 | 0.218690451 / 19716 | 0.917072832 / 68852 | 3.931813679 / 245556 | 1.642503049 / 44604 | 0.356774446 / 20264 | 0.391295301 / 28160 |
| 3 / A | 1.504404042 / 12096 | 5.975736717 / 36668 | 0.640963721 / 11976 | 1.033750178 / 12092 | 0.212726789 / 19836 | 0.921940102 / 68688 | 3.823770648 / 245512 | 1.631943367 / 44480 | 0.361578018 / 20268 | 0.415692325 / 28136 |
| 4 / A | 1.512335850 / 11916 | 5.997853987 / 36672 | 0.643793885 / 12056 | 1.047010500 / 12092 | 0.247286489 / 19820 | 0.922193739 / 68848 | 3.798242922 / 245556 | 1.645667085 / 44596 | 0.360504288 / 20288 | 0.419297631 / 28128 |
| 5 / B | 1.511807832 / 12092 | 6.193944522 / 36476 | 0.643807272 / 12088 | 1.052232178 / 11916 | 0.221139486 / 19656 | 0.903579548 / 68852 | 3.952492454 / 245424 | 1.633253378 / 40380 | 0.364852758 / 20280 | 0.392163021 / 28136 |
| 6 / B | 1.508196007 / 11912 | 6.017502077 / 36668 | 0.643918384 / 12084 | 1.043677849 / 11952 | 0.213310374 / 19832 | 0.905685786 / 68668 | 3.934498644 / 245552 | 1.637998274 / 40340 | 0.360406833 / 20268 | 0.418654198 / 27992 |
| 7 / A | 1.520975611 / 12096 | 6.018655144 / 36664 | 0.647704727 / 12092 | 1.049044529 / 12092 | 0.215372795 / 19832 | 0.894713270 / 68852 | 3.826328186 / 245560 | 1.711561822 / 44480 | 0.357624799 / 20100 | 0.394429422 / 28156 |
| 8 / A | 1.510187482 / 11924 | 6.006079632 / 36644 | 0.641520774 / 11956 | 1.059729071 / 12096 | 0.216043216 / 19836 | 0.913854357 / 68856 | 3.853888025 / 245552 | 1.685202231 / 44604 | 0.357765316 / 20120 | 0.416696237 / 28160 |
| 9 / B | 1.505424046 / 12064 | 6.011788880 / 36644 | 0.650287809 / 11984 | 1.038503386 / 12092 | 0.215545652 / 19836 | 0.922954449 / 68672 | 3.851557697 / 245552 | 1.669430199 / 40508 | 0.360888752 / 20284 | 0.395582896 / 28020 |
| 10 / B | 1.514570599 / 12092 | 6.049257881 / 36668 | 0.651212131 / 11884 | 1.039112948 / 12076 | 0.220301975 / 19656 | 0.897701199 / 68852 | 3.968603383 / 245428 | 1.680843924 / 40488 | 0.357346765 / 20120 | 0.418153303 / 27988 |
| 11 / A | 1.497503338 / 12048 | 6.039116979 / 36500 | 0.643794702 / 12076 | 1.033202352 / 12064 | 0.214775806 / 19832 | 0.892032785 / 68836 | 3.835808197 / 245348 | 1.697649881 / 44608 | 0.361292772 / 20140 | 0.421281052 / 27976 |

### audit-telemetry

| Sample / mode | repeated-8 | repeated-32 | translations-8 | self-source-8 | unique-200000 | unique-800000 | unique-3200000 | table-hit-boundary | raw-near-8 | long-token-8 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0 / A | 1.516265821 / 11924 | 6.044020649 / 36668 | 0.642398076 / 11952 | 1.043484908 / 11924 | 0.221016591 / 19840 | 0.901219100 / 68836 | 3.898104772 / 245556 | 1.626209253 / 40508 | 0.380264507 / 20104 | 0.392486823 / 27976 |
| 1 / A | 1.506003328 / 11896 | 5.990705426 / 36660 | 0.650464538 / 12088 | 1.040925201 / 11928 | 0.214317482 / 19836 | 0.893689652 / 68852 | 3.865785688 / 245424 | 1.679090280 / 40368 | 0.359291214 / 20284 | 0.419554851 / 27976 |
| 2 / A | 1.515572884 / 12092 | 6.032060640 / 36488 | 0.650093404 / 11924 | 1.053481392 / 12092 | 0.217594165 / 19672 | 0.899236915 / 68820 | 3.829594041 / 245432 | 1.675141472 / 40332 | 0.361194461 / 20284 | 0.417142201 / 28156 |
| 3 / A | 1.524766711 / 12076 | 6.032288891 / 36544 | 0.647234701 / 12088 | 1.047542127 / 11912 | 0.212472537 / 19836 | 0.918340225 / 68852 | 3.825926422 / 245556 | 1.671065728 / 40504 | 0.360436864 / 20164 | 0.428117560 / 28128 |
| 4 / A | 1.557086778 / 12008 | 5.993619001 / 36672 | 0.650480999 / 12092 | 1.042599866 / 12060 | 0.213684528 / 19824 | 0.896200517 / 68720 | 3.932460059 / 245556 | 1.627949115 / 40384 | 0.362173986 / 20268 | 0.396205515 / 28000 |
| 5 / B | 1.496203719 / 11884 | 6.030904136 / 36500 | 0.652101649 / 12092 | 1.054956607 / 12088 | 0.215400972 / 19800 | 0.914846889 / 68932 | 3.853691825 / 245540 | 1.686289773 / 40580 | 0.360327647 / 20156 | 0.402043460 / 28272 |
| 6 / B | 1.514669440 / 12092 | 6.121233580 / 36488 | 0.650127072 / 11948 | 1.044059009 / 11972 | 0.215249264 / 19836 | 0.924054229 / 68972 | 3.914965300 / 245532 | 1.685889095 / 40500 | 0.365126142 / 20284 | 0.416478452 / 28280 |
| 7 / A | 1.510765516 / 12068 | 6.036434444 / 36668 | 0.642209502 / 11928 | 1.053211664 / 11912 | 0.219459817 / 19656 | 0.917431019 / 68724 | 3.825002849 / 245424 | 1.680489668 / 40328 | 0.360153458 / 20108 | 0.395038391 / 28144 |
| 8 / A | 1.503520765 / 11960 | 6.038376062 / 36484 | 0.647154130 / 12088 | 1.052161383 / 12064 | 0.213608900 / 19836 | 0.895759473 / 68720 | 3.949798789 / 245556 | 1.630397460 / 40464 | 0.358045541 / 20284 | 0.398405197 / 27972 |
| 9 / B | 1.524788505 / 12052 | 6.047084925 / 36668 | 0.644463549 / 11884 | 1.037880595 / 12092 | 0.221430604 / 19824 | 0.908147046 / 68852 | 3.832544952 / 245548 | 1.681824248 / 40508 | 0.362298461 / 20148 | 0.395801497 / 28156 |
| 10 / B | 1.508741953 / 12088 | 6.035371529 / 36668 | 0.642128379 / 12092 | 1.039361729 / 12096 | 0.215992734 / 19836 | 0.923860199 / 68984 | 3.941885246 / 245536 | 1.655570619 / 40504 | 0.364759160 / 20276 | 0.395504883 / 28156 |
| 11 / A | 1.518456028 / 12088 | 5.991833894 / 36636 | 0.642862170 / 12092 | 1.053516810 / 12056 | 0.220119187 / 19836 | 1.032687150 / 68684 | 3.912987736 / 245528 | 1.685319427 / 40508 | 0.358246903 / 20120 | 0.392964498 / 28140 |

## Final build and harness identities

| Input | SHA-256 |
| --- | --- |
| dev/pptoken.cpp | `36e3ac8480b310bef1849f0974c7d758951fb3219e5d9cbfed919080580018f0` |
| dev/src/preprocess/identifier_table.cpp | `764d4d03d621453864641d7dda74cd2d2663c6330435ba520f2569ef4f3b7daa` |
| dev/src/preprocess/source.cpp | `8ab6c4ec36e83373723587fafeeec23828cf49fb346740355d4fe53c0da237bb` |
| dev/src/preprocess/token_cursor.cpp | `6b57f892d83e59a4ee4016b37eac2ea54717662dab21d3a1454305f1838536de` |
| dev/src/preprocess/token_output.cpp | `391539c3e91e7acf73ad7982326b78e357faac938277372205aab8b4f5745dde` |
| dev/src/preprocess/identifier_table.h | `38508fcd71f6d38258282f3689e5ef9311bee3657d69488260ea7ee38d3a349c` |
| dev/src/preprocess/source.h | `9810a89de174bb4c0ffc6b2b6ed7fc23f2ad59126c6b3c479d12134bdbc7a26e` |
| dev/src/preprocess/token_cursor.h | `4e6e99acb8849e3c43ea1147a26df160e25890ee7ef73664cfac1eee728c1aba` |
| dev/src/preprocess/token_output.h | `c5e994a22437cc25b0c02fc56b83c482668391e023ed831196d87f3e84fdad22` |
| dev/src/support/testing/test_runner.cpp | `d33cbae9c8265d8e128650871a43961e20063cd1b3c386ee20dfa0f23c50ae61` |
| dev/Makefile | `be5823c83f053b5b7df9bc4f369d716ca2e3410a05cd9ae28e6851d2bfceefda` |
| dev/frontend_source_sets.mk | `8b024f6c90a3cb513f05c900136e82f38562d7357bcd24cabecd3ac52015b2f1` |
| audit-abba/benchmark-frozen.py | `261553eed8453235b68454d00ede434a0bd1cafbbe5bf58e7f90ee0782eaca2c` |
| audit-abba-pinned/benchmark-frozen.py | `261553eed8453235b68454d00ede434a0bd1cafbbe5bf58e7f90ee0782eaca2c` |
| audit-telemetry/benchmark-frozen.py | `3e3cba7fcbdd44e8f61808c2549dde642bb134e16148dcb0cd1423ca0fd7f068` |

## Initial implementation evidence (historical)

Run: 2026-09-07 UTC, implementation commit `78a0872d6`. This is a compiler
baseline and telemetry comparison, not an optimization or generated-code claim.

Host: Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43; Intel(R) Xeon(R) CPU @ 2.20GHz.
Compiler: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0; `-std=gnu++11 -Wall -O3`, supplied test runner enabled.
Frozen binary SHA-256: `bdb50c54f12d6ea1b58204edcaed46aeb298cfd5a987eae169c8238532ec03d5`.

A uses the default CLI; B uses the same frozen binary with `--stats`. Each
workload has two A/A calibration pairs followed by two ABBA blocks. Full output
SHA-256 matches between A and B before timing. Wall time includes startup, input
reading, scanning and required token formatting to `/dev/null`; RSS is measured
by `/usr/bin/time`. Output-hashing warmups are excluded from these times/RSS.

Generated-program runtime and text size: **N/A** for every row (PA1 emits tokens).

| Workload | Input bytes | A median [min, max], s | B median [min, max], s | Max RSS, KiB |
| --- | ---: | --- | --- | ---: |
| repeated-8 | 8388511 | 1.503379 [1.493619, 1.509029] | 1.495884 [1.493225, 1.500399] | 12096 |
| repeated-32 | 33554322 | 5.998776 [5.980678, 6.040731] | 5.986913 [5.956214, 6.039817] | 36668 |
| translations-8 | 8388579 | 0.646877 [0.642922, 0.648646] | 0.650390 [0.645059, 0.658675] | 12096 |
| self-source-8 | 8364960 | 1.065686 [1.052848, 1.257020] | 1.070881 [1.050439, 1.115105] | 12092 |
| unique-200000 | 2288891 | 0.219877 [0.211730, 0.222805] | 0.216234 [0.213934, 0.222599] | 19952 |

| Workload | A/A changes, % | Paired ABBA B/A changes, % |
| --- | --- | --- |
| repeated-8 | +1.629, +0.665 | -0.241, -0.557 |
| repeated-32 | -0.368, -0.516 | -0.490, +0.080 |
| translations-8 | +0.311, +1.146 | +0.006, +1.478 |
| self-source-8 | -18.041, -2.601 | -5.824, +0.080 |
| unique-200000 | -2.087, +7.895 | -0.529, -0.680 |

Telemetry has no repeatable measured cost above the observed noise here; negative
changes are not claimed as speedups. Self-source calibration was noisy (18%);
its -5.8% paired result does not establish a benefit. The unique-name workload
also showed 7.9% calibration variation. These limits preclude small-effect claims.

All predeclared envelopes passed: 4x repeated input took 3.9902x median latency
(limit 6x); decoding work stayed below `2 * source bytes + 64`; RSS stayed below
`4 * source bytes + 32 MiB`. Repeated-name identifier storage stayed at 618 bytes
for both sizes. No generated-program optimization, work or growth claim applies.

| Workload | Tokens incl. EOF | Decoded units | Copied spelling bytes | Unique IDs | ID storage bytes | Table storage growth events |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| repeated-8 | 4767572 | 8388511 | 0 | 14 | 618 | 11 |
| repeated-32 | 19070443 | 33554322 | 0 | 14 | 618 | 11 |
| translations-8 | 1417789 | 7679685 | 827043 | 3 | 170 | 5 |
| self-source-8 | 2864689 | 8364960 | 0 | 238 | 11170 | 24 |
| unique-200000 | 400002 | 2288891 | 0 | 200000 | 11370358 | 54 |

The repeated inputs cover template syntax, loops, calls, memory indexing and
floating-point spellings; those are lexical workloads only. Self-source input
concatenates the PA1 entry point and shared implementation headers/sources.
Translation input mixes UCNs, UTF-8, splices, comments and raw literals. The
unique-name input checks the geometric growth of the TU identifier table.

### Frozen inputs and equivalent outputs

| Workload | Input SHA-256 | A/B output SHA-256 |
| --- | --- | --- |
| repeated-8 | `459d7af709027a678fa0c2ed27b86ba51abb340ecbdf366cf5bf4266de65697a` | `450ef8eb52b9098983995289322fa7a3bd6e55e94745e660ad524acf74be282b` |
| repeated-32 | `7719804e57fa7d3127ad47fc3cfeb98ae91eb55d9e3c8e716ce16ba1f4299e9a` | `230812a1ab8149a665a388c1ef9a657460112016560f0217848c3920d460f854` |
| translations-8 | `b7481e1da8429d9a50c8f98d5d9a80644c2032b44277ec66bcd789ed188c5b45` | `3bcb87f0de344e0dc80044a825976c6c29004b4e4034faeb288991c090caada8` |
| self-source-8 | `b0c15a6bbd2f156c9d5aec65f29e6c3414efb9b0515c744d6f30a0466eb16c42` | `05ef6ab33805328a33d6271a57f7c5f7d98b586c23584788f9bff07231735c51` |
| unique-200000 | `d95badc2ff7c09100d90c66097e21d7731a2088ab0a739d28c54a04735af07d1` | `2fcc157f41436c7eda2815513c3a52b1cbe85a46e6c9a9574f48926fef14d61b` |

### All wall-time/RSS observations

Each cell is `seconds / peak KiB`. Samples 0-3 are A/A calibration; samples
4-7 and 8-11 are ABBA blocks. No samples were discarded.

| Sample / mode | repeated-8 | repeated-32 | translations-8 | self-source-8 | unique-200000 |
| --- | --- | --- | --- | --- | --- |
| 0 / A | 1.485485385 / 12092 | 6.004854228 / 36668 | 0.649554305 / 12092 | 1.291414311 / 12088 | 0.219046384 / 19808 |
| 1 / A | 1.509677934 / 12084 | 5.982773704 / 36488 | 0.651575050 / 12096 | 1.058434084 / 11968 | 0.214474785 / 19836 |
| 2 / A | 1.490051111 / 12048 | 5.994785496 / 36664 | 0.638405715 / 11852 | 1.066981846 / 12088 | 0.218468237 / 19820 |
| 3 / A | 1.499954364 / 12064 | 5.963829665 / 36484 | 0.645720997 / 11908 | 1.039228768 / 12092 | 0.235715835 / 19672 |
| 4 / A | 1.493618971 / 12096 | 5.980677614 / 36528 | 0.648646218 / 12048 | 1.257020032 / 12072 | 0.222804960 / 19836 |
| 5 / B | 1.493224901 / 11956 | 5.981534028 / 36644 | 0.645059165 / 12096 | 1.115104514 / 12092 | 0.218300704 / 19836 |
| 6 / B | 1.493552731 / 11912 | 5.956214006 / 36636 | 0.646583764 / 11952 | 1.078079424 / 12088 | 0.213934460 / 19836 |
| 7 / A | 1.500387932 / 12088 | 6.015825325 / 36268 | 0.642922214 / 11964 | 1.071793328 / 11964 | 0.211729504 / 19748 |
| 8 / A | 1.509029041 / 11924 | 5.981726733 / 36668 | 0.646022737 / 12088 | 1.052848340 / 12076 | 0.217370054 / 19836 |
| 9 / B | 1.500398735 / 12072 | 5.992292643 / 36624 | 0.654196236 / 12068 | 1.063682863 / 12044 | 0.222598584 / 19924 |
| 10 / B | 1.498215494 / 11924 | 6.039817208 / 36652 | 0.658675199 / 12056 | 1.050438985 / 12088 | 0.214166775 / 19952 |
| 11 / A | 1.506369284 / 12092 | 6.040731019 / 36528 | 0.647732260 / 11952 | 1.059578884 / 11912 | 0.222384798 / 19792 |

Reproduce with `python3 student.tests/pa1/benchmark.py`. The complete machine
manifest, source hashes, JSON observations (including per-run phase counters),
input files and frozen binary remain in
`obj/student-pa1/performance-20260907-075606/`. Generated artifacts are untracked.
No baseline comparison against the incorrect PA1 stub was made.
