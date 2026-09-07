# PA6 implementation plan

Target: **PA6 full-stage**. Phase: **implement**. Entry: **0/105**. Current: **105/105**, through **498/498**.
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
plus A/A noise, <=15% RSS +1 MiB, <=35% compiler text growth for the new semantic
surface. The initial 25% forecast was exceeded by the required semantic
implementation (32.23% before timing); this revised feature budget is frozen
before the final campaign. It is not an optimization profitability claim.
Incremental signature/fact work must stay within 10% + A/A wall noise,
15% RSS +1 MiB and 5% text relative to the first complete PA6 implementation. Fourfold PA6 input scaling budget: <6x wall, <5x RSS +1 MiB.
No optimization benefit is claimed without the complete evidence protocol.

## Validation / handoff ledger

- Entry inspection: clean worktree; PA6 dispatcher unimplemented, 105 existing
  failures. Prior PA5 audited complete (393/393 through), current external
  prior-through/file-audit checks pass. No earlier active implementation handle.
  This entry gathers evidence determining the implementation owners.
- Required final checks: `make test-pa6`, `make test-report-through-pa6`,
  `make test-report-through-pa5`, file audit for `dev/src`, explicit personal
  checks. Commit coherent increments; refresh this ledger and verify clean git.
- First implementation: canonical type/scope/entity records, declaration-time
  graph extension, namespace/import lookup, composed types, deferred class
  bodies, enums/constants and dump. First run 101/105; four ownership fixes
  reach 105/105; fresh PA1–6 through 498/498. File audit: 71 files pass.
- Audit expansion: anchored/cyclic/transitive using lookup, qualified scope
  names, injected-class identity, enum definition-time constant transitions,
  scoped conversions, aliases, plain class layout, signature/constant fact
  reuse. Personal behavior 28/28; identity/lifetime API and inherited PA5 API
  pass sanitizers before the last fact-owner changes. Fresh through 498/498.
- Declaration-point audit found that only class bodies may be deferred.
  Free functions now analyze immediately; local classes own queue intervals.
  Five added probes cover free, class, local-class and nested-class contexts
  and keep anonymous object presentation names out of qualifier lookup.
  The first timing campaign was deliberately stopped on this correctness
  finding; its partial observations are retained and make no final claim.
- Corrected implementation: PA6 105/105, through 498/498, 33 personal cases,
  both identity/lifetime APIs and all 105 course cases pass the final isolated
  ASan/UBSan/leak build. Definition and body-scope identities are retained.
  File audit passes 71 files.
- Frozen `3f6de92f5` campaign: all output/protocol/latency/text checks hold,
  but largest template RSS is ~20% above the first PA6 binary, exceeding the
  unchanged 15% +1 MiB budget. Retain every observation in
  `student.tests/pa6/pre-compact-performance.json`.
- Class-only demand state now lives in a separate indexed arena; packed common
  entity and constant records preserve all facts without making every binding
  carry layout/constructor state. No budget was relaxed for this correction.
- Compact records: Entity 96 ->56 bytes, Constant 24 ->16 bytes; class-only
  state is one 32-byte record per class. Fresh 105/105, 498/498, 33 personal
  checks, both APIs and all course fixtures pass ASan/UBSan/leak checks;
  file audit remains green.
- Remaining: final frozen timing rerun.
- Handoff reason: work ongoing; initial plan is not a stopping boundary.
