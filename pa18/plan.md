# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `82fca940b1849d90deffbaba29ee162946f3e23c`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation handoff 81 complete**.
Entry `69247877`: **395/420** → **396/420**, failures **25 → 24**, no new failures.
Implementation `7b8c98a6`, `4ab09a9b`, `25b89fc2`; [handoff](handoff81.md),
[evidence](../student.tests/pa18/loop81-evidence.json). Previous goal turn was
progress (committed handoff 80 and validation); no live process was inherited.
Ralph owns independent acceptance/review; both review markers stay unchanged.

## Design/spec alignment

Requested, completed conversion bodies publish a bounded runtime proof when they forward
an already-known named scalar constant. Lowering consumes that result and the
selected second conversion while preserving receiver effects, lifetime actions,
reference materialization and selected conditional arms. Actual explicit calls,
addresses and roots retain ordinary function emission. Address validation also
repaired qualified conversion lookup and canonical class-alias member-pointer
ownership. The proof never grants constexpr eligibility or demands another body.

The [owner/data-flow ledger](handoff81.md) records bounds and source-to-native
validation. No syntax replay, text transport, global retry, new object ABI or
arbitrary body evaluator is introduced. **1137** explicit controls pass; new
controls improve **63/74 → 74/74**. **Ten** inspection programs prove scaling,
wrapper fallback, emission and the combined trace. Prior **2609/2609**, file
audit, inherited ABI/scaling and all three traces pass. All **420** course sources
and **1686** fixture/reference files and comparison rules remain unchanged.

## Remaining implementation and independent review

| Owner | Required work / concrete boundary |
|---|---|
| Array/aggregate initialization and constant materialization | Unknown-bound array after empty expansion; array/string/constant representations and constexpr union initialization. Earlier fixtures pool shapes represented with stores by PA18. Resolve the shared policy without stage/filename switches. |
| Object ABI, emission and scalar representation | Empty-tag construction/root metadata, class-result conventions, static member publication, discarded loads and arithmetic widening. Together with the first row, **24 original failures** remain. The inherited class-ellipsis reducer remains unfinished. |
| Independent review | Review 79's signatures/lookup/ABI, 80's casts/materialization, and 81's runtime proof, selected-arm cleanup, retained emission and qualified address lookup. Passing controls support these review questions; they remain separate from unfinished implementation. |

The named-result trace was extended through scalar/reference consumers,
conditionals, cleanup, explicit/address uses, aliases and emission. Its scalar
leaf proof cannot establish array images, class-result ABI or static member
storage definitions. Remaining failures require those separate owners and a new
contract trace. Extending this optional summary into an arbitrary body optimizer
would not resolve them. This is a coherent implementation boundary, not full-stage
acceptance or a waiver of implementation/review requirements.

## Performance acceptance

Acceptance is **PA18/O0 LowIR**, spec §9. [Performance 81](performance81.md)
records frozen A/A/ABBA compiler latency/RSS and checked native runtime/size.
The O0 policy inspects one requested completed conversion body and at most eight wrappers,
retains at most one scalar fact and emits no additional executable work for a
summarized use. Unproved bodies keep ordinary calls. Emission visits its deferred
leaf list once. Compiler and executable observations, scaling counters and
retained-use costs are preserved in the raw evidence. Affected runtime paired
ratios are **0.735 / 0.707**, payloads **222 → 196 / 268 → 192 bytes**.
Explicit-only output is byte-identical with zero summary work; compiler text
grows **7232 bytes (0.369%)**. A focused repeat resolves the noisy common-loop
compiler observation (**1.050 → 0.995** paired), with all samples preserved. No higher-level optimizer
or native/self-hosting requirement is introduced.

Historical measurements, including [performance 80](performance80.md), remain
intact. PA18 has no mandated numerical latency/RSS ceiling; historical **+15%,
+16 MiB, 5.5×** diagnostics remain non-gates. Correctness, coverage, mandated
limits, bounded O0 work and measured profitability remain acceptance requirements.
Native optimization/self-hosting belong to PA24–PA34. Prior reference proofs
[65](reference-correction65.md), [67](reference-correction67.md),
[69](reference-correction69.md), [79](reference-correction79.md) remain preserved;
this handoff makes no reference correction.

## Handoff ledger

Stage entry **266/420** → 63 **282** → 64 **312** → 65/66 **327** → 67 **343**
→ 68 **348** → 69/70 **353** → 71 **367** → 72 **372** → 73/74 **379** → 75 **383**
→ 76 **385** → 77/78 **388** → 79 **393** → 80 **395** → 81 **396**.

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
| 80 | `c5e2c271` → code `cec91d23`, `a87dd911`, `ce7d3e7f`; [scalar/reference handoff](handoff80.md), **395/420**, **27 → 25** original failures; prior **2609/2609**, file audit/unchanged coverage pass; **1062** explicit controls, ABI/scaling/traces and frozen performance verified. [Evidence](../student.tests/pa18/loop80-evidence.json). Independent acceptance/review pending. |
| 81 | `69247877` → code `7b8c98a6`, `4ab09a9b`, `25b89fc2`; [named-result handoff](handoff81.md), **396/420**, **25 → 24** original failures; prior **2609/2609**, file audit/unchanged coverage pass; **1137** controls, **10** inspection programs, ABI/scaling/traces and frozen performance. [Evidence](../student.tests/pa18/loop81-evidence.json). Independent acceptance/review pending. |

Required checks: `make test-pa18`, `make test-report-through-pa17`, PA18 file audit.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
