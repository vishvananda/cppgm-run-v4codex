# PA18 loop 72 performance evidence

Frozen entry `ca314a72` and implementation `57ee1f2e`, built with the same
`g++ -std=gnu++11 -Wall -O3` configuration and test runner enabled.
[Harness](../student.tests/pa18/benchmark72.py) and
[observations](../student.tests/pa18/loop72-performance.json) retain binary,
input and output hashes, flags, CPU affinity, warmups, wall time, RSS and work
counters. Neither binary changed during measurement. One warmup each, four A/A
samples, and four ABBA blocks compare equivalent correct outputs; new behavior
has six final-only samples. Validation and telemetry run outside timed commands.

Compiler timings include PA18's required O0 LowIR output. The executable
measurements separately use the supplied native backend, pinned to bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`. Volatile runtime inputs, loops and
checked results prevent measuring a folded/dead workload. Native optimization
and self-hosting remain later-stage responsibilities.

## Compiler latency and peak RSS

| Workload | A / B median ms | Paired B/A median (range) | A / B peak RSS KiB |
|---|---:|---:|---:|
| ordering-600 | 55.80 / 56.11 | 1.004 (0.996–1.012) | 15184 / 15308 |
| ordering-2400 | 212.61 / 213.45 | 1.004 (1.000–1.011) | 43612 / 43552 |
| member-head-600 | 98.63 / 100.36 | 1.010 (0.997–1.019) | 22872 / 22812 |
| member-head-2400 | 397.19 / 397.29 | 1.001 (0.996–1.052) | 72988 / 72956 |
| common-loop-float-1500 | 155.23 / 154.36 | 1.001 (0.983–1.038) | 30392 / 30232 |
| runtime-calls | 6.02 / 6.03 | 1.004 (0.985–1.013) | 5904 / 5832 |
| runtime-memory | 6.22 / 6.15 | 1.006 (0.989–1.015) | 5888 / 5848 |
| runtime-floating | 5.82 / 5.87 | 1.005 (0.992–1.014) | 6008 / 6032 |
| alias-formation-600 | 70.70 / 72.47 | 1.025 (1.016–1.044) | 17244 / 17720 |
| pack-capture-600 | — / 186.72 | new behavior | — / 34364 |
| alias-formation-2400 | 281.33 / 290.53 | 1.029 (0.956–1.038) | 52216 / 52976 |
| pack-capture-2400 | — / 784.68 | new behavior | — / 119660 |
| runtime-new-pack | — / 7.05 | new behavior | — / 5912 |

Empty-input startup medians are **5.67 / 5.81 ms**; small runtime-source compile
times are startup-limited. The larger frontend cases dominate startup. Every
common workload has byte-identical LowIR and native output where applicable.

Alias formation adds about **2.5–2.9%** paired median compiler time, with peak RSS
increases of **476 / 760 KiB** at 600 / 2400 declarations. This is required
argument validity and transparent result handling, not an optional transform.
The canonical facts compute once per consumed key. Other common frontend
results are near parity; no compiler speedup is claimed. All outliers remain:
the 2400-alias paired range includes 0.956, while three blocks show increases.
A/A observations and the first attempt's measurements are retained in full.

Compiler `.text`: **1,871,494 → 1,884,742 bytes**, **+13,248 bytes (0.708%)**.
Generated equivalent code has no size growth.

## Checked executable runtime and size

| Workload | A / B median seconds | Paired B/A median (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 0.720586 / 0.715948 | 0.992 (0.978–0.997) | 206 / 206 |
| runtime-memory | 0.417217 / 0.416825 | 0.999 (0.997–1.002) | 434 / 434 |
| runtime-floating | 0.495554 / 0.495828 | 1.000 (0.998–1.002) | 230 / 230 |
| runtime-new-pack | — / 0.170587 | new behavior | — / 251 |

All runs return zero. The backend emits sectionless ELF: payload is the code-size
proxy for these sources without static data, not a claimed `.text` section.
The common executable hashes are identical, so the measured runtime variation
is environmental. No runtime optimization benefit or regression is attributed
to this frontend change. The new pack workload has no correct entry executable.

## Graph work and stage acceptance

| Family, 600 → 2400 | Type substitution work | Frames | Expansion lanes | Signature shapes |
|---|---:|---:|---:|---:|
| alias-formation | 4208 → 16808 | 1805 → 7205 | 0 → 0 | 4 → 4 |
| pack-capture | 19206 → 76806 | 19801 → 79201 | 6000 → 24000 | 5 → 5 |

Fourfold source growth produces approximately fourfold semantic work. Final
compiler time scales **4.01×** for alias formation and **4.20×** for correlated
packs. Pack discovery work is 604 → 2404; shared declaration shape work stays
constant. Frame traversal follows the actual lexical/capture dependency edges;
there are no unrelated declaration scans or global retries. The separate
32/128/512-class controls retain exactly one completion invalidation.

Acceptance is **PA18/O0 LowIR**, spec §9. No numerical compiler/runtime ceiling
or optional optimization was introduced. Correctness, coverage and graph-work
bounds remain gates. Historical +15%, +16 MiB and 5.5× targets remain diagnostics,
with all inherited measurements preserved; they are not extra stage exit gates.

## Preserved initial attempt

[Initial observations](../student.tests/pa18/loop72-performance-attempt1.json)
contain the first nine completed workloads. The run then stopped during the
new pack workload's validation because its fallback passed a class value through
an ellipsis and emitted invalid variadic LowIR. The original harness and failing
source are frozen under `/tmp/pa18-loop72`, with hashes in the evidence manifest.
The same binaries were used for both runs.

The final workload still varies the enclosing class specialization and exercises
both valid and invalid pack lengths, using scalar call arguments. This isolates
formation work from the separate ordinary class-varargs lowering defect. The
[reducer](../student.tests/pa18/class_ellipsis_pending.cpp) and failed validation
are preserved as unfinished implementation. No course coverage, reference or
comparison rule was changed, and no observation was discarded.
