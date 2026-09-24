# External-entity ABI substitution correction

Pinned bundle: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, manifest bundle SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Entry: `06211ad0438df250952a414eef409d657b0ff5b0`.
Only `pa9/tests/abi/300-function-owner-member-pointer-nttp-data.ref` changes.
Its input, success status, fixture count, and exact-output comparison remain.
The pinned binary bundle is unchanged.

The reduced [ABI input](../student.tests/pa18/address_abi_reducer.abi) and
[C++11 meaning](../student.tests/pa18/address_abi_reducer.cpp) express
`ns::Holder<&C::m>::f(C&)`. N3485 14.3.2 [temp.arg.nontype]/1,5 permits this
member address argument; 14.4 [temp.type]/1 preserves the same member/entity
identity. C++ itself does not specify mangled spellings. The owning
[PA9 contract](../pa9/README.md) explicitly requires the Itanium ABI, including
host-compatible substitution ordering, and directs us to
[`doc/itanium-mangling.txt`](../doc/itanium-mangling.txt).

The decisive contract proof is Itanium ABI §5.1.6.2 (external entity expression)
and [§5.1.10, Compression](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-compression):
substitutable components are processed left to right, including names inside
expressions, and a component is entered before a composite that contains it.
There is no isolated substitution dictionary for an external entity literal.
The slots in this reducer are:

| Slot | Component first encountered |
|---|---|
| `S_` | `ns` |
| `S0_` | `ns::Holder` template prefix |
| `S1_` | `C`, inside the address expression for `C::m` |
| `S2_` | `ns::Holder<&C::m>` |

Thus the trailing `C&` is `RS1_`. The correct complete encoding is
`_ZN2ns6HolderIXadL_ZN1C1mEEEE1fERS1_`.
The old oracle ends in `ER1C`, incorrectly treating `C` as previously unseen.
A live GCC observation agrees, but is not the proof or a test oracle. Explicit
personal ABI tests use fixed contract-derived encodings, not a live host oracle.

This defect was exposed by PA18's source-generated function-address arguments:
the same erroneous isolated dictionary also misnumbered template parameter type
substitutions in external function-template names. The shared typed ABI encoder
now consumes one substitution state for the complete name. Pointer arguments
use an address-expression wrapper; reference arguments name the entity directly.
No semantic name lookup or string reparsing was added to lowering.
