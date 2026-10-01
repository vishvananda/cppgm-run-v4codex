# Namespace typedef lookup reference correction (implementation197)

Reduced input (also unchanged in
`pa6/tests/general/300-ambiguous-using-directive-type-bad.t`):

```cpp
namespace left { typedef int T; }
namespace right { typedef int T; }
using namespace left;
using namespace right;
T value;
```

The prior rejection mistakes two declarations naming one type for two different
entities. C++11 [dcl.typedef] §7.1.3/1 says a typedef-name names its associated
type and introduces no new type; /2 gives alias-declarations the same semantics.
[basic] §3/3 includes types among entities. [namespace.udir] §7.3.4/6 makes the
use ill-formed only when the declarations do not declare the same entity (and
are not functions). Here both denote `int`, so `value` has type `int`.
Sources: checked-in `doc/n3485.txt`, lines 8105–8120 and 9541–9544;
[WG21 N3337](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3337.pdf),
the same C++11 clauses. [CWG 2218](https://cplusplus.github.io/CWG/issues/2218.html)
later clarified lookup's declaration-set wording and explicitly describes
alias-declarations as the established same-entity case; it is corroboration,
not the basis for imposing a newer language feature.

This does not collapse class-base declaration sets: [class.member.lookup]
§10.2/6 compares declaration sets and subobjects before §10.2/7 permits use of
an unambiguous result. Distinct base member typedefs remain ambiguous.

Bundle binding: `reference-binaries/manifest.tsv` source revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Only the three PA6 output/status sidecars are revised: success, empty stdout,
and the full type report with global `value int` and both namespace aliases.
The input, filename, discovery, comparison rules and number of cases remain
unchanged. No reference executable or PA30 oracle is changed.

Explicit controls in `student.tests/pa30/source197` retain rejection for distinct
types, variables, namespace targets and base member aliases. Positive controls
exercise class type aliases, qualification, transitive/cyclic directives and
namespace aliases. Compiler agreement is not the proof above.
