# PA15 aggregate array initialization reference correction

The corrected fixture is `tests/general/100-aggregate-functional-braced-cast`.
Its source, success status and the LowIR validator/comparison rules are unchanged.
This changes the checked LowIR fixture, not the downloaded reference executables.
The pinned bundle source is `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle
SHA-256 `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`
([manifest](../reference-binaries/manifest.tsv)).

The [reducer](../student.tests/pa15/aggregate_relocation.cpp) preserves the
functional braced cast, template aggregate and array member. Each element stores
its own address. Its ordinary, copy and move constructors all establish that
invariant, so the result is independent of permitted copy elision. Compile with
`--emit-lowir -O0`, then run the resulting LowIR through `dev/lowir2native-ref -O0`.
The student executable returns **0**; the pinned reference executable returns
**1**. Reference LowIR initializes a separate array, then calls an aggregate
helper containing `copyobj 16x8` to relocate the nontrivial elements. The self
pointers still designate the old array. No copy/move constructor repairs them.

The governing C++11 rules in [N3485](../doc/n3485.txt) are:

- 8.5.1 [dcl.init.aggr]/2: initializer clauses initialize the corresponding
  aggregate members; an aggregate array member is recursively initialized.
- 12.8 [class.copy]/12 and /15: these user-provided copy/move constructors are
  nontrivial; a generated class copy/move initializes array elements individually.
- 12.8 [class.copy]/31: when permitted temporary copies are elided, source and
  destination are two descriptions of the same object. This allows direct
  construction at the final address, not construction elsewhere followed by
  a raw relocation that changes the observable self-pointer invariant.
- 3.9 [basic.types]/2–3 gives byte-copy value preservation to trivially copyable
  types; it does not authorize bypassing this reducer's nontrivial constructors.

The original empty-element fixture does not observe the bad relocation, but it
pins that same incorrect helper rule. Its replacement initializes the elements
in the aggregate's own storage using the existing typed list-initialization
facts. All required construction and calls remain. The reducer supplies the
observable proof; compiler agreement is not the proof. The full-output comparison
remains active and no fixture or behavior was removed. Before/after hashes and
native result hashes are retained in
[the evidence](../student.tests/pa15/aggregate-reference-correction.json).
