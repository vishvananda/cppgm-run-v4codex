# PA14 implementation ownership

Current contract result: **306/314**, 8 failures; prior assignments **1621/1621**.
This continuation resolves **16 of its 33 entry failures**, with no lost passes.
Cumulatively, **213 of the 230 stage-entry failures** are resolved. Fixtures,
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

## Remaining groups and concrete checkpoint boundary

| Owner | Required next work | Remaining cases |
| --- | --- | --- |
| Inherited transfer/triviality/lifetime facts | Preserve reference-member and empty-object actions, implicit moves, constructor entries, virtual destruction and reentrant completion. Lower each selected action once. | Defaulted reference-member move; inherited typedef/friend overload; virtual destructor; reentrant collection layout/override; rvalue-reference move return; deleted-default member sizeof; base initializer/reference chains; inherited conversion; constref iterator; nested out-of-class defaulted copy. |
| Local ABI and O0 expression/control output | Retain local type linkage/constructor-root provenance and the required discarded-value and constant-condition presentation. | Local enum identity; local alias noop; qualified discarded value; postfix-cv local alias. |
| Remaining template/parser forms | Finish the still-failing declaration context without removing any fixture from coverage. | Variable-template-defaulted fixture; all 314 remain in the exit suite. |
| General semantic fact sharing | Extend expression queries to the remaining expression/bound forms and move fixed body type/conversion facts out of concrete occurrence replay. Separate layout/default/exception/body states; add typed demand/reverse edges and narrow structured expected failures. | These are current PA14 spec requirements even where the course suite does not expose them. |

The expression/signature and fixed-binding group was extended through ordinary
user conversions/operators, out-of-class overlays, lexical jump checks, alias
constructors, nested source projection and an enclosing-environment cache-key
reducer. The remaining output failures consume inherited object/ABI/lifetime
facts. The nested-definition fixture, for example, now binds and instantiates
correctly but still emits calls to trivial defaulted copies instead of direct
storage transfers. Further lookup patches cannot repair those decisions.

Parsed-node sharing and fixed name/query sharing are implemented. Sharing all
nondependent semantic body facts is **not** complete: concrete bodies still
project entire regions and recompute many type/conversion facts. General fixed
expression legality, finer occurrence demand and structured expected rejection
need a broader typed body graph. This is the concrete incomplete handoff
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
