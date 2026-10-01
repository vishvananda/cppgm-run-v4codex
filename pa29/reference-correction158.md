# PA29 checkpoint 158 reference correction

Only `tests/compile/500-builtin-trivial-deleted-copy.ref.exit_status` changes,
from `EXIT_SUCCESS` to `EXIT_FAILURE`. The original source and every assertion
remain unchanged. The compiler already rejects it; no implementation is changed
to recognize this fixture. Diagnostic text remains ungraded.

The pinned reference bundle is still source revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, archive SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7` in
[the manifest](../reference-binaries/manifest.tsv). This is a checked-output
correction for that bundle, not a claim that its executable was repaired.

The [minimal rejecting reducer](../student.tests/pa29/controls158/deleted-copy-reject.cpp)
contains the deleted copy constructor and the first false assertion. The
[positive control](../student.tests/pa29/controls158/deleted-copy.cpp) retains all
three properties from the original and independently checks unusable copy and
assignment, plus a user-provided nontrivial copy constructor.

Proof from [C++11 working draft N3337](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3337.pdf):
[dcl.fct.def.default] §8.4.2/4 excludes functions defaulted or deleted on their
first declaration from user-provided functions. [class.copy] §12.8/12 and /25
therefore classify this empty class's copy constructor and assignment as trivial:
there are no bases, members, or virtual functions. No move operations are
implicitly declared because of the user-declared copy operations (§12.8/9,20).
The defaulted default constructor and implicit destructor are trivial
([class.ctor] §12.1/5, [class.dtor] §12.4/5).
[class] §9/6–9 thus makes the class trivially copyable, trivial, standard-layout,
and POD. Each negated property in the original assertion is false, and
[dcl.dcl] §7/4 requires a diagnostic for a false static assertion.

[Compiler observations](../student.tests/pa29/evidence158/reference-controls.json)
supplement this clause proof; agreement alone is not the justification.
The default harness still runs all 403 fixtures with identical comparison rules.
The explicitly run positive control preserves the three property checks that
cannot all be reached after the original's first required rejection.

The separate forward-declared `std::is_nothrow_*` shorthand fixture is preserved.
[temp.inst] §14.7.1/7 rejects required instantiation of an undefined template;
[an ordinary-namespace reducer](../student.tests/pa29/controls158/forward-trait-reject.cpp)
checks that, and [a defined-template control](../student.tests/pa29/controls158/defined-trait.cpp)
checks all five nothrow properties. However, the original also adds declarations
in `std`, subject to [namespace.std] §17.6.4.2.1/1. That prevents using this
ordinary reducer alone as a strict proof about the original. No oracle change
or special handling of a library type spelling is made for it.
