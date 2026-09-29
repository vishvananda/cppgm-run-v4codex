# PA21 compact plan — implementation 107

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `f65eae8d7d434735a0ce981173a347eb8f8a1e59`.
Target: **PA21 full-stage**. Phase: **implementation**.
107 entry: clean `1422565795ca5d689fe63bbfbaaba7afa0607858`, **92/116 pass, 24 fail**. Current: **92/116 pass,
24 fail**; all 116 original inputs retained, no new required failures.
Earlier PAs: **3596/3596**. This is an implementation handoff, not PA21 completion.

## Design/spec alignment

One cumulative typed frontend and direct LowIR builder remain the implementation.
The [105 audit](audit.md) covers earlier RTTI/capture/list work and its repaired
copied-subobject cleanup. Keep their measurements and [reference proof](reference-corrections102.md).
106 adds executed throw/try/catch semantics and integrates their lifetime owners:

| Owner and data flow | Work bound and validation |
|---|---|
| `semantic/source_exception.cpp`: adjusted thrown type, selected initialization/destructor and catch parameter conversion → retained facts → lowering. Local implicit moves respect the nearest try scope; template handlers publish local bindings. | One record per throw/handler; canonical type/entity keys; lexical scope walk only for language-required move eligibility. Explicit move-only, outer-scope copy, named/unnamed class catch, rethrow and copy-failure termination controls. |
| `lowering/source_exception.cpp`: exception ABI calls, source catch matching, handler entry/exit and operand retirement consume those facts. Pointer-reference catches own pointer storage. | Function-owned context identities; clauses follow actual enclosing handlers and emitted output. 36 explicit source/host-ABI controls, including required source tests and two reduced nested misses. |
| `exception_continuation.cpp` / `cleanup.cpp`: live prefix + complete try/handler context → immutable cleanup suffix + region/handler exits + terminal. Constructor subobjects share reverse suffixes. | Expected O(1) interning per distinct state/terminal; iterative suffix creation; one emitted cleanup per cached suffix. O(b log b) final presentation of newly emitted blocks. No source text/IR-string cache keys. Constructor/destructor prefix, branch escape, cross-function and sibling-throw controls. |

Two checked references omitted required nested-catch clauses/region exits.
[Proof and bundle revision](reference-corrections106.md) reconstruct six added
LowIR lines from the stage base, independently of student output; original and
revised references execute through the pinned object backend. Sources, catches,
destructions and comparison rules are preserved. Supplied standalone backend
RTTI/alias failures are also reproduced on untouched references; host-runtime
controls supplement, and do not waive, required LowIR comparisons.

## Remaining implementation groups and boundary

- **Shared full-expression/value lowering:** complete condition/logical temporary
  regions, result slots, const-reference prvalues, default arguments, range
  increments, guarded statics, parameter prologues and initializer-list backing
  regions. Existing behavioral controls do not waive the remaining LowIR shape.
- **Aggregate/class lowering:** nontrivial aggregate member construction helpers,
  empty aggregate result/fallthrough, value-argument ownership and local class
  identity inside function/nested class template specializations.
- **Function EH boundaries and residual source-EH shape:** implicit/explicit
  nonthrowing body termination wrappers; nested handler region and suffix
  presentation; ambiguous RTTI/global alias pairing. These are unfinished
  implementation, not requests for an audit to excuse failures. Unevaluated
  throw query coverage remains with the query owner.

The completed increment establishes source exception initialization/binding and
its runtime lifetime transitions, extending through class catch copies and default-argument retirement, implicit
moves, pointer references and constructor/destructor prefix composition. Further
failures require changes to the shared value/full-expression scheduler and body
effect ownership. A broad nonthrowing-temporary-region change disturbed PA12
`300-conditional-local-prvalue-init-elides-copy` without improving PA21's pass
count; it was removed. Resolving that cross-stage scheduling contract is the
concrete next implementation boundary, not another small source-handler patch.
All remaining required cases stay open and retain their original coverage.

Independent audit questions: review the cumulative context/terminal key and
operand-retirement compositions, catch ABI facts and six-line oracle proof.
Those questions are separate from the known implementation failures above.

## Validation and performance

[Validation](../student.tests/pa21/validation106.json) records exact before/after
failure sets, root prior-through and stage checks, passing file audit (three
inherited header advisories), explicit personal suites and a 15048-path contract
inventory. The 36 new source-EH controls pass. Across all 376 personal controls,
the only discrepancy is the inherited standalone-runtime RTTI case; its host
runtime counterpart passes. Only the historical RTTI correction and two proved nested-catch
references differ from stage base. No advancement until the through-PA21 report
passes. The final [through report](../student.tests/pa21/through106.json) is
**3688/3712**, with exactly the same 24 PA21 failures. Final performance evidence is recorded in [performance106.md](performance106.md).

PA21/O0 applies spec §9: work tracks consumed/produced facts and IR; the existing
eight-element array expansion cap remains. There is no optional optimizer here.
Historical +15%, +16 MiB and 5.5× diagnostic targets remain measurements rather
than extra exit gates. New semantic costs need measurement, not a speedup claim
against an entry compiler that rejects the source. Native optimization and ELF
remain later-stage work. Earlier 102–105 measurements are preserved.

## Handoff ledger

102: `30fe6353`, `9f2181f9`, `f4224e0b` — RTTI/casts, 37/116.
103: `e835d6dc`, `1c541f84`, `fa079cde` — captures/copies, 49/116.
104: `e2af8963`, `f03b9371`, `511fe9b9`, `3b87e462` — lists/demand, 71/116.
105: audit through `f65eae8d`, recorded by `06b2d989`; accumulated range reviewed,
RTTI reuse and copied-subobject cleanup repaired, 45 required failures retained.
106: `9c4f64da`, `301ef6fd`, `e80ad0c7`, `c64e88fb` — source exception facts/regions, context-owned
continuations, shared constructor suffixes, catch object lifetimes and proved
nested-catch corrections; **21 original failures resolved**, **24 remain**.
The measured empty-lifetime-record regression is removed (9602 → 0 records
on the fixed template workload, identical LowIR). Validation/performance records follow the implementation tip without further
compiler edits. Ralph still owns acceptance and whole-stage review.

107 work in progress: full-expression/value lowering is the initial owner.
Cached cleanup/effect facts feed one function-local region scheduler; argument
activation and branch live prefixes feed shared immutable unwind suffixes. Keep
work linear in visited expressions plus emitted regions/suffixes. Validate
condition/logical/default/static/range behavior together, PA12 elision, all prior
stages, explicit executed lifetime controls, and frozen entry/final performance.
Prior turn classification: progress (committed source-EH implementation and
validated 71→92 passing tests); entry check here confirms 24 failures remain.

107 first group: observable temporary regions, scalar call result slots, terminal
logical branch cleanup and synthesized range result regions. Prior stages pass
3596/3596; PA12 257/257; 32 executed controls cover evaluated branches, nested
results, later operands, defaults, static-once and range lifetimes. Stage result
98/116 (18 failures) is checked sequentially: root reports share scratch, so
concurrent report totals were discarded. Further related work remains active.
