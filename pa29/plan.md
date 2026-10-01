# PA29 compact plan — checkpoint audit166

Target: **PA29 full-stage**. Phase: **checkpoint audit complete; stage unfinished**.
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
