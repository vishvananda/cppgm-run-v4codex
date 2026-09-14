# PA16 checkpoint audit performance

Reviewed code tip: `7c39a6edbfa43c226036b8a92fe236722ac85dcc`.
Campaigns froze implementation `83536144`; the following commit changes only the
audit verifier. Its production tree and frozen compiler are identical.
This is PA16 `--emit-lowir -O0`; native measurements use the supplied backend
only for validation. No optional optimization was added by the audit.

## Protocol and provenance

Three completed campaigns retain **854 observations including warmups**:
[accumulated](../student.tests/pa16/audit-accumulated-performance.json) and
[repeat](../student.tests/pa16/audit-accumulated-repeat.json), 280 each, compare
the frozen stage-base compiler to the reviewed compiler using the unchanged
[storage harness](../student.tests/pa16/storage_benchmark.py).
The [checkpoint campaign](../student.tests/pa16/audit-checkpoint-performance.json),
294 observations, compares `57bac58d` to the reviewed tip using the
[audit harness](../student.tests/pa16/audit_benchmark.py). It adds floating
constant calls and exception-then-body demand to the existing validity corpus.

Each common workload has one warmup per binary, four A/A samples and two wall-time
ABBA blocks. B-only cases retain six observations after warmup and the prior
rejection/incorrect-runtime evidence; they support no A/B speedup claim.
Flags, host build settings, CPU affinity, input/output and binary hashes, all
RSS/CPU/wall/context-switch samples and paired spreads are frozen. Preflight uses
stats and full LowIR validation; timings omit those flags, with identical output
hashes. Compiler and executable measurements are separate; volatile bounds,
indices and checked results prevent dead/constant workloads. Large compiler
inputs take 70–490 ms; approximately 6-ms tiny compiler inputs are retained only
as startup diagnostics. Runtime workloads take 0.10–0.50 s.

All **2,658** historical observations remain: scalar 896, storage 560, storage
noise 726 and validity 476. Earlier preliminary/noisy campaigns are retained,
with their original correctness qualifications. The prior handoff verifier passed
before edits. The [audit verifier](../student.tests/pa16/verify_audit.py) checks
historical source hashes against their recorded commits and preserves raw artifacts.

## Compiler latency, memory and text

Times are medians in milliseconds, RSS maximum observed KiB. Accumulated rows
below use the repeat; paired ranges include both complete accumulated campaigns.

| 4,000-unit input | Stage base / reviewed ms | Base / reviewed KiB | Paired B/A range |
|---|---:|---:|---:|
| Template calls | 73.96 / 69.89 | 14,488 / 13,164 | 0.938–0.990 |
| Memory/float functions | 397.52 / 399.95 | 70,384 / 70,996 | 1.000–1.264 |
| Shared constexpr array data | 411.65 / 388.16 | 72,472 / 68,868 | 0.867–0.944 |
| 40-element constexpr array copy | 241.42 / 228.67 | 43,464 / 41,004 | 0.688–0.948 |
| Persistent local statics | prior behavior incorrect / 226.72 | — / 41,124 | B only |

The first memory campaign includes 620 ms wall / 400 ms CPU; the repeat includes
457 ms wall / 380 ms CPU. Its other B samples are 396–402 ms. Median repeat cost
is 0.6%, with 612 KiB peak RSS growth. The first array-copy campaign includes a
431 ms A observation among 242–267 ms neighbors; its 0.688 pair is not a claimed
31% implementation speedup. Every observation remains. Repeats and owner counters
show no persistent computation cost matching those large wall excursions.

| Checkpoint input, 4,000 units | Entry / reviewed ms | Entry / reviewed KiB | Paired B/A |
|---|---:|---:|---|
| Template calls | 71.70 / 70.58 | 13,196 / 13,052 | 0.983, 0.987 |
| Memory/float functions | 422.32 / 414.61 | 70,816 / 70,820 | 0.981, 1.007 |
| Literal classes/member calls | 487.58 / 486.00 | 84,172 / 84,216 | 0.995, 0.868 |
| Default exception queries | 152.40 / 152.98 | 26,084 / 26,076 | 0.996, 1.008 |
| Dependent exception declarations | 251.31 / 254.22 | 36,760 / 36,776 | 1.002, 1.016 |
| Floating constant calls | 79.24 / 78.15 | 14,516 / 14,356 | 0.999, 0.982 |
| Exception plus body demand | entry rejects / 380.18 | — / 55,292 | B only |

The 0.868 class ratio also contains an A wall-time outlier; no general compiler
speedup is inferred. Reviewed compiler text is **1,578,630 bytes**: **+50,304
(3.29%)** over the stage base and **+64 bytes** over audit entry. The accumulated
validity work's previously measured 2–5% class-heavy cost remains disclosed in
[validity-performance.md](validity-performance.md); it establishes required
semantic facts. The audit fixes show no repeatable avoidable regression.

## Executable runtime and size

All common native binaries in the checkpoint comparison are byte-identical;
paired runtime ratios span 0.991–1.009. Calls, memory and floating native binaries
also remain identical over the accumulated range. The backend writes sectionless
ELF: report payload minus typed global data as code-plus-alignment, not a
fabricated .text section. Native peak RSS is 256 KiB.

| Accumulated workload | Base / reviewed code+padding | Base / reviewed data | Paired runtime B/A, both campaigns |
|---|---:|---:|---:|
| Calls | 206 / 206 | 0 / 0 | 0.994–1.001 |
| Memory | 434 / 434 | 0 / 0 | 0.992–1.012 |
| Floating | 230 / 230 | 0 / 0 | 0.996–1.001 |
| Array sharing | 349 / 349 | 14 / 7 | 0.990–0.999 |
| Array copy | 298 / 276 | 0 / 160 | 0.903–1.034 |

Array copying has medians 251.5/223.5 ms initially and 245.1/225.4 ms on repeat.
Three of four current pairs and the prior storage campaigns show a repeatable
benefit; the 1.034 noisy repeat pair is retained. This required form trades 22
fewer code/padding bytes for 160 readonly bytes: total payload grows by 138.
There is no claim that every array size becomes faster. Static-local execution
is newly correct relative to the stage base, so its roughly 103-ms result is
reported without a speedup ratio.

## Work bounds and stage acceptance

At 1,000/4,000 units, dependent declaration queries have zero body transitions and
7,000/28,000 exception work. The repaired spec/body workload has exactly
1,000/4,000 body transitions, checks and scalar activations, retaining the same
7,000/28,000 exception work. Its time grows 97.2 -> 380.2 ms and RSS
18,104 -> 55,292 KiB for 4x input. Floating calls share one template body.
Array sharing and static classification retain their linear typed-owner counts;
there is no global retry or optional transformation search.

PA16/O0 mandates correct behavior, complexity/ownership and bounded work, with
no numeric latency/RSS/compiler-text ceiling. Inherited +15% wall, +16 MiB RSS,
5.5x scaling and PA14/15 text diagnostics are investigation thresholds, not
accumulated exit gates. The original 32-byte copy cutoff cannot override the
README's copy requirement; it remains removed for supported constexpr arrays.
The ordinary-array oracle conflict remains explicit unfinished work.

Evaluator limits stay at 512 active calls and 1,000,000 executed steps per root.
These are implementation resource bounds, not an additional benchmark target;
the million-step number is not mandated by the README or spec. Boundary controls
preserve the 512-call behavior and bounded conservative failure. No legality,
coverage, comparison rule or mandated stage limit was weakened. Native selection,
allocation, optimized levels/debug and self-hosting retain their later-stage
owners. No historical diagnostic miss permanently fails this corrected checkpoint.

