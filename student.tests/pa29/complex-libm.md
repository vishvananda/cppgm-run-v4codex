# Complex hosted libm builtin gap

PA34's semantic/complex.cpp includes the selected hosted `<complex>` header.
Its ordinary inline overloads expose the missing `__builtin_cabsf` family.
This belongs to PA29's hosted builtin construction/probe boundary; PA30's
broader header acceptance consumes that same machinery.

[GCC's Library Builtins contract](https://gcc.gnu.org/onlinedocs/gcc/Library-Builtins.html)
defines these builtins with the same type and behavior as their C library
counterparts. The bounded shared libm registry now records unary complex,
binary complex and real-from-complex signatures for float/double/long double.
Semantic construction uses canonical fundamental types and ordinary C linkage;
the existing complex ABI/lowering emits the call to the corresponding libm
symbol. No source compilation is delegated and no reference output changes.

The explicit script checks all 66 advertised signatures, incorrect arity and
runtime magnitude/conjugate/projection/square-root/power identities at O0/O3
with runtime inputs and all three precisions. The original compiler source is
also compiled by the PA34 object probe and canonical build.

That probe next exposed GNU complex `{real, imaginary}` initialization, used
by the header's ordinary constructors. The component pair now uses the same
typed list plans as other initialization, without changing complex types into
C++ aggregates. Constant evaluation packs the two canonical component values;
lowering preserves the selected list conversion at scalar/member boundaries.
Component narrowing checks use real floating types rather than interpreting
the complex pair's packed identities as one floating constant. Constructor
summaries preserve that list conversion rather than discarding its second item.
`check_complex_braces.py` exercises constants, members, templates, references,
arguments, arrays, three precisions, signed zero, sequencing and invalid lists.
