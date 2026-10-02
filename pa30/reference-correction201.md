# Replacement-new exception reference correction

Fixture: `tests/compile/700-hosted-replaceable-operator-new-dynamic-exception-spec.t`.
Only its expected exit status changes from success to failure. Source, test
inventory, other sidecars and comparison rules are preserved. Diagnostic text
is not graded. The fixture's comment incorrectly assumes a repeated dynamic
specification; preprocessing shows that the first declaration has none.

Proof: C++11 [except.spec]/3–4 requires compatible exception specifications on
all declarations of a function once a declaration supplies a nonempty dynamic
specification. An absent specification and `throw(std::bad_alloc)` are not
compatible. [basic.stc.dynamic]/2 lists the implicit allocation declaration
without a specification. [res.on.exception.handling]/4 permits implementations
to add an explicit stronger specification, but this header has not done so.
This is a declaration constraint, separate from the runtime requirement that
allocation failure throw `bad_alloc`. No special exception to declaration
compatibility is provided for replacement allocation functions. See
[N3337, §§3.7.4, 15.4, 17.6.5.12](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3337.pdf)
and the corresponding sections in `doc/n3485.txt`.

`/usr/include/x86_64-linux-gnu/c++/15/bits/c++config.h:247` defines
`_GLIBCXX_THROW(_EXC)` to nothing for C++11; `/usr/include/c++/15/new:137`
uses it on `operator new`. The same declarations remain after `cppgm++ -E`.
The header-free reducers in `student.tests/pa30/source201/` are
`new-hosted-reduced.reject.cpp` (retains the exception type name) and
`new-spec-reduced.reject.cpp` (renames it). Both contain an explicit unrestricted
allocation declaration followed by a restrictive definition.

[Observations](../student.tests/pa30/evidence201/exception-observations.json)
retain source hashes, commands and diagnostics. The bundled reference accepts
the hosted fixture and the reducer retaining `std::bad_alloc`, but rejects
the identical mismatch after renaming that type; g++ accepts even
`throw(int)` on replacement new. Compiler agreement is not the proof.
The bundle source revision is `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`,
archive SHA256 `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`,
compiler SHA256 `e32741e2504f7c75a91abdf5cea0d5fc0b56eb43deb503d2ffbddbcc2f854f70`.
The bundle and manifest themselves are unchanged.

The wrong-type hosted rejection remains required. Explicit controls retain
successful hosted replacement-new emission with a matching unrestricted
declaration, linked throwing/catching behavior, matching dynamic declarations,
set-order/duplicate normalization, and rejection of unrestricted/restricted and
changed-set redeclarations. No compiler rule is weakened to fit the reference.
