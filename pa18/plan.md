# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `ecc308bc5ee33ed40fa773f7981961f3018a5867`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation handoff 85**.
Entry `f953e42a`: **414/420**; current **417/420**, **6 → 3 failures**.
The previous goal turn supplied committed implementation/audit evidence (progress).
Entry inspection found no inherited live compiler/test process to resume.

## Design/spec alignment

[Handoff 85](handoff85.md) completes discarded-expression demand and validates
static-storage publication. Selected built-in source forms are retained in one
packed semantic expression bit. Immediate child facts compose comma/conditional
forms in O(1); lowering consumes that bit and value category. It no longer rebuilds
a recursive syntax-classification cache. Expression records remain 36 bytes.

Volatile class discards retain a checked copy recipe and a concrete NodeId-owned
materialization/lifetime only when evaluated. Fixed template uses share selection;
dependent queries return compact failure for unavailable copies without demanding
bodies. Functional `void(expr)` queries use the same conversion rules. Normal and
exceptional cleanup consume ordinary selected constructor/destructor facts.
Storage, query, source-form and lifetime owners remain separate; no grammar replay,
textual semantic keys, unrelated initializer/body demand or global retry is added.

[Reference proof 85](reference-correction85.md) corrects two dormant static
member definitions and one forbidden discarded reference-call load. Its independent
transformer reads entry oracles only. Pinned-bundle observations, standard anchors
and old/new hashes are retained. All other oracle bytes, **420** course inputs,
**1686** fixture paths, status sidecars and comparison rules are preserved.
[Handoff 83](handoff83.md) and [handoff 84](handoff84.md) remain intact.

## Validation and performance evidence

The [loop85 manifest](../student.tests/pa18/loop85-evidence.json) binds final required
checks, **1355** explicitly run controls (**1279 inherited + 60 discard + 16 storage**),
16 new volatile-access inspections, 15 fixed-recipe scaling checks, inherited ABI/scaling/summary checks, seven
source-to-native traces and all three corrected-oracle validations. The frozen
entry passes 31/60 new discard controls and 9/16 inspections; final results pass all.
Earlier PAs pass **2609/2609** and file audit passes. No course failure is added.

[Performance 85](performance85.md) records frozen binaries/inputs, A/A calibration,
four ABBA blocks, compiler latency/peak RSS and checked executable runtime/payload
size. Incorrect entry class-copy behavior receives final-only costs. Template
scaling checks one shared selection, materialization counts and linear emitted work.
The initial measurements exposed repeated source-fixed copy selection; that defect
was corrected, and both complete measurement sets are preserved.

Acceptance is **PA18/O0 LowIR**, spec §9. Source-form checks, required volatile
reads and legal class copies/lifetimes are necessary semantics. No optional pass,
code cloning or code-growth budget is introduced. Existing initialization and
summary budgets remain. PA18 has no mandated numeric compiler latency/RSS ceiling;
historical +15%, +16 MiB and 5.5× targets remain diagnostics. Correctness, coverage,
bounded work and optional-transform profitability remain requirements. Native
optimization, source exceptions, debug and self-hosting remain later-stage work.

## Remaining implementation and independent review

**Implementation:** three original class-result ABI mismatches remain: friend
alias result, conversion-template object result and dependent defaulted result
type. The inherited class-ellipsis reducer also remains unfinished. Calls,
definitions and indirect signatures need a coherent result convention; class
ellipsis needs a representation across LowIR's scalar variadic boundary. Reduced
bundle observations do not yet establish a uniform canonical type rule for the
ABI differences. The completed discard/static facts cannot make those decisions.
This is a coherent incomplete handoff, not a waiver or advancement to PA19.

**Independent review:** source-form fidelity through overloads and fixed/dependent
queries; complete contextual keys and recipe reuse; one selected volatile copy
and temporary; default-argument and normal/exceptional cleanup; the dormant-static
and discarded-reference proofs. [Audit 82](audit.md) remains the last review.
Handoffs 83–85 are unaudited; this third accepted implementation handoff preserves
the accumulated-review boundary. Passing controls do not waive those questions.
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
| 82 | `82fca940` → entry `85ea42c0` → code `ecc308bc`; accumulated audit, **396/420**, 24 failures; earlier **2609/2609**, file/coverage and **1164** controls pass. |
| 83 | Entry `48c864ab` → code `ac354fad`; arrays/query/declaration repairs, **411/420**, nine failures; 17 proved oracle corrections. Earlier **2609/2609**, file/coverage and **1225** controls pass; independent review pending. |
| 84 | Entry `09a77fca` → code `b4d66361`, proof `7a7ce959`; constructor/empty-value repairs, **414/420**, six failures; 51 new controls, 21 inspections, three proved oracle revisions. Earlier **2609/2609**, file/coverage pass; independent review pending. |
| 85 | Entry `f953e42a` → code `53252879`, extensions `f1fae6ad`/`076eccdd`, proof `37c7c832`; demand/discard group, **417/420**, three failures; 76 new controls and 16 inspections, three proved oracle revisions. Earlier **2609/2609**, file/coverage pass; independent review pending. |

Required commands: `make test-pa18`, `make test-report-through-pa17`,
`perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src`.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes and the
whole-stage independent audit is resolved.**
