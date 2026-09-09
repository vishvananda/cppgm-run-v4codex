# PA12 reference correction

Affected bundle: `cppgm-reference-binaries-linux-x86_64-c2f713cd70d0.tar.gz`,
source revision `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`
([manifest](../reference-binaries/manifest.tsv)). The binary bundle is unchanged.

`400-synthesized-constructor-bit-field-units.ref` repeats the bit-field retype
defect already proved in [PA11](../pa11/reference-corrections.md). Its four
comparisons consume masked `i32` storage values directly as `u32` operands.
[LowIR's enforced contract](../pa8/lowir.md#the-comparison-enforces) explicitly
requires `copy` for a bit-field read completing to its declared type and for
each integer retype. The [invalid reduced IR](../student.tests/pa12/bitfield-retype-bad.lowir)
and [corrected IR](../student.tests/pa12/bitfield-retype-good.lowir) isolate that
type disagreement. Strict LowIR validation rejects the first and accepts the
second; no validation or comparison rule is relaxed.

C++11 [4.5/5](https://timsong-cpp.github.io/cppwp/n3337/conv.prom#5) promotes
integral bit-fields to `int` when their entire value range fits. The 3-, 4- and
5-bit unsigned fields all qualify. Equality uses the usual arithmetic
conversions ([5.10/1](https://timsong-cpp.github.io/cppwp/n3337/expr.eq#1),
[5/9](https://timsong-cpp.github.io/cppwp/n3337/expr#9)), so each field and its
integer literal are compared as `int`. The [reduced C++ program](../student.tests/pa12/bitfield-promotion.cpp)
also demonstrates the semantic importance: with a zero field, `(value-1)/2`
is zero after signed promotion and truncation toward zero, whereas unsigned
arithmetic produces 2147483647.

Exactly four comparison sites are revised, adding declared-type `copy u32`
and promotion `copy i32` and changing the comparison's operand type to `i32`.
All allocation-unit transfers, loads/stores, control flow, source fixtures,
sidecars, required behavior and coverage are preserved. References were edited
only at those sites, not replaced with compiler output.
