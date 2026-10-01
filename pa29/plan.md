# PA29 compact plan — implementation180

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `667edd800e4e5eb1b1ef92a3108da3bd96c51708`.
Previous reviewed commit: `7139ceb5769eea12c6e2e00b54932f09d59c79b5`.
Entry HEAD: `259329eec6b08718a059b5198cb32b4f5b5ac17f`.
Code tip: `f613c5549812dd306d6e7a97ead04c944283db22`.

## Design and spec alignment

The cumulative pipeline remains streaming source → integrated semantic graph →
typed LowIR → per-function MIR → direct ELF. This group completes static callable
selection and mandatory inline preparation, extending through static subscripts,
mixed member templates, surrogates, constants, receiver/default effects, typed
ABI boundaries, phi edges, exceptional returns and bounded conservative fallback.
Selected calls retain an evaluated receiver without an implicit-object parameter.
The shared LowIR transform uses typed identity maps and per-callee return-region
facts; it never replays frontend syntax or reconstructs semantics through names.
[Handoff180](handoff180.md) records owners, data flow, complexity and repairs.
[Audit178](audit.md) and the preceding [handoff179](handoff179.md) remain intact.

## Validation and performance

PA29 **385 → 387/403**: two original failures removed, **16 remain**, no new
failures. PA1–28 **4538/4538**; through PA29 **4925/4941**. File audit passes
with four inherited warnings. All 403 stage inputs and 1,707 contract/harness
paths remain unchanged across entry and review. Personal controls pass **47**;
inspection commands pass **251**. Exact commands, source binding and coverage
are in [evidence180](../student.tests/pa29/evidence180/manifest.json).

[Performance180](performance180.md) reports all four dimensions at PA29/O0:
**552 final** observations, **552 preliminary** observations preserved, A/A+ABBA
for equivalent inputs and final-only scaling for newly accepted static calls.
Common executable images are identical. Large straight-line and branching
inline workloads improve runtime; compiler latency/RSS and floating-point text
costs are disclosed. Optional optimization work/growth remains zero. Mandatory
expansion reserves at most 262,144 units/caller and 4,194,304/program, depth 64;
unsafe or over-budget calls remain valid calls. Actual work is bounded by
reservation. Inherited blanket 15% and zero-growth targets remain diagnostic
under spec §9; mandated limits, correctness and coverage are unchanged.

## Remaining groups and handoff

The [16-case ledger](../student.tests/pa29/evidence180/remaining.json) distinguishes
unfinished implementation **13** from independent contract questions **3**, all
still counted failures: extended syntax/types/layout **12**, template demand/
hosted ABI **3**, legacy trait **1**. Char-traits conversion remains implementation
work. There is no reference correction, coverage reduction or waiver.

The completed callable group is validated across declaration, deduction/query,
constant/runtime, ABI, exception and budget boundaries. Remaining failures need
separate extended numeric/vector/complex representations, layout, GNU constants,
deduction-guide/coroutine syntax, library conversion or ABI policy; they do not
consume the changed callable facts or inline remapping. That concrete ownership
boundary makes further related fixes impractical in this group.

Commits `639ebd7f`, `a613b21a`, `95d43769`, `f613c554` implement the group;
`6574d6c8` records entry scope. The final record commit returns control for
independent review. This handoff does not certify the whole assignment. Full
stage/through-PA29 success and resolution of whole-stage audit findings remain
required before advancement; preserved review markers are not advanced here.
