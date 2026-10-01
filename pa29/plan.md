# PA29 compact plan — implementation160 in progress

Target: **PA29 full-stage**. Phase: **implementation; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `1ab3499d7046daf5c298d958a8770b413edb3615`.
Entry: `8bdc6bbf28980810ae1bc33895243d4de7c15149`, clean, **321/403**, 82 failures.
All 403 fixtures, reference outputs, harnesses and comparison rules are retained.
Previous goal turn: progress (handoff159 changed code and validated 321/403).

## Current implementation group

Owner: intrinsic invocation, shared member-pointer application and typed unary
operator recipes. Trace source operands → canonical query / selected receiver
and argument conversions → typed LowIR → existing member-pointer ABI encoding.
Complete direct/base, raw-pointer and pointer-like member invocation together,
including query/SFINAE, cv/ref categories, lifetime, constexpr and exception facts.
No fabricated syntax, repeated resolution in lowering, or library-name matching.
Work is bounded by required operator candidates, arguments and base edges; query
facts and receiver recipes are TU-owned. Freeze entry/final binaries, measure
common A/A+ABBA compiler latency/RSS and runtime/text, plus new-capability scaling.
Validation includes existing invoke fixtures, explicit positive/rejection/runtime
controls, LowIR roundtrip, full stage delta, earlier stages and file audit.
Further structured intrinsics (source locations, constant evaluation/fences) have
distinct fact owners and remain implementation work, not audit questions.

## Completed ownership and spec alignment

[Implementation159](implementation159.md) traces six legacy member traits and
three reference-temporary traits from the shared probe/parser registry through
canonical queries, member/exception facts and selected conversion recipes to
typed LowIR and native calls. Direct reference initialization now shares ordered
binding phases with traits, casts and template initialization. Both source and
result temporary destruction are checked. Structural triviality follows selected
subobject members and remains separate from access/deletion and callability.

TU-owned flat caches use operation/type or declaration identity. Completed
negative and positive facts are reused; unavailable dependencies remain retryable.
Work visits the required families, candidates and subobjects, without textual
reconstruction, body replay, whole-program retries or extra phase graphs.
Telemetry observes actual fact computations. Parser → semantic graph → typed
LowIR → per-function MIR → direct ELF remains the production path.

## Remaining stage work and independent review

[The fixture ledger](../student.tests/pa29/evidence159/remaining.json) retains
every failure and its disposition; broad owner labels are not root-cause proof.

| Owner / retained failures | Remaining work |
|---|---|
| Atomic/assembly: 21 | Atomic storage/order/effect facts and assembly constraints through typed IR/native execution, layout and noexcept controls. |
| Extended syntax/types/layout: 37 | Extended numeric/complex types; vector width/layout and unused-wrapper validation; designated initialization, folds, lambdas and bindings. |
| Template demand/hosted ABI: 19 | Packs/aliases/context keys, demand, pretty-function, extern/inline emission and ABI; includes the contract question below. |
| Structured intrinsic operands: 4 | Constant-evaluation/address/fence semantics, invoke receiver/member-pointer recipes, source-location facts. |
| Legacy trait contract: 1 | The forward-declared std-trait oracle question preserved from audit158. |

Known inherited implementation work also includes code-alignment placement,
dependent offsetof ABI signatures and class-convertible designator indices.
Extended float suffix recognition does not implement extended precision.

Independent contract questions: the undefined `std::is_nothrow_*` specializations
and the reserved-name nothrow-invocable cache fixture whose primary is explicitly
false. Neither authorizes library-name shortcuts; both failures remain. Audit
must resolve their contracts. Review must also assess this handoff's binding
phase order, cache validity after class completion and structural/usable-member
separation. Those are review obligations, not implementation waivers.

## Validation and performance

- `make test-pa29`: **321/403**, exit 2; [exact delta](../student.tests/pa29/evidence159/stage-delta.json) is four existing failures removed, none added.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- File audit: exit 0, four inherited substantial-header warnings.
- Explicit controls: **47/47** new, **34/34** audit158, two inherited trait
  executables; native selection inspection, LowIR roundtrip and telemetry byte
  equality pass. Repeating identical traits 100 → 10,000 times keeps query work
  at 5, class-property work at 4, member-property work at 3 and body demand at 0.
- [Validation hashes](../student.tests/pa29/evidence159/validation.json) and
  [performance159](performance159.md) preserve frozen binaries/inputs, A/A and
  ABBA observations, latency/RSS, checked runtime/text size, demand scaling and
  timing spread. No speedup or optional optimization is claimed.

New optional optimization work/growth budgets remain **zero**. Historical
blanket 15%/zero-growth targets remain diagnostics under spec §9; all historical
measurements and mandated limits are preserved. PA29 excludes runtime vector
lowering; broad headers, general hosted execution, optimization/allocation and
self-hosting retain their PA30–34 owners. Required PA29 behavior is not waived.

## Handoff ledger and boundary

| Increment | Result |
|---|---|
| `46fa99d2` | Registry, legacy/lifetime traits and shared reference binding; four course failures resolved. |
| `8bc08195` | Selected-member structural caching, deleted-default triviality, inspection and performance workloads. |
| `5d2b1657` | Source-prvalue destruction validation, with positive existing-glvalue controls; all required checks refreshed. |

This handoff closes the trait/conversion ownership group, including its known
correctness edges. Further failure reduction now requires the distinct atomic,
extended-type/syntax, template/ABI or intrinsic models above, or independent
resolution of retained contract questions. Those cannot be implemented by
extending the established trait facts alone. The stage remains unfinished;
Ralph's independent audit and eventual full through-PA29 report are still required
before advancement. Stage base and last-reviewed markers are preserved.
