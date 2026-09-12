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
