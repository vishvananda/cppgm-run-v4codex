# PA17 compact plan — implementation handoff, loop 53

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `c43e8eb68db7e9b3f1dd0bbb18f14c92e4fc4b30`

Target: **PA17 full-stage**, still incomplete. Entry `65b02f17` was clean at
306/343 (37 failures). Code `0a720134` passes **315/343**: nine original failures
resolved, no new failures, all 343 course inputs unchanged. PA1–PA16 pass
**2266/2266**; through PA17 is **2581/2609**. File audit passes with the same
three inherited header-division warnings. **344/344 personal controls** pass
(313 inherited, 31 new); entry fails 19 of the new controls. No reference,
fixture, comparison-rule or bundle correction was used.

| Completed owner / data flow | Design, bound and validation |
|---|---|
| Class/variable partial declarations → canonical arguments → selected pattern | Shared exact cv/value/pack matching and ordering; primary-indexed candidates, immutable pair cache, O(C) winner comparisons. Variable duplicate/default/nondeducible/equivalent-primary checks occur at declaration. All seven previous selection cases and identity/order/rejection controls pass. |
| Symbolic pack queries and alias/template arguments → parent-linked substitution frames | Preserve symbolic sizeof-pack cardinality, rename its parameter identity, expand argument lists at their owner, and distinguish a member alias entity from applying its body. Missing dependent alias members return an empty candidate-probe result. Empty/value/repeated packs and conditional partial matches execute. |
| Out-of-class member partial → selected head and enclosing lexical environment → ordinary demand | Partial heads register separately from the primary. Source outer aliases survive member rebinding; selected class environments retain the defining lexical parent. Renamed types, constants and member bodies execute; duplicate definitions reject. |
| Qualified callable lookup → candidate family/query → concrete alias | Qualified lookup completes alias target classes. A concrete overload family does not inherit candidate-signature dependence; explicit qualified/member arguments and ordinary function decltype are retained. Static-member non-type query control passes. |
| Recorded boolean constant → typed LowIR condition | O(1) direct jump selection without evaluating a new expression, cloning syntax or scanning IR. Required constant-member comparisons pass; ordinary expression, volatile and cleanup paths remain subject to existing checks. |

[Performance evidence](selection-performance.md) contains frozen A/B compilation
latency/RSS, A/A and ABBA observations, checked runtime/text, scaling and the
[source-to-LowIR-to-ELF trace](../student.tests/pa17/selection-trace.json).
Common compiler paired changes range −1.87% to +1.54%; peak RSS changes range
−76 to +208 KiB. New workloads scale 3.83–4.30× for 4× input. Compiler text grows
3,008 bytes (0.170%). Required boolean lowering saves 12 native bytes but its
runtime rises 2.66%; this cost is disclosed, with no speedup claim. PA17/O0 has
no mandated numerical ceiling; inherited +15%, +16 MiB and 5.5× targets remain
diagnostic under spec.md §9. All historical measurements, existing evaluator
limits and lowering work/growth fallbacks remain intact.

| Remaining implementation group | Cases | Next owning work |
|---|---:|---|
| Qualified lookup, candidate queries and remaining syntax | 12 | Recursive/ambiguous candidate query states, current-instantiation inherited/local type lookup, anonymous injection, array casts, explicit specifiers, lambda/qualified-pack calls. |
| Required LowIR storage, initialization, transfer and cleanup | 16 | Static member/local storage, alignment, construction/assignment, destruction and emission order. The local-static fixture now compiles but still differs in required LowIR. |

All **28 failures are unfinished implementation**, not independent-review
questions. Exact cases, current diagnostics and ownership/data flow are in the
[handoff ledger](../student.tests/pa17/selection-handoff.json). Do not advance to
PA18. Handoff boundary: the seven-case selection/argument group is closed,
extended through qualified callee queries, source declaration diagnostics,
renamed member bodies and required condition lowering. Further failures require
candidate failure/recursion scheduling, source current-instantiation/inherited
lookup or storage/lifetime lowering changes; they no longer arise from this
owner's argument matching or substitution. Extending those distinct owners
without their own semantic analysis and controls would not be a coherent
continuation of the completed group.

Independent review remains required for this increment and its interactions:
canonical partial/query keys and negative-result validity; retained member-head
and lexical-owner publication; and constant-fact lowering/performance costs.
These questions are separate from the 28 implementation failures and waive none.
The prior [checkpoint audit](audit.md), its markers and all historical handoffs
remain preserved; this turn does not certify the full assignment.

| Loop / phase | Handoff ledger |
|---|---|
| 52 / checkpointAudit | Progress: accumulated review plus ownership fixes complete through `c43e8eb6`; 306/343 and 313 controls. Review findings and measurements remain preserved. |
| 53 / implement | Previous turn classified as progress from verified audit findings. `0a720134` closes and extends partial/argument ownership: 306→315, 344 controls, prior/file checks pass, stage-scoped performance recorded. Remaining implementation: 28 cases; independent review pending. |

Run `python3 student.tests/pa17/verify_selection.py` to verify this handoff.
The records commit changes only evidence and this plan, leaving the reviewed
marker unchanged. Historical verifiers describe their own frozen code tips.
