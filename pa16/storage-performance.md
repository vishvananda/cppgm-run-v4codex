# PA16 storage handoff performance

Implementation: `e85d39a9` (storage group `4aab464e`, counters `66f123ab`,
string-reference correction `e85d39a9`). The frozen entry is `cc842756`.
The [harness](../student.tests/pa16/storage_benchmark.py) uses fixed binaries,
inputs and flags (`--emit-lowir -O0`), one permitted CPU, one warmup per binary,
four A/A observations and two ABBA blocks. Newly correct local-static programs
have six B observations after warmup; the entry executable fails its result
check, so it is not used for a performance comparison. Compilation and execution
are timed separately. Volatile trip counts, runtime indices, calls, memory and
floating operations plus checked results prevent a dead or constant workload.

Both complete campaigns remain, with **560 timing observations including warmups**:
[initial](../student.tests/pa16/storage-performance-initial.json) and
[final](../student.tests/pa16/storage-performance.json). The initial binary preceded
the string-reference correction. Each record retains hashes, all observations,
wall time, peak RSS, user/system CPU time, context switches, telemetry, A/A range,
paired ratios and executable results. The original 896 scalar observations remain
in [the historical record](performance.md). No measurement has been discarded.

## Compiler latency and memory

Final campaign medians / maximum observed RSS. Ratios are the two paired B/A
blocks, including the outlier discussed below. All source workloads other than
arrays have identical common LowIR; array differences are checked by native
sharing/copy controls. Sources at 1,000/4,000 functions dominate the roughly 6 ms
process startup floor; tiny executable-source compilation is retained as noise
information, not the basis for a frontend speed claim.

| Source | A ms / KiB | B ms / KiB | Paired B/A |
|---|---:|---:|---|
| template, 1,000 | 21.52 / 7,488 | 21.58 / 7,504 | 1.003, 0.969 |
| memory-float, 1,000 | 105.84 / 21,980 | 102.85 / 21,708 | 0.997, 0.942 |
| array-sharing, 1,000 | 111.88 / 22,584 | 101.93 / 21,256 | 0.935, 0.869 |
| array-copy, 1,000 | 64.78 / 15,056 | 61.15 / 14,204 | 0.956, 0.954 |
| static, 1,000 | entry incorrect | 59.75 / 14,340 | B only |
| template, 4,000 | 70.47 / 13,156 | 69.95 / 13,144 | 1.006, 0.998 |
| memory-float, 4,000 | 401.70 / 70,644 | 404.75 / 70,664 | 1.374, 0.993 |
| array-sharing, 4,000 | 418.88 / 73,008 | 383.19 / 64,120 | 0.905, 0.942 |
| array-copy, 4,000 | 244.39 / 44,700 | 228.88 / 40,136 | 0.899, 0.913 |
| static, 4,000 | entry incorrect | 223.35 / 40,776 | B only |

The 4,000-function memory/floating outlier is **0.7213 s elapsed / 0.39 s
user+system CPU**; the other B samples are 0.4019–0.4075 s. The first campaign's
paired ratios were 1.062 and 1.010; final ratios are 1.374 and 0.993. Its peak RSS
increases only 20 KiB and LowIR is identical. A focused repeat is recorded below;
the large outlier is retained, not silently excluded. The static-only 4,000 case
also has a 0.4651 s observation versus a 0.2233 s median (0.26 s CPU for that
outlier). Small-source startup noise includes a 1.202 floating compiler ratio;
the generated floating executable is identical and runtime remains stable.

Array sharing lowers final 4,000-function peak RSS by 8,888 KiB; the large-copy
workload lowers it by 4,564 KiB. Both campaigns' paired compiler ratios improve
for these affected workloads. Compiler `.text` grows **10,368 bytes / 0.67%**,
from 1,544,838 to 1,555,206 bytes for the new semantic/storage functionality.

## Executable runtime and size

The supplied backend writes sectionless ELF files. Size below is **code plus
alignment padding / typed global data bytes**: executable payload after entry,
minus the LowIR global byte count. Full payload and file size are also retained.
This explicitly accounts for readonly data growth instead of mislabeling the
whole executable payload as `.text`. No host backend is part of implementation;
measurement uses the supplied LowIR native backend, bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.

| Native workload | A/B median seconds | A code/data bytes | B code/data bytes | Paired runtime B/A |
|---|---:|---:|---:|---|
| calls | 0.3604 / 0.3617 | 206 / 0 | 206 / 0 | 0.999, 0.989 |
| memory | 0.2133 / 0.2144 | 434 / 0 | 434 / 0 | 1.003, 0.999 |
| floating | 0.2497 / 0.2499 | 230 / 0 | 230 / 0 | 1.002, 0.998 |
| sharing | 0.2070 / 0.2100 | 349 / 14 | 349 / 7 | 0.987, 1.006 |
| copy | 0.2502 / 0.2262 | 298 / 0 | 276 / 160 | 0.916, 0.905 |
| static | — / 0.1017 | entry incorrect | 312 / 16 | B only |

Common calls, memory and floating executables are byte-identical. Their runtime
ratios vary within the observed noise. Sharing preserves code size and halves
this input's backing data from 14 to 7 bytes; no runtime speedup is claimed.
For the 160-byte array, the required copy is 7–10% faster across all four ABBA
pairs in the two campaigns. Code/padding falls by 22 bytes, while readonly data
adds 160 bytes; total payload grows by 138 bytes. This is a disclosed storage
tradeoff of PA16's mandated copy form, not a claim that every array length runs
faster. The backend's bulk-copy policy remains PA24/PA33 work.

## Ownership, work bounds and acceptance

There is no optional optimizer or profitability search. Each local static owns
one global and, when needed, one guard and one destructor callback. Classification
is memoized per declaration and per (initializer plan, local context). The local
reference callback index visits only reference-owned temporaries. Each action is
lowered once; inherited array expansion remains bounded by its eight-element
unrolling policy with a loop fallback.

Readonly data interning owns one image per complete typed key, plus at most one
tentative candidate while hashing/comparing. The key includes size, alignment,
data-item kinds/types/payloads, symbol IDs and addends; x87 padding is excluded.
Hash/equality work follows emitted items; omitted zero ranges remain compact.
The unique image and flat indexes release with the lowering TU. Each automatic
array still owns its distinct stack slot and one object copy.

At 1,000/4,000 functions, sharing performs **27,993/111,993** data-item visits,
with one image and 1,999/7,999 cache hits. The large-copy workload performs
**7,996/31,996** visits and 1,000/4,000 plan classifications; each omitted range
is one typed zero item. Static workloads perform exactly **1,000/4,000**
classification transitions and create the same number of local-static owners.
Final B compiler median scaling is 3.24x template, 3.94x memory/floating, 3.76x
sharing, 3.74x copy and 3.74x statics for 4x input. Measurements and work counters
support linear/near-linear ownership without whole-program retry or search.

PA16/O0 has no mandated numeric latency/RSS/text ceiling. The recorded +15%
common compiler latency, +16 MiB RSS and 5.5x scaling targets are diagnostic
investigation thresholds, not accumulated PA14/PA15 exit gates. The required
512-call / 1,000,000-step constexpr limits remain. The inherited 32-byte copy
heuristic is reclassified: it cannot override PA16's explicit constexpr-array
copy requirement. Its historical evidence is preserved. Extending the copy rule
to ordinary non-constexpr arrays remains an explicit contract issue in
[plan.md](plan.md): the trial broke 16 earlier required store comparisons; neither
coverage nor comparison rules was weakened. Later ELF, native optimization and
self-hosting costs retain their stage owners.

## Focused noise repeat

[Repeat harness](../student.tests/pa16/storage_noise.py) and
[all 726 invocations](../student.tests/pa16/storage-noise.json) freeze the final
campaign's binaries, flags and inputs. After warmups and four A/A batches it runs
four ABBA blocks. The tiny floating source is batched 32 times per observation.
Every output hash is checked; all invocation RSS/CPU/context-switch data remain.

Memory/floating 4,000-function paired ratios are **0.998, 1.200, 1.005, 1.015**;
A/B medians are **401.03/404.54 ms**, peak RSS **70,676/70,732 KiB**. The remaining
outlier is 0.5532 s elapsed with 0.39 s CPU. Thus the measurements do not show a
repeatable 20–37% compiler computation cost; scheduling/I/O delay remains visible,
and no general speedup is claimed. Batched tiny-source ratios are **1.012, 0.994,
0.986, 0.986**; medians **177.58/177.50 ms** for 32 processes. The A/A calibration
retains a 0.4042 s batch. These observations resolve the diagnostic concern
without dropping outliers or adding a new performance exit gate.

The two full storage campaigns plus this repeat preserve **1,286 observations**.
The [handoff verifier](../student.tests/pa16/verify_storage.py) checks hashes,
coverage/progress, all sample sequences, ownership counters, and the historical
scalar artifacts. The required compiler changes are accepted for PA16/O0: no
persistent avoidable regression is evidenced, the changed array cases improve
compiler cost and one affected runtime, and all code/data growth is disclosed.
