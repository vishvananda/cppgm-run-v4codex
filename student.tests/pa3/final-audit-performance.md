# PA3 performance evidence

PA3 emits expression results; generated executable runtime and text size are **N/A**.
No speedup over the nonfunctional entry stub or generated-code benefit is claimed.
These measurements establish frontend resource bounds and compare the declared A/B modes.

Protocol: frozen binaries, flags, source/harness hashes and fixed input hashes;
independently checked complete outputs; two A/A pairs followed by two ABBA
blocks per workload. A/B modes, host and affinity are recorded below. Wall time
and peak RSS are measured separately from untimed hashing/warmup. All runs
have a 60-second timeout. Campaigns run serially after correctness checks,
with no compiler builds or test processes running during measurement.
No samples are discarded. Small positive and negative changes are disclosed;
interpret changes against A/A noise and the predeclared budgets.

Budgets and workload meanings are in [README.md](README.md). Every campaign
passes the compiler work, memory and scaling envelopes. Flat chains retain
two values/one operator (35 bytes allocated capacity); repeated 4/16 MiB
inputs retain identical 140-byte expression scratch and 126-byte name storage.
Deep conditional/parenthesis storage tracks nesting, without host recursion.
RSS below is `/usr/bin/time` whole-child peak; telemetry samples RSS before
reporting and teardown, and can be lower than the whole-child peak.
Untimed output-hash launch RSS is retained in raw summaries but is excluded
from the measurement table and resource conclusions.

## Campaign 1: final-audit-ab-1

Frozen artifacts: `obj/student-pa3/final-audit-ab-1`. All 108 samples retained.
Compiler: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`; CPU: `model name	: Intel(R) Xeon(R) CPU @ 2.20GHz`.
Host: `Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43`; affinity: `[0]`.
Binary SHA-256: `5082974968e5eaaf3fa72fc8e06cca754155c2552126eab9dbf33e7946a9dbeb`.
A: baseline, default flags; B: current, default flags.
Baseline SHA-256: `b997d3e9716361414a1036475c11d596c6179b4833e8e094133bf583a113793c`; source revision: `c0bb94df8`.
Harness SHA-256: `da1e9ae9642fd53a919502c8e2735401dd7c5dd539691a33718c143f7753a75d`.

Build flags and host-tool size (not generated text):

```text
-std=gnu++11 -Wall -O3
-std=gnu++11 -Wall -O3  -Dmain=test_runner_real_main
   text	   data	    bss	    dec	    hex	filename
  86656	   4520	   1272	  92448	  16920	/home/vishvananda/work/v4codex/obj/student-pa3/final-audit-ab-1/ppexpr-baseline
   text	   data	    bss	    dec	    hex	filename
  86968	   4520	   1272	  92760	  16a58	/home/vishvananda/work/v4codex/obj/student-pa3/final-audit-ab-1/ppexpr-frozen
```

| Workload | A median [min, max] s | B median [min, max] s | Peak RSS A/B KiB | ABBA B/A changes % | A/A noise % |
| --- | --- | --- | --- | --- | --- |
| repeated-4 | 0.436424 [0.433701, 0.439111] | 0.436786 [0.435654, 0.439071] | 8036/8028 | +0.083, +0.219 | +1.267, +8.213 |
| repeated-16 | 1.727638 [1.723762, 1.730457] | 1.720812 [1.719551, 1.732234] | 20316/20320 | -0.238, -0.228 | -0.062, -1.950 |
| chain-4 | 0.657535 [0.655744, 0.662947] | 0.638653 [0.636359, 0.639794] | 11872/11864 | -2.837, -3.260 | +0.576, +0.152 |
| nested-4 | 0.654773 [0.653165, 0.659867] | 0.644597 [0.642853, 0.655179] | 19996/20000 | -1.089, -1.608 | -3.614, -0.226 |
| conditional-4 | 0.587056 [0.584505, 0.598015] | 0.582276 [0.581175, 0.585330] | 44768/44772 | -0.733, -1.434 | +0.030, -0.074 |
| unique-names-200000 | 0.451020 [0.447377, 0.463831] | 0.456552 [0.450101, 0.467643] | 24156/24152 | +1.122, +0.816 | +3.401, -0.743 |
| recovery-4 | 0.442635 [0.440041, 0.459218] | 0.376687 [0.376145, 0.378348] | 8032/8028 | -14.859, -16.137 | -10.933, +0.268 |
| floating-rejection-4 | 0.408697 [0.407348, 0.410910] | 0.252487 [0.250144, 0.257016] | 8028/8028 | -38.921, -37.315 | -0.121, -0.199 |
| suffix-rejection-200000 | 0.255328 [0.251421, 0.262835] | 0.206270 [0.202190, 0.209499] | 20856/8024 | -19.968, -19.190 | -6.266, +31.111 |

Fourfold-source latency ratios: A 3.958626x, B 3.939715x (budget ≤ 6x).

| Workload | Bytes | Input SHA-256 | Checked output SHA-256 |
| --- | --- | --- | --- |
| repeated-4 | 4194303 | `e83ab754ffff234bcd45408a3aed3543dfd49f5c093d80bd7127299fdcd4cc3c` | `6a46eb5ef70df17b150e03c864ecf655f2d47e559a708b7fb9245570a6fd0218` |
| repeated-16 | 16777212 | `ea1859584bb8de67addc04e41278aae01eafa12d92673c2fe752c956f841ca28` | `57cd1750585c62594de8e3e17ea0742bea128d2468b611848632b17955e7621b` |
| chain-4 | 4194306 | `0bb83d30a2bdb79d37367a78305671e39443ab10450af7fc8b5a76db239a382e` | `85c89cd46776f458747ccb429fdfe556dd9d899c7638d5c134a2c5adac856ce9` |
| nested-4 | 4194306 | `d493565d0ec67aae1e9cbf439bd66be3b0ce2b4d4b7af14478803225cbc8e026` | `c9974c49ecebffc752c4b70105593dce5822973be6626ed55d1f5137af92818f` |
| conditional-4 | 4194303 | `2e072a48fb403f3a2e3afeafcd4fd9fb3d66cf1886839844bb3edf0a9946be1f` | `bfb37c2d12f03df96f36b524d360b2825276f5c59f4d7068aee3f8573e893287` |
| unique-names-200000 | 5688890 | `195d761971673154dfc3fda22165d251ce78856920b514f6a4be2eeb32714363` | `11919a637e3dab8bad135a0f241ecf924b8cd946c0007c2d49d3d2714215cc9e` |
| recovery-4 | 4194299 | `9eac0c83cd7acca8ee5f046476a83a5c07b498fef3a05289c1d328a9888633a4` | `599b1d9c4c9288cd3d0b95a5bf17e15d919c6acd19ac045ac7f7f9461339a875` |
| floating-rejection-4 | 4194288 | `5777f66353ff5228fc31fcf2d9a5ed6bce6918f7aa34447a89f91918dd8705b5` | `4d0bff9e9e36219fbfe35e9390dd5ba4552e6c0d66ae255a597445bbf6fe4cde` |
| suffix-rejection-200000 | 3488890 | `3065ce2d9fbea49166352f9ca580a2acc0ad38ea095a538f8b50ce4e0ff58f54` | `8a65e585aee274d238cbfd07e2733f8b7d6057cf659453a0c542564cbbe95827` |

| Workload | Expression tokens / reductions | Max values / operators | Scratch bytes / growths | Name bytes |
| --- | --- | --- | --- | --- |
| repeated-4 | 1696572 / 518397 | 5 / 3 | 140 / 7 | 126 |
| repeated-16 | 6786288 / 2073588 | 5 / 3 | 140 / 7 | 126 |
| chain-4 | 4194305 / 2097152 | 2 / 1 | 35 / 3 | 64 |
| nested-4 | 4194305 / 0 | 1 / 2097152 | 6291472 / 23 | 64 |
| conditional-4 | 3355442 / 838861 | 1677721 / 838861 | 36700160 / 43 | 64 |
| unique-names-200000 | 1200000 / 200000 | 3 / 1 | 67 / 4 | 11369756 |
| recovery-4 | 1880203 / 0 | 1 / 1 | 19 / 2 | 174 |
| floating-rejection-4 | 466032 / 0 | 1 / 0 | 16 / 1 | 64 |
| suffix-rejection-200000 | 400000 / 0 | 1 / 0 | 16 / 1 | 64 |

| Rejection workload | Identifiers A/B | Name storage A/B bytes |
| --- | --- | --- |
| floating-rejection-4 | 0/0 | 64/64 |
| suffix-rejection-200000 | 200000/0 | 12935068/64 |
floating-rejection-4: both ABBA blocks improve beyond this campaign's A/A noise.
suffix-rejection-200000: A/A noise exceeds the measured gain; this campaign alone is inconclusive.

Other workloads pass the ≤5% plus calibrated noise budget on the campaign mean
of paired changes. Individual block excursions remain visible in the table.
Host-tool text stays within 1% growth; peak RSS B stays within A + 1 MiB.


All observations; first four rows per workload are A/A calibration,
then two ABBA blocks. Times retain nanosecond-resolution observations.

| Workload | Index | Block/mode | Wall s | Peak RSS KiB | Telemetry RSS KiB | Read ms | Evaluate/emit ms |
| --- | --- | --- | --- | --- | --- | --- | --- |
| repeated-4 | 0 | AA/A | 0.432841485 | 8028 | — | — | — |
| repeated-4 | 1 | AA/A | 0.438324743 | 8012 | — | — | — |
| repeated-4 | 2 | AA/A | 0.442828030 | 8028 | — | — | — |
| repeated-4 | 3 | AA/A | 0.479196790 | 8028 | — | — | — |
| repeated-4 | 4 | ABBA/A | 0.437577622 | 8028 | — | — | — |
| repeated-4 | 5 | ABBA/B | 0.437802146 | 8024 | — | — | — |
| repeated-4 | 6 | ABBA/B | 0.435769540 | 8024 | — | — | — |
| repeated-4 | 7 | ABBA/A | 0.435269648 | 8036 | — | — | — |
| repeated-4 | 8 | ABBA/A | 0.439111298 | 8024 | — | — | — |
| repeated-4 | 9 | ABBA/B | 0.439070812 | 8028 | — | — | — |
| repeated-4 | 10 | ABBA/B | 0.435654191 | 8028 | — | — | — |
| repeated-4 | 11 | ABBA/A | 0.433701462 | 8036 | — | — | — |
| repeated-16 | 0 | AA/A | 1.722630848 | 20312 | — | — | — |
| repeated-16 | 1 | AA/A | 1.721568464 | 20312 | — | — | — |
| repeated-16 | 2 | AA/A | 1.757061913 | 20136 | — | — | — |
| repeated-16 | 3 | AA/A | 1.722798526 | 20072 | — | — | — |
| repeated-16 | 4 | ABBA/A | 1.729563890 | 20316 | — | — | — |
| repeated-16 | 5 | ABBA/B | 1.732233948 | 20320 | — | — | — |
| repeated-16 | 6 | ABBA/B | 1.719550735 | 20320 | — | — | — |
| repeated-16 | 7 | ABBA/A | 1.730456544 | 20316 | — | — | — |
| repeated-16 | 8 | ABBA/A | 1.723762408 | 20312 | — | — | — |
| repeated-16 | 9 | ABBA/B | 1.720276282 | 20312 | — | — | — |
| repeated-16 | 10 | ABBA/B | 1.721347056 | 20088 | — | — | — |
| repeated-16 | 11 | ABBA/A | 1.725711606 | 20276 | — | — | — |
| chain-4 | 0 | AA/A | 0.657527439 | 11872 | — | — | — |
| chain-4 | 1 | AA/A | 0.661311674 | 11856 | — | — | — |
| chain-4 | 2 | AA/A | 0.655760496 | 11828 | — | — | — |
| chain-4 | 3 | AA/A | 0.656757652 | 11864 | — | — | — |
| chain-4 | 4 | ABBA/A | 0.659263569 | 11868 | — | — | — |
| chain-4 | 5 | ABBA/B | 0.639793541 | 11864 | — | — | — |
| chain-4 | 6 | ABBA/B | 0.637906946 | 11856 | — | — | — |
| chain-4 | 7 | ABBA/A | 0.655743512 | 11688 | — | — | — |
| chain-4 | 8 | ABBA/A | 0.662946662 | 11628 | — | — | — |
| chain-4 | 9 | ABBA/B | 0.636358504 | 11844 | — | — | — |
| chain-4 | 10 | ABBA/B | 0.639399996 | 11692 | — | — | — |
| chain-4 | 11 | ABBA/A | 0.655806771 | 11688 | — | — | — |
| nested-4 | 0 | AA/A | 0.684127414 | 19980 | — | — | — |
| nested-4 | 1 | AA/A | 0.659402695 | 19996 | — | — | — |
| nested-4 | 2 | AA/A | 0.645330773 | 19792 | — | — | — |
| nested-4 | 3 | AA/A | 0.643870199 | 19996 | — | — | — |
| nested-4 | 4 | ABBA/A | 0.659867162 | 19980 | — | — | — |
| nested-4 | 5 | ABBA/B | 0.655179318 | 20000 | — | — | — |
| nested-4 | 6 | ABBA/B | 0.646267644 | 19992 | — | — | — |
| nested-4 | 7 | ABBA/A | 0.655912871 | 19980 | — | — | — |
| nested-4 | 8 | ABBA/A | 0.653164790 | 19984 | — | — | — |
| nested-4 | 9 | ABBA/B | 0.642926711 | 19996 | — | — | — |
| nested-4 | 10 | ABBA/B | 0.642853408 | 19812 | — | — | — |
| nested-4 | 11 | ABBA/A | 0.653634064 | 19984 | — | — | — |
| conditional-4 | 0 | AA/A | 0.585198053 | 44768 | — | — | — |
| conditional-4 | 1 | AA/A | 0.585373815 | 44588 | — | — | — |
| conditional-4 | 2 | AA/A | 0.583448997 | 44764 | — | — | — |
| conditional-4 | 3 | AA/A | 0.583015200 | 44588 | — | — | — |
| conditional-4 | 4 | ABBA/A | 0.588650708 | 44720 | — | — | — |
| conditional-4 | 5 | ABBA/B | 0.582572449 | 44756 | — | — | — |
| conditional-4 | 6 | ABBA/B | 0.581979457 | 44768 | — | — | — |
| conditional-4 | 7 | ABBA/A | 0.584505367 | 44756 | — | — | — |
| conditional-4 | 8 | ABBA/A | 0.598015229 | 44768 | — | — | — |
| conditional-4 | 9 | ABBA/B | 0.585330006 | 44592 | — | — | — |
| conditional-4 | 10 | ABBA/B | 0.581175142 | 44772 | — | — | — |
| conditional-4 | 11 | ABBA/A | 0.585461437 | 44752 | — | — | — |
| unique-names-200000 | 0 | AA/A | 0.450356686 | 24132 | — | — | — |
| unique-names-200000 | 1 | AA/A | 0.465671438 | 24136 | — | — | — |
| unique-names-200000 | 2 | AA/A | 0.462019977 | 24156 | — | — | — |
| unique-names-200000 | 3 | AA/A | 0.458586026 | 24156 | — | — | — |
| unique-names-200000 | 4 | ABBA/A | 0.452044562 | 24132 | — | — | — |
| unique-names-200000 | 5 | ABBA/B | 0.467642708 | 24152 | — | — | — |
| unique-names-200000 | 6 | ABBA/B | 0.458513485 | 24140 | — | — | — |
| unique-names-200000 | 7 | ABBA/A | 0.463831151 | 24156 | — | — | — |
| unique-names-200000 | 8 | ABBA/A | 0.447376672 | 23912 | — | — | — |
| unique-names-200000 | 9 | ABBA/B | 0.454590947 | 24116 | — | — | — |
| unique-names-200000 | 10 | ABBA/B | 0.450100949 | 24128 | — | — | — |
| unique-names-200000 | 11 | ABBA/A | 0.449994789 | 24116 | — | — | — |
| recovery-4 | 0 | AA/A | 0.497134698 | 8012 | — | — | — |
| recovery-4 | 1 | AA/A | 0.442785040 | 7780 | — | — | — |
| recovery-4 | 2 | AA/A | 0.442387342 | 7988 | — | — | — |
| recovery-4 | 3 | AA/A | 0.443572861 | 7992 | — | — | — |
| recovery-4 | 4 | ABBA/A | 0.440040521 | 8032 | — | — | — |
| recovery-4 | 5 | ABBA/B | 0.376144557 | 7784 | — | — | — |
| recovery-4 | 6 | ABBA/B | 0.376853178 | 8028 | — | — | — |
| recovery-4 | 7 | ABBA/A | 0.444370957 | 7996 | — | — | — |
| recovery-4 | 8 | ABBA/A | 0.459218445 | 8024 | — | — | — |
| recovery-4 | 9 | ABBA/B | 0.376521432 | 8028 | — | — | — |
| recovery-4 | 10 | ABBA/B | 0.378348362 | 7844 | — | — | — |
| recovery-4 | 11 | ABBA/A | 0.440899233 | 8028 | — | — | — |
| floating-rejection-4 | 0 | AA/A | 0.413377177 | 8028 | — | — | — |
| floating-rejection-4 | 1 | AA/A | 0.412878440 | 8024 | — | — | — |
| floating-rejection-4 | 2 | AA/A | 0.411833616 | 8008 | — | — | — |
| floating-rejection-4 | 3 | AA/A | 0.411012054 | 7784 | — | — | — |
| floating-rejection-4 | 4 | ABBA/A | 0.409672962 | 8028 | — | — | — |
| floating-rejection-4 | 5 | ABBA/B | 0.250144257 | 8028 | — | — | — |
| floating-rejection-4 | 6 | ABBA/B | 0.251063797 | 8004 | — | — | — |
| floating-rejection-4 | 7 | ABBA/A | 0.410910319 | 7996 | — | — | — |
| floating-rejection-4 | 8 | ABBA/A | 0.407347843 | 8000 | — | — | — |
| floating-rejection-4 | 9 | ABBA/B | 0.253909481 | 8028 | — | — | — |
| floating-rejection-4 | 10 | ABBA/B | 0.257015974 | 8024 | — | — | — |
| floating-rejection-4 | 11 | ABBA/A | 0.407720691 | 8028 | — | — | — |
| suffix-rejection-200000 | 0 | AA/A | 0.268029047 | 20836 | — | — | — |
| suffix-rejection-200000 | 1 | AA/A | 0.251235543 | 20848 | — | — | — |
| suffix-rejection-200000 | 2 | AA/A | 0.254971992 | 20816 | — | — | — |
| suffix-rejection-200000 | 3 | AA/A | 0.334295092 | 20856 | — | — | — |
| suffix-rejection-200000 | 4 | ABBA/A | 0.251421055 | 20844 | — | — | — |
| suffix-rejection-200000 | 5 | ABBA/B | 0.202189951 | 8024 | — | — | — |
| suffix-rejection-200000 | 6 | ABBA/B | 0.209379804 | 8020 | — | — | — |
| suffix-rejection-200000 | 7 | ABBA/A | 0.262835369 | 20812 | — | — | — |
| suffix-rejection-200000 | 8 | ABBA/A | 0.252403083 | 20804 | — | — | — |
| suffix-rejection-200000 | 9 | ABBA/B | 0.203160769 | 8012 | — | — | — |
| suffix-rejection-200000 | 10 | ABBA/B | 0.209498771 | 7988 | — | — | — |
| suffix-rejection-200000 | 11 | ABBA/A | 0.258252931 | 20848 | — | — | — |
## Campaign 2: final-audit-ab-2

Frozen artifacts: `obj/student-pa3/final-audit-ab-2`. All 108 samples retained.
Compiler: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`; CPU: `model name	: Intel(R) Xeon(R) CPU @ 2.20GHz`.
Host: `Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43`; affinity: `[0]`.
Binary SHA-256: `5082974968e5eaaf3fa72fc8e06cca754155c2552126eab9dbf33e7946a9dbeb`.
A: baseline, default flags; B: current, default flags.
Baseline SHA-256: `b997d3e9716361414a1036475c11d596c6179b4833e8e094133bf583a113793c`; source revision: `c0bb94df8`.
Harness SHA-256: `da1e9ae9642fd53a919502c8e2735401dd7c5dd539691a33718c143f7753a75d`.

Build flags and host-tool size (not generated text):

```text
-std=gnu++11 -Wall -O3
-std=gnu++11 -Wall -O3  -Dmain=test_runner_real_main
   text	   data	    bss	    dec	    hex	filename
  86656	   4520	   1272	  92448	  16920	/home/vishvananda/work/v4codex/obj/student-pa3/final-audit-ab-2/ppexpr-baseline
   text	   data	    bss	    dec	    hex	filename
  86968	   4520	   1272	  92760	  16a58	/home/vishvananda/work/v4codex/obj/student-pa3/final-audit-ab-2/ppexpr-frozen
```

| Workload | A median [min, max] s | B median [min, max] s | Peak RSS A/B KiB | ABBA B/A changes % | A/A noise % |
| --- | --- | --- | --- | --- | --- |
| repeated-4 | 0.434097 [0.432484, 0.442868] | 0.436332 [0.434275, 0.441038] | 8028/8032 | -0.841, +1.365 | -0.530, -0.570 |
| repeated-16 | 1.720863 [1.715096, 1.735166] | 1.734180 [1.729544, 1.757853] | 20320/20320 | +1.865, -0.007 | +0.015, +1.248 |
| chain-4 | 0.658573 [0.655452, 0.660332] | 0.640820 [0.636341, 0.647916] | 11876/11868 | -2.412, -2.679 | +0.369, +0.900 |
| nested-4 | 0.652083 [0.647995, 0.660014] | 0.650982 [0.646278, 0.652672] | 20000/20000 | -0.608, -0.255 | -1.086, -14.240 |
| conditional-4 | 0.584374 [0.583331, 0.591438] | 0.580674 [0.580297, 0.581740] | 44776/44772 | -0.503, -1.213 | -1.333, +0.230 |
| unique-names-200000 | 0.459811 [0.455825, 0.501014] | 0.453420 [0.451377, 0.456559] | 24160/24156 | -4.901, -1.608 | +0.773, -3.742 |
| recovery-4 | 0.440588 [0.438211, 0.443910] | 0.374708 [0.372605, 0.380390] | 8036/8028 | -15.518, -14.069 | -0.367, +0.654 |
| floating-rejection-4 | 0.409213 [0.406476, 0.411603] | 0.244763 [0.243443, 0.250170] | 8028/8028 | -40.401, -39.445 | -3.708, +1.564 |
| suffix-rejection-200000 | 0.255833 [0.253561, 0.257686] | 0.208121 [0.204298, 0.208809] | 20852/8028 | -18.395, -19.448 | -1.731, -0.369 |

Fourfold-source latency ratios: A 3.964237x, B 3.974450x (budget ≤ 6x).

| Workload | Bytes | Input SHA-256 | Checked output SHA-256 |
| --- | --- | --- | --- |
| repeated-4 | 4194303 | `e83ab754ffff234bcd45408a3aed3543dfd49f5c093d80bd7127299fdcd4cc3c` | `6a46eb5ef70df17b150e03c864ecf655f2d47e559a708b7fb9245570a6fd0218` |
| repeated-16 | 16777212 | `ea1859584bb8de67addc04e41278aae01eafa12d92673c2fe752c956f841ca28` | `57cd1750585c62594de8e3e17ea0742bea128d2468b611848632b17955e7621b` |
| chain-4 | 4194306 | `0bb83d30a2bdb79d37367a78305671e39443ab10450af7fc8b5a76db239a382e` | `85c89cd46776f458747ccb429fdfe556dd9d899c7638d5c134a2c5adac856ce9` |
| nested-4 | 4194306 | `d493565d0ec67aae1e9cbf439bd66be3b0ce2b4d4b7af14478803225cbc8e026` | `c9974c49ecebffc752c4b70105593dce5822973be6626ed55d1f5137af92818f` |
| conditional-4 | 4194303 | `2e072a48fb403f3a2e3afeafcd4fd9fb3d66cf1886839844bb3edf0a9946be1f` | `bfb37c2d12f03df96f36b524d360b2825276f5c59f4d7068aee3f8573e893287` |
| unique-names-200000 | 5688890 | `195d761971673154dfc3fda22165d251ce78856920b514f6a4be2eeb32714363` | `11919a637e3dab8bad135a0f241ecf924b8cd946c0007c2d49d3d2714215cc9e` |
| recovery-4 | 4194299 | `9eac0c83cd7acca8ee5f046476a83a5c07b498fef3a05289c1d328a9888633a4` | `599b1d9c4c9288cd3d0b95a5bf17e15d919c6acd19ac045ac7f7f9461339a875` |
| floating-rejection-4 | 4194288 | `5777f66353ff5228fc31fcf2d9a5ed6bce6918f7aa34447a89f91918dd8705b5` | `4d0bff9e9e36219fbfe35e9390dd5ba4552e6c0d66ae255a597445bbf6fe4cde` |
| suffix-rejection-200000 | 3488890 | `3065ce2d9fbea49166352f9ca580a2acc0ad38ea095a538f8b50ce4e0ff58f54` | `8a65e585aee274d238cbfd07e2733f8b7d6057cf659453a0c542564cbbe95827` |

| Workload | Expression tokens / reductions | Max values / operators | Scratch bytes / growths | Name bytes |
| --- | --- | --- | --- | --- |
| repeated-4 | 1696572 / 518397 | 5 / 3 | 140 / 7 | 126 |
| repeated-16 | 6786288 / 2073588 | 5 / 3 | 140 / 7 | 126 |
| chain-4 | 4194305 / 2097152 | 2 / 1 | 35 / 3 | 64 |
| nested-4 | 4194305 / 0 | 1 / 2097152 | 6291472 / 23 | 64 |
| conditional-4 | 3355442 / 838861 | 1677721 / 838861 | 36700160 / 43 | 64 |
| unique-names-200000 | 1200000 / 200000 | 3 / 1 | 67 / 4 | 11369756 |
| recovery-4 | 1880203 / 0 | 1 / 1 | 19 / 2 | 174 |
| floating-rejection-4 | 466032 / 0 | 1 / 0 | 16 / 1 | 64 |
| suffix-rejection-200000 | 400000 / 0 | 1 / 0 | 16 / 1 | 64 |

| Rejection workload | Identifiers A/B | Name storage A/B bytes |
| --- | --- | --- |
| floating-rejection-4 | 0/0 | 64/64 |
| suffix-rejection-200000 | 200000/0 | 12935068/64 |
floating-rejection-4: both ABBA blocks improve beyond this campaign's A/A noise.
suffix-rejection-200000: both ABBA blocks improve beyond this campaign's A/A noise.

Other workloads pass the ≤5% plus calibrated noise budget on the campaign mean
of paired changes. Individual block excursions remain visible in the table.
Host-tool text stays within 1% growth; peak RSS B stays within A + 1 MiB.


All observations; first four rows per workload are A/A calibration,
then two ABBA blocks. Times retain nanosecond-resolution observations.

| Workload | Index | Block/mode | Wall s | Peak RSS KiB | Telemetry RSS KiB | Read ms | Evaluate/emit ms |
| --- | --- | --- | --- | --- | --- | --- | --- |
| repeated-4 | 0 | AA/A | 0.436140103 | 8016 | — | — | — |
| repeated-4 | 1 | AA/A | 0.433828803 | 8028 | — | — | — |
| repeated-4 | 2 | AA/A | 0.435710600 | 8028 | — | — | — |
| repeated-4 | 3 | AA/A | 0.433228369 | 8016 | — | — | — |
| repeated-4 | 4 | ABBA/A | 0.442868122 | 8016 | — | — | — |
| repeated-4 | 5 | ABBA/B | 0.434274587 | 8020 | — | — | — |
| repeated-4 | 6 | ABBA/B | 0.436323128 | 7848 | — | — | — |
| repeated-4 | 7 | ABBA/A | 0.435110539 | 8024 | — | — | — |
| repeated-4 | 8 | ABBA/A | 0.432483919 | 8024 | — | — | — |
| repeated-4 | 9 | ABBA/B | 0.436341244 | 8032 | — | — | — |
| repeated-4 | 10 | ABBA/B | 0.441037675 | 7848 | — | — | — |
| repeated-4 | 11 | ABBA/A | 0.433083449 | 8020 | — | — | — |
| repeated-16 | 0 | AA/A | 1.731953674 | 20312 | — | — | — |
| repeated-16 | 1 | AA/A | 1.732212458 | 20140 | — | — | — |
| repeated-16 | 2 | AA/A | 1.726657199 | 20316 | — | — | — |
| repeated-16 | 3 | AA/A | 1.748206306 | 20316 | — | — | — |
| repeated-16 | 4 | ABBA/A | 1.716754798 | 20316 | — | — | — |
| repeated-16 | 5 | ABBA/B | 1.757852640 | 20316 | — | — | — |
| repeated-16 | 6 | ABBA/B | 1.738018871 | 20132 | — | — | — |
| repeated-16 | 7 | ABBA/A | 1.715096464 | 20316 | — | — | — |
| repeated-16 | 8 | ABBA/A | 1.724971505 | 20320 | — | — | — |
| repeated-16 | 9 | ABBA/B | 1.730341715 | 20320 | — | — | — |
| repeated-16 | 10 | ABBA/B | 1.729544383 | 20296 | — | — | — |
| repeated-16 | 11 | ABBA/A | 1.735165900 | 20316 | — | — | — |
| chain-4 | 0 | AA/A | 0.656412478 | 11688 | — | — | — |
| chain-4 | 1 | AA/A | 0.658837137 | 11812 | — | — | — |
| chain-4 | 2 | AA/A | 0.654216037 | 11824 | — | — | — |
| chain-4 | 3 | AA/A | 0.660106709 | 11848 | — | — | — |
| chain-4 | 4 | ABBA/A | 0.655452305 | 11864 | — | — | — |
| chain-4 | 5 | ABBA/B | 0.644641799 | 11852 | — | — | — |
| chain-4 | 6 | ABBA/B | 0.636341306 | 11868 | — | — | — |
| chain-4 | 7 | ABBA/A | 0.657195515 | 11844 | — | — | — |
| chain-4 | 8 | ABBA/A | 0.659949996 | 11820 | — | — | — |
| chain-4 | 9 | ABBA/B | 0.647916172 | 11684 | — | — | — |
| chain-4 | 10 | ABBA/B | 0.636998792 | 11824 | — | — | — |
| chain-4 | 11 | ABBA/A | 0.660332261 | 11876 | — | — | — |
| nested-4 | 0 | AA/A | 0.659310463 | 19976 | — | — | — |
| nested-4 | 1 | AA/A | 0.652153193 | 19976 | — | — | — |
| nested-4 | 2 | AA/A | 0.765123398 | 19996 | — | — | — |
| nested-4 | 3 | AA/A | 0.656167443 | 19996 | — | — | — |
| nested-4 | 4 | ABBA/A | 0.649405764 | 19956 | — | — | — |
| nested-4 | 5 | ABBA/B | 0.646278405 | 20000 | — | — | — |
| nested-4 | 6 | ABBA/B | 0.649962943 | 19984 | — | — | — |
| nested-4 | 7 | ABBA/A | 0.654759465 | 20000 | — | — | — |
| nested-4 | 8 | ABBA/A | 0.660014222 | 19976 | — | — | — |
| nested-4 | 9 | ABBA/B | 0.652001287 | 19976 | — | — | — |
| nested-4 | 10 | ABBA/B | 0.652671953 | 19952 | — | — | — |
| nested-4 | 11 | ABBA/A | 0.647994967 | 19960 | — | — | — |
| conditional-4 | 0 | AA/A | 0.590421986 | 44772 | — | — | — |
| conditional-4 | 1 | AA/A | 0.582552576 | 44776 | — | — | — |
| conditional-4 | 2 | AA/A | 0.582451991 | 44772 | — | — | — |
| conditional-4 | 3 | AA/A | 0.583793720 | 44768 | — | — | — |
| conditional-4 | 4 | ABBA/A | 0.585062513 | 44768 | — | — | — |
| conditional-4 | 5 | ABBA/B | 0.580771502 | 44764 | — | — | — |
| conditional-4 | 6 | ABBA/B | 0.581739842 | 44524 | — | — | — |
| conditional-4 | 7 | ABBA/A | 0.583331480 | 44744 | — | — | — |
| conditional-4 | 8 | ABBA/A | 0.591438343 | 44768 | — | — | — |
| conditional-4 | 9 | ABBA/B | 0.580577370 | 44756 | — | — | — |
| conditional-4 | 10 | ABBA/B | 0.580296775 | 44772 | — | — | — |
| conditional-4 | 11 | ABBA/A | 0.583686269 | 44736 | — | — | — |
| unique-names-200000 | 0 | AA/A | 0.466600770 | 24156 | — | — | — |
| unique-names-200000 | 1 | AA/A | 0.470208810 | 24152 | — | — | — |
| unique-names-200000 | 2 | AA/A | 0.468421729 | 24156 | — | — | — |
| unique-names-200000 | 3 | AA/A | 0.450894971 | 24160 | — | — | — |
| unique-names-200000 | 4 | ABBA/A | 0.501014172 | 24156 | — | — | — |
| unique-names-200000 | 5 | ABBA/B | 0.453386054 | 24156 | — | — | — |
| unique-names-200000 | 6 | ABBA/B | 0.456559001 | 24120 | — | — | — |
| unique-names-200000 | 7 | ABBA/A | 0.455825326 | 24136 | — | — | — |
| unique-names-200000 | 8 | ABBA/A | 0.460395769 | 24156 | — | — | — |
| unique-names-200000 | 9 | ABBA/B | 0.451376865 | 24152 | — | — | — |
| unique-names-200000 | 10 | ABBA/B | 0.453453327 | 23976 | — | — | — |
| unique-names-200000 | 11 | ABBA/A | 0.459225700 | 23912 | — | — | — |
| recovery-4 | 0 | AA/A | 0.445220506 | 8024 | — | — | — |
| recovery-4 | 1 | AA/A | 0.443588474 | 8028 | — | — | — |
| recovery-4 | 2 | AA/A | 0.441502193 | 7792 | — | — | — |
| recovery-4 | 3 | AA/A | 0.444390749 | 8036 | — | — | — |
| recovery-4 | 4 | ABBA/A | 0.440152772 | 8028 | — | — | — |
| recovery-4 | 5 | ABBA/B | 0.372604694 | 7848 | — | — | — |
| recovery-4 | 6 | ABBA/B | 0.374271788 | 7984 | — | — | — |
| recovery-4 | 7 | ABBA/A | 0.443910231 | 7848 | — | — | — |
| recovery-4 | 8 | ABBA/A | 0.441023538 | 8004 | — | — | — |
| recovery-4 | 9 | ABBA/B | 0.380389893 | 8028 | — | — | — |
| recovery-4 | 10 | ABBA/B | 0.375143619 | 8024 | — | — | — |
| recovery-4 | 11 | ABBA/A | 0.438211043 | 8028 | — | — | — |
| floating-rejection-4 | 0 | AA/A | 0.419015616 | 8028 | — | — | — |
| floating-rejection-4 | 1 | AA/A | 0.403479445 | 8024 | — | — | — |
| floating-rejection-4 | 2 | AA/A | 0.403964688 | 8028 | — | — | — |
| floating-rejection-4 | 3 | AA/A | 0.410281162 | 8028 | — | — | — |
| floating-rejection-4 | 4 | ABBA/A | 0.411603343 | 8016 | — | — | — |
| floating-rejection-4 | 5 | ABBA/B | 0.245532111 | 8020 | — | — | — |
| floating-rejection-4 | 6 | ABBA/B | 0.243443292 | 8028 | — | — | — |
| floating-rejection-4 | 7 | ABBA/A | 0.408843311 | 8028 | — | — | — |
| floating-rejection-4 | 8 | ABBA/A | 0.406475718 | 8024 | — | — | — |
| floating-rejection-4 | 9 | ABBA/B | 0.250170148 | 8016 | — | — | — |
| floating-rejection-4 | 10 | ABBA/B | 0.243993784 | 8024 | — | — | — |
| floating-rejection-4 | 11 | ABBA/A | 0.409582233 | 8020 | — | — | — |
| suffix-rejection-200000 | 0 | AA/A | 0.260819030 | 20848 | — | — | — |
| suffix-rejection-200000 | 1 | AA/A | 0.256303451 | 20852 | — | — | — |
| suffix-rejection-200000 | 2 | AA/A | 0.257332846 | 20604 | — | — | — |
| suffix-rejection-200000 | 3 | AA/A | 0.256382634 | 20836 | — | — | — |
| suffix-rejection-200000 | 4 | ABBA/A | 0.253561471 | 20836 | — | — | — |
| suffix-rejection-200000 | 5 | ABBA/B | 0.207824793 | 8000 | — | — | — |
| suffix-rejection-200000 | 6 | ABBA/B | 0.208416414 | 8024 | — | — | — |
| suffix-rejection-200000 | 7 | ABBA/A | 0.256505911 | 20844 | — | — | — |
| suffix-rejection-200000 | 8 | ABBA/A | 0.255159596 | 20852 | — | — | — |
| suffix-rejection-200000 | 9 | ABBA/B | 0.208808877 | 7960 | — | — | — |
| suffix-rejection-200000 | 10 | ABBA/B | 0.204297836 | 8028 | — | — | — |
| suffix-rejection-200000 | 11 | ABBA/A | 0.257685650 | 20848 | — | — | — |
## Campaign 3: final-audit-ab-3

Frozen artifacts: `obj/student-pa3/final-audit-ab-3`. All 108 samples retained.
Compiler: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`; CPU: `model name	: Intel(R) Xeon(R) CPU @ 2.20GHz`.
Host: `Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43`; affinity: `[0]`.
Binary SHA-256: `5082974968e5eaaf3fa72fc8e06cca754155c2552126eab9dbf33e7946a9dbeb`.
A: baseline, default flags; B: current, default flags.
Baseline SHA-256: `b997d3e9716361414a1036475c11d596c6179b4833e8e094133bf583a113793c`; source revision: `c0bb94df8`.
Harness SHA-256: `da1e9ae9642fd53a919502c8e2735401dd7c5dd539691a33718c143f7753a75d`.

Build flags and host-tool size (not generated text):

```text
-std=gnu++11 -Wall -O3
-std=gnu++11 -Wall -O3  -Dmain=test_runner_real_main
   text	   data	    bss	    dec	    hex	filename
  86656	   4520	   1272	  92448	  16920	/home/vishvananda/work/v4codex/obj/student-pa3/final-audit-ab-3/ppexpr-baseline
   text	   data	    bss	    dec	    hex	filename
  86968	   4520	   1272	  92760	  16a58	/home/vishvananda/work/v4codex/obj/student-pa3/final-audit-ab-3/ppexpr-frozen
```

| Workload | A median [min, max] s | B median [min, max] s | Peak RSS A/B KiB | ABBA B/A changes % | A/A noise % |
| --- | --- | --- | --- | --- | --- |
| repeated-4 | 0.437474 [0.434471, 0.438313] | 0.440050 [0.438986, 0.471069] | 8032/8028 | +0.927, +3.922 | +0.228, +2.135 |
| repeated-16 | 1.732200 [1.730964, 1.732527] | 1.743335 [1.729295, 1.765117] | 20324/20320 | +0.687, +0.848 | -1.690, -0.157 |
| chain-4 | 0.657848 [0.657238, 0.667234] | 0.638365 [0.636910, 0.640121] | 11872/11868 | -2.913, -3.630 | +2.028, -0.041 |
| nested-4 | 0.654758 [0.650690, 0.657830] | 0.653025 [0.647908, 0.752861] | 19996/20000 | +7.639, -0.853 | -1.016, -0.463 |
| conditional-4 | 0.590178 [0.582964, 0.604910] | 0.592244 [0.580629, 0.606755] | 44768/44772 | -2.075, +2.397 | +4.678, +0.470 |
| unique-names-200000 | 0.454887 [0.449311, 0.462921] | 0.450635 [0.449466, 0.520715] | 24156/24152 | +5.870, -0.480 | -8.523, -2.900 |
| recovery-4 | 0.442881 [0.436397, 0.452480] | 0.376000 [0.374010, 0.423868] | 8028/8032 | -8.799, -16.426 | -1.542, +3.468 |
| floating-rejection-4 | 0.404525 [0.402788, 0.406583] | 0.244215 [0.241159, 0.257686] | 8028/8032 | -37.903, -40.093 | +0.349, +0.044 |
| suffix-rejection-200000 | 0.256839 [0.252970, 0.267766] | 0.202613 [0.201843, 0.207823] | 20852/8032 | -21.245, -21.198 | -1.123, -1.332 |

Fourfold-source latency ratios: A 3.959548x, B 3.961673x (budget ≤ 6x).

| Workload | Bytes | Input SHA-256 | Checked output SHA-256 |
| --- | --- | --- | --- |
| repeated-4 | 4194303 | `e83ab754ffff234bcd45408a3aed3543dfd49f5c093d80bd7127299fdcd4cc3c` | `6a46eb5ef70df17b150e03c864ecf655f2d47e559a708b7fb9245570a6fd0218` |
| repeated-16 | 16777212 | `ea1859584bb8de67addc04e41278aae01eafa12d92673c2fe752c956f841ca28` | `57cd1750585c62594de8e3e17ea0742bea128d2468b611848632b17955e7621b` |
| chain-4 | 4194306 | `0bb83d30a2bdb79d37367a78305671e39443ab10450af7fc8b5a76db239a382e` | `85c89cd46776f458747ccb429fdfe556dd9d899c7638d5c134a2c5adac856ce9` |
| nested-4 | 4194306 | `d493565d0ec67aae1e9cbf439bd66be3b0ce2b4d4b7af14478803225cbc8e026` | `c9974c49ecebffc752c4b70105593dce5822973be6626ed55d1f5137af92818f` |
| conditional-4 | 4194303 | `2e072a48fb403f3a2e3afeafcd4fd9fb3d66cf1886839844bb3edf0a9946be1f` | `bfb37c2d12f03df96f36b524d360b2825276f5c59f4d7068aee3f8573e893287` |
| unique-names-200000 | 5688890 | `195d761971673154dfc3fda22165d251ce78856920b514f6a4be2eeb32714363` | `11919a637e3dab8bad135a0f241ecf924b8cd946c0007c2d49d3d2714215cc9e` |
| recovery-4 | 4194299 | `9eac0c83cd7acca8ee5f046476a83a5c07b498fef3a05289c1d328a9888633a4` | `599b1d9c4c9288cd3d0b95a5bf17e15d919c6acd19ac045ac7f7f9461339a875` |
| floating-rejection-4 | 4194288 | `5777f66353ff5228fc31fcf2d9a5ed6bce6918f7aa34447a89f91918dd8705b5` | `4d0bff9e9e36219fbfe35e9390dd5ba4552e6c0d66ae255a597445bbf6fe4cde` |
| suffix-rejection-200000 | 3488890 | `3065ce2d9fbea49166352f9ca580a2acc0ad38ea095a538f8b50ce4e0ff58f54` | `8a65e585aee274d238cbfd07e2733f8b7d6057cf659453a0c542564cbbe95827` |

| Workload | Expression tokens / reductions | Max values / operators | Scratch bytes / growths | Name bytes |
| --- | --- | --- | --- | --- |
| repeated-4 | 1696572 / 518397 | 5 / 3 | 140 / 7 | 126 |
| repeated-16 | 6786288 / 2073588 | 5 / 3 | 140 / 7 | 126 |
| chain-4 | 4194305 / 2097152 | 2 / 1 | 35 / 3 | 64 |
| nested-4 | 4194305 / 0 | 1 / 2097152 | 6291472 / 23 | 64 |
| conditional-4 | 3355442 / 838861 | 1677721 / 838861 | 36700160 / 43 | 64 |
| unique-names-200000 | 1200000 / 200000 | 3 / 1 | 67 / 4 | 11369756 |
| recovery-4 | 1880203 / 0 | 1 / 1 | 19 / 2 | 174 |
| floating-rejection-4 | 466032 / 0 | 1 / 0 | 16 / 1 | 64 |
| suffix-rejection-200000 | 400000 / 0 | 1 / 0 | 16 / 1 | 64 |

| Rejection workload | Identifiers A/B | Name storage A/B bytes |
| --- | --- | --- |
| floating-rejection-4 | 0/0 | 64/64 |
| suffix-rejection-200000 | 200000/0 | 12935068/64 |
floating-rejection-4: both ABBA blocks improve beyond this campaign's A/A noise.
suffix-rejection-200000: both ABBA blocks improve beyond this campaign's A/A noise.

Other workloads pass the ≤5% plus calibrated noise budget on the campaign mean
of paired changes. Individual block excursions remain visible in the table.
Host-tool text stays within 1% growth; peak RSS B stays within A + 1 MiB.


All observations; first four rows per workload are A/A calibration,
then two ABBA blocks. Times retain nanosecond-resolution observations.

| Workload | Index | Block/mode | Wall s | Peak RSS KiB | Telemetry RSS KiB | Read ms | Evaluate/emit ms |
| --- | --- | --- | --- | --- | --- | --- | --- |
| repeated-4 | 0 | AA/A | 0.434062845 | 8008 | — | — | — |
| repeated-4 | 1 | AA/A | 0.435051039 | 8032 | — | — | — |
| repeated-4 | 2 | AA/A | 0.435072740 | 7988 | — | — | — |
| repeated-4 | 3 | AA/A | 0.444360348 | 7788 | — | — | — |
| repeated-4 | 4 | ABBA/A | 0.437548874 | 8020 | — | — | — |
| repeated-4 | 5 | ABBA/B | 0.440566849 | 8024 | — | — | — |
| repeated-4 | 6 | ABBA/B | 0.439533410 | 8028 | — | — | — |
| repeated-4 | 7 | ABBA/A | 0.434470653 | 8024 | — | — | — |
| repeated-4 | 8 | ABBA/A | 0.438313214 | 8028 | — | — | — |
| repeated-4 | 9 | ABBA/B | 0.471069170 | 8016 | — | — | — |
| repeated-4 | 10 | ABBA/B | 0.438985612 | 8000 | — | — | — |
| repeated-4 | 11 | ABBA/A | 0.437399548 | 8028 | — | — | — |
| repeated-16 | 0 | AA/A | 1.755349079 | 20324 | — | — | — |
| repeated-16 | 1 | AA/A | 1.725679891 | 20240 | — | — | — |
| repeated-16 | 2 | AA/A | 1.733441580 | 20144 | — | — | — |
| repeated-16 | 3 | AA/A | 1.730721326 | 20140 | — | — | — |
| repeated-16 | 4 | ABBA/A | 1.731899240 | 20116 | — | — | — |
| repeated-16 | 5 | ABBA/B | 1.731323681 | 20136 | — | — | — |
| repeated-16 | 6 | ABBA/B | 1.755345600 | 20276 | — | — | — |
| repeated-16 | 7 | ABBA/A | 1.730964453 | 20304 | — | — | — |
| repeated-16 | 8 | ABBA/A | 1.732527391 | 20312 | — | — | — |
| repeated-16 | 9 | ABBA/B | 1.765116998 | 20280 | — | — | — |
| repeated-16 | 10 | ABBA/B | 1.729294634 | 20320 | — | — | — |
| repeated-16 | 11 | ABBA/A | 1.732500820 | 20316 | — | — | — |
| chain-4 | 0 | AA/A | 0.658461342 | 11692 | — | — | — |
| chain-4 | 1 | AA/A | 0.671816516 | 11852 | — | — | — |
| chain-4 | 2 | AA/A | 0.659681532 | 11868 | — | — | — |
| chain-4 | 3 | AA/A | 0.659411979 | 11872 | — | — | — |
| chain-4 | 4 | ABBA/A | 0.658105844 | 11872 | — | — | — |
| chain-4 | 5 | ABBA/B | 0.636910479 | 11868 | — | — | — |
| chain-4 | 6 | ABBA/B | 0.640121199 | 11840 | — | — | — |
| chain-4 | 7 | ABBA/A | 0.657238051 | 11868 | — | — | — |
| chain-4 | 8 | ABBA/A | 0.667233531 | 11872 | — | — | — |
| chain-4 | 9 | ABBA/B | 0.638826910 | 11844 | — | — | — |
| chain-4 | 10 | ABBA/B | 0.637902973 | 11848 | — | — | — |
| chain-4 | 11 | ABBA/A | 0.657589226 | 11684 | — | — | — |
| nested-4 | 0 | AA/A | 0.659301135 | 19996 | — | — | — |
| nested-4 | 1 | AA/A | 0.652599373 | 19996 | — | — | — |
| nested-4 | 2 | AA/A | 0.646313809 | 19964 | — | — | — |
| nested-4 | 3 | AA/A | 0.643322583 | 19816 | — | — | — |
| nested-4 | 4 | ABBA/A | 0.657830284 | 19992 | — | — | — |
| nested-4 | 5 | ABBA/B | 0.655615550 | 19944 | — | — | — |
| nested-4 | 6 | ABBA/B | 0.752861239 | 20000 | — | — | — |
| nested-4 | 7 | ABBA/A | 0.650690050 | 19816 | — | — | — |
| nested-4 | 8 | ABBA/A | 0.656992376 | 19960 | — | — | — |
| nested-4 | 9 | ABBA/B | 0.650434259 | 19996 | — | — | — |
| nested-4 | 10 | ABBA/B | 0.647907716 | 19968 | — | — | — |
| nested-4 | 11 | ABBA/A | 0.652523736 | 19820 | — | — | — |
| conditional-4 | 0 | AA/A | 0.583380747 | 44724 | — | — | — |
| conditional-4 | 1 | AA/A | 0.610671193 | 44768 | — | — | — |
| conditional-4 | 2 | AA/A | 0.585360789 | 44600 | — | — | — |
| conditional-4 | 3 | AA/A | 0.588109518 | 44736 | — | — | — |
| conditional-4 | 4 | ABBA/A | 0.604910181 | 44768 | — | — | — |
| conditional-4 | 5 | ABBA/B | 0.582598195 | 44772 | — | — | — |
| conditional-4 | 6 | ABBA/B | 0.580629063 | 44592 | — | — | — |
| conditional-4 | 7 | ABBA/A | 0.582963646 | 44764 | — | — | — |
| conditional-4 | 8 | ABBA/A | 0.589959246 | 44768 | — | — | — |
| conditional-4 | 9 | ABBA/B | 0.601889500 | 44756 | — | — | — |
| conditional-4 | 10 | ABBA/B | 0.606755114 | 44768 | — | — | — |
| conditional-4 | 11 | ABBA/A | 0.590396299 | 44728 | — | — | — |
| unique-names-200000 | 0 | AA/A | 0.497815263 | 24156 | — | — | — |
| unique-names-200000 | 1 | AA/A | 0.455385338 | 24076 | — | — | — |
| unique-names-200000 | 2 | AA/A | 0.465250985 | 24152 | — | — | — |
| unique-names-200000 | 3 | AA/A | 0.451760062 | 24108 | — | — | — |
| unique-names-200000 | 4 | ABBA/A | 0.453468371 | 24156 | — | — | — |
| unique-names-200000 | 5 | ABBA/B | 0.520715213 | 24152 | — | — | — |
| unique-names-200000 | 6 | ABBA/B | 0.449465791 | 24132 | — | — | — |
| unique-names-200000 | 7 | ABBA/A | 0.462920617 | 23916 | — | — | — |
| unique-names-200000 | 8 | ABBA/A | 0.449310528 | 24152 | — | — | — |
| unique-names-200000 | 9 | ABBA/B | 0.449658241 | 24132 | — | — | — |
| unique-names-200000 | 10 | ABBA/B | 0.451611506 | 24092 | — | — | — |
| unique-names-200000 | 11 | ABBA/A | 0.456306421 | 24156 | — | — | — |
| recovery-4 | 0 | AA/A | 0.443356260 | 7852 | — | — | — |
| recovery-4 | 1 | AA/A | 0.436520229 | 8012 | — | — | — |
| recovery-4 | 2 | AA/A | 0.453777438 | 7848 | — | — | — |
| recovery-4 | 3 | AA/A | 0.469516159 | 8024 | — | — | — |
| recovery-4 | 4 | ABBA/A | 0.438638486 | 7848 | — | — | — |
| recovery-4 | 5 | ABBA/B | 0.374173625 | 8032 | — | — | — |
| recovery-4 | 6 | ABBA/B | 0.423868496 | 7848 | — | — | — |
| recovery-4 | 7 | ABBA/A | 0.436397201 | 7984 | — | — | — |
| recovery-4 | 8 | ABBA/A | 0.452480015 | 7996 | — | — | — |
| recovery-4 | 9 | ABBA/B | 0.374009845 | 8032 | — | — | — |
| recovery-4 | 10 | ABBA/B | 0.377826411 | 8028 | — | — | — |
| recovery-4 | 11 | ABBA/A | 0.447123617 | 8028 | — | — | — |
| floating-rejection-4 | 0 | AA/A | 0.408972289 | 8028 | — | — | — |
| floating-rejection-4 | 1 | AA/A | 0.410401369 | 7992 | — | — | — |
| floating-rejection-4 | 2 | AA/A | 0.409291976 | 7800 | — | — | — |
| floating-rejection-4 | 3 | AA/A | 0.409472141 | 8000 | — | — | — |
| floating-rejection-4 | 4 | ABBA/A | 0.402787546 | 8028 | — | — | — |
| floating-rejection-4 | 5 | ABBA/B | 0.257685769 | 7788 | — | — | — |
| floating-rejection-4 | 6 | ABBA/B | 0.244908973 | 8024 | — | — | — |
| floating-rejection-4 | 7 | ABBA/A | 0.406583214 | 8012 | — | — | — |
| floating-rejection-4 | 8 | ABBA/A | 0.404439469 | 8028 | — | — | — |
| floating-rejection-4 | 9 | ABBA/B | 0.243520824 | 8024 | — | — | — |
| floating-rejection-4 | 10 | ABBA/B | 0.241158832 | 8032 | — | — | — |
| floating-rejection-4 | 11 | ABBA/A | 0.404610769 | 8028 | — | — | — |
| suffix-rejection-200000 | 0 | AA/A | 0.257967550 | 20848 | — | — | — |
| suffix-rejection-200000 | 1 | AA/A | 0.255071691 | 20844 | — | — | — |
| suffix-rejection-200000 | 2 | AA/A | 0.256543309 | 20848 | — | — | — |
| suffix-rejection-200000 | 3 | AA/A | 0.253125534 | 20852 | — | — | — |
| suffix-rejection-200000 | 4 | ABBA/A | 0.252970106 | 20848 | — | — | — |
| suffix-rejection-200000 | 5 | ABBA/B | 0.207822578 | 8020 | — | — | — |
| suffix-rejection-200000 | 6 | ABBA/B | 0.202281271 | 8028 | — | — | — |
| suffix-rejection-200000 | 7 | ABBA/A | 0.267766198 | 20828 | — | — | — |
| suffix-rejection-200000 | 8 | ABBA/A | 0.256235035 | 20808 | — | — | — |
| suffix-rejection-200000 | 9 | ABBA/B | 0.202945362 | 8012 | — | — | — |
| suffix-rejection-200000 | 10 | ABBA/B | 0.201843153 | 8032 | — | — | — |
| suffix-rejection-200000 | 11 | ABBA/A | 0.257442135 | 20808 | — | — | — |

Frozen build-source hashes (identical across campaigns):

| Source | SHA-256 |
| --- | --- |
| dev/ppexpr.cpp | `dd56bb0048104a66b126cf5ea474f2ea694d2b90c3c884661004e2d1966f2510` |
| dev/src/preprocess/expression.cpp | `268d074167952b63048d7bb55e34691a97efa1ec163a5e27a0540a7e38f808dd` |
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
| dev/src/posttoken/number.cpp | `8c685ca4d654579de8149ad5594793a275c79f74da695d6b9af95cb222d5ca26` |
| dev/src/posttoken/output.cpp | `6a9cab69e45c7f3a7b5c8ba682f21dc93a97cb558c4802ddf71b6b38c7d3ba54` |
| dev/src/posttoken/token_types.cpp | `5b02b37b204bd63501c309d8ceb61d4e9455ff3b9ee7909add9cd4099cb1aef0` |
| dev/src/posttoken/cursor.h | `58ff4ff585b6ba5dc37d77bc3a77b461f5e4c8879c703154289716e5ae528d98` |
| dev/src/posttoken/literal.h | `a71cb923f8e011af34b8a1f32c7db5339e15fed9ca4656d1125dc21b69ea113e` |
| dev/src/posttoken/number.h | `d07b3b18bc3cbf04ce7c1612bb1ed58a1d60e98f5c5331c973b93708aefce2a5` |
| dev/src/posttoken/output.h | `8e43cc9eb6f9e4736bc4b34ad37f3f957ffa99a717c1ab84b8a5c19ef3d22ae9` |
| dev/src/posttoken/token.h | `3d09ef949cae144030b60c50bef5a944624037a4942f6ba634d189a1ca64ec33` |
| dev/src/posttoken/token_types.h | `871b468fd872ed348f791f462a7c9ac080b9a70afe29763514dc3faa1d2a7dd9` |
| dev/src/support/testing/test_runner.cpp | `d33cbae9c8265d8e128650871a43961e20063cd1b3c386ee20dfa0f23c50ae61` |
| dev/Makefile | `be5823c83f053b5b7df9bc4f369d716ca2e3410a05cd9ae28e6851d2bfceefda` |
| dev/frontend_source_sets.mk | `fe08ce2ea6d4a290cda654696faed697fcd1864687775b42997f8e84497f5045` |

Calibrated confirmations across campaigns: {'floating-rejection-4': 3, 'suffix-rejection-200000': 2}.
Each affected workload has at least two campaigns with both ABBA gains beyond A/A noise.
Inconclusive campaigns remain in the evidence and are not used alone to claim a gain.


Total: 324 retained timed observations. Compiler outputs and final source/binary
provenance agree. Generated-program runtime/text size are N/A; frontend changes
do not establish generated-code optimization profitability.
