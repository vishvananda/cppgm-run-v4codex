# Implementation handoff 125

Target: PA23 full-stage. This is an implementation handoff; independent
whole-stage audit still owns advancement. Stage base `f33dd077` and last reviewed
commit `36e612a0` remain unchanged. Entry was clean at `7a644d69`, with 24/45
passing and 21 named failures. The preceding audit turn made verified progress.

## Completed owners and data flow

| Owner | Published facts and consumers | Complexity and lifetime |
|---|---|---|
| Semantic class layout | `LifecycleBase` records physical type, offset, virtual status and VTT slice; virtual views record physical store order | Once per completed class; O(bases + views log views), contiguous TU records and class-local ordering scratch |
| Semantic lifecycle | Complete constructors/transfers own shared bases in depth-first order; base entries skip them; destruction reverses the established order; assignment follows direct-base declaration order | One action plan per canonical member; each emitted entry visits its own actions. Existing bounded cleanup/array policies remain |
| Construction support demand | A class publishes its VTT dependencies once; construction-slot demand has independent active/success/failure state, including key-owned external bases | Each required class/slot fact once; no global retry or dependence on TU order |
| Typed construction lowering | Complete/base ABI entries, VTT slices, hidden base addresses, constructor/destructor projections, construction RTTI, offset-to-top and rebased final-overrider targets | O(emitted actions and table rows), each physical segment has one program identity; no traversal of duplicate shared paths |
| Parameter ABI | Program signature identity owns visible parameter count and `(parameter, virtual-base offset)` facts; all source call paths append by-value hidden addresses through one emitter | Linear in actual signature/argument facts; function body binds hidden addresses by parameter/base identity; pointers/references have none |
| Member pointers | A callable address and receiver displacement are used by formation, static storage, conversions, bounded value proofs and invocation; an addressed virtual declaration owns a dispatch callable | O(1) average program lookup; at most one small dispatch body per addressed virtual declaration, forwarding the already prepared ABI arguments |
| Output scheduling | Base/complete/deleting entries retain independent emission identities and canonical presentation order | Linear in reserved emission identities; un-emitted deferred declarations cannot index emission arrays |

The parameter ABI handles ordinary and indirect calls, virtual calls, member
pointers, constructors/conversions, inherited forwarding, lifecycle hidden
arguments and separate/merged TUs. Transfers read a source base's virtual layout
through that source's table; construction destinations use their explicit entry
context. Hidden addresses remain valid before construction vptr publication.

Construction views contain the active constructor/destructor class's RTTI and
offset-to-top, while locating virtual bases in the complete object's physical
layout. Construction support is demanded independently of an ordinary table's
key definition. Ordinary and construction table identities, adjustor thunks and
virtual member callables survive merged TUs in `Linkage`; class facts and
function emission scratch retain narrower lifetimes. Lowering consumes typed
facts into `Program`/`FunctionBuilder`; it performs no textual phase transport,
parser replay, host source compilation or reference delegation.

The current source-to-LowIR member-pointer convention keeps the existing
callable-plus-displacement representation. A virtual callable performs the
logical-slot lookup after the displacement is applied. This also works when an
inverse conversion names a nonpolymorphic owner. It is a language implementation
for PA23, not a claim of later host Itanium member-pointer interoperability;
that ABI boundary belongs to the host-compatibility stages. The semantic
constant still identifies the selected member, independently of its callable's
emission identity, so later ABI adapters need not reconstruct source semantics.

## Evidence and remaining review

[Validation125](../student.tests/pa23/validation125.json) records exact commands,
terminal exits, compiler identity and log/evidence hashes. All required course
checks pass: PA23 45/45, earlier stages 3811/3811, through PA23 3856/3856. File
audit passes with the three inherited header-organization warnings. All 44
accepted outputs roundtrip. Explicit controls cover lifecycle/parameters
22/22, member pointers 13/13, TU ownership 12/12, inherited ownership 21/21,
layout 19/19, semantics 26/26, earlier behavior 33/33, earlier lifecycle 20/20,
and deep final-overrider/construction 2/2. The two former lifecycle probes and
the deeper runtime fault are now passing observations, not waived failures.

The final check caught a PA18 emission-order regression: the new schedule
visited declarations without reserved emission identities. The correction
bounds traversal by those identities and skips un-emitted entities. The full
through report was repeated successfully afterwards. A further narrow fix
publishes each construction dependency set once; final checks and measurements
use that binary.

[Reference correction125](reference-correction125.md) documents reduced C++11
and contract proofs, the pinned bundle revision, exact coverage hashes and all
20 revised oracles. Two polymorphic outputs have only the proven metadata
field corrections. Eighteen nonpolymorphic virtual-base outputs require a
consistent table-based layout and dependent lifecycle/ABI changes. No fixture,
expected status or comparison rule was removed or relaxed. The source fixture
with indeterminate virtual scalar members remains a LowIR comparison; its
undefined runtime result is not used as a correctness benchmark.

[Performance125](performance125.md) records the frozen protocol, observations,
compiler latency/RSS, checked generated runtime/text and explicit work/growth
bounds. PA23/O0 stage acceptance applies; historical diagnostic targets are not
new exit gates. The explicit LowIR contract retains rooted VTTs, including
view-only cases, so their complete cross-TU-compatible shape is preserved.
All 19 common executable text outputs are identical. Six final-only semantic
workloads pass; compiler text grows 35,072 bytes (1.50%). The forest adds 8 ms
and 3,384 KiB peak RSS, mostly accounted for by lifecycle-record capacity.
All samples, including outliers, remain recorded; no speedup is claimed.

No known PA23 implementation group remains unfinished. Independent audit must
still examine this full range, the oracle proofs, construction identity and
signature ownership, private LowIR member-pointer representation, and measured
costs; passing tests does not waive those questions. The inherited standalone
backend's shared-RTTI scan limitation remains a supplied-backend observation,
not a source lowering substitution or a relaxed comparison. Native backend,
host interoperability and later debug/optimization/self-hosting requirements
stay with their owning stages. Do not advance before whole-stage audit.

| Handoff | Range | Boundary | Independent review |
|---|---|---|---|
| 125 | `7a644d69..d9179e84` plus evidence records | Complete lifecycle/parameter ABI, virtual member-pointer behavior and justified oracle reconciliation; 24→45/45 | Whole-stage audit pending; review markers preserved |
