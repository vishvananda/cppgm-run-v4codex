# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `f59e8f67cd8c832361130aef9af1a0337b25c45d`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation 72**.
Loop 72 entry: `ca314a72`, **367/420**, 53 original failures. Previous goal
turn classified **progress**: committed context implementation and validation;
no prior compiler/check process remains live. Review markers below are preserved.
Initial ownership group: retained alias formation and correlated pack expansion.
`template_entities` must retain dependent argument validity alongside transparent
result types; `template_call`/`template_packs` consume those facts under immutable
frames, with structured candidate failure. Deduction/signature comparison must
preserve alias transparency, and lowering must consume concrete ordinary types.
Work is bounded by retained argument/query edges and demanded expansion lanes,
with TU-owned canonical identities/caches. Validate detectors, nested aliases,
unequal/empty packs, redeclarations, runtime selection and inherited controls;
freeze entry/final binaries for stage-scoped latency/RSS/runtime/size evidence.
Previous goal turn: **progress** (audit 70 ownership fixes and validation).
Loop 70: **progress** (accumulated review, independent reducers, ownership fixes,
required validation and frozen performance evidence).
Loop 71: **progress**, entry `2eb83de5` → implementation `b6287928`.
Retained contexts, query bases and declaration/body identity complete;
[handoff](handoff71.md) records ownership, bounds, validation and the concrete
boundary to correlated-expansion and alias-formation model changes.

Stage entry **266/420** → handoffs 63 **282**, 64 **312**, 65 **327**;
audit 66 **327** → handoff 67 **343** → 68 **348** → 69 **353** → audit 70
**353** → handoff 71 **367/420**. Fourteen original failures repaired, none added;
**53 failures** remain (29 status, 24 LowIR), all 420 inputs and comparison rules
retained. Earlier PAs: **2609/2609**;
file audit passes with the same three inherited header advisories.
[Audit 70](audit.md) reviews all 13 commits since audit 66 plus the audit fix.
[Audit 66](audit66.md) preserves the first accumulated review and its evidence.

| Ownership group | State and next work |
|---|---|
| Reviewed: deduction, ordering, conversion, query demand, address NTTPs, outer-head defaults and concrete braced/base/array deduction | Preserve canonical identities, immutable frames, precise completion edges, selected conversions, address demand and internal linkage. Handoffs [67](handoff67.md), [68](handoff68.md) and their interactions are reviewed. |
| Reviewed: assignment/destructor queries and ordinary lowering | Handoff [69](handoff69.md) reviewed and corrected: volatile reference targets and builtin ranking, scalar receiver effects/cleanup, query-only arrow validity, and caller-independent destructor deletion facts. 56 independent audit controls pass; 34 failed at audit 70 entry. |
| Implemented, review pending: context and signature identity | Handoff 71 composes concrete parent frames, preserves explicitly symbolic lexical frames, expands query template arguments, resolves decltype bases, and separates structural signature keys from semantic access contexts. Renamed out-of-class heads attach bodies to the selected declaration. 410 semantic controls, 14 repaired-course checks, three completion-scaling cases and four ABI controls pass. |
| **Remaining: retained packs/results and expression validity** | Correlated outer/inner expansion recipes; alias argument-formation obligations (including unused arguments); lazy nested class definitions, first-declaration result lookup and prototype object facts; typed braced query/list plans and cast/access/selected-conversion validity. These require new retained facts, not scope recovery. |
| **Remaining: constructor/explicit deduction** | Constructor and inherited-constructor participation, remaining explicit/member-template and alias contexts. |
| **Remaining: LowIR initialization and result facts** | Constant/array initialization, bool/result metadata, class-result conventions and discarded-value loads. Execution agreement does not replace canonical LowIR comparison. |

The remaining groups are required implementation, not waived audit
questions. Avoid another sequence of narrow handoffs across one ownership path:
finish each group's query, substitution, ordinary execution, ABI and cache
interactions together, then package shared validation once. Loops 67–69 repeated
evidence packaging and left shared query/access/effect interactions to this audit.

Performance acceptance is **PA18/O0 LowIR** under spec §9. [Performance 71](performance71.md)
records frozen A/A/ABBA latency/RSS, checked runtime/text proxy and linear shape
work. Equivalent outputs are byte-identical; member-context semantic overhead
is 1.7–2.6%, with compiler text +5120 bytes. [Performance 70](performance70.md)
preserves the prior checkpoint and complete review-range comparisons.
No optional optimization or new numerical limit is introduced. Historical PA17
**+15%, +16 MiB, 5.5×** targets remain diagnostics; preserve all earlier
[63](performance.md), [64](performance64.md), [65](performance65.md),
[66](performance66.md), [67](performance67.md), [68](performance68.md),
[69](performance69.md) measurements. Correctness, coverage and graph-work bounds
remain gates. Native backend/optimization and self-hosting belong to PA24–PA34.

Reference corrections remain [65](reference-correction65.md),
[67](reference-correction67.md) and [69](reference-correction69.md), each with
reducer, standard/contract proof and pinned bundle. Loops 70–71 add none.

| Checkpoint ledger | Reviewed range / disposition |
|---|---|
| 66 | Stage base → `3a883d10`; 327/420, prior/file audit pass; [archived record](audit66.md). |
| 70 | `3a883d10` → entry `15b34993` → fix `f59e8f67`; all three handoffs reviewed, ownership findings fixed; 353/420, same 67 failures, prior/file audit pass, coverage retained; [evidence](../student.tests/pa18/loop70-evidence.json). |
| 71 | Entry `2eb83de5` → code `b6287928`; 367/420, failures 67 → 53, prior/file audit pass, no fixture/reference changes; [evidence](../student.tests/pa18/loop71-evidence.json). Independent review remains pending for frame identity, signature-key completeness, access isolation, body matching and measured bounds; these questions do not replace unfinished implementation. |

Required checks: `make test-pa18`, `make test-report-through-pa17`, and the PA18
file audit. Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
