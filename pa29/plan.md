# PA29 implementation / handoff157 (in progress)

Target: **PA29 full-stage**. Phase: implementation handoff; stage unfinished.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Turn entry: `0bad8c207712b2077afc78f209a469c55b07847b`, clean, **301/403**, 102 failing.
Previous handoff: 29 original failures resolved, no new failures.
Prior turn was verified progress; review markers remain unchanged.

## Active work

Complete declaration attributes and empty-member layout, then extend related
parser/declaration fixes while the same semantic owners apply. No fixture edits.

| Owner | Data flow / complexity | Validation |
|---|---|---|
| Attribute parser / declaration prediction | Parse alignment operands once into source graph; bounded token lookahead recognizes attributed declarations and pointer annotations. O(tokens + attribute operands). | Condition/for/range, namespace/parameter placement, dependent alignment and negative constants |
| Layout / object facts | Canonical member identity owns overlap permission; class layout consumes alignment and empty-subobject summaries once. Preserve bounded conflict work and nonoverlap of equal types. | Empty/repeated/nested objects, generated copies and constructors, source LowIR/native execution |
| Performance | Freeze turn-entry binary; measure common equivalent A/B plus new layout costs under spec §9. No optional optimization budget. | Latency/RSS, runtime/text, A/A and ABBA with checked outputs |

Implementation gaps and independent review questions below remain open; the
existing Stage base / Last reviewed markers are unchanged.

## Design/spec alignment

Prior hosted driver, numeric decoding, canonical type queries and shared
access/exception/inspection work remain in place; their evidence is preserved in
[handoff155 performance](performance155.md) and `student.tests/pa29/evidence155/`.

| Completed owner | Data flow / complexity | Validation |
|---|---|---|
| Runtime signature registry | Immutable builtin vocabulary → canonical declarations/conversions → ordinary C ABI calls. Includes libm mixed signatures, memory/string families, explicit builtin redeclarations and operator allocation aliases. Bounded vocabulary; O(operands) per call. Probes use the same registry. | Hosted cmath/cstring, arity/type/qualification negatives, link/runtime controls |
| Integer operation facts | Selected intrinsic + canonical signature → constexpr evaluation or direct LowIR. Fixed-width conversions and generic unpromoted widths stay distinct. TU-owned integer/overflow signature caches have complete typed keys and independent lifetimes. | Exhaustive small inputs, every 64/128-bit position, constexpr activation, zero fallback, promotion and side effects |
| Overflow/hint/FP operations | Checked argument facts → modular LowIR and representability check, including a bounded 256-bit pair for 128-bit products. Hints evaluate arguments once without inventing optimizer promises. Fenv rounding queries use the target C ABI. | 1,944 Python-integer oracle cases, volatile result stores, fenv modes, default/optional hint operands |
| Floating constants/effects | Sign/NaN classification and comparison use typed operand facts; static and runtime signaling/payload NaNs retain exact bits. Ordinary LowIR integer storage represents payloads that nan/snan words cannot express. Inline operations consume no-unwind facts while operand exceptions remain visible. | Negative zero, quiet/signaling payloads, globals/class fields, source→LowIR→reader→native/MIR execution |

No production text roundtrip, source replay, fake semantic node, host compiler
implementation, process-global cache or optional optimizer was added. Constant
bit counts consume at most 128 bits; runtime sequences are O(log width), with
width ≤128. Overflow uses at most four 64×64 partial products. Registry/signature
work, operand checking and lowering track demanded calls, not unrelated entities.

## Remaining implementation

[Owner ledger](../student.tests/pa29/evidence156/remaining.json) retains all 102
failing fixtures. Owner assignments guide reducers; they do not prove root causes.

| Owner / failures | Required facts and next validation |
|---|---|
| Atomic/assembly: 21 | Atomic-qualified storage, ordering/effects and assembly constraints → serializable LowIR/native operations. Atomic layout, runtime and noexcept checks. |
| Extended syntax/types/layout: 51 | Retained attributes/dependent widths, vector/complex/float storage, designated initializers, folds/lambdas/bindings and no-unique-address copies. Parser/layout/execution controls. |
| Legacy traits/lifetime proofs: 6 | All relevant special-member properties and binding/materialization/lifetime facts; not ordinary overload viability aliases. Trait and reference-temporary reducers. |
| Template demand/hosted ABI: 19 | Complete specialization/context keys, demand edges and typed ABI entries. Pack/alias/cache, pretty-function, extern/inline and symbol controls. |
| Structured intrinsic operands: 5 | Two offsetof-containing fixtures need member-path/layout and constant-context facts; two invoke fixtures need receiver/member-pointer/dereference recipes shared with query substitution; source-location needs lexical/caller context. These remain implementation, not runtime signature-table entries. |

## Performance acceptance

[Protocol, costs and budgets](performance156.md) retain all four dimensions,
A/A calibration, six ABBA blocks and standalone new-capability measurements.
Common A/B objects and executables are byte-identical; median paired compilation
ratios are 1.0016–1.0088. No speedup is claimed. New optional optimization work
and growth budgets are **zero**. Necessary semantic costs and later PA32/33
optimization remain distinct. Historical blanket 15%/zero-growth diagnostics
remain diagnostics under spec §9, not additional mandated exit gates.

## Handoff ledger

- Commits: plan `5716fcfd`; signature/integer/FP group `3611c078`;
  redeclaration, hint/overflow and exact-bit LowIR extension `81a9f67f`.
- Final required checks: PA1–28 **4538/4538**; PA29 **301/403** (exit 2);
  file audit passes with four inherited header-body warnings. Explicit controls
  **40/40**, including 1,944 overflow cases, and inherited controls **77/77**.
  [Validation](../student.tests/pa29/evidence156/validation.json) records hashes,
  unchanged fixture coverage, original failures resolved and sequential reruns
  after concurrent course reports contaminated their shared counter totals.
- Boundary: completed runtime signatures and scalar operation semantics, then
  extended through hosted wrapper acceptance, implicit declarations, constexpr
  predicates, allocation aliases and lossless NaN storage. The five residual
  intrinsic fixtures require different language facts: parsed layout paths,
  caller-sensitive constant/source context, or member-pointer receiver recipes
  valid during both substitution and concrete lowering. More signature entries
  or scalar expansion cannot establish those facts; each needs its own coherent
  semantic owner work. The other 97 failures also remain unfinished implementation.
- Independent review questions: whole-stage source-to-ELF/spec audit; review
  shorthand `std::is_nothrow_*` forward-declared-only classes and the deleted-copy
  triviality oracle against C++11. No proof bundle/reference correction is claimed.
  These questions and review markers are preserved; this handoff is not stage approval.
