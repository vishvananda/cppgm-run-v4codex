# Implementation176 — hosted declaration identity and emission

Entry: `2b7a513297b2e9719fcd5686d3c1c97ee7565b13`, 378/403 course tests (25 failures).
The original Stage base and Last reviewed commits in [plan.md](plan.md) remain
unchanged. This is implementation work; independent review remains required.

## Ownership and data flow

**Explicit-instantiation exclusion.** Syntax retains the GNU/Clang attribute in
`NativeAttributes`; the canonical declaration owns its boolean semantic fact.
Projected declarations retain an indexed edge to the source pattern so an
out-of-class definition can strengthen that fact without replaying declarations
or invalidating unrelated specializations. Matching the definition updates only
its selected prototype. `explicit_instantiation` skips excluded children of the
selected class, and suppression walks only the actual enclosing class chain.
Direct instantiation of the member still overrides this class-level policy.
Nested excluded classes stop inherited suppression; functions, static data and
member classes use the same rule. The preprocessor advertises the implemented
attribute under both GNU spellings. See the
[Clang attribute contract](https://clang.llvm.org/docs/AttributeReference.html#exclude-from-explicit-instantiation).

**Inline variable identity.** Semantic declarations retain a distinct
`inline_variable` property, separate from function inlining and source `static`.
Namespace inline constants have external linkage unless explicit static or an
internal enclosing scope supplies internal linkage. Lowering consumes these facts
when publishing weak ELF definitions. Explicitly inline static class members are
definitions; duplicate definitions within one TU remain errors. Const variable
templates retain their inline fact through specialization. Declaration-only,
internal-linkage and merged source-unit paths preserve their separate rules.

**Initialization and lifetime.** Each dynamically initialized non-TLS inline
object reuses the typed static-duration initializer/lifetime machinery. One
64-bit weak guard with a typed Itanium `Guard` target is shared across TUs.
Initialization and destructor registration occur once. References that extend
temporary lifetimes publish typed `ReferenceTemporary` / `ReferenceGuard` ABI
targets with source preorder ordinals and weak linkage; separate registration
guards preserve host interoperation and reverse destruction order. Parent object
destructors run before the temporaries they reference. Initialization guards are
published before temporary cleanup can reenter. Constant objects without destruction acquire no guard. TLS scalar
objects use the existing weak ABI initializer/wrapper entry. Generated guard
names consume the canonical ABI entity, without reverse parsing a symbol string.
See [Itanium guard variables](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#guards)
and [guard-name encoding](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-special-guards).

**Deferred member initializers.** Completing a class specialization retains an
inline static member's declaration and parsed initializer recipe. A separate
fact state records not-started, active, successful or failed initialization.
Value demand and storage demand share checking, but only storage demand permits
emission. An omitted array bound remains incomplete until an expression/type
query requires its deduction; class completion does not check the initializer.
The completed array type is published before query/expression facts consume it. Explicit class/member instantiation supplies the appropriate storage
request, subject to exclusion/suppression. In-progress self references consume
the existing declaration; they do not start another initializer. Ordinary and
deferred definitions call the same object-initializer checker. This extraction
preserves each declaration's own initializer argument on redeclaration.

## Complexity, allocation and release

Attribute and initializer records belong to the translation-unit analyzer.
Indexes use the existing flat numeric `Index`; recipes retain compact node,
scope and entity identities. Template regions stay parsed once and occurrence
projection supplies substituted facts. No source replay, cloned semantic graph,
text-keyed lookup, global retry or optimizer pass was added.

Exclusion checks visit only the declaration's pattern edge and lexical class
ancestors. Each complete initializer fact is checked once, with constant-time
average completed lookup; work follows its actual expression and dependency
edges. Low-overhead initializer/hit counters are exposed through `--stats`.
Support globals and guards have one identity per emitted object. Lowering and
native emission remain proportional to produced IR/data. The analyzer releases
records with its TU; lowering scratch follows the existing function/TU lifetime.

## Independent contract question: nested member ABI tag

The unchanged course fixture
`800-out-of-class-nested-template-abi-tag-suppression-run` expects an out-of-class
nested-template member's declared tag to disappear. The reducer
[`tag-review.cpp`](../student.tests/pa29/source176/tag-review.cpp) produces
`_ZN3BoxIiE6Member5valueB4keptEv` with this compiler and GCC, but
`_ZN3BoxIiE6Member5valueEv` with Clang. The
[GCC attribute documentation](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Attributes.html)
says tags apply to names and remain on template instantiations; the C++11 standard
does not prescribe this GNU extension's encoding. Compiler agreement alone is
not the permitted proof for changing a reference. No fixture, oracle or comparison
rule is changed, and this failure remains counted. Independent review must
resolve the supported hosted contract; it is not waived or silently implemented
as a source-pattern naming exception.

The [compact plan](plan.md), [performance report](performance176.md) and
[evidence manifest](../student.tests/pa29/evidence176/manifest.json) record final
validation, measurements and the incomplete-stage handoff boundary. General
nontrivial hosted TLS lifetime work remains in the later hosted-runtime scope;
this group validates constant and dynamically initialized scalar TLS identity.

## Representative source-to-object trace

A class specialization that declares an excluded inline static member first
publishes the canonical member and an initializer recipe. A selected member-body
read requests storage through that entity, bypassing enclosing extern-template
suppression only because the retained attribute says to do so. The initializer's
substitution context supplies its template argument without parsing again.
Its fact moves through active to complete once; repeated reads reuse it. The
lowerer consumes the completed type, initializer conversions, lifetime facts,
inline linkage and selected calls. A dynamic object receives its own weak data
symbol and a typed guard ABI identity; its checked initializer contributes one
sequence to the unit initializer. Function and data emission use the existing
LowIR/MIR/ELF pools. Encoding never repeats language lookup or substitution.
The scaling benchmark exercises this trace at three declaration counts.
