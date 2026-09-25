# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `82fca940b1849d90deffbaba29ee162946f3e23c`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation 80 active**.
Loop 80 entry: `c5e2c2719f1ef004e6bddb86487e797c37471079`, **393/420**, 27 failures.
Prior turn: progress (committed handoff 79 and its validation); no live test process.
Current group: scalar conversion facts → reference temporary / constant storage →
condition and value lowering. Entry trace confirmed competing pooled/store array
contracts; this remains separate unfinished initialization policy. First fixes:
**395/420**, no new failures; earlier **2609/2609** after retaining the null-to-pointer
conversion boundary. Extended controls exposed query/ordinary cast divergence,
bit-field aliasing and converted constexpr reference values; repairs in progress.
Owners: `explicit_conversion.cpp` selects once for ordinary/query use; typed
conversion records drive runtime/constant materialization. Work follows one
conversion and one temporary per use, cached existing query identities; no scan,
new optimizer or phase roundtrip. Validation includes native/query/constexpr,
volatile, null/base and width/category controls plus frozen A/B evidence.
Preserve the stage-base and reviewed markers above.
Entry `ef0e43c0`: **388/420** → **393/420**, failures **32 → 27**, no new failures.
Implementation `67c4f685`, `50407fc1`; [handoff](handoff79.md),
[evidence](../student.tests/pa18/loop79-evidence.json). Ralph still owns acceptance
and independent review. The prior accumulated [audit](audit.md) covered 75–77;
its reviewed marker is unchanged.

## Design/spec alignment

Source-signature ownership is completed through its related consumers: scoped
parser heads/typedef syntax → typed explicit member arguments → dependent-name
comparison shapes and first-declaration lookup → renamed head/raw body recipes →
query overload selection and ABI facts. Out-of-class member signatures bind
before their bodies. Fixed-type, value-dependent calls retain their source
selection and distinct ABI entities. Source lookup and comparison keys have
separate owners; lowering reads selected facts. Parsed regions remain shared,
frames immutable, caches keyed by typed identity, and work local to source heads,
parameters, candidate sequences and demanded specializations. Owner/data-flow,
complexity budgets and the source-to-native trace are in [handoff 79](handoff79.md).

Validation: **2609/2609** earlier tests; file audit passes with the same three
header advisories; **922 inherited + 65 new semantic controls** and four repaired
course executions pass (**991** cases). ABI, completion/scaling, previous repaired
course checks and both traces also pass. All **420 original inputs** are unchanged.
Of **1,686** fixture/reference files, one expected exit status has a reduced
[C++11 proof](reference-correction79.md); all other files and comparison rules are
unchanged. Its positive prefix remains a separately executed control.

## Remaining implementation and review

| Owner | Required work / concrete boundary |
|---|---|
| Array/aggregate initialization and constant materialization | Unknown-bound array after empty expansion (one rejection); constant/array and empty-tag comparisons. Reconcile shared pooling policy with earlier scalar-array fixtures. |
| Ordinary object/result lowering | Object-root and bool/result metadata, class-result conventions, discarded loads; 26 comparisons across both rows remain required. Preserve the inherited nested-alias cast and class-ellipsis reducers as unfinished runtime implementation. |
| Independent audit | Check dependent-name comparison versus semantic bindings, renamed enclosing/pack-head raw recipes, and fixed-type value-dependent call/ABI facts. These are review obligations with passing controls, not waivers for known failures. |

The initial syntax/member-argument group was extended through declaration
identity, first-lookup semantics, body queries and ABI before handoff. Its known
defects are resolved. Remaining work needs object extent/materialization and
result-convention facts from different initialization/emission owners; source
signature rebinding cannot repair those policies. This is the coherent handoff
boundary, not a claim that PA18 is complete. Every remaining failure stays an
implementation requirement.

## Performance and references

Acceptance is **PA18/O0 LowIR**, spec §9. [Performance 79](performance79.md) retains
frozen A/A/ABBA latency/RSS and checked runtime/size for **18 workloads**, including
600/2400 source-signature, member-body and explicit-member-argument scales.
Ten equivalent sources produce identical LowIR/native bytes. Heavy comparable
compiler ratios are 0.978–1.019; ordering's small +1.9% paired cost is disclosed.
Equivalent runtime ratios are 0.998–1.002. Compiler text grows **13,376 bytes
(0.69%)**. Eight new-behavior workloads have final-only costs. Counters track
source/demand size and reuse source checks; there is no optional optimizer.

The [78 evidence](performance78.md) and historical measurements remain intact.
There is no mandated numerical ceiling at this stage; historical **+15%, +16 MiB,
5.5×** diagnostics remain non-gates. Correctness, coverage, mandated limits and
current-stage work bounds remain gates. PA24–PA34 own native optimization and
self-hosting. Prior reference corrections [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md) are preserved;
[79](reference-correction79.md) adds one proven rejection with pinned bundle.

## Handoff ledger

Stage entry **266/420** → 63 **282** → 64 **312** → 65/66 **327** → 67 **343**
→ 68 **348** → 69/70 **353** → 71 **367** → 72 **372** → 73/74 **379** → 75 **383**
→ 76 **385** → 77/78 **388** → 79 **393**.

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

| 79 | `ef0e43c0` → code `67c4f685`, `50407fc1`; [signature handoff](handoff79.md), 393/420; 32 → 27 original failures, prior 2609/2609, file audit/coverage pass, 991 controls and frozen performance; one proved reference correction. [Evidence](../student.tests/pa18/loop79-evidence.json). Independent acceptance/review pending. |

Required checks: `make test-pa18`, `make test-report-through-pa17`, PA18 file audit.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
