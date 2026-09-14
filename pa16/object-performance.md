# PA16 object/address implementation evidence

Entry: `caac2cda243e4ab71d6f5aca48159f88b9b8da3b`. Implementation tip:
`f1497ca2`. The executable and implementation-path hashes in
[objects-checkpoint.json](../student.tests/pa16/objects-checkpoint.json) identify
this handoff independently of later documentation commits.

## Ownership and cost

The semantic evaluator owns canonical typed aggregate payloads, storage roots,
and interned field/base/element paths. Initializer plans, selected conversions,
constructor actions and checked function bodies feed these values. Frames own
parameter/local bindings and retire their addressed storage. Declarations publish
constant values and static-data facts; lowering consumes them. Arrow chains
record selected functions, receiver adjustments, return types and temporary
identities once; evaluation, exception checking and lowering use those records.
No grammar replay, synthetic frontend expressions or reference execution supplies
required compiler output.

Aggregate hashing/equality visits actual initializer parts, with sparse ranges
for omitted array elements. Class projection is indexed; array projection uses a
binary search. Address projection is interned; byte offsets are computed once per
path after layout. Call keys include typed arguments, receiver paths, and the
version/liveness of storage reachable through pointers/references. Aggregate
summaries restrict dependency traversal to address-containing children, so a
scalar-only aggregate does not require a field walk on every call. Constructor
builders expose initialized fields and advance storage versions as fields become
available. Repeated related base projections follow explicit inheritance edges.
These owners live for one translation unit; execution frames/builders are local.

The evaluator retains the mandated implementation resource policy: 512 active
calls and 1,000,000 executed expression/statement/loop steps per root. PA16/O0
adds no native optimizer. The scalar conversion fast path uses the same numeric
conversion rules while avoiding address processing; it adds no allocation,
iteration or emitted-program growth. Invocation key vectors reserve their known
minimum size.

## Frozen protocol

Artifacts are in
`/home/vishvananda/work/private/v4codex/artifacts/pa16-object/`.
[objects_benchmark.py](../student.tests/pa16/objects_benchmark.py) freezes inputs,
A/B executables and `--emit-lowir -O0`, uses one warmup each, four A/A observations
and two ABBA blocks, and retains every timing/RSS observation. Newly accepted
programs have six B-only observations, with recorded entry rejection; they do not
support an A/B speedup claim. Runtime programs use volatile trip counts and check
their results. The supplied native backend is validation only, bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, at O0.

- A: `entry-cppgm`, SHA-256 `7f3b39ee21c6e4f9ac3f8e361bb9843c6b98e4ff9935800c1233d642131a46f1`.
- B: `final-cppgm`, first completed implementation measurement, SHA-256 `20d74289b04652485422030929e965dce30d80defa024908b9e5da26c6edfda8`.
- C: `scalar-fast-cppgm`, scalar cost repair; its complete hash is in the checkpoint.
- D: `literal-demand-cppgm`, final literal-demand repair, SHA-256 `07d000c2d4443a6091609916a7b80cfbfad531a7019cd974cbe84016035f5de8`.

`performance.json` retains 224 A/B observations including warmups.
`scalar-fast-performance.json` retains 224 A/C observations.
`literal-demand-performance.json` retains 224 final A/D observations.
[objects_noise.py](../student.tests/pa16/objects_noise.py) adds 132 B/C observations
in `scalar-fast-control.json`: four A/A observations and four ABBA blocks per
workload, prompted by scalar overhead and large timing spikes. Thus all **804**
new observations are retained, alongside the **3,512** observations linked from
[audit-performance.md](audit-performance.md) and its predecessors.

The first preflight found that A emitted an undemanded template body while B
correctly omitted it. No timings were taken for that comparison. Its inputs,
outputs and failure log remain under `benchmark-preflight-1*`; the final common
workload explicitly demands the function at runtime. All common measured LowIR
outputs are byte-identical. All four measured runtime executables are also
byte-identical across A, B, C and D.

## Results and noise

Compiler times are medians in milliseconds; RSS is the maximum observed KiB.
The table uses the final A/D run, without dropping outliers.

| Workload (4,000 units) | A / D ms | A / D RSS | Paired D/A |
| --- | ---: | ---: | --- |
| Scalar template calls | 69.44 / 74.12 | 13,176 / 13,224 | 1.0752, 1.0578 |
| Runtime memory/floating functions | 402.43 / 407.84 | 71,072 / 70,612 | 1.0079, 1.0208 |
| Runtime literal classes | 493.82 / 494.06 | 83,940 / 83,768 | 1.0051, 0.6530 |
| New constant objects | rejected / 174.65 | — / 29,596 | no A/B claim |
| New sparse aggregate argument keys | rejected / 90.94 | — / 18,996 | no A/B claim |

The scalar fast path gives repeatable improvement over B: four paired C/B ratios
**0.974, 0.968, 0.972, 0.970**, with A/A calibration 74.83–76.18 ms. Compiler `.text`
is 1,578,630 / 1,634,566 / 1,634,630 / 1,634,950 bytes for A/B/C/D: required
functionality adds 56,320 bytes over entry, and the fast path adds 64 bytes over B.
The remaining approximately 6–8% scalar-call cost accompanies generalized typed-value conversion,
invocation keys and frame lifetime handling; semantic work counters remain
40,087 nodes, 4,000 activations and 28,000 execution steps for both A and D.

The large-object A/C run includes C samples from 174.27 to 321.44 ms; other
workloads show similarly large spikes on A as well as B/C. The additional B/C
blocks retain those spikes, too (one object ratio is 0.448). They give median C
object times 46.91 / 178.20 ms at 1,000 / 4,000 units, and sparse-key times
27.38 / 94.14 ms. The corresponding work scaling is about 3.80x and 3.44x;
RSS is 11,508 / 29,124 KiB and 8,856 / 18,948 KiB. These are diagnostic evidence,
not replacement samples or new acceptance gates.

| Runtime | Final A / D median ms | Executable text A / D bytes |
| --- | ---: | ---: |
| Calls | 359.76 / 360.14 | 206 / 206 |
| Memory | 211.60 / 210.41 | 434 / 434 |
| Floating point | 249.99 / 249.51 | 230 / 230 |
| Literal class calls | 503.00 / 505.86 | 248 / 248 |

The intermediate A/C calls observations span 360.45–633.30 ms despite identical
executable bytes (medians 590.82 / 487.31 ms); the earlier A/B run gave
359.72 / 360.76 ms. The final A/D runtime table above is more stable. No runtime improvement or regression
is attributed to this noise. Text counts are executable payload after ELF entry
for the sectionless backend outputs; these workloads have no static data. Small
runtime-input compiler timings (roughly 6–9 ms) are startup diagnostics, not
compiler throughput claims.

The final literal-demand repair reserves symbols from typed constant facts and
flushes string data after other globals. A new native control checks an object
first used in static_assert and its string-pointer addend. D adds 320 compiler
text bytes over C; the common benchmark IR/executables stay unchanged. Final D
object and sparse-key medians scale 3.68x and 3.37x for 4x source units.

## Acceptance

Required semantics and coverage remain mandatory. PA16/O0 has no mandated
percentage compiler-latency, RSS or code-growth ceiling. Historical +15%, +16 MiB
and 5.5x diagnostic targets remain preserved; the noisy 6.18x large-object median
in the A/C run is not a permanent stage failure. The repeated follow-up, unchanged
work shape, fixed resource limits, and absence of a new optimizer support the
stage-scoped acceptance. No optional runtime transform was used to justify extra
compiler work. Native optimization, encoding and self-hosting retain their later
assignment owners.
