# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `f59e8f67cd8c832361130aef9af1a0337b25c45d`.
Target: **PA18 full-stage**, unfinished. Phase: **implementation handoff 72**.
Previous goal turn: **progress** (committed context group, validation and evidence).
Loop 72: **progress**, entry `ca314a72` → implementation `57ee1f2e`.
[Handoff 72](handoff72.md) records owners, data flow, bounds, validation and the
boundary to invocation/declaration/initialization work. Review markers above
are preserved; this implementation handoff is not independent certification.

Stage entry **266/420** → handoffs 63 **282**, 64 **312**, 65 **327**;
audit 66 **327** → handoff 67 **343** → 68 **348** → 69 **353** → audit 70
**353** → handoff 71 **367** → handoff 72 **372/420**.
This turn repairs five original failures with none added; **48 failures** remain
(23 status, 25 LowIR). All 420 inputs/references/comparison rules are retained.
Earlier PAs: **2609/2609**. File audit passes with the same three inherited
header advisories. [Audit 70](audit.md) and [audit 66](audit66.md) remain intact.

| Ownership group | State and required next work |
|---|---|
| Reviewed: deduction, ordering, conversion, query demand, address NTTPs, outer defaults and braced/base/array deduction | Preserve canonical identities, immutable frames, precise completion edges, conversions, address demand and linkage. Handoffs [67](handoff67.md), [68](handoff68.md) and their interactions are reviewed. |
| Reviewed: assignment/destructor queries and ordinary lowering | [Handoff 69](handoff69.md) reviewed and corrected: volatile references/builtin ranking, scalar receiver effects, arrow validity, caller-independent destructor facts. 56 independent audit controls pass. |
| Implemented, review pending: context and signature identity | [Handoff 71](handoff71.md): lexical/concrete frames, query argument expansion, decltype bases, structural signature keys and renamed definition attachment. |
| Implemented, review pending: alias formation and correlated expansions | Handoff 72 retains erased/default argument obligations and pack captures; shares type/value/query expansion; preserves source access, transparent signatures, parameter/conversion adjustments and selected alias cast types; interprets template argument roles through their typed owner. 487 semantic, five repaired-course, three completion and six ABI controls pass. |
| **Remaining: invocation and declaration timing** | `__builtin_invoke` needs query and ordinary callable/object/conversion facts. Lazy nested classes, first-declaration result lookup, prototype `this`, and remaining member/alias syntax and lookup still fail. |
| **Remaining: constructor/explicit deduction and query validity** | Constructor/inherited-constructor and member-template participation; braced initialization and cast/access/selected-conversion validity. |
| **Remaining: LowIR initialization and result facts** | Constant/array initialization, bool/result metadata, class-result conventions and discarded-value loads. A nested alias cast now compiles/executes but still fails its array-initialization comparison. A retained personal reducer exposes invalid ordinary class-varargs LowIR; its implementation is unfinished. |

The remaining groups are required implementation, not waived audit questions.
Finish each group's query, substitution, ordinary execution, ABI and cache
interactions together. Loop 72 expanded through access, adjustments, conversion,
source signatures, explicit pack arguments, casts and function-type arguments;
remaining nearby failures require invocation or ordinary lowering facts that
alias/pack substitution cannot supply. Preserve the detailed boundary in the
handoff instead of treating test progress alone as completion.

Performance acceptance is **PA18/O0 LowIR**, spec §9.
[Performance 72](performance72.md) records frozen A/A/ABBA compiler latency/RSS,
checked executable runtime/size and graph work. Equivalent outputs are
byte-identical. Alias semantic cost is 2.5–2.9%; compiler text grows 13,248 bytes
(0.708%). Fourfold source growth gives about fourfold work/time. The first
benchmark's class-varargs failure and all observations are preserved; the final
pack workload uses scalar arguments and introduces no new acceptance gate.
No optional optimization or mandated numerical ceiling is introduced.
Historical PA17 **+15%, +16 MiB, 5.5×** targets remain diagnostics. Preserve
[63](performance.md), [64](performance64.md), [65](performance65.md),
[66](performance66.md), [67](performance67.md), [68](performance68.md),
[69](performance69.md), [70](performance70.md), [71](performance71.md).
Correctness, coverage and graph bounds remain gates; native optimization and
self-hosting belong to PA24–PA34.

Reference corrections remain [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md), each with
reducer, rule proof and pinned bundle. Loops 70–72 add none.

| Handoff ledger | Reviewed range / disposition |
|---|---|
| 66 | Stage base → `3a883d10`; 327/420, prior/file audit pass; [audit](audit66.md). |
| 70 | `3a883d10` → entry `15b34993` → fix `f59e8f67`; all three handoffs reviewed, findings fixed; 353/420, prior/file audit pass; [evidence](../student.tests/pa18/loop70-evidence.json). |
| 71 | Entry `2eb83de5` → code `b6287928`; 367/420, failures 67 → 53, prior/file audit pass; [evidence](../student.tests/pa18/loop71-evidence.json). Review pending for context/signature/body/access identity and bounds. |
| 72 | Entry `ca314a72` → code `57ee1f2e`; 372/420, failures 53 → 48, prior/file audit pass; [evidence](../student.tests/pa18/loop72-evidence.json). Review pending for alias/capture keys, source/concrete frames, signature/ABI transparency, argument disambiguation, access/error boundaries and measured bounds. These questions do not replace unfinished implementation. |

Required checks: `make test-pa18`, `make test-report-through-pa17`, and the PA18
file audit. Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**
