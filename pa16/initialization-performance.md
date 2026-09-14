# PA16 initializer storage evidence — implementation 42

The [initial campaign](../student.tests/pa16/initialization-performance-initial.json) retains
**364 observations**, including warmups. Its [producer](../student.tests/pa16/initialization_benchmark.py)
records binaries, source/LowIR/native hashes, flags, CPU affinity, all wall times,
peak RSS, CPU times, context switches, telemetry, A/A ranges and paired spread.
A is turn entry `fe4dbd88`; B is the implementation committed as `9e967c33`.
B's snapshot was taken before that commit; the record contains its exact source
diff and binary hash. Both use `--emit-lowir -O0`. Each workload has one warmup
per binary, four A/A observations, then two wall-time ABBA blocks. Compilation
and native execution are measured separately, with no concurrent build/test load.

All executable results are checked. Common LowIR/native outputs are byte-identical;
affected arrays are compared by checked native outcomes and the distinct-storage,
zero-fill, conversion and pointer-addend controls. Volatile trip counts and runtime
indices keep the loops, calls, memory and floating workloads live. The supplied
native backend is measurement infrastructure, not compiler implementation; its
pinned bundle and hash are in the record. Self-hosting remains a later-stage owner.

## Compiler latency and memory

Medians of paired samples in milliseconds; RSS is maximum observed KiB. The
roughly 6 ms process floor is separated from 1,000/4,000-function workloads.

| Source | A ms / KiB | B ms / KiB | Paired B/A |
| --- | --- | --- | --- |
| template 1,000 | 22.58 / 7,652 | 22.36 / 7,508 | 0.998, 0.984 |
| memory/float 1,000 | 104.48 / 21,772 | 104.42 / 21,804 | 0.999, 0.996 |
| array sharing 1,000 | 111.45 / 22,468 | 98.96 / 19,536 | 0.888, 1.014 |
| array copy 1,000 | 59.54 / 14,356 | 58.41 / 14,048 | 0.988, 0.981 |
| local statics 1,000 | 58.79 / 14,172 | 58.89 / 14,180 | 0.999, 1.005 |
| template 4,000 | 72.47 / 13,208 | 71.39 / 13,208 | 0.980, 0.989 |
| memory/float 4,000 | 406.43 / 70,784 | 409.69 / 70,440 | 1.005, 1.008 |
| array sharing 4,000 | 439.99 / 74,164 | 378.51 / 63,968 | 0.838, 0.784 |
| array copy 4,000 | 226.09 / 40,376 | 227.45 / 40,492 | 1.015, 0.997 |
| local statics 4,000 | 226.16 / 41,052 | 226.77 / 40,988 | 0.998, 1.005 |

Common large-source timing remains near entry. The array-sharing workload saves
10,196 KiB at 4,000 functions and improves both large paired latency ratios.
The small sharing pair includes an outlier; all observations remain in the raw
record. No overall compiler speedup is claimed. Compiler `.text` is unchanged
at **1,634,950 bytes**; binary hashes differ.

## Executable runtime and size

Medians in milliseconds. The backend emits sectionless ELF, so size is **code
plus alignment / typed global data bytes**, not a mislabeled `.text` section.
File bytes and complete executable payload are also retained. These arrays
require readonly data even where immediates would occupy fewer data bytes.

| Runtime | A / B ms | A code/data | B code/data | Paired B/A |
| --- | --- | --- | --- | --- |
| calls | 359.50 / 359.83 | 206 / 0 | 206 / 0 | 1.005, 0.999 |
| memory | 210.87 / 211.08 | 434 / 0 | 434 / 0 | 0.998, 1.007 |
| floating | 249.87 / 249.93 | 230 / 0 | 230 / 0 | 0.999, 0.997 |
| two string arrays | 234.92 / 207.18 | 337 / 0 | 349 / 7 | 0.886, 0.881 |
| 40-element array | 287.38 / 249.52 | 298 / 0 | 276 / 160 | 0.892, 0.862 |
| local static scalar | 102.16 / 102.47 | 312 / 16 | 312 / 16 | 1.049, 1.004 |
| 2-element array | 121.93 / 116.17 | 223 / 0 | 232 / 8 | 0.953, 0.953 |
| 8-element array | 129.05 / 128.31 | 265 / 0 | 244 / 32 | 0.993, 0.997 |

The two string arrays, 40-element array and 2-element array improve in both
paired blocks. The 8-element result is near noise. Common executables are
byte-identical; their spread is timing noise, including the local-static outlier.
The observed benefit does not claim that every scalar type/bound will be faster.
Native selection and optimization policy remain PA24/PA33 work.

## Ownership, limits and acceptance

This is the PA16 mandated storage rule, not an optional optimizer. Work is bounded
by one classification per typed initializer plan, one emitted image per distinct
complete typed key, one distinct slot and one copy per automatic declaration.
The interner's size/alignment, data kinds, values, symbol IDs and addends are
unchanged; transient candidate data is discarded on a hit. No unrolling or new
search is introduced. Code/data growth is bounded by the representation of unique
initializer images plus one copy per object; no speculative cloning is allowed.

For 1,000/4,000 sharing functions, semantic classification performs **2,000/8,000**
actions; interning performs **27,993/111,993** item visits with **1,999/7,999** hits
and one image. The 40-element workload performs **5,000/20,000** classifications
and **7,996/31,996** data visits, retaining one image. Omitted zero ranges remain
sparse. B median compiler scaling for 4x input is 3.19x template, 3.92x
memory/float, 3.82x sharing, 3.89x copy and 3.85x statics.

There is no evidenced avoidable common regression and no optional transform to
remove. The current-stage spec does not impose a numeric latency/RSS/text cap;
older percentage/scaling targets remain diagnostics, not permanent exit gates.
The mandated 512-call/1,000,000-step constant-evaluation bounds, correctness,
coverage and complete comparisons remain intact. All historical measurements
remain in [storage](storage-performance.md), [objects](object-performance.md)
and [audit](audit-performance.md); this report supplements rather than replaces them.

The removed local-reference startup queue also has a correctness-only comparison:
a two-TU reducer gives duplicate singleton `init` roles at entry, rejected by the
backend; B validates and executes in either source order. Comparing timings of
that invalid entry program would not be a valid performance claim.

## Final lifecycle implementation

The [final campaign](../student.tests/pa16/initialization-performance.json) freezes
`fc309df7` and retains another **364** observations under the same A/A and ABBA
protocol. All B LowIR and executable outputs in this corpus are identical to
those of the initial B: program coordination preserves the one-TU form. Compiler
text is now **1,641,990 bytes**, a **7,040-byte / 0.43%** increase over entry for
program lifecycle ownership and its typed scheduling support.

Final compiler medians / peak RSS and paired ratios:

| Source | A ms / KiB | B ms / KiB | Paired B/A |
| --- | --- | --- | --- |
| template 1,000 | 23.06 / 7,520 | 23.05 / 7,508 | 0.991, 1.004 |
| memory/float 1,000 | 103.28 / 21,772 | 103.59 / 21,880 | 1.003, 1.321 |
| array sharing 1,000 | 113.78 / 22,556 | 98.71 / 20,168 | 0.870, 0.860 |
| array copy 1,000 | 61.40 / 14,260 | 60.20 / 14,024 | 0.978, 0.990 |
| local statics 1,000 | 61.05 / 14,552 | 60.83 / 14,512 | 0.991, 1.001 |
| template 4,000 | 73.08 / 13,008 | 73.24 / 12,968 | 1.014, 0.997 |
| memory/float 4,000 | 407.27 / 70,528 | 408.89 / 70,684 | 1.009, 0.990 |
| array sharing 4,000 | 439.59 / 73,896 | 370.12 / 63,964 | 0.839, 0.528 |
| array copy 4,000 | 225.86 / 40,124 | 229.65 / 40,576 | 1.018, 1.014 |
| local statics 4,000 | 233.26 / 44,976 | 231.12 / 44,988 | 0.997, 1.001 |

The final 4,000-array copy case costs about 1.7% compiler time and 452 KiB peak
RSS over entry for the required plan classification/interner path; the executable
benefit below accompanies that bounded semantic cost. Sharing saves 9,932 KiB.
The 0.528 sharing ratio includes an **A** stall (0.9845 s wall / 0.43 s CPU),
so it is not evidence of a 47% computation speedup. The memory/float outlier is
0.1696 s wall / 0.10 s CPU against other samples near 0.103 s; its repeat follows.

Final runtime medians/spread, with code/data sizes identical to the initial table:

| Runtime | A / B ms | Paired B/A |
| --- | --- | --- |
| calls | 359.82 / 359.24 | 0.997, 1.001 |
| memory | 210.79 / 210.73 | 0.999, 0.999 |
| floating | 251.85 / 253.96 | 0.985, 1.010 |
| two string arrays | 236.10 / 208.40 | 0.876, 0.896 |
| 40-element array | 252.83 / 228.48 | 0.899, 0.924 |
| local static scalar | 102.60 / 102.60 | 1.000, 0.955 |
| 2-element array | 119.11 / 115.42 | 0.963, 0.971 |
| 8-element array | 131.83 / 130.95 | 1.004, 0.987 |

The [focused repeat](../student.tests/pa16/initialization-noise.json), produced by
[this harness](../student.tests/pa16/initialization_noise.py), retains **726**
invocations. After warmups and four A/A batches it uses four ABBA blocks, batching
the tiny 8-element compiler input 32 times. Final binaries, flags and input/output
hashes remain fixed. Memory/float-1,000 ratios are **0.983, 1.005, 1.006, 0.998**,
with A/B medians **103.64/103.46 ms** and peak RSS **21,836/21,880 KiB**.
The 32-invocation tiny batches have ratios **1.026, 1.373, 0.998, 0.980**,
medians **194.91/199.04 ms** and peak RSS **5,772/5,824 KiB**. The remaining
stall is retained. The original tiny-input samples also stalled on both A and B
(0.1896/0.3726 s with process CPU rounded to zero); neither campaign supports a
repeatable 37–94% compiler computation regression. No tiny-source speed claim
or new numeric exit gate is introduced.

## Multiple-TU ownership and scaling

The [lifecycle producer](../student.tests/pa16/lifecycle_benchmark.py) and
[raw evidence](../student.tests/pa16/lifecycle-performance.json) add **28**
observations (one B warmup and six B samples for each compiler/runtime phase).
Both entry programs fail typed validation with duplicate singleton roles, so only
the correct final binary is timed. Runtime uses a volatile 60-million-iteration
loop reading an initialized external object and checks all TU initializers' values.

| Initialized TUs | Compiler median / peak RSS | Runtime median | Code+alignment / data | IR instructions |
| --- | --- | --- | --- | --- |
| 64 | 12.31 ms / 6,116 KiB | 281.38 ms | 3,940 / 256 bytes | 615 |
| 256 | 29.59 ms / 7,324 KiB | 282.98 ms | 15,076 / 1,024 bytes | 2,343 |

The emitted initializer-unit counters are exactly 64/256. Coordinator work is
one call per participating TU, at most two coordinator functions, and O(TUs)
retained IDs. IR has exactly nine additional instructions per additional TU;
IR pool capacity grows from 169,184 to 676,064 bytes. Frontend/lowering telemetry
is retained separately. Finalization uses the reversed unit sequence and is
validated by both-order native constructor/destructor LIFO controls. Program
facts retain function IDs only after each semantic TU is released.

Across the initial, final, noise and lifecycle campaigns, **1,482 observations**
are preserved. The implementation meets current-stage performance acceptance:
required semantic costs and growth are bounded and disclosed; affected array
benefits repeat, common output is unchanged, and the noise investigation does not
show a persistent avoidable common regression. The result/ABI and member-pointer
implementation gaps remain in the plan; performance evidence does not waive them.
