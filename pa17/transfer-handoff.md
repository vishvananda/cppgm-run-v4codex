# PA17 transfer and automatic-initializer implementation — loop 57

Entry: `6915296626992eb1ed20f6ca6e8c25a028c0335f` (324/343).
Implementation: `4e82009f`, then `794b150e` (330/343).
This is an implementation handoff, not independent stage certification.
No course fixture, reference, bundle revision or comparator changed.

## Owners and data flow

| Behavior | Semantic owner → consumer | Work and lifetime |
|---|---|---|
| Specialized wide member stores | `operators.cpp` records the conversion policy → `values.cpp` consumes the converted immediate with the correct signedness/width. Ordinary O0 assignment boundaries and union lifetime rules remain covered. | Constant work per conversion, no new cache or representation. |
| Empty subobject transfers | `transfer_actions.cpp` prepares selected copy/move actions → `transfers.cpp` emits the memberwise actions. Trivial empty subobjects do not turn a sparse assignment into a whole-storage copy. Nontrivial empty assignments, volatile members, references, arrays and unions retain their own actions. | One existing transfer fact per special member; work proportional to declared subobjects. No expansion beyond inherited eight-element array budget. |
| Base conversion and qualified receiver | `member.cpp` owns canonical reachability paths and separately demanded layout. `overload.cpp` / `template_call_facts.cpp` record the receiver-to-qualifier and qualifier-to-declaration adjustments → `expression.cpp` emits those two recorded conversions. Ordinary conversions consume the total offset. | Flat `(from entity, to entity)` index; vector records share tails and cache both missing and ambiguous paths. Each demanded pair visits its declared base edges once. Layout totals are established once from completed layout. TU-owned; no name keys, global invalidation, graph clones or lowering lookup. |
| Constant addresses | `constant_addresses.cpp` and `constant_objects.cpp` consume the same path total as runtime pointer/reference conversion. Null pointer adjustment remains conditional only for nonzero totals. | Shared typed path facts; no second search or textual adapter. |
| Automatic array initialization | `constant_array.cpp` retains short wide-integer stores within the eight-lane budget and keeps class materializations in evaluated scalar initializer expressions. Required constexpr array facts and established scalar images remain available. Ordinary initialization lowering consumes the existing typed initializer plans. | Product of dimensions saturates at nine; no bound expansion. Materialization eligibility scans explicit initializer nodes once per declaration, without grammar replay. Large arrays retain bulk-copy/loop fallbacks. |

The base path graph is fixed before class member checking. Template patterns and
concrete specializations have distinct canonical entity identities. Reachability
is a graph fact; candidate viability must not demand class layout. A performance
preflight exposed that violation in the initial increment. The retained reducer
`student.tests/pa17/qualified-fixed-base.cpp` and focused control now pass after
separating graph and layout demand. The interrupted performance JSON retains all
observations and the frozen failing compiler; it supplies no acceptance claim.

N3485 [class.copy]/15,28 specifies memberwise non-union copy/move; [conv.integral]
governs converted integer values; [conv.ptr], [dcl.init.ref] and [expr.ref] govern
unambiguous base subobjects and receiver adjustment. Required ambiguous
conversions fail; overload viability returns an invalid conversion without an
exception, allowing a viable `void*` alternative. Pattern-only queries preserve
the later concrete check. No new optimization pass or native backend was added.

## Trace and validation

The nonzero qualified-receiver control goes through retained template syntax,
canonical `D<B>`/`Mid<B>` entities, selected `B::f`, recorded adjustment IDs,
completed base offsets, direct typed LowIR and the supplied O0 native backend.
The compiler itself does not invoke that backend. The trace artifact records
source, LowIR, native output, telemetry, hashes and checked exit status.

The evidence manifest binds the complete unchanged 343-case stage suite,
turn-entry and final failure sets, earlier-stage report, file audit, accumulated
personal controls, performance observations and source digest. Personal controls
are run explicitly by `transfer_replay.py`; they are not discovered by course
suite targets. The entry compiler fails both new ambiguity controls; final
passes them. Six pre-existing course failures close, with no replacement failure.

## Handoff boundary

The four transfer/adjustment failures are closed together, and their shared
initialization investigation also closes both automatic-array failures. Remaining
storage work is seven cases about publication/definition demand and static versus
dynamic initialization (including relocation targets and guards). Those facts
must be established before the transfer/initializer consumers modified here;
further edits confined to these consumers cannot settle those cases correctly.
The four query/closure failures require recursive candidate-demand states and
closure body/lifetime ownership. The two cleanup failures require full-expression
exception-region scheduling. Each is unfinished implementation, not an audit
question or a waiver. PA17 remains the active assignment.

Independent review should check graph-key validity across pattern/concrete
publication, qualified receiver provenance on both call paths, and the measured
O0 storage/work bounds. These questions remain open for Ralph's independent
review; the implementation checks and evidence do not replace that review.
