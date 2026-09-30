# PA28 loop153: virtual primary layout and ABI prefixes

Entry `fefbf8a7` failed only
`200-host-covariant-return-layout-finalization`: the virtual-result thunk used
-24 instead of -32. The implementation fixes the class/table model; it does not
rename a thunk or insert unexplained padding. No course input, reference,
comparison rule or runtime implementation is changed.

## Owners and data flow

The host ABI is selected once on the semantic analyzer by the existing host
object driver. The course LowIR/private runtime retains its explicit layout
contract. Both paths use the same parser, canonical semantic identities, typed
LowIR construction, native backend and ELF writer. Text is inspection output,
never production transport. The host compiler is used only for validation and
the assignment-authorized host link.

`ClassFacts` owns primary selection, nearly-empty classification, nonvirtual
size/alignment and final virtual offsets. `VirtualClass` owns typed prefix rows,
canonical virtual-base preorder, slots, physical views and VTT order. Each prefix
row names either a canonical base EntityId or a declaration and subobject ID.
Inherited rows keep their distance from the address point. A separate canonical
prefix describes the same type when it is a virtual base. Physical view slices
record the resolved final-overrider receiver once. `virtual_base_rows` provides
indexed class/base lookup for conversions, covariant results and RTTI.

Primary selection prefers the first nonvirtual dynamic base; otherwise it uses
the first eligible nearly-empty virtual base, excluding indirect primaries when
possible. Layout preserves alignment and permits dynamic non-POD base tail
padding reuse. Primary storage claims follow inheritance preorder, including
claims inside virtual bases. A selected primary can lose sharing of its own
virtual primary to an earlier occurrence. Each claim depends on one canonical
virtual anchor plus a fixed tail; the resolver memoizes anchors and path suffixes
instead of repeatedly scanning unresolved claims.

C++ virtual-base construction/destruction keeps the existing postorder. ABI
base rows, VTT secondary pointers and virtual sub-VTTs use distinct preorder
facts. Physical vptr ownership selects the first, most-derived occurrence at a
shared address. Host vtables emit complete groups, including secondary tables
needed by host-generated inline constructors referencing a key-owned table.
Construction segments use the same typed rows with complete-object offsets;
shared addresses alias an existing segment. No semantic lookup is repeated in
lowering. Inherited callable receivers retain their actual virtual occurrence,
even after that occurrence loses primary sharing. Base entry emission keeps
ordinary secondary vptr stores as well as VTT-selected stores; the latter also
include virtual primary aliases that may move. The lifetime control checks
secondary virtual dispatch during construction and destruction both with and
without a VTT.

`ReturnDerived::self` consumes the finalized fixed offset of 32. The virtual
`VirtualResult` destructor's vcall row precedes the `VirtualOwner` virtual-base
row, so the latter is -32. The derived thunk projects through that recorded row,
and null pointers retain the existing guard. The same row identity feeds RTTI,
constructor projections, complete tables and construction tables.

## Complexity, validity and lifetime

Primary selection visits unique base types/edges; physical storage and VTT walks
visit actual subobject identities and deduplicate virtual diamonds. Prefix and
slot work is linear in consumed/published rows. Physical table ordering is
O(v log v). Composition caches have complete `(outer, inner)` subobject keys;
fixed-tail and claim maps live only during one class layout. Published class,
base and prefix identities are immutable after their existing completion/layout
transitions. TU-owned vectors and flat ID indexes release with the analyzer;
native transient state still releases per function. No global retries, grammar
replay, string equality keys, per-node owning pointers or optional optimizer
passes are introduced. Existing work/storage telemetry includes the new prefix
arenas and virtual-base rows. Stage-scoped measurements are recorded separately.

## Contract and controls

The design follows [Itanium ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html)
§1.1 (nearly empty), §2.4 (primary selection and physical allocation), §2.5.2–3
(inherited mixed prefixes, vcall ordering and covariance), and §2.6.2–4
(VTT order and construction entries). C++ initialization order is N3485
[class.base.init]/10, in checked-in `doc/n3485.txt`. These are implementation
proofs, not a reference correction; the pinned reference bundle is untouched.

`virtual-primary153.py` compiles producer and consumer independently with the
host and student compilers, and checks all four link combinations against the
host control. It covers ten complete-object hierarchies plus the required covariance fixture: fixed layout/covariance, pointer-null and
reference results, nearly-empty and non-nearly-empty virtual bases, diamonds,
nested and indirect primaries, multiple vcall functions and secondary bases,
a later selected primary that loses inherited sharing, alignment, RTTI cross
casts, dynamic-cast-to-void, host inline constructors importing keyed table
groups, and constructor/destructor ordering and dispatch. The inherited naming,
exception and ownership controls run explicitly too. `inspection153.py` validates
host LowIR and verifies native-view/ordinary-object byte equality, required
thunk names, unwind facts and executable behavior.

Known implementation failures in this completed group: none after these
controls. Whole-stage independent review remains pending for all PA28 commits,
including canonical publication, demand/cache lifetime, table/VTT ownership,
course ABI separation, source-to-ELF tracing and performance evidence. Passing
this implementation handoff does not waive those review questions.
