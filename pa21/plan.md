# PA21 compact plan — implementation 112

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `f57bdd3b0e5c7dd2dd87aacaecc73c9ddc96d114`.
Target: **PA21 full-stage**. Phase: **implementation handoff complete; full audit pending**.
Entry: clean `9457e2fca6bdb876eaee509b0481f631b13587b4`, **114/116**.
Previous goal turn: **progress** (committed implementation and evidence).
Current required checks: **116/116 PA21**, **3596/3596 earlier PAs**,
**3712/3712 through PA21**; file audit passes with three inherited warnings.
Both original failures are resolved. All required checks are recorded against
the committed implementation and the frozen measured compiler.

## Completed owners and spec alignment

| Owner | Data flow, complexity and evidence |
|---|---|
| Exhaustive source dispatch | Checked handler types → catch-all fact plus parent context/live prefix → O0 miss-edge emission. A parent summary distinguishes catch-only entries from cleanup-bearing entries. Compute once with O(1) parent access and one additional scan of each try's handlers; no ancestor query, global scan or fixed point. |
| Region and lifetime continuation | Real typed-catch misses retain the existing full live/handler/terminal identities. Exhaustive synthetic misses omit exits only for a parent catch-only entry or no parent; active-handler and cleanup-bearing joins retain balanced exits. Contexts are function-owned and reset between bodies; stable IDs survive vector growth. |
| Constant automatic arrays | The inherited PA16 owner already consumes checked constant initializer plans, interns typed readonly images and copies into distinct automatic storage. PA16 explicitly mandates that form. The old PA21 reference's stores were an inconsistent contract example, not a legitimate reason for a template/name-specific materialization switch. |

Implementation `e32e9f2f` resolves the exhaustive-dispatch mismatch and two
reference defects documented in [reference corrections 112](reference-corrections112.md).
The array correction follows the explicit inherited course rule. The raw
nested-handler correction supplies three explicit exits for three retained
regions, preserving all destructor/end-catch operations and the 106 correction.
It does not claim that the supplied backend rejects the old full fixture: its
shared-entry reconciliation masks the missing exit. The isolated IR reducer
exposes it. Exact reconstruction uses committed input bytes, not student output.

New controls cover scalar/class throws, direct calls, returns, rethrow,
catch-all/typed dispatch, live outer handlers, goto, templates, local class
specializations, distinct mutable arrays and arrays in handlers. **22/22 pass**.
The initial overbroad catch-all omission broke one rethrow control; its
observations are preserved, and the parent dispatch summary repairs it.
No production text transport, semantic name matching, new owning graph,
process-global cache, code cloning or host implementation delegation was added.

## Remaining implementation and independent review

No known required PA21 implementation group remains open. Accumulated controls
and handoff evidence are complete. Required sources/statuses and comparison
rules are unchanged.
The only contract-path changes from entry are the two proved `.ref` revisions.

Independent **whole-stage audit remains required** before PA22. It must review
the accumulated range since `f57bdd3b`, including prior callee-effect proofs,
lifetime/handler identities and the reference-correction chain, as well as the
new parent-summary validity and the distinction between an unreachable selector
edge and a live raw cleanup. The existing [audit](audit.md) and [105 review](audit105.md)
remain authoritative review records; neither marker moves during implementation.
Passing course checks does not waive the independent architecture/spec review.

## Performance and validation evidence

[Performance 112](performance112.md) records frozen A/A and ABBA compiler latency,
peak RSS, checked runtime and native text. Compiler text adds **704 bytes
(0.0317%)**. At 512/2048 handler functions the output removes exactly **one dead
LowIR instruction per function**; native text and semantic/full-expression work
are unchanged. Additional compiler work is O(handlers + contexts), with one
boolean in each existing context record and no output growth. Timing spreads
do not establish a general speedup. The five common workloads produce
byte-identical executables; their runtime noise is retained explicitly.

This is required PA21/O0 dispatch behavior, not an optional runtime transform.
Historical +15%, +16 MiB and 5.5× diagnostic targets add no exit gate; mandated
limits, correctness and coverage remain binding. Preserve [111](performance111.md),
[110](performance110.md) and earlier measurements. Student native optimization,
debug encoding and self-hosting remain later-stage owners.

[Validation 112](../student.tests/pa21/validation112.json) records the required
checks, the exact two-to-zero original failure reduction, 116 unchanged source
identities and the 15048-path contract inventory. Personal suites record **687/690** passes on their original backend lanes,
including **22/22** new controls. The three failures are retained supplied-backend
limits: the previously recorded freestanding RTTI case passes its hosted
counterpart, and two inherited PA16 multi-TU lifecycle cases cannot resolve
`__builtin_abort` through the freestanding backend. Both latter cases have
byte-identical entry/current LowIR with `object=abort`, and both pass through the
supplied object backend and host runtime (**2/2** additional checks). No source,
comparison, failure result or required check is removed to accommodate them.
All five reference-revision scripts and all six new reference observations are
verified. Native execution corroborates the cited proofs; it does not replace
course LowIR comparisons.

## Handoff ledger

102–104: RTTI/casts, captures/copies, lists/demand; **71/116**.
105: audit through `f65eae8d`, record `06b2d989`; 45 failures retained.
106: `9c4f64da` through `c64e88fb`; EH/lifetimes, **92/116**.
107: `b821682b`, `20476a77`; expressions/defaults/consumers, **101/116**.
108: `2e392cec`, `a30acab5`; destination/construction ownership, **108/116**;
evidence through `e6582e37`.
109: accumulated audit through `f57bdd3b`; protected jumps repaired;
eight required comparisons retained; recorded at `d9528e58`.
110: `f724bc43`, `6b3aecdb`, `bdfdb31b`, `df6e8299`; handler/static/list/array
ownership, generated construction and bounded sharing; **113/116**;
evidence `0e5a32dd`, **3596/3596** earlier cases.
111: `3028366d` proves completed scalar-body effects, **114/116**;
final evidence `9457e2fc`, **633/634** controls (one supplied-backend discrepancy).
112: `707b06d7` records entry; `e32e9f2f` completes dispatch and reference
alignment, **116/116**; `e70d2cee` retains frozen performance evidence.
Final evidence records **116/116**, **3712/3712**, passing file audit and the
complete personal/backend results above. The handoff boundary is the completed
PA21 implementation, including the two formerly open owners. Stage base and
last-reviewed commits are unchanged. Return to Ralph for independent full audit;
this handoff does not certify that audit or advance to PA22.
