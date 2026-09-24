# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Target: PA18 full-stage. Stage entry: **266/420**; handoff 63: **282/420**.
Loop 64 entry: `30a610065e586c481a094c60698f7ae8a7952904`, **282/420**.
Loop 64 handoff: **312/420**, 30 existing failures fixed, zero new failures.
Previous turn classification: progress (16 existing failures fixed). Review markers above remain unchanged.

| Owner / group | Design, data flow and work |
|---|---|
| Function deduction and ordering (completed group) | Canonical type/parameter identities feed directional partial deduction; only participating call parameters (full signature for addresses). Reference/cv and pack tie rules select the declaration once; conversion facts carry it to typed LowIR. Scratch bindings live for one comparison; O(candidate count × compared type edges), no body demand, rendered keys, exception-based rejection or global scans. |
| Prototype packs / ABI (completed group) | Signature expansion and element expression types are distinct. Typed object/type lanes; structured indirect-query failure; direct `sizeof...` ABI graph. O(required lanes/query edges/output), TU-owned facts. |
| Immediate substitution / SFINAE (loop 64) | `specialize`/defaults and query/type owners unify explicit/deduced probing, propagate query/array/alias failures before access checks, preserve class/body hard errors. Incomplete prerequisites publish class→query→consumer edges; completion invalidates only affected queries/constants, and blocked alias/signature facts retry on demand. O(required type/query edges + notified dependency edges); canonical identity, TU storage, stack-local candidate state. |
| Retained member/lexical contexts and remaining expressions (unfinished) | Bind declaring/source owner frames for nested/member results/defaults. Add missing compound assignment, braced construction and destructor queries; implement remaining cast/access/conversion validity at their semantic owners. These require new facts/checks, not broader exception suppression. |
| Conversion/constructor/explicit deduction (remaining) | Target or call values → deduction → ordinary recorded conversions; preserve demand boundaries and member owners. |
| LowIR mismatches (remaining) | Separate already-correct deduction from initialization, result metadata, and constant lowering differences; preserve comparison rules. |

Spec alignment: extend the existing typed semantic graph, TU interners and
per-comparison scratch; no token replay or semantic text transport. Stage is
O0 LowIR: no optional optimization or native-backend work is introduced.
Prior performance: [frozen measurements](performance.md) cover A/A and ABBA compiler
latency/RSS, exact common LowIR/executable identity, final-only semantic costs,
checked runtime/text size and 600→2400 specialization scaling.
PA18/O0 mandates no numeric latency/RSS ceiling; later native/self-host stages
remain outside this stage. Preserve explicit work bounds and all observations.
Loop 64: [current evidence](performance64.md) records small common compiler
increases (paired 0.4–1.4%), identical common executables/zero growth, final-only
semantic costs, linear query/edge scaling and precise completion invalidation.

Validation: run focused required fixtures and explicit personal controls,
`make test-pa18`, root through-PA17, and the PA18 file audit. Full through-PA18
must pass before advancement. No fixtures, references or coverage are waived.

| Handoff ledger | Implementation | Independent review |
|---|---|---|
| 63 | Ordering/address + prototype pack result/identity/ABI group complete through `4a49ea10`; [handoff boundary](handoff63.md). Remaining groups above are unfinished implementation. Required prior/file checks pass; PA18 282/420, 64 personal controls pass. | Last-reviewed remains the stage base. Ordering direction/context/cache and pack/query/ABI integration need independent review; whole-stage findings remain required before advancement. |
| 64 | Immediate substitution/query failure and completion-dependency group through `e01b8763`; [handoff boundary](handoff64.md). Prior 2609/2609, file audit pass; PA18 312/420. 33 substitution + 64 prior controls, ABI and completion scaling pass. Remaining 86 status/22 LowIR failures require the other owners above. | Review immediate vs side-effect failures, cache dependency/active-query lifecycle and ABI encodings. Earlier review markers remain open; this is an implementation handoff, not whole-stage certification. |
