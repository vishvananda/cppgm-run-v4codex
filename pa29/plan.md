# PA29 compact plan — implementation173 in progress

Target: **PA29 full-stage**. Phase: **implement; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `221d6d0e4930da05db2913bdf5f50d808f89c744`.
Entry HEAD: `51fde1023d4d8f4135bf88429a0706a3c49fa7eb`.
Validated implementation: `827b4c7c41a7a15b8d769e8f04915ae43cf791d7`.

## Design and completed owner

Fold expressions now retain operator, direction and source operands in one
parsed node. Canonical typed queries preserve definition lookup and immutable
substitution frames; pack expansion reduces them to selected operators and
conversion facts. Runtime prvalues survive query expansion; constant template
argument formation retains its separate checks. Nested folds own their packs.
Typed `FoldStep` records feed constant execution and LowIR/MIR/native lowering,
including short circuit, overloaded calls, bound member calls and cleanup.
No synthetic syntax, textual phase transport, global invalidation or unrelated
body demand is added. ABI operands retain source order.

All four forms, empty identities, association and substitution failure follow
the PA29 hosted extension's [fold grammar](https://timsong-cpp.github.io/cppwp/n4659/expr.prim.fold)
and [pack reduction rules](https://timsong-cpp.github.io/cppwp/n4659/temp.variadic).
These are later-standard extensions to the cumulative C++11 compiler.
[Itanium expression encodings](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-expressions)
define `fl/fr/fL/fR`. No contract fixture or reference was changed.

Work/storage scale with consumed pack elements and emitted operations, plus
language-required overload candidates. Queries and typed steps use contiguous
TU arenas and identity indexes; temporary traversal vectors die at operation or
function exit. Iterative reduction and explicit traversal stacks avoid recursive
fold spines, including the 24,000-element alias that exposed a stack overflow.
Existing query/mode and constexpr activation keys retain validity and failures.
Required evaluator, generator and backend limits remain unchanged. Optional
optimizer work and code-growth budgets are zero.

## Validation and performance

- PA29 **370/403**: three existing fold failures fixed, **33** remain; no new
  failures. PA1–28 **4538/4538**; through PA29 **4908/4941**.
- File audit passes with four unchanged inherited header warnings.
- **51** focused checks and **13** inspection checks pass: all fold forms,
  type/constant/runtime use, nested/empty packs, SFINAE, sequencing, overloads,
  exceptions/cleanup, member pointers, host ABI peer, AST, LowIR roundtrip,
  MIR/native execution and telemetry equivalence.
- Required reports ran sequentially on the final compiler. Earlier overlapping
  report attempts are not evidence. [Validation](../student.tests/pa29/evidence172/validation.json),
  [failure delta](../student.tests/pa29/evidence172/stage-delta.json) and
  [coverage](../student.tests/pa29/evidence172/coverage.json) preserve all 403
  inputs and all 1,707 tracked PA29 contract/harness paths against entry.

[Performance172](performance172.md) retains all 224 common A/A+ABBA observations
and 96 new-capability observations, with compiler latency/RSS and executable
runtime/text. Common outputs are byte-identical and existing work counters
unchanged; every paired timing range crosses 1. No speedup is claimed.
Required runtime fold steps are `2N+8`, alias value steps `N`; the 24,000-element
alias takes median 0.1543 s / peak 31,092 KiB. Compiler size grows 0.96%.
Entry rejects new-capability inputs and is not an equivalent speed baseline.

Historical [performance171](performance171.md) and [performance170](performance170.md)
remain unchanged. Unsupported blanket 15%/zero-growth gates remain diagnostic
under spec §9; no mandated limit, correctness rule or coverage was relaxed.
Heavier hosted runtime, optimization/allocation and self-hosting retain PA30–34
ownership. Necessary semantic costs are disclosed, not used to excuse regressions.

## Remaining work and handoff ledger

The [remaining ledger](../student.tests/pa29/evidence172/remaining.json) groups
extended syntax/types/layout **24**, hosted template/emission **7**, legacy trait
**1**, and source-invocation intrinsic operands **1**. Numeric representations,
templated lambdas, bindings, conditional explicit/control flow, zero-length
arrays, source coordinates and hosted emission remain implementation work.
Audit170's two independent contract questions (nothrow default-construction
shorthand and nothrow-invocable cache default) remain unresolved, counted failures.
Preserve its char-traits discussion and existing alignment, dependent offsetof
ABI and convertible-index reducers. Nothing is waived or silently corrected.

| Range | Implementation result | Independent review |
|---|---|---|
| audit170 through `221d6d0e` | Previously reviewed owner corrections; historical ledger in [audit.md](audit.md) | Completed for that recorded range; stage unfinished |
| entry `6525c1af` → `846ef3fb` | Pack selection, sequence generation, nested/empty/indirect aliases, dependent head matching and exact integral deduction; 42 → 36 failures | Awaiting Ralph's independent audit |
| entry `1bc21c57` → `827b4c7c` | Fold grammar, substitution, types/constants, runtime/lifetimes, callable facts, ABI and bounded traversal; 36 → 33 failures | Awaiting Ralph's independent audit; checks do not certify the whole stage |

Boundary: all three fold-owned failing fixtures are resolved. Work extended
beyond those type/constant fixtures through runtime operators, short circuit,
temporary cleanup, constexpr/member calls, ABI peers and large-pack traversal.
The remaining failures require distinct closure/binding/control representations,
numeric/layout machinery, emission state or source-origin handling; they cannot
be completed by extending this fold owner alone. Independent whole-stage audit
and full through-PA29 success remain necessary before advancement. This is a
validated implementation handoff, not assignment completion.

## Implementation173 working group

Entry is clean, PA29 370/403 (33 failures). Previous turn supplied validated
progress (fold owner); its markers and evidence remain preserved above.
Owner: explicit-template lambda closure and function-template semantics (seven
existing failures). Data flow: parse head once, retain canonical parameters and
lexical bindings, create a member-template call operator, substitute through the
existing specialization frames, then consume selected calls/capture fields in
typed lowering. Work tracks source nodes, captures and demanded specializations;
no token replay, synthetic function AST or broad retry. Extend through packs,
defaults, nested captures, noexcept and runtime calls while this owner applies.
Validate focused acceptance/rejection/runtime/inspection controls, every required
suite and unchanged coverage; freeze entry/final binaries for A/A+ABBA latency,
RSS, runtime and text evidence. Optional optimizer work/growth budget is zero.
Remaining groups and independent contract questions above stay unresolved.

Implementation173 progress: all seven explicit-template lambda fixtures now
compile (377/403 preliminary PA29); 52 personal checks pass. Retained source
heads/capture recipes, ordinary specialization frames and typed adapter records
cover packs/defaults/noexcept, captures and function-pointer conversions. ABI
heads and deduced returns are typed, and hosted inline/template-local entities
retain ODR linkage. Final reports/performance evidence remain outstanding.
