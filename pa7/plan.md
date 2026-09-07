# PA7 implementation plan

Target: **PA7 full-stage**. Phase: **implement**.
Stage base commit: `14f1d402fe54e9d2fe5cc9a557b1bddde7ac10fe`.
Last reviewed commit: `14f1d402fe54e9d2fe5cc9a557b1bddde7ac10fe`.
Entry: **0/186**, all stop at the dispatcher stub. PA1–6: **498/498**.
Previous goal turn: no implementation progress; current logs confirm the stub.

## Design and remaining groups

Extend the parser-consumer's single source graph with compact typed facts;
retain canonical TypeId/EntityId/ScopeId keys and TU-owned flat indexes/arenas.
No token replay, semantic tree copy, string identity, or external compilation.

| Group / owner | Data flow and expected work | Validation |
| --- | --- | --- |
| Declaration/lookup | Canonical function signatures -> overload bindings; lexical/using edges -> candidate IDs. Indexed name/kind lookup; visit required edges/candidates only. | Redeclarations, namespaces, using, target function names |
| Expression/conversion | Source nodes -> type/category/constant/selected declaration and conversion facts. One analysis per node; linear operand/type-depth work. | Operators, pointers/references, casts, calls, initializers/returns |
| Statement scopes/control | Source-order declarations -> nested scopes and typed conditions; explicit loop/switch context. Linear statement work. | Local declarations, branches/loops/switches, required rejection |
| Rendering/driver | Recorded facts -> deterministic PA7 view, preserving PA5/6 adapters. Linear graph/output work. | All 186 unchanged fixtures, multifile and personal graph checks |

Candidate failure is a compact conversion result; final invalid programs may
diagnose once. Preserve source syntax for later class/template extension and
record selected function/conversions for future direct lowering.

## Performance evidence

No speedup or executable optimization claim. PA7 emits no executable; runtime
and generated text size are N/A. Freeze baseline/final binaries and inputs for
compiler wall/RSS/host-text measurements; preserve observations, A/A calibration
and ABBA paired spread. Budget: inherited PA6 workloads <=10% wall + A/A noise,
<=20% RSS +1 MiB; 4x PA7 input <6x wall and <5x RSS +1 MiB; host text <=50%
growth for the new semantic feature. Any needed budget revision must precede
the final measurement and state its reason.

## Handoff ledger

| Increment | Evidence / remaining boundary |
| --- | --- |
| Initial inspection | Read AGENTS, testing guide, spec, PA7 handout and representative fixtures. Preserve both review markers above. All 186 failures remain; implementation in progress. |

Handoff reason: none; active implementation. Required final checks are
`make test-pa7`, `make test-report-through-pa7`, prior-through PA6 and
`perl scripts/cppgm_file_audit.pl --stage pa7 --paths dev/src`. Commit intended
changes and verify an empty worktree at handoff.
