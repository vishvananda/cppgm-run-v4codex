# PA18 checkpoint audit 74

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Previous Last reviewed commit: `f59e8f67cd8c832361130aef9af1a0337b25c45d`.
Entry: `2433d6de4f40148d561a1aae89b586a007e9513c`.
Last reviewed commit: `8dc4636d23a38f2bbcc8662b88b07f7979715c2d`.

**Checkpoint audit passes; PA18 full-stage remains incomplete.** This review
covers every accumulated commit since the previous reviewed code tip, their
combined changes, and interactions across handoffs 71–73. The final range changes
52 implementation/registration files. The preceding audit is preserved verbatim
in [audit70.md](audit70.md); [audit66.md](audit66.md) covers the first stage review.
[Evidence](../student.tests/pa18/loop74-evidence.json) records full commit IDs,
patch/source hashes, checks, failure sets, coverage, controls and frozen artifacts.

## Complete range

| Commits in chronological order | Review and interactions |
|---|---|
| `2eb83de5` | Prior audit records and retained stage obligations; establishes the code/records boundary. |
| `b6287928`, `ca314a72` | Context/signature implementation and handoff 71: lexical versus concrete frames, source-parameter identities, decltype query expansions, renamed definition attachment and structural signature keys. Reviewed body/access consumers and original measurements. |
| `35afaec1`, `2539a319`, `57ee1f2e`, `12cbfe83` | All three alias/pack implementation commits, not just the final handoff: erased/default formation obligations, correlated captures, symbolic prefixes, transparent applied types, explicit argument roles and signature/ABI consumers. Reviewed substitutions across nested owner frames, pack length failure and ordinary call/query use. |
| `9ebc507c`, `2433d6de` | Callable/prototype implementation and handoff 73: shared invocation selection, actual callee facts, source projection, constexpr function references, immediate conversion rejection, surrogate exception effects, prototype object contexts, using exposure/hiding and ABI identity. Findings below repair interacting ownership gaps. |
| `f4b9020b`, `8dc4636d` | Audit repairs, independent controls, combined trace and benchmark harness. Code was validated before each commit. This record commit changes no implementation or test code. |

Historical handoff source hashes were checked against their respective code tips,
and record hashes against their record commits. Their check logs, fixture hashes
and frozen binaries were also verified. Current files are not substituted for
historical evidence. Handoffs 71–73 improved the stage from 353 to 379 passing
fixtures; this audit retains all 26 repaired paths and adds no course failures.

## Findings and ownership repairs

**A — parentheses lost the implicit object and template-id in retained calls.**
`query_call` treated the outer parenthesis as the callee, losing prototype-member
receiver facts and explicit template arguments. Dependent `type_query` wrappers
also failed to propagate selected facts, and constant execution could omit the
member receiver. Consumers now unwrap parentheses to read canonical callee facts
while retaining the expression's type/category and actual object. Only an
unparenthesized ordinary name enables ADL. Reduced controls cover implicit and
explicit objects, cv/ref qualification, explicit member template arguments,
constexpr calls and suppressed ADL. N3485 §5.1.1 [expr.prim.general]/6 preserves
the parenthesized expression's semantics; §3.4.2 [basic.lookup.argdep]/1 defines
the unqualified-id ADL condition.

**B — exception specifications reconstructed incomplete prototype environments.**
Ordinary member declarations with no named parameters had no prototype object
context. Templates projected exception syntax and recreated scalar parameter
bindings, losing pack identities, outer frames and declared cv. The ordinary
path also used adjusted function signature types, incorrectly dropping top-level
`const` from parameter names.

The exception owner now retains a source `QueryId` per declaration exception
fact. Projected member occurrences resolve their original declaration fact and
reuse that query; substitution consumes the complete parent/source/local frame.
Prototype parameters read their already-established raw declared types and
ordinal/pack identities; `parameter_body_type` applies the array/function
adjustments without erasing parameter cv. Ordinary declarations use the same
raw-type rule. Prototype object context records class, cv and static availability.
The existing exception fact retains not-started/active/success/failure state and
memoizes each demanded specialization result. Bodies are never required merely
to bind parameter names. A missing source declaration or type is an invariant
violation, not a parsing or name-search fallback.

Controls cover ordinary/static/out-of-class functions, throwing versus nonthrowing
cv overloads, raw cv, array adjustments, empty and correlated outer/inner packs,
repeated specializations, and unused poison bodies. Proof: N3485 §5.1.1/3,
§8.3.5 [dcl.fct]/5 (function-type cv removal does not alter parameter types),
§14.5.3 [temp.variadic], and §15.4 [except.spec]. The new implementation source
[exception_query.cpp](../dev/src/semantic/exception_query.cpp) is registered in
`dev/frontend_source_sets.mk`.

**C — conversion using-declarations lost their access owner.**
Using lookup did not recognize conversion-function-ids. Later conversion checks
could also reject a publicly exposed conversion because the ABI object adjustment
crossed a private base, even after member access had succeeded through the class
introducing the using-declaration. `declaration` now uses typed conversion lookup,
without deducing conversion-function-template specializations. The recorded using
access owner governs member access; ordinary and query conversions consume the
selected canonical declaration and base adjustment without repeating an unrelated
derived-to-base language conversion check. Unexposed private bases, private or
deleted declarations, and attempts to name conversion template specializations
remain rejected. Native controls check scalar/pointer/surrogate conversion and
side effects exactly once. N3485 §7.3.3 [namespace.udecl]/3–4,17–18 and
§14.5.2 [temp.mem]/7 establish these lookup and access boundaries.

**D — indirect and surrogate calls accepted abstract by-value parameters.**
The shared direct callable path had the check; ordinary indirect/surrogate calls
and retained query counterparts did not. Each selected-call consumer now rejects
abstract by-value parameters. Query failure remains structured immediate-context
rejection; ordinary invalid use is diagnosed. Abstract references remain valid,
and abstract return types in `decltype` retain the unevaluated exception. Controls
exercise both callable categories and positive reference/return neighbors.
N3485 §10.4 [class.abstract]/3 and §14.8.2 [temp.deduct] define the distinction.

**E — namespace template conflicts reused the class hiding signature.**
The handoff's canonical using shape always replaced the result type with `void`.
This rejected two namespace function templates with matching parameter lists but
different return types, including transparent alias results. Namespace template
keys now retain return type; class hiding and ordinary function conflicts retain
their different rules. Existing typed template-head/type keys own cached shapes,
with no rendered signatures or global invalidation. Four reduced controls check
both accepted distinct-return templates and rejected matching templates/plain
functions; inherited member hiding is independently retained. The earlier
reviewed binary accepts the distinct-return reducer, while audit entry rejects
it; cumulative performance therefore compares equivalent correct output.
N3485 §7.3.3/14 explicitly includes the return type for namespace templates;
/15 excludes it from inherited member hiding. See [local C++11 draft](../doc/n3485.txt).

[Audit controls](../student.tests/pa18/audit74_controls.py): **60/60 pass**;
**33 fail on frozen audit entry**. Every accepted case validates LowIR and executes
the supplied native backend's output with a checked result. Rejection tests check
status, not diagnostic text. Initial failed reducers and intermediate runs remain
in scratch evidence, including the completed `f4b9020b` performance observation.
No reference correction was needed or made in this range.

## Architecture and optimization trace

[Combined trace](../student.tests/pa18/audit74_trace.cpp) follows ordinary `main`
and demanded `Processor<int,long>::run<int,int>` through correlated `Zip`, a
prototype `this` exception query, parenthesized callable query, builtin invocation,
forwarded function reference and a conversion exposed over a private base.
The real driver retains immutable preprocessing buffers and streams the token
cursor into integrated parser/semantic construction. Occurrence/context IDs
project already parsed source regions; template substitution does not call the
parser. Canonical types, entities, queries, packs and structural signature shapes
are integer identities. Immutable parent-linked frames retain lexical and concrete
owners. Applied alias recipes retain formation obligations even when the result
erases an argument; pack captures use canonical correlated lanes and lengths.

The trace has 319 tokens, 464 parsed nodes and 431 projected occurrences, 69
substitution frames, 74 type-substitution computations, 39 query computations and
one value-query computation. There are two template body transitions, six semantic
body checks and exactly six emitted definitions: `main`, `sum`, the exposed
conversion, fallback and selected `run`, and `forward_call`. The query-only `leaf`
and unused poison body are absent. Completed queries are shared by full query/frame
keys; signature shapes report 28 computations and 17 hits. Repeated completion
controls at 32/128/512 unrelated edges each invalidate exactly one affected query.
No global retry, generation flush, reparsing or rendered semantic key was added.

Selected callable/conversion/object-use facts cross into typed lowering directly.
The source ABI graph retains `this` and parameter identities (four ABI controls
pass), and the shared symbol encoder owns per-symbol substitutions. LowIR contains
69 instructions and 116 operands, validates explicitly, and the supplied backend
produces an ELF whose checked result is zero. The receiver conversion increments
`effects` exactly once. PA18 explicitly requires textual LowIR output; its writer
and the external native harness are tool boundaries, not internal phase transport.
Own machine selection/allocation/encoding belongs to later PAs.

Allocation owners remain explicit: TU vectors/arenas own source facts, canonical
types, occurrence graphs, immutable frames and dense indexes. Local candidate,
substitution and signature scratch dies at the query/call boundary. The new query
cache has TU lifetime and declaration-fact keys; exception results have complete
specialization identity. The driver releases frontend/lowering scratch per TU;
only required typed output and ABI/linkage facts survive to writing. Hot expression
records remain 36 bytes. No per-node owning graph or process-global mutable cache
was introduced. Work follows actual candidates, recipe operands, pack lanes and
demanded declarations; fourfold source experiments check the resulting scaling.

A useful preserved fact is the selected conversion/object identity and its
observable effect. Lowering consumes it once, preserving access, ABI adjustment,
volatile inputs, exception behavior and source locations. These fixes establish
required semantics; they are not optional code transformations. O0 adds no
fixed-point optimization, speculative rewrite, inlining or growth search. Unknown
facts remain conservative. Semantic fact completion owns invalidation before
lowering; lowering never redoes overload resolution. Existing bounded policies
are unchanged. There is no optional transform whose profitability could justify
violating semantics, nor an optimization benefit claimed from IR size alone.
Actual loops/calls/memory/floating workloads and text/payload sizes are measured
in [performance74.md](performance74.md), including checkpoint and cumulative costs.
Native spills, register allocation quality, optimization levels and self-hosting
are later-stage obligations, not fictitious PA18 gates.

## Validation, coverage and remaining work

| Check | Result at reviewed code tip |
|---|---|
| `make test-pa18` | **379/420**, exit 2; exact same 41 entry failure paths: 16 status and 25 LowIR mismatches. |
| Required prior-through command, `n=18; if …; else make test-report-through-pa$((n - 1)); fi` | **2609/2609**, exit 0. |
| `perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src` | Pass, exit 0; same three inherited header-division advisories. |
| Coverage/comparison | All 420 inputs and fixture/reference hashes retained; no changes to course harness, references or comparison rules. |
| Personal semantic controls | **587 inherited + 60 audit**, all pass. |
| Accumulated repaired course paths | **26/26** retained: 25 checked executions and one compile-only contract with an undefined assignment operator; no stub or oracle relaxation. |
| ABI/completion/trace | Four ABI controls, three precise completion controls and validated source-to-ELF trace pass. |
| Performance | Frozen A/A/ABBA compiler/runtime/RSS/text evidence; PA18/O0 stage-scoped acceptance in performance74.md. |

The earlier reference corrections in [65](reference-correction65.md),
[67](reference-correction67.md) and [69](reference-correction69.md) retain their
reducers, rule proofs and bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
There are no new reference changes to approve or justify.

Remaining work stays broad in [plan.md](plan.md): declaration timing/lookup;
constructor and explicit deduction/list-initialization queries; ordinary LowIR
initialization/result policy. The class-ellipsis reducer also remains mandatory.
All 41 course failures remain implementation obligations. Do not advance to PA19
before the root through-PA18 report passes. Splitting context, alias and callable
work across handoffs left closely coupled exception, prototype, access and using
consumers unaudited together and duplicated packaging; those boundaries were
avoidable and should not be repeated for the remaining ownership groups.

| Checkpoint ledger | Range / fixes | Evidence / disposition |
|---|---|---|
| 74, accumulated audit | `f59e8f67` → entry `2433d6de` → `8dc4636d`; every commit across three handoffs, plus five ownership repairs | PA18 379/420, exact same 41 failures; earlier 2609/2609; file audit and coverage pass; 647 semantic controls; stage-scoped performance accepted. Code tip above is the next review baseline; full-stage remains unfinished. |
