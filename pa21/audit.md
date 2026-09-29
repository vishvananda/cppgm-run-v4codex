# PA21 final whole-stage audit 113

Target: **PA21 full-stage**, phase **audit**. Stage base
`ac988ea33d4997b44e82baaca5a86623fff3127a`; clean entry `f3af4630`;
reviewed implementation `30aec177`. This review reconstructs the current
pipeline from the source and stage changes. The [105](audit105.md) and
[109](audit109.md) checkpoints remain historical evidence, not substitutes
for this review. The [manifest](../student.tests/pa21/audit113-review.json)
retains all 42 stage commit identities, per-commit diff hashes and 93 current
implementation source hashes. The [artifact manifest](../student.tests/pa21/audit113-artifacts.json)
locates archived frozen binaries, complete logs, reducers and native/LowIR outputs.

## Reconstructed design and Spec Alignment

The PA21 production boundary is source → typed LowIR. `lowering/driver.cpp`
creates one preprocessor/cursor/parser/analyzer per translation unit, finishes
semantic demand, and passes those same owners directly to `Procedural`.
`lowir_model::write_program` runs once at the explicitly requested output
boundary. Source/semantic owners die after each translation unit; the shared
typed program and linkage records survive until its required LowIR output.
The supplied backend is used **only by validation and measurement** to inspect
and execute that output. Student MIR, selection, register allocation, ELF/debug
encoding and self-hosting are later-stage owners (PA24 onward, PA34 for inception).

| Spec | Current owner and audit conclusion |
|---|---|
| §1 source/parser | Immutable source buffers, interned identifiers and the streaming cursor feed a single source-faithful graph. Bounded lookahead scans delimiters rather than replaying grammar. Template source regions remain parsed once; context/occurrence edges project them without cloning a second semantic tree. |
| §2 canonical facts | Types, declarations, scopes, template arguments, queries, conversions, layouts and ABI nodes use compact canonical identities. Fact slabs store selected operations and object/lifetime recipes. Manglings are output, never semantic keys. |
| §3 lookup | Name/kind scope indices and explicit parent/base/ADL relationships select candidates. Shape filters precede expensive substitution; checked selected conversions reach lowering by ID. New/delete selection is owned by semantics, including saved placement argument recipes and access checks. |
| §4 demand | `template_instantiation.cpp` uses monotonic declaration/body states, immutable substitution frames and deferred occurrence regions. Unrelated template bodies remain undemanded. The new delete query inspects destructor/deallocator declarations without requesting bodies. Implicit allocation declarations explicitly suspend the enclosing template head. |
| §5 scheduling/caches | Member/body/default/completion queues have entity/fact identities. Query completion invalidates only explicit reverse dependencies and their exception/value facts. RTTI caches begin after semantic completion; expression-effect caches belong to the completed TU. Helper emission now drains its deduplicated growing queue by index. |
| §6 direct typed lowering | Constructors, transfers, captures, list backing storage, RTTI/casts, cleanups and ABI entries consume retained facts. Required missing facts fail invariants. No text transport, semantic name recovery or fake source nodes were introduced. Full LowIR validation is explicit audit work. |
| §7 O0 policy | There is no new optimizer pass. Local effect proofs justify only specific omitted unwind edges/helper argument ordering. Unknown effects retain ordered construction and cleanup. Native profitability and frame costs are measured through the supplied backend; no student allocator/debug claim is made. |
| §8 ownership | Source nodes/facts use TU pools; candidate scratch is local. Lowering release actions/operands, temporary states, saved EH slots, continuation caches and list-address caches are function-owned and reset together. No process-global accumulating cache or owning per-node child graph was added. |
| §9 evidence/budgets | Fixed template, loop/call, memory and floating workloads, affected heap execution, helper nesting and context growth supplement course fixtures. Frozen binaries, source hashes, A/A and ABBA observations are retained in [performance 113](performance113.md). Correctness and mandated limits remain gates; inherited numeric diagnostics do not. |
| §10 self-containment | Source tracing observes exactly one `execve`, no reference reads and no implementation subprocess. All frontend/semantic/LowIR output is student-produced. Host linking and supplied native execution remain the handout's validation boundary. |

## End-to-end traces

The fresh [source-to-ELF record](../student.tests/pa21/audit113-trace-final.json)
follows `Item`, polymorphic `Base`/`Derived`, and demanded `inspect<0>` and
`inspect<1>`. Canonical `initializer_list<Item>` records select const backing
element conversions and destruction; each occurrence owns its backing lifetime.
Closure captures retain source object/conversion identities, while their call
operators consume checked range, RTTI and cast recipes. The selected dynamic
`typeid` reads the vptr; the static type operand supplies canonical RTTI. Throws,
`continue` across a handler, normal returns and backing destruction share the
complete lexical/handler continuation machinery.

Observed: 362 tokens and delimiter visits, 47 maximum pending tokens, 550 parsed
source nodes plus 472 projected occurrences, 37 fixed expressions reused 74
times, two template body transitions for the two demanded `inspect` bodies,
one list-type completion, 15 member demands/15 processed, three RTTI records
and ten hits. The repeated `inspect<0>` call adds no body transition. The result
is 385 typed LowIR instructions, valid native execution and 2,100 bytes of ELF
`.text`. Plain and telemetry/audit output are byte-identical. Saved LowIR,
syscalls and disassembly allow inspection through final encoding.

The allocation trace additionally follows an active-handler template through
implicit global runtime declaration, canonical new/delete queries, selected
constructor/deallocator, saved original allocation/arguments, a completed-array
counter and the handler's exact incoming lifetime prefix. Failure destroys only
completed elements, releases storage, ends any exited handler and reaches the
correct catch. Successful initialization retires only the allocation guard.
Throwing deletion decrements the remaining count **before** each destructor;
remaining elements and storage still have owners if that call throws.

## Findings and repairs

1. Nested omitted aggregates could enqueue helpers during range-for traversal
   of the helper vector, invalidating traversal or leaving a function body
   absent. A stable-index drain emits every demanded helper once.
2. Automatic-array raw unwind paths bypassed same-function handlers. Heap-array
   construction discarded the live enclosing prefix, including active-handler
   templates. Typed prefix ownership now composes those cases with default
   arguments and source continuations. The existing raw whole-loop form remains
   valid when it owns an empty prefix and no default-argument recipe.
3. Failed scalar construction leaked its allocation. Throwing scalar/array
   deletion skipped deallocation and sometimes remaining elements. A flat
   release-action pool extends existing lifetime states; saved typed operands
   prevent reevaluation. Placement matching uses the selected allocator's
   parameter types; an absent matching function correctly means no release.
   Access is checked even for nonthrowing initialization, while body demand
   remains attached to a retained call. Sized delete retains the original size.
4. Template allocation inherited the active template head when synthesizing
   global runtime declarations. New-array type queries selected scalar allocation
   and the wrong construction/result type; delete had no query owner. Declaration
   scope isolation and typed query facts repair those paths. Dependent return
   types also exposed a missing ABI handoff: typed new-expression nodes and
   scalar/array/global delete operations now reach the shared encoder, following
   [Itanium §5.1.6](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#expressions).
5. `independent_initializer` assumed all type-trait operands were unevaluated.
   Polymorphic `typeid(read())` observed an earlier aggregate member before it
   was stored. Its evaluated operand now prevents helper hoisting. The reducer
   uses placement construction and verifies the preceding member value;
   C++11 [expr.typeid]/2–3 and [dcl.init.list]/4 establish the required evaluation.

The current [46 composition controls](../student.tests/pa21/audit113.py) improve
**13/46 → 46/46**, preserving the frozen entry failures. Four access/demand
controls pass. A separately compiled host caller links and executes **14**
dependent allocation/deletion signatures. These controls supplement the
unchanged contract sources and statuses.

One inherited PA12 reference omitted exceptional release around an external
constructor. [Correction 113](reference-corrections113.md) gives the reducer,
C++11 [expr.new]/18–21 proof, pinned bundle revision, exact reconstruction from
committed original bytes and executable original/revised observation. This is
the only contract-path change from audit entry. All six accumulated correction
scripts are checked; no comparison rule or source/status fixture is relaxed.

## Legality, invalidation and work/growth limits

The completed leaf-constructor proof scans only zero-argument bodies with no
subobject actions, once per constructor in the allocation-finalization pass.
Only checked nonthrowing scalar statements qualify; calls, unknown bodies,
default recipes and subobjects retain allocation cleanup. It does not change
the declaration's exception specification or remove the constructor call.
Its local cache dies when finalization ends. Empty-TU/sentinel handling is
covered by the unchanged earlier-stage suite.

Inherited scalar-return proofs, initializer independence, helper transfer
commutation and suffix sharing were rechecked with their consumers. The
`typeid` repair narrows a failed legality proof. Mutable/aliased/volatile or
effectful sources keep ordered initialization; unknown or virtual call targets
cannot borrow a direct-body proof. Suffix sharing keys include lifetime state,
terminal and complete try/handler context, including exit operations. No global
retry, global cache flush, iterative whole-body optimization or code cloning is
used. Source type-query initializer form is now part of the canonical key and
therefore remains distinct through substitution and ABI publication.

Small automatic-array expansion and list-address reuse remain capped at eight;
larger counts use constant-size loops. Aggregate repetition carries the same
pipeline expansion budget. Deleting-destructor sharing delegates larger
nontrivial member sets to the complete destructor instead of expanding quadratic
suffixes. New release state is O(selected arguments + lifetime actions), and
contextual heap loops add a constant number of owners irrespective of runtime
extent. Helper emission is O(demanded helpers + emitted actions), including
newly requested nested helpers. These are work/output bounds, not claims of
faster executables. Conservative fallback retains valid ordered typed IR.

## Validation and handoff ledger

The final required file audit passes with the same three inherited header
organization warnings. The root through report passes **3712/3712** comparisons
and **21/21** stages, plus its separate PA10 **5**, PA11 **4** and PA12 **13**
property checks. The root counter and these property counts are recorded
separately; coverage hashes, rather than the entry prompt's aggregate count,
establish that no test was removed. Final controls, gates, compiler hashes,
reference observations and unchanged contract inventory are recorded in the
[gate/coverage history](../student.tests/pa21/audit113-validation.json),
[inherited rerun](../student.tests/pa21/validation113.json) and review manifest.

The complete inherited personal rerun passes **687/690** original lanes and
**2/2** additional hosted lifecycle checks, retaining its three known supplied-backend
limitations: one freestanding RTTI case passes its hosted counterpart; two
PA16 multi-TU lifecycle cases retain byte-identical entry/final LowIR and pass
the hosted lane, while the freestanding backend fails to resolve `__builtin_abort`
despite the retained `object=abort` metadata. Their original failures are kept;
no course failure is waived.

102–104 established RTTI/casts, captures/copies and list demand (71/116).
105 reviewed through `f65eae8d`. 106–108 added EH, full-expression and destination
ownership (108/116). 109 reviewed through `f57bdd3b`, repairing protected jumps.
This review includes **all** subsequent handoffs: 110's `6b3aecdb`, `bdfdb31b`
and `df6e8299` handler/static/array ownership and helper sharing; 111's
`3028366d` completed-body effect proof; 112's `e32e9f2f` exhaustive-dispatch
summary and reference alignment, plus their evidence/documentation commits
through `f3af4630`. Audit 113 repairs the cross-owner defects above at `30aec177`.
No PA21 implementation handoff remains unaudited. Later object-model, optimizer,
native/debug and inception work remains assigned to its owning milestones.
