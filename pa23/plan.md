# PA23 compact plan — implementation 125

Target: **PA23 full-stage**. Phase: **implementation**.
Stage base commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Last reviewed commit: `36e612a07bae32eb20d10ffc76bb03529555d4bb`.

Entry 125: `7a644d69fa5eed60841ed2a231c38591c46d9390`, clean; prior
turn classified as progress (completed checkpoint audit). No compiler/build
process remains live. Frozen entry compiler: `/tmp/pa23-125/entry-cppgm++`.
Baseline: 24/45, 21 existing failures. Preserve both review markers above.

Owner plan 125: semantic lifecycle actions own shared-base order and entry
selection; completed class ABI facts own construction-table/VTT slices and
hidden-parameter layouts; lowering consumes those facts at every call,
definition, transfer and cleanup. Publish each fact once, with flat identity
indexes and TU/function lifetimes; work tracks physical subobjects, required
table rows and emitted actions. Then finish member-pointer representation
across formation, conversion, proofs and calls. Validate nested graphs,
construction dispatch, transfers, TU order, all contract cases and inherited
controls. Reconcile proven erroneous oracles only after correctness is checked.
Freeze final binaries/inputs for A/A and ABBA latency/RSS, checked runtime and
text; apply PA23/O0 acceptance. Independent audit remains separate from these
unfinished implementation obligations.

[Audit124](audit.md) reviews all 18 commits from the stage base through
`2bba7273`, their combined 32-file implementation range and interactions, and
fix `36e612a0`. The following records commit changes no implementation code.
The previous goal turn was verified implementation progress; no interrupted
process was live at entry. Entry and final PA23 results are **24/45**, with
identical **21** failure names and all fixtures/comparison rules retained.

The audit fixed program ownership of adjustor thunks and secondary views,
external key-owned virtual-base view publication, exponential repetition of
shared views, and repeated base-path layout summation. Final-overrider candidates
are all checked before canonical physical views are coalesced. Linkage caches
use program symbol/adjustment identities, preserve internal targets, and survive
merged TUs; semantic facts and scratch keep their narrower owners. Both TU
orders, covariance/null results, templates and nested shared graphs are covered.

[Validation124](../student.tests/pa23/validation124.json): earlier PAs
**3811/3811**, **22/22** stages; file audit passes with three inherited warnings;
through PA23 **3835/3856**. New audit controls **21/21**; semantic **26/26**;
completed layout **17/17**; lifecycle **20/20**; inherited **32/33**; all **44**
accepted outputs roundtrip. The two original lifecycle failures, the inherited
member-pointer failure and a deeper lifecycle runtime fault reproduced on both
entry/final binaries stay explicit. Deep ambiguity/resolution checks pass.
The standalone shared-RTTI limit is reproduced with both reference/student IR,
and both pass hosted execution. These are retained observations, not waivers.

[Performance124](performance124.md) freezes 19 correct A/B workloads with A/A
calibration, ABBA wall/RSS, checked runtime and native text. The long shared
forest improves **566→258 ms**, **208,624→54,400 KiB**; native text is identical
on every measured input. Compiler text grows 4,352 bytes; ordinary RSS at most
236 KiB. The 16-level graph falls from **786,358→578** retained views.
All historical/intermediate samples remain preserved. Apply spec §9's
**PA23/O0** acceptance: +15%, +16 MiB and 5.5× historical targets are diagnostics,
not extra gates. Correctness, coverage, mandated complexity/growth and evidence
remain required; native/debug/self-host work stays with its owning stages.

Remaining implementation is grouped by complete owner:

1. **Virtual-base lifecycle and parameter ABI, including oracle reconciliation.**
   Complete/base construction, destruction and transfers; construction tables,
   VTT slices and one-time shared-base actions; by-value hidden pointers across
   declarations, ordinary/conversion/inherited calls, placement-new and cleanup.
   Reference/pointer access continues through the object table with no hidden
   argument. Complete the affected oracles using the retained reduced proof,
   C++11/contract citations and pinned bundle revision. No oracle changed here;
   the earlier one-field RTTI correction was independently confirmed.
2. **Virtual member-function pointer representation.** Formation, static data,
   conversions (including a nonpolymorphic named owner), value proofs and call
   decoding must use a consistent representation and select final overriders.
3. **Full-stage closure.** Preserve every remaining failure/fixture, validate
   these complete groups, and pass the required root through report before PA24.
   Recheck performance on the resulting correct implementation without inventing
   additional exit gates.

The three earlier handoffs exposed real owner boundaries, but repeated partial
publication fixes and sealing were avoidable fragmentation. The next delivery
should span each owner across all its consumers, with TU order and nested-depth
controls together. The audit's single ledger row and complete commit coverage
are in [audit.md](audit.md). Checkpoint acceptance does not certify PA23 completion.
