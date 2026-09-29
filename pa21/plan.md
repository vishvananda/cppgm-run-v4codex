# PA21 compact plan — implementation 111

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `f57bdd3b0e5c7dd2dd87aacaecc73c9ddc96d114`.
Target: **PA21 full-stage**. Phase: **implementation**.
Entry 111: clean `0e5a32ddec4027d67d38aa98263f964b383dbed2`, **113/116**.
Previous goal turn: progress (committed ownership fixes and validation).
Current work: resolve remaining EH region retirement and backing-list continuation
under the shared lifetime/exception-context owner; then constant-array O0 policy.
Trace semantic lifetime IDs through region entry/exit and typed cleanup emission;
preserve expected O(1) continuation keys and work proportional to emitted actions.
Validate required comparisons, hosted nested-handler/list controls, all earlier
PAs, file audit, and frozen A/B compiler/RSS plus runtime/text measurements.
Stage/review markers above are preserved from the existing implementation ledger.
Current: **113/116**, five original failures resolved; earlier PAs **3596/3596**,
through PA21 **3709/3712**, file audit passes. All 116 sources and comparison
rules remain intact. PA21 completion still requires a passing through report.

## Design/spec alignment and completed group

| Owner | Data flow, bounds and validation |
|---|---|
| EH contexts/support | Canonical TypeIds render stable fundamental support names. Lexical lifetime snapshots and handler ancestry retain cleanup on typed catch misses. Complete context/terminal identities still key shared suffixes. Hosted nested misses, rethrows and caught-object ordering controls exercise these paths. |
| Array/list destruction | Construction retains exact addresses for at most eight backing elements, keyed by the emitted backing ValueId and released per function. Throwing destruction decrements a remaining-prefix counter before each call; a second exception terminates through the existing cleanup boundary. Above eight elements, counted loops bound emitted growth. Controls cover ordinary/list arrays, first/last throws and 1/3/8/9/16 elements. |
| Local statics | The initializer owns its expression boundary. A successfully completed object publishes its guard/finalizer before observable initializer-temporary cleanup; a failed constructor remains retryable. Controls check construction retry and completion before throwing temporary cleanup. |
| Generated construction | Canonical initializer actions distinguish omitted in-place members from arguments already supplied by the caller. Omitted recipes with argument state own their helper key, exception effects and temporary cleanup; zero-argument constructor calls share by canonical type and supplied-prefix arity. Required empty copy entries override cached representation-transfer eligibility. Proven scalar representation prefixes retain the required O0 form; transfer cleanup stops at the final action unless temporaries remain. |
| Function boundary | Nonthrowing functions retain normal parameter destruction and inner source catches; the uncaught unwind suffix stops at incoming parameters before termination. No parameter ABI or lifetime fact is reconstructed from text. |

Work is proportional to demanded facts and emitted actions, with expected O(1)
ID lookups and existing O(b log b) final block ordering. There is no new source
reparse, textual phase transport, host implementation dependency or unbounded
optimization. New construction and closure controls pass **40/40**; closure
controls improve **16/27 → 27/27**. Full validation is recorded in
[validation110](../student.tests/pa21/validation110.json).

The only reference revision is the active-handler continuation in
`400-handler-context-cleanup-continuation.ref`: argument cleanup precedes
caught-object retirement. The [proof](reference-corrections110.md), reduced
hosted execution, exact reconstruction and bundle manifest preserve provenance.
Original/revised reduced execution returns **1/0**. No other reference, test
source, harness or comparison rule changed in this implementation.

## Remaining implementation, not waived

- **Automatic constant-array policy:**
  `100-function-template-local-class-specialization-identity` expects element
  stores; current typed constant images/copyobj preserve the program but differ
  structurally. PA17 requires pooling for an ordinary array with the same type,
  extent and values. Resolve the general O0 policy without a template-presence,
  name or fixture gate; preserve the typed constant owner and bounded work.
- **Raw EH region continuation:**
  `100-source-nested-catch-miss-cleans-active-handler` now executes correctly;
  its residual comparison concerns the order/number of `eh_end` in raw handler
  cleanup and the catch-all miss edge. Account for automatic versus explicit
  region retirement before changing those edges. The prior 106 reference repair
  remains preserved; execution agreement does not settle the required shape.
- **Backing-list lifetime at a call:**
  `200-initializer-list-backing-array-lifetime` differs only in its cleanup edge
  around `held.size()`. Current lowering retains the two live backing elements;
  the reference resumes directly. The accessor body is nonthrowing in this
  fixture but its declaration has no `noexcept`. Resolve O0 proof/continuation
  policy without suppressing cleanup for genuinely throwing calls. No new
  reference correction has been asserted for this dormant path.

These are unfinished required comparisons, distinct from independent audit
questions about the completed implementation (reference-110 proof, helper/cache
identities, and handler/array/static lifetime ownership). Neither is waived.
The implementation boundary is the completed construction/destruction ownership
group, extended across static storage, helpers and templates. A further local
cleanup deletion would not resolve the three remaining policies safely: one
conflicts with an inherited pooling contract, and two need a separate proof of
O0 region/proven-call conventions on dormant paths. No fixture-shaped exception
or unproved reference rewrite is used to manufacture completion.

## Performance and validation

Current required gate logs are in validation110. Cumulative personal controls
pass **620/621**; the only failure is the inherited supplied freestanding RTTI
discrepancy, whose hosted counterpart passes. The **15048-path** contract
inventory changes only the proved reference above. The
[frozen performance campaign](performance110.md) records A/A and ABBA compiler/RSS and
checked executable runtime/text. Compiler text adds **8064 bytes (0.364%)**;
zero-argument helpers share one entry (1024-occurrence host text **57885 →
16965 bytes** from the intermediate implementation). Required copy/helper call
costs and normal-path-only list observations are disclosed. Larger throwing
arrays retain **148 instructions** at extents 9/64/1024. Preserve
[performance109](performance109.md) and all earlier observations. Spec §9 applies
to PA21/O0: historical **+15%, +16 MiB, 5.5×** diagnostics add no exit gate.
Required O0 costs must be measured separately from optimization benefits; no
performance improvement is claimed from the semantic repairs. The eight-element
expansion bound, correctness, coverage and comparison requirements remain binding.
Native small-copy selection, optimization/debug encoding and self-hosting remain
later-stage owners.

## Handoff ledger

102–104: RTTI/casts, captures/copies, lists/demand; **71/116**.
105: audit through `f65eae8d`, record `06b2d989`; 45 failures retained.
106: `9c4f64da` through `c64e88fb`; EH/lifetimes, **92/116**.
107: `b821682b`, `20476a77`; expressions/defaults/consumers, **101/116**.
108: `2e392cec`, `a30acab5`; destination/construction ownership, **108/116**;
evidence through `e6582e37`.
109: [whole accumulated audit](audit.md) through `f57bdd3b`; protected jumps
repaired; eight required comparisons unchanged; recorded at `d9528e58`.
110: `f724bc43` preserves review boundary; `6b3aecdb` completes catch-miss,
static/list/array ownership and proved reference correction, **111/116**.
`bdfdb31b` completes generated construction ownership, **113/116**.
`df6e8299` removes measured 256/1024 duplicate-helper growth for zero-argument
constructor recipes; argument-bearing recipes remain occurrence-owned. Five
additional prvalue controls pass. Final evidence records **620/621** personal
controls (one inherited backend limitation), **113/116** required cases,
**3596/3596** earlier cases and a passing file audit. The evidence commit closes
this implementation handoff; PA21 remains incomplete and returns to Ralph for
the three groups above. Review markers remain unchanged.
