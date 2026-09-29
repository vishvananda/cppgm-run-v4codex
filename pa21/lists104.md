# Initializer-list semantics and storage (loop 104)

## Owners and data flow

Initialization owns the canonical `std::initializer_list` template identity,
checked library representation, element conversions and backing-array object.
Recognition occurs at the language-mandated namespace declaration; aliases and
specializations thereafter use entity/type identities. Other namespaces' classes
with the same spelling remain ordinary classes. The course's permitted forward
declaration receives typed pointer/count fields, without fabricated syntax. A
supplied library definition keeps its actual field identities, accessibility,
member bodies and arbitrary field names.

Source list formation and retained query formation publish the same `ListPlan`
facts. Selection examines initializer-list constructors first (except empty-list
default construction), then the ordinary constructor set. Overload comparison
uses the library-list preference and worst element conversion rank. Selected
conversions undergo narrowing, access, deletion, reference-binding and explicit
copy-list checks. Constructor defaults retain their original checking context.
Template and `auto` deduction examine each element, including expanded packs;
ordinary nondeduced braced arguments do not invent a type. Nondependent template
initializers reuse checked recipes; concrete dependent occurrences retain their
own conversion and storage identities.

Materialization creates `const E[N]` backing storage and explicit pointer/count
stores. Range lowering consumes recorded element and field identities, without
resolving library member names. Class elements construct directly into their
final array slots. Constant evaluation consumes the same element plans, with
persistent backing identities for namespace-scope constexpr lists. The semantic
rules correspond to the checked-in N3485 [dcl.init.list]/2–6,
[over.match.list], [over.ics.list], [over.ics.rank], [dcl.spec.auto] and
[temp.deduct.call] (`doc/n3485.txt`). No reference output was changed.

## Lifetime and demand

Direct local list initialization extends backing storage to the list object's
scope. Direct reference binding, nested lists and static/global duration use the
existing reference-storage owner; copying a list does not extend its backing
lifetime again. Nested backing arrays are retained in construction order and
cleaned in reverse. Constructor argument lists keep full-expression duration.
Local-static finalization destroys retained arrays even when the library wrapper
has a trivial destructor.

Each completed class element adds a partial-construction state. Exceptional
exits destroy only completed elements, in reverse order. Successful construction
retires those states and activates the complete backing array. Existing suffix
sharing avoids duplicating every prefix for long lists. Array destruction uses
the inherited expansion cap of eight elements, then a reverse loop. Static
arrays have finalization ownership; they do not enter ordinary temporary cleanup.

Backing-array plans do not demand aggregate transport helpers or otherwise
elided transfer entries. A related inherited defect was corrected: checking an
ordinary in-class constructor/destructor body still schedules its mandatory
semantic work, but `LocalDefinition` alone no longer marks an unused inline
entry referenced. A later actual use demands normal emission. Controls verify
both rejection of an invalid unused copy body and execution of an actual copy.

## Work and storage bounds

Library representation is cached by canonical class entity. List formation is
cached by source/target and initialization mode, or by the retained typed query;
validation has separate active/success/failure state. Candidate selection visits
candidate/element edges; materialization and lowering visit each selected element
once. Explicit different initializer values require emitted work per element.
Nested retention uses an iterative worklist over the actual nested list tree.

Plans, conversions, backing entities and lifetime alternatives use dense
translation-unit-owned vectors and identity indices. Lowering partial states and
suffix indices are function-owned; retirement visits reachable states once.
There is no source replay, textual LowIR transport or process-global cache.
Existing phase/work counters plus list-plan/object/type counts observe these
structures without initiating work. [Performance evidence](performance104.md)
checks specialization count, list length and class cleanup independently.

## Validation and unfinished integration

[Validation record](../student.tests/pa21/validation104.json): 22 original required
failures resolved, no new failure or coverage reduction; 51/51 personal semantic,
rejection and execution controls, plus 25/25 external-unwind controls. The latter
compile source with this compiler, validate its LowIR, use the supplied native
object backend, and link a host-compiled throwing driver. The host never compiles
the tested source. Counts 1, 3 and 16 cover partial construction, consumer throws,
reverse destruction and loop cleanup; nested lists and local-static atexit are
also checked.

The completed contract group is list formation, selection/deduction, scalar
storage/ranges and class-element construction selection. Class backing lifetime
behavior is implemented and tested, but its **two required LowIR integrations
remain unfinished**: `200-global-initializer-list-backing-storage-duration` and
`200-initializer-list-backing-array-lifetime`. Differences include placement and
closure of structural EH regions, inline versus shared resume suffixes, and reuse
of completed element addresses. The local case also expects a different unwind
path around the library accessor. Native success does not waive those comparisons;
no reference defect is claimed.

These integration failures join the existing 43 EH/lifetime failures. Finishing
that owner requires source try/handler and exception-object facts, complete
continuation context keys, and consistent full-expression/lexical region exits.
Changing list ranking or backing ownership further cannot supply those states;
changing only these fixtures' cleanup shape would duplicate the shared EH owner
and leave its source-handler compositions unresolved. This is the concrete
boundary for this implementation handoff, not a PA21 completion claim.

Independent review remains owed for list/query key completeness, dependent recipe
reuse, nested storage ownership and constructor checking/emission separation.
Those review questions are distinct from the known unfinished EH implementation.
