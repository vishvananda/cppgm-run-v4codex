# PA17 compact plan — implementation handoff, loop 61

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `2290a7bf8b56bc33e6ad975a8ae714a341f55ce5`

Target: **PA17 full-stage**. Entry `9efd570f` was clean at **340/343**;
all three original failures now pass, with unchanged course coverage. This is an
implementation handoff; Ralph's independent full-stage audit remains pending.
The stage base and last-reviewed markers above are preserved.

| Completed owner | Design/spec alignment and data flow | Work bound / validation |
|---|---|---|
| Full-expression cleanup | Semantic call effects and live lifetime suffix → expression region → typed LowIR. Calls with live locals share the full-expression region; noexcept temporary activation retains the required structural regions. Closure argument storage precedes call protection after receiver activation. | Memoized expression walks plus shared cleanup suffixes, proportional to expressions/actions. Two original failures and 10 native/5 structural controls. |
| Closure entities and signatures | Parsed occurrence + substitution context → unique class/call operator and retained body → ordinary value/call lowering. Source binding retains lookup; specialization publishes raw parameter types (including packs), return and noexcept facts. Function-pointer adapters demand the same operator body. | One closure per occurrence/context; one body check per specialization; flat entity indices; one O(L log L) ABI numbering pass. Original chained nonprimary failure and closure controls. |
| Placeholder initialization | Canonical pending object → initializer type → existing typed deduction/substitution → declaration. Pending identity prevents shadowed self-reference; multi-declarator deductions must agree. | Proportional to typed declarator/initializer; no text reconstruction. Function-address selection preserves access/deletion checks. |
| ABI publication | Typed closure context/signature → Itanium first/numbered discriminator → existing typed graph/encoder. Inspection facts preserve both the unsuffixed first closure and existing numeric discriminator forms. | ABI exact/roundtrip/source checks plus earlier PA9 contracts. No reference changes. |

Captureless closure support covers the PA17 fixture and adjacent consumers;
captures and initializer-list deduction are not introduced. No native backend,
optimization level or self-hosting surface is added (PA17 still uses the supplied
backend for behavioral validation). Remaining known implementation in the two
original groups: **none**. Independent review must assess whole-stage correctness
and architecture; a passing implementation handoff does not waive that audit.

Performance evidence: frozen entry/final compiler comparison is being collected
by `student.tests/pa17/handoff61_benchmark.py`. It includes fixed common inputs,
cleanup/closure scaling, A/A + ABBA compiler latency/RSS, and checked executable
runtime/text. Entry-rejected closure cases get final-only observations, never
speedup claims. PA17/O0 has no mandated numeric ceiling or optional transform.
Historical +15%, +16 MiB and 5.5× gates remain diagnostics under spec.md §9;
all prior measurements and mandated correctness/coverage limits are preserved.

| Handoff ledger | Status |
|---|---|
| 56 / checkpointAudit | `c43e8eb6..e14b96fa`; 324/343; [previous audit](audit-loop56.md). |
| 57–59 / implement | Transfers, queries and storage; 324 → 330 → 333 → 340 / 343, including six proved reference corrections. |
| 60 / checkpointAudit | `e14b96fa..2290a7bf`; accumulated ownership fixes; 340/343, prior 2266/2266, 610 controls; [audit](audit.md), [performance](checkpoint60-performance.md), [evidence](../student.tests/pa17/checkpoint60-evidence.json). |
| 61 / implement | `9efd570f` entry; `2827d328` closes cleanup owner, closure increment closes the final failure. PA17 343/343 and prior 2266/2266; final evidence refresh pending. |

Handoff boundary: both remaining semantic owners are complete and reviewable
as one implementation turn. Prior whole-stage findings/measurements remain in
the linked records; independent full-stage audit is the next phase, not another
unresolved implementation group.
