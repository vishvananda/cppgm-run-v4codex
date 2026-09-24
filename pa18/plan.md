# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Target: PA18 full-stage. Stage entry: **266/420**; handoff 63: **282/420**.
Loop 64 entry: `30a610065e586c481a094c60698f7ae8a7952904`, **282/420**.
Loop 64 handoff: **312/420**, 30 existing failures fixed, zero new failures.
Previous turn classification: progress (16 existing failures fixed). Review markers above remain unchanged.
Loop 65 entry: `52d972234f7f4dfdb9ec4c0969f559b9521bdf62`, **312/420**.
Previous goal turn: progress (loop 64's validated substitution/dependency work).
Loop 65 result: **327/420**, 15 existing failures fixed, zero new failures.

| Owner / group | Design, data flow and work |
|---|---|
| Function deduction and ordering (completed group) | Canonical type/parameter identities feed directional partial deduction; only participating call parameters (full signature for addresses). Reference/cv and pack tie rules select the declaration once; conversion facts carry it to typed LowIR. Scratch bindings live for one comparison; O(candidate count × compared type edges), no body demand, rendered keys, exception-based rejection or global scans. |
| Prototype packs / ABI (completed group) | Signature expansion and element expression types are distinct. Typed object/type lanes; structured indirect-query failure; direct `sizeof...` ABI graph. O(required lanes/query edges/output), TU-owned facts. |
| Immediate substitution / SFINAE (loop 64) | `specialize`/defaults and query/type owners unify explicit/deduced probing, propagate query/array/alias failures before access checks, preserve class/body hard errors. Incomplete prerequisites publish class→query→consumer edges; completion invalidates only affected queries/constants, and blocked alias/signature facts retry on demand. O(required type/query edges + notified dependency edges); canonical identity, TU storage, stack-local candidate state. |
| Conversion templates / retained heads (loop 65) | Target TypeId + indexed class conversions → return-only deduction/ordering → immediate defaults/signature → recorded object/result conversions → ordinary demand/LowIR. Reference/cv fallback, explicit calls, constructor ties and inherited hiding use canonical types. Retained non-type parameter types substitute left-to-right under head/outer frames. TU-owned facts; local candidate state; work follows candidate/type/query and lexical frame edges. |
| Retained member/lexical contexts and remaining expressions (unfinished) | Correlated outer/inner pack lanes, nested/member result/default owners; compound assignment, braced construction, destructor and remaining cast/access queries. These require new facts/checks, not broader exception suppression. |
| Constructor/explicit deduction and NTTP values (remaining) | Braced/explicit argument deduction, pointer/reference/static-member non-type value identities; preserve demand boundaries and member owners. |
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
Loop 65: [evidence](performance65.md) covers equivalent common binaries plus
new conversion/default/head costs, 600→2400 scaling, compiler latency/RSS and
checked runtime/text size. No optional optimizer is added.

Validation: run focused required fixtures and explicit personal controls,
`make test-pa18`, root through-PA17, and the PA18 file audit. Full through-PA18
must pass before advancement. No fixtures, references or coverage are waived.

| Handoff ledger | Implementation | Independent review |
|---|---|---|
| 63 | Ordering/address + prototype pack result/identity/ABI group complete through `4a49ea10`; [handoff boundary](handoff63.md). Remaining groups above are unfinished implementation. Required prior/file checks pass; PA18 282/420, 64 personal controls pass. | Last-reviewed remains the stage base. Ordering direction/context/cache and pack/query/ABI integration need independent review; whole-stage findings remain required before advancement. |
| 64 | Immediate substitution/query failure and completion-dependency group through `e01b8763`; [handoff boundary](handoff64.md). Prior 2609/2609, file audit pass; PA18 312/420. 33 substitution + 64 prior controls, ABI and completion scaling pass. Remaining 86 status/22 LowIR failures require the other owners above. | Review immediate vs side-effect failures, cache dependency/active-query lifecycle and ABI encodings. Earlier review markers remain open; this is an implementation handoff, not whole-stage certification. |
| 65 | Conversion deduction/selection, typed member head construction and constexpr reference-address consumption through `644dd88b`; [handoff](handoff65.md). One [proved oracle correction](reference-correction65.md) in `4eaf273d`. Prior 2609/2609 and file audit pass; PA18 327/420, same 420 tests. Remaining 69 status/24 LowIR failures are unfinished implementation. | Review return-only ordering, hiding identity, head-prefix frame validity and constant-reference proof. Earlier review markers remain open; full-stage review and a passing through-PA18 report remain required before advancement. |
