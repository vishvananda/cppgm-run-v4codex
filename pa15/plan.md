# PA15 implementation handoff

Stage base commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`
Last reviewed commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`

Target: **PA15 full-stage**, O0 typed LowIR. Implementation handoff only;
whole-stage completion and independent review remain pending.
Entry `19225b3d`: **128/177**; handoff: **166/177**, **11 failures**.
**38 original failures resolved**, no previously passing fixture regressed,
and coverage remains 177. All cluster-200 pack/literal fixtures now pass.

## Design and spec alignment

| Owner | Data flow, complexity and validation |
|---|---|
| Canonical packs and deduction | One interned argument slice per template parameter preserves empty and nonempty pack partitions. Explicit pack prefixes have a separate partial-selection index; completed specializations use complete arguments. Deduction consumes actual arguments, then signature substitution expands parameter types. Native controls exercise explicit-prefix extension, function targets and 64 equal-flattened partitions reused twice. |
| Expansion and source occurrences | Cached source/typed-pattern pack discovery feeds immutable substitution frames, keyed by parent, parameter identities, bindings and lane. Nested expansions retain their own packs; lockstep lengths are checked. Source syntax stays shared; only changed child lists receive occurrence edges. Work follows source regions and produced lanes, without grammar replay or unrelated Cartesian scans. Empty, repeated, nested and unequal-length controls cover identity and rejection. |
| Ordinary declaration/body consumers | Parameter, argument, brace, base and constructor lists consume those occurrences and typed facts. Complete/base constructor demand and reverse base destruction use existing lifetime/ABI owners. Native array, reference, multiple-base and constructor/destructor order controls validate execution. |
| Unevaluated/default queries | Retained new-expression, expansion and sizeof-pack queries preserve allocation/constructor operands and complete pack sizes. Substitution checks signatures without demanding execution; default arguments retain their declaring environment. Placement-new defaults and sizeof inside nested expansion are exercised. |
| Literal and output owners | Phase 7 retains spelling and decoded numeric values; semantic selection chooses cooked/raw/character-pack calls and ordinary conversions. Lowering uses normal call/result/lifetime facts. Typed call operands preserve f80 through the LowIR writer. Immutable string code units support checked integral subscripts. Native raw/cooked precedence, overflow, reference/class results, wide strings and floating controls pass. |
| Lifetime and telemetry | New caches are flat and TU-owned with complete semantic keys; no process cache or textual phase transport. Counters observe existing source traversal, expansion lanes, frames and body transitions. Scalar parameter lists retain source edges and skip unused expansion indexes. |

## Remaining implementation and handoff boundary

| Required group | Remaining fixture stems / missing owner |
|---|---|
| Class partial matching and dependent aliases | `dependent-bool-trait-nontype-argument`, `dependent-typename`, `function-template-dependent-alias-parameter-overloads`, `type-equivalence-default-argument`: selected-pattern substitution and dependent alias candidate facts. |
| Constant objects, initialization and storage | `aggregate-functional-braced-cast`, `nontype-conversion-operator-dependent-template-id`, `qualified-type-before-constexpr-local`, `reference-static-constexpr-member-replay`, `static-constexpr-member-call-initializer`, `stale-function-template-instantiation-lookup`: constant execution/binding and initializer/storage contracts; the stale-function mismatch is in initialization/LowIR, not function selection. |
| Ordinary source obligations | `unused-member-function-static-assert-bad`: ordinary member validation must be independent of emission demand, including explicit-class member bodies. Eager lowering would violate that boundary. |

The pack/literal group was extended through declarations, nested scopes,
constructors/destructors, default new queries, literal result lifetimes,
string-element constants and explicit-prefix deduction. Its contract fixtures
and personal execution/rejection controls pass. Further changes need class
partial-pattern matching, constant-object execution/storage, or an ordinary-body
validation owner; changing pack partitions or literal selection cannot establish
those facts. This is the concrete implementation boundary, not a progress quota.
All remaining groups are required; none is reclassified as an audit question.

Independent review remains open for whole-stage correctness/spec conformance,
complete keys, source sharing, demand/invalidation boundaries and performance.
Neither review marker advances. No reference, fixture, bundle, harness or
comparison rule changed.

## Validation, performance and ledger

Required checks: `make test-pa15` **166/177**, exit 2 with strict reduction;
`make test-report-through-pa14` **1935/1935 pass**; file audit **pass** with the
same three inherited header warnings. Personal suites explicitly run:
**27 native + 8 rejection pack/literal controls**, **24 native + 13 rejection
specialization controls**, **26 value** and **10 constant** groups.

[Performance evidence](../student.tests/pa15/pack-performance.md) records frozen
A/A and ABBA latency/RSS, native runtime/text, output parity and linear work
scaling. The first campaign exposed avoidable scalar-list overhead; the final
campaign follows its removal. All observations remain in
`$RALPH_ARTIFACT_DIR/pa15-packs/`. PA15/O0 supplies no numerical cost ceiling or
optional optimization in this increment. Historical self-selected diagnostic
gates do not override stage-scoped acceptance; correctness, complexity and
coverage remain mandatory. Later native optimization/self-hosting limits retain
their own stages. Prior [scalar](../student.tests/pa15/performance.md) and
[specialization](../student.tests/pa15/specialization-performance.md) evidence is preserved.

| Increment | Commit | PA15 |
|---|---|---:|
| Canonical packs and typed list expansion | `53f1afa9` | 162/177 |
| Literal calls, string constants and explicit pack prefixes | `768e31f0` | 166/177 |
| Remove unnecessary scalar-list expansion edges | `992b1170` | 166/177 |

[Handoff ledger](../student.tests/pa15/pack-handoff.json) lists the exact original
failures, resolved/remaining sets, unchanged fixture trees, check hashes and
performance artifacts. [Verifier](../student.tests/pa15/verify_packs.py) checks
those artifacts, counter scaling and preserved review markers. This returns
control for Ralph's scheduled review; it does not certify the whole assignment.
