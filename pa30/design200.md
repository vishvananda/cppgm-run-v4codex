# PA30 implementation200 ownership

Entry: `377d92a00e728199f86381f88f239a88904b829d`; O0 cumulative compiler.
Stage/review markers remain in plan.md. No course fixture, reference or
comparison rule is changed.

| Group / owner | Data flow | Complexity / lifetime | Validation |
|---|---|---|---|
| Selected aggregate destruction: `initializers.cpp`, `value_initialization.cpp` | Selected typed subobject → contextual destructor access/validity and deduplicated member demand → completed action fact → aggregate cleanup lowering. Union destruction alone does not demand its active variant's destructor. | O(selected initialization nodes + demanded destructor edges), existing TU fact/state tables and queues. Inactive union members are not demanded. | Five formerly failing hosted callable/shared-pointer fixtures; selected/inactive union members, inaccessible destructors, runtime aggregate cleanup and validated LowIR controls. |
| Enclosing complete-class contexts: `template_binding_declarations.cpp`, `class_enum.cpp` | Parsed pattern body → enclosing source-pattern/ordinary-class completion interval → bind once after declarations complete → dependent substitutions later. Draining distinguishes pattern members from concrete bodies. | One queued body per source, no parse replay, global retry or broader instantiation. Existing body interval owns storage and releases it at completion. | Nested templates in an ordinary class and in a class template, deleted fixed enclosing constructor rejection, hosted nested callable pack. |

Final reports: earlier PAs **4941/4941**, PA30 **148/153**, through30
**5089/5094**. File audit passes with four inherited warnings. The fixed
153-case inventory has ten fewer failures and no regression. Controls and
performance evidence are bound in `student.tests/pa30/evidence200/`; independent
review remains outstanding and this file does not certify the whole stage.

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


## Source-to-object trace and final boundary

`member-sequence-deduction.cpp` traces a demanded constructor through canonical
member alias identity, generated integral argument packs, selected constructor
facts and conversions, typed LowIR calls and direct ELF encoding.
`enclosing-completion.cpp` traces a member pattern bound after the enclosing
class finishes, then checked under its specialization frame. The source region
is parsed once; no concrete body is checked while draining a source pattern.
`aggregate-union-unwind.cpp` follows selected subobject destructor demand into
completed actions, lifetime/cleanup records, LowIR unwind edges and ELF frames.
The original unavailable fact was `_Nocopy_types`' destructor action list,
queried by aggregate lowering after a union's own empty destruction plan had
left its selected nested variant unscheduled. The repair belongs to the
initializer's actual subobject edges, not lowering-time semantic recovery.

The [trace](../student.tests/pa30/evidence200/trace.json) retains eight validated
LowIR views, object symbol tables, disassembly and unwind frames. All eight
pass external LowIR read/write roundtrip and linked execution, with identical
objects with/without telemetry. There are 50 current controls, 195 inherited
control commands and 81 trace commands; all have their expected outcomes.
Negative controls enforce destructor access for aggregate members/array-new,
deleted enclosing constructors, invalid sequence counts and the existing
generator budget. Positive controls include scalar-new with an inaccessible
destructor, inactive union alternatives, nested class templates, captured outer
alias environments, empty packs and separate constructor parameter packs.

[Performance200](performance200.md) preserves frozen A/B common comparisons,
final-only corrected-owner scaling and all ten repaired hosted compile costs.
These changes add no optional transform, code-growth policy or new cache. The
existing per-function backend and per-TU source/fact owners retain their release
boundaries. Later-stage runtime and optimization requirements remain distinct.

The work expanded from unavailable prerequisites through every related
call/constructor fixture and the scalar-new lifetime correction. All ten
fixtures now pass. The five remaining fixture failures need separate owners:
packed SIMD operations require saturation/lane and subsequent builtin semantics;
automatic-local odr-use needs function/capture ownership checks; missing-return
rejection needs CFG reachability; allocation exception compatibility needs a
semantic or documented reference-policy resolution. General vector subscripting
is also still unfinished. These cannot be repaired by changing completion order,
destructor demand or template provenance flags. This is the concrete incomplete
handoff boundary; none is waived or recast as an independent-review question.
Independent review of implementation199/200 deltas remains for Ralph's audit.
