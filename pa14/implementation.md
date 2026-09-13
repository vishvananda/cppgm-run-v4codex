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
| `semantic/type_query.cpp`, `type_query.h` | Canonical query IDs retain bound names, parameter ordinals, literal/null provenance, operations, types and child slices. Each query has Active/Success/Failure state. Substitution visits dependent edges and reuses completed fixed queries. | All five previously failing dependent-signature cases now pass; query-instance scaling and work counters. Later continuations add scalar casts, conditional/logical values and dependent bounds; broader query forms remain bounded by PA14. |
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
| Typed template body facts | Source properties and call-input slices are shared; concrete identity/evaluation state has sparse use ownership. Joint declaration/scope/lifetime ownership must still establish which context facts are newly required and eliminate remaining fixed rechecks. Whole-region occurrence and dense Fact storage remain. |
| Demand and failure facts | Finer declaration/layout/default/exception/body states, typed reasons and reverse dependency edges, narrow structured expected failures. |

Parsed source nodes, fixed name/query/scalar/call properties and declaration type
recipes are shared. Concrete expression bindings, receivers and conversion
applications now have separate use owners. The remaining full-stage audit crosses
`demand_region`, parameter/local declaration facts, block scopes, object/storage
identities and lifetime consumers. An expression property cannot replace those
identities. Source/context occurrences remain compact views of one parsed graph;
zero occurrence count is not a mandated numerical budget. The open work is to
complete the typed demand/failure/dependency ownership and dependent-only semantic
checking, not to meet an invented record-count ceiling. No requirement is deferred
to PA15.

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


Dependent array bounds now retain a canonical query ID with their element type.
Substitution evaluates integral query values once per concrete query, then
publishes an ordinary canonical array type. Qualified static constants/enumerators,
sizeof/alignment, unary/binary operations, arithmetic casts and conditional
selection use the existing semantic/conversion rules. Both arms must be well
formed; only the selected conditional/logical operand is evaluated. Parameter
array adjustment precedes later parameter queries. Source arithmetic obligations
remain checked even when layout makes the value dependent.

The ABI graph consumes those query identities directly, including `st`/`sz`,
`at`/`az`, `qu`, and cast forms from the
[Itanium expression grammar](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-expressions).
No reference output changed. The first typed-value implementation exposed an O0
constant-widening output difference. Conversion sequences now share at most four
source/flag variants; entry/current LowIR and native bytes match for the reduced
left/right layout-offset control and body-values program. The initial differing
outputs remain in the artifacts. Value/query tables are translation-unit owned;
query Active/Success/Failure states and canonical frame/query keys bound repeated
work. This does not replace whole-region occurrence projection or the remaining
local identity/lifetime dependency owners.

Current correctness before measurement: PA14 314/314 and prior 1621/1621;
23 native programs, eight body-value and fourteen bound rejection controls,
four new ABI controls and the explicit conversion reducer. Frozen performance,
sanitizer and final source-layout validation follow this implementation increment.


Final validation of `66fe52c1` also passes through PA14 (1,935 tests), 337
release/sanitizer status-and-LowIR parity inputs, 124 explicit rejection controls
on both compilers, six ABI controls and seven reducer/native parity checks.
The frozen value campaign adds 462 verified observations; all 9,394 historical
and current observations verify. Offset workloads improve in both paired blocks,
with zero common-correct generated-code growth. Performance costs, current/frozen
layouts and the remaining occurrence/identity/lifetime boundary are recorded in
performance.md and the compact plan. No course or reference coverage changed.


Expression ownership continuation from `5a795af4`: immutable properties now
have compact fact identities; concrete value/receiver identities, incoming
conversions and evaluation state have separate sparse use records. Definition
facts are reused by identity and only changed properties publish new records.
Snapshots remain valid across recursive arena growth. Layout-conversion variants
reuse source/sequence keys. This separates declaration and object bindings from
shared type/category/conversion properties without recomputing those decisions.

Fixed call inputs retain their source slices. Typed `CallInputs` distinguishes
concrete slices, retained source slices and contextual expression views. All
semantic, lowering, conversion, unwind and cleanup consumers use the same
`call_argument` accessor. It projects an already-established source/context
identity; declaration-owned defaults retain their own context. Argument
materializations keep concrete slices and distinct objects/lifetimes.

PA14 314/314, prior 1621/1621, through 1935/1935, 24 native programs, and the
explicit storage/snapshot control pass. A complete untimed preflight of all 28
prior workloads plus three call-input scaling cases and one checked native loop
preserves exact LowIR/native outputs. Performance and sanitizer acceptance are
still active; remaining whole-region projection and demand dependencies persist.


Local expression views extend the same immutable ownership boundary. Semantic
fixed-expression reuse and ordinary expression/operator/call lowering retain a
single stack snapshot of the established source/context view during each visit.
They no longer project all child edges again for each kind/operator/first-child
read. Recursive arena growth cannot invalidate the copied view; source topology
and the occurrence context do not change during these visits. This adds no
persistent cache, invalidation, semantic recomputation or output transform.
PA14, prior/through reports, file audit and all 24 native programs pass. The
initial 560-observation ownership campaign and CPU profile remain frozen; the
local-view candidate receives separate sanitizer and AA/ABBA validation.


Final validation of `5ae726e0` passes all required stage/prior/through reports,
file audit and 24 native programs. The 338-input sanitizer parity campaign, 124
rejections, six ABI controls, seven reducers and storage/lifetime controls pass.
The final frozen comparison resolves the layout-offset slowdown: both paired
blocks improve, with identical LowIR/native bytes and lower peak RSS. Both
560-observation campaigns and all earlier measurements verify, totaling 10,514.
The complete cost/spread review and remaining declaration/lifetime/demand boundary
are recorded in performance.md and plan.md. No course/reference coverage changed.


Definition-demand continuation from `167f5f43`: each concrete specialization and
immutable source definition-list head now owns a completed traversal. Repeated
member demand returns that fact directly; a newly published head traverses its
new prefix and reuses an already-completed tail. Per-definition Active/Success/
Failure states remain authoritative, and a re-entrant traversal cannot publish
completion while an application is active. Keys include the concrete owning
specialization and unique source-head identity, so other overload buckets, nested
paths and later definitions do not invalidate or alias completed facts.
Requests/hits/visited edges are observable without altering demand.

The new definition-demands.cpp control covers repeated calls, late overloads,
a previously absent member definition, nested paths, independent static storage
and unused invalid dependent bodies. Entry/current validated LowIR and native
bytes agree; 37 current requests visit ten edges for the same ten definition
applications. PA14 314/314, prior 1621/1621 and all 25 native programs pass.
The initial personal pointer comparison was ill-formed between int* and long*;
explicit void-pointer conversion preserves the intended distinct-storage test.
Its original source and failure log remain in the artifacts. Full source/key
scaling, sanitizer and performance acceptance follow this coherent increment.


The application and completed-tail facts now share one compact index entry.
Typed NotStarted/Active/Applied/Failed/CompleteTail states preserve their separate
meaning: CompleteTail proves this head applied and every reachable older head
completed. Re-entrant Active applications do not publish that derived fact.
This removes the preliminary second index and its duplicated keys; only three
telemetry counters extend the Analyzer owner. PA14, prior13 and all 25 native
programs still pass after the representation change.


Ordinary out-of-class member definitions now match retained typed prototypes at
definition time. Head parameters normalize by ordinal; current-instantiation
aliases expand through their declaring class and immutable argument bindings.
Aliases of unrelated dependent specializations remain symbolic. Parameter shape,
return type, cv/ref qualifiers, variadic form and known exception specifications
must agree. Special-member and unavailable source types retain their existing
checking owner pending the broader typed prototype extension.

N3485 [class.mem]/1 and [class.mfct]/2 prohibit adding an undeclared member in
an out-of-class definition; [except.spec]/3 requires compatible exception
specifications. Nine focused unused-definition controls now reject. A valid
current-instantiation alias chain initially exposed incomplete normalization; its
313/314 log remains, and the alias normalization restores 314/314. Prior13, all
26 native programs and file audit pass. definition-signatures.cpp covers renamed
heads, nested aliases, array adjustment, function pointers, ref qualifiers and
trailing decltype returns. No course fixture or reference changed.


Matched prototypes now own their definition lists. Concrete member facts retain
the source prototype ID when the class declaration is established; later demand
visits that prototype's definitions plus any still-unmatched special-member
records in source order. An indexed canonical signature table is built once per
complete source prototype bucket. Incomplete source types cannot publish a
negative signature result. Friend declarations are excluded from member
prototypes, and repeated ordinary definitions are rejected at definition time.

Source application state remains keyed by specialization/definition. Traversal
results now have the distinct member/source-head owner, including a valid absent
result. Source checking may re-enter an application before prototype selection
is published; the checked flag prevents caching a final selection at that point.
This requires separate application and demand indexes, replacing the preliminary
combined state optimization. Later source publication changes only the affected
head identity. The overload control drops six entry applications to two, with
three source signature visits and identical validated LowIR/native output.
PA14, prior13, all 27 native programs and twelve rejection controls pass.


Out-of-class nested aliases retain their declaring template head explicitly.
The binding overlay records that head, so aliases introduced under U and later
used by a member definition under V normalize against their own parameter IDs.
The reduced nested-head program was accepted by entry, rejected by the initial
selected-definition build, and now passes; both original outputs remain in the
artifact directory. All 314 stage tests, 1621 prior tests, 27 native programs
and twelve rejection controls pass after the correction and unresolved-type
guards. No semantic decision uses parameter spelling as identity.


Matched ordinary definitions apply directly to the retained concrete member.
Function-template and ordinary member bodies share parameter-fact substitution:
raw source cv, array and function forms survive independently of the callable
signature. Declaration attributes use a shared application helper; defaults,
exception checks and virtual specifiers retain their existing owners. Body
registration stays deferred. This removes repeated name/declarator/overload
reconstruction on the matched path. Unresolved and special-member definitions
continue through their established declaration path. A direct-application
counter makes the two paths observable without adding analysis work.

The new definition-parameters.cpp executes array/function adjustment, body const,
nested function-pointer returns, renamed heads, dependent noexcept and inline
attributes across int/long specializations. Stage 314/314, prior 1621/1621,
all 28 native programs and twelve rejection controls pass. Freeze the current
implementation for full parity, transitive layout and A/A+ABBA evidence next.


The complete definition-owner campaign retains 644 new observations alongside
10,514 earlier observations. Four independent N/K/Q cases improve 12–56% in both
paired blocks with lower peak RSS; all 37 LowIR/nine native hashes match. Compiler
text grows 7,552 bytes and Analyzer 240 bytes. New source/member record sizes,
every inherited latency/RSS increase and all outliers remain in performance.md.
The call-input-128 native RSS increase is retained: separate Massif traces show
only a 24-byte peak-live difference with identical dominant allocations, supporting
an allocator/page-retention explanation without proving the exact native cause.

All 342 release/sanitizer parity inputs, 136 rejection controls, six ABI controls,
seven native reducers, 28 native programs and store/lifetime controls pass. Stage,
prior, through and file audit pass. Source signatures for special members and the
joint declaration/lifetime/demand graph remain current-stage work; missing source
types prevent safely extending direct ordinary application by a local shortcut.


Special-member source checking now retains void constructor/destructor signatures
and declared conversion targets through the ordinary typed declarator machinery.
The canonical prototype matcher replaces the old nullary syntax/exception scan,
so different conversion targets may have different exception specifications.
Special definitions also participate in duplicate/in-class-definition checking.
Twelve focused rejection controls and a native conversion/constructor/destructor
control pass, alongside stage 314/314, prior 1621/1621 and all 29 native programs.
A missing helper declaration caused the first build to fail; that log remains.
The next owner change supplies injected-class and declaring-head parameter facts
before extending direct special-member body application.


Injected primary/member-class names now carry symbolic current-instantiation
types during source checking. Nonterminal injected qualifiers still consult their
bound source scope, preserving fixed alias facts. Definition overlays retain
parameter slices rather than just head scopes. Canonical matching sees the actual
source class/head chain; concrete application builds the corresponding immutable
parent frames, including an out-of-class nested definition's differently named
head. Qualified injected-class names normalize to their owning current type.

Matched constructors, destructors, conversions and defaulted definitions now
apply to the selected member identity. Raw parameter substitution, default and
exception owners, transfer classification, virtual facts and body registration
are shared with existing semantics. User-written bodies remain deferred; defaulted
members keep late-definition demand and base/complete-entry behavior. No syntax
or overload reconstruction is needed on the matched path. The same prototype
owner rejects duplicate defaulted definitions, nondefinition redeclarations and
late deleted definitions (N3485 [class.mfct]/2, [basic.def.odr]/1,
[dcl.fct.def.delete]/4).

The injected-signatures.cpp control executes renamed copy/move construction and
assignment, nested raw aliases spanning U/V heads, returned references and late
defaulted operations. All 314 stage tests, 1621 prior tests, 30 native programs
and 21 special-signature rejection controls pass. Freeze full sanitizer, layout
and source/key/request performance evidence before accepting the owner costs.


The completed signature group has 21 reduced rejection controls: entry accepted
20 invalid definitions, while its existing conversion-noexcept rejection remains
covered. The proofs cite N3485 [class.mem]/1, [class.mfct]/2, [class.conv.fct]/1,
[except.spec]/3–4, [basic.def.odr]/1 and [dcl.fct.def.delete]/4. Injected-name
controls also cover [temp.local]/1–2. Both new valid programs fail at entry and
execute successfully with the corrected head-frame and direct-application builds.
Host results support the source-rule proofs; no reference output changed.

Performance comparisons validate both compiler outputs using the unchanged
course LowIR rules. Five new constructor cases differ only in local slot
suffixes, which PA8's positional canonicalization absorbs. A personal adapter
loads the course validator/canonicalizer with student presentation mode for both
inputs and requires identical canonical output. It does not invoke the broader
generated-projection fallback. The initial raw-byte requirement and subsequent
reference-only ordering requirement were inappropriate for two student outputs;
both failed preflights and their harness versions remain frozen. Thirty-nine
cases have exact LowIR hashes, and all ten native executable hashes match.

Nested-head timing uses the frozen correct head-frame build as its A baseline:
entry rejects the valid nested alias program. This separates the measured direct
application change from the necessary semantic correction. The other 42 cases
compare continuation entry with final. The new live layout probe retains 17
transitive headers; earlier probes remain immutable historical snapshots.

The complete validation records 344 release/ASan/UBSan parity sources, 157
rejection controls, six ABI controls, seven native reducers, thirty native
programs and expression-store/lifetime controls. The stage, prior and through
reports pass 314, 1621 and 1935 tests respectively, and file audit passes with
three inherited header advisories. The remaining architecture group requires
joint local declaration/type-query/scope/object/lifetime identities and their
consumers; class-scope special/injected signatures are now established. Typed
reverse demand edges, narrow failure records and all independent fact states
also remain open. No external blocker prevents that subsequent work.


The isolated special-member campaign is accepted with all 756 observations
retained (11,914 cumulative). Constructor scaling improves 7.06–57.12% in median
latency and reduces peak RSS; both ABBA blocks improve for each affected case.
Nested-head direct application also improves against its correct intermediate
baseline. Compiler text grows 960 bytes, measured layouts stay unchanged and all
ten generated executables remain byte identical. The full report discloses
inherited latency increases, the 3,622 KiB body-large-8 RSS increase, calibration
spread and historical costs. The cumulative verifier passes all new and inherited
measurements, source proofs, current layouts and required-check evidence. The
class-scope signature/application group is complete; the remaining joint
local-declaration/lifetime/demand graph is the explicit handoff boundary.


Continuation from fda0a178 publishes concrete template declarations through the
source-declaration/substitution-frame owner. Source binding records the canonical
pattern entity; concrete declaration, class and enum producers publish its
selected identity before consumers run. Type-query binding and fixed-expression
uses consume that overlay directly, including parent frames. They no longer
recover entity decisions from projected syntax and dense Fact slots. Local class
and enum patterns retain symbolic Named identities; substitution resolves them
through the same published declaration owner before layout, aliases or objects
are consumed. Concrete destruction remains attached to the resulting object.

The local-declaration-facts.cpp control covers copied local objects, raw aliases,
decltype/reference aliases, enum-driven bounds, nested local types, lexical
shadowing, specialization identity and cleanup. It exposed declaration prediction
misclassifying object(T(5)) as a function declaration. The bounded lookahead now
rejects expression-only tokens at that parameter-prefix position, consistent with
N3485 [dcl.ambig.res]/1; it builds no speculative grammar tree. An initial control
missed typename and used function-local static storage outside PA10's inherited
boundary. That source and all failures remain in the artifact directory. The
corrected control uses supported class-template static data members and executes
successfully under the host and this compiler. Stage 314, prior 1621 and all 31
personal native controls pass. Next reduce dense occurrence Fact storage while
preserving these explicit publication and lifetime consumers; full evidence and
performance acceptance follow the complete owner group.


Fact storage now materializes records only on explicit publication. All 137
existing mutation sites use edit(); ordinary reads of absent syntax/occurrence
facts do not allocate. A four-byte optional index replaces the dense twenty-byte
Fact slot, and TU-owned 1024-record slabs preserve references across recursive
publication and index growth. The lifetime/object/declaration consumers retain
the same identities. Record count, retained storage bytes, source declaration
work and unique concrete publications are exposed through existing telemetry.

The standalone fact-store.cc control checks absent reads, independent objects
and queries, and references held across index/slab growth under release and
ASan/UBSan. The complete through report passes 1935 tests and all 31 personal
native programs pass. Three diagnostic inputs materialize 558924/1523505,
1256342/2455650 and 663670/1727733 possible Fact records respectively. These
counts establish sparse ownership, not a compiler latency or peak-RSS benefit;
freeze equivalent A/B outputs and measure those costs before acceptance.


The first frozen sparse-fact trial retains 42 observations on three unchanged,
byte-identical LowIR inputs. Median peak RSS falls 35392/15078/13086 KiB; compiler
latency changes +1.43%/+2.98%/-3.18%. These are diagnostics, not full-corpus
acceptance. The next increment groups adjacent field publications through one
stable writable Fact view, avoiding repeated sparse lookups. Review corrected
one mechanical grouping that moved an unconditional entity assignment under a
value condition; the initial patch/logs remain. The reviewed code again passes
all 1935 course tests and 31 native controls. Repeat the same frozen trial before
full sanitizer and corpus/performance acceptance.


The final source review confirms that all grouped Fact writes preserve their
original conditional scope and use stable slab references. The concrete local
binding owner is published by declaration/class/enum producers before type-query
or fixed-expression consumers run; immutable parent frames resolve enclosing
bindings. Fact indices still follow whole-region occurrence identities, but
absent reads no longer manufacture semantic records.

The next signature change has a concrete joint boundary. `bind_template_type`
retains checked specifier and non-function declarator types, while ordinary
function declarations still run `declarator`. That path establishes prototype
parameter scopes, signature-parameter identities, raw parameter types and
trailing-return query contexts before `declare_object` establishes concrete
identity, defaults, exception, transfer and virtual facts. The existing
`instantiate_parameters` helper supplies raw body types for selected functions
and out-of-class definitions; it does not by itself replace those declaration
scope publications. Reusing only a Function TypeId would bypass required facts.
Some class-scope enum/embedded type queries also lack source identities. Their
producers and consumers must change together, with independent parameter/context
scaling and demand/failure states; this is the remaining architecture boundary,
not a failed course test or an external blocker.


The declaration/fact campaign completes with 924 new unfiltered observations:
84 from the two preliminary trials and 840 from 49 compiler inputs/eleven native
programs. All four local scaling cases improve median latency in both blocks;
three save peak RSS, while the 4000-specialization case adds 16,308 KiB. Compiler
text adds 6080 bytes and Analyzer adds 80; generated output growth is zero, with
all 49 LowIR and eleven native hashes exactly equal. The full report retains
paired spread, timing outliers, both trials, inherited regressions and the
unisolated native allocation variations. Explicit source/key/use and O0 budgets
remain; no unsupported numerical threshold overrides stage-scoped acceptance.

The cumulative verifier passes all 12,838 observations, source/output proofs,
current transitive layouts, work/storage equations and validation manifests.
Required stage/prior/through reports pass 314/1621/1935 tests, all 31 native
programs and 345 release/ASan/UBSan parity sources pass, and file audit passes
with three inherited advisories. The complete local binding, initializer,
sparse-store and grouped-publication behavior group is committed with its
acceptance evidence; remaining architecture ownership is described above.


Continuation from 97006205 reuses checked ordinary and special member function
signatures together with raw parameter publications. Source signature records
retain the declaration NodeId separately from its packed source index. Existing
canonical type/query substitution resolves parameter ordinals and access contexts;
prototype lookup scopes and parameter entities need not be reconstructed per
specialization. Raw cv/array/function forms are published once per concrete
parameter occurrence for the body. Concrete declaration creation continues through
the existing default, exception, transfer, virtual and lifetime owners.

The first native run exposed template_method_shape binding prototype names by
republishing source declarations. That erased raw parameter types and could
replace body parameter identity. Prototype bindings now use their own local
entities without republishing source declarations; pattern identity publication
also preserves an already-established raw source type. The original patch,
compiler and diagnostic traces are retained. The source-index/declaration-ID
separation independently preserves streaming/interleaved instantiation identity.

The new combined native control exposed an entry failure for a member default
that calls a later-declared private static member. N3485 [basic.scope.class]/1 and
[class.mem]/2 make defaults complete-class contexts, including defaults in nested
classes. Source defaults now queue once per declaration and bind after the
outermost enclosing source class declarations are complete. A detached local
batch permits reentrant completion of another class without retrying unrelated
consumers. Not-started, queued, active, complete and failed states belong to the
source default declaration. Binding fixed names remains definition-time work;
dependent default values/bodies remain demand-driven. The batch releases its
storage at source completion.

Stage/prior through validation passes all 1935 tests; all 32 personal native
programs, six new rejection controls, three signature reducers and inherited
object reducers pass. Controls include const/array/function parameter bodies,
trailing-return and parameter queries, nested declarators, static/nonstatic
prototype identities, local copies/cleanup, late constructor and nested defaults,
and unused ill-formed dependent defaults supplied with explicit arguments.
File audit passes with three inherited advisories. Signature, raw-parameter and
source-default work counters are available; isolated scaling, sanitizer parity,
C++11 proof manifests and full performance acceptance remain to be completed.


The same complete-class event now owns non-static data-member initializer
bindings. The reduced signature-late-initializer.t is rejected by entry and the
signature/default intermediate even though N3485 [basic.scope.class]/1 and
[class.mem]/2 make it valid. Typed DefaultArgument and MemberInitializer uses
collect only affected source consumers; each initializer has its own
queued/active/complete/failed record keyed by pattern entity. Static initializer
lookup keeps its declaration-point rules. The expanded native control checks
nested late lookup, member initialization order and an overridden ill-formed
dependent initializer; nine rejection controls and four reduced native programs
pass. The unchanged 1935-test through report and all 32 personal native programs
also pass. This extension preserves the prior failed probes and adds initializer
binding work/queue counters before the full evidence campaign.


Class-scope enum declarations now retain the same symbolic Named identity as
local/anonymous enums. Canonical source signature matching expands qualified
enum members of the current instantiation to that declaration identity, including
renamed and nested out-of-class definition heads. Concrete enum producers already
publish the binding before member signatures and queries consume it; substitution
uses that binding directly. A correct intermediate using symbolic qualified enum
types and its native proof remain frozen, while the final source identity avoids
reconstructing a qualified member lookup during each substitution.

The new enum-signatures.cpp control covers scoped/unscoped and anonymous enum
aliases, underlying widths, dependent enumerator values, nested definitions,
renamed heads and distinct overload identities across class specializations.
Entry and the host accept it; both the qualified intermediate and final compiler
execute it successfully. The through report remains 1935/1935, all 33 personal
native programs pass, and all nine new signature/class-use rejections plus the
21 inherited special-signature rejections pass. Full sanitizer/proof/performance
acceptance follows this completed source declaration/signature/class-use group.


The first proof harness required the host to diagnose an unused private-call
default, but GCC accepted that unused definition. N3485 [temp.decls]/2 makes
each default a separate definition; [temp.inst]/1 defers defaults, /12–13
checks a demanded default, and [temp.res]/8 permits early diagnostics when no
valid uninstantiated specialization exists. The unused input remains an optional
early-diagnostic observation, and a separate call using that default requires
rejection. Both harness versions, the host's acceptance, the partial proof and
the initial 118 passing validation checks are preserved. The corrected catalog
keeps nine required rejections plus the optional input; no course check changed.
Six native proofs and the corrected full 118-check release/sanitizer validation
pass. The current 18-header probe measures Analyzer at 6152 bytes (was 6000),
the typed class use at 20 bytes, and unchanged public hot-record sizes. Older
probes now validate frozen snapshot integrity; the current probe checks live
headers. Historical live-header equality was a diagnostic gate, not a mandated
PA14 limit. All historical sizes and measurements remain intact.


Performance acceptance freezes 54 compiler inputs and twelve executables with
924 observations, plus fourteen observations repeating an unchanged wide local
case after an unisolated timing transition. All 54 LowIR and twelve native hashes
match exactly. The four new N/K/Q signature cases improve median compiler latency
5.87/5.72/4.51/0.68%, in both ABBA blocks each, and reduce peak RSS by
10,632/63,294/29,430/11,818 KiB. Source signatures are checked K+1 times, concrete
applications N(K+1), raw parameter publications 6NK, and each source default and
initializer binds K times, independent of repeated calls. Keyed substitutions
and canonical signature construction increase as recorded in performance.md;
6K(N−1) entities and K(N+1) scopes are avoided on these inputs.

The wide local case initially showed +13.18% median latency as both binaries
shifted from roughly eleven-second to 25–31-second samples. The unchanged repeat
returns to eleven seconds and B improves 0.84% in both blocks. Neither the initial
result nor its unisolated cause is hidden. The inherited repeated-special case
has +17,564 KiB peak RSS despite 49,000 fewer Facts and 983,040 fewer bytes of Fact
storage; its exact native allocation cause remains unisolated. It improves
latency 2.03% in both blocks. Other inherited costs and outliers remain disclosed.
The compiler grows 3,136 text bytes (0.2380%); generated growth is zero and no
runtime optimization benefit is claimed. This satisfies the stage-scoped
source/key/use work and storage budgets without inventing a numerical O0 gate.

The completed behavior group includes the related raw signature, complete-class
default, non-static initializer and member enum changes. Further work crosses
concrete definition/layout/default/exception/body/vtable/emission owners and
structured failure/reverse dependencies; their independent recursion and late
source-definition semantics cannot be replaced by the new source-completion
batch. Remaining embedded type/query producers also need an ownership audit
before eliminating their declarator work. No external blocker exists.

The cumulative verifier passes all **13,776** observations and the proof, layout,
coverage and validation manifests. Required checks and the frozen release
identity remain recorded in signature-publication-handoff.json. This completes
the behavior group and its performance acceptance, with the remaining concrete
demand/failure and embedded type/query boundary retained in the compact plan.

## Terminal concrete demand failures

Specialization declaration/body, class layout, selected member-definition and
translation-unit completion producers now publish terminal failure on an aborted
attempt. Retried failures identify a typed fact kind, entity and source without
allocating diagnostic strings. The original request retains its original error.
Function and class completion restore their temporary semantic context; a failed
nested definition clears its complete flag even when validation fails after that
flag was published. Class completion checks failure independently of the complete
flag. These changes add one analyzer state byte (eight bytes after padding),
with no growth in existing entity, class, member or specialization records.

The public API controls repeat nine failed demands 10,000 times each, requiring
terminal rejection and stable graph sizes, then query an unrelated valid layout.
Entry observations preserve two completion loops that reject only once and then
return success, class errors that turn into spurious incomplete/recursive errors,
and the stale complete flag under ASan. Native controls preserve recursion,
forward uses, nested definitions and unused dependent bodies/defaults. The course
fixtures, references and comparison rules are unchanged.

This finishes the audited failure intervals, including member-definition
environment construction. It does not provide a shared dependency scheduler.
Defaults still have occurrence keys; selected definitions have source-head keys;
destruction/exception facts and vtable/emission facts have separate producers.
Their ready/absent results need insertion-aware dependency edges before they can
share scheduling. In particular, `actions_ready` is shared storage for distinct
constructor/destructor paths, while destructor exception caches also encode a
boolean result. Replacing those flags with a blanket state table would conflate
independent facts. That producer/consumer audit is the next implementation group.

The member-pointer exploration is archived outside the checkout. PA12 explicitly
excludes member pointers, PA13 does not introduce them, and PA14 excludes templates
requiring unsupported earlier class features. It adds no stage requirement or
production change. Performance evidence for the required group is recorded in
[performance.md](performance.md) and in the demand-failure manifests.

The independent full-stage audit begins at `78c2f13e`. Its current architecture,
findings and remaining ownership work are consolidated in [audit.md](audit.md)
and [plan.md](plan.md); the historical increments above remain evidence, not a
substitute for that review.

The explicit-initializer continuation retains source conversions/constructor
recipes and projects list operands into concrete object/lifetime contexts.
[Its evidence](../student.tests/pa14/initializer-performance.md) records the
current checks, costs and newly found common initialization-mode/query handoffs.
The full-stage audit remains open.
