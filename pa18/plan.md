# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `3a883d10a27e41d1b126eaef05eaf0b454de1646`.
Target: **PA18 full-stage**, still unfinished. Previous goal turn: **progress**
(loop 67's committed implementation and validated evidence). Loop 68: **progress**,
entry `047215cf` → code `bd1d7b4c`; independent review remains pending.

Loop 69 in progress, entry `e09162fa`, **348/420** (72 failures). Previous
turn is progress: committed implementation and measured validation changed the
authoritative state. Stage/review markers above are preserved. Initial group:
retained expression validity (`type_query`, `query_operator`, builtin operators,
member/destructor lookup and list initialization). Source query IDs and typed
children flow through substitution to cached candidate facts, then selected
conversions/constant execution; no syntax replay or fake nodes. Work must follow
query edges, required candidates and list elements; scratch is query-local and
durable facts TU-owned. Validate repaired course cases, explicit personal
positive/negative controls, inherited controls and frozen A/B latency/RSS plus
checked runtime/size. Extend related fixes while these owners support them.

Stage entry **266/420** → handoffs 63 **282**, 64 **312**, 65 **327**;
audit 66 **327** → handoff 67 **343** → handoff 68 **348/420**.
Loop 68 fixes **five original failures**, with **zero new failures** and unchanged
420-test coverage, references and comparison rules. Earlier PAs pass **2609/2609**;
file audit passes with the same three inherited advisories.
[Audit 66](audit.md) retains the accumulated review and earlier ledger.

| Ownership group | Design/spec alignment and remaining implementation |
|---|---|
| Reviewed deduction, ordering, conversion and query demand | Canonical directional comparisons, nominated types, selected declarations/conversions, explicit class→query→consumer edges, local prerequisite revisions and observational ordering inspection. Preserve these owners. |
| Completed in 67: address NTTPs and outer-head defaults | Typed argument queries, canonical constant storage identity, immutable frames, demand and ABI/linkage. Review remains pending; [handoff 67](handoff67.md). |
| Completed in 68: concrete call deduction and array/base conversions | Partial explicit heads preserve typed non-type parameters; deduction precedes defaults. Braced arguments/explicit pack lanes consume selected list conversions and constexpr storage. Failed direct class matching traverses indexed base edges with isolated trial bindings and canonical-primary filtering. Array cv and reference-related conversions remain source-faithful. [Owners, bounds and validation](handoff68.md). |
| **Remaining: retained contexts, packs and expression validity** | Correlated outer/inner pack lanes, nested/member aliases/results/defaults and out-of-class ownership; retained braced query operands, compound assignment, destructor, cast/access and selected-conversion validity. Extend typed facts and structured failure results. |
| **Remaining: constructor/explicit deduction** | Constructor and inherited-constructor participation, remaining member-template/alias contexts. Concrete list non-deduction and ordinary explicit ADL are repaired; the broader required groups remain. |
| **Remaining: LowIR initialization and result facts** | Constant/array initialization, bool/result metadata, class-result conventions and discarded-value loads. Executable agreement does not replace required LowIR comparison. |

The **48 status failures and 24 LowIR mismatches** are unfinished required
implementation, not review questions. One former base-deduction rejection now
compiles but remains an IR mismatch and is **not** counted as a passing test.
Further related work crosses into missing retained list/member/alias contexts
or initialization/result owners; concrete deduction cannot supply their missing
facts. [Handoff 68](handoff68.md) records this boundary, the extended group and
a retained probe of later virtual-base identity/layout work.

Spec scope is **O0 LowIR**. Work tracks head/argument/query edges and matching
base candidates, not unrelated declarations. TU vectors/flat indexes own durable
facts; candidate bindings and graph traversal scratch release locally. The new
base search is O(V+E) plus matching-argument work, with primary filtering before
binding copies. Partial frames and query substitution use complete typed keys.
[Performance 68](performance68.md) freezes A/B inputs/binaries, A/A calibration,
ABBA latency/RSS and checked runtime/size. No optional optimizer or native backend
is added. No PA18 numerical ceiling is mandated; inherited PA17 **+15%, +16 MiB,
5.5×** targets remain diagnostics under spec §9. All earlier
[63](performance.md), [64](performance64.md), [65](performance65.md),
[66](performance66.md), and [67](performance67.md) measurements remain.
Graph bounds, correctness and coverage remain required; native optimization and
self-hosting remain PA24–PA34 work.

Reference corrections since stage base: [constant initialization in 65](reference-correction65.md)
and [one PA9 ABI oracle in 67](reference-correction67.md). Loop 68 changes none.

| Handoff ledger | Implementation and evidence | Independent review |
|---|---|---|
| 66 audit | stage base → `3a883d10`, **327/420**, prior **2609/2609**, file audit and stage-scoped performance accepted | Completed accumulated review; markers above preserved. |
| 67 implementation | `06211ad0` → `0b60ca52`, **343/420**, prior/file audit pass; [hashed evidence](../student.tests/pa18/loop67-evidence.json) | Pending: argument definition-demand, canonical reference/pack identity, outer-head frames, internal linkage and ABI proof. |
| 68 implementation | `047215cf` → `bd1d7b4c`, three coherent code commits; **348/420**, prior/file audit pass; 56 new controls, 211 inherited controls and five repaired course inputs; [hashed evidence](../student.tests/pa18/loop68-evidence.json) | Pending: partial-frame identity/default timing, binding isolation across base alternatives, array/reference cv, constexpr temporary identity and performance evidence. No review obligation waived. |

Required checks: `make test-pa18`, `make test-report-through-pa17` and the PA18
file audit. Root reports run sequentially (shared `.test_counts`).
**Do not advance to PA19 until `make test-report-through-pa18` passes and the
independent whole-stage audit is resolved.**
