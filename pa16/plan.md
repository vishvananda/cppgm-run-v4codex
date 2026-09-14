# PA16 implementation handoff — loop 37

Stage base commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`
Last reviewed commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`

Target: **PA16 full-stage**. Implementation handoff is incomplete at stage level:
**63/154 passing, 91 failing**, versus entry **45/154, 109 failing**. All 18
improvements fix existing failures; none of the previous passes regressed.
Previous goal turn: progress (PA15 final audit); entry state was revalidated.

## Completed group and spec alignment

Arithmetic by-value constexpr execution now supports nested/recursive/defaulted
calls, mutable scalar locals/parameters, condition declarations, branches,
for/while/do loops, break/continue, assignments, increments and empty scalar
initialization. Executed invalid initializers invalidate the call even if unused.
Core arithmetic failures return a failed constant probe; required-constant
contexts reject them, while ordinary initialization retains its dynamic path.

| Owner | Data flow / complexity / lifetime | Evidence |
|---|---|---|
| `semantic/constant_execution` and `constant_statements` | Checked parsed body + selected conversions -> transient flat entity-indexed frame -> typed result. Completed activation key includes body/function, receiver and converted typed argument identities. Mutable node values are recomputed; no stale per-node execution cache. Frames release at return. Work follows executed visits, capped at 512 active calls and 1,000,000 statement/expression/loop steps per root. | Scalar controls, recursive/deferred PA15 controls; activation and step counters. |
| `semantic/constant_floating` | Literal or scalar input -> target-rounded float/double/x87 value -> interned payload ID. The common Constant record stays compact. TU flat index ignores x87 padding, preserves signed zero; arithmetic/conversions and template queries share the engine. Range checks precede floating-to-integer host casts. | Float, precision, unsigned magnitude, casts, query/default/local/global controls. |
| `semantic/constant_array`, static initializer facts, lowering | Validated constexpr initializer plan -> once-indexed child ranges -> scalar projection. O(explicit children) setup, O(log children) indexed access; omitted ranges remain compact. Typed scalar facts feed LowIR data/immediates directly. | Nested arrays, bounds, million-element omitted range, runtime data checks and index counters. |
| Declaration and lookup | Reuse static member initializer identity; reject a second initializer. Separate ordinary/tag binding indexes preserve both declaration orders. Complete-class hints retain nested tag categories. | New lookup/static-definition controls and all earlier course tests. |

The shared graph, canonical types/declarations/queries, demand owners and direct
LowIR pipeline remain in use. No text transport, host compiler delegation,
fixture recognition, reference correction, or additional optional optimizer.

## Remaining implementation and boundary

| Unfinished group | Owner and necessary next data flow | Validation to close |
|---|---|---|
| Object, pointer, reference and callable values | Extend constant-value identity with object roots, subobject paths, activation/lifetime and address bounds; execute selected constructor/base/member actions in their access context; consume member/operator/conversion facts. | Constructors, aggregates, mutable/static address identity, reference parameters, callable/member/arrow/operator calls and relevant scalar-call crossovers. |
| Declaration validity | Literal-type and constexpr validity facts at declaration/class completion, preserving dependent-template rules. | Nonliteral owners/parameters/results, void functions, uninitialized members and template instantiations. |
| Exception expressions | Completed selected-call/default/constructor/destructor exception facts -> noexcept values. | Deferred templates, constructors/destructors, assertions and decltype. |
| Storage and lowering | Constant probe + declaration/lifetime identity -> constant data or guarded first use and destruction. Extend automatic constant scalar-array copy policy beyond inherited PA15's 32-byte heuristic as PA16 requires. | Local scalar/array/class statics, static references, class factories, automatic-array LowIR contract. |

The scalar group is coherent and its known defects are resolved. Further related
progress now requires the object/address/lifetime domain: scalar payloads alone
cannot safely identify repeated base subobjects, escaped references or mutable
receivers in call keys. Implementing those through scalar stand-ins would violate
the spec. This is the concrete handoff boundary; those owners remain required
implementation, not waived review questions. PA17 advancement is not authorized.

## Performance and validation

[Performance evidence](performance.md) preserves four frozen campaigns and all
896 timing observations, including preliminary/noisy results. Final common
LowIR/native outputs are identical. Compiler text grows 16,512 bytes (1.08%);
new execution/index work scales with actual visits/children. PA16/O0 has no
mandated numeric latency/RSS/text ceiling; historical PA14/15 diagnostic budgets
are not accumulated gates. Evaluator limits and course requirements remain.

Final serial `make test-pa16`: **63/154**, expected exit 2 for 91 unfinished cases.
`make test-report-through-pa15`: **2112/2112**, exit 0. Through PA16:
**2175/2266**, failures confined to PA16. File audit passes with three inherited
header-ownership warnings. Personal PA16 suites pass **40 native + 23 rejection**
controls; PA15 execution and final-audit controls pass **41 native + 29 rejection**.
[Evidence verifier](../student.tests/pa16/verify.py) checks frozen hashes, unchanged
contract coverage, failure-set progress, measurements and current source/binary.
Root reports share `.test_counts`; concurrent report totals were discarded and
both required reports rerun serially with full coverage. Original logs remain.

## Handoff ledger / independent review

| Increment | Implementation disposition |
|---|---|
| `d7594d63` | Scalar execution, floating values, array reads, initializer validity and tag lookup; 18 existing PA16 failures fixed. |
| `482c6b44` | Fixed seven nested-definition regressions exposed by tag-category preservation; full earlier coverage restored. |
| `7484f22b` | Completed empty scalar initialization and committed frozen benchmark harness. |
| Loop 37 evidence handoff | All intended changes committed, required progress/prior/file checks verified; remaining owners listed above. |

Independent review remains pending for the accumulated PA16 change: retrace
source/query/body activation identity, mutation cache validity, target floating
representation/conversions, array range ownership and evidence acceptance.
These are review obligations distinct from the known unfinished groups. Neither
review marker is advanced by this implementation handoff; whole-stage audit must
resolve findings before advancement.
