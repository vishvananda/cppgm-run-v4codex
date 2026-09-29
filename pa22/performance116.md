# PA22/O0 performance evidence — implementation 116

This group repairs inherited conversion lookup, class-value conversion ranking
and omitted aggregate elements' zero-initialization. It adds no optional
optimization. Mandatory semantic work is measured without comparing incorrect
execution against corrected execution as a speed result.

## Protocol and artifacts

[The harness](../student.tests/pa22/benchmark116.py) freezes sources, hashes,
flags, compiler/backend identities, host version, telemetry and all observations.
Both campaigns use CPU 0, one warmup per lane, four A/A observations and four
ABBA blocks. Compiler and executable wall time/peak RSS are separate. Flags are
`--emit-lowir -O0`; `--stats --validate-lowir` produces identical LowIR to plain
compilation. The supplied native backend consumes student-generated LowIR;
g++ links the resulting object. Every executable's expected exit is checked.
No build or course report overlaps these campaigns. Volatile iteration counts
and checked sums keep the runtime workloads live. Small mains and small compiler
inputs are startup-sensitive; they do not establish speed improvements.

Frozen binaries in `/tmp/pa22-116/`:

| Lane | Source | SHA-256 | Compiler `.text` bytes |
|---|---|---|---:|
| A: `entry` | `7b3685fc` | `66f8ed6d5d9cd6cae9660303d15e4f280414a7e63b20fc17e505848d8c0895f8` | 2262278 |
| B: `final` | production through `7b6d62b9` | `7d6e3b2880b9da9524957977af405d916d82e37e2b058aef7e581f387f1bbbae` | 2263302 |

Compiler text grows **1,024 bytes (0.045%)**. All eleven inputs produce
byte-identical LowIR in A/B, checked equivalent execution and equal native text
sizes. Unsupported entry cases remain correctness controls, not A/B workloads.

## Common correct subset

The six unchanged inputs cover template-heavy work, calls, memory loops,
floating point and member-pointer calls. The large function-count input also
checks compiler growth. Native size includes the same hosted ELF startup code.
RSS below is the maximum among the timed ABBA observations; every sample,
warmup, A/A observation, range and paired ratio is retained in the
[common campaign](../student.tests/pa22/performance116-common.json).

| Workload | Compiler median ms A/B | Peak RSS KiB A/B | Runtime median ms A/B | Native `.text` A/B |
|---|---:|---:|---:|---:|
| 9,600 template specializations | 734.41 / 703.83 | 108536 / 108768 | 5.71 / 5.71 | 768286 / 768286 |
| Calls | 6.79 / 6.52 | 6140 / 6244 | 200.63 / 190.97 | 430 / 430 |
| Memory loop | 8.97 / 8.58 | 6152 / 6288 | 109.14 / 99.87 | 658 / 658 |
| Floating loop | 6.83 / 6.44 | 6336 / 6416 | 91.33 / 89.15 | 454 / 454 |
| Member-call loop | 8.51 / 8.22 | 6140 / 6356 | 210.07 / 209.97 | 488 / 488 |
| 2,048 member-call functions | 150.82 / 149.92 | 27972 / 28224 | 3.81 / 3.75 | 184632 / 184632 |

| Workload | Paired compiler B/A range | Paired runtime B/A range |
|---|---:|---:|
| Templates | 0.710–1.345 | 0.959–1.037 |
| Calls | 0.656–1.900 | 0.802–1.039 |
| Memory | 0.943–0.981 | 0.821–1.058 |
| Floating | 0.936–0.966 | 0.953–1.020 |
| Member loop | 0.955–0.965 | 0.994–1.004 |
| Member functions | 0.992–0.999 | 0.953–1.014 |

Template compilation A/A spans 694.93–1077.95 ms; calls compilation A/A spans
6.59–21.61 ms. Member-loop runtime A/A spans 209.15–349.59 ms. These outliers and
the identical executable work preclude an optimization claim from lower medians.
The maximum common-subset RSS increase is 252 KiB. No latency gain is claimed.

## Conversion width and count

These inputs have a valid first-base conversion in both compilers. Other bases
are empty, so A's incomplete collector happens to be correct on these specific
programs. B still performs the required sibling search. The comparison measures
its cost on semantically equivalent outputs; it does not justify first-base-only
lookup for general programs. All observations and exact input contents are in
the [conversion campaign](../student.tests/pa22/performance116-conversions.json).

| Bases / functions | Compiler median ms A/B | Peak RSS KiB A/B | Runtime median ms A/B | Native `.text` A/B |
|---|---:|---:|---:|---:|
| 8 / 512 | 26.13 / 26.48 | 9236 / 9448 | 3.95 / 3.82 | 6447 / 6447 |
| 8 / 2,048 | 83.15 / 83.49 | 19404 / 19452 | 3.76 / 3.73 | 24879 / 24879 |
| 32 / 512 | 26.18 / 26.10 | 9404 / 9424 | 3.94 / 3.87 | 6447 / 6447 |
| 32 / 2,048 | 84.22 / 86.26 | 19384 / 19384 | 3.88 / 3.99 | 24879 / 24879 |
| 40-million-call loop | 6.75 / 6.59 | 6164 / 6352 | 175.11 / 175.19 | 393 / 393 |

Compiler paired B/A ranges are respectively 0.996–1.016, 1.008–1.015,
0.996–1.009, 1.002–1.257 and 0.495–0.994. The widest/largest case adds 2.04 ms
(2.4%) at the median and no peak RSS. Its compiler A/A range is 82.52–84.04 ms;
one ABBA block has a larger outlier, preserved in the raw data. The first input's
A/A range is 26.18–365.44 ms. No general compiler-speed conclusion is supported.
The runtime loop's paired ratios are 0.693, 0.680, 1.019, 0.998, with A/A
177.89–179.79 ms: the equal medians and identical generated work are the useful
observations, not a claimed improvement from the two baseline outliers.

The existing `semantic_lookup_work` now counts conversion scope visits. A/B
counts are 1,568/6,176; 6,176/24,608; 1,616/18,512; 6,224/73,808. Each difference
is exactly `(bases + source class) * functions`. Width and call count therefore
account for the new work; it does not scan unrelated declarations. Candidate
selection/demand and generated outputs remain unchanged on these controls.

## Bounds, correctness costs and acceptance

Candidate collection visits inheritance paths and their conversion declarations,
restoring hiding on path exit and deduplicating emitted entity IDs. Flat indexes
and geometrically grown traversal scratch have per-call lifetimes. The existing
ranking hierarchy query and selected conversion facts supply semantic decisions;
lowering does no new lookup. No optional fixed point, specialization, inlining
or code-growth transform is added.

The zero flag occupies existing padding: `InitAction` is **56 bytes in both
lanes**, as recorded in [layout116.json](../student.tests/pa22/layout116.json).
Zero plans reuse canonical type ownership; their actions are consumed directly
without per-element semantic work. Existing eight-element expansion limits and
loop fallback remain intact. The repaired reference-cast fixture gains exactly
its missing `zeroinit 4x4`; skipping this required initialization is not an
equivalent baseline. Prefilled placement storage controls exercise nested
objects, a 12-element array, user-provided and late-defaulted constructors, and
an implicit constructor with a nontrivial base. Entry fails; final execution
passes. This necessary semantic cost is distinct from optimization profitability.

PA22/O0 has no mandated numeric wall-time/RSS gate. Under spec §9, the bounded
semantic costs above satisfy current-stage acceptance without claiming a runtime
benefit. Historical +15%, +16 MiB and 5.5× diagnostic targets remain non-gating;
their earlier observations are preserved in 114/115's evidence. Correctness,
coverage, comparison rules and existing work/growth bounds are unchanged.
Native selection/allocation and self-hosting benchmarks remain later-stage
responsibilities. No later-stage limit is used to excuse a current defect.
