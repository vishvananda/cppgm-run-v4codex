# PA7 final plan and ledger

Target: **PA7 full-stage**. Phase: **complete**.
Stage base commit: `14f1d402fe54e9d2fe5cc9a557b1bddde7ac10fe`.
Last reviewed commit: `14f1d402fe54e9d2fe5cc9a557b1bddde7ac10fe`.
Entry: **0/186**. Final: **186/186**; PA1–7 **684/684**.
Both review markers are preserved for Ralph; [audit.md](audit.md) records the
implementation's own source/fact traces, not an independent review.

## Design/spec alignment

One streamed frontend and one source graph carry canonical TypeId/EntityId/
ScopeId facts. No token replay, semantic tree copy, textual phase transport,
reference delegation or host compilation implements source semantics.

| Completed owner/group | Data flow, complexity and validation |
| --- | --- |
| Declaration/lookup | Flat scope/name and family/signature indexes -> immutable overload unions -> required candidates. Lexical/using edges only; redeclaration, scope and namespace fixtures pass. |
| Expression/conversion | NodeId -> type/category/constant/selection and separate incoming/outgoing conversion facts. One analysis per node, type-depth qualification work, linear candidate tournament plus verification. Operators, initialization, returns and indirect calls pass. |
| Statements | Source-order bindings and explicit condition/substatement scopes; linear control-flow traversal. Conditions, loops, switches and required rejections pass. |
| Members/template intake | Typed member-pointer and object-action identities; deduplicated body demand. Retained declaration patterns, canonical argument packs and separate declaration/emission states. Closed types reuse TU-owned dependence facts. All required intake fixtures and the fact API pass. |
| Output/driver | Deterministic view of recorded facts; PA5/6 adapters remain intact. Full unchanged PA7 corpus and multi-primary isolation pass. |

General class/template semantics, LowIR, optimization, native emission and
self-hosting remain later milestones. Remaining **PA7** behavior groups: **none**.

## Performance and validation

[performance.md](performance.md) identifies the final frozen binary and all
908 observations across the checkpoint/final campaigns. Both complete verifiers
pass unchanged budgets: wall <=10% + A/A noise; RSS <=20% +1 MiB; host `.text`
<=50% growth; 4x input <6x wall / <5x RSS +1 MiB. Final inherited pairs range
from 0.8% faster to 4.8% slower; maximum RSS growth is 5.3%. PA7 wall scaling is
3.925–4.051x. Host `.text` is 349,894 bytes (+26.00%). No speedup is claimed;
executable runtime/text are N/A. Repeated template inputs retain two
specializations and compute five type-dependence facts at both sizes.

Final serial checks pass: `make test-pa7` (186/186), prior-through PA6 (498/498),
`make test-report-through-pa7` (684/684), and file audit (79 files). One overlapped
report attempt produced harness-file races; its results were discarded and both
reports rerun serially with the full counts above. No coverage was reduced.
ASan/UBSan/leak checks pass the PA7 fixtures, 29 personal cases, multi-TU case,
PA7 fact API, PA5/6 APIs and 105 PA6 fixtures. Inherited personal suites pass.

## Handoff ledger

| Commit | Outcome |
| --- | --- |
| `c559848bd` | Recorded stage base/review markers before implementation; 186 stub failures. |
| `75e512523` | Procedural owners complete: 170/186; earlier PAs and file audit pass. Continued related work. |
| `6690406f2` | Member identities, demand and template declaration selection: 186/186; first full frozen campaign passes. |
| `e5e777a65` | Closed-type dependence cache and architecture audit; 684/684, sanitizer/API checks and final frozen campaign pass. |
| Final evidence | Full serial exit gates and both performance verifiers pass; intended changes committed and clean worktree verified at handoff. |

Handoff reason: **PA7 complete**. No incomplete group or progress-minimum stop.
