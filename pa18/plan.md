# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `8dc4636d23a38f2bbcc8662b88b07f7979715c2d`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation 75**.
Entry HEAD: `9fa236532563035e24552fd7c4be583f8dc74322`; 379/420, 41 failures.
Previous goal turn completed audit evidence (progress); no live build remains.
Current group: immediate-context initialization/cast queries. Retained query IDs
and canonical target types feed candidate conversions, narrowing/access checks,
constant execution and noexcept without reparsing or fake syntax. Query/list
owners retain per-key success/failure and selected recipes; work follows actual
initializer edges and candidates, with compressed omitted array tails. Validate
required failures, nearby personal controls, prior suites and graph scaling;
freeze entry/final compiler and checked executable measurements under spec §9.
Implementation now repairs four original failures (383/420). The group includes
ordinary/fixed narrowing after user conversions, default-constructor validity,
string aggregate plans, cast adjustments, ABI/value/effect consumers and a
representation-completion lifetime repair found by full validation. Seventy-one
new semantic controls pass; final prior report/performance evidence are pending.
Array pooling/result-policy mismatches remain separate implementation work:
earlier fixtures require pooling for the same small scalar-array shapes. No
stage-specific lowering switch or weakened comparison is authorized.
[Audit 74](audit.md) reviews all nine accumulated commits from `f59e8f67`
through entry `2433d6de`, plus fixes `f4b9020b` and `8dc4636d`.
The checkpoint preservation gate passes; full-stage advancement remains pending.

Stage entry **266/420** → handoffs 63 **282**, 64 **312**, 65 **327**;
audit 66 **327** → handoffs 67 **343**, 68 **348**, 69 **353** → audit 70
**353** → handoffs 71 **367**, 72 **372**, 73 **379/420**.
Audit 74 retains **379/420** with exactly entry’s **41 failures**
(16 status, 25 LowIR). All 420 inputs/references/comparison rules are retained.
Earlier PAs: **2609/2609**. File audit passes with the same three inherited
header advisories. Audits [70](audit70.md) and [66](audit66.md) remain intact.

| Ownership group | State and required next work |
|---|---|
| Reviewed: deduction, ordering, conversion, query demand, address NTTPs, outer defaults, braced/base/array deduction | Preserve canonical identities, immutable frames, completion edges, conversions, address demand and linkage. Handoffs [67](handoff67.md), [68](handoff68.md) and their interactions are reviewed. |
| Reviewed: assignment/destructor queries and ordinary lowering | [Handoff 69](handoff69.md), reviewed/corrected by audit 70: volatile references/builtin ranking, receiver effects, arrow validity and destructor facts. |
| Reviewed: context/signature identity | [Handoff 71](handoff71.md): lexical/concrete frames, query expansion, decltype bases, structural signature keys and renamed definition attachment. |
| Reviewed: alias formation/correlated packs | [Handoff 72](handoff72.md): erased/default obligations, captures, transparent source/concrete types, argument roles, access, casts and function-type arguments. |
| Reviewed: callable/prototype facts | Handoff 73: shared invocation selection, actual callee edges, projected source recipes, constexpr function decay, immediate conversion failures, surrogate exception effects, prototype `this`/member calls, using exposure/hiding and ABI identity. Audit 74 repairs parenthesized receiver queries, exception prototype/pack types, conversion exposure, abstract parameters and namespace return identity. 647 semantic, 26 repaired-course, three completion and four ABI controls pass. |
| **Remaining: declaration timing and lookup** | Lazy nested classes, first-declaration result lookup, remaining member/alias syntax and lookup. Prototype `this` and refreshed member-call overload sets are now repaired. |
| **Remaining: constructor/explicit deduction and initialization queries** | Constructor/inherited-constructor and explicit member-template participation; braced initialization/narrowing, cast/access validity. The dependent tag invocation needs a braced-value initialization query. |
| **Remaining: ordinary LowIR facts/policy** | Constant/array initialization, bool/result metadata, class-result conventions and discarded loads. Preserve the earlier nested-alias cast comparison and the personal class-ellipsis reducer as unfinished implementation. |

These remaining groups are required implementation, not waived audit questions.
Context, alias and callable handoffs shared prototype, exception, using and
execution consumers. Their separation left ownership gaps and repeated evidence
packaging. Consolidate each remaining ownership group through its consumers and
validation; avoid small handoffs split at those same boundaries.

Performance acceptance is **PA18/O0 LowIR**, spec §9.
[Performance 74](performance74.md) records frozen checkpoint and cumulative
A/A/ABBA compiler latency/RSS, checked runtime/size and graph scaling. New required
behavior has final-only cost evidence where the baseline rejects it; equivalent
correct output is checked before paired comparisons. The initial audit run is
preserved alongside the final measurements. No optional optimization or mandated
numerical ceiling is introduced. Historical **+15%, +16 MiB, 5.5×** targets remain
diagnostics, not exit gates. Preserve evidence
[63](performance.md), [64](performance64.md), [65](performance65.md),
[66](performance66.md), [67](performance67.md), [68](performance68.md),
[69](performance69.md), [70](performance70.md), [71](performance71.md),
[72](performance72.md), [73](performance73.md), including unsuccessful attempts.
Correctness, coverage and graph bounds remain gates; own native optimization and
self-hosting belong to PA24–PA34. Remaining course failures are not excused by
runtime agreement or the performance acceptance policy.

Reference corrections remain [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md), with reducers,
rule proofs and pinned bundles. Loops 70–74 add none.

| Handoff ledger | Reviewed range / disposition |
|---|---|
| 66 | Stage base → `3a883d10`; 327/420, prior/file audit pass; [audit](audit66.md). |
| 70 | `3a883d10` → entry `15b34993` → fix `f59e8f67`; prior three handoffs reviewed/fixed; 353/420; [evidence](../student.tests/pa18/loop70-evidence.json). |
| 71 | Entry `2eb83de5` → code `b6287928`; 367/420; prior/file audit pass; [evidence](../student.tests/pa18/loop71-evidence.json). Reviewed with consumers in audit 74. |
| 72 | Entry `ca314a72` → code `57ee1f2e`; 372/420; prior/file audit pass; [evidence](../student.tests/pa18/loop72-evidence.json). Reviewed with consumers in audit 74. |
| 73 | Entry `12cbfe83` → code `9ebc507c`; 379/420; failures 48 → 41, no new failures; prior/file audit pass; [evidence](../student.tests/pa18/loop73-evidence.json). Reviewed and corrected by audit 74; required remaining implementation is grouped above. |
| 74 | `f59e8f67` → entry `2433d6de` → code `8dc4636d`; all three handoffs and their interactions audited; five ownership findings repaired. 379/420, exact same 41 failures; prior 2609/2609; file audit and coverage pass; [evidence](../student.tests/pa18/loop74-evidence.json). Stage-scoped performance accepted; full-stage remains unfinished. |

Required checks: `make test-pa18`, `make test-report-through-pa17`, PA18 file audit.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
