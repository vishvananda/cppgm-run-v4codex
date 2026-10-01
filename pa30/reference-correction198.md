# Base typedef lookup correction (audit198)

The unchanged `400-base-same-type-alias-ambiguous-bad.t` reduces to:

```cpp
struct first { typedef int value_type; };
struct second { typedef int value_type; };
struct derived : first, second { value_type value; };
```

This is well-formed C++11. [dcl.typedef] §7.1.3/1 makes each typedef a
synonym for its associated type. [class.member.lookup] §10.2/3 normalizes
type declarations to their designated types before merging lookup sets.
Both base declaration sets are therefore `{int}`. Neither base dominates
the other; the equal-set branch of /6 merges their subobject sets, and /7
returns the valid type set. These are public aliases and public bases, so
access imposes no additional restriction. There is no nonstatic member
access or ambiguous derived-to-base conversion in this declaration.

Proof source: [WG21 N3337](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3337.pdf),
§7.1.3 and §10.2 (printed pages 222–223); identical lookup wording is
retained in `doc/n3485.txt:12777–12811`. The normalization predates C++11:
[CWG 39](https://cplusplus.github.io/CWG/issues/39.html) introduced it in
the resolution adopted in April 2005. This is independent of the namespace
rule documented in [reference-correction197](reference-correction197.md).
That record's provisional claim that distinct member typedefs must remain
ambiguous is superseded by this proof.

The observed GCC rejection is not normative; Clang accepts the reducer.
Neither observation is the basis of the correction. The implementation
compares canonical TypeIds in the existing lookup merge owner, excludes
alias templates, and retains a declaration representative for access checking.
It does not change variable/function identity or base conversion rules.

Bundle binding: `reference-binaries/manifest.tsv` source revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Only this fixture's `.ref.exit_status` and `.ref.stdout` change: successful
compilation and empty stdout. Its input, empty `.ref`, filename, discovery,
timeout, comparison rules and all other PA30 oracles remain unchanged.

`source198/base-alias.cpp` checks fundamental/class aliases, both base orders,
qualification, dependent base aliases and executed object layout. Neighboring
controls still reject distinct types, distinct values, distinct alias templates
and inaccessible member aliases. `check198.py` explicitly runs all checkpoint
controls, applying this correction to the unchanged personal base reducer from
197. Historical measurements and expectations remain available at their commits.
