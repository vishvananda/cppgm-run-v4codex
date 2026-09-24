# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `f59e8f67cd8c832361130aef9af1a0337b25c45d`.
Target: **PA18 full-stage**, unfinished. Phase: **checkpoint audit 70**.
Previous goal turn: **progress** (loop 69 implementation, proof and validation).
Loop 70: **progress** (accumulated review, independent reducers, ownership fixes,
required validation and frozen performance evidence).

Stage entry **266/420** → handoffs 63 **282**, 64 **312**, 65 **327**;
audit 66 **327** → handoff 67 **343** → 68 **348** → 69 **353** → audit 70
**353/420**. The audit retains exactly the entry's **67 failures** (43 status,
24 LowIR), all 420 inputs and comparison rules. Earlier PAs: **2609/2609**;
file audit passes with the same three inherited header advisories.
[Audit 70](audit.md) reviews all 13 commits since audit 66 plus the audit fix.
[Audit 66](audit66.md) preserves the first accumulated review and its evidence.

| Ownership group | State and next work |
|---|---|
| Reviewed: deduction, ordering, conversion, query demand, address NTTPs, outer-head defaults and concrete braced/base/array deduction | Preserve canonical identities, immutable frames, precise completion edges, selected conversions, address demand and internal linkage. Handoffs [67](handoff67.md), [68](handoff68.md) and their interactions are reviewed. |
| Reviewed: assignment/destructor queries and ordinary lowering | Handoff [69](handoff69.md) reviewed and corrected: volatile reference targets and builtin ranking, scalar receiver effects/cleanup, query-only arrow validity, and caller-independent destructor deletion facts. 56 independent audit controls pass; 34 fail at entry. |
| **Remaining: retained contexts, packs and expression validity** | Correlated outer/inner pack lanes, nested/member aliases/results/defaults and out-of-class ownership; typed braced query/list conversion plans shared by narrowing, constant evaluation and exception effects; remaining cast/access/selected-conversion validity. |
| **Remaining: constructor/explicit deduction** | Constructor and inherited-constructor participation, remaining explicit/member-template and alias contexts. |
| **Remaining: LowIR initialization and result facts** | Constant/array initialization, bool/result metadata, class-result conventions and discarded-value loads. Execution agreement does not replace canonical LowIR comparison. |

These three remaining groups are required implementation, not waived audit
questions. Avoid another sequence of narrow handoffs across one ownership path:
finish each group's query, substitution, ordinary execution, ABI and cache
interactions together, then package shared validation once. Loops 67–69 repeated
evidence packaging and left shared query/access/effect interactions to this audit.

Performance acceptance is **PA18/O0 LowIR** under spec §9. [Performance 70](performance70.md)
records the checkpoint and complete review-range comparisons, A/A calibration,
ABBA latency/RSS, checked runtime/text proxy, scaling and all observations.
No optional optimization or new numerical limit is introduced. Historical PA17
**+15%, +16 MiB, 5.5×** targets remain diagnostics; preserve all earlier
[63](performance.md), [64](performance64.md), [65](performance65.md),
[66](performance66.md), [67](performance67.md), [68](performance68.md),
[69](performance69.md) measurements. Correctness, coverage and graph-work bounds
remain gates. Native backend/optimization and self-hosting belong to PA24–PA34.

Reference corrections remain [65](reference-correction65.md),
[67](reference-correction67.md) and [69](reference-correction69.md), each with
reducer, standard/contract proof and pinned bundle. Audit 70 adds none.

| Checkpoint ledger | Reviewed range / disposition |
|---|---|
| 66 | Stage base → `3a883d10`; 327/420, prior/file audit pass; [archived record](audit66.md). |
| 70 | `3a883d10` → entry `15b34993` → fix `f59e8f67`; all three handoffs reviewed, ownership findings fixed; 353/420, same 67 failures, prior/file audit pass, coverage retained; [evidence](../student.tests/pa18/loop70-evidence.json). |

Required checks: `make test-pa18`, `make test-report-through-pa17`, and the PA18
file audit. Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
