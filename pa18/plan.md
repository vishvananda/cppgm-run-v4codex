# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `8dc4636d23a38f2bbcc8662b88b07f7979715c2d`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation handoff 77**.
Entry `2ce99c6a` → code `86e142ba`, `b6294394`, `975e6162`.
Independent review of handoffs **75–77 remains pending**.

## Design/spec alignment and evidence

[Handoff 77](handoff77.md) separates nested member-class declarations from
completeness demand. Source regions and class definitions have distinct retained
identities; one monotonic definition state publishes members into the declared
scope. Explicit nested instantiation/specialization, complete-class defaults,
layout and ordinary consumers use that identity. Anonymous unions and ordinary
local classes retain their required eager behavior. Source bodies remain shared;
no grammar replay, unrelated retry, host/reference implementation or new tolerance.

Indexed base lookup propagates a compact ambiguous result. Type/value queries
consume it as substitution failure; class-definition side effects and ordinary
lookup still diagnose. Partial matching checks retained access recipes even when
an alias erases its argument. Work follows requested class members, source regions,
lookup edges and candidate arguments; records have translation-unit ownership.

Required PA18 tests: **388/420**, entry **385/420**; failures **35 → 32**, no new
failures. All **420 inputs and 1,686 tracked fixture/reference files** are unchanged.
Earlier PAs: **2609/2609**. File audit passes with the three inherited header
advisories. **875 personal semantic controls** pass (97 new, 778 inherited), plus
18 nested demand/region controls, three nested completion-invalidation controls
and three inherited completion controls. [Evidence](../student.tests/pa18/loop77-evidence.json)
retains checks, intermediate failures/repairs, frozen binaries and observations.

## Remaining required implementation

| Ownership group | Required next work / boundary |
|---|---|
| Source signatures and member/alias syntax | First-declaration function-result lookup; explicit member specialization deduction; pack-expanded explicit member-template arguments; alias/function argument cv syntax; using-directive function-template argument participation. These own five rejected cases, separate from nested class completion. |
| Ordinary LowIR facts/policy | One empty-pack unknown-bound-array rejection and 26 comparisons: constant/array and empty-tag initialization, constructor object-root metadata, bool/result metadata, class-result conventions and discarded loads. Earlier fixtures require pooling for the same small scalar-array shapes; reconcile the policy without a stage switch or weakened comparison. Preserve the nested-alias cast and class-ellipsis reducers as unfinished implementation. |

The completed group was extended through explicit class demand, access checks,
value queries and completion invalidation. The **32 unfinished course cases**
require distinct signature/syntax or ordinary lowering owners; changing the
completed class-demand path cannot repair them while preserving its timing rules.
The inherited member-alias-pack source still executes but fails empty-tag/root
metadata comparison, and is not counted as repaired.
Independent review questions concern class declaration/definition identity,
source-region boundaries, access environments and interactions with 75–76's
constructor/lifetime facts. They are separate from unfinished implementation;
neither category is waived.

## Performance and references

Acceptance is **PA18/O0 LowIR**, spec §9. [Performance 77](performance77.md) and
[final observations](../student.tests/pa18/loop77-performance-final.json) preserve
frozen A/A and ABBA compiler latency/RSS, checked executable runtime/size and
output equivalence. The first batch remains in [its original record](../student.tests/pa18/loop77-performance.json).
Dormant definitions create zero member occurrences; demanded definitions are
linear in their actual members, and repeated demand reuses one definition.
Newly accepted behavior has final-only costs when the entry rejects it. No
optional optimization or mandated numeric ceiling is introduced. Historical
**+15%, +16 MiB, 5.5×** targets remain diagnostics, not exit gates; all measurements
in records 63–76 remain. Correctness, coverage and graph bounds remain gates.
Native optimization/self-hosting belongs to PA24–PA34 and does not excuse failures.

Reference corrections remain [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md), with reducers,
rule proofs and pinned bundles. Loops 70–77 change no references.

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
| 74 | `f59e8f67` → `2433d6de` → code `8dc4636d`; [audit](audit.md), all three handoffs/interactions reviewed, five ownership findings repaired; unchanged 41 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop74-evidence.json). |
| 75 | `9fa23653` → code `6229b49a`; [list/cast handoff](handoff75.md), 383/420; 41 → 37 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop75-evidence.json). Independent audit pending; full-stage remains unfinished. |
| 76 | `b87de70e` → code `e052e939`, `bff709db`, `c8a2aad8`; [inherited forwarding handoff](handoff76.md), 385/420; 37 → 35 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop76-evidence.json). Independent audit pending; full-stage remains unfinished. |
| 77 | `2ce99c6a` → code `86e142ba`, `b6294394`, `975e6162`; [nested completion/lookup handoff](handoff77.md), 388/420; 35 → 32 failures, prior 2609/2609, file audit/coverage pass; [evidence](../student.tests/pa18/loop77-evidence.json). Independent review of 75–77 pending; full-stage unfinished. |

Required checks: `make test-pa18`, `make test-report-through-pa17`, PA18 file audit.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
