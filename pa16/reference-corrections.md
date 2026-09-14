# PA16 initialization reference corrections

Pinned reference bundle: source `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`,
SHA-256 `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`
([manifest](../reference-binaries/manifest.tsv)). The bundle is unchanged.
The [revision manifest](../student.tests/pa16/initialization-reference-revisions.json)
records every old/new fixture hash. Sources, status sidecars, coverage and the
validator/full-output comparator remain unchanged.

## Static initialization

C++11 [N3485](../doc/n3485.txt), 3.6.2 [basic.start.init]/2, requires constant
initialization before **any** dynamic initialization. Its three cases cover
constant reference bindings, constexpr constructor calls (including their
arguments/conversions/member initializers), and other constant initializers.
5.19 [expr.const]/2–3 admits the reference/address operations and the constexpr
calls used here; 7.1.5 [dcl.constexpr]/9 requires a constant initializer for a
constexpr object. Block statics use 6.7 [stmt.dcl]/4's static-initialization rule.

The six `order_*.cpp` [reducers](../student.tests/pa16/initialization) put a
non-constexpr observation before the constant object's definition. They retain
the original semantic operation: constructor argument call, base copy, unary
member operator, reference alias, reference-parameter address, or template static
string object. The observer must see the complete constant value. The pinned
reference instead executes the observer before the binding/construction: five
executables return 1 and the string case dereferences a null pointer. The student
executables return 0. `local_order.cpp` similarly makes an early dynamic call to
a function with a constant local reference: the reference executable dereferences
null, while the student returns 0.

These are observations of the standard violation, not compiler agreement as a
proof. The original small fixtures mostly observe the object only after startup,
so do not expose the error; they nevertheless pin the same incorrect dynamic
classification. Moving a constant binding to the start of one TU's dynamic hook
is also unnecessary and does not provide program-wide static data semantics.

The eight PA16 reference edits replace zero storage with the computed constant
or relocation, and remove only the now-redundant startup function. All other
instructions, aliases, symbols and metadata remain byte-for-byte unchanged.
A two-TU local-reference reducer also tests the removed queue against turn entry:
the old output has duplicate singleton `init` roles and is rejected by the native
backend; current output validates and returns 0 in both source orders.

The implementation already had checked values for the seven entry failures;
this increment additionally removes the local-reference startup exception and
its separate lowering queue. The explicit [harness](../student.tests/pa16/initialization.py)
validates and executes each reducer against both implementations.

## Automatic scalar arrays

The PA16 [assignment boundary](README.md#assignment-boundary) explicitly requires
an automatic, nonvolatile array of trivial scalar elements with a completely
known initializer to use readonly constant data and **one object copy**, with
distinct storage for every automatic object. This requirement is independent of
whether its declaration spells `constexpr`. The cumulative compiler has no PA
switch: PA10–15 inputs use the same `--emit-lowir -O0` mode. Sixteen older oracles
still pin element stores, so cannot express the current required representation.
This is a course-contract correction, **not a claim that scalar stores alone
violate the C++ abstract machine**.

The [array reducer](../student.tests/pa16/initialization/automatic_array.cpp)
initializes two mutable arrays with identical values, changes one and checks
that the other remains intact. Under N3485 8.5.1 [dcl.init.aggr]/2 and /7 the
initializer clauses and omitted zero elements determine the complete values.
3.9 [basic.types]/3 and /9 permit value-preserving byte copies for scalar types
and arrays of them. 1.8 [intro.object]/6 keeps these nonzero-size complete objects
distinct. LowIR [copyobj](../pa8/lowir.md#memory-and-addressing) transfers the
specified object representation; it does not merge destination identity.
These rules prove the required form is legal; the README supplies its necessity.

Each oracle was edited at its initializer, independently of compiler output:
extract the existing typed literal stores in ascending checked byte offsets,
retain their values/types in one readonly data image, retain the original
stack slot/address, and replace just that store/projection sequence with one
`copyobj` of the declared size/alignment. The manifest retains the exact data
items and slot for all 18 affected arrays. Every later instruction, local name,
call, conversion, alias and source test is unchanged. The full through report
checks the revised oracles against the compiler without a comparator relaxation.

The implementation routes eligible declarations through the existing typed
plan checker and data interner. Nonconstant initializers keep ordinary execution;
volatile stores and class construction/lifetimes retain their required actions.

## Program lifecycle ownership

The [two-TU lifecycle reducer](../student.tests/pa16/initialization/lifecycle_caller.cpp)
and [second TU](../student.tests/pa16/initialization/lifecycle_second.cpp) require
both dynamic constructors and reverse-order destructors. Before the connected
fix, each TU introduced its own singleton role and `--validate-lowir` rejected
the whole program. PA8's [runtime-hook contract](../pa8/lowir.md#reserved-runtime-hooks)
permits only one `init` and one `fini` definition. N3485 3.6.2
[basic.start.init]/2 permits the chosen sequential TU order, and 3.6.3
[basic.start.term]/1 requires the reverse destruction relation.

Program-owned typed function-ID sequences now supply one coordinator per role
when multiple TUs participate. The existing TU bodies and their order-sensitive
actions are preserved; no semantic graph survives just to reconstruct them.
A single TU keeps its original output. Helpers use names distinct from legacy
`__cppgm_init`/`__cppgm_fini` so the serialized reader cannot infer extra roles.
Both source orders validate, roundtrip and execute; no additional oracle edit
is needed for this implementation correction.
