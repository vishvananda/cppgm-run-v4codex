# PA15 loop 33 matching handoff evidence

Entry compiler: `3aff4801` (166/177). First matching implementation: `57f0cd95`.
Final compiler: `3c356866` (169/177). Final source tree and binary identity,
checks, exact failure sets, fixture/harness trees and artifact hashes are pinned
in [the handoff manifest](../student.tests/pa15/matching-handoff.json).
Run `python3 student.tests/pa15/verify_matching.py --live` to verify this handoff
against the current implementation, or omit `--live` to verify frozen evidence
in a later implementation. This is an implementation boundary; the independent
review marker remains `538cfcb0`.

## Protocol and preserved campaigns

All measurements use frozen binaries and source files, g++ C++11/O3 host build
flags, student `--emit-lowir -O0`, CPU 31, and the same hash-pinned supplied native
backend at O0. Preflight validates LowIR, checks executable results, and requires
byte-identical LowIR/executables whenever both compilers are correct. Compiler
and runtime measurements are separate `/usr/bin/time` invocations. Each common
workload gets two warmups, four A/A observations and two ABBA blocks. When the
entry compiler rejects, record that rejection and seven absolute final runs
(one warmup and six observations); no speedup over incorrect rejection is claimed.

| Campaign | Comparison | Invocations including warmups |
|---|---|---:|
| `performance.json` | Entry versus first matching implementation; preliminary | 154 |
| `performance-final.json` | Entry versus final; new selected-member runtime added | 168 |
| `performance-reuse.json` | First matching implementation versus final; all inputs correct on both | 224 |
| `performance-noise.json` | Repeat scalar/common runtime controls after noisy observations | 112 |

All **658 observations**, source/output/native hashes, peak RSS, user/system time,
context switches, phase/work counters and frozen harness versions are retained
under `$RALPH_ARTIFACT_DIR/pa15-loop33/`. The verifier checks the exact sampling
orders and artifacts; none of the preliminary or noisy samples was discarded.

## Compiler measurements

Absolute final medians and observed peak-RSS ranges for newly accepted workloads:

| Workload | N=1,000 latency / RSS KiB | N=4,000 latency / RSS KiB |
|---|---:|---:|
| Repeated/equal versus mismatching class arguments | 0.1117 s / 19,512–19,844 | 0.4346 s / 62,524–62,816 |
| Pointer-pattern ordering | 0.0520 s / 11,892–12,164 | 0.1972 s / 32,020–32,336 |
| Dependent-alias overloads | 0.1151 s / 24,240–24,512 | 0.4751 s / 79,724–79,944 |

The fourfold input increase costs 3.89x, 3.79x and 4.13x compiler wall time.
At N=1,000/4,000, repeated classes have 2N specializations/candidate visits,
2N substitution frames, two retained source regions and zero member-body
transitions. Pointer ordering has N specializations, 2N candidate visits, just
**two ordering computations**, and **2N−2 ordering-cache hits**. These counters
observe existing work without requesting extra analyses.

The final ordering/environment repair is compared against the correct first
implementation, with identical output. At N=4,000, repeated classes fall from
0.4508 to 0.4343 s median (paired B/A 0.9604, 0.9736), with peak RSS falling from
65,716–65,956 to 62,544–62,756 KiB. Ordering falls from 0.2039 to 0.1952 s
(paired 0.9564, 0.9513), with RSS 33,424–33,760 to 32,228–32,336 KiB. At N=1,000,
ordering's paired ratios are 0.9347 and 0.9663. This corroborates removing
repeated pair deduction and the discarded primary environment. It is required
semantic fact reuse, not a generated-code optimization or a speed claim for
previously rejected programs.

The common 16-parameter scalar workload does not show a repeatable regression.
Entry/final medians at N=4,000 are 0.9056/0.9076 s; its ABBA ratios disagree
(1.0936, 0.9267) because individual A and B runs stall at 1.05–1.07 s. The first
reuse campaign likewise has ratios 1.1650/1.0049. The preserved repeat gives
0.9076/0.8944 s medians and ratios 0.9530/0.9881. At N=1,000 the repeat is
0.21808/0.21809 s, ratios 0.9928/1.0153, against A/A 0.2137–0.2200 s. There is
no stable scalar speedup claim either. Small runtime-source compilations take
about 5–6 ms and are startup dominated, so their percentage ratios are not used
as frontend performance claims. All spreads and A/A ranges remain in the JSON.

## Executables, size and stage acceptance

The common runtime controls execute volatile-bounded loops and check exact
results for integer calls, memory updates and floating-point accumulation.
The added partial-class control executes a selected static member on the same
60-million-iteration call workload. Student-generated LowIR is passed to the
supplied backend solely by the personal harness.

| Final runtime control | Median runtime | Executable text payload |
|---|---:|---:|
| Calls | 0.3000 s | 206 bytes |
| Memory | 0.1757 s | 434 bytes |
| Floating point | 0.2080 s | 230 bytes |
| Selected partial-class member | 0.3128 s | 217 bytes |

These sectionless ELF payload measurements follow the shared harness's documented
entry-to-end metric; these inputs have no static data. Every timed run returns
the checked result. Common executable bytes are identical across entry/final,
and the selected-member executable is identical across first/final matching
implementations. Runtime ratios fluctuate around parity; no runtime optimization
benefit is claimed. Compiler `.text` grows from 1,495,558 to 1,506,694 bytes
(+11,136, about 0.74%); the ordering/environment repair adds only 320 bytes over
the first matching implementation.

PA15/O0 acceptance requires correctness, bounded work, complete keys, precise
ownership and explicit lifetimes. No numerical latency/RSS/text ceiling or
optional optimization is mandated here. The new selection work is required
semantic behavior, while repeated ordering and duplicate environments were
avoidable and have been removed with measured evidence. There is no inlining,
unrolling, instruction transform, code-growth search or new backend work to
justify. Native allocation/encoding, higher optimization policies and self-hosting
retain their later-stage ownership. Historical self-selected targets and all
prior measurements remain preserved; none weakens correctness or coverage.

## Implementation trace and handoff boundary

`Forward<int,int>::call` in the runtime control is parsed into the existing shared
source/semantic graph. The primary and canonical argument tuple identify the
class specialization. Its indexed candidate family selects `Forward<T,T>`;
the selected head binds T=int in one environment. Matching and ordering follow
N3485 14.5.5.1 [temp.class.spec.match] and 14.5.5.2 [temp.class.order] in
[the supplied draft](../doc/n3485.txt). The ordinary call demands that selected
member body once. Its recorded signature, body, conversions and ABI identity
flow directly into typed LowIR; text writing is the explicit PA15 adapter.
The supplied backend produces the measured ELF. This traces the current stage's
boundary and does not claim a student-native backend implementation.

Source regions are retained once; changed specialization occurrences use existing
immutable substitution frames and pack lanes. Published primary identity is not
replaced by the selected definition. Pair-ordering keys contain both stable
pattern identities; the cache belongs to the TU and survives semantically
identical renamed redeclarations. Candidate visits are confined to the primary's
family. Each completion uses two linear passes over matched candidates, and
ordering storage follows the actually compared pairs, not unrelated templates.
Local scratch is released at return; canonical records and frames release with
the frontend TU after typed lowering. No cloned semantic graph, rendered key,
textual recovery, global retry or process-global cache was added.

Required results are **169/177 PA15**, **1935/1935 prior**, file audit pass with
three inherited warnings. The 177-test fixture tree, earlier fixture trees,
reference bundle metadata, handout and comparison scripts are unchanged. Personal
matching controls include 15 native and seven rejection cases; inherited personal
suites also pass. The [plan](plan.md) records seven unfinished constant-object/
initialization/storage fixtures and one ordinary-body validation fixture. Those
owners need execution and demand facts that class-pattern matching cannot
supply. They remain implementation obligations, distinct from independent
whole-stage review. No reference correction or requirement waiver is used.
