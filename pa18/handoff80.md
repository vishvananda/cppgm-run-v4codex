# PA18 implementation handoff 80

Entry `c5e2c2719f1ef004e6bddb86487e797c37471079`: **393/420**.
Implementation `cec91d23`, `a87dd911`; subsequent narrowing-check repair is
recorded in the evidence manifest. The stage remains unfinished; this handoff
completes scalar/reference conversion selection and materialization, including
its ordinary, substitution-query, constant and exception consumers. Stage-base
and last-reviewed markers are unchanged. Ralph owns independent acceptance.

## Completed behavior and ownership

Two original failures close without changing inputs or reference outputs:
`300-abstract-array-parameter-sfinae` consumes its constant enum condition;
`500-constructor-pack-default-rewritten-pointer` materializes its already-typed
pointer without a redundant conversion. The existing PA12 null-to-pointer
conversion boundary remains intact. Neither change recognizes source names,
stages or fixtures.

The initial scalar trace was extended through reference binding and casts.
Controls exposed real runtime defects: a null derived pointer passed through a
converted reference was adjusted into a non-null base pointer, and reference
casts to bit-fields aliased the containing storage. They now retain null and
materialize a scalar copy, respectively. Mutable lvalue reference, const_cast
and reinterpret_cast attempts to bind bit-fields reject.

Ordinary expressions and substitution queries previously differed on reference
casts that require scalar conversion, and queries rejected valid conversion
functions returning floating/pointer values. `explicit_builtin_conversion` now
owns those selected sequences for both consumers. It preserves immediate-context
failure, access/deleted checks, base rules and cv/category constraints. The query
records the selected conversion function for exception and constant consumers.
No body is invoked merely to determine whether the cast is well-formed.

Constant evaluation now stores the converted scalar type and value before binding
a reference. Both source-node and query paths previously reused the source's
storage, losing narrowing and floating/integer conversion. A selected user
conversion's second conversion now uses the same rule, including reference
results and reference-bound scalar temporaries. These changes do not make
non-constexpr functions eligible for constant evaluation. The inherited list
controls caught premature rounding of a conversion-function result during a
narrowing check; the final path honors that consumer's requested unrounded
result type before reference materialization. The failed intermediate evidence
is preserved separately from final acceptance.

| Owner | Data flow | Complexity / lifetime | Validation |
|---|---|---|---|
| `lowering/control_flow.cpp` | Published constant name → known condition edge | O(1) fact read; no evaluator or subtree walk | Enum/const/volatile conditions; original array-SFINAE case |
| `lowering/values.cpp` | Selected reference conversion → pointer adjustment → correctly typed temporary slot | One conversion/slot per required materialization; null branch only for nonzero adjustment | Null/non-null bases, pointer types/cv/defaults, distinct temporaries, side effects |
| `semantic/explicit_conversion.cpp`, expression/query adapters | Canonical source/target/category and field identity → shared cast sequence | Existing indexed candidates/base paths and query cache; no syntax replay, new global scan or invalidation | Ordinary/dependent casts, bit-fields, scalar/pointer/user results, rejection/SFINAE and noexcept |
| `constant_execution.cpp`, `constant_queries.cpp` | Selected sequence → converted value → typed constant storage/reference | At most one scalar conversion and storage record per materialized reference; recursion clears temporary/reference flags | Narrowing, float/integer, reference results, constexpr user conversion and non-type argument queries |

The new records reuse TU-owned type, conversion and constant-storage identities;
query keys retain their existing complete substitution contexts. Runtime
function-local slots die with their function. There is no textual transport,
new cache, optional optimizer, body-summary pass or class-result ABI policy.
No implementation source was added, so source-set registration is unchanged.

C++11 anchors are [N3337](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3337.pdf)
§5.2.9 [expr.static.cast]/3–4 (bit-field value and direct initialization),
§8.5.3 [dcl.init.ref]/5 (reference binding/converted temporaries), §4.10 [conv.ptr]/3
(null preservation), §5.19 [expr.const] and §14.8.2 [temp.deduct]/8
(immediate context). The corresponding supplied text is in
[doc/n3485.txt](../doc/n3485.txt):6007. A volatile bit-field cannot lose volatile
qualification through an incompatible reference cast; the positive control uses
a volatile-qualified result, and the cv-dropping form remains a rejection.

## Validation and trace

- `make test-pa18`: **395/420**, exit 2; failures **27 → 25**, no new failures.
- `make test-report-through-pa17`: **2609/2609**, exit 0.
- PA18 file audit: exit 0 with the same three inherited header advisories.
- **1062** explicit personal/course controls pass, including **68/68** new controls
  versus **38/68** on the frozen entry compiler. Inherited ABI and scaling checks,
  both source-to-native traces, and both repaired original inputs pass.
- All **420** course sources and **1686** fixture/reference files are byte-identical
  to entry. No coverage or comparison change, reference correction or bundle revision.

Final counts, raw observations, binary and fixture hashes are in
[the evidence manifest](../student.tests/pa18/loop80-evidence.json).
Performance acceptance and all four measurement dimensions are in
[performance80.md](performance80.md).

The combined [trace](../student.tests/pa18/scalar80_trace.cpp) demands a template
whose result contains a narrowed constexpr reference conversion, performs an
ordinary/dependent scalar cast, copies a bit-field into an independent temporary,
adjusts a null base pointer, and executes a selected conversion with an observable
effect. The compiler emits typed LowIR directly; the explicit personal harness
validates it and uses the supplied native backend to execute it. That backend
is the course PA18 boundary, not part of this compiler's implementation.

## Handoff boundary and unfinished work

The remaining **25 original failures** are still implementation requirements:
array/aggregate initialization and constant materialization (including the
unknown-bound array after empty expansion), empty-tag construction/root metadata,
class-result ABI conventions, globals, discarded loads, non-constexpr conversion
body folding, and arithmetic widening presentation. No comparison rule is waived.

The array trace confirmed that earlier fixtures require pooled copies for the
same small scalar shapes for which PA18 requires stores. No stage or filename
switch was introduced. Resolving that shared policy needs a separate contract
and initializer-materialization decision. Class results and object roots need
ABI/emission facts, while folding a non-constexpr scalar-returning conversion
needs a body effect summary and emission-use ownership. A selected conversion
and its scalar temporary cannot provide those facts. Extending this group through
those owners would start a distinct implementation trace; this is the concrete
boundary after repairing the related query, runtime, constant and exception paths.

The inherited nested-alias cast runtime marker was stale: its complete course
source validates and executes on both entry and final compilers. Its LowIR
array-materialization comparison remains open. The class-ellipsis reducer is
still unfinished ordinary variadic lowering.

Independent review must check shared cast selection versus direct initialization,
bit-field/cv rejection, constant reference storage identity, and selected user
conversion exception/result facts. These are review questions with passing
controls, not substitutes for known failures. PA19 still requires the complete
through-PA18 report and whole-stage independent audit.
