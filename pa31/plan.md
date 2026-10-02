# PA31 implementation handoff

Stage base commit: `c0566ded7ed123e1eb0696f0ae93b28ceb492fdf`.
Last reviewed commit: `c0566ded7ed123e1eb0696f0ae93b28ceb492fdf`.
Target: **PA31 full-stage**. Phase: **implementation205 complete; audit pending**.
Code commits: `ffea90db`, `d5bd5aee`.

## Design/spec alignment and completed groups

Extend the inherited typed production pipeline, canonical facts and bounded
owners; no hosted-only backend, source replay or textual phase transport.
[Design205](design205.md) records owner, data flow, complexity and validation.

| Group | Owner and completed behavior | Evidence |
|---|---|---|
| Allocation ownership | EntityId → SymbolId → external hosted ABI declaration; standalone role adapters stay in standalone mode. Constant work per demanded symbol. | Two-TU replacement allocator and ostream runtime controls |
| Complete/base entries | Recorded virtual-base count prevents invalid C1/C2 and D1/D2 aliases; distinct demanded entries retain existing ABI facts. O(1) per symbol. | Required ostream inspection; reduced multi-TU layout/entry control |
| Braced temporaries | Canonical exception plan follows its backing conversion without re-entering its own incoming source conversion. Memoized per plan/node, proportional to edges. | Vector smoke; noexcept, throwing elements/destructors and partial cleanup controls |
| Inherited construction | Retain zero-argument inherited candidates when local constructors suppress the implicit default; recognize `using T::T`; validate additional subobjects once per constructor fact without body demand. | Recursive map/unique_ptr smoke; dependent, access, deletion and trait controls |
| Global relocation oracles | Existing native PC32/GOTPCREL output was correct. Four expectation sidecars now use the required ABI spelling `g`. | [Reduced proof and bundle binding](reference-correction205.md); unchanged relocation classes/counts |

## Performance acceptance

[Performance205](performance205.md) retains **544 observations + 32 launcher
samples**, frozen A/B identities, A/A and six ABBA blocks, compiler latency/RSS,
checked runtime and both text sizes. Common/allocation images are identical;
hosted stream text decreases 39 bytes. Equivalent paired ranges cross unity;
no speedup or repeatable avoidable regression is established. New list/inherited
families have proportional counters through N=1024, with final-only costs for
inputs rejected/crashed by A. Largest measured compile is 1.078 s; affected peak
RSS is 71,944 KiB. No optional transform or growth policy is added. Preserve the
45-second hosted limit and existing inline budgets. Inherited 15%/zero-growth
diagnostics do not override spec §9; earlier measurements remain preserved.

## Handoff ledger

- Entry **78/84**, final **84/84**; all six original failures resolved, no fixture
  removed. Four documented spelling corrections preserve coverage/comparisons.
- Final prior-through30 **5,094/5,094**, through31 **5,178/5,178**, and file audit
  pass (four inherited organization warnings). [Bindings and command evidence](../student.tests/pa31/evidence205/binding.json).
- **46** explicit control commands and **69** LowIR/object trace commands pass.
  Reconstructed hosted objects retain native text, named code relocations and
  function-associated CFI; runtime and telemetry-invariance checks pass.
- **Unfinished implementation:** none known in the completed groups or required
  PA31 contract. The full-stage implementation boundary is reached.
- **Independent review:** whole-stage architecture, cumulative inherited
  behavior, reference proof and performance evidence remain for Ralph's audit.
  These questions are not waived; both review markers remain at the stage base.
  This handoff returns control for audit and does not advance to PA32.
