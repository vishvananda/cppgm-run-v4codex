# Ill-formed compiler source exposed by PA34

PA20 owns placeholder variable deduction; PA14 only supplies the template
context that exposed this failure. Audit 222 moved this personal negative test
from PA14 to that owner. PA20's required positive subset has one declarator;
this C++11 rejection control does not extend the course's PA20 exit criteria.
Historical PA34 evidence retains its original path and hash at `5a90033a`.

`semantic/template_call.cpp`, in `deduce_function_values`, declared a `TypeId`
and a `Type` using one `auto` declaration. N3485 §7.1.6.4 [dcl.spec.auto]/7
requires every declarator's placeholder to deduce the same type. The two types
are `unsigned int` and a class record, so this source is ill-formed.

The reducer `mixed-auto.reject.cpp` is accepted by GCC 15.2.0 and rejected by
Clang and this compiler. Those observations locate the seed acceptance gap;
the standard rule, rather than agreement, establishes the correction. Split
the compiler source into two declarations with identical initializers and
evaluation order. This changes no valid-source support or performance policy.
No fixtures or references change.

Explicit validation:
`dev/cppgm++ -c student.tests/pa20/mixed-auto.reject.cpp -o /tmp/mixed-auto.o`
must fail. The canonical self build must compile the corrected real source.
