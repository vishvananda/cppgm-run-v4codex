# PA34 frozen performance evidence

A is the canonical GCC-built seed; B is the canonical self compiler. Their
frozen hashes are recorded in every final dataset and [compiler-images.json](compiler-images.json).
B and inception match byte for byte. Both compiler builds use the default O3;
benchmark flags below are identical across A/B. The seed uses the system
allocator and the self compiler has the unchanged PA34 jemalloc linkage.
Consequently this measures canonical generations, not an isolated optimization.

`measure.py` reuses the fixed PA32 compiler-source/common harnesses. CPU 2,
A/A calibration (four A runs), then six ABBA blocks, with no concurrent builds
or tests. Compilation and generated execution are timed separately. All 476
observations, flags, binary/input hashes, RSS, paired ratios and A/A ranges are
preserved in [selfhost.json](selfhost.json), [common-o0.json](common-o0.json)
and [common-o2.json](common-o2.json). [Commands](commands.json) record the
invocations and environment. No outlier was discarded.

The common inputs demand 2,400 template specializations and execute checked,
argc-dependent memory/call loops, floating arithmetic and exception/destructor
work. Pruning adds 1,200 unused functions to the memory input. Measured runtimes
of 40–390 ms are longer than process startup. The compiler component input is
unchanged `dev/src/lowir/folding.cpp`; executable runtime is N/A for that object.

## Compiler latency and peak RSS

Seconds and KiB are A/B. Ratios are the median of the six paired block ratios,
with their full range; these need not equal the ratio of independent medians.
Full per-configuration spreads and A/A noise remain in the linked datasets.

| Workload | Compile seconds A/B | Peak RSS KiB A/B | Paired B/A [range] |
| --- | ---: | ---: | ---: |
| Compiler source O0 | 2.8973 / 4.6372 | 77,864 / 87,916 | 1.811 [1.546–2.011] |
| memory O0 | 0.2715 / 0.6443 | 30,232 / 40,416 | 2.366 [2.272–2.632] |
| floating O0 | 0.2914 / 0.5393 | 30,920 / 40,864 | 2.293 [1.758–2.363] |
| exceptions O0 | 0.2225 / 0.4752 | 30,312 / 40,464 | 2.200 [1.635–2.943] |
| pruning O0 | 0.2201 / 0.5455 | 36,276 / 45,932 | 2.445 [2.103–2.898] |
| memory O2 | 0.3464 / 0.7967 | 30,716 / 41,772 | 2.448 [2.195–2.584] |
| floating O2 | 0.2212 / 0.5618 | 30,936 / 42,224 | 2.546 [2.221–2.563] |
| exceptions O2 | 0.2293 / 0.5677 | 30,376 / 42,108 | 2.273 [1.748–2.564] |
| pruning O2 | 0.3882 / 0.9558 | 36,436 / 46,212 | 2.423 [2.009–2.987] |

The compiler-source A/A range is 3.528–4.190 s; later A observations span
1.220–4.232 s. This substantial environmental spread is retained, not hidden by
an absolute-time claim. Common compilation is consistently slower in the self
compiler. Untimed [compiler-source work](selfhost-work.json), all 224 common
compile observations and the [O0–O3 declaration/template trace](trace.json)
have equal nontiming work counters. Every generated object is identical.
This identifies machine-code/allocator quality differences without evidence of
repeated semantic work or miscompilation. Timing and RSS counters are excluded
from work equality; allocations, demand, IR and backend work remain included.

## Generated execution and text

Common A/B executables are also byte-identical, with identical checked results.
Thus observed runtime differences are measurement noise, not code-generation
improvements. `.text` bytes are the sum of actual text sections, not `size`'s
aggregate of text plus other read-only sections. Compiler-source object text is
34,466 bytes in both generations.

| Workload | Runtime seconds A/B | Peak RSS KiB A/B | Paired B/A [range] | Executable text bytes A=B |
| --- | ---: | ---: | ---: | ---: |
| memory O0 | 0.0530 / 0.0531 | 1,760 / 1,760 | 0.982 [0.900–1.172] | 151,633 |
| floating O0 | 0.0589 / 0.0592 | 1,756 / 1,760 | 1.016 [0.996–1.028] | 151,474 |
| exceptions O0 | 0.2518 / 0.2540 | 3,936 / 3,936 | 1.002 [0.971–1.033] | 151,781 |
| pruning O0 | 0.0778 / 0.0784 | 1,760 / 1,760 | 0.992 [0.647–1.319] | 151,633 |
| memory O2 | 0.0490 / 0.0494 | 1,760 / 1,760 | 1.002 [0.987–1.024] | 125,179 |
| floating O2 | 0.0409 / 0.0411 | 1,756 / 1,756 | 1.007 [0.996–1.010] | 125,086 |
| exceptions O2 | 0.3895 / 0.3849 | 3,908 / 3,908 | 0.948 [0.853–1.067] | 125,268 |
| pruning O2 | 0.0664 / 0.0672 | 1,760 / 1,760 | 1.000 [0.964–1.030] | 125,179 |

## Stack diagnosis and acceptance

[The PA10 proof](../../pa10/deep_calls.md) traces the self-only stack exhaustion
to large expression-dispatch frames. [Baseline observations](stack-baseline.json)
and [before/after dispatcher observations](stack-dispatch.json) preserve all
56 measurements, including phase counters and the broad original noise spread.
Only those diagnostics use a 64 MiB stack, to compare correct old/new executions;
all canonical checks and the 400-call reducer retain the original 8 MiB stack.
Raw stderr, frozen binaries, failing cases and original object/IR evidence remain
in the artifact archive bound by [binding.json](binding.json).

The original compiler source produces identical object bytes and work before
and after the dispatcher correction. Paired compiler latency is 1.006
[0.881–1.317], peak RSS falls 4,268 KiB and generated `.text` is unchanged.
The diagnostic self compiler grows 704 `.text` bytes (0.008%); its aggregate
text/read-only size grows 1,592 bytes (0.017%). This repairs resource correctness
with no repeatable general speedup claim. No optimization transform was added.

Under spec §9, inherited ad hoc 2x compile, 1.75x RSS, zero-growth and runtime
ratio targets remain diagnostics as in PA33. They are unsupported as universal
PA34 exit gates, particularly across different code generators and allocator
linkage. The measured self/seed code-quality gap is disclosed; it does not
justify an unrelated optimization or relaxing correctness. Mandated canonical
900/3600-second per-source compile limits, 8 GiB RSS, exact comparisons and all
existing per-level optimization work/growth budgets are unchanged and pass.

Reproduce with `python3 student.tests/pa34/measure.py OUT FROZEN_SEED FROZEN_SELF`
and `python3 student.tests/pa34/trace.py OUT SEED SELF`. Supplemental untimed
folding commands are in `selfhost-work.json`; diagnostic stack commands and
explicit stack metadata are in the two stack datasets. Full canonical build
commands/log hashes and every compared object are recorded in `binding.json`.
