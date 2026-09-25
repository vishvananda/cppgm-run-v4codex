# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `8dc4636d23a38f2bbcc8662b88b07f7979715c2d`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation handoff 75**.
Entry `9fa23653` → code `de0f5228`, `6229b49a`; independent review is pending.

## Design/spec alignment and current evidence

[Handoff 75](handoff75.md) completes immediate-context list/cast queries through
ordinary/fixed initialization, constant execution, noexcept and typed ABI.
Canonical query IDs/context/target types own formation and selected validation;
consumers reuse conversions and plans. No fake syntax, replay, stage switch or
host/reference implementation is introduced. Work follows initializer/candidate
edges; omitted array tails are compressed. Default construction failures are
structured facts. Cast adjustments and parameter representations are retained
before lowering consumes them; unused bodies remain dormant.

Required stage tests: **383/420**, entry **379/420**; original failures **41 → 37**,
no new failures. All **420 inputs and 1,712 fixture/reference files** are unchanged.
Earlier PAs: **2609/2609**. File audit passes with the same three inherited header
advisories. **718 personal semantic controls** pass (71 new, 647 inherited), plus
four repaired course executions, the cast reducer, six new graph controls,
three inherited completion controls, seven ABI checks and the direct ABI API.
Intermediate failures and repairs remain in the evidence; none is waived.

## Remaining required implementation

| Ownership group | Required next work / boundary |
|---|---|
| Declaration timing and lookup | Lazy nested classes, first-declaration result lookup, remaining member/alias syntax and visibility. Preserve the reviewed prototype/context/signature and completion edges. |
| Constructor and explicit deduction | Inherited-constructor and explicit member-template participation. This needs declaration/candidate forwarding ownership, separate from the completed selected-list query consumer. |
| Ordinary LowIR facts/policy | Constant/array initialization, bool/result metadata, class-result conventions and discarded loads. Earlier fixtures require pooling for the same small scalar-array shapes, so a blanket removal breaks prior stages. Reconcile the policy/contract without a stage switch or weakened comparison. Preserve the inherited nested-alias cast and class-ellipsis reducers as unfinished implementation. |

The scope was extended through related consumers and defects before this handoff.
The remaining groups require distinct declaration-state/candidate or lowering
policy work; they are not remaining steps in the selected-query recipe.
Independent audit questions concern source/query identity, validation, completion
edges and cache lifetimes across consumers. These differ from the unfinished
requirements above; neither category is waived. Consolidate further groups through
their consumers rather than splitting at already-known ownership boundaries.

## Performance and references

Acceptance is **PA18/O0 LowIR**, spec §9. [Performance 75](performance75.md) and
[observations](../student.tests/pa18/loop75-performance.json) preserve frozen A/A,
ABBA compiler latency/RSS, checked runtime/size, output equivalence and graph
scaling. New required behavior has final-only costs when the entry rejects it.
No optional transform or mandated numeric ceiling is introduced. Historical
**+15%, +16 MiB, 5.5×** targets remain diagnostics, not exit gates; preserve all
measurements in performance records 63–74, including unsuccessful attempts.
Correctness, coverage and graph bounds remain gates. Native optimization and
self-hosting belong to PA24–PA34; this does not excuse current course failures.

Reference corrections remain [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md), with reducers,
rule proofs and pinned bundles. Loops 70–75 change no references.

## Handoff ledger

Stage entry **266/420** → 63 **282** → 64 **312** → 65/66 **327** → 67 **343**
→ 68 **348** → 69/70 **353** → 71 **367** → 72 **372** → 73/74 **379** → 75 **383**.

| Handoff | Range / disposition |
|---|---|
| 66 | Stage base → `3a883d10`; [audit](audit66.md), 327/420; prior/file audit pass. |
| 70 | `3a883d10` → `15b34993` → fix `f59e8f67`; three handoffs reviewed/fixed; [audit](audit70.md), 353/420; prior/file audit pass. |
| 71 | `2eb83de5` → code `b6287928`; [context handoff](handoff71.md), 367/420; prior/file audit pass; reviewed in audit 74. |
| 72 | `ca314a72` → code `57ee1f2e`; [alias/pack handoff](handoff72.md), 372/420; prior/file audit pass; reviewed in audit 74. |
| 73 | `12cbfe83` → code `9ebc507c`; [callable handoff](handoff73.md), 379/420; 48 → 41 failures; prior/file audit pass; reviewed/corrected in audit 74. |
| 74 | `f59e8f67` → `2433d6de` → code `8dc4636d`; [audit](audit.md), all three handoffs/interactions reviewed, five ownership findings repaired; unchanged 41 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop74-evidence.json). |
| 75 | `9fa23653` → code `6229b49a`; [list/cast handoff](handoff75.md), 383/420; 41 → 37 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop75-evidence.json). Independent audit pending; full-stage remains unfinished. |

Required checks: `make test-pa18`, `make test-report-through-pa17`, PA18 file audit.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
