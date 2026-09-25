# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `82fca940b1849d90deffbaba29ee162946f3e23c`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation 81 active**.
Entry HEAD: `6924787714fac6bdeb0b3df29df49bb7cbe5fa7e`, **395/420**.
Previous goal turn: progress (committed handoff 80 and verified checks); no live inherited job.

Current group: bounded runtime scalar conversion summaries. Semantic owner records
a checked, effect-free constant return; lowering consumes the summary while
evaluating receiver effects and preserving lifetimes, explicit calls and address
uses. One bounded body inspection per completed conversion; no constexpr
eligibility change or additional body demand. Validate native effects, emission,
constant-expression rejection, prior suites, and frozen A/A/ABBA costs/profit.
The group will extend through all discovered consumers before handoff.
Entry `c5e2c271`: **393/420** → **395/420**, failures **27 → 25**; no new failures.
Implementation `cec91d23`, `a87dd911`, `ce7d3e7f`; [handoff](handoff80.md),
[evidence](../student.tests/pa18/loop80-evidence.json). Previous goal turn was
progress: committed handoff 79 and validation. No background job was inherited.
Ralph owns independent acceptance/review; the reviewed marker remains unchanged.

## Design/spec alignment

Scalar/reference conversions share one semantic selection path for ordinary
expressions and substitution queries. Selected conversions drive correctly typed
runtime/constant temporary storage, null-preserving base adjustments, bit-field
copies, exception facts and narrowing checks. Conditions consume published
integral constants. No new semantic cache, parser replay, global scan, text
transport, optimizer or class-result ABI policy is introduced. The owner/data-flow,
source-to-native trace and work bounds are in [handoff 80](handoff80.md).

The initial constant-condition/pointer-temporary group was extended through
scalar casts, SFINAE, bit-fields, null/base conversions, constant materialization,
conversion-function results, narrowing and noexcept. Its discovered correctness
defects are repaired. New controls improve **38/68 → 68/68**. The inherited
nested-alias course source executes on both frozen compilers; only its array
LowIR comparison remains open. Validation: **2609/2609** prior tests, file audit
pass (three inherited header advisories), **1062** explicit controls plus
ABI/scaling and both traces. All **1686** course fixture/reference files and
comparison rules are preserved; no reference correction is made this turn.

## Remaining implementation and independent review

| Owner | Required work / concrete boundary |
|---|---|
| Array/aggregate initialization and constant materialization | Unknown-bound array after empty expansion; array/string/constant representations, constexpr union initialization. Earlier fixtures require pooling for shapes that PA18 represents with stores. Resolve the shared policy without stage/filename switches. |
| Object ABI, emission and body effects | Empty-tag construction/root metadata, class-result conventions, global publication, discarded loads, arithmetic widening representation and non-constexpr conversion-body folding. Together with the previous row, **25 original failures** remain. The class-ellipsis reducer still emits invalid variadic LowIR. |
| Independent review | Review 79's source signatures/lookup/ABI, plus 80's shared cast selection, bit-field/cv rules, constant storage identity and selected conversion exception/result facts. These are review obligations with passing controls, separate from unfinished implementation. |

The next owners need materialization policy, class ABI/emission facts or a
conversion-function body effect summary. A selected scalar conversion and its
reference temporary cannot provide those facts. This is the coherent handoff
boundary after extending the same trace through its runtime and constant
consumers, not an assertion that PA18 is complete. Requirements are not waived.

## Performance acceptance

Acceptance is **PA18/O0 LowIR**, spec §9. [Performance 80](performance80.md) records
frozen A/A/ABBA compiler latency/RSS and checked native runtime/size, including
600/2400 pointer, cast-reference and constant-reference scales. Required scalar
conversion work has a fixed per-use budget and no added optimization pass. Eleven
semantically equivalent sources have unchanged native bytes wherever emitted
(ten executables); eight also have identical LowIR. Large unchanged compiler
paired ratios are **0.964–1.006**; compiler text grows **1536 bytes (0.078%)**.
Five newly supported workloads have final-only costs. Runtime samples are noisy;
no speedup is claimed for identical executable bytes.

Historical measurements and [performance 79](performance79.md) remain intact.
No numerical latency/RSS ceiling is mandated for PA18; historical **+15%, +16 MiB,
5.5×** diagnostics remain non-gates. Correctness, coverage, mandated limits and
stage-scoped work bounds remain gates. Native optimization/self-hosting belong
to PA24–PA34. Prior reference proofs [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md),
[79](reference-correction79.md) and pinned bundle records remain preserved.

## Handoff ledger

Stage entry **266/420** → 63 **282** → 64 **312** → 65/66 **327** → 67 **343**
→ 68 **348** → 69/70 **353** → 71 **367** → 72 **372** → 73/74 **379** → 75 **383**
→ 76 **385** → 77/78 **388** → 79 **393** → 80 **395**.

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

Required checks: `make test-pa18`, `make test-report-through-pa17`, PA18 file audit.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
