# PA12 final architecture audit

Audited range: stage base `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
through entry `18ef3757`, with implementation fixes in `b7a15e3e` and final projection follow-up `0768b868`.
This review read the handout, `spec.md`, testing contract, stage commits and
implementation independently; checkpoint conclusions were inputs to verify.
The compact [plan](plan.md) records the final disposition. All PA12 handoffs,
including parameter representation and scalar consumption after the last
checkpoint, are covered below. No unaudited PA12 handoff remains.

## Scope and production architecture

PA12 implements nonvirtual class values and emits O0 LowIR. The compiler itself
owns preprocessing, parsing, semantics, ABI decisions and typed lowering.
The PA8 native backend is invoked by the external validation/benchmark harness,
as this handout requires. Native selection, allocation, MIR/debug encoding,
ELF writing and self-hosting belong to later assignments. This audit traces
executable behavior through that explicit backend boundary; it does not claim
an implemented PA12 source-to-own-ELF pipeline or a template-body instantiator.
The handout excludes general member pointers and full temporary-materialization
coverage. This implementation already supports the nonvirtual member-pointer
values used by its value/storage fixtures; the audit checks those paths without
claiming general member-pointer support (for example, a global member-pointer
initializer is still unsupported).

| Spec surface | Reconstructed ownership and evidence |
| --- | --- |
| Source and parsing (§1) | Immutable `SourceBuffer` bytes and compact file/offset locations survive in the translation unit. The preprocessor owns input/expansion slabs and exposes a cursor through `PostTokenCursor`; the syntax cursor uses a bounded ring and retains tokens for deferred class grammar. `syntax/parser.cpp`, `class_parser.cpp`, `cursor.cpp` and `semantic/declaration.cpp` cooperate through `Analyzer::consume` on one flat `Ast`/`NodeId` graph. Inline class regions are parsed once, then checked after class completion on demand. There is no second copied semantic syntax tree or successive whole-stream token ownership. |
| Canonical identity (§2) | `semantic/model.{h,cpp}` and `IdIndex` indexes use compact entity/scope/node/type identities. Types intern structural kind, cv, child, class entity, extent and function parameters. ABI spelling is a terminal rendering of canonical entities, not a lookup or conversion key. Rare class/field facts live beside the common entity record. |
| Lookup and rejection (§3) | `lookup.cpp`, `overload.cpp`, `operator_call.cpp`, `conversion.cpp` and `template_call.cpp` follow indexed lexical, using, base, friend and ADL edges. Arity, member/ref qualification and template shape filter candidates before conversion; every language-required viable candidate is ranked. Built-in operator alternatives derive from reachable operand types. Expected nonviability returns an invalid conversion/candidate; final diagnostics are outside that hot rejection path. |
| Demand and templates (§4–5) | `member.cpp`, `declaration.cpp`, class completion and template-call owners distinguish queued/active/completed work, declaration queries and emission. The queue cursor processes deduplicated entity demands; an incremental parameter-query cursor does not restart the entity list. Canonical template pattern plus interned argument pack identifies a demanded declaration specialization in its fixed pattern environment. Local binding/cache overlays substitute dependent type facts; nondependent facts are reused. This inherited declaration/signature surface is what PA12's fixed template corpus exercises; template definition instantiation remains future work. |
| Typed facts and lowering (§6) | `lowering/driver.cpp`, `procedural.h`, `construction.cpp`, `transfers.cpp`, `branch_lifetimes.cpp`, `zero_initialization.cpp` and `array_allocation.cpp` consume selected entities, conversions, layout/actions, lifetime IDs and ABI records. `lowir::Program`/`FunctionBuilder` construct typed instructions directly. Missing zero plans are invariant failures. No fake AST, name-based semantic recovery, emitted-text reparse or delegated compiler appears on this path. Full validation is an explicit audit option; writer output is a requested view. |
| Policies and native boundary (§7) | O0 retains required construction, copy/elision, cleanup and representation boundaries. Optional omission/sharing uses selected semantic proofs and bounded local work, as detailed below. There is no hidden fixed-point optimizer, native allocator or production binary patch. Benchmarks compile student-generated LowIR with the supplied backend and check executed outcomes. |
| Allocation and lifetime (§8) | Source/AST/entity/type/action pools are translation-unit owned flat storage; query scratch is short-lived. Function instruction builders, temporary state, selectors and reference guards reset per function. Completed translation units are released after lowering; the output `Program` and canonical linkage graph remain until requested combined LowIR is written. There is no owning `shared_ptr` per semantic node, recursive graph destruction or accumulating process-global semantic cache. |
| Work and self-containment (§9–10) | Registered source sets build the shared implementation. Work counters observe parsing, lookup, conversions, demand, actions, lifetime states and emitted IR separately from benchmark timing. Searches and direct driver review found no reference/host compiler invocation, textual IR transport or fixture-name dispatch in production semantics/lowering. Fixed calls, memory, floating, reference, template and PA12 object workloads measure all applicable dimensions. |

The table's source names are relative to `dev/src/semantic/` unless qualified.
The three existing file-audit advisories concern substantial header bodies in
`semantic/analyzer.h`, `semantic/model.h` and `lowering/procedural.h`. They do not
indicate transport through text or copied semantic ownership; the required
audit exits successfully. No new implementation translation unit was needed
for the final fixes, so no source-set registration change was needed.

## Representative end-to-end traces and repaired defects

### Reference to a temporary's subobject

`audit-reference-choices.cpp` binds a local reference through dynamic and nested
conditional glvalues to a field of a freshly constructed class. Parser nodes
retain the original member/conditional expression. Selected conversions and
`object_fact` identify the complete temporary object; the reference declaration
has its own canonical entity. Previously only a directly bound temporary was
retained. A projected member's containing object was destroyed at the end of
the initializer, leaving the reference dangling. The reduced executable's live
object count detects the premature destruction without reading a dead object.

`semantic/reference_storage.cpp` now follows only lifetime-preserving built-in
projections: parentheses, qualifying reference casts, non-reference/nonstatic
member access, either operand order of array subscript, data-member-pointer
`.*` access and comma. Conversion to a separate scalar
temporary stops the containing-object walk. User conversion returning a
reference, function return, pointer subscript and reference members do not
extend the receiver. Static reference storage shares that projection query;
this also removes incorrect extension through a static reference's reference
member. These positive and negative cases have separate executable reducers.

A conditional binding records compact `{object,next}` alternatives owned by
the reference entity. `jump_validation.cpp` creates one lexical lifetime entry
and associates its alternatives. Lowering initializes guards before evaluation,
sets only the successfully constructed alternative, and destroys that object
at scope exit, return, break or continue. Cleanup obtains the object slot's
address in the cleanup block, not a branch-defined cached address. Conditional
alternatives involving existing objects therefore perform no new destruction.
A final operand-form cross-check found the reversed subscript and `.*` cases
still ended the containing lifetime too early; the same semantic projection
owner now covers them, with positive and pointer-nonextension reducers.
Single temporary bindings keep the existing direct path. The final LowIR is
validated and executed through the native boundary, including nested choices
and all four exit paths.

C++11 [§12.2/5](https://timsong-cpp.github.io/cppwp/n3337/class.temporary#5)
requires the complete temporary to persist when the reference binds to one of
its subobjects, subject to the stated lifetime exceptions. These tests use
ordinary local/static declarations, not the new-initializer or return-reference
exceptions. No lifetime is extended merely because a function or reference
member happens to refer to an object.

### Qualified list initialization

`audit-volatile-list.cpp` declares both ordinary and volatile rvalue references
initialized by a list, including a nested array aggregate. The old reference
list path stripped cv qualification before aggregate planning; the resulting
temporary and helper emitted ordinary stores. `semantic/list_initialization.cpp`
now retains the complete target `TypeId` through the list plan, selected member
conversions, temporary, helper key and typed store. Ordinary and volatile
helpers remain distinct. The property check follows helper calls from `main`
and counts three executed volatile scalar stores, rather than relying on the
number of textual store instructions.

C++11 [§8.5.4/3](https://timsong-cpp.github.io/cppwp/n3337/dcl.init.list#3)
specifies a temporary of the type referenced by the reference for this
list-initialization case. The [LowIR volatile initialization contract](../pa8/lowir.md)
distinguishes explicit volatile scalar value stores from preliminary class
storage zeroing. The fix retains this distinction; it does not mark every
storage byte operation volatile.

### Typed heap zeroing

`audit-heap-zero.cpp` allocates a runtime-sized value-initialized array of data
member pointers and an array of classes containing them. Previously the array
allocation path bypassed prepared typed zero actions and cleared the storage
with zero bytes. That is not a null data-member-pointer representation.
C++11 [§8.5/6,8](https://timsong-cpp.github.io/cppwp/n3337/dcl.init)
requires scalar zero-initialization via conversion of zero and recursive member
initialization for these classes. The applicable
[Itanium ABI §2.3.1](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#data-member-pointers)
represents a null data member pointer as `-1`, not zero.

`semantic/placement_new.cpp` prepares a zero-plan identity for the leaf type.
`lowering/array_allocation.cpp` consumes that plan: byte-compatible cases retain
their bulk loop; nonzero null representations advance by the leaf extent and
emit typed zero actions. Scalar volatile initialization preserves volatile
stores, including member-pointer parts. Class preliminary zeroing keeps its
distinct storage semantics. Lowering never reconstructs null representation
from source spelling or substitutes a memset answer.

Native reducers check every initialized element. A separate IR property uses
extents 19 and 1,000,000 and gets 25 instructions in both cases: runtime extent
does not replicate compiler IR. The fixed executable benchmark additionally
checks the null representation and a varying result across repeated allocations.

### Transfers, ABI queries and the inherited template boundary

The bit-field constructor reducer traces declared field type and layout-unit
identity into prepared constructor/transfer actions, then typed unit loads,
masking, declared-type completion and integer promotion. A complete-object
constructor entry and a base constructor entry have distinct ABI identities.
Source signatures retain declared parameters; LowIR adds implicit `this`.
Copy/move constructor source and destination promises are justified by distinct
construction objects; assignment retains conservative aliasing. References and
complete object boundaries can carry extents, ordinary pointers cannot.

`parameter_representation.cpp` queries the selected copy body and source-object
identity independently of whether that body must be emitted. A declaration-only
query therefore determines transport without forcing unrelated definitions.
The cached parameter convention is independent of result transport (the PA12
small-result convention is at most 16 bytes). Explicit tests link separately
lowered declaration/definition translation units and inspect required slots,
copy effects, escapes and pointer constants before executing them.

For a repeated supported call such as `consume(1)`, `template_call.cpp` obtains
one canonical pattern/argument-pack specialization, substitutes dependent
signature facts in a local overlay and reuses that declaration on later calls.
The fixed template-semantics corpus exercises demand and rejection without
requesting unsupported template bodies. The trace ends at the selected typed
declaration for that surface. Repeating the grammar or claiming an executable
template body/ELF trace here would misstate PA12's scope.

## Legality, caches, invalidation and whole-pipeline budgets

Published decisions are consumed after their owning completion step. Transfer
plans have class/kind identity and active/completed state; constructor actions
refer to finalized member layout and selected initializers. Delegation-cycle
checking colors vertices/edges once. List plans key `(NodeId, TypeId)` within
the source node's fixed scope; empty-list access-sensitive plans key
`(TypeId, ScopeId)`. Type-only value/zero shapes use canonical qualified types.
The repaired qualified-list path demonstrates why deleting cv from a key is
invalid. No later lowering mutation changes these semantic keys or facts.

Member-demand state, body-query state, class completion and emission are
separate. Recursive work sees active state. Template dependent caches live
inside the applicable pattern/environment overlay; completed declaration
success/failure belongs to its canonical specialization. Ordinary declaration
insertion uses indexed scope entries and does not reset all caches. There is
no global generation counter, repeated pending-registry scan or broad retry
introduced by PA12. Deferred work is triggered by its owning completion/demand
event; there is no incremental-edit promise beyond a compilation invocation.

| Policy or fact | Legality, budget and conservative result |
| --- | --- |
| Copy prefix and trivial transfer | Only proved representation-safe required storage/union prefixes remain. The optional scalar-only prefix was removed after measured runtime regressions. Source identity/effects and selected transfer facts govern elision; unknown or observable copies retain calls. |
| Empty destructor and no-throw cleanup | An empty body can remove executable effects while an explicit object boundary remains. No-throw cleanup omission consumes selected constructor/action facts; it does not strengthen the public exception specification. A scalar transfer proof rejects calls, class subobjects and unknown operations. |
| Destructor suffix sharing | Inline at most eight actions, hence at most 28 duplicated tail actions; larger suffixes share indexed blocks. Function-owned return terminals survive loop contexts. Branches finish only their private suffix, keeping enclosing temporary state. |
| Aggregate/array and zero actions | Local expansion is capped at eight total nested elements. Zero plans share child extents, cap padding-store expansion at eight, then use bulk operations or loops. Heap typed-zero IR is independent of the runtime element count. |
| Conditional references | One compact alternative per possible materialized object and one lexical binding lifetime; per-function guard state, no full AST copy or source replay. Depth 32/128 yields 33/129 alternatives, 98/386 binding visits and 600/2328 instructions. Fourfold independent-source growth is exactly linear in visits/alternatives/IR. |
| Full-expression and scalar consumption | Classifiers use at most three bytes per AST node; scalar transfer proof adds one lazy byte per examined node. One consumer per selected initializer and sparse observations keyed by modified/exposed object. Final conversion/store precedes destructor observation. Unknown, volatile, modified or aliased conditions retain the shared cleanup path. |
| Member-pointer values | Two-part value/copy representation is bounded at 16 bytes; null and adjustment facts survive typed lowering. No pointer-sized shortcut replaces a function-member-pointer value. |

These are pipeline bounds on consumed nodes, selected actions, lifetime edges
and produced instructions, not independent pass allowances multiplied by
repeated fixed-point scans. Added guards and typed stores are required semantic
work. No optional optimization was added by the audit. All new per-function
state is released with that function; semantic alternatives and zero plans are
released with the translation unit. Telemetry observes these existing events.

The earlier optional policies were reviewed against their actual native
measurements, not accepted from smaller IR. Profitable destructor/elision
changes, removed regressions, remaining necessary ABI/ordering costs and the
supplied-backend layout experiment are reconciled in the
[final performance review](final-audit-performance.md). No unsupported positive
runtime, compiler-text or later-backend gate survives in the current plan.

## Reference preservation and validation

The only historical PA12 reference change is `3da4de09`: four bit-field retypes
in one `.ref`, with its [reducer, C++11/LowIR proof and bundle revision](reference-corrections.md).
This audit checked that exact diff and reran strict validation: the invalid
reducer fails with incompatible operand type, the corrected reducer passes.
The promotion executable passes. The supplied binary bundle, fixtures,
comparison rules and coverage remain unchanged. No new reference correction
was necessary.

Final implementation validation:

- `make test-pa12`: **257/257**, including **13/13** behavioral/query controls.
- `make test-report-through-pa12`: **1584/1584**, **12/12** tracked stages;
  inherited stages **1327/1327**. The entry report and provided Ralph log also
  contain 1584, not the stated 1608. The supplied read-only log is
  `/home/vishvananda/work/.ralph/v4codex-gpt-6-astra-xhigh/last-test.log`.
  No tests were removed to reconcile this.
- `perl scripts/cppgm_file_audit.pl --stage pa12 --paths dev/src`: pass, with the
  same three existing header advisories described above.
- All **72** personal C++ sources via `student.tests/pa12/check.py`; the five new
  audit reducers and bounded-work/volatile checks via `audit_check.py`; the
  existing parameter, terminal-return and zero-initialization property scripts.
- The actual final compiler rebuilt with ASan/UBSan at O1, debug information,
  frame pointers and non-PIE: all 72 sources and audit properties pass without
  sanitizer diagnostics, including expected-rejection sources.
- Frozen benchmark verification checks compiler/source/output hashes,
  observation order/ratios and final native outcomes. Two historical A output
  pairs overwritten in reused scratch are reproduced in separate storage from
  their original frozen compiler/source and match their recorded IR/native
  hashes; no measurement was replaced. `git diff --check` passes.

Logs and frozen artifacts reside under
`$RALPH_ARTIFACT_DIR/pa12-final-audit/`: `entry-through.log`, `fix-stage.log`,
`fix-through.log`, `personal.log`, `audit-check.log`, `sanitize-*.log`,
`performance.log`, `stage-performance.log`, `complete-performance.log`,
`complete-stage-performance.log`, `noise-repeat.log`, `projection-sanitize-*.log`, `verify.log`,
`final-file-audit.log` and `final-through.log`.
Tracked JSON keeps all observations and hashes; scripts reproduce the checks.
Ralph state is read-only. The final audit changes are cohesive correctness and
evidence commits; generated objects, outputs and logs are excluded from git.
