# Member-pointer constant values and storage

Implementation 43 closes the recorded member-pointer array reducer and extends
that represented value family through scalar operations, aggregate/array storage,
constexpr calls, references and template static members. The explicit controls
are in [member_constants.py](../student.tests/pa16/member_constants.py).

## Semantic owner and data flow

`Constant` is type-directed: a member-pointer value carries the selected member's
canonical `EntityId`, with zero reserved for null. It is neither a raw object
address nor a rendered name. The existing typed aggregate interner and complete
activation keys include the type and payload, so distinct member arguments cannot
alias a memoized call. No new per-value allocations or global caches are added.

Address formation uses the declaration/access facts. Application follows the
existing typed receiver/base path, then the member selector. This preserves
subobject identity, volatile/mutable restrictions and active-union checks.
Constexpr member-function invocation uses that selected function's existing body
and activation machinery. Same-owner qualification, null, bool, equality,
conditional and value-initialization paths share these facts. Function-pointer
equality also compares the runtime adjustment word and ignores it for null.

Persistent storage converts data-member constants to their layout offsets, with
−1 for null. Member-function constants emit a typed function relocation and the
adjustment word. The represented nonvirtual, same-owner function pointer has
zero adjustment. The readonly-array interner consumes those typed items and
still makes one distinct automatic destination and one copy. Omitted null data
members cannot use zero-byte filling; the existing semantic zero-value predicate
now guards the sparse bulk-fill shortcut.

A static reference binding to a constant scalar prvalue gives its temporary a
semantic declaration and persistent storage identity. Its reference constant,
address comparisons and LowIR relocation all name that same object. Constant
classification creates this backing object only after successful evaluation;
failed/dynamic probes do not publish a temporary. The TU owns it until lowering
finishes. Separate bindings retain separate storage even for equal values.

Template initializer queries retain whether address syntax was qualified and
the selected member identity. An actual static-member storage demand walks its
constant graph once, deduplicates typed values, and requests the function bodies
needed by relocations. An unevaluated class or unused template does not trigger
that walk. Lowering does not reconstruct lookup or instantiate a missing body.

## Work bounds and scope

Member formation/conversion/equality are constant work on existing identities;
application follows only the receiver's required base/subobject path. Constant
arrays remain sparse in semantics. Storage-demand work is O(distinct reachable
constant values plus child edges) per demanded declaration, with a short-lived
flat visited index. Data writing is O(emitted data), and each function relocation
uses the existing deduplicated member-demand state. The existing 512-call and
1,000,000-step constexpr bounds remain unchanged. No optional optimization,
grammar replay, textual key or global retry was introduced.

The source surface remains the inherited member-pointer implementation: member
formation, same-owner values and applications to compatible/inherited receivers.
Cross-owner member-pointer casts and virtual member-pointer dispatch need new
source conversion/ABI facts and are not implemented here. They were absent from
the inherited runtime path (PA12 explicitly excludes member pointers from its
required contract). The whole-stage audit must review that inherited boundary;
the green course suite alone does not certify additional language surfaces.
Performance and validation evidence accompany the final handoff plan.
