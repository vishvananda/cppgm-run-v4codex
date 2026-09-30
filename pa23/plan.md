# PA23 compact plan — implementation handoff 122

Target: **PA23 full-stage**. Phase: **incomplete implementation handoff**.
Stage base commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Last reviewed commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Turn entry: `26b96f27`, clean, **17/45**, **28 failures**.
Handoff: **20/45**, **25 failures**; **three original failures resolved, no new
failures or reduced coverage**. Previous turn: implementation/validation progress;
no interrupted build remained live. This turn also made implementation progress.

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

1. **Shared virtual-subobject ownership:** canonical shared identity, nonvirtual
   extents, layout/access, dynamic projections, segment-local vbase/vcall rows,
   virtual diamond RTTI flags, unique final overriders and inherited primary slots.
2. **Virtual-base lifecycle/parameter ABI:** complete/base construction,
   destruction and transfer actions; hidden virtual-base pointers for by-value
   parameters, and object-vtable access for references/pointers. These owners
   account for all **25 remaining required failures**; all fixtures stay required.
3. **Virtual member-function pointers:** the inherited executable still bypasses
   the final overrider. Formation, static data, conversions, value proofs and
   dispatch decoding must agree, including inverse conversion to nonpolymorphic
   owners and variadic forwarding. The proposed expansion was investigated but
   not implemented; an owner-only dispatch patch would be incorrect.

The initial lifecycle scope was extended through exception/deallocation paths,
termination scheduling, nested views and cross-TU ownership. Further related
work now needs the shared-base and member-value representations above; static
segment offsets and nonvirtual lifecycle actions cannot implement them. This
is an implementation boundary, not a waiver or an independent-review question.

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
| Delivery commit | Sealed validation/performance, retained failures and boundary | Pending |

Independent review must assess demand/ABI identities, group layout and cross-TU
publication, cleanup bounds, the reference proof, storage lifetimes, inherited
pipeline alignment and performance evidence. Whole-stage findings must be
resolved before advancement. Neither review marker moved; no review was waived.
This handoff returns control to Ralph without certifying PA23 or advancing to PA24.
