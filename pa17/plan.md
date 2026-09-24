# PA17 compact plan — final audit, loop 62

Stage base: `21748547a9e5befaae65e4fae120a63b3f9fcafb`.
Last reviewed implementation: `12140852`.
Previous checkpoint: `2290a7bf8b56bc33e6ad975a8ae714a341f55ce5`.
Target: **PA17 full-stage**. **Final Spec Alignment: complete for PA17/O0.**

| Completed owner | Final design / evidence |
|---|---|
| Source and identity | Streaming buffers/cursors; one parsed source graph; compact projected occurrences. Interned names/types/entities/argument tuples and immutable parent-linked frames. |
| Template entity graph | Alias/variable/class/member/friend templates, partial selection, current specialization, explicit specialization/instantiation and renamed definition heads share canonical owners. Indexed lookup, exact matching and deterministic O(C) winner verification. |
| Demand and validity | Separate declaration, definition, default, exception, layout, constant and emission states; precise queues/dependencies; completed-key caching. No body replay, global retries or broad invalidation. |
| Semantic consumers | Access/naming provenance, qualified receiver paths, constants, static initialization and transfer/lifetime actions are recorded once. Direct typed LowIR and ABI consume them. |
| Closures and cleanup | Unique occurrence/context closure entity, ordinary operator/adapter demand, canonical callable signatures, lexical `this`, shared full-expression suffixes. Final audit fixes parameter scope, contextual bool, fixed lookup and array/function signature adjustment. |
| Performance and ownership | TU vectors/slabs/flat maps; function scratch releases locally. Bounded O0 actions and eight-lane expansion fallbacks; existing constexpr limits. Frozen compiler and executable evidence, with all historical observations retained. |

The [final audit](audit.md) independently reconstructs all stage surfaces and
traces a partial/alias, out-of-class member template, closure exception/signature,
qualified constant receiver and live cleanup through typed LowIR to checked ELF.
Its [range manifest](../student.tests/pa17/final-range.json) covers **61 stage
commits / 104 implementation paths**, including **six commits / 36 paths** since
the last checkpoint. No accepted handoff remains unaudited.

Findings are fixed in `12140852`; [30 new controls](../student.tests/pa17/final_controls.py)
fail 16 at entry and pass all at final. All **734 PA17 personal controls**, PA9
API/roundtrip checks and the six prior proved reference corrections pass.
No contract fixtures, references, statuses, coverage or comparison rules changed
in this audit. Prior reference corrections retain their reducers, N3485 proof
and bundle revision in [storage-references.md](storage-references.md).

Validation: PA17 **343/343**; root through-PA17 **2609/2609**, all **17 stages**,
plus separately reported property controls (entry tracker aggregate **2633/2633**).
File audit **pass**, with three inherited advisory header-division warnings.
Fresh command logs, controls, code/tree hashes and trace are bound in
[final-evidence.json](../student.tests/pa17/final-evidence.json);
`python3 student.tests/pa17/verify_final.py` checks the record.

[Final performance evidence](final-performance.md) reports compiler latency/RSS,
checked runtime/text, A/A noise and ABBA spread against both audit entry and stage
base. Necessary new behavior gets final-only observations when entry is invalid.
PA17/O0 has no numerical ceiling; historical +15%, +16 MiB and 5.5× targets
remain diagnostics under spec §9. No correctness, coverage, mandated limit or
explicit work/growth bound is weakened. Native allocation/ELF optimization and
self-hosting remain owned by PA24–PA34; full deduction/SFINAE is PA18.

| Ledger | Status |
|---|---|
| 48, 52, 56 | Earlier checkpoint findings fixed; preserved audit/performance records. |
| 57–60 | Transfer/query/storage implementation and accumulated audit; six proved reference corrections, three course failures still open at checkpoint 60. |
| 61 | Closure/full-expression implementation closed all course failures; [handoff](handoff61.md) and [old plan](plan-loop61.md) preserved. |
| 62 | Independent full-stage audit, shared closure signature/exception fixes, 734 controls, required checks, architecture/trace/performance review and final consolidation complete. |

Remaining PA17 work, defect, blocker or unaudited handoff: **none**.
Intended compiler changes and audit artifacts are committed; final repository
status must be empty before this goal is marked complete.
