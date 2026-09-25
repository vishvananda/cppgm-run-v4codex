# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `ecc308bc5ee33ed40fa773f7981961f3018a5867`.
Target: **PA18 full-stage**, unfinished. Phase: **checkpoint audit 82 complete**.
Entry `85ea42c0`: **396/420** → **396/420**, the **same 24 failure paths**.
The [accumulated audit](audit.md) reviews every commit from `82fca940` through
entry and the validated audit fix `ecc308bc`; [evidence](../student.tests/pa18/loop82-evidence.json).
The previous goal turn supplied committed implementation/evidence (progress);
the entry process check found no inherited compiler or test process to resume.

## Reviewed ownership and validation

The combined review covers signatures/lookup/ABI (79), scalar casts and reference
materialization (80), and bounded runtime conversion summaries/emission (81).
Audit fixes put conversion-name lookup in the shared resolver, publish retained
conversion targets, and project a query's naming class and selected specialization
through its substitution frame. Fixed member queries consume source base edges
without demanding a layout or body. Parser angle bindings reuse parser-owned
scratch bounded by nesting depth.

**1164** explicit controls pass (**1137 inherited + 27 audit**); the new controls
improve **13/27 → 27/27**. Four source-to-native traces, ten summary/emission
inspection programs, inherited ABI/scaling and completion checks pass. Earlier
PAs are **2609/2609**; file audit passes with the same three header advisories.
All **420** course sources and **1686** fixture/reference files are unchanged
from audit entry. Across the full review range, the only oracle change is the
proved [reference correction 79](reference-correction79.md); comparison rules
and required behavior/coverage remain intact.

## Remaining implementation

| Owner | Broad work and boundary |
|---|---|
| Array/aggregate initialization and constant materialization | Unknown-bound arrays after empty expansion; shared array/string/constant representation and constexpr union initialization. Reconcile earlier pooled shapes with PA18 stores through ordinary semantic/materialization policy. |
| Object ABI, emission and scalar representation | Empty-tag construction and root facts, class-result conventions, static member publication, discarded loads and arithmetic widening. These two groups own the **24 remaining course failures**. The inherited class-ellipsis reducer is still unfinished. |

The signature, conversion and named-result groups have now been independently
reviewed together. They cannot supply array images, class-result ABI or storage
definitions. Keep the remaining work at those owners; extending the scalar leaf
summary into an arbitrary evaluator would not resolve it. The nested-alias cast
executes correctly; its array-materialization comparison is still required.

Handoffs 79–81 split ordinary conversion names from retained-template/query
consumers and required repeated packaging and validation. That fragmentation
was avoidable. Carry each remaining ownership group through source facts,
constant/query consumers, lowering, emission and execution before handing it off.

## Performance acceptance

Acceptance is **PA18/O0 LowIR**, spec §9. [Performance 82](performance82.md)
compares the previous reviewed compiler with the frozen audited compiler over
all **33** handoff workloads and **seven** audit probes, with A/A calibration,
four ABBA blocks, separately checked runtime/size and compiler latency/RSS.
Historical measurements and noisy observations remain preserved.

The named-constant rule retains its one-requested-body/eight-wrapper proof
budget, one result fact, no executable growth and ordinary-call fallback.
Source/query repairs use canonical declaration, target, naming-scope and frame
identities; they add required semantic work, not an optional optimization pass.
PA18 has no mandated numerical latency/RSS ceiling. Historical **+15%, +16 MiB,
5.5×** diagnostics remain non-gates. Correctness, coverage, mandated limits,
bounded O0 work and demonstrated optimization profitability remain requirements.
Own native optimization, debug encoding and self-hosting belong to PA24–PA34.

## Checkpoint ledger

Stage entry **266/420**; earlier checkpoints and their complete ledgers are
preserved in [audit66](audit66.md), [audit70](audit70.md), [audit74](audit74.md)
and [audit78](audit78.md).

| Checkpoint | Range / disposition |
|---|---|
| 78 | `8dc4636d` → `0ed4fe5f` → `82fca940`; accumulated audit 75–77, **388/420**, 32 failures; prior/file/coverage and 922 controls pass. |
| 79 | `ef0e43c0` → `50407fc1`; [signature handoff](handoff79.md), **393/420**, 27 failures; one proved reference correction; reviewed in audit 82. |
| 80 | `c5e2c271` → `ce7d3e7f`; [scalar/reference handoff](handoff80.md), **395/420**, 25 failures; reviewed in audit 82. |
| 81 | `69247877` → `25b89fc2`; [named-result handoff](handoff81.md), **396/420**, 24 failures; reviewed and corrected in audit 82. |
| 82, accumulated audit | `82fca940` → entry `85ea42c0` → code `ecc308bc`; all three handoffs and interactions reviewed, shared conversion/query ownership and parser scratch repaired; **396/420**, identical 24 failures; prior **2609/2609**, file/coverage pass, **1164** controls and stage-scoped performance evidence. Full-stage remains unfinished. |

Required checks: `make test-pa18`, `make test-report-through-pa17`,
`perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src`.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
