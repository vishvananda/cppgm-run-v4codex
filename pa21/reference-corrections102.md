# RTTI template name correction

Bundle source: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`; bundle SHA256:
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Local revision: **pa21-rtti-name-102**. The stage-base commit retains the
original oracle. No input, exit status, comparison rule or coverage changes.

The [reducer](../student.tests/pa21/rtti_name_reducer102.cpp) retains a namespace,
two template-template arguments with different arities, a default specialization
argument, and an outer wrapper. The pinned reference emits
`1OIN1n1WINS0_1MENS0_1VENS4_IiJEEEEEE`; the correct encoding is
`1OIN1n1WINS0_1MENS0_1VENS3_IiJEEEEEE`.

The proof does not depend on compiler agreement. N3485
[[expr.typeid] 5.2.8/4](../doc/n3485.txt) requires the RTTI object to represent
the specified type. [temp.type] 14.4/1 makes the template identity and arguments
part of that type's identity: `V<int>` cannot be replaced with another template.
The inherited [PA9 contract](../pa9/README.md) requires Itanium-compatible names
and substitution order. The ABI specifies the
[RTTI name representation](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#rtti-name)
and [substitution ordering](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-compression).
Its sequence encodes the first entry as `S_`, the next as `S0_`, and so on;
components enter before containing structures, and repeated components do not
enter twice.

For the reducer the ordered entries before the reused `V` are `O`, `n`,
`n::W`, `n::M`, `n::V`. Thus `S3_` denotes `n::V`; the reference's `S4_`
refers to an entry that does not yet exist. In the full fixture the entries are:

| Index | Component | Encoding |
|---|---|---|
| 0 | `json_encoder` | `S_` |
| 1 | `nlohmann` | `S0_` |
| 2 | `nlohmann::json_abi_v3_11_3` | `S1_` |
| 3 | namespace-qualified `basic_json` | `S2_` |
| 4 | namespace-qualified `ordered_map` | `S3_` |
| 5 | namespace-qualified `vector` | `S4_` |
| 6 | namespace-qualified `allocator` | `S5_` |
| 7 | namespace-qualified `adl_serializer` | `S6_` |

The erroneous `NS5_IhJEE` therefore names `allocator<unsigned char>` instead
of the declared `vector<unsigned char>`. Host mangling and demangling confirm
this independently, but the table and rules establish the correction.

[Reconstruction](../student.tests/pa21/reference102.py) reads the original from
Git, checks that its name bytes equal its symbol's type encoding, changes the
one substitution digit in the byte array and matching `_ZTS`/`_ZTI` metadata,
and preserves everything else. It never reads student output to generate the
oracle. [The manifest](../student.tests/pa21/reference102-revision.json) records
the original and revised hashes and exact changed byte offset.
