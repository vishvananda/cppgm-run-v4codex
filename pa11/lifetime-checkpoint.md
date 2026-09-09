# PA11 lifetime checkpoint evidence

Final implementation: `3f590f18` (after `2f886c18` and `ae8255d4`). The stage-base and last-reviewed markers in [plan.md](plan.md) remain unchanged. PA11 is incomplete.

## Correctness and ownership

- `make test-pa11`: **173/302**, versus 156/302 at continuation entry. Exact fixture failure-set comparison: **17 fixed, zero regressions**, 146 → 129 failures; all four PA11 property controls pass. The original stage entry was 43/302. Coverage, fixtures, references and comparison rules are unchanged.
- `make test-report-through-pa10`: **1025/1025**. `perl scripts/cppgm_file_audit.pl --stage pa11 --paths dev/src`: exit 0; its Analyzer-header declaration-count advisory remains. All implementation sources are registered.
- `python3 student.tests/pa11/check.py`: five programs validate typed LowIR and execute with exit 0. New programs check preserved return values, reverse destruction, backward goto, break/for-initializer cleanup, nested and large arrays, member arrays, pseudo-destructors, temporary member-call lifetime and noinline constructors.
- Required stage command still exits 2; no later assignment is advanced. Ralph state was not modified.

Canonical class/member entities own destructor demand, exception specifications and ordered destruction actions. A deferred semantic traversal records immutable lexical lifetime tails and sparse statement uses, and resolves initialization barriers for goto/switch. Lowering interns cleanup continuations by state and terminal; return terminals also distinguish enclosing control context. Values are saved before converging on cleanup. Constructor subobjects and array prefixes acquire cleanup only after successful construction. Function-local temporary activation and destination recipes are released with the lowering function.

Lexical traversal, demand closure and lowering are O(source facts + required actions + produced IR), with no full-program cleanup rescans. Small arrays expand at most eight total elements; larger dimensions use counter loops. Each lexical object adds one state, and equal return suffixes share actions instead of cloning the complete sequence. Telemetry observes state/use/action pools and emitted instructions without initiating analysis.

## Frozen common-subset comparison

[Protocol](../student.tests/pa11/performance-protocol.md), [initial campaign](../student.tests/pa11/lifetime-common-performance.json), [final campaign](../student.tests/pa11/lifetime-final-common-performance.json), [targeted follow-up](../student.tests/pa11/lifetime-repeat-performance.json), and their harnesses retain every observation. A is cd054d80; final B is 3f590f18. Both are g++ GNU C++11/O3 builds with identical compiler flags, source hashes and backend, pinned to the same CPU. Four A/A calibrations precede two ABBA blocks. No builds or separate tests ran during timing.

Five compiler-output pairs are byte-identical; four differ in local slot spelling and pass the unchanged course comparator. All three executable pairs are byte-identical and return 0. Native compilation is outside execution timing.

| Compiler workload | A/B median seconds | A/B peak RSS KiB | Paired B/A ratios | A/A spread |
|---|---:|---:|---|---:|
| calls-1 | 0.39094/0.39735 | 73080/76704 | 1.0131, 1.0214 | 0.24% |
| memory-float-1 | 0.33835/0.34303 | 67088/67096 | 1.0152, 1.0082 | 0.61% |
| references-1 | 0.01614/0.01618 | 6456/6556 | 0.9831, 1.0167 | 2.88% |
| template-semantics-1 | 0.06652/0.06593 | 12356/12360 | 0.9913, 0.3220 | 2.61% |
| calls-4 | 1.58753/1.60975 | 281448/280504 | 0.9163, 1.0321 | 1.54% |
| memory-float-4 | 1.35641/1.44672 | 255560/255580 | 1.0974, 1.0388 | 1.92% |
| references-4 | 0.04844/0.04868 | 12868/12888 | 0.9972, 0.2415 | 2.72% |
| template-semantics-4 | 0.25680/0.25398 | 36908/36700 | 0.9835, 0.9892 | 4.39% |
| references-8000 | 0.11236/0.11338 | 24432/24492 | 1.9941, 1.0142 | 0.87% |

The large one-sided wall spikes in template-1, references-4 and references-8000 did not have corresponding user/system CPU increases. Their cause is not established. The memory-float-4 wall increase is also retained. Repeat only these groups with identical frozen inputs/binaries; no observations are removed:

| Follow-up | Paired B/A ratios | A/A spread |
|---|---|---:|
| template-semantics-1 | 0.9966, 1.0092 | 2.96% |
| references-4 | 1.0108, 0.9952 | 5.26% |
| references-8000 | 1.0136, 1.0278 | 0.98% |
| memory-float-4 | 1.0202, 1.0054 | 1.93% |

Compiler .text grows 597894 → 645382 bytes (+47488, 7.94%) for lifetime/array lowering and semantic metadata. Maximum common-subset peak RSS increase is 3624 KiB. The first complete campaign before the redeclaration fix is preserved separately; final B adds 192 text bytes to that candidate. Repeated ratios for the affected groups range 0.9952–1.0278; these are cost observations, not optimization gains.

| Executable | A/B median seconds | A/B text proxy bytes | Paired B/A ratios |
|---|---:|---:|---|
| calls-long | 0.47710/0.47761 | 206/206 | 0.9993, 1.0014 |
| memory-long | 0.27949/0.28023 | 434/434 | 1.0020, 1.0026 |
| floating-long | 0.33131/0.33141 | 230/230 | 1.0023, 1.0007 |

The sectionless native text proxy is payload after ELF entry on inputs without static data, not a .text-section measurement. Runtime inputs have volatile bounds and checked results. Short compiler cases below 20× startup are diagnostic only.

## Newly correct lifetime behavior

[Raw measurements](../student.tests/pa11/lifetime-behavior-performance.json) and [generator](../student.tests/pa11/lifetime_benchmark.py) use final B only: A omits destruction and is not a semantically equivalent baseline. Every generated LowIR validates, and every executable returns 0 before measurements.

| Workload | Median seconds | Peak RSS KiB | A/A spread | Lifetime states / uses | IR instructions |
|---|---:|---:|---:|---:|---:|
| lexical-1000 | 0.02969 | 9796 | 0.97% | 1000 / 1006 | 12049 |
| bodies-1000 | 0.20709 | 39912 | 1.19% | 2000 / 8000 | 56017 |
| lexical-4000 | 0.10235 | 25648 | 1.94% | 4000 / 4006 | 48049 |
| bodies-4000 | 0.84704 | 146072 | 1.01% | 8000 / 32000 | 224017 |
| array-12 | 0.00586 | 4696 | 188.81% | 1 / 2 | 57 |
| array-48000 | 0.00553 | 4476 | 5.22% | 1 / 2 | 57 |

Fourfold lexical object growth gives 3.45× latency and 2.62× RSS; fourfold independent-body growth gives 4.09× latency and 3.66× RSS. State/use and IR counts grow proportionally. Increasing array extent 4000-fold leaves 57 instructions and 11344 bytes of IR pool capacity unchanged. Final-B startup median is 0.00533 s. The lexical pair is also below the 20×-startup threshold, so its latency ratio is diagnostic; the independent-body pair exceeds that threshold at both sizes. Array timing is startup-dominated (including the preserved array-12 spike); only the exact work/IR bound is meaningful there.

The checked 48-million-iteration constructor/destructor runtime has median 0.31202 seconds, 212-byte text proxy, 256 KiB peak RSS and four exit-0 observations. No optional optimization or runtime speedup is claimed.

PA11 has no mandated numeric speed target. The inherited 1.10× latency, 1.20× RSS +16 MiB, +128 KiB text and 4×-input 5.5×-time/5×-RSS targets remain diagnostics under the stage-scoped spec rule. All measurements and misses are retained; correctness, coverage, proportional work and the explicit eight-element expansion budget remain required. The measured compiler growth supplies required semantics rather than an optional transform justified by an invented speedup.

## Concrete handoff boundary

The continuation extended scalar destruction through lexical/control exits, equal return/unwind suffixes, member/base actions, bounded arrays and partial construction, namespace/static lifetime, temporary member calls and inline policy. These fixtures and all PA11 controls now pass; the earlier constructor-only boundary is superseded.

Remaining related destructor failures require access-path/friend provenance and separate externally retained complete/base ABI entry identities, which the graph does not currently carry. Temporary functor failures require canonical operator identities and operator-call selection. Aggregate/zeroinit failures require typed union/volatile/layout facts; TLS needs per-thread storage/guard ownership. These are separate semantic expansions, not additional calls that can be safely appended to the completed cleanup paths. The compact owner/data-flow/complexity/validation ledger in [plan.md](plan.md) records the remaining groups. PA11 full-stage completion remains outstanding.
