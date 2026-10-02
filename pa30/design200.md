# PA30 implementation200 ownership

Entry: `377d92a00e728199f86381f88f239a88904b829d`; O0 cumulative compiler.
Stage/review markers remain in plan.md. No course fixture, reference or
comparison rule is changed.

| Group / owner | Data flow | Complexity / lifetime | Validation |
|---|---|---|---|
| Selected aggregate destruction: `initializers.cpp`, `value_initialization.cpp` | Selected typed subobject → contextual destructor access/validity and deduplicated member demand → completed action fact → aggregate cleanup lowering. Union destruction alone does not demand its active variant's destructor. | O(selected initialization nodes + demanded destructor edges), existing TU fact/state tables and queues. Inactive union members are not demanded. | Six formerly failing hosted callable/shared-pointer fixtures; selected/inactive union members, inaccessible destructors, runtime aggregate cleanup and validated LowIR controls. |
| Enclosing complete-class contexts: `template_binding_declarations.cpp`, `class_enum.cpp` | Parsed pattern body → enclosing source-pattern/ordinary-class completion interval → bind once after declarations complete → dependent substitutions later. Draining distinguishes pattern members from concrete bodies. | One queued body per source, no parse replay, global retry or broader instantiation. Existing body interval owns storage and releases it at completion. | Nested templates in an ordinary class and in a class template, deleted fixed enclosing constructor rejection, hosted nested callable pack. |

Focused hosted result after these repairs: 6/6. Earlier PAs: 4941/4941.
Current-stage full report, final performance protocol and handoff ledger follow
further related investigation. Independent review remains outstanding; this
file does not certify the whole stage.

The constructor investigation extends the same template-fact group:
`builtin_template_types.cpp` and `type_query.cpp` now distinguish member
provenance from unresolved source-member identity. A concrete alias template
keeps `template_member` for definition ownership; only the source pattern still
requires substitution. Integer-sequence generation consumes that identity plus
canonical element/count arguments and its existing 1,048,576-element cap.
No extra lookup, retry or allocation policy is added. Member alias environment,
empty packs, template-template parameters, constructor pack deduction and an
invalid negative count are explicit controls. All four remaining call/constructor
fixtures (map, piecewise pair, bind and regex) now pass their focused checks.

Review refinement: destructor demand attaches to each selected aggregate child,
not the root initialization type. Root storage owns its own destruction rules;
scalar `new T` may construct a type with an inaccessible destructor. Array-new
and aggregate members still require accessible destructors. Controls cover
all three, and partial aggregate construction through an active union variant
executes exactly one cleanup when its second constructor throws.
