# PA18 full-stage final audit 89

Stage base: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Previous reviewed code: `f6eaf8ab213c90eefffd48f5c47b1ac9a75bce8d`.
Last reviewed commit: `f2558ff473410f95de7af49469f404ebf8362e34`.
Disposition: **PA18 full-stage passes; Spec Alignment: aligned for PA18/O0**.
PA19 has not been started. [Audit 86](audit86.md) preserves the previous record
verbatim; audits [82](audit82.md), [78](audit78.md) and their earlier links retain
the accumulated review history. The preceding cleanup turn was progress: it
committed the pending repair as `56e17b79`. Process inspection found no live
inherited job to restart or wait on.

[The evidence manifest](../student.tests/pa18/loop89-evidence.json) binds reviewed
sources, binaries, checks, coverage, controls, trace and measurements. The review
covers the complete combined change since audit 86: class ellipsis conversion,
query/default/effect demand, prvalue storage, floating zero, class-result oracles,
and extensible explicit-pack deduction. No implementation source was added, so
no source-set registration was necessary.

## Findings and repairs

**Explicit packs were treated as completed deductions.** Binding a supplied
pack before deduction rejected valid extensions such as taking `f<int>`'s address
as `int(*)(int,long)`. Moving the immutable explicit prefix through both call and
target deduction, and through nested type/argument sequences, fixes all consumers.
Completed deductions remain separate and must agree across repeated occurrences;
conflicts still discard a candidate. An unused explicit pack retains its supplied
arguments. N3485 §14.8.1 [temp.arg.explicit]/9, in
[the supplied draft](../doc/n3485.txt:20350), expressly permits prefix extension.
This is general deduction, not a fixture or library-name exception.

The [51 audit controls](../student.tests/pa18/audit88_controls.py) improve
**38/51 → 51/51** on identical source hashes. They cover direct calls, pointers,
references, overload sets, nested packs, non-type arguments, members, constant
queries, SFINAE conflicts and class ellipsis lifetime/default behavior. The
new controls supplement the unchanged course suite.

**Prefix-frame creation included unrelated scalar explicit arguments.** The first
frozen run exposed one extra frame and argument tuple per specialization on
pack-bound and scalar-query workloads. `d2980f04` creates the frame only when an
explicitly supplied pack needs it. Ordinary and target deduction share the same
rule. Calls without arguments also retain supplied packs without a deduction frame.
This removes avoidable work without changing selected types or output.
Both the initial observations and the final measurements remain in
[performance 89](performance89.md).

**The class-boundary handoff is sound across its consumers.** Review followed
`ellipsis_conversion_value`, `valid_fixed_conversion`, concrete materialization,
exception queries, both constant evaluators and branch cleanup. The chosen
conversion stays a semantic fact. Unevaluated class glvalues suppress their
lvalue-to-rvalue copy; defaults are demanded by effects or evaluation. Evaluated
unavailable transfers diagnose failure. Prvalues pass their existing result
object; branch cleanup destroys only constructed temporaries. LowIR variadic
scalar lanes carry caller-owned object addresses under the documented PA18
class-ellipsis policy. Hosted class `va_arg` is a later-stage boundary. Typed
floating zero uses the canonical constant representation.

## Spec Alignment and actual data flow

| Spec requirement | Reviewed implementation and evidence |
|---|---|
| §§1–2: immutable input, streaming and canonical graph | `lowering/driver.cpp` directly connects `Preprocessor`, `PostTokenCursor`, syntax cursor/parser and `Analyzer`. Immutable `SourceBuffer` and interned identifiers survive through TU consumption. Parser and semantics cooperate; retained source nodes have compact source/context occurrences in `syntax/occurrence.cpp`, not cloned syntax or replayed tokens. |
| §§2–4: complete identities, candidates and substitution | `template_call.cpp`, `template_ordering.cpp`, `template_deduction.cpp` and `template_packs.cpp` consume typed entities, canonical arguments and types. Explicit prefixes and completed specializations have distinct indexes. Pattern identity includes its enclosing specialization; immutable frames key every environment component. Candidate rejection returns state/zero; body errors remain hard errors. The arity/member/template filters preserve required candidates. |
| §§4–5: separate demand, caches and invalidation | `template_instantiation.cpp` uses NotStarted/Active/Success/Failure body states. Declaration, class, defaults, effects and emission retain separate owners. `template_type_facts.cpp` interns parent-linked frames and publishes concrete declarations. `query_dependencies.cpp` records typed reverse edges and revises only affected completed queries. No global generation or whole-program retry is introduced. |
| §6: direct typed lowering | Selected conversion recipes, object identities, class-result classification, lifetimes and ABI facts feed `values.cpp`, `user_conversions.cpp`, function declarations and branch cleanup. Definitions and indirect signatures use the same canonical class result. Missing prepared conversions are invariant errors. LowIR serialization is the explicit PA18 output adapter; production does not reparse it. |
| §§7,9: bounded O0 work and useful optimization | `conversion_result.cpp` accepts only a requested completed scalar conversion returning an established nonvolatile named constant, with at most eight wrappers. Unknown/effectful/reference/virtual cases retain calls. Lowering preserves receiver effects, second conversions and storage. Empty-helper omission has one action-head check and zero growth. Existing constant-evaluation and array-expansion budgets remain. Inherited legality/emission inspections and cumulative runtime measurements pass. |
| §8: allocation and release | Dense TU-owned arenas/vectors and `IdIndex` hold source occurrences, semantic facts, canonical identities and query state. Candidate scratch is local; immutable frames are shared. Per-function lowering state resets at its existing boundary. The driver's TU scope releases frontend/lowering owners before the next input; only typed program/linkage data survives to output. No owning hot-node graph or process-global semantic cache was added. |
| §10: self-containment | The production emit path calls this compiler's shared phases directly. External process support is confined to the explicit test runner; the supplied backend is invoked only by validation/performance harnesses. There is no reference compiler, cached answer, filename or source-snippet dispatch in required output generation. |

[The representative trace](../student.tests/pa18/audit88_trace.cpp) follows the
ordinary nontrivial `Packet` declaration and demanded `dispatch<int,long>` from
source through canonical deduction, body demand, selected copy and cleanup.
Taking `dispatch<int>`'s address extends the prefix once. `sink(p)` selects its
copy recipe and the potentially throwing effect; concrete lowering creates the
private argument object and its cleanup. `make<Result>` uses the same direct
`obj<4x4>` result for its definition and indirect call despite the alias spelling.

The final trace records **253 tokens, 445 parsed nodes, 142 occurrence records,
two body transitions, zero query-completion invalidations, 117 instructions and
172 operands**. Inspection confirms one `dispatch` body and the selected copy,
full-expression argument destruction and final local destruction. Student LowIR
validates; the supplied backend's ELF exits zero, checking result **14**, exactly
one copy and two destructions. All eight inherited traces also pass.

The named-result optimization is traced separately through the retained result81
and audit82 inputs: one completed constant fact, bounded legality inspection,
recorded result, receiver-preserving lowering and supplied-backend execution.
Performance 82/86 retains its original profitability and empty-helper evidence;
this audit checks that the cumulative corpus keeps those executable bytes.
Native instruction selection/allocation/ELF/debug ownership and self-hosting are
later stages; the explicit backend test boundary is not production delegation.

## Reference corrections and coverage

Exactly **29** fixture paths differ from the PA18 stage base. All have retained
reducers and cited proofs: [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md),
[79](reference-correction79.md), [83](reference-correction83.md),
[84](reference-correction84.md), [85](reference-correction85.md) and
[87](reference-correction87.md). The manifests compose the shared 84/87 oracle
in order. Array images and empty-object zeroing follow cumulative PA16/PA11
contract requirements; static initialization, demand, discarded values and
rejections follow the cited C++11 clauses; PA9 spelling follows its ABI contract.

Independent review accepts proof 87's distinction: the original isolated direct
calls execute correctly, but alias spelling causes inconsistent boundaries for
the same exact function-pointer type. C++ does not mandate a particular LowIR
spelling for an isolated call. Canonical class-result identity and coherent
call/definition boundaries justify the correction. Fresh execution reproduces
all **24** student/bundle/oracle observations, including bundle pointer failures
and successful original direct calls. The reconstruction reads entry oracles,
never student output, and preserves earlier empty-tag zeroing.

All PA1–PA18 fixture paths and all **420 PA18 inputs / 1,686 fixture paths** are
retained. Scripts, Makefiles, comparator, required behavior and coverage are
unchanged from the stage base. The pinned bundle remains
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`; no new reference correction was made
in this audit. The defined discarded-reference reducer, not the original
null-reference fixture's execution, supplies that correction's language proof.

## Validation, performance and final disposition

[validate89.py](../student.tests/pa18/validate89.py) passes **65 explicit checks**
on final code, including **1,556 personal cases in 36 result-bearing suites**,
scaling/completion, ABI and representation inspections, nine traces, reference
reconstruction and fixture preservation. Required commands pass:

- `make test-pa18`: **420/420**, exit 0.
- `make test-report-through-pa17`: **2609/2609**, exit 0.
- `make test-report-through-pa18`: **3029/3029**, all **18 stages**, exit 0;
  PA10/11/12 additionally report **22** passing focused properties.
- `perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src`: exit 0;
  the same three substantial-header advisories, no errors.

The earlier **3053** tally counts every tracked `.t`, including 18 PA8 later
native-debug inputs and six PA9 top-level inputs outside the default roots.
The fresh root report above is the course exit evidence; no test or count was
changed to obtain it. Two preserved exploratory PA21 controls remain `LATER`
and are not counted as PA18 passes.

[Performance 89](performance89.md) reports compiler latency/RSS and checked
executable runtime/payload together over **54 workloads**, with frozen hashes,
A/A calibration, ABBA blocks and all observations. The initial timing concern
on ordering-600 did not reproduce on identical binaries. The actual extra-frame
work was removed. Necessary new deduction costs remain bounded; unchanged
workloads retain exact output. PA18/O0 has no mandated numeric latency/RSS ceiling;
historical +15%, +16 MiB and 5.5× targets remain diagnostics. No correctness,
coverage, comparison, work-bound or optional-profitability requirement is waived.

No unresolved PA18 correctness, architecture, self-containment, timeout or
file-audit finding remains. [The compact plan](plan.md) records the final review
marker and stage-scoped acceptance. Implementation and evidence are committed;
a clean worktree is checked after the closing record commit.
