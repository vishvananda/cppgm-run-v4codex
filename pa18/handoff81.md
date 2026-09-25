# PA18 implementation handoff 81

Entry `6924787714fac6bdeb0b3df29df49bb7cbe5fa7e`: **395/420**.
Implementation `7b8c98a6`, `4ab09a9b`, `25b89fc2`: **396/420**, original failures **25 → 24**.
The completed group is runtime forwarding of named scalar constants through
conversion functions, including selected conditional values, receiver lifetimes
and retained function uses. This is an implementation handoff, not full-stage
acceptance. The stage-base and last-reviewed markers remain unchanged.

## Ownership and design

`500-bool-alias-function-template-result-metadata` now passes its original LowIR
comparison. Its selected conversion forwards a known static scalar constant;
lowering consumes that proof and selects the conditional's live value without
emitting an unnecessary function call or unused inline definition. No fixture,
reference, comparison rule, source-name rule or stage switch is changed.

| Owner | Data flow | Work and lifetime | Evidence |
|---|---|---|---|
| `semantic/conversion_functions.cpp`, `conversion_result.cpp` | Materialized conversion use → completed selected body → one-return/named-constant proof → typed result keyed by function entity | Deduplicated scalar-use requests; shares the existing final declaration walk; one requested body inspection, at most eight parentheses, one canonical constant per successful summary. No body demand, recursion, arbitrary evaluation or new constant-expression eligibility. TU-owned flat index and constant arena. | Named bool/integer/floating/enum results; return conversion; effectful, volatile, reference, virtual and nonconstant exclusions; negative constexpr/NTTP and access tests. |
| `lowering/user_conversions.cpp` | Published result + selected second conversion → scalar value or required reference storage | O(1) per summarized use plus the receiver's normal effects. Uses existing typed conversions and slots; receiver construction/destruction remains observable. Missing destination remains an invariant violation. | Receiver calls, comma/subscript effects, converted references, narrowing, condition declarations and lifetimes. |
| `lowering/control_flow.cpp` | Known converted truth → selected arm's existing conversion/materialization | No new body, instruction cloning or CFG pass. Preserves terminal cleanup state, class destinations, reference category and receiver lifetime. Never evaluates the unselected arm. | Scalar/reference/class/void/nested conditionals, class returns, conversion effects and cleanup counts. |
| `lowering/symbols.cpp`, `function_declaration.cpp` | Summary-only inline candidate → deferred emission; actual calls/addresses/lifecycle consumers request its symbol → shared function declaration and one body | One deferred list visit after ordinary emission. Explicit roots stay eager. Nonvirtual leaf summaries cannot introduce function dependencies. No global retry, IR rewrite, textual key or removal/reindexing of existing functions. | Explicit and mixed calls, member-function addresses, template scales and retained-use benchmark. |
| `semantic/expression.cpp`, `type_builder.cpp` | Qualified conversion name → canonical target/candidate set; class typedef → canonical member-pointer owner | Required use/class lookup contexts inspect already-parsed type syntax; no parser replay. Reuses conversion candidate filtering, access/deletion checks and target-directed overload resolution. Alias normalization is O(1). | Eleven previously failing address/alias controls, cv/target overloads, scope agreement/mismatch, nonclass/deleted/private rejection. |

The address controls exposed two inherited defects while checking the new
emission boundary: conversion-function-ids were sent through ordinary identifier
lookup, and member-pointer declarators tested the alias entity instead of its
canonical class. Both defects are repaired rather than excluded from validation.
Qualified conversion types also enforce agreement between use and class contexts.
The supplied standard anchors are [N3485 §3.4.3.1 and §3.4.5](../doc/n3485.txt:3295),
[§5.1.1/12](../doc/n3485.txt:5329) and §8.3.3 [dcl.mptr]. This is not a reference
correction; no oracle or bundle revision is needed.

The first frozen measurements exposed avoidable work on explicit-only calls.
`25b89fc2` records summary requests at materialized scalar conversions, so
explicit-only/address-only functions receive neither a proof nor deferred
emission. Their exact entry LowIR/native comparison and zero summary-work
inspection guard that boundary; pre-refinement measurements remain preserved.

The O0 summary deliberately handles forwarding an existing named constant, not
arbitrary function-body interpretation. It declines bodies with extra statements,
more than eight wrappers, reference/pointer/class results or virtual dispatch.
Those cases keep the valid ordinary call. Runtime proofs never enter C++11
constant-expression queries; `noexcept` and overload viability keep their source
semantics. The new implementation sources are registered in
`dev/frontend_source_sets.mk`.

## Verification

- `make test-pa18`: **396/420**, exit 2; **25 → 24** original failures, no new failures.
- `make test-report-through-pa17`: **2609/2609**, exit 0.
- PA18 file audit: exit 0, the same three inherited header advisories.
- **1137** explicit controls pass, including **74/74** new controls against
  **63/74** on the frozen entry compiler. ABI and inherited scaling checks pass.
- **Ten** new inspection programs validate and execute, checking summary work
  at 1/600/2400 specializations, conservative wrapper-budget fallback, retained
  explicit functions and the combined source trace, and zero summary work on explicit-only uses. All three current/prior
  source-to-native traces pass.
- All **420** course sources and **1686** fixture/reference files are identical
  to entry. Personal tests run explicitly from `student.tests/pa18/`.

[Evidence](../student.tests/pa18/loop81-evidence.json) records commands, exits,
hashes, raw controls and counters. [Performance](performance81.md) records frozen
A/A and ABBA compiler latency/RSS and checked native runtime/size. The initial
trace exposed alias ownership; its failing scratch log is preserved alongside
the passing final trace. No failing required case is removed or reclassified.

The [combined trace](../student.tests/pa18/result81_trace.cpp) demands a boolean
conversion specialization, evaluates an effectful receiver, chooses a class
result with a destructor, and invokes the retained conversion through both an
explicit call and a member pointer. Semantic IDs and conversions flow directly
into typed LowIR. The personal harness validates that IR and executes it with
the course's supplied native backend; PA18 does not own native code generation.

## Boundary and independent review

**24 original failures remain implementation work.** Array/aggregate images and
unknown-bound empty expansions need a shared materialization policy; class-result
ABI, empty-tag initialization/root metadata and global publication need object
and emission facts; discarded loads and arithmetic widening have separate scalar
representation owners. The inherited class-ellipsis reducer remains unfinished.
These are not independent-review questions and are not waived.

The completed trace has been extended through the relevant scalar, conditional,
reference, lifetime, explicit-call, address, alias and emission consumers. Its
proof concerns a scalar-returning leaf and cannot choose an array image, define
a class-result calling convention or publish a static member definition. Further
progress in those remaining failures needs a different semantic owner and
contract trace. Expanding this optional summary to arbitrary bodies would not
resolve them and would require a separate effects/profitability policy. This is
the concrete handoff boundary; one passing course case alone was not the stopping
condition.

Independent review still owns handoffs 79/80 and this handoff's proof eligibility,
selected-arm cleanup, emission roots/address retention and qualified conversion
lookup. Passing controls support those questions but do not waive the audit.
Do not advance until the through-PA18 report and whole-stage audit succeed.
