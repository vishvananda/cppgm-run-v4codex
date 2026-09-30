# PA23 compact plan — implementation 122

Target: **PA23 full-stage**. Phase: **implement**.
Stage base commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Last reviewed commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Turn 122 entry: `26b96f27e05f82f3cde2e6bf6877288c596f3d06`, clean,
**17/45**, **28 failures**. The previous turn made implementation and validation
progress (handoff 121); no interrupted build is live. Both review markers above
remain unchanged. Entry binary frozen at `/tmp/pa23-122/entry`.

## Current implementation groups

- Lifecycle/vtable owner: finish nonvirtual destructor entry policy and external
  key-function table demand. Class completion publishes entry/view identity;
  lowering consumes segment offsets, with bounded cleanup expansion. Validate
  both remaining nonvirtual fixtures, inherited suites and executable deletion.
- Member-value owner: implement virtual member-function target formation, static
  data, conversion facts and dispatch decoding together. Canonical member IDs
  select tagged slots; lowering must preserve `this` adjustment even after inverse
  conversion to a nonpolymorphic owner. O(1) formation/decoding; any value proof
  stays bounded and falls back conservatively. Validate runtime, null/equality,
  indirect and immediate calls, and PA22 shape preservation.
- Continue into shared virtual-subobject ownership where the resulting facts
  support it; do not call the stage complete on progress alone. Performance
  compares frozen binaries with A/A and ABBA latency/RSS and checked runtime/text;
  PA23/O0 acceptance applies, with no additional self-imposed speed gate.

## Completed group and spec alignment

Ordinary virtual calls over **nonvirtual polymorphic subobjects** now share
semantic primary-base selection, final-overrider identities and completed
layout/view facts. Indexed local signatures preserve unrelated roots. Class
slot arenas provide allocation-free view slices; lowering emits typed tables,
receiver/covariant-result thunks, constructor vptr stores and all direct-base
RTTI descriptors. Signed/null-preserving casts and the non-null `this` fact feed
those same conversions. Typeid, sibling crosscasts and repeated-source downcasts
consume the selected view; private base relationships do not suppress a valid
complete-object crosscast.

RTTI facts have explicit deduplicated TU demand from expressions, tables and
exceptions. Completion computes flags once before lowering. Primary-only chains
have no ancestor-view expansion. New caches use canonical IDs and integer pairs;
no text transport, cloned source bodies or global retry/invalidation was added.
The [handoff detail](handoff121.md) records ownership, data flow, complexity,
language rationale and the concrete implementation boundary.

## Remaining implementation — requirements retained

1. Shared virtual-base subobject identities and layout, nonvirtual extents,
   dynamic projection, segment-local vbase/vcall rows, diamond RTTI flags and
   unique-final-overrider checks.
2. Virtual-base constructor/transfer/destructor complete/base entries and hidden
   pointer forwarding for by-value parameters; reference/pointer ABI uses the
   object's vtable. Also finish the two nonvirtual virtual-destructor lifecycle
   shape failures. All 45 fixtures remain required.
3. Virtual member-function pointer representation and decoding. The retained
   personal `virtual-member-pointer` reproducer calls the base function's raw
   address instead of dispatching to the final overrider. Fix formation,
   serialization, conversion/value facts and calls together, including inverse
   conversions to nonpolymorphic owners; a local vtable fix is insufficient.

The completed ordinary-call/view/RTTI group ends here. Continuing requires new
shared-subobject, lifecycle-argument and member-value representations across
other semantic owners. These are unfinished implementation, not waived cases
or questions deferred to independent review.

## Performance and validation

[Performance evidence](performance121.md) freezes entry/correct-view/final
binaries and sources, records A/A calibration and four ABBA blocks, and reports
compiler wall/RSS plus checked runtime/text across 13 inputs. Earlier pilot and
pre-seal measurements remain preserved. New view work tracks produced slots;
RTTI work tracks demanded reachable base edges. Each table/thunk emits once.
The `this` fact costs O(1), adds no IR and introduces no iterative optimizer.

Apply spec §9's PA23/O0 acceptance. Inherited +15%, +16 MiB and 5.5× diagnostics
remain self-selected measurements, not additional exit gates. Correctness,
coverage, mandated work/growth limits and earlier measurements are unchanged.
Native optimizer/debug and self-host acceptance remain with their owning stages.

[Sealed validation](../student.tests/pa23/validation121.json): `make test-pa23`
**17/45**; `make test-report-through-pa22` **3811/3811**, **22/22 stages**;
required file audit passes with three inherited header-organization warnings.
All **41** accepted stage outputs roundtrip stably. Explicit personal controls
are **32/33**; the one known failure is the unfinished member-pointer group
above. Standalone backend checks are **25/29**: the same member-pointer defect
and three reproduced reference-backend limitations are preserved in the
handoff detail. No reference correction, fixture or comparison change was made.

## Handoff ledger and independent review

| Commit | Implementation disposition | Independent review |
|---|---|---|
| `b28e12ce` | Stage base, immutable review marker and owner plan recorded before edits | Pending |
| `b1aed715` | Ordinary nonvirtual views, dispatch thunks, covariance and RTTI behavior | Pending |
| `55e2053a` | Contiguous slot arenas and precise RTTI demand facts | Pending |
| `72433afa` | Reacquire slot identities after outgoing covariant result-layout demand | Pending |
| `6ae06461` | Distinct private object names and the pure-virtual runtime role for standalone execution | Pending |
| Delivery commit | Seals plan, handoff boundary, validation and performance artifacts | Pending |

Independent review must assess canonical subobject/slot identities, thunk ABI,
precise demand and allocation lifetimes, inherited pipeline alignment and the
performance protocol. It must resolve whole-stage findings before advancement.
No independent review was performed or waived in this implementation turn;
both review markers remain at the original stage base. The handoff returns
control to Ralph without certifying PA23 or advancing to PA24.
