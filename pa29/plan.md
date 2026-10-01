# PA29 compact plan — implementation167

Target: **PA29 full-stage**. Phase: **implementation; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `f07f78236eb475648834ed78afbca6c864f64408`.
Previous review: `cce8634c3c835cf6d5e8f4fa5fea0db213959718`.
Audit entry: `3036bd4ec4b1f8d315cf826437244de5236955bf`.
Current: **350/403**, exactly the entry's **53 failures**; no added failures,
removed tests, reference changes or comparison changes. All 403 fixtures remain.

## Reviewed ownership and fixes

[Audit166](audit.md) reviews every commit and the combined source changes from
the previous review through the code tip, including interactions across all
three accepted handoffs. Assembly163, function-context164 and evaluation165
have now received this independent review. The prior turn was committed progress;
no inherited live process needed reconciliation.

The cohesive correction commits semantic assembly writes/exposure to the scalar
observation owner; distinguishes runtime first new[] bounds from required
constant dimensions and carries that fact into lowering; materializes prvalue
value parameters/enumerators before reference binding; and keys query receivers
by evaluation mode, retiring both variants on their existing completion edges.
The integrated declaration/template trace reaches initialized storage, shared
LowIR and native ELF without phase text transport or semantic reconstruction.

Existing TU arenas, canonical identities, dependent-only substitution, sparse
mode facts, explicit completion edges and per-function backend lifetimes remain
the owners. New controls exercise the intersections, including standalone LowIR
execution, because a host run alone could hide the uninitialized reference slot.

## Validation and performance

- `make test-pa29`: **350/403**, exit 2; identical 53-failure set.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4888/4941**, exit 2; only PA29 fails.
- File audit: pass; four inherited substantial-header warnings.
- Explicit controls162–166: **222 passing behavioral checks**; inherited/new
  LowIR, MIR, native and telemetry inspection: **222 passing checks/groups**.
- [Validation](../student.tests/pa29/evidence166/validation.json),
  [coverage](../student.tests/pa29/evidence166/coverage.json),
  [failure delta](../student.tests/pa29/evidence166/stage-delta.json),
  [complete range](../student.tests/pa29/evidence166/range.json) and
  [performance166](performance166.md) retain the final evidence.

Performance acceptance is **PA29/O0**. The 832 final observations plus 24 launcher
samples measure compiler latency/RSS and checked runtime/text. Equivalent common
images are byte-identical; audit-only paired compiler ratios are 0.9966–1.0013.
Affected correctness costs and demand scaling are disclosed; all six new final
workloads fail entry checksums, so they supply no valid entry speedup baseline.
Scheduling variation limits precise timing claims. Historical observations and
all preliminary samples remain; no speedup is claimed.

New optional work/growth budgets are **zero**. Historical blanket 15%/zero-growth
gates, including inherited plans, remain diagnostics under spec §9. Required
correctness, coverage, evaluator/native limits and assignment timeouts remain.
Broad hosted runtime, optimization/allocation and self-hosting are PA30–34 work.

## Remaining implementation

The [remaining ledger](../student.tests/pa29/evidence165/remaining.json) still
accounts for all 53 failures; owner labels do not prove root causes.

| Owner / failures | Work |
|---|---|
| Extended syntax/types/layout: 36 | Numeric/complex types, vector width, unused-wrapper validation, designators, folds, lambdas and bindings. |
| Template demand/hosted ABI: 15 | Packs/aliases/context keys, extern/inline emission and naming. |
| Source-invocation intrinsic context: 1 | Caller defaults, nested defaults, constructor/member-initializer source facts. |
| Legacy trait contract: 1 | Forward-declared std-trait oracle question; required behavior remains unresolved. |

Retain code-alignment placement, dependent offsetof ABI signatures,
class-convertible designator indices and extended floating precision in their
owners. Runtime vector lowering is outside PA29; compile-time layout is required.
The positive [source-invocation](../student.tests/pa29/pending165/source-invocation.cpp)
and [aggregate-mutation](../student.tests/pa29/pending165/aggregate-mutation.cpp)
reducers remain unfinished implementation. Source coordinates need invocation
identity through call/construction recipes; mutable aggregate state needs its
own overlay rather than replacing immutable aggregate facts. The historical
evaluation-context reducer remains a passing explicit control.

Forward-declared trait and explicitly-false nothrow-invocable primary oracle
questions remain unresolved. Compiler agreement alone cannot justify reference
changes; no library-name shortcut or reference correction was made here.

Three broad owners justified separate handoffs, but the alias followup and
repeated records-only handoffs added avoidable fragmentation. Complete each owner
through dependent queries, effect invalidation, storage/lifetime semantics and
both typed-IR/native paths before handoff. This audit closes review of the
accumulated increments, not implementation of PA29. Full through-PA29 success
is required before advancing. The audit ledger preserves earlier reviews and
adds one row for this complete range. The records commit follows the validated
code tip with no further code edits.

## Active implementation167

Entry HEAD: `7719caea56d56b5c359796e25a08b3f2f659a4c5`; 350/403, 53 failures.
The preserved stage base and last-reviewed markers above remain unchanged.
Previous goal turn: committed audit progress; current process inspection found
no inherited compiler/test job.

Initial owner: canonical hosted vector types and deferred inline validation.
Data flow: parsed attribute expressions -> canonical lane/width type facts ->
layout/initialization validation -> ordinary demand and typed LowIR emission.
Validate unused wrappers independently of emission; only unsupported reserved
builtins can defer a body. Attribute expressions are parsed once and evaluated
in the existing substitution context. Identity lookup is average O(1); width
and layout work are bounded by the type/initializer actually consumed.

Extend related attribute, dependent layout and wrapper-validation fixes while
the same ownership supports progress. Keep runtime vector lowering outside the
explicit PA29 contract. Required suite/earlier suites/file audit, explicit
positive and rejection controls, typed-IR inspection, and frozen O0 latency/RSS
plus runtime/text observations will establish the handoff boundary. Optional
optimization work/growth budgets remain zero; no performance speedup is assumed.

Increment167a: GNU vectors now retain lane/byte identity and template width
queries; initializer literals validate every element. Hosted namespace inline
functions use the existing deduplicated use edges and emission roots, while
member construction facts retain their existing semantic demand. Direct
discarded unknown reserved builtins have an explicit unavailable-body marker,
never a guessed signature. Pointer-to-vector ABI consumes PA9's typed Vector.
Current check: 354/403 (four previous failures fixed, no new failures); 22/22
explicit controls. Next: complete adjacent extended-vector layout and shape
validation, then frozen final validation/performance evidence.

Increment167b: extended vector types retain lane counts separately from GNU
byte widths, including packed boolean layout and dependent partial-specialization
deduction. Constant shape errors are rejected even in unused template patterns.
42/42 controls and the final course tests pass their intended expectations:
PA29 354/403; PA1–28 4538/4538; through PA29 4892/4941. File audit passes
with the four inherited header warnings. Remaining 49 course failures are
unchanged; performance and IR inspections are still pending handoff evidence.

The final substitution control exposed dependent-vector ABI emission after
semantic success. The PA9 Vector graph now retains an optional expression edge;
source lowering, validation, encoder and explicit fact adapters consume it.
SFINAE, pack queries and executable signatures now pass 45 controls. Preliminary
performance is retained separately because this required ABI correction changes
the final compiler. All final checks and measurements will be refreshed.
