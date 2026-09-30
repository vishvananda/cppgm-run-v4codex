# Local reference overlay 145: global-namespace variable spelling

The `200-host-defined-global-data-pcrel` and
`200-host-imported-global-data-got` inspection expectations incorrectly required
`_Z1g` for the ordinary global-namespace variable `g`.

Proof: C++11 N3485 [basic.link]/4,9 gives matching namespace-scope declarations in
different translation units the same entity identity. PA27's “Using PA9 ABI
Names” requires the configured host ABI's raw spelling. Itanium ABI 5.1.2,
[local contract copy](../doc/itanium-mangling.txt:23), states: “Entities with C
linkage and global namespace variables are not mangled.” Thus both the definition
and import must use `g`; `_Z1g` is not the required host symbol. C++11 alone does
not prescribe mangling; the assignment's ABI contract supplies that rule.

Reducer: [definition](../student.tests/pa27/global-name.cpp) and
[consumer](../student.tests/pa27/global-name-user.cpp). The personal object
controls compile these separately, inspect exact names and GOT/PCREL classes,
then link each direction with a host object. Host agreement is corroborating
evidence, not the proof.

Only the two `.inspect.expect` files and corresponding `.ref.inspect` output
lines replace `_Z1g` with `g`. Source fixtures, counts, relocation requirements,
function spelling, runtime outputs, status sidecars and comparison rules remain
unchanged. The pinned downloaded bundle remains unchanged; this is a documented
local expectation overlay on that bundle, not a replacement binary.

Bundle source revision: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`;
bundle SHA-256: `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`
([manifest](../reference-binaries/manifest.tsv)).
