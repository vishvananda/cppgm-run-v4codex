# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `ecc308bc5ee33ed40fa773f7981961f3018a5867`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation 84**.
Entry `09a77fca`: **411/420**, nine failures. Review markers above are preserved.
The previous goal turn supplied committed implementation/audit evidence (progress).
Entry inspection found no inherited live compiler/test process to resume.

## Active work (84)

Group the nine remaining failures by semantic ownership before editing:
class value boundaries (three result ABI mismatches), constructor/action and
root facts (empty objects), static-member definition demand (two declarations
without storage), and scalar/discarded-expression consumption. Trace selected
semantic facts into typed lowering; use indexed entity/type facts, with work
proportional to demanded declarations, actions and emitted IR. Validate reduced
source/native controls and the unchanged course suite, then extend related
consumers while these ownership traces remain useful. Reference discrepancies
require independent reduced proof from the standard/contract. Freeze entry/final
binaries for A/A and ABBA latency/RSS and checked runtime/size evidence. Record
implementation boundaries separately from independent review questions.

## Design/spec alignment and completed group

[Handoff 83](handoff83.md) completes array initialization through semantic plans,
source-template checks, substituted queries, constant evaluation and typed LowIR
materialization. Bounds count actual elements after brace elision; braced and
parenthesized strings retain their terminator. Empty expansions reject. Fixed
narrowing obligations survive pack expansion; non-consuming aggregate clauses
reject. Root initialization retains C++11's braces/string/value-init rules.

Source declarations keep spelled signatures; completed entity types supply
sizeof/decltype and deduction. Static array definitions match indexed member and
element identities, checking supplied bounds. Queries consume the instantiated
entity/frame facts without replaying syntax or retrying unrelated candidates.
Constant plans now uniformly follow PA16's scalar-array copy rule. Removed
small-wide and construction exclusions also remove a whole-initializer scan.
Work follows explicit actions and emitted data; existing eight-lane omitted-
element limits and compact zero-data fallback remain.

[Reference proof 83](reference-correction83.md) justifies **17 revisions**:
15 array oracles (13 PA18, two PA17), one constexpr-union static initializer,
and one rejection for an empty unknown-bound array. The independent transformer
extracts entry values/offsets and retains later instructions. Sources, fixture
paths, coverage and comparison rules are unchanged; bundle revision is recorded.

## Validation and performance evidence

Final acceptance is bound to [loop83 evidence](../student.tests/pa18/loop83-evidence.json).
Required stage/prior/file checks, **1225** cumulative controls (**1164 + 61**), seven array-storage checks,
inherited ABI/scaling/summary inspection, source-to-native traces, PA16 array
controls and all corrected success oracles pass their recorded expectations.
The full PA18 suite remains required: **420** inputs, **1686** fixture files.

[Performance 83](performance83.md) uses frozen entry/final binaries, A/A noise
calibration and four ABBA blocks over affected/scaled arrays and unchanged
ordering, loops, calls, memory and floating work. Compiler latency/peak RSS and
checked native runtime/payload size are reported together. Incorrect/rejected
baseline programs receive final-only costs. Historical observations remain.

Acceptance is **PA18/O0 LowIR**, spec §9. Required array representation and bound
completion are semantic costs, not optional transformations. No new optimization
pass, code cloning or growth allowance was added. Existing named-result summary
budgets and performance evidence remain intact. PA18 mandates no numeric compiler
latency/RSS ceiling; historical +15%, +16 MiB and 5.5× targets remain diagnostics.
Correctness, coverage, mandated limits, bounded work and optional-transform
profitability remain requirements. Own native optimization/debug/self-hosting
belong to PA24–PA34.

## Remaining implementation and independent review

**Implementation:** nine original course failures remain at object/ABI/emission
owners: empty-object construction/root facts, class-result ABI, static-member
storage publication, scalar widening and discarded-value representation. The
inherited class-ellipsis reducer is still unfinished. None is an array
initialization failure. Array plans cannot supply these calling-convention,
root-emission or storage-demand facts; further fixes need those owners' own
end-to-end traces. This is the handoff boundary, not a waiver of required behavior.

**Independent review:** source bound completion versus retained signatures,
query/frame identity, static-array bound compatibility and constant-materialization
proofs need review. Passing implementation controls do not waive these questions.
Review markers above are preserved. The [accumulated audit 82](audit.md) remains
the last independent review; handoff 83 has not been independently audited.

## Handoff ledger

Stage entry **266/420**. Older ledgers remain in [audit66](audit66.md),
[audit70](audit70.md), [audit74](audit74.md) and [audit78](audit78.md).

| Checkpoint | Range / disposition |
|---|---|
| 78 | `8dc4636d` → `82fca940`; accumulated audit 75–77, **388/420**, 32 failures; 922 controls; prior/file/coverage pass. |
| 79 | `ef0e43c0` → `50407fc1`; [signature handoff](handoff79.md), **393/420**, 27 failures; one proved correction; reviewed in 82. |
| 80 | `c5e2c271` → `ce7d3e7f`; [scalar/reference handoff](handoff80.md), **395/420**, 25 failures; reviewed in 82. |
| 81 | `69247877` → `25b89fc2`; [named-result handoff](handoff81.md), **396/420**, 24 failures; reviewed/corrected in 82. |
| 82 | `82fca940` → entry `85ea42c0` → code `ecc308bc`; accumulated audit, **396/420**, same 24 failures; earlier **2609/2609**, file/coverage and **1164** controls pass. |
| 83 | Entry `48c864ab` → code `ac354fad`; arrays/constant materialization and connected query/declaration checks, **411/420**, nine failures; 17 proved oracle corrections. Earlier **2609/2609**, file audit and **1225** controls pass; performance/corrected-oracle evidence linked above. Independent review pending. |

Required commands: `make test-pa18`, `make test-report-through-pa17`,
`perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src`.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes and the
whole-stage independent audit is resolved.**
