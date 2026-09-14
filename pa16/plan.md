# PA16 implementation plan

Stage base commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`
Last reviewed commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`

Target: **PA16 full-stage**. Entry: **45/154 passing, 109 failing**;
through PA15 previously passed. Previous goal turn: progress (PA15 final audit);
current worktree and baseline log revalidated before implementation.

## Design and remaining groups

| Group / semantic owner | Data flow and complexity | Validation |
|---|---|---|
| Constant execution / semantic constant evaluator | Parsed nodes and selected declarations -> typed values and activation frames -> constant bindings, assertions, NTTPs and static initializers. Bound recursive calls and total expression work; retain complete typed cache keys. Implemented arithmetic by-value calls/control flow and floating scalars; references and callable objects remain in object identity group. | Scalar calls, locals/control flow, floating arithmetic/conversions, native personal controls; all course suites. |
| Object evaluation / construction and value facts | Canonical object/subobject identities, constructor/member selections and initializer plans -> object/array/reference values -> typed LowIR data. | Constructors, aggregates, member/operator calls, pointers/references. |
| Declaration validity / declaration and completion facts | Literal-type and constexpr validity at the owning declaration/completion boundary; dependent declaration rules preserved. | Positive and rejection cases, template integration. |
| Exception expressions / exception facts | Selected calls/defaults/destruction -> completed exception facts -> noexcept constant. | Deferred templates, constructors/destructors, static assertions. |
| Static storage / initializer facts and lowering | Declaration identity and constant probe -> constant data or guarded first-use initialization and lifetime actions. | Local scalar/array/class statics, native first-use/destruction. |

Retain the shared parsed graph, TU-owned compact semantic facts and direct typed
LowIR. No production text roundtrip, host delegation or fixture recognition.
Extend related groups while the same ownership analysis supports concrete fixes.

## Performance acceptance

PA16/O0 has no mandated numerical latency/RSS/text ceiling. Preserve inherited
measurements and evaluator work limits. Freeze binaries/inputs for A/A and ABBA
compiler wall/RSS measurements, check equivalent output, and measure native
runtime/text through the supplied PA8 backend where executable output exists.
Separate required semantic costs from optional transforms; document budgets and
avoid unsupported optimization claims. Native backend/optimizer work remains
owned by later stages.

## Handoff ledger

Scalar implementation checkpoint: **63/154**, down from 109 to **91 failures**;
18 existing failures fixed, none added. Personal scalar/floating suites: 35 native
and 23 rejection controls. Frozen performance and final handoff checks pending.
The implementation includes bounded mutable scalar frames, target-rounded floating
values, indexed constexpr array reads, nonthrowing arithmetic probes, static member
initializer validity and declaration-order-independent ordinary/tag lookup.
Whole-stage
independent architecture/correctness/performance review remains pending and is
distinct from the unfinished groups above. Preserve both review markers until
independent review. Record final commands, counts, evidence and any concrete
incomplete-handoff boundary here before returning control.
