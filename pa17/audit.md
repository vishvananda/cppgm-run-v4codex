# PA17 final full-stage audit — Ralph loop 62

Stage base: `21748547a9e5befaae65e4fae120a63b3f9fcafb`.
Reviewed implementation: `12140852` (entry `0970f3f0`).
Last checkpoint: `2290a7bf8b56bc33e6ad975a8ae714a341f55ce5`.

**Final Spec Alignment: complete for PA17/O0.** The independent source review
found and fixed closure exception/signature defects despite a green course suite.
All required checks and additional controls pass. This is the final stage audit;
PA18 deduction/SFINAE and PA24–PA34 native generation/optimization/self-hosting
remain at their handout boundaries. No PA17 implementation or unaudited handoff
remains. The prior goal turn was progress: committed implementation plus fresh
passing evidence. Process inspection at entry found no unfinished test/build job.

Read `spec.md`, the entire PA17 README, testing/reference rules, project layout,
stage commit history, current source, compact plan, prior audit and handoff
ledgers. Reconstructed the pipeline from its implementation, rather than treating
checkpoint conclusions as proof. The [range manifest](../student.tests/pa17/final-range.json)
binds the entire stage history and every implementation patch, with a separate
last-checkpoint-to-final range. The historical [loop-60 audit](audit-loop60.md)
and [loop-61 plan](plan-loop61.md) remain intact.

## Findings and changes

The closure implementation bypassed the ordinary exception-specification owner.
`[](int n) noexcept(sizeof(n)==4){return n;}` was rejected because lookup used
the enclosing scope without parameters. An enclosing same-named variable could
instead silently supply the wrong type. Class-valued exception constants were
interpreted as nonzero constant-object IDs, bypassing contextual conversion:
`noexcept(B())` was true even when `B::operator bool()` returned false, and
missing, deleted or non-constexpr conversions were accepted. Retained lambda
exception expressions also failed to bind fixed names at template definition,
allowing a later overload to change an earlier expression's meaning.

`semantic/lambda.cpp` now publishes and demands the ordinary
`ExceptionSpecificationFact`. The checked call-operator scope already owns the
raw parameter identities, expanded pack bindings and lexical parent; the shared
evaluator uses it without shadowing parameters with adjusted signature types.
It checks contextual boolean conversion and constant evaluation under an
unevaluated context, with the existing monotonic success/failure owner. It accepts
both parser spellings of the noexcept node. The template body binder records
lambda exception operands after publishing parameters and before binding the body,
so fixed lookup is retained and dependent operands specialize normally. Lowering
and function-pointer adapters consume the resulting effect fact.

The same trace exposed a distinct signature defect: array/function parameters
were retained as raw callable parameters. Valid closure calls were rejected.
The closure now applies `Types::signature` before publishing its call operator,
conversion target, thunk and ABI signature. Raw parameter facts remain available
for body and exception queries, including top-level const. No syntax is rebuilt
or names re-resolved during lowering.

Proof uses the checked-in [N3485](../doc/n3485.txt): **3.3.3/2** includes lambda
parameters in parameter scope; **5.1.2/5–7** defines the operator's signature,
exception specification and lexical context; **8.3.5/5** adjusts array/function
parameters and removes top-level parameter cv from the function type;
**15.4/1** requires contextual conversion to bool and a constant expression;
**14.6.3/1** binds nondependent names at their definition-time use. These are compiler corrections,
not oracle changes. [Thirty reducers and neighbors](../student.tests/pa17/final_controls.py)
include template packs, shadowing, cv/array/function parameters, lexical `this`,
fixed overload lookup, function-pointer adaptation, selected partial definitions,
dormant members, constexpr-to-runtime demand and effectful empty copies.
Entry fails 16/30; final passes 30/30, with successful native execution and ordinary
status-1 rejections. Tests and comparisons were not weakened.

## Whole-stage architecture reconstructed from source

| Spec surface | Current ownership, data flow and audit evidence |
|---|---|
| §§1,8 source and parsing | `preprocess/source.h` owns immutable bytes; source character lookahead is bounded, identifiers enter the interner, macro work uses slabs. `PostTokenCursor` streams into the syntax cursor's ring; token buffers are borrowed, without successive full owning streams. `Parser::translation_unit` calls the analyzer as declarations complete. Parsed source nodes and their locations are retained once; `NodePool::Occurrence` adds only source/context IDs. `Ast::source_region/instantiate` defers bodies/defaults and projects edges, without grammar replay or copied trees. PA5 lexical hints are explicitly required by `pa5/parsing.md` for unresolved syntax fixtures; known declarations override them and semantics diagnoses unresolved uses. |
| §2 canonical graph | `Types::intern/signature`, `intern_arguments`, canonical query graphs and flat `IdIndex` tables own type/entity/head/tuple identity. A class/function/variable specialization is keyed by canonical primary and normalized arguments; member primaries already encode the enclosing class. Frames include specialization, source-parameter slice, count, parent and selected tuple. Rendered names and ABI strings are outputs, not semantic keys. Entity records remain 120 bytes; query records 48 bytes. |
| §3 lookup and selection | Scope/name/kind tables, indexed using/base/injection edges and hidden-friend maps visit relevant owners. Candidate arity and shape filters precede substitution; every viable required candidate is inspected. Class/variable partials visit their primary's list, require exact reconstructed arguments, and use O(C) best-candidate/verification comparisons. Context-free ordering caches candidate identity; default-coverage comparisons stay specific to the actual tuple. Expected matching failure is a compact result, without diagnostic string construction. |
| §4 declarations and demand | `template_definition*` joins source head/prototype identities to selected partial owners; renamed enclosing/member heads use immutable parent-linked frames. Alias, variable, class, member-body, default, exception, constant, layout, vtable and emission facts remain distinct. `explicit_instantiation.cpp` separates suppression from locally used inline/defaulted members. Friend declarations use namespace ownership and indexed friendship/ADL edges, with body demand separate. The trace and dormant-member controls verify no unrelated member definition is instantiated. |
| §5 scheduling and cache validity | `Analyzer::finish` drains deduplicated queues by monotonic cursors: selected-callee edges, key vtables, parameter bodies, static storage, specializations, friends and members. A late definition wakes its member/source edge; no all-class retry occurs. Definition traversal includes the current source-list head. Negative qualified-type entries require a completed qualifier; active default-candidate tuples are temporary recursion guards, not permanent failed facts. Constant activation keys include typed values, receiver paths and storage snapshots. No global invalidation generation or process-global semantic cache. |
| §6 lowering | `lowering/driver.cpp` passes the analyzer and source view directly to `Procedural`; selected calls, conversions, base paths, layouts, lifetimes, initializer images and ABI entities become typed `FunctionBuilder` instructions. Each emitted entity/ABI entry owns a symbol and one body. The LowIR writer is the requested final output; there is no production IR parser/serializer bridge. `--validate-lowir` adds explicit audit validation. Missing facts diagnose invariants, without name-based recovery. |
| §§7,9 optimization | PA17 requires O0 LowIR, not a native optimizer. Reviewed inherited constant branches, sparse/member transfers, zero/array lowering and full-expression grouping. Legality, bounded work/growth and conservative fallbacks are detailed below; runtime/code-size evidence accompanies compiler evidence. Own MIR/selection/allocation/ELF and optimized levels are later-stage surfaces, not waived current requirements. |
| §8 allocation/release | Sources, interners, flat maps, type/query/entity vectors, source nodes and fact slabs are TU-owned and release after its lowering. Candidate/definition/constant scratch frames release at return; substitution frames share parents. Function builders reset per function. Typed LowIR survives only because this tool explicitly emits the whole program. No per-node owning smart pointers, recursive tree destruction, deep environment copies or accumulated process-global caches. The three inherited file-audit header-division warnings are advisory, not hot-node ownership defects. |
| §§9,10 observability/self-containment | Existing `--stats` observes phase time, peak RSS, token/node work, candidates, substitutions, cache hits, fact/body transitions, queues and IR sizes; benchmark instrumentation is untimed and separable. All required preprocessing, semantics, ABI naming and LowIR comes from `dev/`. Driver code has no external frontend/backend invocation on this mode. Reference translation is solely the permitted external validation step. Host compilation builds this compiler; it does not implement requested source output. |

The [new source-to-ELF trace](../student.tests/pa17/final-trace.json) follows
`Owner<int/long>::run`, its renamed out-of-class template heads, the demanded
`Selected<T*>` partial through `Alias<T>`, and two closure signatures through
exception facts and conversion/thunk demand. `Both<0>().Right::get()` supplies a
qualified constant receiver and canonical non-type key. Parsed syntax is shared;
there are 594 parsed nodes, 614 occurrence records, two definition applications,
two function-template body transitions and two closure entities. Eleven body and
lifetime checks agree. Four of eight deferred regions are demanded; no unrequested
template body is lowered. Two cleanup regions protect live `Local` objects around indirect
calls; nonthrowing direct closure calls consume their completed effect facts.

The trace contains source, LowIR, frozen compiler and ELF hashes, all telemetry,
checked native exit and disassembly. Typed add instructions preserve the selected
`sizeof` values 4 and 8; distinct int/long operators and adapters use their proper
signatures. Constant execution checks the right base's value 7. The executable
checks results 14 and 22 and exactly two destructor effects. ELF is produced by
the pinned supplied backend as explicitly allowed for this stage. This is evidence
for source-generated LowIR behavior, not a claim that PA17 implements native ELF.

## Optimization legality, work and profitability

`condition()` consumes an already proven nonvolatile boolean constant in O(1);
other conditions retain normal lowering, effects and ordering. Zero plans carry
representation legality from semantic construction: volatile, references,
member-pointers and nontrivial subobjects retain their distinct actions. Small
padding/union stores are capped at eight; large regions keep bulk zeroing.
Array construction and transfer share an eight-element expansion budget across
dimensions, otherwise use counted loops. Constant execution retains the
1,000,000-work/512-depth limits and conservative failure. No extra optimization
pass, global fixed-point scan, inlining or speculative code growth is introduced.

Full-expression lowering memoizes expression effects and shares cleanup suffixes.
Activation changes only the live lifetime tail. Captureless storage allocation
has no initializer effects; calls still consume their conversions and active
cleanup facts. Empty-copy effect controls confirm argument calls are preserved.
Closure work is one entity/signature/body per occurrence/context, at most two
short demanded adapters proportional to parameters, and one O(L log L) ABI
numbering step. The final exception fix adds one shared fact per explicit
expression, reusing body parameter identities; it does not recheck a body.

The code and measurement review separates necessary semantics from optional
optimization. No unprofitable optional transform remains in this stage. Existing
historical scalar/array/union/cleanup benefits and regressions are preserved in
the linked reports; a reduction in IR alone is never counted as runtime profit.
Source/ABI locations and order-sensitive lifetime actions remain intact. Later
backend register allocation and self-hosting cannot be evaluated as student
implementation here; supplied-backend executable checks cover the applicable
calls, loops, memory and floating-point behavior.

[Final performance evidence](final-performance.md) reports the audit-entry and
whole-stage base comparisons, compiler latency/peak RSS, executable runtime and
code size together. Frozen flags/sources/binaries, A/A calibration, wall-time ABBA
pairs, spread and checked results are retained. PA17/O0 has no mandated numerical
ceiling. Historical +15%, +16 MiB and 5.5× targets are self-selected diagnostics,
not additional exit gates under spec §9. Their measurements remain unchanged;
this classification does not weaken correctness, coverage, timeout limits or
explicit work/growth bounds. Necessary semantic costs are disclosed; avoidable
repeated work was checked at its owning source and counter level.

## References, handoffs and validation

Independently rechecked all six loop-59 corrections in
[storage-references.md](storage-references.md): the reduced programs, original
and surgical corrected output, bundle revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, and N3485 §§3.6.2/2, 5.19/4,
6.7/4, 8.5/6,8 and 14.7.1/1–2,8,10. Static binding/constant initialization,
empty value zeroing and dormant static members follow those rules independently
of compiler agreement. The correction script passes. Dynamic referent effects,
selected functions, distinct specializations and ordered lifecycle actions remain.
No reference, fixture, success status, coverage or comparator changed in loop 62.

All previously unaudited commits are closed: `9efd570f` checkpoint records;
`2827d328` full-expression regions; `bb1d961d` closure, placeholder, demand,
ABI and lowering ownership; `7815c150` lexical `this`; `0970f3f0` handoff evidence.
Reviewed each implementation patch and adjacent consumers, including ABI readers,
writer and encoding. The lambda signature and exception gaps found in this review
are fixed by `12140852`. Earlier stage history is bound by the range manifest and
reviewed through the actual current owners above, with prior findings and
measurements retained rather than silently superseded.

Fresh validation: `make test-pa17` **343/343**;
`make test-report-through-pa17` **2609/2609**, all **17 stages** passing;
`perl scripts/cppgm_file_audit.pl --stage pa17 --paths dev/src` passes with the
same three advisory warnings. The raw course summary excludes separately reported
property controls; the entry tracker reported **2633/2633** in its aggregate. The authoritative log is
preserved with its actual 2609 count, not rewritten to match the aggregate.
All **734 PA17 personal controls** pass (610 inherited, 71 closure, 10 lifetime,
5 region predicates, 8 closure ABI, 30 new audit controls). PA9 API, 117
status/serialization, valid/invalid, order and final-boundary checks also pass.
The ABI runner initially received the command-line tool instead of its API test
binary; that invocation error was corrected by building and running the intended
API harness, with both logs retained. It was not a compiler failure.

The [final evidence](../student.tests/pa17/final-evidence.json) binds source trees,
unchanged course inputs, historical artifact hashes, required logs/statuses,
controls, trace, measurements and the review range. Run
`python3 student.tests/pa17/verify_final.py` to check the frozen record.

| Ledger | Disposition |
|---|---|
| 48, 52, 56 / checkpoint audits | Earlier ownership/category/failure findings and all historical observations retained in their linked records. |
| 57–60 / implementation and audit | Transfer/query/storage owners and qualified receiver/constant/emission corrections; six proved oracle edits; loop 60 ended with three implementation failures. |
| 61 / implementation | Closed those three cases with full-expression and captureless-closure ownership; handoff review was still pending. |
| 62 / final audit | Independent whole-stage review; fixed exception scope/conversion/fixed lookup and canonical closure signatures; required checks, 734 controls, ABI/references/trace and stage-scoped performance accepted. No remaining PA17 defect or unaudited handoff. |
