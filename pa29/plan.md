# PA29 compact plan — implementation164 handoff

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `cce8634c3c835cf6d5e8f4fa5fea0db213959718`.
Previous review: `1ab3499d7046daf5c298d958a8770b413edb3615`.
Implementation164 entry: `e5690337eb69b59468d989a143030bd8dbdc8ee0`, **345/403**.
Code tip: `e16108af`. Current: **349/403**, **54 failures**; four removed,
none added. All 403 fixtures, references, sidecars and comparison rules remain.

## Design and completed group

[Function-context164](function-context164.md) records ownership, data flow,
complexity and the concrete boundary. Canonical function-name queries retain
function scope/name identity and substitute the owning context before publishing
the string's entity and array bound. Pretty strings render typed declarations,
enclosing-template arguments and indexed inline-namespace facts. The existing
constant-address and typed LowIR/global-storage paths consume the same entity.

Work extended from the four course failures to sizeof parsing, const array
identity/bounds, references, packs, nested/partial specializations, canonical
aliases, address/member-pointer arguments, cv/ref qualifiers, constructors,
destructors, conversion names and placeholder returns. No text-based semantic
identity, grammar replay, backend bypass or optional optimization was added.
String construction is once per function/name and linear in related semantic
components and output bytes. New telemetry reads the owning flat index in O(1).

Entry reconciliation classified the previous turn as progress: committed
assembly behavior and checks, with no inherited live process. Existing
[audit162](audit.md), [assembly163](assembly163.md) and their review markers remain.
This implementation is not independently reviewed.

## Validation and performance

- `make test-pa29`: **349/403**, exit 2; four fewer existing failures.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4887/4941**, exit 2; only PA29 fails.
- File audit: pass; four inherited substantial-header warnings.
- Explicit personal controls: **52/52**, including LowIR validation/roundtrip,
  native execution and telemetry equality. Explicit IR debug locations survive
  MIR; this is not a claim of source `-g`/DWARF support in the LowIR tool mode.
- [Validation](../student.tests/pa29/evidence164/validation.json),
  [coverage](../student.tests/pa29/evidence164/coverage.json),
  [delta](../student.tests/pa29/evidence164/stage-delta.json) and
  [performance164](performance164.md) retain evidence and all observations.

Performance acceptance is PA29/O0. Frozen common A/A+ABBA compares equivalent
correct implementations; affected measurements record necessary semantic costs
and demand scaling. No speedup is claimed. New optional work/growth budgets are
**zero**. Historical blanket 15%/zero-growth gates remain diagnostics under
spec §9; prior measurements and mandated limits remain. Broad hosted runtime,
optimization/allocation and self-hosting remain PA30–34 responsibilities.

## Unfinished implementation

The [remaining ledger](../student.tests/pa29/evidence164/remaining.json) preserves
all 54 failures; owner labels are not root-cause proof.

| Owner / failures | Work |
|---|---|
| Extended syntax/types/layout: 36 | Numeric/complex types, vector width, unused-wrapper validation, designators, folds, lambdas and bindings. |
| Template demand/hosted ABI: 15 | Packs/aliases/context keys, extern/inline emission and naming. |
| Structured intrinsic operands: 2 | Evaluation-mode/initialization semantics and source-invocation facts. |
| Legacy trait contract: 1 | Forward-declared std-trait oracle question; required behavior remains unresolved. |

Retain code-alignment placement, dependent offsetof ABI signatures,
class-convertible designator indices and extended floating precision in their
owners. Runtime vector lowering is outside PA29; compile-time layout is required.
The [evaluation-context reducer](../student.tests/pa29/pending164/evaluation-context.cpp)
is unfinished implementation: a trial extension computed true in constexpr
execution but stored false through ordinary local initialization. That incomplete
extension was removed. Source-location defaults likewise require caller context,
not the declaration context owned by this completed group.

## Independent review and handoff ledger

| Boundary | Implementation | Independent review |
|---|---|---|
| audit162 / `cce8634c` | Atomic identity/storage corrections; 338/403. | Reviewed through preserved marker; whole stage unfinished. |
| implementation163 / `a6f3d6a2` | Required assembly family; 345/403. | Pending recipe/effect/ownership/performance review; not waived. |
| implementation164 / `e16108af` | Function-name strings and dependent queries; 349/403. | Pending context-key, rendering, ownership and performance review; not waived. |

Forward-declared trait and explicitly-false nothrow-invocable primary oracle
questions remain unresolved. No reference correction or library-name shortcut
was made. Required fixture failures remain implementation work even where their
contract interpretation also needs independent proof.

Handoff boundary: no known defect remains in the completed function-string group.
The two adjacent intrinsics need distinct invocation/evaluation-context facts
through defaults, cached constexpr activations and initialized storage; further
work cannot safely reuse immutable declaration-string facts alone. Other failures
belong to distinct type/layout and template/ABI owners. This is an incomplete
full-stage implementation handoff. Full PA29 validation and independent whole-stage
audit remain required before advancement.
