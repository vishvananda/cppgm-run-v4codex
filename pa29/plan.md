# PA29 compact plan — implementation172 in progress

Target: **PA29 full-stage**. Phase: **implement; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `221d6d0e4930da05db2913bdf5f50d808f89c744`.
Entry HEAD: `1bc21c576272364e790fad3920a76dab9e624558`.
Validated implementation: `846ef3fb` (records-only handoff follows).

## Design and completed owner

Builtin template aliases now retain canonical typed argument queries for
`__type_pack_element` and `__make_integer_seq`. Argument syntax is parsed once;
selection and generation consume retained arguments and immutable substitution
frames. First-class uses lazily create real typed template heads, with no fake
syntax or body. Existing alias specialization facts memoize success/failure;
query facts resolve direct, dependent, nested/empty and indirect applications.
Lowering receives the selected type and uses typed template arguments for ABI
names. The [extension signatures](https://clang.llvm.org/docs/LanguageExtensions.html#builtin-type-aliases)
define selection and sequence generation; references are unchanged.

GNU hosted head matching defers dependent non-type parameter types. Application
checks actual values, and deduction preserves a concrete integral argument's
type. The latter fixes the PA17 regression discovered during this handoff.
No declaration scans, source replay, textual transport, global invalidation,
unrelated body demand or optional optimization was added.

Selection validates consumed arguments in O(n), then chooses the indexed type;
generation is O(n) in produced elements and retains the existing 1,048,576-element
ceiling. Alias heads have bounded fixed size and TU lifetime; queries/argument
slices live in the existing canonical arenas, temporaries die at operation exit.
Existing evaluator/backend limits remain unchanged. Optional optimizer work and
code-growth budgets are zero; required semantic/code costs are measured below.

## Validation and performance

- PA29 **367/403**, six original failures fixed, **36** remain, no new failures.
- PA1–28 **4538/4538**; through PA29 **4905/4941**, PA29 alone unfinished.
- File audit passes with four unchanged inherited header warnings.
- **45** focused checks, **28** inherited integration checks and **13** inspection
  checks pass, including a Clang ABI peer, typed IR roundtrip and telemetry
  equivalence. Results are in [evidence171](../student.tests/pa29/evidence171/validation.json).
- [Failure delta](../student.tests/pa29/evidence171/stage-delta.json) and
  [coverage](../student.tests/pa29/evidence171/coverage.json) preserve all 403 inputs
  and all 1,707 tracked contract/harness paths against entry.

[Performance171](performance171.md) records frozen A/A+ABBA common workloads and
new-capability scaling, compiler latency/RSS and executable runtime/text together.
An entry rejection is never treated as a speed baseline. No speedup is claimed.
Historical measurements, including [performance170](performance170.md), remain
unchanged. Historical blanket 15%/zero-growth targets remain diagnostic under
spec §9; no mandated limit, correctness rule or coverage was relaxed. Larger
hosted runtime, optimization/allocation and self-hosting keep PA30–34 ownership.

## Remaining work and handoff ledger

The [remaining ledger](../student.tests/pa29/evidence171/remaining.json) retains
extended syntax/types/layout **27**, hosted template/emission **7**, legacy trait
**1**, and source-invocation intrinsic operands **1**. Numeric representations,
templated lambdas, folds, bindings, conditional explicit/control flow, zero-length
arrays, source coordinates and hosted emission remain implementation work.
Retain the existing alignment, dependent offsetof ABI and convertible-index
reducers, and audit170's contract questions; none is waived or silently corrected.

| Range | Implementation result | Independent review |
|---|---|---|
| audit170 through `221d6d0e` | Previously reviewed owner corrections; historical ledger in [audit.md](audit.md) | Completed for that recorded range; stage unfinished |
| entry `6525c1af` → `846ef3fb` | Pack selection, sequence generation, nested/empty/indirect aliases, dependent head matching and exact integral deduction; 42 → 36 failures | New range awaits Ralph's independent audit; passing checks do not certify the whole stage |

Boundary: all six failing fixtures owned by these builtin template/pack operations
are resolved, including first-class identity and emitted ABI/runtime use. Nearby
[probes](../student.tests/pa29/evidence171/boundary-probes.json) reach distinct
vector-expression/closure, syntax or contract owners. Further stage work requires
new closure/fold/binding representations, numeric/ABI machinery, emission state
or source-origin handling; it cannot extend this completed argument-query owner
without starting another broad semantic group. The two inherited oracle concerns
remain separate review questions with failures still counted. Full through-PA29
success and independent whole-stage review remain necessary before advancement.

## Active behavior group (172)

Baseline 367/403, 36 failures. Fold expressions own three failures in parser
primary expressions and typed template queries. Parse each fold once; retain
operator, direction and operands. Substitution expands only the pack operand
through immutable lane frames and records typed operator/conversion facts.
Lowering must consume those facts, including short circuit and overloaded
operators, without synthesized frontend syntax. Work/storage scale with consumed
pack elements and required operator candidates; no optional optimization budget.
Validate all four forms, empty/single/multiple packs, diagnostics, substitution,
constant/type queries and runtime side effects; then required stage/prior/audit
gates. Freeze entry/final binaries for equivalent A/A+ABBA and new-capability
compiler latency/RSS plus executable runtime/text evidence. Existing review
markers and all independent review questions above remain unchanged.
