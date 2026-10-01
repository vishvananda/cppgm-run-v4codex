# PA29 implementation / handoff157

Target: **PA29 full-stage**. Implementation handoff complete; stage unfinished.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Turn entry: `0bad8c207712b2077afc78f209a469c55b07847b`, clean, **301/403**.
Result: **316/403**; 15 original failures resolved, zero new failures, 87 remain.
Review markers stay unchanged; this is progress, not whole-stage approval.

## Design/spec alignment

Prior hosted driver, numeric/type queries, runtime signatures, scalar operations
and exact floating storage remain in place; their evidence is preserved in
[handoff155](performance155.md) and [handoff156](performance156.md).

| Completed owner | Data flow / complexity | Validation |
|---|---|---|
| Attribute parser / prediction | Balanced token lookahead recognizes attributed declarations; alignment operands parse once into the source graph. Namespace/pointer annotations and `__thread` spelling use existing declaration semantics. O(tokens + operands). | Condition/for/range, namespace placement, nullability, unknown attribute arguments, hosted system headers |
| Alignment / raw type storage | GNU minimum alignment and strict `alignas` remain distinct. Typedef storage decorations and dependent query operands survive substitution; canonical identity/ABI signatures erase decorations. Sparse field facts carry alignment to layout and object emission. Work follows attributes and demanded type/frame keys. | Dependent classes/typedefs, increased/decreased alignment, stack/global/member/array storage, identity and host ABI, invalid constants/arrays |
| Empty subobject layout / initialization | Exact `(type, offset)` summaries cap at 64 positions; larger shapes use address-directed graph queries. Arrays stay compact; repeated placements use monotone frontiers. Recorded overlap permissions reach zero initialization, generated transfers and typed lowering without erasing constructor/destructor effects. | Same-type/nested/repeated objects, nonzero nested addresses, large footprint fallback, million-element arrays, aggregate/copy/lifetime execution |
| Member-designator queries | Parsed type/member/index path → canonical query/access/layout facts → constant evaluation or typed offset/stride operations. Work tracks path length, actual lookup and demanded layout. | Nested/anonymous/dependent members, runtime indices and exceptions, constexpr activations, negative operands, source→LowIR→native |

No production text roundtrip, source replay, host compiler delegation, fake
semantic node, process-global cache or optional optimizer was added. Per-class
placement indexes are temporary; raw type/query/field facts have TU ownership.

## Remaining implementation

[Owner ledger](../student.tests/pa29/evidence157/remaining.json) lists every
remaining fixture. Group labels guide reducers; they are not root-cause proof.

| Owner / failures | Required facts and next validation |
|---|---|
| Atomic/assembly: 21 | Atomic-qualified storage, ordering/effects and assembly constraints → serializable LowIR/native operations; atomic layout, runtime and noexcept checks. |
| Extended syntax/types: 37 | Vector/complex/extended-float types and operations; designated initializers, folds, lambda forms and structured bindings. Canonical type/grammar/lowering owners and execution controls. |
| Legacy traits/lifetime proofs: 6 | Relevant special-member properties and binding/materialization/lifetime facts; trait and reference-temporary reducers. |
| Template demand/hosted ABI: 19 | Complete specialization/context keys, demand edges and typed ABI entries; packs/aliases/caches, pretty-function, extern/inline and symbols. |
| Structured intrinsic operands: 4 | The composite builtin/offsetof fixture still needs constant-evaluation context, address-of and fence semantics; two invoke fixtures need receiver/member-pointer recipes; source-location needs lexical/caller context. |

Known extension boundaries outside the completed storage/path behavior remain
implementation work, not audit questions: function code-alignment attributes
need code-placement facts; dependent `offsetof` expressions used in mangled ABI
signatures are explicitly unsupported; class-convertible designator indices need
the corresponding integral-conversion facts. These are not accepted substitutes
for required behavior. The owning remaining groups must establish their facts
before those forms can be claimed supported.

## Performance evidence

[Protocol, costs and budgets](performance157.md) and its frozen raw evidence
cover latency, peak RSS, runtime and text size, with A/A and six ABBA blocks.
All common A/B objects/executables are byte-identical. Paired compiler medians
span 0.9411–1.0117 amid substantial noise; no speedup is claimed. New layout
workloads take 0.1081/0.2134/0.4351 seconds and 20,948/34,632/62,480 KiB at
600/1200/2400 demanded classes. Compiler growth is 30,488 bytes. Empty-layout
work stays constant across the separate 100-to-one-million array-bound control.
Optional optimizer work/growth budgets are **zero**. Historical blanket 15% and
zero-growth diagnostics remain diagnostics under spec §9; measurements,
mandated limits, correctness and coverage are preserved.

## Handoff ledger

- Commits: ownership plan `076139b6`; implementation and behavior controls
  `a33d1086`; final evidence/plan committed with this handoff.
- Required checks: PA1–28 **4538/4538**; PA29 **316/403** (exit 2); file audit
  passes with the same four inherited header-body warnings. Explicit controls
  **53/53**, inherited controls **77/77** and **40/40**, inspection **10/10**.
  [Validation](../student.tests/pa29/evidence157/validation.json) retains hashes,
  resolved original failures, unchanged fixtures and final sequential reports.
  Preliminary PA27 typedef-alignment regressions were fixed, not waived.
- Boundary: declaration placement/alignment and empty-member behavior are complete
  through dependent storage, generated initialization/copy and host interoperability;
  member-designator layout extends the same facts into constant/runtime queries.
  The 87 residual fixtures require new grammar/type-operation, atomic/assembly,
  lifetime-proof, template-demand or caller/receiver-context owners. Further
  attribute parsing or layout arithmetic cannot supply those missing facts.
- Independent review questions remain whole-stage source-to-ELF/spec auditing,
  forward-declared-only `std::is_nothrow_*` shorthand classes and the deleted-copy
  triviality oracle against C++11. No reference correction or proof bundle is
  claimed. These questions do not waive the implementation ledger above.
