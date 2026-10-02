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
