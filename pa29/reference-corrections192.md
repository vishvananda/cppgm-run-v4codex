# PA29 reference correction 192: the Q literal representation

Correction ID: **pa29-192-quad-token**. One line in
`tests/preproc/400-host-gnu-hex-float-pp-number.ref` changes from an x87
`long double` representation to the required `__float128` representation.
The input, every other token, success status, stdout, discovery, coverage and
comparison rules are unchanged. No permissive comparison is introduced.

The observed reference bundle remains the pinned revision in
[manifest.tsv](../reference-binaries/manifest.tsv): source
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`,
cppgm++ SHA-256
`e32741e2504f7c75a91abdf5cea0d5fc0b56eb43deb503d2ffbddbcc2f854f70`.
The bundle itself is not rewritten. Regenerating from it will restore this
known error until its producer is corrected.

## Contract and rule proof

The [PA29 handout](README.md) requires GNU builtin type and literal forms for
Linux x86-64, including `__float128`/`_Float128` types; the required
`700-hosted-gnu-float-builtin-spellings.t` overloads `long double` and
`__float128`. [Spec §§2 and 6](../spec.md) require canonical type facts and
faithful typed lowering, rather than substituting a different floating format.

C++11 [N3485](../doc/n3485.txt) §2.14.4 [lex.fcon]/1 specifies the ordinary
suffix/type distinction: `L` selects `long double`. §1.4 [intro.compliance]/8
permits extensions without changing well-formed standard programs. §17.6.4.3.5
[usrlit.suffix]/1 reserves non-underscore suffixes. C++11 itself does **not**
define `Q` or hexadecimal floating literals: their positive meaning here comes
from PA29's expressly required GNU hosted extension, not from pretending that
C++11 specifies a binary128 literal.

The [GNU additional floating types contract](https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html)
defines `q`/`Q` as the `__float128` literal suffix and defines x86-64
`__float128` as the 128-bit type. For C++ since GCC 13, `_Float128` has distinct
canonical identity, as recorded by the [GCC implementation change](https://gcc.gnu.org/pipermail/gcc-cvs/2022-September/371667.html); both retain binary128 storage. Its separate 80-bit
`__float80`/`_Float64x` type does not redefine `Q` as `L`.
Thus the token's type is `__float128`, and the exactly representable value is
2^-16382. Binary128 has a 112-bit fraction and exponent bias 16383; this value
has sign 0, exponent field 1 and fraction 0. Its 128-bit encoding is 1<<112,
whose little-endian bytes are `00000000000000000000000000000100`.
The old bytes `00000000000000800100000000000000` instead encode the x87
explicit integer bit and exponent 1. The distinction is a type/representation
error even though both encodings express the same mathematical value.

The [one-token reducer](../student.tests/pa29/source192/quad-token.t) isolates
the output, and [the typed reducer](../student.tests/pa29/source192/quad-token.cpp)
checks suffix identity and exact bits. The independent rational decoder test
also derives the bit pattern without any compiler oracle. Compiler agreement
is corroboration only; the extension contract and encoding derivation are the
proof. Other extended suffixes and ordinary `f`/`L` tokens remain covered by
the unchanged course fixture and explicit controls.
