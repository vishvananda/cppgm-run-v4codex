# PA18 demand and discarded-value oracle corrections 85

Entry: `f953e42a875bc8f7e3b9882a324db843f0c4191a`. The pinned bundle stays at
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, archive SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
This local revision changes three `.ref` files, retaining all 420 source inputs,
exit-status sidecars, metadata and comparison rules.
[The transformer](../student.tests/pa18/reference85.py) reads only entry oracles,
checks that the removed values are unused and preserves every other byte.
[The manifest](../student.tests/pa18/reference85-revisions.json) records both hashes.

## Dormant static definitions

The keyword-overload fixture never uses `keyword<Tag>::instance`; the source-owner
SFINAE fixture never uses `string_like<char>::npos`. Their owners' instantiation
requires member declarations, not definitions. Replace the two definitions with
the same declarations, preserving symbols, types and metadata.

[N3485 §14.7.1 [temp.inst]/1,2,8,10](../doc/n3485.txt:19603) distinguishes these
facts, forbids unnecessary static-member instantiation, and expressly excludes
initialization side effects without a use requiring the definition. This is the
same rule already applied to [PA17's dormant-static correction](../pa17/storage-references.md).
A reducer with an observable consequence is:

```cpp
int hits;
int init() { return ++hits; }
template<class T> struct X { static const int n; };
template<class T> const int X<T>::n = init();
int main() { X<int> x; return hits; }
```

The required result is zero. Replacing the initializer by `T::missing` must still
compile because the definition is dormant. Demanding `X<int>::n`, or explicitly
instantiating that member/class, instead requires the definition. The dedicated
controls cover both sides, including objects, references, late definitions,
namespace instances and transitive storage demand. This proof does not depend on
agreement with a host compiler or the student's output.

## Discarded reference result

The transitive-base deduction fixture casts a reference-returning call to `void`.
Its oracle correctly calls the selected function, then incorrectly loads the
referent. Remove only that unused `load i32`; preserve the call and all deduction,
base-adjustment, signature and body coverage.

[N3485 §5 [expr]/11](../doc/n3485.txt:5183) specifies when a discarded expression
undergoes lvalue-to-rvalue conversion. A function call is outside its listed
built-in forms, and a nonvolatile reference result does not qualify either.
[§5.2.9 [expr.static.cast]/6](../doc/n3485.txt:6049) makes conversion to void a
discarded-value expression; §5.4 [expr.cast]/4 includes that conversion for the
C-style cast. The reduced, defined program is:

```cpp
volatile int value = 7;
int calls;
volatile int& get() { ++calls; return value; }
int main() { (void)get(); return calls != 1; }
```

The call is required, and a volatile read of `value` is forbidden here. An adjacent
`(void)value` **does** require a volatile read. Structural checks distinguish them;
execution alone cannot count volatile accesses. The original course helper forms
a reference from a null pointer, so its native execution is not used as proof of
C++ correctness. Defined reducers establish the conversion rule without that
unrelated undefined behavior. The corrected course LowIR remains structurally
validated and compared under the unchanged harness.
