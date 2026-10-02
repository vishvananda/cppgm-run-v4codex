# Compiler source narrowing correction

`semantic/deduction_parameters.cpp` and `class_pattern_selection.cpp` used
`Type::bound` (uint64_t) directly in aggregate lists whose corresponding member
is a uint32_t TypeId/ArgumentId. C++11 [dcl.init.list]/3 requires a diagnostic for
narrowing in aggregate initialization; /7 includes a nonconstant integer
conversion to a type that cannot represent all source values. The reduced
`id-narrowing.reject.cpp` must fail, even when runtime values would fit.
See `doc/n3485.txt`, section 8.5.4.

GCC with `-std=c++11 -Werror=narrowing -fsyntax-only` diagnoses both original
compiler sources. Default GCC builds merely warned. The compiler correctly
rejects the reducer; its language behavior is unchanged.

The source fix explicitly recovers the canonical ID from the wide union-like
slot. Both paths first establish TypeKind::PackExpansion. Its bound is written
from the uint32_t ArgumentId by `Types::pack_expansion` in semantic/model.cpp,
so this conversion is exact. No valid construct is avoided for self hosting.
