# PA29 compact plan — implementation165 handoff

Target: **PA29 full-stage**. Phase: **implementation handoff complete; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `cce8634c3c835cf6d5e8f4fa5fea0db213959718`.
Previous review: `1ab3499d7046daf5c298d958a8770b413edb3615`.
Implementation165 entry: `61e023f39a4c1bafdccc484257ab51df98f87088`, **349/403**.
Group commits: `f42ab562`, `db0d0889` (activation rebasing and mutable aliases). Current: **350/403**, **53 failures**; one removed,
none added. All 403 fixtures, references, sidecars and comparison rules remain.

## Design and completed group

[Evaluation165](evaluation165.md) records owner, data flow, complete keys,
complexity, lifetime and the concrete boundary. The builtin registry and selected
call facts identify `__builtin_is_constant_evaluated`. Semantic evaluation mode
reaches expression/query/activation/static-value/array-proof keys. Lowering emits
false for ordinary execution and consumes accepted typed initialization facts
when storage must retain a constant-evaluated result. Reference plans preserve
actual storage identities and subobject offsets.

The group expanded through scalars, references and temporaries, constexpr class
and array storage, defaults, templates, recursive cached calls, nested locals,
volatile access, failed initialization trials, inherited fields, unions,
bit-fields and pointer constants. No grammar replay, serialized phase transport,
backend bypass or optional transform was introduced. Sparse mode facts use
average O(1) lookup, at most two variants per existing evaluation key, and the
existing evaluator work/depth bounds. Telemetry observes those facts directly.

Entry reconciliation classified the previous turn as progress: committed
function-context behavior and checks, with no inherited live process.
[audit162](audit.md), [assembly163](assembly163.md),
[function-context164](function-context164.md) and their review markers remain.
This implementation has not been independently reviewed.

## Validation and performance

- `make test-pa29`: **350/403**, exit 2; one fewer existing failure.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4888/4941**, exit 2; only PA29 fails.
- File audit: pass; four inherited substantial-header warnings.
- Explicit personal controls: **47/47**, typed LowIR/native/telemetry inspection:
  **112/112**; inherited function-context controls: **52/52**.
- [Validation](../student.tests/pa29/evidence165/validation.json),
  [coverage](../student.tests/pa29/evidence165/coverage.json),
  [delta](../student.tests/pa29/evidence165/stage-delta.json) and
  [performance165](performance165.md) retain the final evidence.

Performance acceptance is PA29/O0. Frozen common A/A+ABBA compares equivalent
correct implementations; affected measurements record necessary semantic costs
and demand scaling. No speedup is claimed. New optional work/growth budgets are
**zero**. Historical blanket 15%/zero-growth gates remain diagnostics under
spec §9; measurements and mandated limits remain. Broad hosted runtime,
optimization/allocation and self-hosting remain PA30–34 responsibilities.

## Unfinished implementation

The [remaining ledger](../student.tests/pa29/evidence165/remaining.json) preserves
all 53 failures; owner labels are not root-cause proof.

| Owner / failures | Work |
|---|---|
| Extended syntax/types/layout: 36 | Numeric/complex types, vector width, unused-wrapper validation, designators, folds, lambdas and bindings. |
| Template demand/hosted ABI: 15 | Packs/aliases/context keys, extern/inline emission and naming. |
| Source-invocation intrinsic context: 1 | Caller defaults, nested defaults, constructor/member-initializer source facts. |
| Legacy trait contract: 1 | Forward-declared std-trait oracle question; required behavior remains unresolved. |

Retain code-alignment placement, dependent offsetof ABI signatures,
class-convertible designator indices and extended floating precision in their
owners. Runtime vector lowering is outside PA29; compile-time layout is required.
The historical evaluation-context reducer from implementation164 now passes and
is retained as an explicit control. The
[source-invocation reducer](../student.tests/pa29/pending165/source-invocation.cpp)
remains unfinished implementation, alongside the required source-location fixture.
The inherited constexpr interpreter's aggregate-subobject mutation also remains
unfinished; scalar reference writes are covered here, while mutable aggregate
state requires its own overlay rather than replacing immutable aggregate facts.
The [reducer](../student.tests/pa29/pending165/aggregate-mutation.cpp) and
[entry/final failures](../student.tests/pa29/evidence165/aggregate-mutation-pending.json)
preserve this as positive behavior still required, not a rejection contract.

## Independent review and handoff ledger

| Boundary | Implementation | Independent review |
|---|---|---|
| audit162 / `cce8634c` | Atomic identity/storage corrections; 338/403. | Reviewed through preserved marker; whole stage unfinished. |
| implementation163 / `a6f3d6a2` | Required assembly family; 345/403. | Pending recipe/effect/ownership/performance review; not waived. |
| implementation164 / `e16108af` | Function-name strings and dependent queries; 349/403. | Pending context-key, rendering, ownership and performance review; not waived. |
| implementation165 / `db0d0889` | Evaluation modes and initialized storage; 350/403. | Pending mode-key, dependence, reference-lifetime and performance review; not waived. |

Forward-declared trait and explicitly-false nothrow-invocable primary oracle
questions remain unresolved. No reference correction or library-name shortcut
was made. Required fixture failures remain implementation work even where their
contract interpretation also needs independent proof.

Handoff boundary: the evaluation-mode/storage group is complete under its
controls and required-suite evidence. Source coordinates/defaults need an
additional invocation identity through call and construction recipes; a mode bit
and declaration-string identity cannot supply caller locations. Extending that
family would require a distinct semantic owner and cache inputs. Other failures
belong to separate type/layout and template/ABI owners. This is an incomplete
full-stage handoff. Independent review of these increments and whole-stage
completion remain required before advancement.
