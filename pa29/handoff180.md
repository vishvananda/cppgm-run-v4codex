# Implementation180 handoff ledger

Entry `259329eec6b08718a059b5198cb32b4f5b5ac17f`: **385/403**, 18 failures.
Code tip `f613c554`: the completed group is **callable selection and mandatory
inline preparation**. Static call operators and always-inline code generation
fix two existing required failures. Scope also includes static subscripts,
ordinary named static-member ranking, mixed member templates, function-pointer
surrogates, constants, receiver effects, defaults, exceptional returns and the
shared serialized LowIR/native boundary.

## Ownership, data flow and complexity

| Owner | Retained facts and consumers | Work, storage and release |
|---|---|---|
| Operator declarations | Permit static call/index operators; reject other static operators and object cv/ref qualifiers. Existing canonical declarations/signatures retain static identity. | Constant work per declaration; no additional syntax graph. |
| Overload/deduction/query | Exclude the receiver from explicit arguments and deduction. A static receiver compares neutrally with standard object conversions, but beats user-defined surrogate conversion. Mixed member-template ordering compares explicit parameters. | Actual candidate sequences and conversions; existing typed partial-order cache and TU lifetime. No unrelated declaration scans. |
| Selected call | `ObjectUse.node` retains the evaluated receiver, with no implicit-object ABI type. Selected declaration, defaults and argument conversions remain on the call. Constant evaluation and lowering consume the same identities. | One receiver fact per call; normal construction, destruction, access, ref/cv and default-argument machinery. No synthetic frontend nodes or repeated lowering-time resolution. |
| Inline preparation | A shared LowIR transform maps immutable original parameters, slots, values, blocks, phi predecessor exits and returns into the caller. Single-block scalar/void returns splice directly; general results use typed result storage and a continuation. | Linear original traversal plus bounded expansion. Flat context maps, reused operand scratch and geometrically grown pools; context storage dies after each expansion, original IR pools after preparation. No per-instruction heap allocation or text transport. |
| Exception retirement | A per-callee CFG worklist records registration depth at reachable returns. Cloned returns retire the callee's remaining registrations before their continuation. | One analysis per demanded immutable callee, proportional to its blocks/edges/instructions. Return facts use original instruction identity and die with preparation. |
| Admission/native/ELF | Reserve local expansion work before mutation; recursion, depth, frame operations, variadic boundaries and exhausted budgets retain valid calls. Source inspection and native object preparation share the transform; a completed Program is prepared once. Native selection, unwind and ELF writing consume the result. | Depth **64**, reservation **262,144 units/caller**, **4,194,304 units/program**. A unit bounds an instruction, operand or slot; admission includes boundary copies, return cleanup and continuation storage. Nested admission cannot consume the parent's reserved completion work. Actual work must stay below reservation. |

The static callable rules follow the hosted extension in
[P1169R3](https://open-std.org/JTC1/SC22/WG21/docs/papers/2021/p1169r3.html),
including standard-versus-user-defined receiver ranking. Mixed member-template
ordering follows the correction in [CWG2373](https://cplusplus.github.io/CWG/issues/2373.html).
The [LowIR force-inline contract](../pa8/lowir.md) requires eligible direct
same-program expansion while retaining boundary, ABI and recursion checks.
An external declaration has no body to expand. Indirect calls and `no_inline`
remain calls. These are eligibility/fallback policies, not new source acceptance
gates or optional O1–O3 optimization passes.

## Validation and repaired interactions

- PA29 **387/403**, exactly the two original failures removed, no new failures.
  PA1–28 **4538/4538**; through PA29 **4925/4941**. File audit passes with the
  same four inherited warnings. Final command/source binding is in
  [validation](../student.tests/pa29/evidence180/validation.json).
- **47** personal controls pass, with Clang C++11 extension comparison, host
  linking/running and source LowIR validation. Controls cover recursion,
  variadic and dynamic-stack fallbacks as well as ordinary callable semantics.
- **251** inspection commands cover AST, LowIR roundtrip, object adapter,
  machine IR, symbols/relocations, unwind records, execution, debug locations,
  caller/callee phi predecessors and no-return CFGs. Depth/cycle/growth/frame
  controls assert retained calls and reservation bounds. Stats on/off objects
  agree byte-for-byte.
- The rethrow reducer initially exposed a leaked callee exception registration.
  The final per-callee return analysis repairs the shared boundary; the original
  failed control is preserved. An initial inspection demanded identical ELF
  symbol indices after parsing. The reader publishes declarations first, so that
  was an unsupported presentation check: final checks compare instructions,
  symbolic relocations, complete symbol sets and linked execution. No course
  comparison rule changed. The final [adapter evidence](../student.tests/pa29/evidence180/adapter-equivalence.json)
  records all 13 cases, including three with reordered symbol indices.
- All **403** stage inputs and **1,707** tracked contract/harness paths remain
  byte-identical to entry and the preserved review boundary. No fixture or
  reference correction is claimed. [Performance180](performance180.md) records
  compiler latency/RSS, runtime/text size, raw observations and explicit bounds.

## Boundary and independent review

The [16-case ledger](../student.tests/pa29/evidence180/remaining.json) retains
**12** extended syntax/type/layout, **3** template-demand/hosted-ABI and **1**
legacy-trait failures. Thirteen are unfinished implementation; three remain
independent contract questions, still counted failures. Char-traits conversion
remains implementation work. No requirement or review finding is waived.

Further related callable work is closed by the constant/query/runtime, overload,
ABI, exception and budget controls above. Remaining required cases need distinct
BitInt/vector/complex/extended-float representations, array layout, GNU constant
expressions, deduction-guide/coroutine syntax, library conversion or ABI policy;
none consumes the changed callable-selection or inline-remapping rules. Extending
those owners requires a separate semantic group, rather than additional work
within this completed one. This is an incomplete **assignment** handoff with a
completed behavior group, not full-stage certification or advancement.

The stage-base and last-reviewed markers remain unchanged in [plan](plan.md).
[Audit178](audit.md) and its historical findings remain intact for independent
whole-stage review. That review must still resolve all stage findings before
advancement. Implementation commits: `639ebd7f`, `a613b21a`, `95d43769`,
`f613c554`; `6574d6c8` records entry scope. The final evidence/documentation
commit returns control for review.
