# PA29 trait and reference-binding handoff159

Code endpoint: `5d2b1657`; entry: `044d9627`. This completes the legacy member
trait and reference-temporary implementation group, not PA29 or its audit.
No fixtures, references, comparison rules or discovery rules changed.

## Ownership and data flow

- `support/type_traits.h` supplies the parser and `__has_builtin` with one
  bounded registry: six legacy construction/copy/assignment properties and
  three reference-temporary properties. Arity is checked by the common query
  evaluator. Dependent operands retain typed queries and substitute canonical
  arguments; no tokens are replayed and no secondary syntax graph is built.
- `legacy_traits.cpp` owns member properties. It completes declarations, visits
  the relevant constructor/assignment family, and consumes exception facts or
  structural triviality. Structural recursion follows each selected subobject
  member, including source cv and mutable fields. It does not equate deletion,
  inaccessible members or throwing destruction with nontrivial construction.
  `__is_trivial` also consumes this structural default-constructor fact;
  constructibility retains its separate usability checks.
- `conversion_functions.cpp` owns the common reference-binding phases:
  standard binding, applicable lvalue conversion, applicable rvalue conversion,
  then indirect copy initialization. Explicit functions participate in direct
  initialization's permitted candidate set. Invalid/ambiguous candidates are
  ordinary typed conversion results, not diagnostic exceptions. Ordinary
  initialization, template initializer checking, casts and operation traits use
  this path. Lowering consumes the selected declaration/conversion recipe.
- `reference_traits.cpp` supplies the hypothetical source's type/category and
  inspects that checked recipe. Both a fresh source prvalue and a newly created
  result object must permit destruction. Existing lvalues/xvalues do not acquire
  a source temporary. Reference-returning conversion functions preserve object
  identity; a non-reference return or value-changing conversion creates a
  temporary. Function references do not materialize objects.

## State, complexity and limits

Class properties use the existing flat index keyed by operation and type;
structural member properties use a TU-owned flat index keyed by declaration ID.
Both cache positive and negative results, mark in-progress computation, and
restore not-started state on unavailable dependencies. They are established
after class completion. Later body emission and access contexts cannot change
structural triviality. Exception specifications retain their existing owner.

Work follows relevant member families, selected subobjects and conversion
candidates. Reference initialization has a fixed number of language phases;
there is no global registry scan, graph retry or unbounded fixed point. Each
completed structural fact is computed once. Query and conversion storage is
the existing compact TU representation, with stack expression views and no
fake frontend nodes. The new index is released with the analyzer. No additional
representation survives into native emission.

Telemetry increments two work counters where facts are computed; it does not
request analysis. Repeating the same five traits 100 and 10,000 times leaves
type-query work at 5, legacy class-property work at 4, member-property work at 3
and template body transitions at zero. [Inspection evidence](../student.tests/pa29/evidence159/inspection.json)
also checks selected native calls: the unused alternative `Both::operator int`
is absent. The native runtime control and its LowIR roundtrip agree.

## Semantic references and controls

[GNU's legacy trait definitions](https://gcc.gnu.org/onlinedocs/gcc/Type-Traits.html)
distinguish these member properties from expression constructibility. The
controls include private/deleted members, throwing copies versus nothrow moves,
overloaded copy/assignment families, default arguments, arrays, cv/ref types,
virtual functions, late defaulting and selected mutable/const subobjects.

The checked-in [C++ draft](../doc/n3485.txt), [class.ctor], [class.copy] and
[dcl.fct.def.default], separates structural triviality from deletion. The
less-qualified defaulted-copy rule follows the adopted
[CWG 2171 correction](https://cplusplus.github.io/CWG/issues/2171.html).
Reference phases follow [dcl.init.ref]/5 and [over.match.ref], including hosted
acceptance of scalar prvalue direct binding as in the
[current binding wording](https://eel.is/c++draft/dcl.init.ref).
[Clang's documented primitives](https://clang.llvm.org/docs/LanguageExtensions.html#type-trait-primitives)
and [P2255's hypothetical expression categories](https://open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2255r2.html)
define the hosted lifetime queries; the deprecated binds spelling shares the
direct-query semantics. [class.temporary]/1 requires destructor validity even
for an unevaluated class temporary. Host observations supplement these rules;
they are neither production dependencies nor substitutes for the rules.

## Boundary and independent review

The [validation manifest](../student.tests/pa29/evidence159/validation.json)
and [performance report](performance159.md) cover this complete code endpoint.
The next implementation owners require distinct work: atomic storage/order and
assembly, extended numeric/vector/syntax representation, template demand/ABI,
and structured intrinsic operands. Extending those here would require new
models and reducers rather than further application of the trait/conversion
facts established here.

Two retained failures need independent contract review: the forward-declared
`std::is_nothrow_*` fixture from audit158, and the cache fixture whose declared
`__is_nothrow_invocable` primary always inherits false. The latter uses reserved
identifiers, so an ordinary-name reducer alone cannot settle its contract.
Neither justifies recognizing library spellings or discarding source-defined
semantics. Their failures and coverage remain. Independent review must also
assess this handoff's conversion phase ordering, class-completion cache
validity and structural/usable-member distinction; these are review obligations,
not unfinished implementations or waived requirements.
