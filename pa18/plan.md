# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `8dc4636d23a38f2bbcc8662b88b07f7979715c2d`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation 77 in progress**.
Entry `b87de70e` → code `e052e939`, `bff709db`, `c8a2aad8`; independent review of 75–76 pending.

## Active ownership group (77)

Entry `2ce99c6a` is clean, **385/420** (35 failures). Prior turn completed
validated handoff 76: progress, with no live compiler/build process to resume.
Work on lazy nested-class declaration/definition demand first, extending through
related declaration timing consumers when the same ownership applies. Class
identity and lexical substitution frames must survive declaration-only use;
completion owns one monotonic definition state, and only demanded source regions
may publish concrete members. Work must follow class/member edges, with no
global retry, source replay or eager dormant definitions. Validate unused-invalid
and demanded-valid/invalid nested definitions, shared identities, complete-class
contexts and graph scaling, then the course/prior/file checks and frozen O0
latency/RSS/runtime/size observations. Remaining lookup and ordinary LowIR groups
remain implementation work; independent review markers above remain unchanged.
Nested completion now repairs the two initial course failures (**387/420**, no
new failures). 52 nested controls cover dormant/demanded definitions, identity,
explicit instantiation and complete-class contexts. Extend the same class-query
path to ambiguous inherited type lookup: preserve a compact ambiguous result
through indexed base traversal and let substitution discard the candidate, while
class-definition side effects and ordinary lookup retain hard diagnostics.

## Design/spec alignment and current evidence

[Handoff 76](handoff76.md) completes inherited constructor template participation
through notional default-omission signatures, local hiding, access/deletion,
forwarding, constexpr/noexcept, value transfers/defaults and parameter destruction.
Derived candidates retain the base head and typed constructor edge. Canonical
specializations and forwarding recipes own selected conversions; consumers reuse
those facts without source replay or overload resolution in lowering. Omitted parameter types remain dormant until forwarding is required. Work tracks
candidate parameter edges and demanded recipes; repeated query work is cached.
No unused body, host/reference implementation, stage switch or new tolerance.

Required stage tests: **385/420**, entry **383/420**; original failures **37 → 35**,
no new failures. All **420 inputs and 1,686 tracked fixture/reference files** are unchanged.
Earlier PAs: **2609/2609**. File audit passes with the three inherited header
advisories. **778 personal semantic controls** pass (60 new, 718 inherited), plus
three course executions, twelve forwarding graph controls and three inherited
completion controls. [Evidence](../student.tests/pa18/loop76-evidence.json) retains
checks, intermediate failures/repairs, and frozen performance observations.
The prior 1,712 count also included 26 generated `.check*` observations; the
tracked manifest is 1,686 files. This corrects accounting, with no coverage change.

## Remaining required implementation

| Ownership group | Required next work / boundary |
|---|---|
| Declaration timing and lookup | Lazy nested classes, first-declaration result lookup, remaining member/alias syntax and visibility. Preserve the reviewed prototype/context/signature and completion edges. |
| Constructor and explicit deduction | Explicit member-template participation and remaining alias argument syntax. Inherited forwarding is complete; its remaining course mismatch belongs to ordinary LowIR policy below. |
| Ordinary LowIR facts/policy | Constant/array and empty-tag initialization, constructor object-root metadata, bool/result metadata, class-result conventions and discarded loads. Earlier fixtures require pooling for the same small scalar-array shapes, so a blanket removal breaks prior stages. Reconcile the policy/contract without a stage switch or weakened comparison. Preserve the previous nested-alias cast and class-ellipsis reducers as unfinished implementation. |

The scope was extended through the forwarding consumers and exposed defects.
The member-alias-pack inherited case now executes but still fails comparison on
empty-tag zeroing/object-root policy; it is not counted as repaired. Remaining
work needs distinct declaration-state or ordinary lowering-policy changes.
Independent audit questions concern proxy/head identity, completion retries,
notional signatures, lifetime/ABI facts and interactions with handoff 75. These
are separate from the **35 unfinished course cases**; neither is waived.

## Performance and references

Acceptance is **PA18/O0 LowIR**, spec §9. [Performance 76](performance76.md) and
[observations](../student.tests/pa18/loop76-performance-final.json) preserve frozen A/A,
ABBA compiler latency/RSS, checked runtime/size, output equivalence and graph
scaling. New required behavior has final-only costs when the entry rejects it.
No optional transform or mandated numeric ceiling is introduced. Historical
**+15%, +16 MiB, 5.5×** targets remain diagnostics, not exit gates; preserve all
measurements in performance records 63–74, including unsuccessful attempts.
Correctness, coverage and graph bounds remain gates. Native optimization and
self-hosting belong to PA24–PA34; this does not excuse current course failures.

Reference corrections remain [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md), with reducers,
rule proofs and pinned bundles. Loops 70–76 change no references.

## Handoff ledger

Stage entry **266/420** → 63 **282** → 64 **312** → 65/66 **327** → 67 **343**
→ 68 **348** → 69/70 **353** → 71 **367** → 72 **372** → 73/74 **379** → 75 **383** → 76 **385**.

| Handoff | Range / disposition |
|---|---|
| 66 | Stage base → `3a883d10`; [audit](audit66.md), 327/420; prior/file audit pass. |
| 70 | `3a883d10` → `15b34993` → fix `f59e8f67`; three handoffs reviewed/fixed; [audit](audit70.md), 353/420; prior/file audit pass. |
| 71 | `2eb83de5` → code `b6287928`; [context handoff](handoff71.md), 367/420; prior/file audit pass; reviewed in audit 74. |
| 72 | `ca314a72` → code `57ee1f2e`; [alias/pack handoff](handoff72.md), 372/420; prior/file audit pass; reviewed in audit 74. |
| 73 | `12cbfe83` → code `9ebc507c`; [callable handoff](handoff73.md), 379/420; 48 → 41 failures; prior/file audit pass; reviewed/corrected in audit 74. |
| 74 | `f59e8f67` → `2433d6de` → code `8dc4636d`; [audit](audit.md), all three handoffs/interactions reviewed, five ownership findings repaired; unchanged 41 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop74-evidence.json). |
| 75 | `9fa23653` → code `6229b49a`; [list/cast handoff](handoff75.md), 383/420; 41 → 37 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop75-evidence.json). Independent audit pending; full-stage remains unfinished. |
| 76 | `b87de70e` → code `e052e939`, `bff709db`, `c8a2aad8`; [inherited forwarding handoff](handoff76.md), 385/420; 37 → 35 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop76-evidence.json). Independent audit pending; full-stage remains unfinished. |

Required checks: `make test-pa18`, `make test-report-through-pa17`, PA18 file audit.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
