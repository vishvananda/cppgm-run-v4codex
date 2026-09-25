# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `f59e8f67cd8c832361130aef9af1a0337b25c45d`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation handoff 73**.
Previous goal turn: **progress**, revalidated clean entry `12cbfe83` and baseline.
Loop 73: **progress**, entry `12cbfe83` → implementation `9ebc507c`.
[Handoff 73](handoff73.md) records semantic owners, data flow, bounds, checks and
the boundary to declaration timing and initialization/LowIR work. Review markers
above remain unchanged; this handoff is not independent certification.

Stage entry **266/420** → handoffs 63 **282**, 64 **312**, 65 **327**;
audit 66 **327** → handoffs 67 **343**, 68 **348**, 69 **353** → audit 70
**353** → handoffs 71 **367**, 72 **372**, 73 **379/420**.
Loop 73 repairs seven original failures, adding none: **41 failures** remain
(16 status, 25 LowIR). All 420 inputs/references/comparison rules are retained.
Earlier PAs: **2609/2609**. File audit passes with the same three inherited
header advisories. Audits [70](audit.md) and [66](audit66.md) remain intact.

| Ownership group | State and required next work |
|---|---|
| Reviewed: deduction, ordering, conversion, query demand, address NTTPs, outer defaults, braced/base/array deduction | Preserve canonical identities, immutable frames, completion edges, conversions, address demand and linkage. Handoffs [67](handoff67.md), [68](handoff68.md) and their interactions are reviewed. |
| Reviewed: assignment/destructor queries and ordinary lowering | [Handoff 69](handoff69.md), reviewed/corrected by audit 70: volatile references/builtin ranking, receiver effects, arrow validity and destructor facts. |
| Implemented, review pending: context/signature identity | [Handoff 71](handoff71.md): lexical/concrete frames, query expansion, decltype bases, structural signature keys and renamed definition attachment. |
| Implemented, review pending: alias formation/correlated packs | [Handoff 72](handoff72.md): erased/default obligations, captures, transparent source/concrete types, argument roles, access, casts and function-type arguments. |
| Implemented, review pending: callable/prototype facts | Handoff 73: shared invocation selection, actual callee edges, projected source recipes, constexpr function decay, immediate conversion failures, surrogate exception effects, prototype `this`/member calls, using exposure/hiding and ABI identity. 587 cumulative semantic, seven repaired-course, three completion and four ABI controls pass. |
| **Remaining: declaration timing and lookup** | Lazy nested classes, first-declaration result lookup, remaining member/alias syntax and lookup. Prototype `this` and refreshed member-call overload sets are now repaired. |
| **Remaining: constructor/explicit deduction and initialization queries** | Constructor/inherited-constructor and explicit member-template participation; braced initialization/narrowing, cast/access validity. The dependent tag invocation needs a braced-value initialization query. |
| **Remaining: ordinary LowIR facts/policy** | Constant/array initialization, bool/result metadata, class-result conventions and discarded loads. Preserve the earlier nested-alias cast comparison and the personal class-ellipsis reducer as unfinished implementation. |

These remaining groups are required implementation, not waived audit questions.
Loop 73 expanded from invocation through shared ordinary/constant execution,
access/deletion, prototypes, inherited exposure/ranking/hiding and ABI. Remaining
nearby failures now require declaration timing, constructor/list-initialization
facts or ordinary LowIR policy; see the concrete boundary in the handoff.

Performance acceptance is **PA18/O0 LowIR**, spec §9.
[Performance 73](performance73.md) records frozen A/A/ABBA latency/RSS, checked
native runtime/size and graph work. Equivalent outputs are byte-identical;
common compiler medians are near parity. Hiding halves candidate work and has
2.4–2.8% lower paired compiler medians. Compiler text grows **8192 bytes (0.435%)**.
Fourfold affected source growth produces approximately fourfold work/time.
No optional optimization or mandated numerical ceiling is introduced.
Historical **+15%, +16 MiB, 5.5×** targets remain diagnostics. Preserve evidence
[63](performance.md), [64](performance64.md), [65](performance65.md),
[66](performance66.md), [67](performance67.md), [68](performance68.md),
[69](performance69.md), [70](performance70.md), [71](performance71.md),
[72](performance72.md), including unsuccessful initial observations.
Correctness, coverage and graph bounds remain gates; native optimization and
self-hosting belong to PA24–PA34.

Reference corrections remain [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md), with reducers,
rule proofs and pinned bundles. Loops 70–73 add none.

| Handoff ledger | Reviewed range / disposition |
|---|---|
| 66 | Stage base → `3a883d10`; 327/420, prior/file audit pass; [audit](audit66.md). |
| 70 | `3a883d10` → entry `15b34993` → fix `f59e8f67`; prior three handoffs reviewed/fixed; 353/420; [evidence](../student.tests/pa18/loop70-evidence.json). |
| 71 | Entry `2eb83de5` → code `b6287928`; 367/420; prior/file audit pass; [evidence](../student.tests/pa18/loop71-evidence.json). Review pending for context/signature/body/access identity and bounds. |
| 72 | Entry `ca314a72` → code `57ee1f2e`; 372/420; prior/file audit pass; [evidence](../student.tests/pa18/loop72-evidence.json). Review pending for alias/capture keys, frames, transparent signatures/ABI, argument roles and failure boundaries. |
| 73 | Entry `12cbfe83` → code `9ebc507c`; 379/420; failures 48 → 41, no new failures; prior/file audit pass; [evidence](../student.tests/pa18/loop73-evidence.json). Review pending for callable/prototype keys, receiver projection, conversion failure boundaries, using shapes/access, source ABI and bounds. These questions do not replace unfinished implementation. |

Required checks: `make test-pa18`, `make test-report-through-pa17`, PA18 file audit.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
