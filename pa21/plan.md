# PA21 compact plan — checkpoint audit 109

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `f57bdd3b0e5c7dd2dd87aacaecc73c9ddc96d114`.
Target: **PA21 full-stage**. Phase: **checkpointAudit complete; implementation remains**.
Entry: clean `e6582e37`, **108/116 passing, eight failing**.
Current: **108/116**, exactly the same eight failures. Earlier PAs **3596/3596**;
through PA21 **3704/3712**. File audit passes; coverage and comparisons unchanged.
PA21 completion and advancement still require a passing through-stage report.

## Reviewed design and repair

[Audit 109](audit.md) reviews the entire `f65eae8d..f57bdd3b` range: 14 entry
commits, 41 implementation paths and the audit fix. The [105 record](audit105.md)
and all 102–108 correctness/performance evidence are preserved.

| Owner | Facts and bounded work |
|---|---|
| Source EH and jumps | Selected exception conversions, handler objects and lexical destinations feed typed regions. Audit repair records sparse jump target identities, closes exited try/handler scopes in order and uses the remaining context for throwing outer cleanup. Illegal protected entry is rejected; nested labels remain reachable. |
| Full expressions and destinations | Final consumers, default arguments, named results and completed subobject prefixes retain explicit ownership. Immutable rebasing preserves earlier failure snapshots; complete context/terminal identities govern suffix sharing. |
| Construction and helper transport | Canonical initializer/constructor identities own nonthrowing and independence facts. Alias/effect/throwing uncertainty keeps ordered lowering; arrays expand only through eight elements, then use loops. |
| Phase boundaries | Streaming source cursor, retained template occurrences, typed semantic facts and direct LowIR remain the production path. TU/function owners release transient state; supplied native backends are validation only. |

New execution/rejection controls improve **7/22 → 22/22**. They cover ordinary
and nested try/handler exits, loops/switches/ranges, catch-object retirement,
throwing outer destruction, template lambdas and labels. Semantic jump ancestry
supplies the facts once; lowering does not rediscover them from names or searches.
The source-to-ELF trace checks two demanded template bodies, lists, captures,
RTTI and handler `continue`; ordinary/instrumented LowIR is identical.

## Remaining implementation

- **EH/lifetime and support identity (five comparisons):** source nested
  catch-miss, guarded-static initialization and handler-context continuations;
  global and local initializer-list backing storage/lifetimes. Complete region,
  handler-exit and concrete backing/support identities together.
- **Generated construction and template lowering (three comparisons):**
  template-member temporary cleanup and shared-call hidden temporary actions;
  constant-array O0 policy for function-template/local-class specialization,
  preserving the PA17 ordinary-array contract.

The [validation record](../student.tests/pa21/audit109-validation.json) retains
all eight exact failing paths. Passing execution controls do not waive required
LowIR comparisons. The accepted handoffs improved 71 → 92 → 101 → 108 cases,
but split closely related exit/consumer/destination owners too often. The missed
jump paths expose avoidable fragmentation. Complete these broad groups with
cross-feature validation rather than separate fixture-shaped handoffs.

## Validation and stage-scoped performance

Required earlier/file/progress gates pass. All **116** source cases and the
**15048-path** contract/harness inventory match checkpoint 108. Personal controls
pass **580/581**, with only the inherited supplied freestanding RTTI discrepancy;
its host-runtime counterpart passes. Proved reference corrections 102, 106 and
108 were reconstructed and checked; no reference changed in this audit.

[Performance 109](performance109.md) freezes the last-reviewed and final binaries
and inputs, with A/A noise calibration, four ABBA blocks, separate compiler/RSS
and checked executable runtime/text measurements. It also measures new jump
semantics independently of invalid entry output. Required O0 costs and later
backend constraints are disclosed; prior measurements remain intact. Historical
**+15%, +16 MiB, 5.5×** diagnostics add no exit gate under spec §9. Correctness,
coverage, comparison rules and the eight-element expansion bound remain binding.
Native optimization/debug encoding and self-hosting remain later-stage owners.

## Handoff ledger

102–104: RTTI/casts, captures/copies, lists/demand; **71/116**.
105: audit through `f65eae8d`, recorded by `06b2d989`; 45 failures retained.
106: `9c4f64da` through `c64e88fb`; EH/lifetimes, **92/116**.
107: `b821682b`, `20476a77`; full expressions/defaults/consumers, **101/116**.
108: `2e392cec`, `a30acab5`; destination and partial-construction ownership,
**108/116**; evidence through `e6582e37`.
109: audit through `f57bdd3b`; complete accumulated range reviewed; protected
jump ownership repaired; all checkpoint gates pass, eight failures unchanged.
Record commit follows the code tip without further implementation edits.
