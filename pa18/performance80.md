# PA18 performance evidence 80

Acceptance is **PA18/O0 LowIR**, spec §9. These changes implement required
scalar/reference semantics and the course O0 representation. There is no new
optimization pass. Historical +15%, +16 MiB and 5.5× diagnostics remain non-gates;
the stage specifies correctness and bounded work, with no numerical compiler/RSS
ceiling. Earlier observations remain preserved in [performance79.md](performance79.md).

Frozen A is entry `c5e2c271`; B contains `ce7d3e7f`. The
[raw campaign](../student.tests/pa18/loop80-performance.json) retains all source
text/hashes, binary hashes, flags, affinity/platform, telemetry, warmups and samples.
The harness as run is committed in `f660fae4`. Build: `g++ -std=gnu++11 -Wall -O3`,
course test runner enabled. Timed flags: `--emit-lowir -O0`; an untimed preflight
adds `--validate-lowir --stats`. Each equivalent workload gets one warmup per
binary, four A/A noise observations, then four ABBA blocks. Five new-behavior
workloads get a warmup and six B-only observations; rejected A compilations are
not timed as equivalent work. Validation finished before measurement began.

The supplied O0 native backend is invoked only by the explicit benchmark harness;
construction is outside compiler and runtime timings. Bundle:
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`. Every native preflight and timed run
checks its result. Volatile bounds and data-dependent loops/calls/memory/floating
work prevent timing a constant-folded or dead workload. Self-hosting and native
optimization remain later-stage requirements.

## Compiler latency and peak RSS

Times are median milliseconds. Ratios are medians of within-ABBA B/A means,
with the full paired range. RSS is maximum observed KiB; no outliers are removed.

| Workload | A / B ms | Paired B/A (range) | A / B RSS KiB |
|---|---:|---:|---:|
| ordering-600 | 55.03 / 54.89 | 0.993 (0.985–1.006) | 15356 / 15296 |
| ordering-2400 | 213.38 / 212.21 | 0.964 (0.854–0.996) | 43924 / 43960 |
| member-head-600 | 97.94 / 98.34 | 1.006 (1.003–1.014) | 23016 / 22972 |
| member-head-2400 | 395.35 / 396.22 | 0.999 (0.996–1.004) | 73708 / 73648 |
| common-loop-float-1500 | 154.51 / 152.96 | 0.993 (0.980–1.137) | 30364 / 30436 |
| runtime-calls | 5.69 / 5.76 | 1.010 (0.998–1.039) | 5968 / 6028 |
| runtime-memory | 9.60 / 9.14 | 0.992 (0.957–1.029) | 5892 / 6040 |
| runtime-floating | 9.46 / 9.38 | 0.982 (0.854–1.103) | 6080 / 6184 |
| pointer-temporaries-600 | 54.41 / 53.97 | 0.988 (0.945–1.019) | 10120 / 9612 |
| cast-reference-600 | — / 134.77 | new behavior | — / 16524 |
| constant-reference-600 | — / 156.12 | new behavior | — / 16168 |
| pointer-temporaries-2400 | 220.79 / 216.38 | 0.942 (0.883–0.976) | 23048 / 21764 |
| cast-reference-2400 | — / 497.50 | new behavior | — / 47956 |
| constant-reference-2400 | — / 584.03 | new behavior | — / 47816 |
| runtime-pointer-temporary | 11.32 / 11.05 | 0.949 (0.911–1.032) | 5884 / 5996 |
| runtime-cast-reference | — / 9.95 | new behavior | — / 6056 |


Startup medians are 5.42 / 5.39 ms; tiny source compilation times are startup-limited.
The five large unchanged frontend comparisons have paired ratios 0.964–1.006.
Member-head-600 adds about 0.4 ms at its median (+0.6% paired); larger member-head
work remains approximately even. The full ranges disclose scheduling noise,
including a 1.137 paired block for common-loop-float despite a 0.993 median.
Unchanged large-workload peak RSS increases by at most 72 KiB.

Pointer-temporary compiler work drops by one LowIR copy per materialization.
At 2400 functions, median compilation is 220.79 → 216.38 ms and peak RSS
23048 → 21764 KiB. Its paired ratio is 0.942, with substantial noise (0.883–0.976).
This is required typed-value output; no general optimization speedup is claimed.
New cast and constexpr-reference workloads have no correct A implementation,
so their B-only costs describe required semantics rather than a speed comparison.

## Executable runtime and size

Times are median milliseconds, with full paired ranges. Compiler size is actual
`.text`; supplied sectionless native sizes are executable payload proxies, not
section measurements. All ten equivalent sources with an entry point produce **identical native
bytes**, including the three pointer-temporary sources with different LowIR.
Eight equivalent sources also have identical LowIR. Scaled cases do have
untimed native preflights when `main` is present; the inherited raw metadata
sentence saying they do not emit an executable is inaccurate. Their native
hashes, checked exits and sizes are retained in the actual output records.

| Workload | A / B ms | Paired B/A (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 1590.25 / 1499.26 | 0.959 (0.820–1.028) | 206 / 206 |
| runtime-memory | 655.02 / 633.29 | 0.977 (0.930–1.129) | 434 / 434 |
| runtime-floating | 808.87 / 816.77 | 1.063 (0.991–1.247) | 230 / 230 |
| runtime-pointer-temporary | 181.69 / 180.34 | 0.997 (0.989–1.014) | 239 / 239 |
| runtime-cast-reference | — / 120.16 | B only (118.06–156.73 ms) | — / 215 |

Runtime variation is visibly noisy: floating has a median paired regression
despite byte-identical executable output, and calls/memory show apparent gains
with equally identical output. These are environmental observations, not changes
in generated work. All A/A and ABBA samples remain in the raw evidence; no runtime
benefit is claimed. The affected pointer-temporary executable is identical too.
The new runtime cast-reference source checks converted values at each call.

Compiler `.text`: **1,958,534 → 1,960,070 bytes**, **+1,536 (0.078%)**. There is
no executable growth on equivalent inputs and no optional transform requiring
an unproven runtime-profit exception.

## Work bounds and acceptance

The per-use budget is one existing conversion sequence and, when the language
requires it, one scalar temporary. Reference constant evaluation clears the
reference/temporary flags before its scalar step, so recursion adds at most one
step. Selected user-conversion results consume their recorded second sequence
and the requesting consumer's target. Query results retain the existing complete
canonical keys and memoization. All added storage is TU-owned constant storage
or function-local LowIR slots; there is no new cache or global invalidation.

At 600 → 2400 source declarations:

- Pointer-temporary conversions: **2417 → 9617**; LowIR instructions **3627 → 14427**.
- Cast-reference substitutions: **3603 → 14403**; queries **614 → 2414**; instructions **8413 → 33613**.
- Constant-reference queries: **3013 → 12013**; constant-address work **1800 → 7200**; conversions **6002 → 24002**. Only the one-instruction entry function is emitted.

These counters and the measured scaling follow source/demanded conversion facts.
Native/data sizes, IR pool sizes and all telemetry are retained in the raw
records and [handoff evidence](../student.tests/pa18/loop80-evidence.json).
The required correctness repairs, unchanged coverage, bounded work, negligible
compiler-text growth and unchanged equivalent executable work satisfy this
group's PA18/O0 acceptance. The 25 remaining course failures still prevent
whole-stage acceptance.
