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
