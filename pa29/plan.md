# PA29 compact plan — implementation173 handoff

Target: **PA29 full-stage**. Phase: **implement; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `221d6d0e4930da05db2913bdf5f50d808f89c744`.
Entry HEAD: `51fde1023d4d8f4135bf88429a0706a3c49fa7eb`.
Validated implementation: `128e7e39`.

## Design and completed owner

Explicit-template lambdas retain their parsed head, canonical parameters and
lexical bindings. A closure owns a member-template call operator; ordinary
specialization frames supply enclosing and deduced arguments. Definition-time
capture recipes propagate nested capture dependencies once. Each concrete closure
establishes its fields and initialization before calls or layout; specializations
cannot append storage after layout. Non-dependent source facts are shared, and
bodies remain dormant until required. Selected calls and pointer adapters consume
typed parameter, conversion, capture, exception and lifetime facts in LowIR.

The group includes packs, explicit/default arguments, dependent trailing returns
and substitution failure, `noexcept`, mutable/copy/reference/this/nested captures,
copying and cleanup, function-pointer conversions and their distinct ABI entries.
The [closure](https://timsong-cpp.github.io/cppwp/n4868/expr.prim.lambda.closure)
and [capture](https://timsong-cpp.github.io/cppwp/n4868/expr.prim.lambda.capture)
rules define this hosted explicit-template extension. Typed ABI head declarations,
`auto` returns, lexical numbering and host ODR linkage retain the existing shared
mangler; the source `decltype` identity bit now reaches its `Dt`/`DT` encoding.
[ABI rules](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-type)
and the [host head grammar](https://github.com/llvm/llvm-project/blob/main/clang/lib/AST/ItaniumMangle.cpp)
support those encodings. No fixture, reference or comparison rule changed.

Work/storage track source nodes, capture dependency edges and demanded
specializations/IR, plus required overload candidates. Source capture recipes and
adapter records occupy contiguous TU storage; flat indexes use complete source,
frame, declaration and specialization identities. Scratch vectors end with their
operation; source and semantic facts end with the TU. No grammar replay, fake
function AST, textual phase transport, global invalidation or unrelated body
demand is introduced. Optional optimizer work and code-growth budgets are zero.

## Validation and performance

- PA29 **377/403**: all seven existing explicit-template-lambda failures fixed;
  **26** remain, with no new failures. PA1–28 **4538/4538**; through PA29
  **4915/4941**. A through-PA29 pass is still required before advancement.
- **59** focused and **29** inspection checks pass: acceptance/rejection/runtime,
  nested and pack captures, defaults/query substitution, cleanup, host names and
  peers, typed ABI roundtrip, AST, LowIR roundtrip, MIR/native and telemetry.
- File audit passes with four unchanged inherited header warnings. Required
  reports ran sequentially on the final compiler. [Validation](../student.tests/pa29/evidence173/validation.json),
  [delta](../student.tests/pa29/evidence173/stage-delta.json) and
  [coverage](../student.tests/pa29/evidence173/coverage.json) retain all 403 inputs
  and all 1,707 contract/harness paths unchanged from entry.

[Performance173](performance173.md) records frozen compiler latency/RSS and
checked executable runtime/text, A/A noise and ABBA spread, source/capture/body
counts and explicit budgets. No optimization speedup is claimed. All observations
are retained, including preliminary binaries preceding the final query/ABI fixes.
Historical [performance172](performance172.md), [performance171](performance171.md)
and [performance170](performance170.md) remain unchanged. Unsupported blanket
15%/zero-growth targets remain diagnostic under spec §9; mandated limits,
correctness and coverage are preserved. PA30–34 still own heavier hosted runtime,
optimization/allocation and self-hosting.

## Remaining work and handoff ledger

The [remaining ledger](../student.tests/pa29/evidence173/remaining.json) groups
extended syntax/types/layout **17**, template demand/hosted ABI **7**, legacy trait
**1**, and source-invocation intrinsic operands **1**. Numeric representations,
structured bindings, control flow/conditional explicit, deduction guides,
zero-length arrays, static call operators, source coordinates and hosted emission
remain implementation work. Audit170's two independent contract questions
(nothrow default-construction shorthand and nothrow-invocable cache default) remain
unresolved, counted failures. Preserve its char-traits discussion and existing
alignment, dependent offsetof ABI and convertible-index reducers. Nothing is
waived or silently corrected.

| Range | Implementation result | Independent review |
|---|---|---|
| audit170 through `221d6d0e` | Reviewed owner corrections; history in [audit.md](audit.md) | Completed for that range; stage unfinished |
| entry `6525c1af` → `846ef3fb` | Pack selection, sequences, aliases, head matching and deduction; 42 → 36 failures | Awaiting Ralph's audit |
| entry `1bc21c57` → `827b4c7c` | Fold syntax/substitution, constants/runtime, lifetimes, callable/ABI facts and bounded traversal; 36 → 33 failures | Awaiting Ralph's audit |
| entry `51fde102` → `128e7e39` | Explicit-template closure heads, captures, callable specializations, pointer adapters, queries and ABI; 33 → 26 failures | Awaiting Ralph's audit; whole-stage requirements remain open |

Boundary: all seven fixtures owned by explicit-template closure construction are
resolved. Work continued through runtime storage, lookup, packs, conversions,
exception/cleanup, query rejection and ABI peers beyond their compile-only oracle.
The remaining failures require different numeric/layout representations,
decomposition/control nodes, static-receiver evaluation, emission state or source
origin handling. Extending the closure-template owner cannot supply those facts.
This completes the implementation handoff, not the assignment or its independent
whole-stage audit. Both unfinished implementation and review questions remain.
