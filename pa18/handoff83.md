# PA18 implementation handoff 83

Entry `48c864abc19c24c4e9f5706a86109c023e2da7f7`: **396/420**.
Code increments `e9108b5a` through `ac354fad` complete array initialization and constant
materialization through their source, substitution, query and storage consumers.
The stage-base and last-reviewed markers remain unchanged. This is an
implementation handoff; independent review and whole-stage completion remain.

## Completed behavior and ownership

| Owner | Data flow | Complexity / lifetime | Validation |
|---|---|---|---|
| `initializers.cpp`, `type_builder.cpp` | Visible declaration → checked initializer plan → completed canonical array type | One plan per initializer/type, O(explicit actions); count actual elements after brace elision, never replay grammar; TU fact storage | Flat/mixed nested arrays, aggregate elements, strings, self-address, prior bounds, static and constexpr arrays |
| `template_initializers.cpp`, `default_arguments.cpp` | Retained source initializer → fixed shape/checks or dependent completion | One source check; scalar pack patterns and fixed suffixes retain narrowing checks; each array iteration consumes a clause or rejects | Packs, sentinel/empty lists, unused bodies, trailing narrowing, empty aggregates, fixed and dependent element types |
| `type_query.cpp` | Dependent array declaration identity + substitution frame → completed entity type → sizeof/decltype/call deduction | Existing query/frame caches; O(1) completed-bound reads; no broad invalidation or candidate retry | Different expansion lengths, array-reference deduction, pointer rows, fixed source arrays |
| `template_checks.cpp` | Source member prototype + canonical element identity → matching static-array declaration → supplied-bound check | Indexed prototype bucket; no unrelated declarations visited; source signatures remain source-faithful | Unsized/sized definitions, renamed bounds, incompatible bounds, inherited static constexpr arrays |
| `constant_array.cpp` and existing typed lowering | Checked constant plan → readonly typed image → one copy into distinct automatic storage | O(explicit actions + emitted data), existing typed payload interner; removed whole-initializer construction scan and small-wide exclusion | Wide scalars, constexpr conversion, mutable identity, pointers, strings, nested arrays, volatile/effectful fallback |

The group was extended after initial course progress: bound inference now counts
initialized elements rather than source clauses; braced strings include the
terminator; source queries wait only for genuinely unknown expansion shape;
static member declarations preserve spelled signatures and completed object types
separately. The no-progress guard prevents an empty aggregate from looping over
the same initializer clause. Expansion shape does not suppress fixed narrowing
obligations. Root array initializers enforce C++11 braces/string/value-initialization
forms; parenthesized strings preserve their complete bound. No source file was added, so source-set registration is unchanged.

Constant classification keeps nonconstant calls, volatile writes, automatic
addresses and class lifetimes on their ordinary paths. Required constexpr
objects still receive constant-expression validation. No optional optimization
pass, wider evaluator, text transport, new global cache or emitted code clone was
introduced. The existing bounded omitted-element and typed-data policies remain.

## Reference corrections

[The proof](reference-correction83.md) derives **17 oracle revisions** from the
cumulative PA16 constant-array requirement and C++11 initialization rules.
Fifteen array oracles (13 PA18 and two PA17) now require the inherited readonly
image/copy form. The constexpr union uses required static initialization. An
empty expansion initializing an unknown-bound array correctly rejects under
[dcl.init.aggr]/4. Original sources, fixture paths and comparison rules remain.
The independent transformer reads entry references, checks offsets/full object
sizes, preserves later instructions, and records all before/after hashes.

Reduced reference observations and every revised success oracle are validated
and executed by the explicit personal harness. The startup-order union reducer
returns 1 with the reference and 0 with this compiler. The empty-array reference
accepts the invalid source and emits an invalid `obj<1x4>` slot that its own native
backend rejects. The reference cannot lower the reduced constexpr conversion;
that failure is recorded, not treated as performance evidence or language proof.
The proof rests on the cited standard/contract, not compiler agreement.

## Validation and performance

- `make test-pa18`: **411/420**, exit 2; original failures **24 → 9**, no new failures.
- Earlier stages: **2609/2609**. File audit: pass, with three inherited header advisories.
- New array controls: **61/61**, including initializer forms, query/static-member consumers and progress guards; seven additional storage checks.
- The final **1225/1225** cumulative personal run, inherited ABI/scaling/summary inspections, five source-to-native traces, **34** PA16 controls and **46** reference observations pass and are bound in the manifest below.

Final check counts and all four performance dimensions are recorded in
[performance83.md](performance83.md) and
[loop83-evidence.json](../student.tests/pa18/loop83-evidence.json).
Final evidence must bind the compiler hash, final required checks, unchanged
source coverage, reference revision manifest, personal controls and frozen
compiler/native measurements. Initial attempts remain in `/tmp/pa18-loop83`;
only final evidence is acceptance evidence. The initial baseline run passed 25/45
controls then present; a later baseline probe of an empty aggregate template
initializer timed out after 30 seconds. Both observations are retained. The final
61-control set includes the added rejection and parenthesized-clause probes.

The new [array trace](../student.tests/pa18/array83_trace.cpp) follows one pack
expansion through element-count inference, sizeof/decltype, array-reference
deduction, typed readonly data, copies into distinct arrays and checked native
execution. The compiler itself never invokes the reference backend.

Performance records **424 observations across 16 fixed workloads**. Required wide-array and converted-array
materialization adds paired 3.7% and 7.7% compiler time at 2400 functions. The
wide-array native probe is 6.5% slower, with payload 284 → 320 bytes; the report
accounts for the required 32-byte image/copy and retains all observations. Work
counts scale linearly; common native output is unchanged. These are disclosed
contract costs, not an optional optimization benefit or a new numerical gate.

## Handoff boundary

The completed group supplies array types, initializer actions, constant values
and materialization. None of the **nine remaining course failures** is an array
initialization failure. They require distinct facts: empty-object constructor and
object-root emission, class-result ABI classification, static-member storage
publication, and scalar widening/discarded-value representation. The inherited
class-ellipsis reducer also remains unfinished. Changing array plans to force
these results would either guess calling conventions or introduce unrelated
emission policy; those owners need their own end-to-end implementation trace.
All remain implementation requirements. No coverage or comparison is waived.

Independent review questions are separate: verify source-bound completion versus
retained signatures, query/frame identity after local-array instantiation,
static-array bound compatibility, and proof boundaries of constant-array
materialization. Controls and measurements support the implementation handoff;
they do not replace that review. The existing review markers are preserved.
PA19 cannot start before the complete through-PA18 report and whole-stage audit.
