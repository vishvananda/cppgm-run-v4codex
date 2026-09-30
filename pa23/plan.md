# PA23 compact plan — implementation handoff 123

Target: **PA23 full-stage**. Phase: **incomplete implementation handoff**.
Stage base commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Last reviewed commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Turn entry: `9688f9e54a2b206b9add41ce78acd9b5f0452c62`, clean, **20/45**,
**25 failures**. Handoff: **24/45**, **21 failures**; four original failures
resolved, no new failures and no coverage reduction. Previous turn and this
turn made verified implementation progress; no interrupted build remains live.

## Completed group and spec alignment

[Handoff 123](handoff123.md) completes shared-subobject identity/final overriders,
complete-object layout, segment-local table facts, RTTI and dynamic projections.
Canonical virtual anchors/nonvirtual paths own base identity. Separate
nonvirtual extents place shared bases once; per-segment rows/address points feed
typed lowering. Inherited overrides retain required primary slots. Covariant
return thunks consume dynamic result projections with null preservation and
complete typed cache keys. C++ polymorphism stays separate from needing a
virtual-base layout pointer.

Work follows imported base/slot edges, receiver paths and emitted rows. Flat
identity indexes live in the TU; receiver-containment facts live for one class
completion. Candidate selection/verification and publication are linear scans;
repeated receiver relations reuse their completed facts. No grammar replay,
text transport, source-body cloning, global retry/invalidation or optimizer was
added. [Handoff 121](handoff121.md) and [handoff 122](handoff122.md) retain the
ordinary-view/lifecycle ownership history and earlier reference correction.

## Remaining implementation — requirements retained

1. **Virtual-base lifecycle/parameter ABI:** complete/base construction,
   destruction and transfer actions; construction tables and VTT slices;
   by-value hidden pointers across declarations, ordinary/conversion/inherited
   calls, placement-new and cleanup paths. Reference/pointer accesses now use
   the object table and acquire no hidden ABI argument.
2. **Reference reconciliation:** [the reduced proof](reference-observation123.md)
   shows the pinned reference reading a sibling field through a nonpolymorphic
   virtual-base reference. Full affected oracles need correction with completed
   lifecycle output. No fixture, oracle, status or comparator changed this turn.
3. **Virtual member-function pointers:** inherited formation, conversions,
   value proofs and dispatch decoding still bypass final overriders. The
   preserved failing control remains implementation work.

The scope extended through layout, RTTI, table demand, dynamic access,
inherited primary slots, covariance and reuse of receiver-containment facts.
Two explicit lifecycle probes still fail: duplicate shared-base construction
and a base constructor overwriting a sibling vptr. Further work requires one
consistent ABI/action representation across entry-local parameter binding,
construction-table selection and all forwarding/cleanup paths. Static C2 calls
cannot consume the new complete-object layout facts correctly. This concrete
owner boundary explains the incomplete handoff; it does not waive any failure.

## Performance and validation

[Performance 123](performance123.md) retains frozen A/B binaries/inputs, A/A
calibration, ABBA compiler wall/RSS and checked runtime/text across 11 workloads; a focused repeat retains compiler
latency uncertainty.
Common native text is unchanged. Required dynamic-access workloads cost +20 and
+62 text bytes with disclosed runtime increases. Compiler text grows 15,360
bytes; comparable peak RSS rises at most 264 KiB. New-capability scaling has
linear work/storage counters. No optimization or speedup is claimed.
[Performance 121](performance121.md), [performance 122](performance122.md) and
all intermediate measurement series remain preserved.

Apply spec §9's **PA23/O0** acceptance. Historical +15%, +16 MiB and 5.5× targets
remain diagnostics, not extra gates. Correctness, coverage, mandated complexity
and growth limits remain required; later native/debug/self-host work stays with
its owning stages.

[Sealed validation](../student.tests/pa23/validation123.json): `make test-pa23`
**24/45**; earlier through report **3811/3811**, **22/22** stages; through PA23
**3835/3856**; file audit passes with three inherited warnings. Semantic controls
**26/26**, completed layout/dispatch controls **17/17**, prior lifecycle controls
**20/20**, inherited controls **32/33**. The two new lifecycle failures and the
inherited member-pointer failure remain explicit. All **44** accepted stage
outputs roundtrip stably. Native observations compile our LowIR with the
supplied backend; the source frontend does not delegate implementation. The
standalone backend passes **16/17** completed controls; its shared-base RTTI
scan failure is reproduced with reference IR and both IRs pass hosted execution
([probe](../student.tests/pa23/backend-limit123.json)).

## Handoff ledger and independent review

| Commit/group | Implementation disposition | Independent review |
|---|---|---|
| `b28e12ce` | Original stage/review markers recorded before edits | Pending |
| Handoff 121: `b1aed715` through `26b96f27` | Ordinary views, dispatch/covariance and RTTI | Pending |
| Handoff 122: `b2010915`, `9417f88f`, `8b505a75`, `3b9cf2b5`, `a04a8cae`, `9688f9e5` | Lifecycle entries, key ownership, TU publication, RTTI oracle proof and evidence | Pending |
| `2f27c0e5` | Frozen turn-123 baseline and owner/data-flow plan | Pending |
| `c7944d23` | Canonical subobjects, final overriders and semantic controls | Pending |
| `67354b79` | Shared extents, table rows, RTTI, projections and covariance | Pending |
| `08cb8657` | Completion-local reuse of receiver-containment facts | Pending |
| `6a762f25` | Correct virtual-path RTTI hints and nonpolymorphic table entries | Pending |
| Delivery commit | Sealed checks/evidence, reference observation and retained boundary | Pending |

Independent review must assess identity/cache completeness, demand and ABI
boundaries, layout/table ownership, RTTI, thunk keys, storage lifetimes,
inherited pipeline alignment and performance evidence. Whole-stage findings
must be resolved before advancement. Neither review marker moved and no review
was waived. This handoff returns control to Ralph without certifying PA23 or
advancing to PA24.
