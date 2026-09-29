# PA20 automatic-array contract correction

The pinned reference bundle is source revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Local revision: **pa20-array-images-95**. This carries forward the already
established [PA16 proof](../pa16/reference-corrections.md#automatic-scalar-arrays)
and the same corrections in PA17–19.

[PA16's assignment boundary](../pa16/README.md#assignment-boundary), lines
174–176, requires every automatic nonvolatile array of trivial scalar elements
with a completely known initializer to use readonly constant data and one
object copy, preserving distinct destination storage. PA20 extends that
cumulative compiler. Eleven PA20 references still contain element-store
initialization for such source-declared arrays. Scalar stores are legal C++;
the defect is their conflict with this explicit inherited output contract.
No stage-dependent compiler policy can satisfy the cumulative contract by
silently switching this requirement off for a later source feature.

The [reduced source](../student.tests/pa20/array_reducer95.cpp) constructs equal
mutable arrays, modifies only the first through a range reference, and checks
both identity and every element (including an omitted zero). N3485
[dcl.init.aggr] 8.5.1/2,7 determines those values; [dcl.init.string] 8.5.2/1,2
determines the character and terminating zero in the string case.
[basic.types] 3.9/3,9 permits representation copying for scalar objects and
arrays of them; [intro.object] 1.8/6 preserves distinct nonzero complete
objects. The [LowIR copyobj contract](../pa8/lowir.md#memory-and-addressing)
copies the bytes into the existing destination. These standard rules prove
legality and value/identity preservation; the PA16 handout proves necessity.
The local standard text is [N3485](../doc/n3485.txt).

[Reconstruction](../student.tests/pa20/reference95.py) reads only the committed
entry oracles, checks the complete contiguous typed store span against the
slot's declared size/alignment, extracts precisely those literals into readonly
data, retains the destination address, and replaces just the initialization
span with one copy. It checks removed SSA addresses have no later uses. All
later instructions, source tests, statuses, coverage and comparison rules are
unchanged. The [manifest](../student.tests/pa20/reference95-revisions.json)
records every original/revised hash and extracted value. No implementation
output is used to create an expected result. Validation also executes old and
revised LowIR with the supplied backend and checks equal program outcomes.
