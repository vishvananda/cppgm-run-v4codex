# PA16 result and member-pointer performance — implementation 43

The [producer](../student.tests/pa16/result_benchmark.py) and
[raw evidence](../student.tests/pa16/result-performance.json) retain **302**
observations, including warmups. A is entry `c9c4ddb7`; B is `048014e8`.
Binaries, source text/hashes, `--emit-lowir -O0` flags, backend bundle/hash, CPU
binding, LowIR/native hashes, wall/CPU times, RSS, context switches and telemetry
are recorded. Both binaries were frozen before measurement. No build or test
process ran concurrently with these timings.

Each common workload has a warmup per binary, four A/A observations, then two
ABBA blocks. A focused compiler repeat uses four ABBA blocks. Cases rejected by
A have one B warmup and six B observations. Correct native outcomes are checked
before timing; every timed invocation must also exit zero. Common LowIR and
executables are byte-identical. The affected result and member-array workloads
check constructor/destructor counts or runtime-dependent member reads/calls.
Volatile trip counts retain executable work. The supplied native backend is
measurement infrastructure; PA16 still owns source-to-LowIR implementation.

## Compiler latency and peak RSS

Medians in milliseconds; RSS is maximum observed KiB. Small runtime-source
compilations are approximately the 6 ms process floor and are not the basis of
compiler speed claims. All their samples remain in the evidence.

| Workload | A ms / KiB | B ms / KiB | Paired B/A |
| --- | --- | --- | --- |
| Template, 1,000 | 23.19 / 7,452 | 23.15 / 7,668 | 1.000, 0.995 |
| Template, 4,000 | 74.60 / 13,168 | 74.93 / 13,324 | 0.998, 1.016 |
| Memory/float, 1,000 | 105.00 / 21,696 | 104.52 / 21,796 | 1.011, 0.997 |
| Memory/float, 4,000 | 410.65 / 70,512 | 409.96 / 70,484 | 1.006, 0.998 |
| Final results, 1,000 | 33.50 / 9,868 | 29.71 / 9,336 | 0.896, 0.926 |
| Final results, 4,000, initial | 115.08 / 23,532 | 104.73 / 20,616 | 1.472, 0.888 |
| Final results, 4,000, repeat | 115.28 / 23,552 | 102.79 / 20,772 | 0.901, 0.799, 0.902, 0.984 |
| Member constants, 1,000 | rejected | 35.96 / 9,484 | final only |
| Member constants, 4,000 | rejected | 127.10 / 21,140 | final only |
| Template member storage, 1,000 | rejected | 104.59 / 20,728 | final only |
| Template member storage, 4,000 | rejected | 414.72 / 65,444 | final only |

The initial 4,000-result B outlier is 234.9 ms wall with 90 ms reported CPU;
other B samples are 102.1–106.5 ms. A/A is 114.1–116.6 ms. The repeat retains
its own outliers, including A at 139.2 ms and B at 123.7 ms. All four repeat
pairs improve; the 0.799 ratio includes the A outlier and is not a general 20%
speed claim. Common large-source timings stay near entry.

For 1,000/4,000 final-result functions, full-expression analysis work is unchanged
at 6,007/24,007 visits. EH regions fall from 2,000/8,000 to 1,000/4,000;
instructions fall from 13,012/52,012 to 8,012/32,012. At 4,000, IR capacity falls
from 8,372,592 to 4,571,504 bytes. The result identity adds constant state per
lowering owner, without a new analysis pass or repeated statement scans.

The new template-storage workload scales 3.97x in latency for 4x input. It checks
1,001/4,001 bodies (one shared member body plus the consumers), makes exactly
1,000/4,000 class completions and definition applications, and retains **four**
constant-object work units and **two** member demands at both sizes. Initializer
classification is 3,000/12,000 actions, and output is 10,002/40,002 instructions.
There is no repeated whole-program function search or eager unrelated body use.

## Executable runtime and size

The backend emits sectionless ELF. Size below is code plus alignment / typed
global data bytes; full payload and file sizes are also preserved. Native peak
RSS reports the measurement tool's 256 KiB floor for these programs.

| Runtime | A / B ms | A code/data | B code/data | Paired B/A |
| --- | --- | --- | --- | --- |
| Calls | 359.08 / 359.29 | 206 / 0 | 206 / 0 | 0.954, 1.002 |
| Memory | 210.85 / 211.06 | 434 / 0 | 434 / 0 | 1.004, 0.998 |
| Floating | 252.42 / 250.73 | 230 / 0 | 230 / 0 | 0.997, 0.985 |
| Final results, 4 million | 56.23 / 32.73 | 1,072 / 8 | 944 / 8 | 0.584, 0.587 |
| Final results, 24 million | 319.68 / 192.01 | 1,072 / 8 | 944 / 8 | 0.612, 0.604 |
| Member arrays, 24 million | 2,622.20 / 685.47 | 553 / 0 | 440 / 352 | 0.260, 0.261 |

The longer result loop confirms the short loop's benefit with runtime well above
startup. Member arrays exchange repeated per-call initialization for the PA16
required readonly images and copies: code shrinks 113 bytes while data grows
352 bytes. That bounded representation cost accompanies a repeatable runtime
benefit on this affected workload. Compiler `.text` grows from **1,641,990** to
**1,652,870** bytes: **10,880 bytes / 0.66%** for the combined semantic/lowering
changes. No broad compiler or executable speedup is claimed.

## Acceptance and retained observations

PA16/O0 has no mandated numerical latency/RSS/text ceiling. Existing constexpr
limits (512 calls, 1,000,000 execution steps), correctness and full comparisons
remain mandatory. Work is bounded by evaluated operations, initializer edges and
emitted data; each automatic declaration gets one destination/copy, each demanded
storage declaration one relocation walk, and each selected function its existing
deduplicated demand state. There is no speculative code growth or new optimizer.
The affected runtime benefits justify the small compiler-text growth; no common
avoidable regression is evidenced. Historical diagnostic thresholds in earlier
plans do not add exit gates; all earlier evidence remains linked from the plan.

The first producer incorrectly assumed the ordinary member-array runtime source
would reject at entry. Its compile succeeded; both executables subsequently
validated and returned the checked result, so this workload correctly uses A/B.
The campaign resumed only unfinished/additional cases. The raw record retains
the failed assumption, original producer source and continuation producer versions,
all original observations, the noise repeat and the longer loop. No sample was
discarded or replaced. Self-hosting/native compiler performance remains owned by
later assignments.
