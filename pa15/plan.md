# PA15 checkpoint plan

Stage base commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`
Last reviewed commit: `538cfcb00441f57c0629f6d27fddbad723539479`

Target: **PA15 full-stage**, O0 typed LowIR. Loop 33 enters at `3aff4801`,
**166/177**, and hands off **169/177**: three existing failures fixed, eight
remain, no coverage reduction. This is an incomplete implementation handoff,
not the independent whole-stage audit or permission to advance to PA16.
The preceding goal turn was progress: committed audit repairs and verified
measurements. Preserve [the accumulated audit](audit.md), its review range,
[historical evidence](../student.tests/pa15/checkpoint-evidence.json), and both
markers above; this turn does not advance the reviewed marker.

## Completed owner and spec alignment

Class-pattern selection now belongs to the primary template's indexed family.
Canonical primary/argument identity survives selection; separate definition and
argument facts drive the selected head's sole environment and retained body.
Argument deduction requires exact substitution back to the actual tuple,
including cv, repeated values and pack boundaries. Pair ordering has a TU-owned
flat cache keyed by the two immutable declaration identities. Redeclaration
renaming preserves its shape; insertion cannot invalidate unrelated pairs.
Selection runs once at concrete completion, after late visible declarations;
explicit specializations retain their existing separate selection path.

Dependent nested template-ids retain their typed argument list. Qualified alias
lookup consumes selected declarations, while abstract pointer/reference casts
use bounded parser lookahead without replay. The shared semantic/source graph,
immutable substitution frames, separate member-body demand and direct typed
LowIR remain intact. No output, reference, bundle or comparison rule changed.

Per completion, work follows this family's C candidates and their argument
shapes: O(C) matching/ordering comparisons, local candidate scratch, no registry
scan. Ordering computes each encountered pair once; storage follows visited
pairs and demanded class facts, released with the TU. Class declarations do not
allocate a discarded primary environment before selecting a partial definition.
Scalar/no-partial paths do no candidate or ordering work. Pack lanes use existing
expansion frames; unrelated member bodies remain dormant.

## Remaining required implementation

| Owner | Unfinished work and boundary |
|---|---|
| Constant execution, initialization and storage | Seven fixtures: constant-object bool conversion, aggregate braced casts, dependent conversion-operator static storage, constexpr local array backing, static constexpr member replay, constexpr call initializers, and stale function initialization output. Constant execution must consume checked bodies/conversions; initializers and storage/emission remain separate facts. |
| Ordinary body validation | One fixture: unused ordinary member `static_assert`. Check ordinary and explicit-class member bodies independently of emission; do not instantiate unrelated template member bodies. |

The dependent-bool fixture's type matching is not its remaining failure: it
requires evaluating `B{}` through a constexpr conversion into the non-type bool
parameter. Its scalar member-binding counterpart works. Extending matching or
forcing layout cannot provide that execution fact. Likewise, ordinary-body
checking needs validation versus emission dependencies, not another class-pattern
lookup. These are concrete distinct owners: carrying them into this increment
would require a constant execution model and a body-demand redesign rather than
further fixes supported by the completed matching work. They remain required
implementation, not review uncertainties. The turn extended beyond the first
three fixtures through nested arguments, packs, member demand, ordering reuse,
cv/value conflicts, late visibility, injected identity and environment ownership.

Independent review remains outstanding for the combined stage changes and
whole-stage spec compliance, including selection/constant-storage interactions
once the unfinished owners are implemented. The new controls and self-review
are evidence for this handoff, not a waiver of that audit.

## Validation, performance and handoff ledger

`make test-pa15`: **169/177**, exit 2; failures **11 -> 8**.
`make test-report-through-pa14`: **1935/1935**, exit 0.
File audit: **pass**, the same three inherited header warnings. Personal suites:
15 matching native + 7 rejection controls; 27 pack native + 8 rejections;
24 specialization native + 13 rejections; 26 value groups; 10 constant groups;
11 checkpoint native + 7 rejections; hash-index collision/removal control.
All run explicitly. The PA15 handout's LowIR validator runs in required tests;
there is no additional current-stage native/debug gate.

[Loop 33 evidence](matching-performance.md) preserves the preliminary and final
frozen A/A/ABBA compiler latency/RSS and runtime/text campaigns, input/output
hashes, counter scaling and the acceptance rationale. PA15/O0 mandates correct,
bounded semantic work and lifetimes; it has no numerical latency/RSS/text ceiling
and introduces no optional optimizer. Later native optimization and self-hosting
retain their owning stages. Historical diagnostic targets and measurements in
[audit performance](audit-performance.md) remain preserved.

| Handoff | Implementation / review boundary | Evidence |
|---|---|---|
| Checkpoint 32 | Reviewed base through `538cfcb0`; matching, constants/storage and ordinary validation unfinished | 166/177; prior 1935/1935; [audit](audit.md) |
| Loop 33 | `9dea2f0a` plan, `57f0cd95` matching/parser, `3c356866` ordering/environment ownership; independent review marker unchanged | 169/177; prior 1935/1935; file audit pass; [verified handoff](../student.tests/pa15/matching-handoff.json); constant/storage and ordinary-body owners remain |
