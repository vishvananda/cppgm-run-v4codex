# PA17 compact plan — checkpoint audit, loop 60

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `2290a7bf8b56bc33e6ad975a8ae714a341f55ce5`

Target: **PA17 full-stage**. Checkpoint audit complete; implementation remains
incomplete and must not advance. Entry `119fa9fe` was clean at **340/343 with
three failures** (make exit code 2). Reviewed all **15 commits / 40 implementation
paths** in `e14b96fa..2290a7bf`, including all three handoffs, two audit
implementation fixes and the evidence verifier correction. Benchmarks at `3ad8f2f4` have identical compiler sources.
Previous goal turn: progress established by the committed storage handoff;
no prior live compiler/test process remained.

| Completed owner / audit correction | Design and evidence |
|---|---|
| Transfer and automatic initialization | Canonical base graph/layout facts; bounded short-array stores and evaluated materializations; sparse memberwise transfer. Qualified calls now consume both selected receiver segments. |
| Candidate and query facts | Complete active head/tuple keys, structured immediate failures, deletion ownership, source/instantiation snapshots, signature-only normalization. Naming/access provenance and typed member qualifiers survive substitution and ABI adaptation. |
| Static definition and initializer demand | Prototype/signature/definition links and entity-keyed storage demand; typed function-address relocations. Six prior oracle corrections independently verified with reducers and standard proof. |
| Constant and emission consumers | Constant calls/query fields consume recorded subobject paths. First runtime demand activates only deferred selected-callee edges of constexpr-checked bodies; deeper unevaluated operands and unused bodies stay dormant. |

Final checks: PA17 **340/343**, exactly the same three failing inputs; prior
**2266/2266**; through PA17 **2606/2609**. File audit passes with the same three
inherited warnings. All **610 personal controls** pass: 567 inherited plus 43
new controls (entry fails 18/43). Course coverage remains 343 cases, with no source,
status or comparison changes. The native trace and ABI qualifier checks pass.

[Audit and ledger](audit.md), [range](../student.tests/pa17/checkpoint60-range.json),
[performance](checkpoint60-performance.md), and
[evidence](../student.tests/pa17/checkpoint60-evidence.json) bind the reviewed tip.
Historical transfer/query/storage handoffs and measurements remain preserved,
including the first audit campaign before constant/emission completion. PA17/O0
has no mandated numerical ceiling. Historical +15%, +16 MiB and 5.5× targets
remain diagnostics under spec.md §9. Necessary semantic/contract costs and later
backend constraints do not add exit gates; correctness, coverage, bounded work
and avoidance of unnecessary regressions remain required. No optional transform
or broader optimization level was introduced. Audit common compiler ratios span
0.9561–1.0336 (RSS −252 to +588 KiB); cumulative ratios span 0.9761–1.0378
(RSS −140 to +2,324 KiB). Sparse transfer and eight-element array runtime benefits
repeat; the required static-read boundary costs about 2% runtime and eight bytes.

| Remaining implementation group | Failures | Required next work |
|---|---:|---|
| Closure entities | 1 | Closure identity, call-operator/body binding, class value and lifetime/lowering facts. |
| Exception cleanup scheduling | 2 | Call-region boundaries and temporary/automatic cleanup order. |

All accumulated independent review questions are closed. These three behavior
failures remain unfinished implementation, not waivers. Avoidable handoff
fragmentation split shared receiver, query and emission ownership across three
increments, and the initial audit fix missed the constant consumer. Close broad
owners together with their declaration, substitution, query, constant, emission,
ABI and lowering consumers in future handoffs.

| Loop / phase | Checkpoint ledger |
|---|---|
| 56 / checkpointAudit | `c43e8eb6..e14b96fa`; 324/343; [previous audit](audit-loop56.md) preserved. |
| 57–59 / implement | Transfers, queries and storage; 324 → 330 → 333 → 340 / 343; three accepted handoffs, including six proved reference corrections. |
| 60 / checkpointAudit | `e14b96fa..2290a7bf`; full accumulated review and ownership fixes; 340/343 unchanged, earlier 2266/2266, 610 controls, file audit and stage-scoped evidence pass. Two implementation groups remain. |

Run `python3 student.tests/pa17/verify_checkpoint60.py`. Historical verifiers
remain records of their frozen tips; the review markers above are the next audit
baseline. The subsequent records commit contains no implementation edits.
