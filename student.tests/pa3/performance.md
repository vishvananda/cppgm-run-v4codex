# PA3 performance evidence

PA3 emits expression results; generated executable runtime and text size are **N/A**.
No speedup over the nonfunctional entry stub or generated-code benefit is claimed.
These measurements establish frontend resource bounds and expose telemetry cost.

Protocol: frozen binaries, flags, source/harness hashes and fixed input hashes;
independently checked complete outputs; two A/A pairs followed by two ABBA
blocks per workload. A is ordinary execution, B adds `--stats`. Wall time
and peak RSS are measured separately from untimed hashing/warmup. All runs
pin CPU 0; each invocation has a 60-second timeout. The second campaign
runs after sanitizer/course checks have finished, with the final harness.
No samples are discarded. Small positive and negative changes are disclosed;
noise and optional instrumentation cost do not justify an optimization claim.

Budgets and workload meanings are in [README.md](README.md). Every campaign
passes the compiler work, memory and scaling envelopes. Flat chains retain
two values/one operator (35 bytes allocated capacity); repeated 4/16 MiB
inputs retain identical 140-byte expression scratch and 126-byte name storage.
Deep conditional/parenthesis storage tracks nesting, without host recursion.
RSS below is `/usr/bin/time` whole-child peak; telemetry samples RSS before
reporting and teardown, and can be up to 284 KiB lower in these campaigns.
Untimed output-hash launch RSS is retained in raw summaries but is excluded
from the measurement table and resource conclusions.

## Campaign 1: performance-20260907-092035

Frozen artifacts: `obj/student-pa3/performance-20260907-092035`. All 84 samples retained.
Compiler: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`; CPU: `model name	: Intel(R) Xeon(R) CPU @ 2.20GHz`.
Host: `Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43`; affinity: `[0]`.
Binary SHA-256: `b997d3e9716361414a1036475c11d596c6179b4833e8e094133bf583a113793c`.
Harness SHA-256: `a469237999d4024484afa24a8200805a0f1875dc0cd116815f4ac1d5792934b8`.

Build flags and host-tool size (not generated text):

```text
-std=gnu++11 -Wall -O3
-std=gnu++11 -Wall -O3  -Dmain=test_runner_real_main
   text	   data	    bss	    dec	    hex	filename
  86656	   4520	   1272	  92448	  16920	/home/vishvananda/work/v4codex/obj/student-pa3/performance-20260907-092035/ppexpr-frozen
```

| Workload | A median [min, max] s | B median [min, max] s | Peak RSS A/B KiB | ABBA B/A changes % | A/A noise % |
| --- | --- | --- | --- | --- | --- |
| repeated-4 | 0.435700 [0.434595, 0.440645] | 0.440099 [0.439681, 0.448644] | 8032/8032 | +1.968, +0.541 | -1.273, +9.616 |
| repeated-16 | 1.731616 [1.724209, 1.734343] | 1.739454 [1.736346, 1.749644] | 20320/20316 | +0.388, +0.858 | +1.256, -0.817 |
| chain-4 | 0.659626 [0.657358, 0.666482] | 0.670145 [0.665724, 0.695723] | 11872/11868 | +1.538, +2.897 | -0.873, -1.053 |
| nested-4 | 0.646085 [0.643180, 0.648664] | 0.653233 [0.646750, 0.673431] | 20000/20112 | +2.841, +0.459 | +1.623, -1.406 |
| conditional-4 | 0.587920 [0.587086, 0.589066] | 0.590275 [0.589327, 0.593902] | 44768/44768 | +0.360, +0.642 | +0.812, +2.744 |
| unique-names-200000 | 0.451020 [0.449016, 0.460832] | 0.459741 [0.455439, 0.478535] | 24156/24160 | +2.036, +2.550 | +0.242, -0.871 |
| recovery-4 | 0.448557 [0.442484, 0.496519] | 0.450893 [0.438999, 0.461935] | 8032/8028 | +1.421, -4.914 | +5.274, -0.034 |

Fourfold-source latency ratios: A 3.974328x, B 3.952414x (budget ≤ 6x).

| Workload | Bytes | Input SHA-256 | Checked output SHA-256 |
| --- | --- | --- | --- |
| repeated-4 | 4194303 | `e83ab754ffff234bcd45408a3aed3543dfd49f5c093d80bd7127299fdcd4cc3c` | `6a46eb5ef70df17b150e03c864ecf655f2d47e559a708b7fb9245570a6fd0218` |
| repeated-16 | 16777212 | `ea1859584bb8de67addc04e41278aae01eafa12d92673c2fe752c956f841ca28` | `57cd1750585c62594de8e3e17ea0742bea128d2468b611848632b17955e7621b` |
| chain-4 | 4194306 | `0bb83d30a2bdb79d37367a78305671e39443ab10450af7fc8b5a76db239a382e` | `85c89cd46776f458747ccb429fdfe556dd9d899c7638d5c134a2c5adac856ce9` |
| nested-4 | 4194306 | `d493565d0ec67aae1e9cbf439bd66be3b0ce2b4d4b7af14478803225cbc8e026` | `c9974c49ecebffc752c4b70105593dce5822973be6626ed55d1f5137af92818f` |
| conditional-4 | 4194303 | `2e072a48fb403f3a2e3afeafcd4fd9fb3d66cf1886839844bb3edf0a9946be1f` | `bfb37c2d12f03df96f36b524d360b2825276f5c59f4d7068aee3f8573e893287` |
| unique-names-200000 | 5688890 | `195d761971673154dfc3fda22165d251ce78856920b514f6a4be2eeb32714363` | `11919a637e3dab8bad135a0f241ecf924b8cd946c0007c2d49d3d2714215cc9e` |
| recovery-4 | 4194299 | `9eac0c83cd7acca8ee5f046476a83a5c07b498fef3a05289c1d328a9888633a4` | `599b1d9c4c9288cd3d0b95a5bf17e15d919c6acd19ac045ac7f7f9461339a875` |

| Workload | Expression tokens / reductions | Max values / operators | Scratch bytes / growths | Name bytes |
| --- | --- | --- | --- | --- |
| repeated-4 | 1696572 / 518397 | 5 / 3 | 140 / 7 | 126 |
| repeated-16 | 6786288 / 2073588 | 5 / 3 | 140 / 7 | 126 |
| chain-4 | 4194305 / 2097152 | 2 / 1 | 35 / 3 | 64 |
| nested-4 | 4194305 / 0 | 1 / 2097152 | 6291472 / 23 | 64 |
| conditional-4 | 3355442 / 838861 | 1677721 / 838861 | 36700160 / 43 | 64 |
| unique-names-200000 | 1200000 / 200000 | 3 / 1 | 67 / 4 | 11369756 |
| recovery-4 | 1880203 / 0 | 1 / 1 | 19 / 2 | 174 |

All observations; first four rows per workload are A/A calibration,
then two ABBA blocks. Times retain nanosecond-resolution observations.

| Workload | Index | Block/mode | Wall s | Peak RSS KiB | Telemetry RSS KiB | Read ms | Evaluate/emit ms |
| --- | --- | --- | --- | --- | --- | --- | --- |
| repeated-4 | 0 | AA/A | 0.445232989 | 8012 | — | — | — |
| repeated-4 | 1 | AA/A | 0.439566933 | 8028 | — | — | — |
| repeated-4 | 2 | AA/A | 0.434649566 | 8028 | — | — | — |
| repeated-4 | 3 | AA/A | 0.476447244 | 8024 | — | — | — |
| repeated-4 | 4 | ABBA/A | 0.435108122 | 8004 | — | — | — |
| repeated-4 | 5 | ABBA/B | 0.439908297 | 7844 | 7844 | 8.41717 | 423.858 |
| repeated-4 | 6 | ABBA/B | 0.448643572 | 8028 | 8028 | 8.40624 | 432.715 |
| repeated-4 | 7 | ABBA/A | 0.436292515 | 8032 | — | — | — |
| repeated-4 | 8 | ABBA/A | 0.440644907 | 7852 | — | — | — |
| repeated-4 | 9 | ABBA/B | 0.440289820 | 8032 | 8032 | 8.4179 | 424.215 |
| repeated-4 | 10 | ABBA/B | 0.439681448 | 7976 | 7976 | 8.32062 | 423.823 |
| repeated-4 | 11 | ABBA/A | 0.434595157 | 8028 | — | — | — |
| repeated-16 | 0 | AA/A | 1.729142124 | 20288 | — | — | — |
| repeated-16 | 1 | AA/A | 1.750864471 | 20312 | — | — | — |
| repeated-16 | 2 | AA/A | 1.751003075 | 20312 | — | — | — |
| repeated-16 | 3 | AA/A | 1.736692940 | 20320 | — | — | — |
| repeated-16 | 4 | ABBA/A | 1.733779920 | 20316 | — | — | — |
| repeated-16 | 5 | ABBA/B | 1.740340763 | 20316 | 20316 | 34.5799 | 1696.61 |
| repeated-16 | 6 | ABBA/B | 1.736345838 | 20316 | 20316 | 34.9161 | 1692.4 |
| repeated-16 | 7 | ABBA/A | 1.729452062 | 20316 | — | — | — |
| repeated-16 | 8 | ABBA/A | 1.724208559 | 20136 | — | — | — |
| repeated-16 | 9 | ABBA/B | 1.749643990 | 20136 | 20136 | 34.3287 | 1706.56 |
| repeated-16 | 10 | ABBA/B | 1.738566706 | 20140 | 20140 | 36.0356 | 1693.77 |
| repeated-16 | 11 | ABBA/A | 1.734342890 | 20268 | — | — | — |
| chain-4 | 0 | AA/A | 0.689424590 | 11692 | — | — | — |
| chain-4 | 1 | AA/A | 0.683406263 | 11828 | — | — | — |
| chain-4 | 2 | AA/A | 0.661863804 | 11848 | — | — | — |
| chain-4 | 3 | AA/A | 0.654892819 | 11872 | — | — | — |
| chain-4 | 4 | ABBA/A | 0.657575470 | 11868 | — | — | — |
| chain-4 | 5 | ABBA/B | 0.665723752 | 11692 | 11692 | 12.6094 | 645.489 |
| chain-4 | 6 | ABBA/B | 0.673818328 | 11840 | 11840 | 12.802 | 653.192 |
| chain-4 | 7 | ABBA/A | 0.661676201 | 11624 | — | — | — |
| chain-4 | 8 | ABBA/A | 0.666482435 | 11868 | — | — | — |
| chain-4 | 9 | ABBA/B | 0.666470734 | 11868 | 11868 | 12.865 | 645.443 |
| chain-4 | 10 | ABBA/B | 0.695723251 | 11868 | 11868 | 13.5968 | 674.644 |
| chain-4 | 11 | ABBA/A | 0.657358496 | 11816 | — | — | — |
| nested-4 | 0 | AA/A | 0.641209226 | 19996 | — | — | — |
| nested-4 | 1 | AA/A | 0.651615284 | 19996 | — | — | — |
| nested-4 | 2 | AA/A | 0.659823346 | 19768 | — | — | — |
| nested-4 | 3 | AA/A | 0.650547795 | 20000 | — | — | — |
| nested-4 | 4 | ABBA/A | 0.643180279 | 19968 | — | — | — |
| nested-4 | 5 | ABBA/B | 0.673431017 | 19960 | 19960 | 12.7226 | 651.313 |
| nested-4 | 6 | ABBA/B | 0.655120466 | 19984 | 19984 | 12.7689 | 633.346 |
| nested-4 | 7 | ABBA/A | 0.648663648 | 19944 | — | — | — |
| nested-4 | 8 | ABBA/A | 0.648042259 | 19996 | — | — | — |
| nested-4 | 9 | ABBA/B | 0.646750149 | 20112 | 19856 | 12.7603 | 625.841 |
| nested-4 | 10 | ABBA/B | 0.651345585 | 20080 | 19796 | 13.0733 | 629.506 |
| nested-4 | 11 | ABBA/A | 0.644127784 | 19996 | — | — | — |
| conditional-4 | 0 | AA/A | 0.580889248 | 44724 | — | — | — |
| conditional-4 | 1 | AA/A | 0.585603653 | 44736 | — | — | — |
| conditional-4 | 2 | AA/A | 0.588053669 | 44744 | — | — | — |
| conditional-4 | 3 | AA/A | 0.604187904 | 44592 | — | — | — |
| conditional-4 | 4 | ABBA/A | 0.587723644 | 44736 | — | — | — |
| conditional-4 | 5 | ABBA/B | 0.589326718 | 44748 | 44748 | 8.33632 | 570.11 |
| conditional-4 | 6 | ABBA/B | 0.589709745 | 44744 | 44744 | 8.43705 | 570.071 |
| conditional-4 | 7 | ABBA/A | 0.587086047 | 44768 | — | — | — |
| conditional-4 | 8 | ABBA/A | 0.588116547 | 44740 | — | — | — |
| conditional-4 | 9 | ABBA/B | 0.593902491 | 44768 | 44768 | 8.46383 | 574.288 |
| conditional-4 | 10 | ABBA/B | 0.590840660 | 44588 | 44588 | 8.51528 | 571.183 |
| conditional-4 | 11 | ABBA/A | 0.589065895 | 44732 | — | — | — |
| unique-names-200000 | 0 | AA/A | 0.457469721 | 23976 | — | — | — |
| unique-names-200000 | 1 | AA/A | 0.458576977 | 24116 | — | — | — |
| unique-names-200000 | 2 | AA/A | 0.460403786 | 24128 | — | — | — |
| unique-names-200000 | 3 | AA/A | 0.456393915 | 23908 | — | — | — |
| unique-names-200000 | 4 | ABBA/A | 0.449015690 | 24156 | — | — | — |
| unique-names-200000 | 5 | ABBA/B | 0.461457246 | 24116 | 24116 | 14.1309 | 437.737 |
| unique-names-200000 | 6 | ABBA/B | 0.458025016 | 24160 | 24160 | 14.3216 | 434.457 |
| unique-names-200000 | 7 | ABBA/A | 0.452119990 | 24144 | — | — | — |
| unique-names-200000 | 8 | ABBA/A | 0.460831968 | 23980 | — | — | — |
| unique-names-200000 | 9 | ABBA/B | 0.455438540 | 24132 | 24132 | 14.2621 | 428.814 |
| unique-names-200000 | 10 | ABBA/B | 0.478534864 | 24156 | 24156 | 14.4145 | 438.242 |
| unique-names-200000 | 11 | ABBA/A | 0.449920841 | 23980 | — | — | — |
| recovery-4 | 0 | AA/A | 0.444166198 | 8032 | — | — | — |
| recovery-4 | 1 | AA/A | 0.467591965 | 7796 | — | — | — |
| recovery-4 | 2 | AA/A | 0.439619683 | 8028 | — | — | — |
| recovery-4 | 3 | AA/A | 0.439470592 | 8028 | — | — | — |
| recovery-4 | 4 | ABBA/A | 0.446490954 | 8028 | — | — | — |
| recovery-4 | 5 | ABBA/B | 0.461934826 | 8028 | 8028 | 8.3481 | 426.753 |
| recovery-4 | 6 | ABBA/B | 0.447921852 | 7992 | 7992 | 8.38481 | 431.437 |
| recovery-4 | 7 | ABBA/A | 0.450622081 | 8024 | — | — | — |
| recovery-4 | 8 | ABBA/A | 0.496518658 | 8016 | — | — | — |
| recovery-4 | 9 | ABBA/B | 0.453863259 | 8016 | 8016 | 8.38094 | 437.53 |
| recovery-4 | 10 | ABBA/B | 0.438999091 | 8016 | 8016 | 8.42939 | 424.204 |
| recovery-4 | 11 | ABBA/A | 0.442483852 | 7824 | — | — | — |
## Campaign 2: performance-20260907-092337

Frozen artifacts: `obj/student-pa3/performance-20260907-092337`. All 84 samples retained.
Compiler: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`; CPU: `model name	: Intel(R) Xeon(R) CPU @ 2.20GHz`.
Host: `Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43`; affinity: `[0]`.
Binary SHA-256: `b997d3e9716361414a1036475c11d596c6179b4833e8e094133bf583a113793c`.
Harness SHA-256: `4bcbfbca422fcb6ebab6417b5d373547ac74c88400d3bc8095318a08b0e12134`.

Build flags and host-tool size (not generated text):

```text
-std=gnu++11 -Wall -O3
-std=gnu++11 -Wall -O3  -Dmain=test_runner_real_main
   text	   data	    bss	    dec	    hex	filename
  86656	   4520	   1272	  92448	  16920	/home/vishvananda/work/v4codex/obj/student-pa3/performance-20260907-092337/ppexpr-frozen
```

| Workload | A median [min, max] s | B median [min, max] s | Peak RSS A/B KiB | ABBA B/A changes % | A/A noise % |
| --- | --- | --- | --- | --- | --- |
| repeated-4 | 0.437491 [0.435035, 0.439597] | 0.444017 [0.441235, 0.506894] | 8032/8032 | +1.936, +7.932 | +1.273, -0.984 |
| repeated-16 | 1.735391 [1.733311, 1.743361] | 1.747998 [1.738424, 1.759240] | 20320/20312 | +1.040, +0.292 | +0.665, +0.404 |
| chain-4 | 0.661010 [0.657445, 0.664970] | 0.669067 [0.666781, 0.672153] | 11872/11868 | +1.828, +0.647 | -0.370, -0.500 |
| nested-4 | 0.654049 [0.652936, 0.658459] | 0.658408 [0.654048, 0.677744] | 20000/20000 | +2.259, -0.031 | -2.857, -0.752 |
| conditional-4 | 0.588100 [0.584007, 0.591944] | 0.588060 [0.584170, 0.590828] | 44772/44764 | -0.988, +0.913 | +0.012, -9.134 |
| unique-names-200000 | 0.456625 [0.453903, 0.460071] | 0.457325 [0.452062, 0.463757] | 24160/24228 | -1.415, +1.784 | +1.219, -1.235 |
| recovery-4 | 0.443454 [0.439966, 0.445834] | 0.446045 [0.440534, 0.448302] | 8028/8028 | +0.615, +0.312 | -1.372, +0.600 |

Fourfold-source latency ratios: A 3.966689x, B 3.936782x (budget ≤ 6x).

| Workload | Bytes | Input SHA-256 | Checked output SHA-256 |
| --- | --- | --- | --- |
| repeated-4 | 4194303 | `e83ab754ffff234bcd45408a3aed3543dfd49f5c093d80bd7127299fdcd4cc3c` | `6a46eb5ef70df17b150e03c864ecf655f2d47e559a708b7fb9245570a6fd0218` |
| repeated-16 | 16777212 | `ea1859584bb8de67addc04e41278aae01eafa12d92673c2fe752c956f841ca28` | `57cd1750585c62594de8e3e17ea0742bea128d2468b611848632b17955e7621b` |
| chain-4 | 4194306 | `0bb83d30a2bdb79d37367a78305671e39443ab10450af7fc8b5a76db239a382e` | `85c89cd46776f458747ccb429fdfe556dd9d899c7638d5c134a2c5adac856ce9` |
| nested-4 | 4194306 | `d493565d0ec67aae1e9cbf439bd66be3b0ce2b4d4b7af14478803225cbc8e026` | `c9974c49ecebffc752c4b70105593dce5822973be6626ed55d1f5137af92818f` |
| conditional-4 | 4194303 | `2e072a48fb403f3a2e3afeafcd4fd9fb3d66cf1886839844bb3edf0a9946be1f` | `bfb37c2d12f03df96f36b524d360b2825276f5c59f4d7068aee3f8573e893287` |
| unique-names-200000 | 5688890 | `195d761971673154dfc3fda22165d251ce78856920b514f6a4be2eeb32714363` | `11919a637e3dab8bad135a0f241ecf924b8cd946c0007c2d49d3d2714215cc9e` |
| recovery-4 | 4194299 | `9eac0c83cd7acca8ee5f046476a83a5c07b498fef3a05289c1d328a9888633a4` | `599b1d9c4c9288cd3d0b95a5bf17e15d919c6acd19ac045ac7f7f9461339a875` |

| Workload | Expression tokens / reductions | Max values / operators | Scratch bytes / growths | Name bytes |
| --- | --- | --- | --- | --- |
| repeated-4 | 1696572 / 518397 | 5 / 3 | 140 / 7 | 126 |
| repeated-16 | 6786288 / 2073588 | 5 / 3 | 140 / 7 | 126 |
| chain-4 | 4194305 / 2097152 | 2 / 1 | 35 / 3 | 64 |
| nested-4 | 4194305 / 0 | 1 / 2097152 | 6291472 / 23 | 64 |
| conditional-4 | 3355442 / 838861 | 1677721 / 838861 | 36700160 / 43 | 64 |
| unique-names-200000 | 1200000 / 200000 | 3 / 1 | 67 / 4 | 11369756 |
| recovery-4 | 1880203 / 0 | 1 / 1 | 19 / 2 | 174 |

All observations; first four rows per workload are A/A calibration,
then two ABBA blocks. Times retain nanosecond-resolution observations.

| Workload | Index | Block/mode | Wall s | Peak RSS KiB | Telemetry RSS KiB | Read ms | Evaluate/emit ms |
| --- | --- | --- | --- | --- | --- | --- | --- |
| repeated-4 | 0 | AA/A | 0.436276636 | 8024 | — | — | — |
| repeated-4 | 1 | AA/A | 0.441831815 | 8028 | — | — | — |
| repeated-4 | 2 | AA/A | 0.440311645 | 8032 | — | — | — |
| repeated-4 | 3 | AA/A | 0.435979275 | 8024 | — | — | — |
| repeated-4 | 4 | ABBA/A | 0.435035451 | 8024 | — | — | — |
| repeated-4 | 5 | ABBA/B | 0.441520436 | 8032 | 8032 | 8.27039 | 425.572 |
| repeated-4 | 6 | ABBA/B | 0.446513341 | 7976 | 7976 | 8.43719 | 430.535 |
| repeated-4 | 7 | ABBA/A | 0.436129351 | 7792 | — | — | — |
| repeated-4 | 8 | ABBA/A | 0.438852823 | 8024 | — | — | — |
| repeated-4 | 9 | ABBA/B | 0.506893747 | 8024 | 8024 | 8.29104 | 491.013 |
| repeated-4 | 10 | ABBA/B | 0.441235472 | 8020 | 8020 | 8.4296 | 425.162 |
| repeated-4 | 11 | ABBA/A | 0.439596573 | 8008 | — | — | — |
| repeated-16 | 0 | AA/A | 1.731428966 | 20312 | — | — | — |
| repeated-16 | 1 | AA/A | 1.742949232 | 20312 | — | — | — |
| repeated-16 | 2 | AA/A | 1.732914122 | 20280 | — | — | — |
| repeated-16 | 3 | AA/A | 1.739916121 | 20140 | — | — | — |
| repeated-16 | 4 | ABBA/A | 1.733311387 | 20276 | — | — | — |
| repeated-16 | 5 | ABBA/B | 1.759240162 | 20136 | 20136 | 35.4177 | 1714.37 |
| repeated-16 | 6 | ABBA/B | 1.744547970 | 20312 | 20312 | 35.3202 | 1699.98 |
| repeated-16 | 7 | ABBA/A | 1.734416237 | 20312 | — | — | — |
| repeated-16 | 8 | ABBA/A | 1.736365501 | 20320 | — | — | — |
| repeated-16 | 9 | ABBA/B | 1.751447102 | 20312 | 20312 | 34.7221 | 1707.74 |
| repeated-16 | 10 | ABBA/B | 1.738424493 | 20140 | 20140 | 35.0269 | 1694.3 |
| repeated-16 | 11 | ABBA/A | 1.743361190 | 20140 | — | — | — |
| chain-4 | 0 | AA/A | 0.658390935 | 11852 | — | — | — |
| chain-4 | 1 | AA/A | 0.655955801 | 11872 | — | — | — |
| chain-4 | 2 | AA/A | 0.663024289 | 11868 | — | — | — |
| chain-4 | 3 | AA/A | 0.659710457 | 11828 | — | — | — |
| chain-4 | 4 | ABBA/A | 0.657458014 | 11868 | — | — | — |
| chain-4 | 5 | ABBA/B | 0.672153223 | 11868 | 11868 | 12.8155 | 651.45 |
| chain-4 | 6 | ABBA/B | 0.666780754 | 11688 | 11688 | 13.0757 | 646.306 |
| chain-4 | 7 | ABBA/A | 0.657445129 | 11868 | — | — | — |
| chain-4 | 8 | ABBA/A | 0.664970094 | 11868 | — | — | — |
| chain-4 | 9 | ABBA/B | 0.667516545 | 11844 | 11844 | 13.0869 | 646.523 |
| chain-4 | 10 | ABBA/B | 0.670616899 | 11688 | 11688 | 13.1818 | 646.326 |
| chain-4 | 11 | ABBA/A | 0.664562128 | 11828 | — | — | — |
| nested-4 | 0 | AA/A | 0.659479630 | 19948 | — | — | — |
| nested-4 | 1 | AA/A | 0.640641280 | 20000 | — | — | — |
| nested-4 | 2 | AA/A | 0.645906951 | 19996 | — | — | — |
| nested-4 | 3 | AA/A | 0.641046960 | 19816 | — | — | — |
| nested-4 | 4 | ABBA/A | 0.653709318 | 19944 | — | — | — |
| nested-4 | 5 | ABBA/B | 0.677743876 | 19960 | 19960 | 12.9947 | 655.637 |
| nested-4 | 6 | ABBA/B | 0.658421153 | 19996 | 19996 | 12.8485 | 636.632 |
| nested-4 | 7 | ABBA/A | 0.652936274 | 19992 | — | — | — |
| nested-4 | 8 | ABBA/A | 0.654389242 | 20000 | — | — | — |
| nested-4 | 9 | ABBA/B | 0.654047978 | 19996 | 19996 | 12.9173 | 631.944 |
| nested-4 | 10 | ABBA/B | 0.658394293 | 20000 | 20000 | 13.1315 | 636.56 |
| nested-4 | 11 | ABBA/A | 0.658458509 | 19984 | — | — | — |
| conditional-4 | 0 | AA/A | 0.583273577 | 44524 | — | — | — |
| conditional-4 | 1 | AA/A | 0.583345928 | 44588 | — | — | — |
| conditional-4 | 2 | AA/A | 0.642871594 | 44740 | — | — | — |
| conditional-4 | 3 | AA/A | 0.584152478 | 44768 | — | — | — |
| conditional-4 | 4 | ABBA/A | 0.591943850 | 44768 | — | — | — |
| conditional-4 | 5 | ABBA/B | 0.584169544 | 44764 | 44764 | 8.42849 | 565.169 |
| conditional-4 | 6 | ABBA/B | 0.587717994 | 44732 | 44732 | 8.46448 | 568.722 |
| conditional-4 | 7 | ABBA/A | 0.591642045 | 44768 | — | — | — |
| conditional-4 | 8 | ABBA/A | 0.584007492 | 44772 | — | — | — |
| conditional-4 | 9 | ABBA/B | 0.588402198 | 44764 | 44764 | 8.45031 | 569.467 |
| conditional-4 | 10 | ABBA/B | 0.590827970 | 44764 | 44764 | 8.54535 | 571.581 |
| conditional-4 | 11 | ABBA/A | 0.584557252 | 44768 | — | — | — |
| unique-names-200000 | 0 | AA/A | 0.454113827 | 24156 | — | — | — |
| unique-names-200000 | 1 | AA/A | 0.459650111 | 24140 | — | — | — |
| unique-names-200000 | 2 | AA/A | 0.460886495 | 24156 | — | — | — |
| unique-names-200000 | 3 | AA/A | 0.455194425 | 24148 | — | — | — |
| unique-names-200000 | 4 | ABBA/A | 0.460071183 | 24100 | — | — | — |
| unique-names-200000 | 5 | ABBA/B | 0.452636423 | 24156 | 24156 | 14.2673 | 428.99 |
| unique-names-200000 | 6 | ABBA/B | 0.452062006 | 24228 | 23972 | 14.0432 | 428.922 |
| unique-names-200000 | 7 | ABBA/A | 0.457609636 | 24160 | — | — | — |
| unique-names-200000 | 8 | ABBA/A | 0.453902828 | 24156 | — | — | — |
| unique-names-200000 | 9 | ABBA/B | 0.462014134 | 24156 | 24156 | 14.1788 | 438.65 |
| unique-names-200000 | 10 | ABBA/B | 0.463756578 | 24152 | 24152 | 14.1688 | 440.212 |
| unique-names-200000 | 11 | ABBA/A | 0.455640671 | 24124 | — | — | — |
| recovery-4 | 0 | AA/A | 0.446574446 | 8024 | — | — | — |
| recovery-4 | 1 | AA/A | 0.440447741 | 8028 | — | — | — |
| recovery-4 | 2 | AA/A | 0.442485295 | 8028 | — | — | — |
| recovery-4 | 3 | AA/A | 0.445141173 | 8016 | — | — | — |
| recovery-4 | 4 | ABBA/A | 0.443426137 | 8000 | — | — | — |
| recovery-4 | 5 | ABBA/B | 0.448302308 | 8024 | 8024 | 8.55012 | 432.006 |
| recovery-4 | 6 | ABBA/B | 0.444065058 | 7856 | 7856 | 8.53756 | 428.01 |
| recovery-4 | 7 | ABBA/A | 0.443482341 | 8028 | — | — | — |
| recovery-4 | 8 | ABBA/A | 0.439966359 | 8028 | — | — | — |
| recovery-4 | 9 | ABBA/B | 0.440534490 | 8028 | 8028 | 8.65868 | 424.529 |
| recovery-4 | 10 | ABBA/B | 0.448025635 | 8024 | 8024 | 8.38981 | 432.197 |
| recovery-4 | 11 | ABBA/A | 0.445833970 | 7848 | — | — | — |

Frozen build-source hashes (identical across both campaigns):

| Source | SHA-256 |
| --- | --- |
| dev/ppexpr.cpp | `dd56bb0048104a66b126cf5ea474f2ea694d2b90c3c884661004e2d1966f2510` |
| dev/src/preprocess/expression.cpp | `1b26ff4caa8cd6a167b838b7ea0365f89f0865495cd88cc30749eaeae0e3067c` |
| dev/src/preprocess/expression_value.cpp | `69b3408a42d8838078257e964bf3e2ad11c6acd7ccc52578c776c12b107d23c5` |
| dev/src/preprocess/identifier_table.cpp | `764d4d03d621453864641d7dda74cd2d2663c6330435ba520f2569ef4f3b7daa` |
| dev/src/preprocess/source.cpp | `8ab6c4ec36e83373723587fafeeec23828cf49fb346740355d4fe53c0da237bb` |
| dev/src/preprocess/token_cursor.cpp | `95563177f4d50c369c3cb0918f6c7ff108f45e5ec9c8e89bdc49871162421c39` |
| dev/src/preprocess/token_output.cpp | `391539c3e91e7acf73ad7982326b78e357faac938277372205aab8b4f5745dde` |
| dev/src/preprocess/expression.h | `0634d59413601c74aeb6ce54b4f192171a5ccc78b7bb0eb7717f2431ab96bd6b` |
| dev/src/preprocess/expression_value.h | `50af831c9abf79fe1484e6c15fa0a218f400a717d30cc1e7f166e182e38e8a63` |
| dev/src/preprocess/identifier_table.h | `38508fcd71f6d38258282f3689e5ef9311bee3657d69488260ea7ee38d3a349c` |
| dev/src/preprocess/source.h | `9810a89de174bb4c0ffc6b2b6ed7fc23f2ad59126c6b3c479d12134bdbc7a26e` |
| dev/src/preprocess/token_cursor.h | `ed6eab679072801a248f38684a026de35ae2a58efefdd6d4719cc46d5871e435` |
| dev/src/preprocess/token_output.h | `c5e994a22437cc25b0c02fc56b83c482668391e023ed831196d87f3e84fdad22` |
| dev/src/posttoken/cursor.cpp | `bebee3a41c03a6737ed881077f755f775633c2fb82c7aa75503effb513914374` |
| dev/src/posttoken/literal.cpp | `9478fb70d3cc52a896af7b5e8a1a28119c0b42e72493b43bad3304477c42f415` |
| dev/src/posttoken/number.cpp | `eb3fe8b6dc8854c238274c60204320795327c82a2393efc676b246e168c495c4` |
| dev/src/posttoken/output.cpp | `6a9cab69e45c7f3a7b5c8ba682f21dc93a97cb558c4802ddf71b6b38c7d3ba54` |
| dev/src/posttoken/token_types.cpp | `5b02b37b204bd63501c309d8ceb61d4e9455ff3b9ee7909add9cd4099cb1aef0` |
| dev/src/posttoken/cursor.h | `58ff4ff585b6ba5dc37d77bc3a77b461f5e4c8879c703154289716e5ae528d98` |
| dev/src/posttoken/literal.h | `a71cb923f8e011af34b8a1f32c7db5339e15fed9ca4656d1125dc21b69ea113e` |
| dev/src/posttoken/number.h | `8f2d94b07309af398448dd9ecfb24258f3f63505571e2e3299da76199ab588b6` |
| dev/src/posttoken/output.h | `8e43cc9eb6f9e4736bc4b34ad37f3f957ffa99a717c1ab84b8a5c19ef3d22ae9` |
| dev/src/posttoken/token.h | `3d09ef949cae144030b60c50bef5a944624037a4942f6ba634d189a1ca64ec33` |
| dev/src/posttoken/token_types.h | `871b468fd872ed348f791f462a7c9ac080b9a70afe29763514dc3faa1d2a7dd9` |
| dev/src/support/testing/test_runner.cpp | `d33cbae9c8265d8e128650871a43961e20063cd1b3c386ee20dfa0f23c50ae61` |
| dev/Makefile | `be5823c83f053b5b7df9bc4f369d716ca2e3410a05cd9ae28e6851d2bfceefda` |
| dev/frontend_source_sets.mk | `fe08ce2ea6d4a290cda654696faed697fcd1864687775b42997f8e84497f5045` |

Total: 168 retained timed observations. Compiler outputs and final source/binary
provenance agree. No runtime/text-size or optimization-profitability claim applies.
