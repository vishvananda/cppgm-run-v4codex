# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `82fca940b1849d90deffbaba29ee162946f3e23c`.
Target: **PA18 full-stage**, unfinished. Phase: **checkpoint audit 78 complete**.
Reviewed `8dc4636d` → entry `0ed4fe5f` → code `82fca940`, every commit and the
combined changes across handoffs 75–77. [Audit](audit.md),
[evidence](../student.tests/pa18/loop78-evidence.json); previous audit in [74](audit74.md).

## Design/spec alignment and evidence

The list/cast, inherited-constructor and nested-class groups now have accumulated
review, including their shared demand, conversion, lifetime and emission paths.
Audit repairs preserve braced allocation queries and allocated-object lifetime;
keep declaration-only class parameters incomplete with locally updated cross-TU
signature views; and attach constexpr-created constructor actions' later emission
dependencies to the constructor itself. Canonical keys retain all context, parsed
regions remain shared, monotonic facts/worklists are local, and typed lowering
consumes selected actions without semantic reconstruction or grammar replay.

Validation at the reviewed tip: **388/420**, the exact same **32 entry failures**;
**2609/2609** earlier tests; file audit passes with three inherited header advisories.
All **420 inputs and 1,686 tracked fixture/reference files** are unchanged.
**875 inherited + 35 audit semantic + 12 multi-TU controls** pass, as do the ABI,
completion/scaling and combined source-to-ELF checks. All nine course paths repaired
across 75–77 retain checked execution. No comparison or reference rule changed.

## Remaining required implementation

| Ownership group | Required work / boundary |
|---|---|
| Source signatures and member/alias syntax | First-declaration function-result lookup; explicit member specialization deduction; pack-expanded explicit member-template arguments; alias/function argument cv syntax; using-directive function-template argument participation. These own five rejected cases. Retained source-expression ABI coverage remains part of the same signature obligation. |
| Ordinary LowIR facts/policy | One empty-pack unknown-bound-array rejection and 26 comparisons: constant/array and empty-tag initialization, constructor object-root metadata, bool/result metadata, class-result conventions and discarded loads. Earlier fixtures require pooling for the same small scalar-array shapes; reconcile policy without a stage switch or weakened comparison. Preserve the nested-alias cast and class-ellipsis reducers as unfinished implementation. |

All 32 failures remain requirements, including the inherited member-alias-pack
case whose execution succeeds but empty-tag/root-metadata comparison still fails.
This checkpoint audit does not certify the full stage. Future work should finish
an ownership group through its source, query, constant, exception, lifetime, ABI
and runtime consumers before handoff. Separating the previous three groups left
shared consumer defects and repeated evidence packaging; that fragmentation was
avoidable. No review finding is being substituted for an implementation waiver.

## Performance and references

Acceptance is **PA18/O0 LowIR**, spec §9. [Performance 78](performance78.md) records
frozen checkpoint and cumulative A/A/ABBA compiler latency/peak RSS, checked runtime
and generated size across the full handoff corpus, plus focused noise resolution.
The combined trace verifies retained fact identity and required executable work.
Newly accepted behavior has final-only costs when the earlier compiler rejects it.
There is no new optional optimization or mandated numeric ceiling. Historical
**+15%, +16 MiB, 5.5×** targets remain diagnostics, not exit gates; all measurements
from 63–77 and the audit's original observations remain. Correctness, coverage,
mandated limits and current-stage graph bounds remain gates. Native optimization
and self-hosting belong to PA24–PA34, and do not excuse these PA18 failures.

Reference corrections remain [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md), with reducers,
standard proofs and pinned bundle. Loops 70–78 change no references.

## Handoff ledger

Stage entry **266/420** → 63 **282** → 64 **312** → 65/66 **327** → 67 **343**
→ 68 **348** → 69/70 **353** → 71 **367** → 72 **372** → 73/74 **379** → 75 **383** → 76 **385** → 77 **388**.

| Handoff | Range / disposition |
|---|---|
| 66 | Stage base → `3a883d10`; [audit](audit66.md), 327/420; prior/file audit pass. |
| 70 | `3a883d10` → `15b34993` → fix `f59e8f67`; three handoffs reviewed/fixed; [audit](audit70.md), 353/420; prior/file audit pass. |
| 71 | `2eb83de5` → code `b6287928`; [context handoff](handoff71.md), 367/420; prior/file audit pass; reviewed in audit 74. |
| 72 | `ca314a72` → code `57ee1f2e`; [alias/pack handoff](handoff72.md), 372/420; prior/file audit pass; reviewed in audit 74. |
| 73 | `12cbfe83` → code `9ebc507c`; [callable handoff](handoff73.md), 379/420; 48 → 41 failures; prior/file audit pass; reviewed/corrected in audit 74. |
| 74 | `f59e8f67` → `2433d6de` → code `8dc4636d`; [audit](audit74.md), all three handoffs/interactions reviewed, five ownership findings repaired; unchanged 41 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop74-evidence.json). |
| 75 | `9fa23653` → code `6229b49a`; [list/cast handoff](handoff75.md), 383/420; 41 → 37 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop75-evidence.json). Reviewed and corrected in audit 78; full-stage remains unfinished. |
| 76 | `b87de70e` → code `e052e939`, `bff709db`, `c8a2aad8`; [inherited forwarding handoff](handoff76.md), 385/420; 37 → 35 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop76-evidence.json). Reviewed and corrected in audit 78; full-stage remains unfinished. |
| 77 | `2ce99c6a` → code `86e142ba`, `b6294394`, `975e6162`; [nested completion/lookup handoff](handoff77.md), 388/420; 35 → 32 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop77-evidence.json). Reviewed and corrected in audit 78; full-stage unfinished. |
| 78 | `8dc4636d` → `0ed4fe5f` → code `82fca940`; [accumulated audit](audit.md), all three handoffs and interactions reviewed; allocation-query, declaration/representation and constexpr-emission owners repaired; unchanged 32 failures, prior 2609/2609, file audit/coverage pass, 922 controls and frozen performance accepted; [evidence](../student.tests/pa18/loop78-evidence.json). |

Required checks: `make test-pa18`, `make test-report-through-pa17`, PA18 file audit.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
