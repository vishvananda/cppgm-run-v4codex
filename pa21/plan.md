# PA21 compact plan — implementation 112 (in progress)

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `f57bdd3b0e5c7dd2dd87aacaecc73c9ddc96d114`.
Target: **PA21 full-stage**. Phase: **implement**.
Entry 112: clean `9457e2fca6bdb876eaee509b0481f631b13587b4`, **114/116**.
Frozen entry compiler: `/tmp/pa21-112/compiler-A`.
Previous goal turn: **progress** (committed ownership fixes and validation).
Current: **114/116**, one original failure resolved; earlier PAs **3596/3596**,
through PA21 **3710/3712**, file audit passes with three inherited header warnings.
All 116 required sources, references and comparison rules remain unchanged in 111.
PA21 remains incomplete; a passing through report is still required to advance.

## Implementation 112 work map

- Constant-array owner: checked initializer plan → typed materialization policy →
  stores or pooled data. Reconcile both inherited contracts using general semantic
  facts; no fixture/name/template switch. Work must stay linear in checked actions
  and emitted bytes. Validate PA17/21 comparisons and explicit array controls.
- Source-handler owner: lexical context plus actual retained LowIR regions →
  ordered cleanup/handler exits → parent dispatch. Trace raw landing pads, catch
  misses, active-handler destruction and catch-all reachability together; preserve
  complete continuation identities. Validate executable nesting/jump controls.
- Freeze A/B and inputs, measure compiler latency/RSS and checked runtime/text
  with A/A and ABBA evidence; run prior-through, PA21, through-PA21 and file audit.
- Independent review remains separate; do not move either review marker.

112 progress: PA16's explicit readonly-copy rule resolves the array policy
conflict; the exact contract correction and raw-region proof are in
[reference corrections 112](reference-corrections112.md). Dispatch lowering
records a constant-time parent summary and preserves balanced cleanup joins
while handling exhaustive selector fallbacks. New composition controls pass
**22/22** after repairing an overbroad initial omission. Required gates and
frozen performance measurements remain in progress; no handoff claim yet.

## Previous completed group and spec alignment

| Owner | Data flow, complexity and validation |
|---|---|
| Semantic function effects | After body/lifetime completion, a sole scalar return with no owned parameter cleanup may prove the implementation nonthrowing. It reuses checked expression/conversion/exception facts, records a positive fact by EntityId, and never changes the declaration's exception specification. One classification per completed body; memoized expression work; no recursive body demand or call-graph fixed point. |
| Full-expression continuation | Two cached views retain declared effects and bounded body proofs. Arguments, defaults, conversions and temporary ownership remain part of the proof. Indirect/virtual calls and unknown allocation/initialization retain conservative cleanup. With no active source handler, a completely proved expression retains its O0 protected region but needs only the resume terminal on its impossible exceptional edge. |
| Lifetime boundaries | Normal destruction and all actually throwing paths retain the same lifetime identities. Constructor/list recipes and owned return/parameter cleanup are not inferred from a callee spelling or reconstructed during emission. Thirteen hosted controls cover declaration `noexcept`, templates, direct/indirect/virtual calls, conversion/allocation failure, throwing argument/temporary/parameter cleanup, and source-handler composition. |

Implementation `3028366d` resolves
`200-initializer-list-backing-array-lifetime`: the scalar accessor's checked body
proves that the `held.size()` edge cannot throw, without omitting cleanup from
other potentially throwing calls. The same mechanism covers ordinary scalar
functions and instantiated templates. No library-name recognition, reference
revision, host implementation dependency, source replay or textual phase transport
was introduced. Body facts and expression caches are TU-owned; full-expression
flags reset at their boundary. Queries use the existing flat identity index.

## Remaining implementation, not waived

- **Automatic constant-array O0 policy** —
  `100-function-template-local-class-specialization-identity` requires stores,
  while PA17 `400-local-value-shadows-template-relational` requires a pooled
  image for the same `long[2] = {0, 0}`. The current typed-image owner preserves
  behavior but fails the former comparison. A template/name/fixture switch is
  not acceptable; reconcile the general materialization policy with both
  contracts. No C++11/LowIR violation proving either reference wrong is asserted.
- **Raw source-handler region continuation** —
  `100-source-nested-catch-miss-cleans-active-handler` executes correctly but
  differs in `eh_end` ordering/count in a raw nested-handler cleanup and on the
  catch-all miss edge. Account for automatic versus explicit region retirement
  before changing those edges. Preserve the proved 106 reference correction and
  complete handler/terminal identities; execution success alone does not settle
  the required LowIR shape.

The handoff boundary is the completed **callee-effect/full-expression proof**
group, expanded beyond the initial list accessor to argument, conversion,
dispatch and lifetime composition. Further related cleanup deletion is
impractical: body-effect facts cannot prove which *source-handler stack entry*
a raw landing pad retains, and cannot choose between the inherited constant
array materialization forms. Those require separate region/materialization
owners, not a broader scalar proof or removal of real cleanup. Both remain
unfinished implementation, not independent-review questions or waived checks.

Independent audit must review the completed proof's immutable publication,
complete expression inputs, conservative dispatch/recipe boundaries and the
measured work/cache bounds. Existing whole-stage audit questions and review
markers remain in force; this handoff does not certify the assignment.

## Performance and validation evidence

[Performance 111](performance111.md) retains frozen A/A and ABBA compiler/RSS and
checked executable runtime/text evidence. Compiler text adds **1920 bytes
(0.086%)**. At 1024/4096 repeated functions, emitted instructions fall by **4 per
function**, native text by **36 bytes per function**, and peak RSS by
**2204/9224 KiB**. An additional memoized proof view costs one byte per TU node
and seven node visits per generated function; semantic exception work adds
18 operations at both sizes. Source graph, checked-body and protected-region
counts are unchanged. Runtime and compiler latency spreads do **not** justify
a general speedup claim. Required O0 output work is distinguished from optional
optimization. Historical +15%, +16 MiB and 5.5× diagnostics add no exit gate;
all mandated limits, correctness and coverage remain binding. Preserve
[110](performance110.md) and all earlier measurements.

[Validation 111](../student.tests/pa21/validation111.json) records required gates,
progress from three failures to two, all **15048** unchanged contract-path hashes,
**633/634** explicit personal controls and reconstruction of every inherited
reference correction. The new controls pass **13/13**. The
existing freestanding RTTI backend discrepancy remains separately disclosed;
its hosted counterpart passes. Native optimization/debug encoding and
self-hosting remain later-stage owners.

## Handoff ledger

102–104: RTTI/casts, captures/copies, lists/demand; **71/116**.
105: audit through `f65eae8d`, record `06b2d989`; 45 failures retained.
106: `9c4f64da` through `c64e88fb`; EH/lifetimes, **92/116**.
107: `b821682b`, `20476a77`; expressions/defaults/consumers, **101/116**.
108: `2e392cec`, `a30acab5`; destination/construction ownership, **108/116**;
evidence through `e6582e37`.
109: [accumulated audit](audit.md) through `f57bdd3b`; protected jumps repaired;
eight required comparisons retained; recorded at `d9528e58`.
110: `f724bc43`, `6b3aecdb`, `bdfdb31b`, `df6e8299`; catch-miss/static/list/array
ownership, generated construction and bounded helper sharing; **113/116**.
Final evidence `0e5a32dd`: **620/621** controls, **3596/3596** earlier cases.
111: `d99f3ec1` records entry/review boundary; `3028366d` proves completed scalar
body effects and resolves the backing-list comparison, **114/116**. Final
evidence records **633/634** controls (one inherited backend discrepancy),
**3596/3596** earlier cases, **114/116** current cases and a passing file audit.
It closes this implementation handoff with the two groups above still open.
Stage base and last-reviewed commits are unchanged.
