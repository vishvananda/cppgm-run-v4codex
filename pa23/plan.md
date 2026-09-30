# PA23 implementation 121

Target: **PA23 full-stage**. Phase: **implementation**.
Stage base commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Last reviewed commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Entry: clean; required suite **3/45**, **42 failures** (Ralph primary log).
Previous interrupted turn: no live compiler/build process; current inspection
establishes the baseline and owners rather than assuming prior work continued.

## Design and work groups

1. **Nonvirtual polymorphic subobjects**: semantic class completion owns primary
   base, slot identities and final overriders; layout owns base displacements;
   conversion facts feed signed/null-preserving projections. Completed view
   records feed direct typed vtables, receiver thunks, lifecycle stores and RTTI.
   Visit relevant base edges/slots, cache per completed class, emit each view and
   thunk once. No name-based semantic recovery or text transport.
2. **Shared virtual bases**: canonical shared subobject identity, nonvirtual size,
   complete layout and per-view negative rows; dynamic projection through those
   rows. Extend the same facts into inherited dispatch and final-overrider checks.
3. **Virtual-base lifecycle/parameters**: typed complete/base ABI entries and
   hidden pointers only for by-value parameters; forwarding, construction,
   transfers and source inline policy. Preserve reference/pointer ABI.

Work extends across related groups while the established ownership supports it.
The full stage remains required; a partial handoff must identify a concrete
completed group and the different ownership preventing further related work.

## Validation and performance

Run focused contract diffs and explicit personal runtime/rejection controls,
then `make test-pa23`, `make test-report-through-pa22` (through-pa23 if green),
and `perl scripts/cppgm_file_audit.pl --stage pa23 --paths dev/src`.
Keep all 45 contract fixtures/comparison rules. Reference changes require a
reducer and standard/contract proof; none planned.

Freeze entry/final binaries and workloads; A/A plus ABBA compiler wall/RSS and
separate executable runtime/text measurements. Compare equivalent correct
outputs; newly supported semantics have necessary costs, not speed claims.
PA23/O0 introduces no optional optimization; no additional performance gate
beyond spec's mandated complexity and stage-scoped acceptance. Earlier
measurements and explicit work budgets remain preserved.

## Handoff ledger

Implementation 121: in progress; no handoff yet. `b1aed715` completes the first
nonvirtual view implementation: **17/45** required tests, **29/29** explicit
personal controls, and fresh prior-through report **3811/3811**. It includes
covariant result adjustment, null handling, repeated source downcasts, private
base crosscasts and later-base override rejection. Slot arena refinement and
performance evidence are still in progress. A simultaneous root-test invocation
collided in wrapper relinking; serial retries establish the results above. All stage edits after the last
reviewed commit await independent review. Unfinished implementation is listed
above; independent review must check view/slot ownership, inherited architecture,
performance evidence and whole-stage scope without waiving remaining work.
