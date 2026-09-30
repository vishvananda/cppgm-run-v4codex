# PA23 compact plan — implementation 123 in progress

Target: **PA23 full-stage**. Phase: **implement**.
Stage base commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Last reviewed commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Turn entry: `9688f9e54a2b206b9add41ce78acd9b5f0452c62`, clean,
**20/45**, **25 failures**. Previous turn: verified implementation progress;
no prior build process is live. Entry binary and failure log frozen in
`/tmp/pa23-123/`. Stage/review markers above are unchanged.

## Current completed group

**24/45**, **21 failures**, four original failures resolved with no new failures.
Class-owned canonical virtual anchors/nonvirtual paths now determine base
conversion identity and unique final overriders. Separate nonvirtual extents
feed shared complete-object layout, segment-local vbase/vcall rows, address
points, RTTI flags and dynamic projections. Inherited overrides retain required
primary slots. Covariant result thunks consume dynamic return projections with
null preservation and complete typed cache keys. Language polymorphism remains
separate from having a virtual-base layout pointer.

Explicit semantic controls pass **25/25**; executable layout/dispatch controls
pass **17/17**. Two separately recorded lifecycle probes still fail: repeated
construction of one shared base and a base constructor clobbering a sibling
vptr. They require complete/base action separation, construction-table slices
and hidden-pointer signatures across all call paths. These are unfinished
implementation, not review questions. Final validation/performance is pending.

## Completed ownership and spec alignment

[Handoff 121](handoff121.md) completed ordinary nonvirtual views/dispatch/RTTI.
[Handoff 122](handoff122.md) completes the remaining **nonvirtual polymorphic
lifecycle/table ownership** group: external key references versus local table
requirements, recorded group address points, direct destructor declarations,
base-only ABI entries, bounded two-action deleting entries, termination-adapter
scheduling, and separate/merged TU publication in both orders.

Class/member identities own demand; immutable slot slices and layout facts feed
typed lowering. References do not demand external virtual bodies/RTTI. A key
notification wakes its table once; a later TU upgrades the same typed global.
Work is O(views + emitted rows/actions), with indexed identity lookup and TU or
emission-local storage. No grammar replay, text phase transport, source-body
clone, global invalidation or new optimizer was added.

A [one-field reference correction](reference-correction122.md) changes the
nonvirtual destructor diamond's VMI flags from 0 to 1. The reducer, N3485 rules,
ABI proof and observed bundle revision are documented. Fixtures, exit statuses,
bodies, slots and comparison rules remain unchanged.

## Remaining implementation — requirements retained

1. **Virtual-base lifecycle/parameter ABI:** complete/base construction,
   destruction and transfer actions; construction tables and VTT slices;
   by-value hidden pointers across declarations/call paths. Reference/pointer
   accesses now use the object's table and acquire no hidden argument.
2. **Reference reconciliation:** nonpolymorphic virtual-reference output uses
   a fixed complete-type offset. A deterministic reducer fails in the pinned
   reference. Correct affected oracles only with the recorded standard/contract
   proof and completed lifecycle behavior; no oracle has changed in turn 123.
3. **Virtual member-function pointers:** inherited formation/conversion/value
   proofs and call decoding still bypass final overriders; the preserved control
   remains required implementation work.

The initial semantic scope extended through layout, RTTI, table demand, dynamic
access, inherited primary slots and covariant returns. Construction/destruction
and value transfer now need a distinct ABI-action owner, including entry-local
parameter binding, construction-table selection and exception cleanups. Existing
static lifecycle calls cannot consume complete-object projections correctly.

## Performance and validation

[Performance 122](performance122.md) freezes entry/final binaries and inputs,
retains A/A and ABBA compiler wall/RSS and checked runtime/text over 13 inputs,
and adds longer eight-block repeats for template/deletion noise. The small D0
policy is capped at two actions; larger cleanup suffixes remain linear. The
final long deletion repeat is noisy (paired median 1.0004, range 0.805–1.164),
so no speedup is claimed. Required O0 shape costs +146 executable text bytes;
compiler text grows 4,864 bytes. All samples, spread and work counts are retained.
[Performance 121](performance121.md) remains historical evidence.

Apply spec §9's **PA23/O0** acceptance. Inherited +15%, +16 MiB and 5.5× targets
remain diagnostics, not extra gates. Correctness, mandated work/growth limits
and coverage remain mandatory. Later native optimizer/debug/self-host evidence
belongs to those stages.

[Sealed validation](../student.tests/pa23/validation122.json): required
`make test-pa23` **20/45**; earlier through report **3811/3811**, **22/22** stages;
through PA23 **3831/3856**; file audit passes with three inherited warnings.
Explicit lifecycle controls **20/20**; inherited controls **32/33**, with the
member-pointer defect preserved above. All **41** accepted outputs roundtrip
stably. Standalone lifecycle checks **19/20**: one supplied-backend duplicate
object-label limit is reproduced with reference IR; both IRs pass hosted execution.

## Handoff ledger and independent review

| Commit/group | Implementation disposition | Independent review |
|---|---|---|
| `b28e12ce` | Original stage/review markers recorded before edits | Pending |
| Handoff 121: `b1aed715` through `26b96f27` | Ordinary views, dispatch/covariance and RTTI; history in handoff 121 | Pending |
| `b2010915` | Turn 122 baseline and owner plan | Pending |
| `9417f88f` | Lifecycle entries, key ownership and documented RTTI oracle correction | Pending |
| `8b505a75` | Merged-TU definition publication, precise telemetry and controls | Pending |
| `3b9cf2b5` | Single base-only entry identity; covariant secondary group check | Pending |
| `a04a8cae` | Typed base-alias deduplication and prior-TU schedule preservation | Pending |
| `9688f9e5` | Handoff 122 validation/performance, retained failures and boundary | Pending |
| `2f27c0e5` | Turn 123 frozen baseline and shared-base owner/data-flow plan | Pending |
| `c7944d23` | Canonical subobjects, final overriders and semantic controls | Pending |
| Layout implementation commit | Shared extents, table rows, RTTI, projections and covariant returns | Pending |

Independent review must assess demand/ABI identities, group layout and cross-TU
publication, cleanup bounds, the reference proof, storage lifetimes, inherited
pipeline alignment and performance evidence. Whole-stage findings must be
resolved before advancement. Neither review marker moved; no review was waived.
This handoff returns control to Ralph without certifying PA23 or advancing to PA24.
