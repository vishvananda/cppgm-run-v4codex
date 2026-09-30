# Implementation handoff 121: ordinary nonvirtual polymorphic views

This is an incomplete PA23 implementation handoff. The full stage and its
independent audit remain required. No handout, fixture, reference output,
comparison rule or test coverage was changed.

## Completed behavior and ownership

Class completion selects the first nonvirtual polymorphic base as primary,
independently of declaration order. It indexes local overrides by interned name
and canonical signature, then visits inherited slots by subobject identity.
Unrelated same-signature roots retain different final overriders. Secondary
pure, final, static and exception-specification checks use those same identities.

`VirtualClass` owns one contiguous slot arena. Each `VirtualView` is a slice
with a parent subobject, base edge, static offset and vptr-store ownership.
Primary chains need no extra views; their aliases are materialized only when
an enclosing class requires secondary tables. Layout establishes receiver and
covariant result displacements. A new primary slot preserves an overriding
function's unadjusted result when the inherited slot requires a base result.
Outgoing result-layout demand reacquires records by class identity, so deferred
class completion cannot invalidate an arena reference.

Lowering consumes these facts to emit separate tables, one store per physical
vptr, and thunks keyed by target, receiver adjustment, result adjustment and
deleting-entry identity. Thunks forward visible arguments and indirect results;
covariant pointer results preserve null, while references project directly.
Signed downcast operands render correctly. The source `this` value retains its
non-null fact, avoiding unnecessary pointer-conversion branches.

RTTI requests from expressions, vtables, throws and handlers enter one
TU-owned deduplicated type queue. After declaration/body discovery, the semantic
owner computes class flags once. Single-base propagation is constant work;
branching nonvirtual RTTI visits only reachable base edges and detects repeated
types without expanding repeated subtrees. Lowering requires published flags,
emits every direct-base descriptor, and uses the selected view's offset-to-top
and dynamic RTTI. A `-2` cast hint does not imply failure: a more-derived object
can provide the public source and target paths for a sibling crosscast.
Repeated public source subobjects use the multiple-base hint.

Secondary views have distinct private object names, and the pure-virtual
helper declares its runtime role, so standalone emission does not confuse a
LowIR name with its object alias.

The relevant language rules are N3485 [class.virtual]/2,7–8 (overriding and
covariant results), [expr.dynamic.cast]/7–9 (complete-object runtime checks),
and [expr.prim.general]/2 (`this` denotes the invoking object), in the checked-in
[standard draft](../doc/n3485.txt). These explain implementation decisions;
there were **no reference corrections**.

## Complexity, validity and validation

All new hot keys are canonical type/entity IDs and integer adjustment pairs.
Views and slots have TU lifetimes; completion scratch and RTTI graph traversal
scratch release after their owner finishes. Flat indexes deduplicate views,
thunks, requests and function demands. No textual phase transport, source-body
clone, name-based semantic recovery, global invalidation or broad body demand
was introduced.

Class completion and view lowering are linear in participating declarations,
base edges and produced slot facts, with average constant indexed lookup.
A branching class's repeated-base RTTI flag requires examining its reachable
base graph, once per demanded class; this is ABI work, not an optimization over
unrelated declarations. Primary-only inheritance avoids an ancestor-view
Cartesian product. Each demanded view/table and distinct thunk emits once.
The non-null fact costs constant work and has zero instruction-growth budget;
there is no new iterative optimizer or optional cloning/inlining pass.

The [sealed validation](../student.tests/pa23/validation121.json) records the
original 42 failures and the 28 remaining ones: **14 existing failures resolved,
no new failures**, and **17/45** required tests passing. The final compiler
passes **3811/3811** earlier tests and the file audit (three inherited header
organization warnings). All **41** accepted PA23 outputs roundtrip stably;
that validates IR structure, not the correctness of still-failing cases.

Explicit controls are **32/33**: ordinary dispatch, primary-base placement,
repeated/unrelated roots, later-base override rejection, forwarded scalar and
class returns, pointer/reference covariance, null downcasts, RTTI, private-base
crosscasts, covariant result-layout demand stability, and exception base matching pass.
The failed control below is preserved as unfinished work. No independent
review has been performed over these implementation commits.

## Concrete next boundary: unfinished implementation

1. **Shared virtual-subobject identity and layout.** Current view occurrences
   represent nonvirtual paths. Shared virtual bases need canonical complete-
   object identity, separate nonvirtual extent, dynamic base projections,
   per-segment vbase/vcall rows, diamond RTTI flags and final-overrider dominance.
   Appending more static tables cannot implement these relationships correctly.
2. **Lifecycle and hidden-parameter ABI.** Complete/base construction,
   destruction and transfer entries need distinct typed arguments and targets.
   By-value virtual-base parameters carry hidden pointers; reference/pointer
   parameters must recover addresses through the object's vtable. The two
   nonvirtual virtual-destructor shape failures also belong to lifecycle entry
   demand/sequencing. All shipped fixtures remain required despite the handout's
   broader special-member exclusions.
3. **Virtual member-function pointer representation.** The new retained
   `virtual-member-pointer` control compiles but returns 1: its low word is the
   raw address of `B::b`, so calling it on `D` bypasses the final overrider.
   This needs coordinated semantic/static-data formation, tagged target
   representation, conversions, value proofs and indirect-call decoding,
   including inverse conversions to a nonpolymorphic owner. Changing ordinary
   vtable dispatch or adding a polymorphic-owner-only call test would leave that
   representation inconsistent. Preserve PA22 coverage and its comparison
   contract while resolving this distinct owner.

These are implementation tasks, not audit questions. This handoff stops at the
completed ordinary-call/view/RTTI path because further work requires those new
shared-subobject, lifecycle and member-value representations. It does not label
any remaining failure acceptable or complete the assignment.

Independent review still needs to assess this group's canonical identities,
demand/allocation boundaries, thunk ABI and performance evidence, then resolve
whole-stage findings under Ralph's audit schedule. Review markers in
[plan.md](plan.md) remain at the stage base.

## Supplied standalone-backend limits

[Standalone checks](../student.tests/pa23/native121.json) pass **25/29** executable
controls. The member-pointer control is our unfinished implementation above.
The other three failures are preserved and reproduced with source-generated
reference IR in [backend probes](../student.tests/pa23/backend-limits121.json):
`crosscast-null-and-miss`, `crosscast-private-base`, and `exception-rtti`.
The bundle is `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.

The reference-generated IR first encounters duplicate native symbols when
private object metadata equals an internal alias. A scratch observation changes
only those private object spellings (no body, type, data, relocation, order or
checked-in reference is altered). The reference standalone executable then
returns 1 on all three sources, which require 0 under [expr.dynamic.cast]/8–9
and [except.handle]'s public unambiguous base matching. The hosted backend/link
lane executes both our unmodified LowIR and the source-generated reference IR
successfully. These observations identify supplied-backend limits, not accepted
compiler failures or permission to change the fixture comparisons.
