# PA25 floating calculator boundary (implementation136)

The five preserved differences in [floating-differences135.json](floating-differences135.json)
are reproduced by [floating-rounding.cc](floating-rounding.cc). This observation
uses the host only as a test program; production compilation never calls it.
The probe compares direct double multiplication/division with an explicit
long-double intermediate followed by conversion to double. It prints the raw
64-bit results (the original calculator prints little-endian bytes).

| Input line | Direct double / current compiler | Extended intermediate / reference |
|---:|---|---|
| 84840 | b48b0ee4fd9ecb91 | b48b0ee4fd9ecb90 |
| 352410 | 11274f8051b35b7d | 11274f8051b35b7c |
| 795222 | 2a172559d8e6f241 | 2a172559d8e6f240 |
| 837831 | 5915188be1339835 | 5915188be1339834 |
| 873937 | f23acc62e8d1c6e7 | f23acc62e8d1c6e6 |

C++11 N3485 §5 [expr]/12, in [the checked-in standard](../../doc/n3485.txt),
permits greater precision/range for floating operands and expression results
without changing their types. Thus host agreement with the current compiler
is **not proof that these reference values are wrong**. The checked-in oracle,
comparison rules and million-input coverage are preserved. Reference bundle:
`cppgm-reference-binaries-linux-x86_64-c2f713cd70d0.tar.gz`, SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
There is no reference correction in this handoff.

PA24's encoder emits scalar SSE arithmetic for f32/f64 and x87 for f80; its
contract also calls for the ordinary f32/f64 register path. This remaining PA25
failure therefore needs an explicit source floating-evaluation policy consistent
with the exact source-program oracle and inherited LowIR/native contracts.
Changing the generic native f64 operation to double-round, or special-casing
these five inputs, is not part of this scalar extension. The stage requirement
remains unfinished. Independent review must resolve the precision-policy question;
that question does not waive the failing behavioral comparison.

Explicit observation command:

```sh
g++ -std=c++11 -O0 student.tests/pa25/floating-rounding.cc -o "$RALPH_ARTIFACT_DIR/pa25-136/floating-rounding"
"$RALPH_ARTIFACT_DIR/pa25-136/floating-rounding"
```
