# Defaulted class argument pack correction

The pinned bundle remains `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, archive
SHA-256 `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Local revision `pa19-defaulted-pack-91` changes one reference, with original and
corrected hashes in [the manifest](../student.tests/pa19/reference91-revisions.json).
[The transformer](../student.tests/pa19/reference91.py) reads the stage-base
oracle, never generated student output. All source inputs, status sidecars,
validator coverage and comparison rules remain unchanged.

The fixture declares a tuple with ten fixed type parameters. Three explicit
arguments and seven defaults form its full type. Deduction against
`tuple<T0, Ts...>` binds the first argument to `T0` and the remaining **nine**
arguments to `Ts`. The old reference incorrectly counted only the two explicitly
written arguments after the first.

[N3485 §14.3 [temp.arg]/4](../doc/n3485.txt:17585) shows that omitted defaults
form the same specialization as explicitly supplied arguments. §14.8.2.5
[[temp.deduct.type]/9](../doc/n3485.txt:21002) compares a trailing expansion
against every remaining argument of the actual type. §5.3.3
[[expr.sizeof]/5](../doc/n3485.txt:6367) gives the resulting pack cardinality.
[The reducer](../student.tests/pa19/defaulted_pack91.cpp) checks the canonical
type identity and cardinality for both partially and wholly defaulted spellings.
The independent personal controls also cover non-type defaults, inherited
specializations and inconsistent deductions.

The correction changes the pack-size constant from 2 to 9. The fixture's `main`
still compares with 2 and therefore correctly returns **1**; PA19 grades LowIR
and compilation status, not a zero runtime exit. Its source is preserved.
The function's ABI metadata now names `T0=int`, `Ts={int,int,null_type × 7}`
and the parameter pattern `const tuple<T0,Ts...>&`. The checked-in
[Itanium mangling contract](../doc/itanium-mangling.txt) specifies `J...E` for
the template argument pack, `Dp` for the dependent pack expansion, and ordinary
type substitutions for the repeated `null_type`. Names, signature, constructor,
instructions, aliases and all other metadata remain unchanged. This proof uses
the language and ABI contracts; compiler agreement is not its basis.
