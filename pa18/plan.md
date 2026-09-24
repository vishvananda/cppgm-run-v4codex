# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `3a883d10a27e41d1b126eaef05eaf0b454de1646`.
Target: **PA18 full-stage**, still unfinished. Previous goal turn: **progress**
(loop 68's committed implementation and measured validation). Loop 69 is
**progress**: entry `e09162fa` → implementation `f2a9d7a3`, reference proof
`6073dbc0`. Independent review of loops 67–69 remains pending.

Stage entry **266/420** → handoffs 63 **282**, 64 **312**, 65 **327**;
audit 66 **327** → handoff 67 **343** → 68 **348** → 69 **353/420**.
Loop 69 resolves **five original failures**, with **zero new failures** and
unchanged 420-source coverage/comparison rules. Four are compiler repairs; one
is a documented rejection-oracle correction. Earlier PAs pass **2609/2609**;
file audit passes with the same three inherited advisories.
[Audit 66](audit.md) retains the accumulated review and earlier ledger.

| Ownership group | Design/spec alignment and remaining implementation |
|---|---|
| Reviewed deduction, ordering, conversion and query demand | Canonical directional comparisons, nominated types, selected declarations/conversions, explicit class→query→consumer edges, local prerequisite revisions and observational ordering inspection. Preserve these owners. |
| Completed in 67: address NTTPs and outer-head defaults | Typed argument queries, canonical constant storage identity, immutable frames, demand and ABI/linkage. Review remains pending; [handoff 67](handoff67.md). |
| Completed in 68: concrete call deduction and array/base conversions | Partial explicit heads preserve typed non-type parameters; deduction precedes defaults. Braced arguments/explicit pack lanes consume selected list conversions and constexpr storage. Failed direct class matching traverses indexed base edges with isolated trial bindings and canonical-primary filtering. Array cv and reference-related conversions remain source-faithful. [Owners, bounds and validation](handoff68.md). |
| Completed in 69: retained assignment/destructor queries | Typed source operations, canonical substitution/failure facts, required operator candidates, implicit deletion and class completion edges. Concrete/retained compound assignment consumes selected reference conversion and arithmetic type exactly once. Destructor template-ids, receiver effects and ABI names retain source meaning. [Owners, bounds and validation](handoff69.md). |
| **Remaining: retained contexts, packs and expression validity** | Correlated outer/inner pack lanes, nested/member aliases/results/defaults and out-of-class ownership; retained braced query operands/conversion plans, cast/access and other selected-conversion validity. Extend typed facts and structured failure results. |
| **Remaining: constructor/explicit deduction** | Constructor and inherited-constructor participation, remaining member-template/alias contexts. Concrete list non-deduction and ordinary explicit ADL are repaired; the broader required groups remain. |
| **Remaining: LowIR initialization and result facts** | Constant/array initialization, bool/result metadata, class-result conventions and discarded-value loads. Executable agreement does not replace required LowIR comparison. |

The **43 status failures and 24 LowIR mismatches** are unfinished required
implementation, not review questions. [Handoff 69](handoff69.md) records the
extended group and concrete boundary: braced queries need a typed list plan
shared by conversion, narrowing, constant evaluation and exception effects;
source-only ListPlan inputs cannot supply this without forbidden fake nodes.
Nested/member result contexts and initialization facts have separate owners.

Spec scope is **O0 LowIR**. New work follows query/operand edges, language-required
candidates and destructor subobjects. Existing canonical TU facts/cache keys,
precise completion dependencies and local scratch remain; typed lowering records
avoid semantic reconstruction. [Performance 69](performance69.md) freezes A/B
inputs/binaries, A/A calibration, ABBA latency/RSS and checked runtime/size.
Equivalent generated outputs are byte-identical; no optimizer benefit is claimed.
No PA18 numerical ceiling is mandated. Historical PA17 **+15%, +16 MiB, 5.5×**
self-selected targets remain diagnostics under spec §9. Preserve earlier
[63](performance.md), [64](performance64.md), [65](performance65.md),
[66](performance66.md), [67](performance67.md), [68](performance68.md) evidence.
Correctness, coverage and graph-work bounds remain gates. Native optimization
and self-hosting remain PA24–PA34 obligations.

Reference corrections since stage base: [constant initialization in 65](reference-correction65.md),
[one PA9 ABI oracle in 67](reference-correction67.md), and
[pseudo-destructor receiver effects in 69](reference-correction69.md).
Loop 69 changes only that fixture's required rejection status, with a reducer,
N3485 proof and pinned-bundle observations; no coverage/comparison waiver.

| Handoff ledger | Implementation and evidence | Independent review |
|---|---|---|
| 66 audit | stage base → `3a883d10`, **327/420**, prior **2609/2609**, file audit and stage-scoped performance accepted | Completed accumulated review; markers above preserved. |
| 67 implementation | `06211ad0` → `0b60ca52`, **343/420**, prior/file audit pass; [hashed evidence](../student.tests/pa18/loop67-evidence.json) | Pending: argument definition-demand, canonical reference/pack identity, outer-head frames, internal linkage and ABI proof. |
| 68 implementation | `047215cf` → `bd1d7b4c`, three coherent code commits; **348/420**, prior/file audit pass; 56 new controls, 211 inherited controls and five repaired course inputs; [hashed evidence](../student.tests/pa18/loop68-evidence.json) | Pending: partial-frame identity/default timing, binding isolation across base alternatives, array/reference cv, constexpr temporary identity and performance evidence. No review obligation waived. |
| 69 implementation | `e09162fa` → `f2a9d7a3` plus `6073dbc0` oracle proof; **353/420**, prior/file audit pass; 330 semantic controls, four emitted ABI identities and five repaired course checks; [hashed evidence](../student.tests/pa18/loop69-evidence.json) | Pending: builtin ranking/single evaluation, destructor property/access separation and completion edges, ABI identity, reference proof and performance. No review obligation waived. |

Required checks: `make test-pa18`, `make test-report-through-pa17` and the PA18
file audit. Root reports run sequentially (shared `.test_counts`).
**Do not advance to PA19 until `make test-report-through-pa18` passes and the
independent whole-stage audit is resolved.**
