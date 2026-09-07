# PA6 implementation plan

Target: **PA6 full-stage**. Phase: **implement**. Entry: **0/105**.
Stage base commit: `9249196518f45492822fb2e3da4eb5d82af0ed13`.
Last reviewed commit: `9249196518f45492822fb2e3da4eb5d82af0ed13`.
Preserve both review markers during implementation.

## Design / remaining behavior groups

| Owner | Data flow and spec alignment | Complexity / validation |
| --- | --- | --- |
| Semantic graph / driver | Streaming PA1–5 cursor -> single source-faithful graph; semantic facts attach to NodeId; TU-owned flat storage and canonical IDs; deterministic view only | One analysis per declaration; independent TU lifetime; PA5 + PA6 100 groups |
| Types / declarators | Intern structural types by typed keys; preserve source parameter types separately from adjusted signatures; array completion and alias reference collapse | Expected constant-time interning, linear declarator traversal; PA6 composed/array/function groups + API probes |
| Scopes / lookup | Interned-name indexes, explicit namespace/using edges, lexical parents and qualified scopes; entities separate from source declarations | Work proportional to relevant scope edges; no global invalidation; namespace/using/shadowing groups |
| Class / enum / constants | Deferred member bodies consume completed class facts; small typed integral evaluator consumes parsed nodes and literal payloads | Demand only required constants; short circuit; class/template/enum/constant/rejection groups |

## Performance policy

No executable output exists at PA6: generated runtime/text and self-hosting
are N/A. Record compiler latency, peak RSS, compiler text and phase/work
counters on fixed declaration/template/namespace/constant inputs. Freeze
binaries, flags and inputs; verify outputs; use A/A calibration and ABBA
for any comparative performance claim. PA5 preservation budget: <=10% wall
plus A/A noise, <=15% RSS +1 MiB, <=25% compiler text growth for the new semantic
surface. Fourfold PA6 input scaling budget: <6x wall, <5x RSS +1 MiB.
No optimization benefit is claimed without the complete evidence protocol.

## Validation / handoff ledger

- Entry inspection: clean worktree; PA6 dispatcher unimplemented, 105 existing
  failures. Prior PA5 audited complete (393/393 through), current external
  prior-through/file-audit checks pass. No earlier active implementation handle.
  This entry gathers evidence determining the implementation owners.
- Required final checks: `make test-pa6`, `make test-report-through-pa6`,
  `make test-report-through-pa5`, file audit for `dev/src`, explicit personal
  checks. Commit coherent increments; refresh this ledger and verify clean git.
- Remaining: all four groups and performance/architecture evidence.
- Handoff reason: work ongoing; initial plan is not a stopping boundary.
