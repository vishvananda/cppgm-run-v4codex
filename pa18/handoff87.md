# PA18 class call boundaries, implementation 87

Entry `01b47c20b41f68d3142df6a8691fc905c20341a8`: 417/420.
Stage/review markers in [plan](plan.md) are preserved. This document is updated
with final validation and performance before handoff; independent stage audit
remains separate.

## Ownership and data flow

`ellipsis_conversion_value` now selects the class transfer while overload
resolution retains ellipsis rank even when that transfer is unavailable. Packed
`Conversion::ellipsis_object` and `ellipsis_unavailable` facts
keeps source type/category and the chosen constructor available to queries,
constant evaluation and concrete materialization. Evaluated applications create
their own existing `ConversionObject`; fixed template recipes remain shared and
unevaluated queries do not demand bodies. N3485 §4.1 [conv.lval]/2 suppresses
the class copy for an unevaluated operand: a missing/deleted/inaccessible copy
does not discard its ellipsis candidate during a type query. Evaluated use still
diagnoses the invalid transfer; constant execution cannot accept it. Exception
queries retain potential transfer effects rather than treating an unavailable
evaluated transfer as proven nonthrowing. Query recipes retain the source binding
but defer copy-constructor defaults until exception-effect or constant/evaluated
demand. Concrete `sizeof` operands use the same rule as retained type queries;
constant evaluation completes the recipe and enforces literal-type lifetime
requirements. These facts are copied with the recipe,
not inferred again in lowering. Scalar default promotions are unchanged.

The LowIR variadic suffix accepts scalar lanes. A class value therefore uses a
pointer to private argument storage, with its selected copy/move and full
expression lifetime. This is an internal LowIR transport, not an assertion about
the later native platform's aggregate varargs ABI. Source `va_arg` and hosted
varargs interoperation are later surfaces. For nontrivial classes, PA18 chooses
copy/move construction and caller destruction as its implementation-defined
semantics under N3485 §5.2.2 [expr.call]/7. Trivial, empty and large classes use
the same object model; no scalar payload is invented for an empty class.

`converted` consumes the flag to pass an address and activate cleanup. Branch
cleanup queries include the converted temporary, so conditional and short-circuit
paths only destroy the selected argument. The shared exception query consumes
constructor, default-argument and destructor effects. The existing constant
evaluator consumes the same selected conversions, retaining literal-type checks
and operand evaluation. Native try/catch execution is PA21; two exploratory
inputs and their failures remain recorded as `LATER` controls, not PA18 passes.

The remaining conversion fixture also exposed an untyped default floating zero:
`static_value_impl(0, T)` returned an integer record even for floating `T`.
It now consumes canonical typed zero construction for floating types, as for
member pointers. Positive zero, explicit negative zero and all other constant
paths retain their values. This avoids using an integer operand to represent a
floating static fact, including nested aggregate/conversion results.

## Complexity and scope

One transfer selection per examined ellipsis candidate; selected recipes reuse
the existing TU arenas and canonical class transfer/destruction facts. Each
evaluated argument owns one materialization and visits its selected conversion
edges. Lowering is linear in actual arguments and transfer output; a single
packed facts add no new graph or owning allocation. Branch analysis remains
memoized by expression identity and omission mode. Typed floating zero uses the
existing `(NodeId, TypeId)` static-value cache and constant interner. No textual
keys, token replay, global invalidation, candidate exceptions, optional optimizer,
new growth allowance or stage-specific production path is introduced.

Class result classification remains owned by the completed canonical class,
shared by definitions, calls, conversions, arrows, virtual/indirect signatures
and lambda adapters. A result's dependent spelling does not establish a different
source function type. [The result-boundary proof](reference-correction87.md)
documents the bundle inconsistency and independently reconstructed oracle repair.

## Validation and handoff

Validation and frozen performance evidence are pending at this implementation
increment. Required course sources, coverage and comparison rules are unchanged.
The final ledger will distinguish implementation work from independent review.
