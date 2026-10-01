# PA29 compact plan — implementation179

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `667edd800e4e5eb1b1ef92a3108da3bd96c51708`.
Previous reviewed commit: `7139ceb5769eea12c6e2e00b54932f09d59c79b5`.
Entry HEAD: `cd283a59949c9d56236f6022d018bd3f2057105c`.

## Design and spec alignment

The cumulative pipeline remains streaming source → integrated semantic graph →
typed LowIR → per-function MIR → direct ELF. [Audit178](audit.md) retains the
previous accumulated review and [performance178](performance178.md) its evidence.
This increment completes local class/member and array decomposition: one hidden
object, typed aliases and canonical member shapes, shared initialization,
template/range facts, constant evaluation and normal/unwind destruction. Array
copies emit at most eight leaves or one counted loop with prefix cleanup.

Shared fixes resolve unnamed parameter-array parsing, source-versus-destination
constexpr constructor identity and arithmetic narrowing through reference types.
No source replay, whole-registry retry, textual phase transport, optional
optimizer or host/reference implementation delegation was introduced.

## Validation and performance

PA29 **381 → 385/403**: four original failures removed, **18 remain**, no new
failures. PA1–28 **4538/4538**; through PA29 **4923/4941**. File audit passes
with four inherited warnings. All 403 stage inputs and 1,707 contract/harness
paths are unchanged across entry and review. Personal controls pass **42**;
inspection commands pass **91**. Exact commands, status and source binding are
in [evidence179](../student.tests/pa29/evidence179/manifest.json).

[Performance179](performance179.md) reports all four dimensions at PA29/O0:
224 final A/A+ABBA observations on equivalent inputs, 144 final-only observations
on newly correct decomposition, eight launchers, and all 224 preliminary common
observations retained. Entry rejects the new syntax; no false speedup baseline
is used. Shapes are cached once per canonical type; projections scale with
actual bindings. Optional optimization work/growth budgets remain zero.
Inherited blanket 15% and zero-growth targets remain diagnostic under spec §9;
mandated limits, correctness and coverage remain unchanged.

## Remaining groups and handoff

The [18-case ledger](../student.tests/pa29/evidence179/remaining.json) separates
unfinished implementation from three independent contract questions, all still
counted failures: extended syntax/types/layout **13**, template demand/hosted ABI
**4**, legacy trait **1**. Char-traits conversion remains implementation work.
No reference correction or waiver is claimed. General tuple-protocol and
namespace-scope decomposition were not implemented in this local-binding group.

[Handoff179](handoff179.md) records ownership, data flow, complexity, validations,
repaired interactions and the concrete boundary: remaining cases require
separate type representations, GNU/coroutine rules, inline control flow, library
conversion or ABI policy rather than additional decomposition machinery.
Commits `a0a67bdf`, `bcef72ad`, `bf487a41` form the coherent implementation;
the final record commit returns control for independent review. Full stage and
through-PA29 checks plus whole-stage audit are still required before advancement.
