# Bit-field reference corrections

The affected bundle is `cppgm-reference-binaries-linux-x86_64-c2f713cd70d0.tar.gz`,
source revision `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`
([manifest](../reference-binaries/manifest.tsv)). The bundle itself is unchanged.

## Proof

[LowIR's enforced comparison contract](../pa8/lowir.md#the-comparison-enforces)
requires a `copy` for each change of integral type at unchanged width, explicitly
including a bit-field read completing to its declared type. It explains that
omitting the copy makes the temporary's type disagree with its uses.
[Reduced invalid IR](../student.tests/pa11/bitfield-retype-bad.lowir) passes an
`i32` result directly to a `u32` operation;
[the corrected IR](../student.tests/pa11/bitfield-retype-good.lowir) adds the
required `copy u32`. The compiler's LowIR validator rejects the former and
accepts the latter. This observation checks the rule; it is not the proof.

C++11 [4.5/5, integral promotions](https://timsong-cpp.github.io/cppwp/n3337/conv.prom#5)
uses an integral bit-field's value range. The one- and two-bit unsigned fields
in these fixtures fit in `int`; their arithmetic operands therefore promote to
`int`. Enumerated bit-fields continue to use ordinary enumeration promotions.
The [reduced C++ program](../student.tests/pa11/bitfield-promotion-reduced.cpp)
initializes a one-bit unsigned field to zero and computes `(bits.value - 1) / 2`.
Promotion to `int` gives zero (division truncates toward zero under
[5.6/4](https://timsong-cpp.github.io/cppwp/n3337/expr.mul#4)); performing unsigned
arithmetic instead gives 2147483647. This establishes a semantic difference
without relying on agreement among compilers.

## Exact revisions

- `400-bit-field-member-access-bad.ref` and
  `400-bit-field-assignment-result.ref`: add the missing `copy u32` between
  the masked storage result and the existing return conversion to `i32`.
- `400-bit-field-prefix-postfix-increment.ref`: complete each read with
  `copy u32`, then promote through `copy i32`. Existing increment operations
  retain `i32`; the two comparisons use their promoted `i32` operands.
- `400-bit-field-repeated-subobject-init.ref`: complete both reads with
  `copy u32`, promote both to `i32`, and add in `i32`. The obsolete unsigned
  result conversion is removed.

Edits preserve original source inputs, initialization/store sequences, all
other instructions, sidecars, coverage and comparison rules. References were
edited at the identified conversion sites, not replaced with compiler output.

The initializer continuation applies the identical proof to
`400-volatile-initialization-paths.ref`: the prefix read and four comparisons
receive declared-type completion and integral promotion. Volatile loads/stores,
all array projections, source inputs and control flow are retained. This is the
fifth affected output from the same pinned bundle.

## Aggregate helper volatile initialization

The same bundle's `100-volatile-access-markers.ref` initializes the explicitly
volatile `Device::status` member in its aggregate helper with ordinary `store`.
[LowIR memory access rules](../pa8/lowir.md#memory-and-addressing) explicitly require
volatile markers for scalar initialization of aggregate members and elements;
only preliminary whole-class zero-initialization has the stated exception.
The [reduced source](../student.tests/pa11/volatile-init-reduced.cpp) initializes
one such member through a one-element aggregate array. Its helper's destination
is that volatile member, not preliminary zeroed storage. The correction adds
`volatile` to exactly that store. All source tests, other instructions, sidecars
and comparison rules remain unchanged. The personal check validates/executes
this source and inspects the required volatile store marker.
