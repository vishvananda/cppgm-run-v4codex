# Implementation175 — source invocation facts

Entry `235ffa3947d7921a1c34c66198f590df5aa77de2`; code through `704538c1`.
This is an implementation handoff, not an independent whole-stage audit.

## Ownership, data flow and complexity

The shared builtin registry admits FILE, LINE, FUNCTION and COLUMN with fixed
zero-argument signatures and nonthrowing intrinsic identities. Source sites retain
presumed file, line, lexical function spelling and source-node identity. Locations
come from the existing token cursor, including macro expansion and `#line`;
there is no source-text recognition or second parse. Each checked occurrence owns
one indexed site. Lexical function ancestry is memoized by scope, and function
spellings are memoized by function scope. Work follows nodes and lexical edges,
not the Cartesian product of declarations and calls.

A checked default argument retains its declaration-owned expression and conversion
recipe. A conversion bit marks its invocation use. Stack-local invocation state
is shared in shape by the constexpr evaluator and typed lowering: direct calls
install their site; default evaluation inherits that site through nested defaults;
an executed function body restores lexical context. Constructors retain sites on
subobject actions, and implicit object initialization retains its declarator site.
This covers multiple constructors, synthetic constructors, implicit bases,
arrays, local statics and allocation. It does not clone a default expression or
repeat overload selection. Query value keys include invocation site and evaluation
mode. Existing per-query revisions invalidate all context variants lazily. Storage
queries and ordinary node/static-value caches cannot reuse a declaration-only
answer while evaluating an invocation-dependent default.

LINE and COLUMN lower directly to scalar constants. FILE and FUNCTION consume
interned text facts through a typed support-string queue keyed by IdentifierId.
Constant evaluation can establish anonymous string object identities; lowering
reserves their support symbols separately from declaration/lifecycle tables.
Data is written once after other globals, so a newly demanded string cannot
split an aggregate initializer's data slice. A constexpr character read needs no
emitted string. Line-only code emits none. TU-owned sites, strings and semantic
facts are released with their owners; per-evaluation guards are stack-local.
The new source-site capacity check protects the 32-bit query-context key.

Direct-call restrictions are checked for addresses, function decay and queries.
Validated intrinsics have no body to enqueue in deferred definition demand; this
also fixes the semantic-view path for source calls in constexpr bodies.

## Contract and validation

Invocation behavior follows the documented GNU/Clang extension:
[GCC other builtins](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html) and
[Clang source location builtins](https://clang.llvm.org/docs/LanguageExtensions.html#source-location-builtins).
Default arguments use the caller; default member initializers use the selected
constructor; ordinary function bodies use their own lexical site. A function name
outside function scope is empty. Function spelling follows this compiler's
existing `__FUNCTION__` convention; template pretty-signatures are not invented.
COLUMN preserves PA29's explicitly permitted unavailable-column value, zero
([course fixture](tests/run/800-source-location-builtin-macros-run.t.1)). No
reference, test, harness or comparison rule was edited.

[17 controls](../student.tests/pa29/evidence175/controls.json) cover direct/global
calls, nested defaults, references and class temporaries, side effects, constexpr
and runtime agreement, template defaults/body queries, macros/includes/`#line`,
member initialization and implicit construction, strings/relocations, noexcept,
wrong arity, addresses/decay and invalid qualification. Eight executable controls
match GCC results; COLUMN has the explicit course expectation. The NTTP query
control checks template function-name content rather than GCC's optional template
signature decoration. The pending165 source-invocation reducer is covered by the
expanded controls and resolved.

[Inspection](../student.tests/pa29/evidence175/inspection.json) executes 37 commands
plus assertions: AST, the PA7 semantic view on its supported inputs, LowIR and
external LowIR validation, native relocations/disassembly, no external source
builtin calls, no terminate landing in an intrinsic-only noexcept function,
telemetry-identical objects, and absent unused source-string data. Template NTTP
class evaluation remains on the production/LowIR path: the early PA7 semantic
renderer does not instantiate that class. No production requirement is waived.

[Required checks](../student.tests/pa29/evidence175/validation.json): PA29
**378/403**, PA1–28 **4538/4538**, through PA29 **4916/4941**, file audit pass
(four inherited warnings). The exact [delta](../student.tests/pa29/evidence175/stage-delta.json)
is **26 → 25 failures**, fixing the source-location fixture with no new failures.
All **403** fixtures and **1,707** contract/harness paths are preserved by the
[coverage comparison](../student.tests/pa29/evidence175/coverage.json).

## Handoff boundary and review ledger

The source-invocation group has no known unfinished required implementation in
these paths. It includes the related constructor, constant/query cache, support
string demand and deferred intrinsic uses exposed while extending the initial
fix. Further course failures require independent mechanisms: extended numeric
representations, decomposition/control-flow syntax, declaration/template emission
and ABI, or an actual forced-inline transformation (the current pipeline only
retains that attribute). None can be implemented by extending source-site facts.
That ownership boundary, rather than the one-fixture progress minimum, ends this
handoff. PA29 remains unfinished and cannot advance.

The [remaining ledger](../student.tests/pa29/evidence175/remaining.json) distinguishes
unfinished implementation from audit170's two independent contract questions:
nothrow default-construction shorthand and nothrow-invocable cache default. Both
remain counted failures. The char-traits reducer remains implementation work.
Independent review must also assess this cumulative code and evidence; green
controls are not a waiver of architecture, performance, or whole-stage review.
Stage base and Last reviewed commit in `plan.md` are unchanged.
