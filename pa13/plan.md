# PA13 implementation plan

Stage base commit: `823e929cdc74fedbb487973dcad33b8a4dce18f4`
Last reviewed commit: `823e929cdc74fedbb487973dcad33b8a4dce18f4`
Target: **pa13 full-stage**. Phase: **implement**.
Entry: **2/37 passing, 35 failing**; inherited PA1–PA12 pass.
Previous goal turn: completed PA12 evidence/commit is progress; PA13 entry is
verified from the clean worktree and Ralph's complete failure log.

## Design/spec alignment and remaining groups

| Owner | Data flow / complexity | Validation |
| --- | --- | --- |
| Semantic class completion | Interned member signature → inherited slots → override/final/pure/covariant facts. Indexed per-name candidates; work proportional to actual inherited slots and declarations. | All required rejection cases, overload/covariance personal controls. |
| Layout / ABI support | Class-owned polymorphism and base offset → typed layout → vtable/RTTI support globals; canonical class IDs, one emission per demanded class. | Root/derived/base-offset/local/header fixtures. |
| Call resolution / lowering | Selected declaration and explicit qualification → recorded dispatch slot → indirect call. No lookup or syntax replay in lowering. | Object/pointer/reference/qualified/temporary/member-pointer cases. |
| Lifetimes / deallocation | Existing constructor/destructor actions + vptr stores and deleting-entry selection. Class/entry identity preserves slot/action ordering. | All destructor/delete/cleanup cases; PA12 unchanged. |

O0 stage: no optional optimization introduced. Typed LowIR remains direct;
backend/runtime execution uses the supplied native backend only in test tools.
Native backend, templates and generalized RTTI are later-stage work.

## Performance evidence

Freeze stage-entry binary and fixed input hashes before edits. Measure compiler
latency/RSS with A/A and ABBA on equivalent correct inherited workloads, plus
new virtual workload correctness/latency/RSS/runtime/text size. Preserve raw
observations and binary/flag/input hashes. No speedup claim from smaller IR;
necessary semantic cost is disclosed, diagnostic targets are not new gates.

## Handoff ledger

| Group | Commit / result |
| --- | --- |
| Entry | `823e929c`: 2/37; no coverage or reference changes |
| Virtual declaration/signature/covariance ownership | 12/37; all required rejections and 12 personal controls pass; inherited 1584/1584 and file audit pass |

No handoff yet. Required final checks: `make test-pa13`,
`make test-report-through-pa13`, file audit, explicit personal controls,
performance evidence and committed clean worktree.
