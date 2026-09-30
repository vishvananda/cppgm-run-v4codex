# PA25 driver135 performance evidence

The entry driver was a stub, so it cannot be a correct performance baseline.
Frozen A is the enabled driver before the TLS/generated-helper and final ELF
alignment fixes; B is the final driver. Both are correct on these inputs.
B's hash matches the production binary. No optimization or speedup is claimed.

Reproduce with `performance.py ARTIFACT_DIR FROZEN_A FROZEN_B`. Frozen binaries,
inputs, hashes, every observation, checked executables and phase telemetry are
in `/home/vishvananda/work/private/v4codex/artifacts/pa25-135/`.
`performance.json` contains 168 samples: four A/A observations and six ABBA
blocks for each of three compilation and three execution workloads. Wall time
and peak RSS use `/usr/bin/time`; measurements run sequentially with no build
or test workload in parallel. No observations or outliers were discarded.

The template input demands 4800 function specializations across eight source
TUs, then links a ninth main TU. Memory and floating inputs each execute three
million iterations, with loop bounds read from argc and independently computed
checked results. Memory covers calls, arithmetic and indexed load/store work;
floating covers calls, double arithmetic and conversion. All executable pairs
are byte-identical, pass their result checks and retain runtime-dependent work.
Self-hosting remains PA34's scope; inherited fixed PA24 benchmarks are preserved.

| Workload | B compiler batch s | Compiler peak KiB A/B | Paired compiler B/A median [range] | A/A compiler batch range s |
|---|---:|---:|---|---|
| Templates (one compile) | 0.25275 | 14676 / 14656 | 1.021 [0.947, 1.102] | 0.26156–0.29214 |
| Memory (64 compiles) | 0.40990 | 6688 / 6688 | 0.999 [0.956, 1.069] | 0.40827–0.44291 |
| Floating (64 compiles) | 0.37234 | 6784 / 6784 | 0.997 [0.965, 1.027] | 0.36938–0.39103 |

Small compiles are batched to exceed startup timing granularity; these totals
include process creation and the measurement utility. Per-compile medians are
about 6.40 ms and 5.82 ms for memory and floating. They are not phase-only times.

| Workload | B runtime batch s (3 runs) | Paired runtime B/A median [range] | Text bytes A/B |
|---|---:|---|---:|
| Templates | 0.11806 | 1.002 [0.992, 1.034] | 384567 / 384567 |
| Memory | 0.14979 | 1.001 [0.991, 1.120] | 521 / 521 |
| Floating | 0.13842 | 0.999 [0.993, 1.009] | 340 / 340 |

Runtime A/A ranges are 0.11718–0.11793, 0.14786–0.15032 and
0.13804–0.13862 seconds. Executable RSS observations are 256 KiB; runtime
measurements include process startup and the measurement utility. Identical
images establish no generated-code change on these workloads; timing variation
is disclosed rather than described as a speedup or regression.

The optional `--stats` source-to-ELF trace records 4800 specializations, 4800
body transitions, 4809 selected native functions and 96140 native instructions.
It compiles with an empty PATH, executes correctly and reproduces the same ELF
bytes. Trace RSS is 16972 KiB; this instrumented run is separate from timing.

O0 adds no optional transformations, search, iteration or code growth. Linking
costs O(bytes + (symbols + relocations) log symbols), with average constant-time
flat interning/lookup, one native definition per emitted source definition and
at most one GOT slot per foreign symbol per object. The linker retains final
native bytes, compact identities and relocations; frontend/TU graphs and local
MIR are released at their established boundaries. Alignment padding is bounded
by the requested section alignment (maximum supported foreign alignment 4096).
Compiler object storage and transient linking memory are linear in input size.
Inherited 15% latency/RSS and zero optional text-growth diagnostics remain
measurements/diagnostics, not additional gates. Mandated correctness/coverage,
finite work bounds and the eventual stage pass remain required.

The first benchmark candidate used an unused dependent local declaration and
was rejected by the inherited frontend before native compilation. Its inputs
and error remain in `initial-inputs/` and `initial-benchmark-error.txt`; the
reducer `unused-dependent-local.cc` is recorded as unfinished implementation,
not waived or used as a performance baseline. The final fixed benchmark uses
supported function templates. Required fixtures and references were unchanged.
