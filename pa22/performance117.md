# PA22/O0 checkpoint audit 117 performance

Last reviewed commit: `e90fa3fa514e2990bbe5ea4716252d8c42ae782a`.

The first campaign below measures the initial fixes at `015feb8d`; the final
canonical-constant campaign follows it. All observations from both are retained.
The [common/hierarchy harness](../student.tests/pa22/benchmark117.py) and
[proof harness](../student.tests/pa22/proof_benchmark114.py) use frozen binaries,
source contents/hashes, `--emit-lowir -O0`, CPU 0, one warmup per lane, four A/A
observations, then four wall-time ABBA blocks. Compilation and execution are
measured separately with peak RSS. Every generated executable checks its result;
volatile counts keep the calls, memory and floating loops live. Small compiler
inputs and trivial mains remain startup-sensitive. No build or course report
overlapped these campaigns. Telemetry plus validation preserves plain LowIR.

Frozen artifacts are under `/tmp/pa22-117/`. The harness's `implementation` field
records checkout HEAD before the fixes were committed; the actual B source is
pinned by the complete source hashes in [validation](../student.tests/pa22/audit117-validation.json)
and the initial audit-fix commit `015feb8d`. Initial binary identities are:

| Lane | Compiler `.text` bytes | SHA-256 |
|---|---:|---|
| Entry A, `853c4493` | 2263302 | `7d6e3b2880b9da9524957977af405d916d82e37e2b058aef7e581f387f1bbbae` |
| Reviewed B | 2264326 | `0167a0815f55b77c45808d760be97687c34e88b2d774be5cfbe8acda64d27ddd` |
| Same-source analysis-disabled G | 2263238 | `bafb7a642641299db4a762b185967dce5d8eaa84300102bead9b997921740258` |

G is reproduced by [proof_variant117.py](../student.tests/pa22/proof_variant117.py):
disable the local adjustment proof and its getter, compile that translation unit
with the production flags and relink the same objects. [Exact commands and hashes](../student.tests/pa22/audit117-proof-variant.json)
are retained. Both B and G pass all 18 composition controls. Incorrect entry
programs are correctness reducers, never speed baselines.

## Entry versus reviewed implementation

All ten inputs have byte-identical A/B LowIR, checked equivalent execution and
identical hosted ELF `.text`, including the same startup code. These inputs are
a common correct subset. The compiler adds 1024 text bytes, **0.045%**.
All observations, warmups, telemetry, binary/input hashes and paired ratios are
in the [common](../student.tests/pa22/performance117-common.json) and
[hierarchy](../student.tests/pa22/performance117-hierarchy.json) records.

| Workload | Compile median ms A/B | Compile peak RSS KiB A/B | Runtime median ms A/B | Native `.text` A/B |
|---|---:|---:|---:|---:|
| 9600 template specializations | 690.81 / 682.87 | 108500 / 108688 | 4.47 / 4.72 | 768286 / 768286 |
| Calls | 6.78 / 6.83 | 6144 / 6356 | 123.20 / 123.59 | 430 / 430 |
| Memory loop | 6.52 / 6.40 | 6152 / 6292 | 73.91 / 73.59 | 658 / 658 |
| Floating loop | 6.22 / 6.06 | 6180 / 6476 | 86.69 / 86.72 | 454 / 454 |
| Member-call loop | 6.56 / 6.58 | 6140 / 6352 | 209.62 / 210.09 | 488 / 488 |
| 2048 member-call functions | 152.29 / 151.19 | 28008 / 28256 | 3.88 / 3.96 | 184632 / 184632 |
| Hierarchy depth 8 / 512 functions | 58.10 / 57.60 | 14076 / 14128 | 4.06 / 4.11 | 217399 / 217399 |
| Hierarchy depth 8 / 2048 functions | 222.95 / 221.49 | 41836 / 42056 | 3.97 / 3.92 | 868663 / 868663 |
| Hierarchy depth 32 / 512 functions | 69.30 / 68.59 | 13804 / 14052 | 4.08 / 4.01 | 217399 / 217399 |
| Hierarchy depth 32 / 2048 functions | 255.05 / 251.59 | 37432 / 37564 | 3.91 / 3.98 | 868663 / 868663 |

Compiler paired B/A ranges for the six common inputs are respectively
0.660–0.996, 0.957–1.085, 0.960–0.982, 0.949–0.982, 0.963–1.077 and 0.292–1.029.
For the hierarchy inputs they are 0.984–1.012, 0.965–3.045, 0.751–0.992 and
0.982–2.713. A/A template compilation spans 674–1471 ms. The two large hierarchy
cases have B outliers of 1123/1125 ms; their B medians are 221/252 ms. The large
common function case has an A outlier of 882 ms. None are discarded. These
observations support no general latency improvement claim. The largest measured
compile RSS increase is 296 KiB. Generated work is identical, so runtime noise
is not represented as an optimization result.

The hierarchy inputs repeatedly convert a local `Root::*` to `D::*` through
8/32 intermediate classes and a nonzero base offset. At 512→2048 functions,
base-path work remains **20→20** (depth 8) or **68→68** (depth 32) in both lanes.
B's path-cache hits are **1536→6144**, versus A's **1024→4096**: every newly
cached hierarchy question reuses the existing owner instead of doing A's
additional uncached graph walk. Proof work is **2048→8192**. There is no extra
path storage on these workloads. Work follows demanded facts and functions;
output size and peak memory follow the consumed source/IR.

## Optional local proof: generic versus reviewed

[All G/B observations](../student.tests/pa22/performance117-proof.json) are kept.
This isolates the full analysis cost, rather than disabling only its consumer.

| Workload | Compile median ms G/B | Compile peak RSS KiB G/B | Runtime median ms G/B | Native `.text` G/B |
|---|---:|---:|---:|---:|
| 40-million member-call loop | 6.77 / 6.43 | 6132 / 6296 | 254.18 / 211.19 | 566 / 488 |
| 2048 member-call functions | 157.99 / 154.06 | 28396 / 28308 | 3.92 / 3.89 | 336184 / 184632 |

The live loop improves **16.9%** at the median. All four paired runtime B/G
ratios improve: **0.833, 0.824, 0.831, 0.776**. A/A runtime spans 251.22–255.24 ms;
measured G/B ranges are 253.65–291.39 / 210.58–215.12 ms. Compiler paired ranges
are 0.922–0.960 (tiny input) and 0.258–0.973 (large input, with a 1045 ms G
outlier). No broad compiler-speed claim follows. B adds **1088 compiler text
bytes (0.048%)** over G while removing **78 loop text bytes** and **151552 text
bytes** in the large case. The latter's main is an execution check, not a runtime
benchmark.

Disassembly of the hosted loop confirms the useful work removed: G extracts
the high word with a `shrd` sequence, writes/reads an extra stack temporary and
adds the receiver displacement. B omits that sequence. The frame reservation
falls **128→112 bytes**; the indirect call and live loop remain. This is actual
runtime and stack-traffic evidence, not merely a smaller LowIR count.

## Legality, bounds and stage acceptance

The proof consumes checked incoming conversions **before** traversing wrappers.
All local writes and reference/address/capture exposure participate before the
TU's boundary pass freezes the fact. Volatile, unknown, adjusted or cyclic
values retain generic extraction. Each root shares a **64-node** budget across
recursive dependencies; active/proven/unknown states prevent retries. The
pipeline limit is at most 64 visits per demanded local fact, plus linear write
recording. A visit now checks one existing conversion record; it never resolves
an overload or demands an arbitrary body. The inherited 200-local cyclic control
and 512→2048 growth evidence remain in [114](performance114.md).

Hierarchy facts belong to the existing TU-owned canonical base-path cache and
are reused only after source-class completion. Open classes retain uncached
queries; there is no global invalidation. Ranking examines required candidate
and hierarchy facts, with no emitted growth. Member conversion/application emits
constant-size IR; the eight-element initializer expansion cap and loop fallback
remain unchanged. No new optimization level or native pass is introduced.

The accumulated [114](performance114.md), [115](performance115.md) and
[116](performance116.md) observations remain intact. In particular, 114's earlier
member representation has a documented runtime cost in the supplied backend;
115's immediate NTTP target does **not** establish a repeatable runtime win. That
NTTP form is required by the O0 contract, while the optional local proof has the
independent repeatable benefit above. No historical latency or smaller-IR claim
is used to waive correctness or profitability.

PA22 requires O0 LowIR shape and semantics, with no numeric latency/RSS exit
limit. Under spec §9, inherited +15%, +16 MiB and 5.5× targets remain diagnostic;
all samples and misses are preserved. Mandated correctness, comparison rules,
coverage and work/growth bounds are unchanged. Native selection/allocation,
object emission, debug acceptance and self-hosting belong to later stages.
The five existing shape failures remain implementation work, not performance
exceptions or approval to advance.

## Final canonical-constant campaign

The final implementation is `e90fa3fa`, frozen as `/tmp/pa22-117/sealed`, SHA-256
`b0c10c42b6532a3ff00072a1c3ec02c768af51944246c81ffd82ad3f2e365c13`.
Its compiler `.text` is **2267526 bytes**, up **4224 (0.187%)** from audit entry.
The new [generic lane construction](../student.tests/pa22/audit117-proof-variant-sealed.json)
has SHA-256 `0663dd8e5a73a651e71370224e7ea1261250e4cb362614ed5cb212f6c5263b65`
and 2266438 text bytes. Its only difference is disabled adjustment analysis and
consumption. All 22 audit controls pass in both final lanes.

Fresh [common](../student.tests/pa22/performance117-common-sealed.json),
[hierarchy](../student.tests/pa22/performance117-hierarchy-sealed.json) and
[constant-owner](../student.tests/pa22/performance117-constants.json) campaigns use
the same A/A + ABBA protocol without overlapping checks/builds. All **14** inputs
have identical entry/final LowIR and native text, and checked equivalent results.
The address workloads share a nonzero-displacement member function across many
constexpr globals; receiver workloads repeat a valid member access in static
assertions. These are common correct inputs. The broken chained/repeated-base
reducers are correctness evidence only.

| Workload | Compile median ms A/B | Compile peak RSS KiB A/B | Runtime median ms A/B | Native `.text` A/B |
|---|---:|---:|---:|---:|
| auto-specializations-9600 | 2460.45 / 2017.73 | 108140 / 108316 | 6.57 / 6.51 | 768286 / 768286 |
| runtime-calls | 13.00 / 12.65 | 6180 / 6408 | 289.87 / 290.37 | 430 / 430 |
| runtime-memory | 15.94 / 16.50 | 6172 / 6336 | 156.20 / 159.83 | 658 / 658 |
| runtime-floating | 14.72 / 14.52 | 6412 / 6452 | 143.41 / 142.55 | 454 / 454 |
| runtime-member | 14.91 / 14.28 | 6140 / 6312 | 448.30 / 358.23 | 488 / 488 |
| member-functions-2048 | 465.45 / 553.46 | 27620 / 27656 | 6.96 / 8.25 | 184632 / 184632 |
| hierarchy-8-512 | 217.15 / 217.58 | 13916 / 14124 | 11.98 / 12.13 | 217399 / 217399 |
| hierarchy-8-2048 | 776.43 / 813.29 | 41864 / 42068 | 6.77 / 7.38 | 868663 / 868663 |
| hierarchy-32-512 | 229.40 / 219.70 | 14904 / 15040 | 9.38 / 9.60 | 217399 / 217399 |
| hierarchy-32-2048 | 897.55 / 780.75 | 41820 / 41872 | 5.20 / 5.27 | 868663 / 868663 |
| constant-addresses-512 | 35.05 / 34.16 | 7956 / 8128 | 5.02 / 5.10 | 403 / 403 |
| constant-receivers-512 | 21.29 / 20.88 | 6964 / 7224 | 5.00 / 4.97 | 248 / 248 |
| constant-addresses-2048 | 88.69 / 108.71 | 13820 / 14176 | 4.29 / 4.51 | 403 / 403 |
| constant-receivers-2048 | 60.93 / 60.86 | 9552 / 9808 | 5.11 / 5.15 | 248 / 248 |

The final campaign has much larger scheduling spread than the initial campaign.
Template compiler A/A spans **2082–3078 ms**; measured A/B ranges are
1310–2938 / 1341–3009 ms. The large member-function compilation's paired ratios
are **1.428, 0.879, 1.017, 1.189**, and its median rises 18.9%; A/B ranges overlap
at 296–640 / 298–632 ms. The 2048-constant address case rises 22.6% at the median,
with paired ratios **0.977, 3.936, 0.914, 1.200** and A/B ranges
65–131 / 66–435 ms. These observations are retained, not hidden by the earlier
campaign. They do not establish a repeatable latency regression or gain. Likewise,
identical generated code cannot justify claiming the apparent member-loop runtime
improvement. One tiny hierarchy main has a runtime ratio of 56.536; its entire
observation remains recorded. No general latency/runtime claim is made from
these common-input comparisons. Peak compiler RSS grows by at most **356 KiB**.

The new owners have bounded, measured storage/work: ordinary member workloads
retain **one 16-byte constant record** (32 bytes capacity including null), at
both 512 and 2048 functions. The nonzero address workloads retain **two records**
(64 bytes capacity) at both 512 and 2048 globals. Repeated constexpr reads retain
one record and execute **one receiver-search visit** at both assertion counts;
subsequent requests hit the address/value cache. The source-to-target conversion
is composed in semantics and lowering reads a pool entry without hierarchy
search. Receiver address selection does not cache mutable values or bypass
lifetime/readability checks. These required semantic facts add no emitted work
on equivalent correct inputs, and repair static initialization where the entry
implementation is incorrect.

The [final proof comparison](../student.tests/pa22/performance117-proof-sealed.json)
reports G/B compiler medians **9.21/8.76 ms** (loop) and **161.87/207.24 ms**
(2048 functions), with peak RSS **6160/6364** and **27432/27664 KiB**. The latter
median regression is disclosed; paired compiler ratios are
**1.211, 0.985, 0.955, 0.671**, with A/A 154–274 ms and overlapping G/B ranges
154–662 / 152–279 ms. It supports no stable compiler-latency claim.

The live loop runs **449.21/323.61 ms**, native text **566/488 bytes**, and
paired runtime B/G ratios **0.732, 0.714, 0.711, 1.019**. The final block's slight
regression is retained. Together with the first audit campaign's four improving
pairs and 114's repeated proof experiments, this supports retaining the bounded
proof. The final loop LowIR and generated text match the first campaign's proof
lanes; the previously inspected stack/shift traffic is unchanged. The large
function input retains **336184/184632** text bytes, with runtime
3.85/3.89 ms serving only as a checked trivial-main execution. The proof adds
1088 compiler text bytes (0.048%) over its same-source generic lane.

The existing 64-visit proof limit, immutable fact keys, O0 emission bounds and
stage-scoped acceptance above still apply. No optional transform or numeric gate
is added for the constant-owner correction. All fourteen inputs, the profitability
comparison and both sets of raw observations are checked by
[the final gate recorder](../student.tests/pa22/record117_final.py).
