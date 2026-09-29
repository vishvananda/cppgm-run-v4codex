# PA20 implementation plan — handoff 94 in progress

Stage base commit: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Last reviewed commit: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Target: **PA20 full-stage**. Phase: **implement**.
Entry: **73/144 pass, 71 failures**; PA1–19 and file audit pass.

## Design and remaining groups

| Owner | Data flow / complexity | Validation |
|---|---|---|
| Placeholder deduction | Source declarator + checked initializer/returns → canonical type → recorded conversions; one body check and O(returns + type shape) deduction | Ordinary/member/template returns, pointer/reference/cv/collapse, recursive/conflicting returns, condition declarations |
| Initialization / conversion | Checked operands → typed aggregate/list/conversion plans → direct LowIR; linear in required subobjects, indexed type/plan identity | Scalar/array/class braces, omitted and nested elements, functional construction, reference conversions and increment |
| Range statements (unfinished) | Range fact owns single evaluation, begin/end selection, iterator operations, iteration variable and lifetimes; typed operations must feed lowering without fake syntax | Arrays/lists/member/ADL/inherited ranges; references, prvalue lifetime, template composition |
| Closures (unfinished) | Source occurrence + enclosing specialization → closure identity, captures/call/conversion/ABI facts → helper emission | Captures including nested this/local, return deduction, defaults, special members, local statics, helper shape |

Start with the related deduction/initialization owners; extend through shared
semantics while the same investigation supports progress. Existing typed graph,
canonical IDs, scoped fact caches and direct LowIR remain the production path.
No reference or coverage change is planned; any proved oracle defect needs its
own reducer and contract/standard proof.

## Performance and handoff ledger

Freeze entry binary `/tmp/pa20-entry-cppgm` (SHA256
`1a181c95ee97d646d41c1009780200049e776d67e1d715840d5526b3d10e1d40`).
Measure fixed equivalent workloads with A/A and ABBA blocks; retain compiler
latency/RSS, checked native runtime and text sizes. New correctness costs are
reported separately from equivalent workloads. PA20/O0 adds no optional optimizer;
native backend and optimized policies remain later-stage owners. Diagnostic
targets do not supersede spec §9 stage-scoped acceptance.

Implementation handoff pending. Independent review pending for all stage edits;
the review marker is preserved. Required exit: stage failure reduction without
coverage loss, through-PA19, file audit, explicit personal controls, committed
clean tree. Full through-PA20 remains required before stage advancement.
