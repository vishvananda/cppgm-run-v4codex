# PA14 implementation checkpoint

The stage remains incomplete: **222/314**, with **138 of the 230 entry failures
resolved and no entry pass lost**. The six personal programs execute successfully.
Neither course fixtures nor references have changed.

## Implemented ownership

| Owner | Data flow and bounds |
| --- | --- |
| `syntax/occurrence.cpp`, `syntax/ast.h` | The streaming parser stores each source region once. Demanded regions receive compact source/context occurrence IDs; a projected view maps child edges without copying source nodes or replaying grammar. Ordinary source reads use an inline O(1) path. Source nodes, occurrence indexes and semantic facts belong to the translation unit. |
| `semantic/template_declaration.cpp`, `template_call.cpp` | Declaration parameter ordinals normalize callable shapes into interned TypeId packs. Canonical template entity plus interned argument pack indexes specialization declarations. Partial explicit arguments, direct/target deduction and overloaded argument contexts feed the existing conversion/ranking machinery. Work follows visited candidate signatures; trial deductions contain only parameter bindings. |
| `semantic/template_instantiation.cpp` | Selected calls and addresses enqueue a canonical specialization once. Its retained declarator/body is checked in a compact parameter overlay; defaults are demanded separately. Recursive demand observes the existing specialization. Local classes acquire specialization-specific entity identities. |
| `semantic/template_class.cpp` | Class specializations have canonical identities before layout. Completion substitutes the retained class through the ordinary PA11–13 class path; member bodies retain independent demand. Defaults, nested identities, base validation and ADL use typed class/argument identities. Constructor conversion, value transfer and associated-class lookup request completion at their owning operation. |
| `semantic/overload.cpp`, `conversion.cpp`, `syntax/declaration.cpp` | Explicit template arguments filter the ordinary/ADL candidate union; selected conversions own function address demand. A same-scope ordinary function preserves a template overload's parser name category, while local values hide it. |
| `lowering/*` ABI integration | Typed specialization arguments and declaration ordinals construct ABI identities. Ordinary instantiated bodies feed typed LowIR; template patterns are not emitted. No reference tool participates in implementation. |

## Remaining semantic groups

These groups share a few failures. Representatives identify the next validation
surface; they are not a narrowed test set. The full 314-case suite remains the gate.

| Owner / remaining group | Required data flow and complexity | Representative validation |
| --- | --- | --- |
| Template definition registry | Index retained out-of-class function, nested-class and static-data definitions by canonical template/member owner. Attach declaration-owned parameter heads and defaults to the definition; completion/storage/body consumers request the narrow fact. Notify only indexed dependents, never scan all specializations after a declaration. | `300-out-of-class-member-owner-param-rename`, `300-nested-out-of-class-member-definition-forms`, `300-later-redeclaration-default-template-argument`, `300-unevaluated-static-member-does-not-demand-definition`, reference-shell and owning-destructor cases. |
| Dependent type/expression graph | Preserve symbolic qualified types, `decltype`, trailing returns and array bounds with their parameter environment. Substitute dependent edges, reuse fixed facts, then publish the concrete signature/layout. Declarator parameter names must be available to dependent trailing returns. Work is proportional to newly demanded dependent facts/edges. | `100-partial-explicit-function-template-id-call` (its unresolved part is the trailing return), `100-function-template-parameter-decltype-ref-array`, `300-dependent-sizeof-type-array-member`, current-specialization aliases. |
| Definition-time lookup/checking | Record nondependent bindings and each base-specifier's dependence before specialization. Check unused bodies, condition/block scopes, template-parameter redeclarations and type/value categories at definition. Use lexical/base/associated indexes with per-owner validity. | Unused-body rejection cases, `100-dependent-direct-base-lookup-provenance`, `100-local-dependent-base-lookup-provenance`, `300-unqualified-call-skips-dependent-base`, typedef redefinition. |
| Parser declaration context | Qualified declarators and template-ids must use their own name-category context. Preserve explicit class instantiation syntax and qualified nested definition owners without grammar replay. Keep checkpoints bounded by the declaration. | `300-qualified-explicit-class-instantiation`, `100-qualified-value-does-not-shadow-class-template`, `300-dependent-functional-template-id-hides-outer-function`, CV/alias declarator cases. |
| Existing value/lifetime and LowIR facts | Carry the selected object/value category, empty-object transfer, local ABI root, and storage/lifetime actions through specialized PA12–13 paths exactly once. No template-specific text repair in lowering. | `100-local-constref-converting-iterator` now produces valid LowIR but differs in empty-object copies/root metadata; reference-member moves, rvalue-reference returns, local enum identity, static object/vpointer initialization and layout cases remain. |

## Spec alignment still open

Sharing the parsed graph is implemented; sharing all nondependent **semantic
facts** is not. Current demanded-region occurrences cover whole function/class
regions, including metadata for unused member bodies. Finer occurrence demand,
dependent-only fact checking, typed demand reasons/edges, distinct declaration/
layout/default/body states and narrow structured failure memoization still need
the symbolic graph and definition registry. These are current-stage requirements,
not deferred-stage exemptions. No parser replay or syntax-tree cloning is an
acceptable shortcut for that work.

The coherent boundary is the completed declaration/completion/call-demand
increment, extended through overload-context handling. The remaining work changes
what a generic declaration *means before substitution* and how later definitions
attach to it. Extending eager concrete lookup or rechecking every projected node
would make that architecture harder to implement and violate the spec. The next
increment should establish the symbolic fact/definition owners together, then
resolve their dependent signature, out-of-class and definition-time validation
families. PA15 has not been started.
