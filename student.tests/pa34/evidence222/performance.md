# PA34 independent frozen performance evidence 222

A is the canonical GCC-built seed; B is the canonical self compiler. Frozen
copies have the same hashes as evidence 221 and the current canonical binaries.
Self and inception match byte for byte. Both builds use default O3; the seed
uses the system allocator and self retains the standard PA34 jemalloc linkage.
This compares canonical generations, not an isolated optimization. Compiler
`.text` is 4,224,070 bytes in A and 8,562,501 in self/inception; executable file
sizes are 5,108,720 and 12,674,288 bytes. These are code-quality observations,
not an added size gate. [Image identities](../evidence/compiler-images.json).

All 476 fresh observations are retained in [selfhost.json.gz](selfhost.json.gz),
[common-o0.json.gz](common-o0.json.gz) and [common-o2.json.gz](common-o2.json.gz).
Each group uses four A calibration runs followed by six ABBA wall-time blocks,
pinned to CPU 2 without concurrent builds/tests. Compilation and execution are
timed separately. [Commands](commands.json), [environment](environment.json),
flags and input/binary hashes make the configurations explicit. No observation
is discarded. Deterministic gzip only compresses the JSON; all raw values remain.

The fixed common sources demand 2,400 template specializations and execute
argc-dependent memory/call, floating and exception/destructor loops with checked
results. Pruning adds 1,200 unused functions. Runtime samples are 40–453 ms,
well above process startup; compiler samples are 168 ms or longer. The compiler
component remains dev/src/lowir/folding.cpp at O0; executable runtime is N/A
for that component object. There is no workload rewrite or resource override.

## Compiler latency and peak RSS

Seconds and KiB are A/B. Ratios are median paired block ratios with full range;
independent medians need not yield the same ratio. [Recomputed summaries](samples.json)
include every A/A range and per-configuration spread.

| Workload | Compile seconds A/B | Peak RSS KiB A/B | Paired B/A [range] |
| --- | ---: | ---: | ---: |
| Compiler source O0 | 2.1718 / 4.1474 | 77,852 / 87,896 | 1.981 [1.582–2.205] |
| memory O0 | 0.3970 / 0.9531 | 30,756 / 40,400 | 2.383 [2.276–2.670] |
| floating O0 | 0.1724 / 0.4147 | 30,972 / 40,652 | 2.393 [2.277–2.667] |
| exceptions O0 | 0.1740 / 0.4088 | 30,704 / 40,376 | 2.365 [1.760–2.388] |
| pruning O0 | 0.2249 / 0.5707 | 36,236 / 46,140 | 2.372 [1.409–2.699] |
| memory O2 | 0.2489 / 0.6573 | 30,760 / 41,728 | 2.604 [2.249–3.031] |
| floating O2 | 0.2563 / 0.5648 | 30,920 / 42,160 | 2.457 [2.076–3.276] |
| exceptions O2 | 0.2217 / 0.5756 | 30,364 / 42,088 | 2.584 [2.351–3.126] |
| pruning O2 | 0.2465 / 0.6077 | 36,472 / 46,648 | 2.463 [2.071–2.721] |

Compiler-source calibration spans 2.121–4.503 s and later seed samples span
1.978–2.868 s. This environmental spread is retained. Common compilation is
consistently slower in self, with paired medians 2.365–2.604. Every timed object
has a verified hash: 28 compiler-source and 224 common compilations. All A/B
prepared objects/executables and all nontiming common work counters agree.
The unchanged compiler-source work is also recorded in
[selfhost-work.json](../evidence/selfhost-work.json). This identifies a generated
compiler code/allocator quality gap without evidence of extra semantic work.

## Generated runtime and text

All compared A/B executable images are byte-identical and every execution
checks its result. Runtime ratios therefore describe measurement variation,
not a generated-code improvement or regression. `.text` means actual text
sections, excluding other readonly sections. The compiler-source object has
34,466 text bytes in both generations.

| Workload | Runtime seconds A/B | Peak RSS KiB A/B | Paired B/A [range] | Executable text A=B |
| --- | ---: | ---: | ---: | ---: |
| memory O0 | 0.0953 / 0.0928 | 1,760 / 1,756 | 0.952 [0.908–1.064] | 151,633 |
| floating O0 | 0.0496 / 0.0496 | 1,884 / 1,756 | 1.001 [0.992–1.007] | 151,474 |
| exceptions O0 | 0.2552 / 0.2527 | 3,936 / 3,932 | 0.997 [0.902–1.007] | 151,781 |
| pruning O0 | 0.0717 / 0.0712 | 1,760 / 1,760 | 0.979 [0.930–1.070] | 151,633 |
| memory O2 | 0.0572 / 0.0500 | 1,760 / 1,760 | 0.970 [0.863–1.012] | 125,179 |
| floating O2 | 0.0409 / 0.0408 | 1,760 / 1,760 | 0.994 [0.933–1.006] | 125,086 |
| exceptions O2 | 0.2501 / 0.2494 | 3,908 / 3,856 | 0.999 [0.992–1.012] | 125,268 |
| pruning O2 | 0.0490 / 0.0489 | 1,756 / 1,760 | 0.998 [0.997–1.001] | 125,179 |

## Provenance and acceptance

`verify_samples.py` independently recomputes all 476 fresh observations and
252 timed object hashes. It also checks all 532 historical observations
(476 final plus 56 stack diagnosis) and their recorded summaries; the result
is [historical-samples.json](historical-samples.json). The original common
harness checked prepared images and work but did not retain each timed common
object hash. The audit adds that check after stopping the timer, without
altering previous data or excluding samples. All nine current prepared A/B
object configurations match the accepted historical images.

[The trace](trace.json) adds direct/replay/generation equality at O0–O3 and
24 checked executions. Eight reporting-off objects equal their --stats versions.
The tracing overhead is separable; reporting does not trigger semantic demand.
The final compiler-source and common benchmarks keep flags identical in A/B.

The earlier 64 MiB stack measurements remain diagnosis only. The actual
dispatch correction has before/after ratio 1.006 [0.881–1.317], a 4,268 KiB
RSS decrease and identical generated text; compiler text grows 704 bytes
(0.008%). It restores correctness at the normal 8 MiB limit. All canonical
validation and the three-generation 400-call reducer use that normal limit.

Spec §9 governs acceptance: inherited 2x latency, 1.75x RSS, zero-growth, 10%
runtime and subsequent 1.5x/1.05x/1.25x targets are historical diagnostics, not
universal PA34 gates. Neither these ratios nor compiler agreement waive
correctness, avoidable regressions, mandated 900/3600-second and 8 GiB limits,
MIR envelopes or the finite optimization reservoirs in the plan/audit. No
optional PA34 transform or speedup claim was introduced. Existing PA32/PA33
affected-workload benefits and disclosed regressions remain recorded at their
owners. All four dimensions and the canonical generation gap are preserved.

Reproduce with `python3 student.tests/pa34/measure.py OUT FROZEN_SEED FROZEN_SELF`
after correctness work finishes, then run `verify_samples.py EVIDENCE OUT.json`
on the retained datasets. `trace.py OUT SEED SELF` reproduces the source trace.
Raw artifacts, frozen binaries and full logs are bound by [binding.json](binding.json)
under `$RALPH_ARTIFACT_DIR/pa34-222`; generated executables/objects/logs stay
outside version control.
