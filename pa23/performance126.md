# PA23/O0 final performance audit 126

The final production binary is unchanged from implementation125. Its SHA-256
matches both `dev/cppgm++` and the frozen B binary. This audit independently
reran all 25 fixed workloads using the unchanged
[benchmark125.py](../student.tests/pa23/benchmark125.py) and
[benchmark124_forest.py](../student.tests/pa23/benchmark124_forest.py).
[Main observations](../student.tests/pa23/performance126.json) and
[forest observations](../student.tests/pa23/performance126-forest.json) retain
all sources, flags, binary/backend/harness hashes, checked exits, warmups,
A/A observations, ABBA samples, paired ratios, ranges, telemetry and text hashes.

Frozen binaries: `/tmp/pa23-125/{entry,final}-cppgm++`:

- A (checkpoint124): `24688bccd2cc1b6926654b33d9aec6a8f92ddcdce419ef1f898fa2aca2022106`.
- B (`d9179e84`): `979a332fdeb3133af19b09d93bfbf4cb061236b59573e682b2c4a747fda5acaa`.

The repository HEAD recorded by the harness is evidence commit `83ba7c58`;
its `dev` tree is the B implementation. Compiler build flags remain
`g++ -std=gnu++11 -Wall -O3`; source flags are `--emit-lowir -O0`.
Each lane warms once, then records four A/A observations and four wall-time
ABBA blocks on a pinned CPU. RSS comes from `/usr/bin/time`. Compilation and
generated-program execution are separate measurements. No correctness suite,
build or other benchmark run overlapped these measurements. The supplied
backend consumes our LowIR; the host links its objects. Stats/validation
observations are separate from ordinary latency/RSS and preserve output hashes.

All **19** common inputs are correct in both lanes, with **identical LowIR and
native .text**. Six B-only inputs measure newly required semantics; using the
incomplete A implementation as their performance baseline would be invalid.
Volatile iteration counts and checked results keep calls, loops, memory access,
floating point, virtual dispatch, crosscasts and member-pointer work live.
Template-heavy and deep/wide inheritance inputs exceed compiler startup time.
Small compiler inputs and non-loop executable timings remain marked as startup
sensitive. There is no new runtime speedup claim.

## Fresh measurements

Times are median milliseconds; RSS is peak KiB; text is bytes. Ratios are the
median of paired B/A block ratios with their full range, not the ratio of two
unpaired medians. A single number denotes a B-only semantic workload.

| Workload | Compile A → B ms | RSS A → B | Compile B/A [range] | Runtime A → B ms | Runtime B/A [range] | Text A / B |
|---|---:|---:|---:|---:|---:|---:|
| auto-specializations-9600 | 921.78 → 963.25 | 108148 → 108192 | 1.067 [0.848–1.102] | 5.41 → 5.47 | 1.003 [0.959–1.287] | 768286 / 768286 |
| runtime-calls | 7.06 → 7.30 | 6208 → 6196 | 1.033 [0.992–1.156] | 150.31 → 161.67 | 1.060 [0.976–1.111] | 430 / 430 |
| runtime-memory | 9.20 → 9.28 | 6144 → 6160 | 1.003 [0.998–1.007] | 108.17 → 107.52 | 0.985 [0.973–1.015] | 658 / 658 |
| runtime-floating | 9.23 → 9.26 | 6424 → 6444 | 0.992 [0.961–1.027] | 99.87 → 99.56 | 0.998 [0.962–1.044] | 454 / 454 |
| runtime-member | 7.23 → 7.12 | 6180 → 6196 | 0.977 [0.916–1.034] | 230.69 → 226.40 | 1.003 [0.926–1.054] | 488 / 488 |
| member-functions-2048 | 160.36 → 161.55 | 27720 → 27736 | 1.028 [0.977–1.087] | 4.58 → 4.54 | 0.959 [0.932–1.002] | 184632 / 184632 |
| runtime-virtual-single | 7.65 → 7.71 | 6148 → 6332 | 0.994 [0.877–1.073] | 89.85 → 78.00 | 0.966 [0.765–1.061] | 570 / 570 |
| runtime-nonpoly-single | 8.38 → 8.49 | 6180 → 6164 | 1.015 [0.976–1.056] | 70.02 → 73.28 | 1.025 [0.968–1.102] | 434 / 434 |
| runtime-this-downcast | 7.05 → 7.03 | 6212 → 6196 | 0.997 [0.969–1.037] | 172.58 → 172.80 | 0.997 [0.806–1.055] | 396 / 396 |
| runtime-secondary | 8.00 → 8.20 | 6292 → 6376 | 1.104 [0.948–1.466] | 273.39 → 272.17 | 1.014 [0.989–1.211] | 632 / 632 |
| runtime-crosscast | 7.25 → 7.41 | 6212 → 6404 | 1.007 [0.965–1.022] | 682.61 → 677.76 | 1.013 [0.987–1.137] | 634 / 634 |
| base-paths-256 | 32.99 → 33.06 | 8708 → 8760 | 1.009 [1.003–1.011] | 5.13 → 5.18 | 1.008 [0.996–1.018] | 4900 / 4900 |
| base-paths-1024 | 153.89 → 155.63 | 15888 → 15912 | 1.075 [1.014–1.078] | 5.63 → 5.56 | 0.980 [0.945–1.012] | 18724 / 18724 |
| base-paths-4096 | 968.16 → 987.33 | 45508 → 45876 | 1.026 [1.000–1.165] | 3.99 → 4.02 | 1.003 [0.989–1.008] | 74020 / 74020 |
| shared-depth-4 | 6.62 → 6.62 | 6212 → 6376 | 1.010 [0.988–1.024] | 4.08 → 4.01 | 0.987 [0.976–1.020] | 280 / 280 |
| shared-depth-8 | 6.89 → 6.97 | 6180 → 6200 | 1.016 [0.980–1.023] | 3.98 → 3.93 | 0.989 [0.977–0.997] | 280 / 280 |
| shared-depth-12 | 7.08 → 7.10 | 6180 → 6376 | 1.004 [0.995–1.019] | 4.17 → 4.20 | 1.016 [0.990–1.034] | 280 / 280 |
| shared-depth-16 | 7.76 → 7.92 | 6436 → 6632 | 1.014 [1.002–1.035] | 4.01 → 4.07 | 0.997 [0.985–1.038] | 280 / 280 |
| new-runtime-member-pointer | 7.25 | 6360 | — | 128.89 | — | 1014 |
| new-runtime-lifecycle | 7.60 | 6352 | — | 41.93 | — | 1739 |
| new-runtime-value-abi | 6.95 | 6368 | — | 77.06 | — | 547 |
| new-construct-depth-4 | 8.22 | 6652 | — | 5.40 | — | 1910 |
| new-construct-depth-8 | 13.32 | 7676 | — | 5.47 | — | 5744 |
| new-construct-depth-12 | 25.79 | 10460 | — | 5.19 | — | 12801 |
| shared-forest-512-depth8 | 271.88 → 278.60 | 54392 → 57912 | 1.033 [1.020–1.049] | 4.03 → 4.07 | 1.006 [0.965–1.007] | 16632 / 16632 |

## Noise, cost and stage acceptance

This run is noisier than handoff125. Template compiler A/A spans **820.67–940.23
ms**; its A/B ranges are **749.35–992.99 / 711.12–1060.21 ms**, paired ratios
**1.085, 0.848, 1.102, 1.048**. No template improvement or repeatable regression
is established: this input has zero virtual views/slots, and prior paired
ratios were **0.888–1.070**. The 1024-path A/A range is **302.73–536.10 ms**,
versus subsequent A/B medians **153.89/155.63 ms**; the transient slowdown is
retained, not removed to improve the result. The 4096-path A/A range is
**970.05–1131.60 ms**, A/B ranges **954.90–1075.71 / 965.53–1412.62 ms**;
its four ratios are **1.165, 1.017, 1.036, 1.000**. The 1412.62 ms B observation
and the depth-4 A/A outlier **64.73 ms** remain in the data.

The forest repeats the small implementation125 cost: **271.88→278.60 ms**,
paired ratios **1.049, 1.043, 1.020, 1.024**, peak RSS **54,392→57,912 KiB**.
A/A spans **268.03–269.39 ms**, A/B ranges **260.94–338.30 / 270.90–323.66 ms**.
The added **3,520 KiB** accompanies **74,241** lifecycle entries (including the
sentinel) in **3,145,728** capacity bytes. These records provide the required
physical base/VTT ownership; rooted VTT output requires the complete layout
facts even when a test uses only a view. Views (**82,944**), instructions
(**6,657**) and output bytes (**371,560**) are unchanged. The required cost is
disclosed; there is no optional transform whose profit could justify or hide it.
The 4096-path input adds **368 KiB** RSS and retains exactly **4096** memoized
base-layout computations. Template RSS rises **44 KiB** in this run.

Compiler text is **2,333,190→2,368,262 bytes**, **+35,072 (1.50%)**, unchanged
from handoff125. All common executable text is identical. Timing increases on
calls/crosscasts coexist with decreases on memory/virtual dispatch; all of those
paired runtime ranges straddle one. They do not establish changed executable
work or runtime profit. The tiny shared-depth-8 executable range is slightly
below one, but is startup-sensitive and has identical text; no gain is claimed.

Required new behavior remains separately measured: member-pointer runtime
**128.89 ms**, lifecycle **41.93 ms**, value-ABI **77.06 ms**. Constructing shared
graphs at depths 4/8/12 takes **8.22/13.32/25.79 ms** to compile and
**6652/7676/10460 KiB** peak RSS. The deepest input retains **338** views,
**314** lifecycle entries, **2241** instructions, **771,958** LowIR bytes and
**3,526,920** bytes of IR capacity, with **12,801** native text bytes. These are
required distinct layout/table facts, not an optional speculative optimization.
The program constructs and dispatches on the object and checks the result.

Acceptance follows **spec §9 at PA23/O0**. Historical **+15% compiler latency**,
**+16 MiB RSS** and **5.5× scaling** thresholds were self-selected diagnostics,
not handout/spec limits; [audit124's rationale](performance124.md) remains valid.
No correctness, fixture comparison, coverage or mandated bound is weakened.
Historical misses do not permanently fail a corrected implementation. The
measured semantic costs do not justify avoidable repeated work: source review
and counters confirm one dependency publication, one physical view, memoized
path layout and bounded emission. The [final audit](audit.md) records complete
keys, invalidation, fallback, per-policy and aggregate work/growth bounds.

## Preserved whole-stage evidence

[Performance121](performance121.md) retains the non-null `this` proof's changed
executable work, **428→396** text bytes and repeated paired runtime benefit;
B still emits **396** bytes. [Performance122](performance122.md) and
[performance123](performance123.md) retain the necessary lifecycle shape and
table-based access costs, including **+146** and **+20/+62** native text bytes.
[Performance124](performance124.md) retains the repeatable removal of duplicate
shared views: forest compilation **566.00→257.99 ms**, RSS
**208,624→54,400 KiB**, all four ABBA blocks improving with unchanged native
text. [Performance125](performance125.md) preserves the first full-stage
measurements; the current run neither overwrites nor cherry-picks them.

PA23 does not own native selection/allocation, later debug locations, practical
host ABI integration or self-hosting. Those benchmark/implementation obligations
remain with PA24–PA34. The supplied standalone RTTI limitation is independently
reproduced and does not remove any source behavior or comparison requirement.
There is no remaining demonstrated avoidable performance regression or
unsupported self-imposed performance exit gate for PA23.
