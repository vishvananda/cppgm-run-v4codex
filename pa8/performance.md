# PA8 final independent performance audit

Final implementation: `bc2cd043d` (measurement/protocol revision `2f21a26ac`).
Frozen A is incoming `7313af05a`; its binary hash exactly matches the preceding
checkpoint B. Frozen B includes the phi and builder ownership corrections.
GNU C++11/O3 flags and test-runner configuration are identical. No optimization
or runtime improvement is claimed. This experiment bounds mandatory validation
cost while checking that executable work and size remain equivalent.

The [prospective protocol](../student.tests/pa8/final-audit-protocol.md) fixes
budgets before timing. [Raw final observations](../student.tests/pa8/final-audit-performance.json)
contain binaries, full implementation/harness/input hashes, revisions, flags,
wall/RSS/user/system times, context switches, phase/work counts and output hashes.
The runner rechecks exact AAAA + ABBA + ABBA ordering, successful results,
equivalent output and current artifact identities/text sizes. No builds or tests
ran concurrently with timing. CPU 0: Intel Xeon 2.20 GHz; g++ 15.2.0.

The same corpus generator uses 12,000/48,000 functions, including a newly frozen
handler/cleanup/ordinary-phi family. Runtime loops use 40 million iterations
(40,000,040 for sum). Counts doubled prospectively from the historical run to
dominate startup. All 200 primary observations and 14 construction/adapter
observations are retained; there were no failed or superseded final campaigns.
Compiler startup median is 5.010 ms; native startup is 3.213 ms.
Every main sample exceeds 20x its corresponding startup.

## Compiler latency, peak memory and text

| Workload | B wall median (range), ms | B RSS, MiB | Paired wall change, blocks 1 / 2 | A/A spread |
| --- | ---: | ---: | ---: | ---: |
| calls-integers-1 | 480.23 (467.21–487.75) | 78.73 | -0.65% / -0.86% | 5.13% |
| calls-integers-4 | 1864.93 (1861.22–1873.80) | 304.60 | -1.02% / -0.39% | 5.00% |
| memory-floating-1 | 596.14 (590.52–621.53) | 79.71 | -2.66% / +3.11% | 4.35% |
| memory-floating-4 | 2372.68 (2360.39–2383.09) | 305.46 | +0.33% / +1.57% | 0.89% |
| cfg-phi-1 | 212.80 (212.01–214.21) | 31.59 | +1.16% / +0.07% | 3.02% |
| cfg-phi-4 | 839.83 (836.72–847.56) | 116.64 | +0.99% / +1.05% | 2.62% |
| cfg-handlers-1 | 218.55 (217.39–219.68) | 38.40 | +2.58% / +3.15% | 1.81% |
| cfg-handlers-4 | 846.21 (845.15–848.16) | 142.79 | +1.93% / +2.22% | 0.22% |

Mean paired wall changes range from −0.76% to +2.87%. Handler-family regressions
of +2.08% / +2.87% pay for checking actual EH membership rather than silently
accepting malformed phi nodes. Other small changes include timing noise; no
speedup is inferred. Maximum median RSS growth is 18 KiB (the largest decrease
is 40 KiB). Host `.text` changes from 130,054 to 131,910 bytes (+1.43%).
Fourfold input scales 3.87–3.98x wall and 3.69–3.87x RSS. Every workload passes
the unchanged <=10% + noise wall, <=20% +1 MiB RSS, <=25% host-text and
<6x wall / <5x RSS +1 MiB scaling budgets.

The largest case has 1,392,000 instructions and 2,400,000 operands; B reports
1,392,000 full instruction validations and 169 IR-pool growths. The additional
handler discovery scan visits each instruction once; it is not included in
`validated_instructions`. No repeated fixed point or unrelated-function search
is introduced. Handler scratch is one bit per block. Total validation remains
O(IR + E log E), with O(IR + E) scratch and zero IR growth. The API ownership
checks are constant work per append. Pool counters exclude interner/local-index
and host-library allocations; capacity is distinct from committed RSS.

Separate telemetry observations preserve output and work counts. B telemetry
wall deltas range −1.64% to +2.50%, including noise; they do not establish
negative measurement overhead. Always-active growth counters are included in
the primary A/B costs.

## Executable runtime and text

The fixed harness exercises loops, helper calls, aliased memory, scalar floating
work and conversions. Runtime volatile loads vary arguments, retain accumulators
and check results/exit status. Course callback-effect checks supplement this
benchmark. Construction, adapter invocation, supplied-backend compilation and
execution are measured separately. Backend source revision remains
`5ddcfe408e3d277825a6fc39a06f7f6bdeff7709`, using `-O0`.

| Workload | B runtime median (range), ms | Text bytes A = B | Paired wall change, blocks 1 / 2 | A/A spread |
| --- | ---: | ---: | ---: | ---: |
| sum | 244.60 (227.90–247.73) | 234 | -7.43% / -4.20% | 4.77% |
| swap | 361.88 (361.20–363.76) | 250 | -0.14% / -0.01% | 0.46% |
| call | 398.81 (396.05–401.34) | 270 | +0.61% / -2.03% | 0.82% |
| floating | 784.36 (783.30–785.89) | 417 | -0.21% / -0.26% | 0.46% |

All four pairs have identical executable hashes, checked results, and zero text
growth. The −5.81% mean sum timing difference therefore cannot be attributed to
a code improvement; its magnitude also shows the limits of a short noise
calibration. Every observation remains in the record. No executable regression
exceeds the frozen <=5% + A/A spread budget. Peak runtime RSS is 256 KiB for
both variants in all families.

The backend emits sectionless ELF. These workloads have no static data; text
means the executable load payload from the entrypoint, excluding ELF headers
and including padding. Binary inspection of sum confirms a retained helper
call in the loop, volatile counter/accumulator traffic and the checked result.
The helper begins at entry+0x10, uses multiply and signed division-by-two
lowered by the supplied backend to arithmetic/shift operations, and has a
16-byte frame; the caller uses a 24-byte frame plus saved RBX and a counter
spill/reload across the call. These costs are included in runtime, not inferred
from IR node counts. They describe the supplied PA8 backend, not a student
allocator or an isolated helper speed comparison. ABI/unwind/debug encoding
and optimization-level policies remain later native-stage responsibilities.

## Inherited template frontend

The [fresh frontend record](../student.tests/pa8/final-frontend-performance.json)
uses the exact unchanged PA7 template-demand generator and work assertions,
with eight independent TUs per sample. One frozen `cppgm++` binary is used for
both labels (358,790-byte host `.text`, zero growth); these are absolute costs
and A/A comparisons. The inherited protocol retains two AA pairs, BB warmup,
two ABBA blocks, all 28 main observations, 24 startup observations, four work
observations and 14 telemetry observations. Its budgets and verification pass.

| Template corpus | B wall median (range) | Peak RSS | Paired differences | A/A noise |
| --- | ---: | ---: | ---: | ---: |
| 1x: 3,600 uses/TU | 0.928 s (0.921–0.935) | 21.31 MiB | +0.93% / −0.30% | 0.96% |
| 4x: 14,400 uses/TU | 3.710 s (3.699–3.727) | 68.73 MiB | +0.20% / −0.15% | 0.92% |

Wall scales 4.00x and RSS 3.23x. Semantic startup is 5.42 ms, well below the
principal observations. Each TU has two specializations, two canonical
argument packs, five dependence computations and eleven types at both scales.
Expression work is 18,000/72,000; candidate work 3,600/14,400. The live cursor
peaks at three pending tokens. The workload measures repeated declaration
specialization demand through semantic output; it does not instantiate template
bodies or produce an executable. The source diff from the PA7 exit confirms
that preprocessing, parsing, semantic implementation and the frontend entry
point are unchanged. Fresh PA7 audit/API checks independently exercise those
facts and repeatable demand completion.

## Historical evidence and artifact lifetimes

The previous raw record is preserved unchanged. Its `/tmp` artifacts were absent
at final-audit entry, so its verification was not reused as a current gate.
The final runs have fresh artifacts and identity checks. Reproduction commands
are in [the personal-test README](../student.tests/pa8/README.md). Generated
inputs, binaries, ELF payloads and logs stay in `/tmp`; only scripts, raw numeric
observations and reports are committed. Self-hosting and template-source native
execution are unavailable at PA8; this audit does not substitute another
compiler implementation for those future surfaces.

# Historical checkpoint measurement

The first working PA8 implementation (`66167cf72`) is A; the final implementation
(`01d39a2f6`) is B. The stage-entry scaffold produces no LowIR and is not a valid
performance baseline. Both measured versions pass the 109 course cases, but A
loses signalling NaNs in an added fidelity probe. B adds local shape checks,
fixes literal preservation and counts IR pool growth. No optimizer or speedup
claim is made; this experiment bounds the cost of those correctness changes.

The [raw observations](../student.tests/pa8/performance.json) retain binary,
source and input hashes, host/CPU identity, GNU C++11/O3 compiler flags, backend
revision/flags, output hashes, wall/RSS measurements and phase/work telemetry.
Each workload has AAAA calibration and two ABBA blocks on one pinned CPU.
Every compiler output agrees within its family; all four native A/B executable
pairs are byte-identical. Reported latency deltas average the two paired block
ratios; paired spread and calibration spread are disclosed below.

Budgets fixed before measurement: compiler wall <=10% + A/A spread; RSS <=20%
+1 MiB; host text growth <=25%; 4x input <6x wall / <5x RSS +1 MiB; each main
sample >20x startup. Native runtime <=5% + A/A spread; native text growth 0%.
All gates pass. All 168 primary observations and 14 small exercise/adapter
observations remain, including the callback calibration outlier. None were
removed or superseded. Compiler startup median is 4.953 ms; native startup
median is 3.084 ms. The smallest main samples still exceed the 20x floor.

## Compiler latency and memory

| Workload | B median wall | B peak RSS median | Paired change (two blocks) | A/A spread |
| --- | ---: | ---: | ---: | ---: |
| calls-integers-1 | 236.48 ms | 41.08 MiB | +2.61% to +3.08% | 2.99% |
| calls-integers-4 | 940.55 ms | 154.02 MiB | +2.38% to +3.18% | 2.51% |
| memory-floating-1 | 296.72 ms | 40.61 MiB | +3.59% to +3.84% | 1.83% |
| memory-floating-4 | 1174.62 ms | 155.39 MiB | +3.27% to +3.50% | 1.26% |
| cfg-phi-1 | 109.22 ms | 17.37 MiB | +2.40% to +4.06% | 3.12% |
| cfg-phi-4 | 417.30 ms | 59.91 MiB | +2.39% to +2.64% | 4.08% |

The mean paired compiler regression is 2.52–3.72%; maximum median RSS increase
is 22 KiB. Host `.text` grows from 125,382 to 130,054 bytes
(+3.73%). These costs are disclosed, not presented as speedups. At 4x input,
wall scales 3.82–3.98x and RSS 3.45–3.83x. The largest case contains 696,000
instructions and 1,200,000 operands; the final validator visits each instruction
once. Its 14 IR pools grow only 160 times. Name interning and per-function
indexes are separate allocations, outside that pool-specific counter.

Separate stats-enabled observations preserve every output and report phase
and work counts. Measured B telemetry deltas range from -1.38% to +1.26%; these
small deltas include timing noise and are not evidence of negative overhead.
The allocation-growth counters themselves are always active in B and their
cost is included in the ordinary A/B comparison.

## Executable runtime and text

The fixed harness loads volatile loop state, varies helper arguments at runtime,
accumulates results, and checks the final value and exit status. Sum traverses
its complete allowed domain 20 times; the other workloads run 20 million
iterations. Swap includes identical-pointer calls, callbacks remain ordered,
and floating work includes volatile loads/stores and scalar conversions.
Generation, reference-backend compilation and execution are timed separately.
The backend is pinned to the manifest's source revision
`5ddcfe408e3d277825a6fc39a06f7f6bdeff7709` with `-O0`.

| Workload | B median runtime | Text bytes A = B | Paired change (two blocks) | A/A spread |
| --- | ---: | ---: | ---: | ---: |
| sum | 129.16 ms | 234 | -0.47% to +1.78% | 5.93% |
| swap | 183.77 ms | 250 | +0.47% to +0.48% | 0.80% |
| call | 201.83 ms | 270 | -0.13% to +0.26% | 47.39% |
| floating | 394.27 ms | 417 | -0.37% to +0.20% | 0.53% |

The supplied backend emits sectionless ELF. These harnesses contain no globals;
text size is the executable load payload from its entrypoint onward, excluding
ELF headers and including code padding. There is no static data in that span.
This is recorded explicitly instead of inventing a `.text` section.

The callback A/A calibration has a 47.39% outlier spread. Its paired deltas are
small, and identical executable hashes independently establish unchanged code;
no runtime improvement is inferred from timing differences in any family.
The experiment proves current-stage costs and equivalence, not the quality of
our future native backend. Template-heavy frontend and self-hosting costs remain
owned by their stages; the unchanged frontend's existing evidence is linked in
[PA7](../pa7/performance.md). Reproduction commands and artifact lifetimes are in
[the personal-test README](../student.tests/pa8/README.md).
