# PA17 partial-owner definition performance evidence

Entry: `ae5087da8794d159b16b716e3c48cf64a271414f`; implementation: `3bef03e042b2c474c7d793719169279d23c898a5`.
[Raw observations](../student.tests/pa17/definition-performance.json) retain all
binary/input hashes, flags, source text, phase/work telemetry, output hashes,
warmups, A/A samples, ABBA samples and timing/RSS ranges. The
[harness](../student.tests/pa17/definition_benchmark.py) freezes both compiler
binaries, pins one CPU and checks all subprocess results. Common inputs use one
warmup each, four A/A observations, then four ABBA blocks. Newly supported
partial-owner inputs use one warmup and six final-binary observations; the entry
binary must reject them. Compilation and native execution are timed separately.

| Compiler workload | A / B median ms | A / B peak KiB | Paired B/A median (range) |
|---|---:|---:|---:|
| common-partials-1500 | 109.26 / 109.71 | 22,232 / 22,516 | 1.0031 (0.9888–1.0176) |
| common-loop-float-1500 | 150.12 / 152.15 | 30,320 / 30,288 | 1.0151 (1.0091–1.2751) |
| entity-pack-alias-1500 | 144.78 / 140.57 | 26,504 / 26,436 | 0.9730 (0.6100–0.9918) |
| common-partials-6000 | 447.25 / 449.89 | 72,628 / 71,608 | 0.9982 (0.9888–1.0180) |
| common-loop-float-6000 | 594.21 / 594.67 | 103,964 / 104,048 | 1.0010 (0.8648–1.0039) |
| entity-pack-alias-6000 | 566.18 / 563.17 | 89,640 / 89,976 | 0.9994 (0.9803–1.0089) |
| member-qualified-1500 | 287.39 / 288.89 | 49,764 / 49,896 | 1.0019 (0.9907–1.0136) |
| common-qualified-access-1500 | 68.67 / 69.39 | 15,156 / 15,248 | 1.0163 (0.9856–1.0199) |
| member-qualified-6000 | 1182.66 / 1186.85 | 179,756 / 180,780 | 1.0063 (0.9983–1.0303) |
| common-qualified-access-6000 | 269.17 / 274.64 | 43,848 / 44,572 | 1.0115 (0.9236–1.0587) |
| primary-definition-1500 | 258.48 / 256.42 | 42,424 / 42,480 | 0.9971 (0.9529–1.0050) |
| partial-definition-1500 | rejected / 195.92 | — / 35,920 | not comparable |
| primary-definition-6000 | 1067.64 / 1068.38 | 152,476 / 153,012 | 1.0013 (0.9989–1.0086) |
| partial-definition-6000 | rejected / 827.53 | — / 126,860 | not comparable |
| partial-runtime | rejected / 6.41 | — / 5,864 | not comparable |
| runtime-calls | 5.67 / 5.67 | 5,712 / 5,712 | 1.0141 (0.9890–1.0326) |
| runtime-memory | 5.81 / 5.86 | 5,736 / 5,712 | 1.0054 (0.9970–1.0182) |
| runtime-floating | 5.45 / 5.51 | 5,740 / 5,840 | 1.0047 (0.9972–1.0149) |

| Native workload | A / B median ms | Text bytes A / B | Paired B/A median |
|---|---:|---:|---:|
| partial-runtime | — / 362.53 | — / 206 | not comparable |
| runtime-calls | 360.88 / 360.03 | 206 / 206 | 0.9989 |
| runtime-memory | 211.36 / 211.54 | 434 / 434 | 0.9995 |
| runtime-floating | 250.02 / 249.57 | 230 / 230 | 0.9977 |

All common LowIR outputs and common native executables are byte-identical.
Runtime loops use volatile bounds and checked results; native RSS is 256 KiB.
The reference native backend only executes this compiler’s emitted LowIR.
Native size measures sectionless executable payload after ELF entry for these
inputs without static data. Compiler `.text` is **1,717,190 → 1,718,406 bytes**
(+1,216 bytes, **0.071%**).

Common compiler paired medians span **0.9730–1.0163**; the changed primary
definition path is **0.9971 / 1.0013** at 1,500 / 6,000 owners. No speedup is
claimed. The largest common RSS increase is 1,024 KiB (member-qualified-6000).
Necessary new storage is the selected-argument tuple ID in substitution frames;
there is no optional transform or retained syntax copy. The table and raw
observations disclose all measured differences, including noise and outliers.

New partial-owner workloads take **195.92 / 827.53 ms**, **35,920 / 126,860 KiB**
at 1,500 / 6,000 instantiations: **4.224× time**, **3.532× RSS** for 4× input.
They retain one owner definition and demand its distinct specializations.
The additional partial member-call runtime executes checked work in **362.53 ms**
with **206 bytes** of executable text; it has no correct entry-binary comparator.
The ordinary source benchmarks take 69–1,187 ms and native loops 211–363 ms,
well above process startup. Tiny runtime-source compilations (5–6 ms) remain
startup-sensitive and support no compilation-performance claim.

PA17/O0 mandates correctness and bounded compiler work but no numerical
latency/RSS/text ceiling. There is no optional optimization to justify through
runtime profit. Retained owner shapes use canonical indexed equality, source
normalization follows actual head/argument nodes, and selected-tuple frames keep
ordinal lookup constant-time within each head. The measured behavior is consistent
with that bounded work; scaling alone does not prove the architecture.
Inherited percentage/scaling diagnostics remain diagnostics under spec.md’s
stage-scoped acceptance. Historical measurements, all required coverage and
mandated limits remain preserved. Native optimization/MIR and self-hosting
remain later-stage owners.
