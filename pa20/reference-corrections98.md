# PA20 closure conversion reference corrections

Pinned bundle source revision: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`;
SHA256 `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Local revision **pa20-closure-conversions-98**. Original bytes remain in commit
`e75e0d6ce1e26a2d50bc0857cd620974c6ee451d`; hashes and reproducible revisions
are recorded by [reference98.py](../student.tests/pa20/reference98.py).

## Implicit wrapper conversion must be rejected

The [reducer](../student.tests/pa20/wrapper_conversion98.cpp) retains just the
function-pointer constructor, reference parameter and lambda argument.
N3337 [expr.prim.lambda]/3,6 says the lambda has its unique closure class type
and a conversion function to a pointer. That conversion is user-defined.
[over.best.ics]/4 restricts the constructor's initial argument conversion in
this context to standard conversions; [over.ics.user]/1 allows one user-defined
conversion between two standard sequences. Binding to `const Wrapper&` does
not allow a second conversion: [over.ics.ref]/2 uses copy-initialization of a
Wrapper temporary. The closure conversion plus Wrapper constructor therefore
cannot form an implicit conversion sequence, so `use(lambda)` is ill-formed.

Sources: [C++11 draft lambda rules](https://timsong-cpp.github.io/cppwp/n3337/expr.prim.lambda#6),
[conversion sequence rules](https://timsong-cpp.github.io/cppwp/n3337/over.best.ics#4),
[user-defined sequence](https://timsong-cpp.github.io/cppwp/n3337/over.ics.user#1),
[reference sequence](https://timsong-cpp.github.io/cppwp/n3337/over.ics.ref#2).
The corresponding N3485 clauses are also in [the local draft](../doc/n3485.txt).

Only `200-captureless-lambda-wrapper-conversion.ref.exit_status` changes to
EXIT_FAILURE. The source and historical LowIR stay intact (failure stdout is
informational). Positive executable controls retain the actual supported
behavior: explicit `Wrapper(lambda)` and `use(+lambda)` succeed. The negative
reducer makes the corrected rule explicit. Coverage is not removed.

## Constructor deduction preserves the closure type

N3337 [temp.deduct.call]/1,2 compares parameter F with the argument's type,
applying only the listed array/function/cv adjustments to a non-reference
parameter. A lambda expression has a class type under [expr.prim.lambda]/3;
its pointer conversion under /6 is optional conversion behavior, not an
adjustment made by deduction. F must therefore be that closure class, and
`f(6)` invokes its call operator on the closure object. This also follows the
PA20 `200-captureless-lambda-closure-type` contract.
[Deduction rules](https://timsong-cpp.github.io/cppwp/n3337/temp.deduct.call#2).

The old oracle instead instantiated `Sink<int(*)(int)>` (visible in its
Itanium symbol), passed a pointer and called through it. Its numeric result
happened to be 7, which does not prove correct deduction. The
[reducer](../student.tests/pa20/constructor_closure98.cpp) adds a pointer-trait
assertion inside the constructor and verifies the result. This separates the
required type rule from coincidental executable agreement.

The [handwritten replacement](../student.tests/pa20/constructor_closure98.lowir)
uses the closure object argument and its receiver-bearing call operator. The
ordinary default/copy constructor definitions remain ordinary definition roots,
with their source-specified 0 and `other.value + 100` actions; neither is selected
by main. The selected constructor has precisely one call with argument 6 and
one store. Empty closure storage has no value bytes to copy. The final result
check, output comparison rules and source are unchanged. The revision script
reads no student compiler output. It validates both original and revised LowIR
with the supplied backend, runs both, checks their expected result and verifies
the replacement symbol/signature and all revision hashes.

These corrections are based on the standard rules above, not compiler agreement.
The implementation already obeyed both rules at turn entry; the 17 newly
implemented required capture cases establish implementation progress separately.
