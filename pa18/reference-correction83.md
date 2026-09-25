# PA18 array and constant-initialization oracle correction 83

The pinned bundle remains `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`,
SHA-256 `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`
([manifest](../reference-binaries/manifest.tsv)). The local revision is recorded
with old/new hashes and independently extracted values in
[reference83-revisions.json](../student.tests/pa18/reference83-revisions.json).
[The reproducer](../student.tests/pa18/reference83.py) reads only the entry
oracles, never the student's generated LowIR. Inputs, coverage, validator and
comparison rules remain unchanged.

## Automatic constant scalar arrays

[PA16's cumulative assignment boundary](../pa16/README.md#assignment-boundary)
requires an automatic nonvolatile scalar array whose complete initializer is
known to use readonly data and one object copy, retaining distinct automatic
storage. Fifteen later fixtures still pin individual stores, including two PA17
fixtures previously accommodated by implementation exceptions. The rule has no
exception for small wide integers or side-effect-free class-to-scalar conversion.
This is a contract correction, not a claim that scalar stores violate C++.

The reduced `array_identity`, `array_wide`, `array_constexpr_conversion`,
`unknown_nested_flat`, `unknown_braced_string` and pointer controls in
[array83_controls.py](../student.tests/pa18/array83_controls.py) check values,
object independence, bound completion and required copy structure. Effectful
conversions, volatile elements and runtime addresses keep ordinary execution.
The inherited [PA16 reducer](../student.tests/pa16/initialization/automatic_array.cpp)
checks the same rule without templates.

[N3485](../doc/n3485.txt) §8.5.1 [dcl.init.aggr]/2,4,7 fixes explicit and omitted
values and deduced bounds; §8.5.2 [dcl.init.string]/1–2 fixes string elements.
§3.9 [basic.types]/3,9 permits representation copies for these scalar arrays;
§1.8 [intro.object]/6 preserves distinct complete objects. PA8's
[copyobj contract](../pa8/lowir.md#memory-and-addressing) supplies the value-preserving
copy. The checked scalar stores are extracted in increasing byte offsets, with
full-size assertions. Only that initialization sequence becomes a readonly
image and one copy. Equal images share storage; destinations do not. All later
instructions and original names remain. In PA17's comma initializer, discarded
`I<T>()` objects are empty, trivial, unobserved temporaries: §1.9 [intro.execution]/1
permits their omission. Their zeroing/slots are removed explicitly before
extracting the unchanged zero scalar values.

## Empty expansion into an unknown-bound array

The reducer is `template<int...N> void f(){int a[]={N...};} int main(){f<>();}`.
N3485 §14.5.3 [temp.variadic]/6 makes the expansion an empty list; §8.5.1
[dcl.init.aggr]/4 explicitly requires a positive number of initializer clauses
and forbids an empty initializer for an unknown-bound array (footnote 104
confirms C++ has no zero-length arrays). The existing success oracle allocated
an arbitrary one-byte object for `int a[] = {}`. Its status is corrected to
`EXIT_FAILURE`; the old LowIR remains informational, preserving the observation.
The nonempty, sentinel, unused-body and nested controls retain positive coverage.
The compiler now rejects at semantic bound completion, rather than reaching
an incomplete-array layout error. It does not invent storage for an invalid type.

## Constexpr union static initialization

The fixture's globals `inactive` and `active` have constexpr constructors whose
constant initializers select `dummy=0` and `value=1`. N3485 §3.6.2
[basic.start.init]/2 requires constant initialization before any dynamic
initialization; §7.1.5 [dcl.constexpr]/9 requires constexpr object initialization
to be constant; §5.19 [expr.const]/2 admits the invoked constructors and forward
reference. The original oracle deferred both constructions into a runtime hook.
The reduced `union_constant_order` places a dynamic observer before the constexpr
union definition, distinguishing required static initialization from that hook.

The revision writes `u8 0` plus three padding bytes for `inactive`, and `i32 1`
for `active`, then removes only the obsolete init hook. The selected constructor
and forwarding bodies remain unchanged. This follows the prior
[PA16 static-initialization correction](../pa16/reference-corrections.md).
