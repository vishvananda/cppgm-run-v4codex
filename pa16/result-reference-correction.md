# Nontrivial empty result reference correction

The [reduced source](../student.tests/pa16/initialization/empty_result.cpp)
retains the failing fixture's dependent constexpr declaration, empty class and
user-provided empty destructor. The pinned bundle is source
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
The [revision record](../student.tests/pa16/result-reference-revision.json)
binds old/new oracle hashes and the unchanged source. The bundle is unchanged.

[N3485](../doc/n3485.txt), 12.4 [class.dtor]/5, makes a destructor trivial only
if it is not user-provided and satisfies the remaining conditions. An empty
body does not make this destructor trivial. 12.2 [class.temporary]/3 permits
the additional register-passing temporary only for a class with trivial copy
constructor **and destructor**. The dependent constexpr declaration is allowed
by the PA16 assignment boundary; instantiation does not reapply literal-type
declaration checks.

The [Itanium ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#non-trivial-return-values),
3.1.3.1, requires caller-supplied result storage for a class nontrivial for
calls; 1.1 includes a nontrivial destructor, and 3.1.3.4's empty-class direct
return exception applies only to classes trivial for calls. The PA12 indirect
result convention and PA8 [object boundary contract](../pa8/lowir.md#object-abi-conventions)
represent this address as the first `ptr [pass=indirect_result]` parameter with
void result. PA12 additionally requires `object_bytes=1` here. The external
object identities in the oracle retain this source ABI; empty bodies do not
authorize changing the externally published call convention.

The correction changes only the two factory signatures, their result-storage
uses/returns, and the two corresponding calls. Caller storage, destructor
calls, exception regions, aliases, linkage and all other instructions remain
unchanged. It is an ABI-contract correction, not a claim that these particular
empty bodies produce a different observable result in a closed executable.
No comparator, source fixture, expected exit status or coverage was changed.

Separately, the compiler now records the full expression's final temporary
identity and avoids eagerly reopening an EH region after that result is
activated. There is no remaining expression evaluation. Destructors that can
throw still establish the live cleanup suffix through the ordinary guarded-call
path. This removes the extra compiler regions without copying oracle code or
changing the already-correct indirect result classification.
