# PA18 implementation handoff 75

Entry `9fa23653` → implementation `de0f5228`, `6229b49a`.
The immediate-context list/cast group and its ordinary, constant, exception and
ABI consumers are complete for this handoff. PA18 remains unfinished.

## Ownership, data flow and complexity

| Owner | Behavior and data flow | Bounds/lifetime |
|---|---|---|
| `semantic/query_list.cpp`, typed query graph | Braces retain immutable child query IDs, source context and canonical target type. Formation supplies candidate rank and a selected list recipe; a distinct validation query checks the selected recipe. Aggregates, brace elision, references, defaults, strings, narrowing, explicit/access/deleted construction and destruction share the conversion owners. No fabricated AST or reparsing is used. | TU-owned plans/edges. Memoized formation/validation follows actual initializer and candidate edges. Omitted array tails use counted fields. Completion dependencies invalidate affected consumers, including failed queries. |
| Ordinary/fixed initialization, default constructor properties | Narrowing consumes the selected conversion, including a user conversion's result before the final arithmetic conversion. Definition-time recipes and ordinary scalar brace casts apply the same numeric predicate. Default-constructor validity returns a structured fact to queries and a hard error at the ordinary boundary; triviality remains a separate BooleanFact value. | Selected conversions are reused, not resolved again. Default subobject properties cache per class/member; failure does not demand a body. Fixed source recipes retain their source/context identity. |
| Constant and exception consumers | Query calls retain actual braced values, including `typename T::type{}`. Constant evaluation constructs scalar/aggregate/class values and string tails from the plans. Variadic constexpr calls evaluate every supplied argument, while binding only named parameters in the body frame. Noexcept consumes the same selected conversions and constructors. | Work follows supplied values, selected defaults, object fields and existing execution memoization. Cache keys include every variadic argument and storage dependency. No skipped ellipsis side effects or nonconstant operands. |
| Explicit conversion and base facts | Expected invalid casts return structured failure. Virtual/transitive/ambiguous downcasts and inaccessible static casts are rejected; C-style access exceptions are preserved. Valid nonzero base downcasts retain cached inverse adjustment, including references and null pointers. | Traverses the actual base path; inverse adjustment is cached on that path. Zero adjustment remains zero, preserving inherited LowIR. |
| Typed ABI graph | Source list expressions encode typed `tl` and untyped `il` nodes. The reader/writer adapter roundtrips the same graph roles; production query projection retains typed braces. | Compact graph identity and linear encoding; no serialized fact transport in the compiler. |
| Parameter representation boundary | An emitted namespace function declaration completes its by-value class specialization before lowering allocates entity-indexed tables. Unused member bodies remain dormant. This repairs the allocator-sensitive duplicate-symbol failure uncovered by the full suite. | Existing monotonic declaration cursor and completion cache; no rescan of unrelated declarations. Representation/body scheduling remains in semantics. |

Language anchors: C++11 N3337 §8.5.4 [dcl.init.list] (including narrowing after a
user-defined conversion), §13.3.3.1.5 [over.ics.list], §14.8.2 [temp.deduct]
(immediate context), §12.1 [class.ctor] (defaulted deletion), §5.2.9
[expr.static.cast], §5.4 [expr.cast], §5.19 [expr.const], §7.1.5 [dcl.constexpr]
and §5.3.7 [expr.unary.noexcept]. The local Itanium expression grammar specifies
`tl`/`il` at [doc/itanium-mangling.txt](../doc/itanium-mangling.txt):520–521,601.
No reference correction, fixture, harness, comparison or bundle revision is made.

## Validation and evidence

`make test-pa18`: **383/420**, original failures **41 → 37**, four repaired
paths and no new failures. All four repaired fixtures validate and execute
through the supplied native backend. The unchanged input/reference manifest
covers **420 inputs, 1,712 files**.

`make test-report-through-pa17`: **2609/2609**. PA18 file audit passes with the
same three inherited header advisories. The new query branches exceeded the
expression-query function limit; moving new-expression query construction to
its existing owner restores the limit without compressing code to evade it.

**718/718 personal semantic controls** pass (71 new and 647 inherited), with
seven ABI checks, the direct graph API and nine completion/array graph controls.
They cover ordinary/fixed/query narrowing, exact/inexact constant
conversions, nested braces, strings, reference binding/ranking, selected defaults,
invalid constructor/destructor properties, poison bodies, effects, casts,
constexpr ellipsis and typed ABI. Graph controls cover localized completion and
constant-size omitted array tails. All personal tests are explicitly run;
`tests/regression/` is outside the exit criteria.

Intermediate observations remain in `/tmp/pa18-loop75`, including failed runs.
One personal expectation was corrected: in C++11 a deleted default constructor
alone is not user-provided and does not make a class nonaggregate. Adding a
user-provided `A(int)` makes the deleted default-subobject control exercise the
intended constructor rule. No course test was changed. Earlier-stage AST-label,
zero-adjustment and extra-constructor regressions were repaired before the final
prior report. See [performance](performance75.md) and the
[evidence manifest](../student.tests/pa18/loop75-evidence.json).

## Handoff boundary and required unfinished work

The initial scope was extended through constant/noexcept/ABI consumption, ordinary
and retained fixed narrowing, constructor property failures, string aggregate
values, base adjustments and the representation lifetime bug. Remaining course
failures require different owners: declaration visibility and lazy nested-class
lookup; inherited-constructor and explicit member-template participation; ordinary
LowIR initialization/result policies. The array-policy failures conflict with
simple blanket changes because earlier fixtures require pooling for the same
small scalar-array shapes. Resolving that policy and the declaration/constructor
state machines is not an extension of this group's selected-query recipe; it
requires separate contract/ownership work. No stage switch or comparison waiver
is introduced. The inherited class-ellipsis and nested-alias cast reducers remain
unfinished ordinary lowering work.

Independent audit must review this group's source/query identities, selected
conversion validation, completion edges and cache lifetimes across consumers.
Those are review questions, distinct from the explicitly unfinished requirements
above. Neither is waived. Ralph owns independent review and stage advancement;
this handoff does not certify PA18 complete.
