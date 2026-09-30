# PA23 implementation handoff 123

Target remains **PA23 full-stage**. This handoff completes the shared-subobject
semantic, complete-object layout and table/projection group; it does not certify
the assignment or waive independent review. Stage base and last reviewed commit
remain `f33dd0775073bf5db6665fb2f4f504783159df76`. Turn entry was clean at
`9688f9e54a2b206b9add41ce78acd9b5f0452c62`, with **20/45** required tests passing.

## Owners and data flow

- **Subobject identities:** `semantic/virtual_subobjects.cpp` interns a shared
  virtual anchor plus a nonvirtual edge path. A relative occurrence has a compact
  identity; ordinary repeated bases remain distinct. Base-path facts merge only
  equal occurrences. Class-owned virtual-base closure slices are immutable after
  completion and indexed by canonical class/base identities.
- **Final overriders:** imported slot roots carry both declaration-occurrence
  and implementation-occurrence identities. Local overrides update candidates;
  a maximum-selection pass followed by a verification pass rejects nonunique
  final overriders. Receiver containment is cached by occurrence pair for this
  class completion, including negative results. All signatures reuse that graph
  fact. The result updates every affected view, abstractness and required primary
  inherited-override slots. No unrelated declarations are inspected.
- **Layout:** class facts distinguish nonvirtual size/alignment from complete
  extent. Only direct nonvirtual bases contribute embedded extents; each virtual
  base is placed once. Base projections record their source owner, virtual row
  and nonvirtual tail. C++ polymorphism is a separate fact from the need for a
  virtual-base layout pointer; nonpolymorphic virtual inheritance remains
  nonpolymorphic for `typeid` and `dynamic_cast`.
- **Tables and RTTI:** each segment records its own vbase/vcall row count,
  address point, physical offset and virtual anchor/tail. Shared addresses have
  one stored view. Vcall rows are deduplicated by virtual signature and belong
  only to their segment. Explicit dispatch demand publishes required tables and
  base-table dependencies; pure semantic completion does not demand member
  bodies. RTTI distinguishes repeated ordinary subobjects from shared diamonds.
  Complete-object VTT entries use the recorded segment facts. Virtual-base
  cast hints use `-1`; a fixed positive hint would incorrectly promise a unique
  public nonvirtual base. Nonpolymorphic layout tables have a zero RTTI entry,
  as required by the [Itanium ABI RTTI contract](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#rtti-layout).
- **Typed lowering:** reference/pointer adjustments and virtual-view stores
  read the recorded row from the object's table. Reference/pointer parameters
  gain no hidden ABI argument. Covariant thunks preserve null, use dynamic
  virtual-return projections and keep required unadjusted primary slots.
  Their keys include target, this adjustment, result tail, result virtual row
  and deleting-entry category. No printed name or serialized IR is a semantic
  key, and lowering does not repeat overload selection.

Work follows imported base-closure entries, physical slot/view records, consumed
receiver paths and emitted rows. Identity/fact lookups use TU-owned flat indexes.
The final-overrider tournament performs two slot scans; repeated receiver
relations use its completion-local cache. RTTI scans relevant base-type edges
without expanding already visited virtual subtrees. Shared source patterns are
not reparsed or cloned, and no global cache generation or retry sweep was added.
TU vectors release with the analyzer; candidate/containment indexes release at
class completion; table buffers and thunk bodies release after emission.
Telemetry counts base imports, subobject identities/paths, overrider work,
storage, views, slots and emitted IR.

## Validation scope

[Sealed validation](../student.tests/pa23/validation123.json) records required
checks, original failures resolved, coverage manifests and all retained failures.
The stage improves **20/45 → 24/45**; the four resolved cases are ambiguous
virtual-diamond rejection, inherited virtual-base dispatch, inherited override
slot preservation, and second-slot dispatch through a rich vtable.

[Semantic controls](../student.tests/pa23/controls123-semantic.json) distinguish
shared versus repeated overriders, pure/final functions, overload signatures,
template instances, mixed virtual/nonvirtual roots, covariant declarations and
nonpolymorphic dynamic-cast rejection. [Executable controls](../student.tests/pa23/controls123-layout.json)
cover shared/nested field access, adjusted/null pointers, multiple virtual rows,
RTTI, alignment, nonvirtual tails within virtual bases, inherited primary slots
and covariant pointer/reference/null results. They check emitted LowIR and run
it through the supplied native backend, using the host linker only for its
objects. Instrumentation must leave generated LowIR unchanged.

The executable record deliberately retains two failing lifecycle probes. They
are unfinished implementation, not accepted behavior or backend exceptions.
Inherited controls and explicit accepted-output roundtrips are also rerun.
The handout's standalone backend passes 16/17 completed controls. The remaining
shared-base RTTI cast fails with both student and reference-generated LowIR,
while both pass hosted execution. [The reproducible probe](../student.tests/pa23/backend_limit123.py)
and [observations](../student.tests/pa23/backend-limit123.json) retain the raw
reference duplicate-private-label failure, a metadata-only private-name probe,
its runtime failure, and both successful hosted runs. The ABI hint bug found
while checking this was fixed before sealing; both IRs now use `-1`. This is a
supplied standalone-runtime limitation, with no fixture/comparator change.
[Performance evidence](performance123.md) reports frozen compiler latency/RSS
and checked native runtime/text under the PA23/O0 stage-scoped acceptance.

## Reference observation, not an oracle change

[The deterministic reducer and proof](reference-observation123.md) establish
that the pinned reference reads a sibling's field when a nonpolymorphic virtual
base is accessed through `B&`. It returns 1 where the source requires 0; our
layout/access control returns 0. N3485 [class.mi]/4,/6, [dcl.init.ref]/5 and
[expr.ref]/4, plus the PA23 reference/pointer access contract, establish the
required behavior without relying on compiler agreement. No fixture, output,
expected status or comparison rule changed in this turn. Full oracle correction
remains implementation work alongside complete lifecycle output.

## Concrete remaining boundary

All **21** remaining required failures stay required. Most reach the distinct
**complete/base lifecycle and parameter ABI owner**: one-time virtual-base
construction/destruction, transfers, construction-table slices, VTT forwarding,
hidden virtual-base addresses and signature agreement across declarations,
ordinary calls, conversions, inherited forwarding, placement-new and cleanup
paths. The completed table/projection facts cannot substitute for those action
and entry-parameter records. Current C2 calls still use ordinary static layouts;
this both constructs a shared base twice and can overwrite a sibling's vptr.
Fixing only a constructor's store or appending an argument at one call site
would leave its other callers and cleanup paths inconsistent.

The initial scope was extended through RTTI, dynamic access, inherited primary
slots, covariance, nonpolymorphic language classification and a reused receiver
relation cache. Further related progress now requires changing the ABI/action
representation across the call paths above, rather than another shared-layout
or table fact. That is the implementation handoff boundary. The inherited
virtual-member-pointer formation/conversion/dispatch defect is also retained;
it needs its own member-value representation work. Neither group is recast as
an independent-review question.

Independent review remains pending for canonical identity completeness, demand
boundaries, containment keys, layout/table ownership, RTTI flags, ABI thunk keys,
storage lifetimes, inherited pipeline alignment and the performance evidence.
No review marker moved, no requirement was waived, and PA24 is not started.
