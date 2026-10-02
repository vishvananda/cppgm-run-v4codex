# Complex arithmetic emission order

PA34's first full object comparison differed in `semantic/complex.o`, inside
`Analyzer::complex_binary`. The differing instructions computed real and
imaginary addition/subtraction in opposite orders; each object then stored the
correct component. The seed and self-produced LowIR had the same difference.
The small `complex-order.cpp` reproduces it at O0 before any optional optimizer.
Both generated programs pass the runtime checks.

The owner is `lowering/complex.cpp::complex_operation`, whose two `operation`
calls were arguments to `complex_construct`. Each call appends IR and assigns
IDs. GCC evaluated the imaginary call first, while the student compiler
evaluated the real call first. This is permitted by C++11 [expr.call]/8 and
[intro.execution]/15: function arguments have no specified relative order;
the nested function executions are indeterminately sequenced. It is an artifact
ordering defect, not evidence that the compiler must copy GCC's argument order.
See the checked-in standard [N3485](../../doc/n3485.txt), §§1.9 and 5.2.2.

Separate full expressions now sequence the emission calls explicitly under
[intro.execution]/14. The existing imaginary-then-real IR order is retained.
Runtime source operands have already been evaluated before this lowering step;
the change does not impose a new C++ source evaluation rule. It introduces no
optimization work, IR growth or self-host mode. The source was valid before;
the fix makes its generated artifacts reproducible across valid host choices.

`check_complex_order.py SEED SELF` checks exact LowIR/object equality and runtime
arithmetic at O0/O3 for all three GNU complex precisions. Replacing only the
self-built `lowering/complex.o` is the diagnostic localization; final validation
uses canonical generations and the unchanged inception comparison.
