# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `ecc308bc5ee33ed40fa773f7981961f3018a5867`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation handoff 84**.
Entry `09a77fca`: **411/420**; current **414/420**, **9 → 6 failures**.
The previous goal turn supplied committed implementation/audit evidence (progress).
Entry inspection found no inherited live compiler/test process to resume.

## Design/spec alignment and completed group

[Handoff 84](handoff84.md) completes constructor-entry propagation, empty aggregate
helper omission and empty-value copy/lifetime legality. Parsed constructor actions
retain a selected delegation edge; a deduplicated worklist forwards actual base,
complete and polymorphic entry facts, each bit at most once per edge. Lowering
consumes those facts and canonical local-type identity. No syntax replay, name
recovery, global retries or unrelated body demand is added.

Empty aggregate plans keep distinct storage and emit no empty helper. Empty
layout no longer bypasses declared copies, deletion/access, volatile binding or
nontrivial destruction. Those cases use ordinary selected conversions and typed
lifetime/ABI materialization. Arithmetic lowering consumes recorded O0 widening
policy for multiplicative/bitwise operands. Existing initialization and summary
budgets remain. [Handoff 83](handoff83.md)'s array/query/static-bound repairs and
[reference proof 83](reference-correction83.md) remain intact.

[Reference proof 84](reference-correction84.md) adds five required `zeroinit`
instructions to three PA18 oracles, using PA11's inherited exact-span requirement
and C++11 initialization rules. The transformer reads only entry references and
preserves all existing bytes. No sources, fixture paths or comparison rules change.
A trial omitting empty-value zeroing was removed; earlier oracles remain unchanged.

## Validation and performance evidence

The [loop84 manifest](../student.tests/pa18/loop84-evidence.json) binds the final
stage/prior/file checks, **1279** explicitly run controls (**1225 + 51 new + three
repaired-course executions**), 21 new inspections, inherited ABI/scaling/summary
checks, six source-to-native traces and all corrected-oracle executions. Coverage
remains **420** sources and **1686** PA18 fixture files; no new course failure.

[Performance 84](performance84.md) retains both frozen-binary runs, including noisy
initial observations. A/A calibration and four ABBA blocks measure affected and
scaled constructors/empty aggregates plus unchanged ordering, loops, calls, memory
and floating work. Compiler latency/RSS and checked native runtime/payload size
are reported together. Invalid baseline lifetime lowering gets final-only costs.

Acceptance is **PA18/O0 LowIR**, spec §9. Entry propagation and legal value
materialization are necessary semantic work. Omission of empty helper actions is
permitted by PA11: one action-head check, zero code-growth allowance. No new pass,
code cloning or optional transformation budget is added. PA18 has no mandated
numeric compiler latency/RSS ceiling; historical +15%, +16 MiB and 5.5× targets
remain diagnostics. Correctness, coverage, bounded work and optional-transform
profitability remain requirements. Native optimization/debug/self-hosting remain
PA24–PA34 work.

## Remaining implementation and independent review

**Implementation:** six original failures remain: class-result ABI (three),
static-member storage publication (two), and discarded reference-result output
(one). The inherited class-ellipsis reducer is still unfinished. These require
consistent result conventions across calls/signatures, storage/odr-use demand,
discarded-value consumption and scalar-only variadic representation respectively.
Constructor entry/empty-copy facts cannot decide those separate boundaries. This
is a coherent implementation handoff, not a waiver or an advancement to PA19.

**Independent review:** delegation-entry inputs/propagation, empty-transfer
eligibility and retained recipes, local-specialization roots, zero-init proof and
scalar widening policy. The [accumulated audit 82](audit.md) remains the last
review. Handoffs 83–84 are unaudited; passing controls do not waive review questions.
Stage-base and last-reviewed markers above are preserved.

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

| 84 | Entry `09a77fca` → code `b4d66361`, oracle proof `7a7ce959`; [constructor/empty-value handoff](handoff84.md), **414/420**, six failures; 51 new controls and 21 inspections. Earlier **2609/2609**, file audit and coverage pass. Three contract-proved oracle revisions; independent review pending. |

Required commands: `make test-pa18`, `make test-report-through-pa17`,
`perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src`.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes and the
whole-stage independent audit is resolved.**
