# PA18 class call boundaries, implementation 87

Entry `01b47c20b41f68d3142df6a8691fc905c20341a8`: 417/420.
Final implementation `7000a4de`. Stage/review markers in [plan](plan.md) are
preserved; independent full-stage audit remains separate.

## Ownership and data flow

`ellipsis_conversion_value` now selects the class transfer while overload
resolution retains ellipsis rank even when that transfer is unavailable. Packed
`Conversion::ellipsis_object` and `ellipsis_unavailable` facts
keep source type/category and the chosen constructor available to queries,
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

The LowIR variadic suffix accepts scalar lanes. A class glvalue therefore uses a
pointer to private argument storage, with its selected copy/move and full
expression lifetime. This is an internal LowIR transport, not an assertion about
the later native platform's aggregate varargs ABI. Source `va_arg` and hosted
varargs interoperation are later surfaces. For nontrivial classes, PA18 chooses
copy/move construction and caller destruction as its implementation-defined
semantics under N3485 §5.2.2 [expr.call]/7. Trivial, empty and large classes use
the same object model; no scalar payload is invented for an empty class.
Prvalues already own a result object and pass its address without requesting an
extra copy/move. Deleted/private transfers therefore do not reject a prvalue
ellipsis argument, and a throwing copy does not affect its `noexcept` result.
Its existing source lifetime still supplies destruction. This follows the
lvalue-to-rvalue boundary in [expr.call]/7, rather than optional copy elision.

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

The `ellipsis_fixed_nontrivial` control traces the full boundary: a fixed call
`f(a)` retains its selected copy constructor in the template's semantic recipe.
`use<int>` and `use<long>` consume that selection with separate materializations;
lowering constructs each private object, passes its address, then activates
caller destruction. Native execution checks two copies and two destructions.
The related dormant-body/default, `noexcept` and constant controls exercise the
same recipe without confusing type, effect and evaluated demand.

## Complexity and scope

One transfer selection per examined ellipsis candidate; selected recipes reuse
the existing TU arenas and canonical class transfer/destruction facts. Each
evaluated argument owns one materialization and visits its selected conversion
edges. Lowering is linear in actual arguments and transfer output; the
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

The final binary (SHA-256
`be6a3c1b75c72e2c2606e4a6b32bd746569d0b76a218dd8f3ff1b864bcdfcb9c`)
is bound to [the evidence manifest](../student.tests/pa18/loop87-evidence.json).
[validate87.py](../student.tests/pa18/validate87.py) explicitly ran **61 checks**:

- `make test-pa18`: **420/420**, up from **417/420**.
- Earlier-stage report: **2609/2609**; through-PA18 report: **3029/3029**.
- File audit: pass, with the same three substantial-header warnings.
- Accumulated controls, scaling, inspection, ABI and eight executable traces:
  pass. The 35 result-bearing personal suites record **1,505** passing cases;
  the **53** new boundary cases improve **30 → 53** on identical source hashes.
- Reference proofs 83, 85 and 87 pass; unchanged historical proof 84 and exact
  composition with proof 87 pass. Separately, **24** final student/bundle/oracle
  observations confirm the documented results, including original direct calls.
- All **420** course inputs and **1,686** PA18 fixture paths are retained. Earlier
  fixtures, required exit metadata and comparison scripts are unchanged. Only
  the three documented result oracles change from entry.

[Performance 87](performance87.md) records the final **438 observations across
17 workloads** using frozen entry/final binaries, A/A calibration and ABBA blocks.
Both preceding measurement runs remain: **1,314 observations** total. Compiler
text grows **7,424 bytes (0.374%)**. Unaffected LowIR/native bytes stay identical;
selected copies add required argument storage and work. At 600 fixed uses the
paired compiler ratio is 1.061 (0.751–1.133), peak RSS 10,988 → 11,808 KiB and
native payload 28,924 → 40,924 bytes. The full report preserves all workload
latency, RSS, runtime, size, counter and spread evidence, with no speedup claim.
No optional optimizer or new growth budget was added. PA18/O0 has no mandated
numeric latency/RSS ceiling; inherited diagnostic targets do not add exit gates.

No unfinished implementation is identified in this completed behavior group.
Native try/catch, hosted class varargs retrieval and native optimization/debug
remain later-stage work. **Independent whole-stage review remains required** for
spec/architecture findings, accumulated oracle corrections (especially the
canonical result-boundary proof) and stage-scoped performance acceptance. Those
questions are not waived or represented as completed audit work. Stage and review
markers remain unchanged; this committed handoff returns control to Ralph and
does not advance to PA19.
