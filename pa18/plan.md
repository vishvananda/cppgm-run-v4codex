# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `3a883d10a27e41d1b126eaef05eaf0b454de1646`.
Target: **PA18 full-stage**, still unfinished. Previous goal turn: **progress**
(audit 66 fixed prerequisite validity and established full-range evidence).
Loop 67: **progress**, completed address NTTPs and the related outer-head,
pack, demand, ABI and internal-linkage paths; independent review remains pending.

Stage entry **266/420** → handoffs 63 **282**, 64 **312**, 65 **327**;
audit 66 **327** → handoff 67 **343/420**. This handoff fixes **16 original
failures**, with **zero regressions** and unchanged coverage. Earlier PAs pass
**2609/2609**; file audit passes with the same three inherited advisories.
[Audit 66](audit.md) retains the accumulated review and earlier ledger.

| Ownership group | Design/spec alignment and remaining implementation |
|---|---|
| Reviewed deduction, ordering, conversion and query demand | Canonical directional comparisons, nominated types, selected declarations/conversions, explicit class→query→consumer edges, local prerequisite revisions and observational ordering inspection. Preserve these owners. |
| Completed in 67: address NTTPs and outer-head defaults | Typed expression query → target conversion and C++11 address restrictions → canonical constant storage identity → immutable substitution frame → ordinary demand and typed ABI/LowIR. Explicit function template-ids, packs, reference categories, static-member demand and internal-linkage isolation are covered. Completed address conversions include access context in their keys; access failures return candidate state. No text keys, grammar replay or broad retries. |
| **Remaining: retained contexts, packs and expression validity** | Correlated outer/inner pack lanes, nested/member aliases/results/defaults and out-of-class ownership; compound assignment, braced construction, destructor, cast/access and selected-conversion validity. Extend the owning typed facts and structured failure results. |
| **Remaining: constructor/explicit deduction** | Braced/explicit arguments, ADL template-id participation, constructor and inherited-constructor participation. Address NTTP conversion is complete at this boundary; do not relax it to mask these failures. |
| **Remaining: LowIR initialization and result facts** | Constant/array initialization, bool/result metadata, class-result conventions. Executable agreement does not replace the required LowIR comparison. |

The remaining **53 status failures and 24 LowIR mismatches** are unfinished
required implementation, not review questions. Further work crosses into the
retained expression/member/constructor or initializer owners above: the address
argument path cannot supply their missing facts. The initial group was extended
through its related ownership edges before stopping; see [handoff67.md](handoff67.md).

Spec scope is **O0 LowIR**. Work tracks argument queries, required overload
candidates, immutable frame and demand edges; cached complete identity/linkage
queries are O(1) average. TU vectors/flat indexes own durable facts, local scratch
releases on return. [Performance 67](performance67.md) freezes A/B inputs/binaries,
A/A calibration, ABBA latency/RSS and checked runtime/size. Common executables
are identical; compiler `.text` grows 0.400%. No optional transform or own native
backend is added. No PA18 numerical ceiling is mandated; inherited PA17
**+15%, +16 MiB, 5.5×** targets remain diagnostics under spec §9. All earlier
[63](performance.md), [64](performance64.md), [65](performance65.md), and
[66](performance66.md) evidence remains. Graph bounds, correctness and coverage
remain required; native optimization/self-hosting remain PA24–PA34 work.

Reference corrections since stage base: [constant initialization in 65](reference-correction65.md)
and [one PA9 ABI substitution oracle in 67](reference-correction67.md), both with
reducers and contract proofs. No source, status, test count or comparison rule changes.

| Handoff ledger | Implementation and evidence | Independent review |
|---|---|---|
| 66 audit | stage base → `3a883d10`, **327/420**, prior **2609/2609**, file audit and stage-scoped performance accepted | Completed accumulated review; markers above preserved. |
| 67 implementation | `06211ad0` → `0b60ca52`; four coherent commits; **343/420**, prior **2609/2609**, file audit pass; 63 address, 13 ABI, 5 merged-source and 3 repeated-address controls and 16 repaired course cases; [hashed evidence](../student.tests/pa18/loop67-evidence.json) | Pending: review argument definition-demand, canonical reference/pack identity, outer-head frames, cached internal linkage and ABI proof as part of the whole-stage audit. No review obligation waived. |

Required next checks remain `make test-pa18`, `make test-report-through-pa17`
and the PA18 file audit; run root reports sequentially (shared `.test_counts`).
**Do not advance to PA19 until `make test-report-through-pa18` passes and the
independent whole-stage audit is resolved.**
