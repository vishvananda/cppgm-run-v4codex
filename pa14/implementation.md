# PA14 implementation ownership

Current contract result: **314/314**, zero course failures; prior assignments **1621/1621**.
The transfer continuation resolved **all 17 of its entry failures**, with no lost passes.
The following body-fact continuations preserve 314/314 and share fixed scalar,
call and constructor-argument facts, including definition-time operand/access
legality and occurrence-owned materializations.
Cumulatively, **all 230 stage-entry failures** are resolved. Fixtures,
references and comparison rules are unchanged. PA15 has not been started.

## Owners, data flow and bounds

| Owner | Implemented data flow / complexity | Validation |
| --- | --- | --- |
| `syntax/ast.h`, `occurrence.cpp` | One parsed graph. Compact source/context occurrences project structural edges; nested specialization keys use original source identities. Semantic environments retain enclosing arguments. Ordinary reads keep the inline O(1) path. | Nested member-operator specializations, source-read benchmarks, sanitizer parity. Whole-region occurrences remain a limitation. |
| `semantic/template_declaration.cpp`, `template_call.cpp` | Parameter ordinals normalize callable shapes; canonical template entities and interned argument packs index specialization declarations. Direct and target deduction use ordinary conversion/ranking rules. Operator member deduction excludes the implicit object through a bounded argument view. | Declaration/call/address cases, overloaded arguments and nested right-shift operator. |
| `semantic/dependent_type.cpp`, `types.cpp` | Canonical dependent type qualifiers, member identifiers and argument slices. Substitution follows dependent structural edges; fixed TypeIds are shared. Concrete member lookup demands the necessary class declaration. | Qualified member aliases, renamed heads, nested concrete type identity. |
| `semantic/type_query.cpp`, `type_query.h` | Canonical query IDs retain bound names, parameter ordinals, literal/null provenance, operations, types and child slices. Each query has Active/Success/Failure state. Substitution visits dependent edges and reuses completed fixed queries. | All five previously failing dependent-signature cases now pass; query-instance scaling and work counters. General casts, assignments, conditionals, dependent value paths and array bounds are still incomplete. |
| `semantic/query_call.cpp`, `query_operator.cpp`, conversion/operator helpers | Typed query operands reuse standard/user conversions, constructor conversions, builtin operator legality, contextual bool, deduction, ADL and ranking. Query facts retain selected declarations and conversion ranges without demanding called bodies. | User conversion, converting constructor, overloaded arithmetic, reference-array results, hidden-friend ADL, null literal distinctions; seven query rejections. This does not yet cover every ordinary callable/surrogate form. |
| `semantic/type_builder.cpp`, `lowering/query_abi.cpp`, `abi_mangle/*` | Function signatures get parameter scopes before trailing returns and dependent `decltype`. Typed queries feed the existing ABI graph; unresolved names have typed qualifier/name/argument edges. Declarators with an already-resolved owning environment retain that scope. | Renamed declarations, trailing returns, function-pointer/reference queries and two independent unresolved-name ABI encodings. No text is used as a semantic key. |
| `semantic/template_class.cpp` | Class identity precedes completion. Defaults keep their declaring head; ordinal substitution merges defaults across renamed heads. Fixed bases are validated at definition time. | Default owners, forward upgrades, fixed-base rejection and dependent alignment. |
| `semantic/template_binding*.cpp`, `template_binding.h` | Definition-owned lexical class/function/block/control scopes retain declaration identities and indexed using/base edges. Nested bodies wait for complete-class declaration contexts. Name facts bind fixed external declarations or retain dependence; concrete occurrences consume only fixed definition-wide facts. Contextual nested bindings are not published into that source-wide cache. | Unused missing/ambiguous/type-as-value rejection; condition visibility; fixed versus dependent direct/local bases; anonymous/bit-field values; renamed heads; nested-specialization runtime reducer. Work follows source nodes and required lookup edges once per pattern. |
| `semantic/jump_validation.cpp` | The existing immutable initialization-prefix graph checks explicit initialization barriers in unused bodies. Prefix ancestry is O(1) per jump after a linear numbering pass. Runtime destruction/lifetime actions remain separate demands. | Switch labels after initialized locals, goto barriers, condition declarations and valid exits from scopes. Dependent implicit class initialization barriers still need richer facts. |
| `semantic/template_definition.cpp` | Owner-path/member buckets retain parsed nested classes, functions and static definitions. Definition-time parameter overlays sit over pattern owners. Concrete root/definition applications have Active/Success/Failure state and narrow body/storage demand. | Qualified renamed heads, nested/static definitions, unused out-of-class rejections; 4× definition scaling. Overloaded buckets still apply every matching-name definition. |
| `semantic/explicit_instantiation.cpp`, member/storage demand | Explicit class demand visits that class's declarations and available definitions. Evaluated static values/addresses request storage; unevaluated operands do not. Bodies retain separate demand. | Explicit qualified/global class demand, namespace/class-key rejection, unused invalid initializers. Full extern emission suppression and general explicit function instantiation remain incomplete. |
| `semantic/inherited_constructors.cpp`, parser contexts | Constructor using-declarations recognize aliases naming the direct base. Declaration-owned parser scopes preserve qualified class-template names, head names and member precedence. | Alias-inherited constructor and lazy right-shift member lookup now pass. |

All indexes, source regions, query pools, argument packs and binding scopes are
translation-unit owned. Candidate and traversal vectors are temporary. Packed expression flags restore
the original 36-byte expression record; declarations remain 112 bytes. There is
no grammar replay, reference delegation, global restoration retry or persistent
semantic cache. The optional optimizer/native backend have not been introduced.

## Remaining architecture and concrete checkpoint boundary

The transfer, lifetime, local ABI, expression-output and retained-syntax groups
are complete for all 314 course fixtures. Their selected facts now reach the
ordinary lowering path; no fixture, reference or comparison rule was changed.
The previous query/binding group was extended through these consumers rather
than stopping at a test-progress threshold.

| Owner | Current PA14 work still required by `spec.md` |
| --- | --- |
| Typed template body facts | Nonstatic member objects, constructor/operator expressions, declaration/return conversions, general expression/bound queries, and dependent-only checking instead of whole-region semantic projection. Fixed scalar/call/argument recipes are shared. |
| Demand and failure facts | Finer declaration/layout/default/exception/body states, typed reasons and reverse dependency edges, narrow structured expected failures. |

Parsed-node sharing and fixed name/query/scalar/call-argument sharing are implemented. Sharing all
nondependent semantic body facts is **not** complete: concrete bodies still
project entire regions and recompute many type/conversion facts. Pattern-owned nonstatic object/member facts, declaration/return conversions,
finer occurrence demand and structured expected rejection need a broader typed
body graph. This is the concrete incomplete handoff
boundary, not a commit or progress threshold. These requirements are not waived
by the stage-scoped performance review or deferred to PA15.

## Transfer, layout and local ABI continuation

- `prepare_transfer` owns reference binding actions and legal storage prefixes.
  Late defaulting remains nontrivial; only unrooted specializations with proven
  representation actions may lower directly. Ordinary external definitions
  retain their calls. Preparation is linear in subobjects, with the inherited
  bounded array lowering unchanged.
- Constructor actions retain the selected conversion constructor and base-entry
  use. Lowering consumes those facts, omits empty payload work, and still
  evaluates source expressions. The class return owner admits an indirect
  return destination for an effect-free empty destructor without changing ABI
  triviality; effectful destruction keeps the existing lifetime path.
- `sizeof` publishes its constant index after recursive layout dependencies
  finish, avoiding an index into a nested enumerator's constant.
- Local enums retain function/ordinal identities. ABI locality is memoized over
  canonical type arguments and scope edges, so member entries for local-type
  specializations carry internal rooted metadata without repeated graph walks.
- Validation: 306/314 current, 1621/1621 prior, file audit, twelve native personal
  programs. `object-transfers.cpp` checks reference identity, late-defaulted
  parameter ABI, empty-source effects, return moves, base copies and reentrant
  layout. No new performance benefit is claimed before measurement.

## Lifetime entries, expression provenance and retained syntax

- Static initialization owns a per-object vptr fact when the demanded implicit
  constructor has no subobject actions. Lowering emits the relocation and zero
  tail directly; user bodies and hierarchy construction keep dynamic actions.
- Deleting-entry sharing counts nontrivial destruction actions, retaining the
  single-action bound instead of counting trivial fields as emitted work.
- Hidden friend bodies are checked normally but emitted on recorded function
  use. Ordinary calls, operators and address selection share `demand_member`;
  uses preceding the friend definition remain recorded.
- Explicit arithmetic casts retain signedness conversion provenance. Template
  layout queries keep their surrounding O0 conversion, while an unrelated
  specialization cannot perturb a fixed layout immediate. Discarded names and
  literal conditions follow the ordinary O0 expression/control path.
- The parser records variable templates as value templates so later template-id
  declaration forms remain parsed. This adds no variable-template specialization
  semantics to PA14.
- Validation: all 314 course tests, all 1621 earlier tests, thirteen native
  personal programs, 21 binding/query checks and file audit pass. The remaining
  architecture items described above are not erased by the course result.

The final emission-use representation occupies existing declaration padding
(`Entity` remains 112 bytes). A byte of flags records hidden-friend origin and
evaluated use, including uses preceding definition, without an auxiliary map
for ordinary function calls. The intermediate map implementation and sanitizer
parity evidence remain frozen in the transfer artifact directory.

The performance review exposed preparation of already-deleted implicit copies.
`prepare_transfer` now publishes the known negative fact before creating a
function scope, parameters or subobject actions. On the transfer corpus this
removes two unused actions per specialization while preserving the selected
move and its reference-binding actions. The first 350-observation campaign
remains preliminary evidence; a separate final campaign measures this change.
The through report remains 1935/1935 and thirteen native reducers pass.

A final return-boundary reducer also excludes volatile locals from both NRVO
and implicit-move eligibility, as required by N3485 [class.copy]/31–32
(`doc/n3485.txt:15295`). The qualifier check belongs to the source object,
independently of the function's return type. The frozen reduced program returns
1 before the fix and 0 afterward (one volatile-lvalue copy, no move); it is
also exercised by `object-transfers.cpp`. No reference output was changed.

## Shared scalar body facts

`semantic/template_expression.cpp` extends definition-time binding with fixed
scalar expression checking. The existing source graph owns the completed
`Expression`, type/category, constant identity and operand-conversion slice.
The fixed index maps immutable parsed-source identity to its semantic NodeId;
these differ when parsing continues after an earlier class instantiation.
Only original definition contexts publish facts. Contextually bound nested
definitions cannot populate this source-wide index.

Instantiation reuses conversions and maps pattern local/parameter declarations
through the occurrence's declaration fact. It retains occurrence-local scope,
incoming conversion and evaluation/observation facts. Reference/address and
mutation observations are applied to concrete objects; sizeof operands remain
unevaluated. No selected conversion has a class materialization or lifetime
record in this slice. Missing/mismatched concrete declaration facts are invariant
failures. Unresolved pattern aliases and nested enum identities are deferred.

The eligible leaves are scalar literals and fundamental-type local, parameter
or namespace names, including references to fundamental types. The slice adds unary/binary/assignment and
conditional operators, parentheses, scalar casts, fixed sizeof/alignment and
subscript expressions. Ordinary semantics validates it once, including unused
bodies (README definition-time checks; N3485 [temp.res], [temp.dep.expr]/1–4,
[temp.dep.constexpr]/1–2 and the respective expression operand constraints).
Work is O(pattern nodes plus consumed occurrence edges), without repeated fixed
operator conversion selection. TU-owned flat indices and existing fact/slice
arenas have no per-node heap owner or process-global cache.

This completes the scalar fact group, not the full body graph requirement.
Calls/class operations, declaration/return/default conversions and dependent
subgraphs still need richer typed edges. Full region occurrence projection and
contextual expression records also remain; finer demand/failure states are a
separate incomplete owner. Measured performance is recorded in performance.md.

Fixed-fact dispatch lives in `resolve_expression`, which constructs its existing
result directly. The ordinary `expression` wrapper retains its original result
construction path; the initial extra temporary was removed after common-workload
measurements. The wrapper's release text returns from 762 to 623 bytes (its entry
size); the resolver's hot text also decreases by 22 bytes. This is an implementation
work reduction, not a standalone runtime-profit claim. Both preceding binary
campaigns remain frozen alongside the final measurements.


## Shared fixed call facts

`call_selection.cpp` is the typed direct-call candidate owner used by ordinary
calls, type queries and fixed template calls. Arity/category filters precede
argument conversions; candidate ranking returns a compact selected/ambiguous/
no-viable result. Query and source callers preserve their respective typed/null
literal inputs. The borrowed expression vector survives recursive arena growth.
This does not yet remove exceptions from every dependent candidate operation.

`template_call_facts.cpp` records fixed callee identity, argument types/categories,
ADL contribution and immutable conversion recipes on the source expression.
It validates unused bodies, deleted/access constraints, class results and
conversion-result transfers without demanding function bodies or constructing
source-owned temporaries. Function parameters share ordinary array/function
adjustment through `parameter_body_type`. Pattern fixed-base access has an
explicit indexed edge with its access level, independent of concrete layout;
local pattern classes need no invented type or layout to validate access.

Each occurrence consumes its mapped operands and emission demand. Scalar
conversions share arena slices; object/user conversions produce occurrence-owned
materializations and lifetimes from unprepared recipes. Declaration-owned
default arguments retain their owner and are evaluated at each required use.
Fixed function pointers/references, reference returns, fixed explicit function
specializations, hidden-friend ADL, defaults and temporary chains have native
controls. Seventeen new unused-definition rejections supplement the course suite.

Selection visits language-required candidates/arguments once per fixed source
call; reuse visits each concrete argument once. Source/occurrence call slices
and access edges are TU-owned, with no global scan or process cache. Scalar
conversions add no per-specialization copy. Object conversions retain new
materialization records. Record layouts stay 112/36 bytes. Nonstatic implicit
objects and pattern-owned member signatures, constructor/operator expressions,
declaration/return conversions and dependent projection remain separate current
owners. These are not claimed complete by fixed direct/indirect call sharing.


The follow-on conversion owner records constructor argument recipes without a
concrete temporary. `materialize_conversion` distinguishes that source recipe
from a completed object by its zero temporary identity, copies each argument's
consumption facts and applies the recorded conversions. User-conversion result
transfers consume the same recipe mechanism, cloning unprepared user records
before use. Standard and nested constructor conversions validate base access
at definition time, including conversion-function pointer results. Default
constructor arguments permit their own user conversion; the restriction on a
second user conversion belongs only to the supplied first argument.

Additional native controls cover side-effecting constructor defaults, class
conversion of defaults, conversion-function reference results with secondary
copy/default objects, repeated specialization use and public base pointers.
Four reduced private-base cases close definition-time errors accepted at entry.
This completes fixed call argument recipes; dependent member/object selection
and declaration/return conversions require facts from their distinct owners.


## Fixed object and member facts

`template_object_facts.cpp` retains explicit member receiver edges on the source
expression. Fixed class/pointer parameter and local identities use the existing
pattern declaration identity and are mapped through the concrete declaration
fact. A source-owned `ObjectUse` contains no temporary. `object_fact` projects
only its receiver/member-pointer edges for the consumer; type, naming scope,
base adjustment and virtual slot stay shared. Class-valued call results obtain
a concrete object-use record when their per-use temporary is established.

`template_call_facts.cpp` now consumes fixed member designators as well as free
and indirect calls. Implicit-object ranking and base access are checked once;
qualified calls suppress virtual dispatch and static calls retain receiver
side effects. Occurrences demand the selected entry and consume their mapped
receiver and argument expressions. Object-preserving related reference casts
without conversion-function participation also share their fixed conversion.
Other class casts remain owned by the pending cast/materialization graph.

Explicit fixed class-template-ids derive dependence from their arguments,
including defaults, rather than inheriting the primary's pattern marker.
Qualified fixed class lookup completes only its required declaration/layout;
selected unused member bodies remain undemanded. Contextually bound nested
source regions still cannot publish definition-wide expression facts.

`member_value.cpp` gives ordinary expressions and type queries one member
value/type/category owner. It rejects nested types used as values and preserves
lvalues for reference and static data members, prvalues for enumerators, and
cv/mutable field rules. N3485 [expr.ref]/4 (`doc/n3485.txt:5834`) requires the
reference/static distinction; `field-category.t` fails at entry and executes
successfully after the change. Nonreference fields of rvalue objects are
xvalues per [CWG 616](https://cplusplus.github.io/CWG/issues/616.html), the C++11
defect resolution adopted in April 2013; queries now agree with ordinary
expressions. No reference output was changed.

Work follows a fixed source member/receiver edge once and each concrete receiver
use once. Sharing named/field receiver recipes adds no per-use object arena
record. `ObjectUse::source_owned` uses existing padding; entity/expression
layouts remain unchanged. Twenty unused-member/type/query/pointer rejections, seventeen native programs,
the through report (1935/1935) and final release/sanitizer parity on 331 inputs
pass. The frozen receiver and follow-up evidence is in [performance.md](performance.md).

This covers receivers whose class type is fixed. A template-owned dependent
class receiver still needs a symbolic member-declaration path and separately
substituted layout/access facts. Constructor/operator expressions, broader
casts, declaration/return conversions, dependent-only projection and finer
demand/failure states remain current-stage work.

Fixed pointer arithmetic now requires a complete pointee at the selected
builtin operation, including type queries; incomplete pointer comparisons
remain valid. This follows N3485 [expr.sub], [expr.post.incr], [expr.pre.incr]
and [expr.add] (`doc/n3485.txt:5677`, `5880`, `6334`, `6782`). The check runs
after candidate selection, so it cannot reject an unselected builtin when a
user operator wins. Six new reduced rejections cover addition, subtraction,
increment, subscript and their query forms. Callable class objects retain the
ordinary operator owner until that owner supplies a reusable call recipe.

## Repeated default materializations

Default expression facts and their selected callees/conversions remain shared.
`lowering/class_values.cpp` establishes a fresh storage identity each time that
materialization is emitted. `TemporaryState` captures its concrete address;
normal and unwind cleanup consume that address rather than the recipe's latest
binding. Class storage consumers reuse the location of the current evaluation.
No syntax clone, lookup, overload selection or template substitution is added.

The lifetime/unwind classifiers follow semantic call-argument edges, including
constructor and user-result defaults outside the caller's syntax. Completed
expression summaries remain cached; work is bounded by syntax and required
semantic argument/conversion edges. Elided constructor calls do not contribute
default-argument effects. Storage and cleanup records follow emitted objects.

`default-object-identity.cpp` fails at entry (native exit 1). Its expanded
control checks ordinary/template calls, dependent defaults, conditional arms,
conversion functions, converting-constructor defaults, trivial class addresses,
destructor identities and small/looped arrays. N3485 [dcl.fct.default]/9 and
[class.temporary]/3-5 (`doc/n3485.txt:10776`, `14038`) require evaluation on each
call, distinct overlapping lifetimes, full-expression destruction and array
element default cleanup. The intermediate conditional reducer also exposed a
missing semantic argument edge in lifetime classification. All seventeen native
programs, through 1935/1935 and release/sanitizer parity on 331 inputs pass.
The final `conversion_call` accessor returns an immutable view; effect consumers
do not copy descriptors. All 6,104 retained observations verify, including
compiler wall/RSS, native runtime/payload, outliers and separate correctness costs.
The attempted try/throw extension is preserved in the artifact directory with
its rejection log; source exception lowering belongs to PA21, not this stage.
Default definition/demand states remain a separate unfinished semantic owner.

## Template-owned fixed field paths

`template_member_facts.cpp` separates a source field's fixed type/category from
its template-owned object. Definition-time method contexts retain the owning
pattern, availability of `this` and cv qualifiers. Ordinary/member-query value
rules now consume cv directly, without constructing a fake source object type.
The ordinary declarator and pattern body share function-qualifier extraction.

Each concrete class definition records its source pattern and declaration
context. A `(source field entity, concrete object TypeId)` cache maps the field
declaration, validates fixed value facts and records the required base adjustment
once; object TypeIds include class identity and cv. Unevaluated static contexts
use the concrete class identity without an object. Later forward declarations
do not replace the definition's context. Repeated field occurrences consume this
record and the shared scalar/call facts above it, preserving concrete member
identity and layout. No source-owned field recipe contains a concrete object.

Known bit-field widths use the ordinary property owner; unknown widths defer
because they can change integral promotion. Eight definition-time rejections,
eighteen native programs and through 1935/1935 pass at the initial increment.
Qualified out-of-line contexts now retain the actual pattern owner, including
nested and repeated injected-class-name qualifiers. Candidate declaration cv/ref
facts and, when necessary, cached fixed parameter-type shapes establish static
status only when every remaining declaration agrees. Mixed dependent signatures
remain deferred. Shapes use canonical adjusted function parameter types; no
name spelling or assumed nonstatic status substitutes for declaration evidence.
Field identity preserved by parentheses, assignment and prefix operators uses
the same class-context cache, even when the body has a separate definition
context. This fixed the course's out-of-line `operator++` control after the
initial extension exposed its previous body-context mapping assumption.

The extended increment passes thirteen rejection controls, eighteen native
programs and through 1935/1935. Work is proportional to source method contexts,
actual overload candidates and distinct field/object keys; no class layout or
unused member body is demanded by context discovery. `--stats` exposes context
and concrete field-use record counts.

Prototype scopes retain sequential parameter declarations when later parameter
types query them (`decltype(a)`, including concrete class members). The source
predicate is cached by parsed parameter-list identity, and the ordinary type
builder and fixed method-shape checker share it. Prototype parameters contribute
ordinal/type query facts; they cannot publish runtime-object recipes that would
require a body declaration before the signature exists. `parameter-shape.t`
reduces an inherited rejection; the native class and ordinary function cases
follow C++11 [basic.scope.pdecl], [basic.scope.proto] and [dcl.type.simple]. No
reference output was changed. Scope classification uses two bits per parsed
source node (unknown/unnecessary/needed). One reusable work vector retains at
most the largest parameter-type syntax traversal. The predicate scans syntax
only, so its scratch cannot be reentered through semantic declaration work.
This bounds required classification storage and avoids repeated per-declaration
traversal allocations; it does not explain the measured ordinary-call RSS delta.

Out-of-line matching selects the function suffix nearest the method name,
including through nested pointer-return declarators. Returned function parameter
lists do not determine member static status or cv. `method-parameters.t` checks
both static and nonstatic overloads returning function pointers; the fourteenth
rejection control checks the converse invalid static field use. This follows
C++11 [dcl.fct] and [dcl.ptr], using the same structural selection rule as
ordinary function-body parameter binding.

The expanded reducer also preserves distinct nested enum types across class
specializations and defers unresolved enum signatures during static/nonstatic
matching. Final validation passes all 314 PA14 cases, 1,621 earlier cases,
1,935 through cases, eighteen native programs, 332 release/ASan/UBSan parity
inputs, 82 rejection controls, two ABI controls and all four standalone reducers.
The final four reducers have frozen current inputs and release/sanitizer/native
output hashes. Eight new campaigns add 1,162 verified observations; work and
storage equations, compiler/runtime costs, retained outliers and open RSS
investigation are recorded in [performance.md](performance.md).

This completes fixed-value field facts for template-owned object paths, including
separate class/body declaration contexts. Dependent value/type/storage forms
must enter a context-keyed typed body graph rather than this source-only fact
index. Whole-region projection and finer demand/failure states remain explicit
current-stage work in [plan.md](plan.md).

## Retained declaration types and substitution frames

Function-template parameter declarations retain the signature's source TypeId
when definition-time body binding creates lexical parameter identities. Body
instantiation substitutes these typed facts directly; it no longer resolves the
function's declarator or return type from syntax a second time. Source parameter
cv, array and function forms remain separate from the adjusted callable type.
The body consumes concrete types and creates only its required runtime parameter
identities. Nested pointer-return declarators use the suffix nearest the name.

Canonical substitution frames identify a specialization, its source declaration
head and an optional enclosing frame. Arguments remain in immutable interned
packs; source parameter identity and ordinal must both match. Renamed member
heads overlay the defining class head. Signature and body substitution share
successful type/query facts keyed by complete frame and source identity.

Source specifiers, aliases and supported declarators publish canonical types
before introducing the new declaration's name. Concrete declarations reuse fixed
types or substitute dependent types through those frames. A deferred fact stays
explicitly empty for local class/enum identity, value-dependent bounds and query
syntax owned by other semantic paths. The source probe never constructs a type
from zero and never catches a semantic error to retry with guessed information.
`using typename` now retains its type category through parsing and source binding.

Query frames also map definition access scopes and source overload unions to
concrete declarations. Parameter queries retain type-only ordinals; signature
queries need no runtime parameter object. Member queries retain implicit-object
cv, while ordinary member-value rules handle mutable/reference fields. Named
member calls use the concrete implicit object for overload selection. These are
C++11 [basic.scope.pdecl], [basic.scope.proto], [dcl.fct], [dcl.type.simple],
[expr.ref], [class.access] and [temp.mem] requirements, exercised by
`signature-facts.cpp`, `declaration-types.cpp` and eight rejection controls.

Final validation at `5ce59182` passes PA14 314/314, prior 1621/1621 and the
default through report 1935/1935, twenty native programs, 334 release/ASan/UBSan
parity inputs, ninety rejection controls on both compilers, two ABI controls,
four explicit reducer parity/native checks and file audit. Two frozen campaigns
add 588 verified observations; successful LowIR and executable hashes remain
identical. Record layouts and exact work/storage equations also verify; see
[performance.md](performance.md) for costs, benefits and retained outliers.
Body parameter ordinals remain eligible for fixed-expression checking; only
prototype objects are excluded. A source access context participates in query
identity without itself making fixed operands dependent. This preserves
definition-time rejection of invalid parameter operations and unknown fixed
calls, verified alongside the inherited binding controls.
Whole-region projection, remaining dependent body forms and structured failure
states are still current-stage work; these type caches do not complete them.

## Declaration, body and default-argument regions

Projection retains member body, constructor-initializer and default-argument
roots separately from declaration contents. Each complete source/context key
has at most one occurrence; a deferred root transitions once to demanded and
only then projects its contents. Function body and constructor semantic owners
request their own regions and grow context facts before consuming them. Local
classes preserve the same nested boundary rather than eagerly projecting their
unused methods.

Concrete member defaults retain their declaring scope and are analyzed only
when an argument is omitted. A single default accessor serves ordinary calls,
constructor/conversion paths and retained call recipes. Default state detects
recursive or failed demands. Template definition-time binding still checks
fixed names inside defaults in a sequential parameter scope. This follows
N3485 [temp.inst] and [dcl.fct.default], independent of diagnostic wording.

The source wrapper also owns each function-template default region, whether
its default is used before or after the body. Successful defaults retain their
expression identity for constant-time reuse; expected failure/active states are
separate from projection. A default's declaring template head owns lookup.
Complete specialization/head keys cache immutable overlays containing only that
head's parameters. A default used before a later definition does not establish
the future body's environment. Qualified namespace lookup uses the declaration
scope while retaining the source head's type parameters.

Current checks pass PA14 314/314, a default through report 1935/1935,
twenty-one native controls and twelve new rejections. `demand-regions.cpp` is
entry-rejected and current-accepted, covering unused dependent defaults,
explicit constructor/member arguments, repeated default side effects,
local/nested classes, converting constructors, renamed heads across declarations and definitions, defaults used before definitions and qualified namespace definitions.
The source cache, frozen performance evidence and sanitizer checks are complete;
see the final validation below.

The standards review moved incorrectly permissive personal controls for defaults
added on later template declarations into the rejection suite; their original
source is preserved in the region artifacts. N3485 [dcl.fct.default]/4 and /6
restrict these additions, as confirmed by [CWG 15](https://cplusplus.github.io/CWG/issues/15.html)
and [CWG 217](https://cplusplus.github.io/CWG/issues/217.html). Initial defaults
remain valid across renamed definitions and early calls. Out-of-class member
binding receives an explicit restriction from its declaration owner; function
templates compare the current declaration with the canonical initial source.
No course fixture, reference, or comparison rule changed. Current checks pass
314 PA14 cases, 21 native programs, twelve rejection controls and the focused
`default-heads.t` compiler/native reducer. All campaigns remain preserved.

The AST owns a translation-unit source-region index keyed by parsed root identity.
Each demanded source region is walked once to retain flat node-ID, deferred-root
and attribute-reference slices. Projection consumes those slices for each complete
source/context key. Deferred roots retain the original parsed ID so later demand
uses source attributes even when invoked with an occurrence. All concrete nodes
exist before alignment operands are projected. Storage follows indexed source
regions plus demanded occurrences; there is no per-specialization tree walk or
per-node attribute lookup. This is a region boundary, not yet dependent-only
projection within a used body.

Final validation of the region/default group (`e1afac7c`) passes PA14 314/314,
prior 1621/1621, default through 1935/1935, 21 native programs, 335
release/ASan/UBSan parity inputs, 102 rejection controls on both builds, two ABI
controls, six explicit reducer parity/native checks and file audit.
`region-attributes.t` confirms concrete alignment operands and packing inside
demanded local-class bodies against entry, final release and sanitizer output.
All 8,932 performance observations verify; the corrected fully-used-body slowdown,
isolated cache benefit and remaining RSS costs are retained in performance.md.
Dependent-only projection inside used regions and typed demand/failure dependency
graphs remain separate current-stage representation work.

## Typed body value dependencies (active continuation)

A source layout query now retains its canonical type-query identity separately
from its fixed size_t result type. Body binding can therefore establish scalar
operator conversions and call selection even when a value depends on T. The
source graph does not invent a constant for that value. Each concrete frame/query
substitutes only dependent facts; a separate monotonic value owner caches the
constant identity. Enclosing fixed expressions reuse their type/conversion facts
without copying a source non-constant marker over a dependent concrete value.
Query checking validates the operand of nested sizeof even in unevaluated uses.
The initial group passes 314 course cases, 22 native programs and eight explicit
rejection controls. Value-dependent array types and full evidence remain active.
