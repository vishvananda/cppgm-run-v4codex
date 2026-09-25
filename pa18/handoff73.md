# PA18 implementation handoff 73

Entry `12cbfe8359d17e950fb11694be0cd7e2dfb7cb8c` → implementation `9ebc507c`.
This completes callable selection/consumption and prototype-object behavior;
PA18 remains unfinished. The previous goal turn was progress, confirmed by the
clean entry, baseline log and absence of any surviving compiler job.

## Ownership, data flow and bounds

| Owner | Established behavior | Work and lifetime |
|---|---|---|
| `semantic/callable.cpp`, ordinary and retained call owners | The course `__builtin_invoke` consumes its actual callable and argument nodes. Functions, function pointers/references, callable objects and surrogate conversion calls share ordinary selection and conversion rules. Queries retain typed operands, context and intrinsic identity. No synthetic AST, grammar replay or lowered-name lookup is introduced. | One source query per source/context, one fact per canonical substituted query. Work follows operands and required overload candidates. Source-fixed calls/operators retain recipes; concrete occurrences project only their operand/object edges. |
| `ObjectUse`, constant execution, LowIR calls | A rare callee edge records the intrinsic's actual callable. Source-owned edges project through the existing occurrence context. Constant execution and query evaluation consume selected decay/conversion facts, including forwarded function references and fixed pointer/surrogate calls. Callable/receiver effects execute once. | TU-owned contiguous object-use/conversion records; no field added to the hot `Expression` record. Each selected call lowers once; no second overload search. |
| `template_conversion_facts.cpp`, query calls/operators | Selected access, deletion, argument conversion and base-access failures return normal invalid-query results. The ordinary checker shares the validator and emits hard diagnostics at its boundary. Class-definition/default side effects retain their own hard-error ownership. A noexcept surrogate conversion does not make its indirect function call nonthrowing. | Expected immediate-context failures do not render diagnostics or unwind exceptions. Completed query failures remain attached to their full canonical context; completion invalidation still uses explicit dependency edges. Braced-list validation remains a separate unfinished owner. |
| Member prototype scopes and signature facts | A member trailing return has an explicit object context before any function body exists. `this`, cv/ref-sensitive member calls and prior member overload sets remain available without requiring the incomplete class layout. Current-instantiation types are retained for substitution; nested source classes retain their fixed base edges. A type parameter that substitutes to a function type does not invent a source parameter declaration list. | Prototype contexts live in the existing TU arena/index. Ordinary prototype decltype uses the typed query owner. Parameters and source signatures are consumed once; no context cloning or global retry. |
| Using-declaration access and hiding | Template specializations inherit a using-declaration's exposure through canonical primary edges. This affects access and implicit-object ranking. Imported templates with the same normalized head/parameter/cv/ref shape are hidden by derived declarations in either source order; different templates and non-templates remain candidates. | Access follows the actual specialization/base edges. Hiding examines the name's overload family. Normalized shapes cache by head and type identity, with TU lifetime and no invalidation of unrelated lookups. |
| Typed ABI graph | A `this` expression encodes `fpT`; its fact adapter roundtrips the new node. Invocation-query manglings retain the source intrinsic name and all operands. | One compact graph identity per expression; encoding is linear in emitted graph traversal. Production does not use the fact adapter as transport. |

Language anchors: N3485 §5.1.1 [expr.prim.general]/3 (member-declarator `this`,
cv and incomplete-class member access), §5.2.2 [expr.call], §4.3 [conv.func],
§13.3.1.1.2 [over.call.object] (callable objects and surrogate calls), §7.3.3
[namespace.udecl] (using-declaration hiding/access), §14.8.2 [temp.deduct]
(immediate-context access failures), and §5.3.7 [expr.unary.noexcept].
The checked-in invocation fixtures require `__builtin_invoke` over the PA18
function/callable subset; member-pointer invocation remains outside PA18's
stated boundary. The ABI expression grammar is in
[doc/itanium-mangling.txt](../doc/itanium-mangling.txt), including `fpT` at line 476.
No reference output, input, harness, comparison rule or bundle was changed.

## Validation

- `make test-pa18`: **379/420**, original failures **48 → 41**, seven repaired
  paths and no new failures. All seven repaired inputs also validate and execute
  successfully through the supplied native backend.
- `make test-report-through-pa17`: **2609/2609**. File audit passes with the same
  three inherited header advisories. PA18 has no extra native/debug exit target;
  the prior report includes PA9's ABI contract.
- **587/587 semantic controls** pass: 64 invocation, 36 prototype/using and all
  487 inherited controls. These exercise SFINAE, access/deletion, private bases,
  forwarding, defaults, cv/ref qualification, surrogate calls, effects, constexpr
  execution, source-fixed reuse, nested classes and renamed out-of-class heads.
- Three completion controls preserve one invalidation each at 32/128/512 classes.
  Four ABI checks cover reader/writer roundtrips and emitted source identities.
- Validation exposed a PA18 current-instantiation regression and two prior-stage
  lookup/hiding regressions; all were reduced and repaired in their owning paths
  before final validation. Intermediate logs are retained in `/tmp/pa18-loop73`.
  The audit's function-size finding was resolved by moving call-query construction
  to `query_call.cpp`.

[Performance evidence](performance73.md) and the
[evidence manifest](../student.tests/pa18/loop73-evidence.json) record frozen
compiler/executable observations, input/output hashes, checks and unchanged
coverage. No optional optimization or extra numerical exit gate is introduced.

## Handoff boundary: unfinished implementation

The work expanded from invocation detectors through ordinary/constant calls,
selected-conversion failures, source-fixed recipes, prototype `this`, implicit
member calls, inherited exposure/ranking/hiding, exception effects and ABI
identity. Known defects exposed in those paths have been resolved and checked
against the earlier stages.

The remaining nearby failures cross into other owners. The dependent tag call
uses a braced value whose initialization query does not yet describe list
initialization; the dependent braced-overload and narrowing fixtures share that
missing owner. Completing them requires an initialization-query representation
and its narrowing/constructor participation rules, not another callable lookup
fallback. The explicit member-pack and inherited-constructor fixtures still
reach constructor deduction/initialization. Lazy nested class declarations and
first-declaration result lookup require declaration timing and binding facts.
Other failures are ordinary LowIR initialization/result/ABI policies: array
blobs versus element stores, bool/results, class-return conventions and discarded
loads. The inherited class-ellipsis reducer remains unfinished ordinary variadic
LowIR representation work. Correct call selection does not waive those output
comparisons or that defect.

Further fixes therefore require a separate declaration or initialization/LowIR
behavior group and its own evidence. This handoff does not advance to PA19, nor
claim whole-stage correctness. All 41 failures remain implementation obligations.

## Independent review

Stage base `94dcb8ad21664137e87d574e878c14a4a047348a` and Last reviewed
`f59e8f67cd8c832361130aef9af1a0337b25c45d` are unchanged. Handoffs 71, 72 and 73
remain pending review. Review callable/prototype context key completeness,
source/concrete receiver projections, hard versus immediate failure boundaries,
using-shape keys and access provenance, ABI source fidelity and measured bounds.
These review questions are separate from the unfinished implementation above;
neither is waived. Ralph owns independent audit and advancement.
