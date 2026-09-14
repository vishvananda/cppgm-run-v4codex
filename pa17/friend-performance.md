# PA17 friend-entity performance — loop 50

Entry A: `d3ef5e5d5adc89baf13f9697e5c38b165bba0dcc`. Final B: `a8cc241e67cec028d8e56cd65fe600876968c9ca`.
Frozen binaries, complete source inputs, flags, output hashes, untimed telemetry
and every timing observation are in [the record](../student.tests/pa17/friend-performance.json).
Reproduce with `student.tests/pa17/friend_benchmark.py A B WORK OUT`.

The campaign pins CPU 31 and measures PA17 `--emit-lowir -O0`, using the
same `g++ -std=gnu++11 -Wall -O3` build and test runner. Each common input has
one warmup per binary, four A/A observations and four ABBA blocks. New-only
inputs record the entry rejection, one warmup and six B observations. Timed
compilation excludes `--stats` and `--validate-lowir`; untimed preflight includes
both. Native execution uses the pinned course backend at O0, separately from
compilation. No build or test campaign overlapped these measurements.

All common LowIR and executable hashes match exactly. Every executable checks
its result; volatile loop bounds and live arithmetic, calls, memory and floating
point work prevent dead workloads. Runtime sources compile in roughly 6 ms,
so those compiler measurements are retained as startup-sensitive diagnostics.
Self-hosting and our own native backend remain later-stage responsibilities.

| Common compiler workload | A / B median ms | A / B peak KiB | Paired B/A median (range) | A/A range ms |
|---|---:|---:|---:|---:|
| common-partials-1500 | 111.85 / 112.59 | 22576 / 22544 | 1.0046 (0.9947–1.0136) | 113.07–115.59 |
| common-loop-float-1500 | 152.27 / 150.77 | 29940 / 29852 | 0.9913 (0.9835–0.9938) | 150.36–284.14 |
| common-partials-6000 | 453.22 / 454.67 | 73044 / 72940 | 1.0040 (0.9928–1.0056) | 447.47–1023.64 |
| common-loop-float-6000 | 596.21 / 594.28 | 104220 / 104160 | 0.9337 (0.8508–0.9973) | 595.80–604.93 |
| three-head-demand-1500 | 459.36 / 459.18 | 77824 / 77652 | 1.0025 (0.9911–1.0134) | 452.29–494.57 |
| three-head-demand-6000 | 1958.43 / 1984.65 | 294204 / 293928 | 1.0087 (1.0060–1.0149) | 1965.69–2123.19 |
| ordinary-friend-demand-1500 | 156.01 / 156.70 | 29488 / 29800 | 0.9983 (0.8537–1.0072) | 154.03–162.07 |
| ordinary-friend-demand-6000 | 641.67 / 642.15 | 101424 / 102168 | 1.0008 (0.9830–1.0330) | 639.43–641.93 |

Common paired medians span −6.63% to +0.87%. The −6.63% loop/float result
contains slow A observations: its individual medians are 596.21 / 594.28 ms.
All samples remain in the record; no optimization benefit is inferred from
these cost observations. Common peak RSS changes range from −276 to +744 KiB.
The 6000 three-head input shows a +0.87% paired slowdown (all blocks +0.60%
to +1.49%), with 276 KiB less peak RSS and identical output. This small cost
is consistent with the added friend dispatch/access checks on declaration
paths; the campaign does not isolate a causal attribution. The implementation
adds no optional pass, global retry or duplicate candidate conversion, and
avoids allocating the new candidate index for single-declaration calls.

| Newly supported workload | B median ms (range) | Peak KiB |
|---|---:|---:|
| friend-access-demand-1500 | 298.91 (295.58–300.51) | 50952 |
| friend-access-demand-6000 | 1260.32 (1253.26–1267.73) | 187296 |
| friend-runtime | 6.12 (5.99–6.31) | 5832 |

| Native workload | A / B median ms | Text bytes A / B | Paired B/A median (range) | A/A range ms |
|---|---:|---:|---:|---:|
| runtime-calls | 359.55 / 359.70 | 206 / 206 | 0.9984 (0.9971–1.0005) | 360.26–363.70 |
| runtime-memory | 210.84 / 210.72 | 434 / 434 | 0.9957 (0.9830–1.0036) | 210.72–214.35 |
| runtime-floating | 249.85 / 249.61 | 230 / 230 | 0.9982 (0.9970–0.9994) | 249.08–249.20 |
| friend-runtime | rejected / 366.57 | — / 236 | final only | n/a |

Native text is the sectionless executable payload after ELF entry; these
runtime sources have no static data. Native peak RSS is 256 KiB in all samples.
No native code-growth transform or optional optimization was introduced.

| Scaling input | Type substitution work | Candidate work | Friend definitions / processed |
|---|---:|---:|---:|
| friend-access-demand-1500 | 19508 | 3000 | 0 / 0 |
| ordinary-friend-demand-1500 | 6000 | 1500 | 1500 / 1500 |
| friend-access-demand-6000 | 78008 | 12000 | 0 / 0 |
| ordinary-friend-demand-6000 | 24000 | 6000 | 6000 / 6000 |

The 4× friend-access workload scales to 4.22× latency and 3.68× peak RSS.
Ordinary-friend definitions are processed once per demanded entity. Their
queues, source head slices and friendship edges are TU-owned; body demand
uses entity indexes and separate monotonic function states. Lookup walks only
lexical/associated edges. Candidate deduplication has a call-local identity
index, allocated only for multi-declaration candidate sets, before conversions.
These observations support the stated bounds on these inputs; they do not
certify all whole-stage architecture requirements.

Compiler `.text` grows from 1,740,550 to 1,745,478 bytes
(+4,928, 0.28%) for required semantics. There is no optional transform
whose profit needs to justify extra work or code growth. PA17/O0 has no mandated
numerical ceiling. Existing evaluator limits and lowering growth policies are
unchanged. Inherited +15%, +16 MiB and 5.5× targets remain diagnostic under the
stage-scoped spec; no correctness requirement or coverage is waived.

Prior [head measurements](head-performance.md) and [audit measurements](audit-performance.md)
remain unchanged, including their historical diagnostic and superseded runs.
