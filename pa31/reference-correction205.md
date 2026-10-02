# Global-variable relocation spelling correction

The two `700-hosted-{pcrel-data-reloc,imported-global-got-load}-link-smoke`
inspection expectations incorrectly name the ordinary global variable `g` as
`_Z1g`. Only that spelling changes in their `.inspect.expect` and `.ref.inspect`
sidecars. Required relocation classes, positive counts, forbidden classes,
source programs, compile/link/run status and comparison rules are unchanged.

Proof: C++11 [basic.scope.namespace]/3 places the unqualified declarations
`int g = 7;` and `extern int g;` in global namespace scope; [basic.link]/4 gives
this non-const, non-static namespace variable external linkage (see
`doc/n3485.txt`, §§3.3.6 and 3.5). C++ itself does not prescribe symbol bytes.
PA31's **Hosted ABI Names** contract requires the normal host ABI for every
emitted/reference symbol. The normative [Itanium ABI §5.1.2, General
Structure](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-structure)
says: “Entities with C linkage and global namespace variables are not mangled.”
Thus both relocations must target `g`, not `_Z1g`. The former must be a PC32
data relocation and the imported address must be loaded through GOTPCREL as
required by the unchanged inspection contract. Compiler agreement is not the
proof.

Reducers: `student.tests/pa31/source205/global-defined.cpp` is the single-TU
case; `global-import.cpp` with `global-provider.cpp` is the imported case.
The explicit control runner records source hashes, object symbols and
relocations and verifies runtime outcomes. The entry compiler already emits
the correct spelling and relocation classes; these corrections introduce no
compiler behavior change or coverage reduction.

Pinned bundle source revision: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
Archive SHA256: `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Compiler SHA256: `e32741e2504f7c75a91abdf5cea0d5fc0b56eb43deb503d2ffbddbcc2f854f70`.
The binary bundle and manifest remain unchanged.
