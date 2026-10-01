# PA29 compact plan — implementation175 handoff

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `221d6d0e4930da05db2913bdf5f50d808f89c744`.
Audit entry: `914e1a0a07e40884c91c0b967b0421eea9ef0d48`.
Last reviewed commit: `7139ceb5769eea12c6e2e00b54932f09d59c79b5`.
Implementation175 entry: `235ffa3947d7921a1c34c66198f590df5aa77de2`.
Implementation code: `3edfe15e`, `393b19cc`, `7bb44553`, `704538c1`.

## Design/spec alignment and completed group

[Implementation175](implementation175.md) completes source-invocation intrinsics:
FILE/LINE/FUNCTION/COLUMN, direct and qualified calls, macro locations, constexpr
and runtime uses, nested/default arguments, constructor/member initializers,
implicit objects/arrays/statics/new, and query/address diagnostics. Immutable
source sites and conversion-use markers flow into both evaluators. Complete
query keys include invocation/evaluation context and per-query revision. Parsed
regions and checked default recipes remain shared; no grammar replay, semantic
cloning, textual transport or lowering-time overload resolution was added.

Sites and lexical ancestors use indexed identities. Work follows actual nodes,
queries and invocation edges; lowering follows emitted operations. Required string
objects have one emission identity per interned content and are emitted only when
a runtime address is consumed. Evaluated character reads and line-only calls emit
none. TU records and per-evaluation scratch retain explicit release boundaries.

## Validation and performance

PA29 **378/403**: **26 → 25 failures**, exactly one fixed course fixture and no
new failures. PA1–28 **4538/4538**; through PA29 **4916/4941**. All **403** inputs
and **1,707** contract/harness paths remain unchanged. **17** focused controls and
**37** inspection commands plus assertions pass. File audit passes with four
inherited header warnings. [Evidence](../student.tests/pa29/evidence175/validation.json)
includes commands, binary/log hashes, coverage and the exact failure delta.

[Performance175](performance175.md) applies spec §9 to **PA29/O0**. Frozen A/A+ABBA
compiler latency/RSS and checked runtime/text measurements cover the common
frontend/loop/memory/floating/exception workloads. Affected 600/1200/2400-call
inputs provide required-semantics scaling and live runtime checks. Preliminary
measurements are retained. No optional transform or speedup is claimed; optional
optimization work/growth budget is zero. Inherited blanket 15%/zero-growth targets
remain diagnostic rather than extra exit gates. Mandated evaluator/generator,
native capacity and course timeout limits, correctness and coverage are preserved.
Higher optimization, heavier hosted runtime and self-hosting remain PA30–34 work.

## Remaining implementation and independent review

The [remaining ledger](../student.tests/pa29/evidence175/remaining.json) retains
extended syntax/types/layout **17**, template demand/hosted ABI **7**, and legacy
trait **1**. Examples include numeric representations, structured bindings,
conditional explicit/control flow, deduction guides, zero-length arrays, static
receivers and hosted emission. Through-PA29 success is required before PA30.

[Audit174](audit.md) and its [performance evidence](performance174.md) remain the
last independent review. [Audit170](audit170.md) preserves the char-traits,
alignment/dependent-offset/convertible-index reducers and two independent
contract questions: nothrow default-construction shorthand and nothrow-invocable
cache default. Both remain counted failures; neither is waived. The char-traits
case remains unfinished implementation. The new code still requires independent
review; this handoff does not certify the whole stage.

## Handoff ledger and boundary

The previous goal turn was progress (committed audit174), with no live process to
resume. Implementation175 extended the initial intrinsic fix through implicit
construction, invocation-sensitive caches, lazy support-string emission and
deferred-definition demand. The pending165 source-invocation reducer is resolved.
No known required defect remains in this completed group. Other failures require
new numeric/decomposition representations, declaration/template ABI mechanisms,
or a forced-inline transform; the current pipeline only retains that attribute.
Those owners cannot be repaired by further extending source-invocation facts.
This concrete ownership boundary ends the implementation handoff. Stage base and
Last reviewed commit are preserved; mandatory independent review is outstanding.
