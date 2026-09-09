# PA10 compact plan and final audit

Stage base: `c2a4786e`. Independent full-stage review starts at `a5e10cfd`.
Reviewed and retained implementation: `60f7088f`. Candidate `4fc61de6` was
measured and reverted because it was unprofitable. Final evidence is in
[audit.md](audit.md) and [final-audit-performance.md](final-audit-performance.md).

## Spec Alignment and final ownership

| Surface | Final design and scope |
|---|---|
| Source and semantics | Immutable source buffers → streaming token cursor → one parsed graph with canonical TypeId/EntityId/NodeId facts. Indexed lookup, recorded overload/default/conversion decisions. Earlier views remain shared adapters. |
| Expressions and calls | Direct typed PA8 values/slots/operands consume semantic facts; references use by-address signatures, calls recycle operand scratch, indirect signatures cache by TypeId. Volatile readback and discarded-value forms preserve required accesses. |
| Control flow | Semantic initialization-prefix ancestry checks resolve jump targets. One linear entry-path walk retains nested goto/switch destinations; automatic storage is independent of reachable declaration fallthrough. |
| Static initialization | Sealed-TU `(NodeId,target TypeId)` facts memoize active/success/failure states. Every path applies scalar conversion; addresses preserve EntityId/addend through data relocations. |
| ABI and program identity | PA9 typed targets encode ABI names. Program-owned typed linkage keys coalesce external declarations/definitions across TUs and C-linkage namespaces; internal symbols/object labels stay distinct. One emission per definition; collision-safe monotonic display suffixes. |
| Lifetimes and output | TU frontend/semantic maps release after lowering, builders after each function, call live ranges after each call. Minimal shared ABI/linkage summaries and the typed Program remain until explicit LowIR writing. Optional whole-Program validation runs once. |

PA10 is O0 procedural lowering. Classes, template body generation, MIR/native
selection/allocation, ELF, optimization levels and self-hosting remain with the
later owning stages. The inherited template declaration/demand path was traced
and benchmarked here; no template-to-ELF capability is claimed. No optional IR
optimization or generated-code growth pass was added.

## Performance acceptance

The [frozen protocol](../student.tests/pa10/performance-protocol.md) covers
compiler latency/RSS and native runtime/text with fixed inputs, A/A calibration,
two ABBA blocks, equivalence checks, separate telemetry and preserved raw data.
The stage base emits no IR and is not an equivalent performance baseline.
Historical measurements remain intact, including short samples and outliers.
The final delta compares two correct implementations on the same nine compiler
and three long executable workloads. Generated executable equality is checked;
fewer IR nodes are never used as evidence of runtime improvement.

The inherited 1.10x wall, 1.20x RSS +16 MiB, +128 KiB compiler-text and input-growth
budgets are diagnostic targets, not PA10 exit gates. PA9 naming-tool limits
remain scoped to the unchanged naming tool. Mandated complexity, behavior,
coverage, comparison rules and owning-stage constraints remain unchanged.

## Ledger and remaining work

- Entry: base PA10 0/121; implementation progressed 75 → 119/121 with controls
  passing and earlier stages 904/904.
- Reference review `55b33a44`: two proven constant-initialization corrections;
  sources, statuses, coverage and comparison unchanged. The independent audit
  reran both pinned-reference reducers and reviewed the standard/contract proof.
- Ownership checkpoint `3af0da70`: constant caching, typed validation, signatures,
  scratch ownership, volatile discard, bool storage, padding and ABI separation.
- Sequencing handoff `63592ef0`: computed compound LHS evaluated once after RHS.
  All subsequent benchmark/protocol handoffs through `a5e10cfd` are reviewed.
- Independent audit `60f7088f`: nested control entries, condition barriers,
  skipped local declarations, static conversions/addresses, volatile readback,
  multi-file linkage and collision-safe identities fixed across their owners.
- Performance follow-up `4fc61de6`: attempted display-construction reduction
  saves 640 text bytes but slows the affected workload by 2.1–5.0% in paired
  blocks. Candidate reverted; the final binary exactly matches frozen `60f7088f`.
  Both complete campaigns, outliers and historical observations are preserved.
- Final exit: see audit for exact command results, native/sanitizer/IR checks,
  work/growth evidence, reference provenance and final performance assessment.
  No PA10 implementation group or unaudited handoff remains. Both required exit
  commands pass; final documentation and evidence are committed with a clean tree.
