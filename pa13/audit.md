# PA13 independent final audit

Target: **PA13 full-stage**, O0 source-to-LowIR. Stage base:
`823e929cdc74fedbb487973dcad33b8a4dce18f4`; audit entry: `cf1b9621`.
The independent review covers the implementations in `9a80791c`, `df2be861`,
`6ca0578c` and their evidence handoff, then the audit fixes `3d39b1cd` and
`6c85368f`. The previous stage-completion turn made progress: it changed the
implementation and supplied passing stage evidence. Its conclusions were not
used as proof of architecture or final correctness here.

## Final Spec Alignment

The controlling sources are `spec.md`, `pa13/README.md`, PA8's LowIR contract,
`TESTING_AND_REFERENCES.md`, and the checked-in contract fixtures. PA13 adds
single-inheritance virtual dispatch to PA12. Its production endpoint is the
requested LowIR view. Template bodies, adjusting thunks, generalized RTTI,
general member pointers (PA22), MIR/selection/allocation/ELF emission, native
optimization levels and self-hosting remain with their explicit later assignment
owners. The existing declaration
specialization surface is reviewed below; no executable-template or student-ELF
trace is claimed for this stage.

| Spec surface | Actual ownership and evidence |
| --- | --- |
| Source and parse (§1) | `preprocess/source.h` owns immutable byte buffers; preprocessing and post-token cursors feed `syntax/Cursor`'s geometric ring. `take` discards consumed compact tokens. Identifier IDs and file/offset locations feed the TU-owned `Ast` vector. `Parser::translation_unit` passes parsed declarations to `Analyzer::consume`; semantic facts refer to the same NodeIds. Deferred member/template bodies retain parsed nodes, not grammar replay positions. |
| Identity and lookup (§2–3) | `semantic/model.cpp` interns structural types, parameter sequences and qualifiers; `lookup.cpp` indexes `(ScopeId, IdentifierId)` by lookup kind and follows explicit parent/using/base edges. Overload candidate scratch uses compact EntityIds and conversion records. `complete_virtuals` indexes interned names and canonical function shapes; conversion-type identity now distinguishes conversion-function names. Return covariance and exception compatibility are checked before publishing slots. |
| Demand and caches (§4–5) | `demand_member` queues each dormant body once; `finish` advances a demand cursor and a separate monotonic parameter-boundary cursor. `actions_ready`, layout/exception/transfer states and vtable `demanded` prevent repeated completed work. The key-function pass is one entity traversal, not a retry loop. Class completion establishes slots without demanding unrelated ordinary bodies. The new linkage-scope cache is TU-owned, keyed by ScopeId, and populated only after semantic completion. |
| Typed lowering (§6) | `lowering/driver.cpp` directly passes `Ast` and `Analyzer` to `Procedural`, which constructs the typed `Program`. Selected member, slot, base offset, lifetime actions and deallocation are consumed by identity. `virtual_signature` owns its refined parameter slice; it no longer mutates the canonical indirect-function signature. Full-program validation runs only with `--validate-lowir`; text is written once as the requested output. |
| Work and optimization (§7,9) | PA13 adds required O0 operations, not an optimizer. Vtable layout follows actual inherited/introduced slots, D0 expands at most one prepared suffix or calls D1 once, and lifecycle ordering is a linear FunctionId schedule. Existing bounded cleanup and array policies are retained. The array omission proof now rejects required vpointer initialization. The performance review reports all four applicable dimensions without inventing a runtime-profit gate for missing semantics. |
| Allocation and release (§8) | Sources, intern tables, syntax nodes, semantic entities/types, action vectors and flat indexes are TU-owned. Candidate/conversion scratch has query lifetime; lowering builders, cleanup indexes, guards and temporary state reset per function. The requested combined LowIR `Program` and canonical ABI/linkage graph survive to the writer; earlier TUs are released after lowering. There is no second semantic tree or textual IR transport. Retaining typed function IR for this explicit whole-program LowIR output is not claimed as incremental native emission. |
| Self-containment (§10) | The driver calls this compiler's preprocessing, parsing, semantic and lowering implementation. Native driver modes are still explicit unsupported later-stage boundaries. No reference/host compiler invocation, fixture-name dispatch or cached-answer path implements the audited stage. The supplied PA8 native backend is invoked only by independent validation/performance harnesses after student LowIR has been produced. |

All implementation changes remain in `dev/src`. They extend already registered
translation units, so no new source-set entry is needed. Implementation
sources added during stage implementation are registered in
`dev/frontend_source_sets.mk`. The file audit's three existing header-division
advisories are unchanged; none is a failure or a hidden text transport path.

## Representative data flow and findings

### Class declaration, object layout, dispatch and lifetime

For `Derived : Base`, the parser creates one source-faithful declaration/body
region. `class_type` publishes class/member EntityIds; `virtual_declaration`
records cv/ref/parameter shape, pure/final/override facts and conversion targets.
`complete_virtuals` copies the inherited slot sequence/index and replaces only
matched final overriders. Ordinary result types do not identify a slot;
conversion target types do. Destructor matching uses the common destructor
shape and reserves adjacent complete/deleting slots.

`class_layout` reserves or shares a vpointer at offset zero. A non-polymorphic
nonempty base can move to a nonzero recorded offset. Ordinary reference,
pointer, field, constructor, destructor, zero and transfer paths consume that
same layout. Nullable pointer projection branches before adding a nonzero
offset. Existing offset/copy/assignment controls check both data and dynamic
identity. Copy construction initializes the destination's vpointer; assignment
does not overwrite it with the source's identity.

Construction/destruction or a defined key function demands a class's vtable.
That demand visits its actual virtual slots, selects the destructor's
deallocator, and queues required member bodies once. Typed support globals
contain offset-to-top, typeinfo relocation and ordered function relocations.
The vpointer stores select the current construction/destruction class's address
point. Ordinary calls consume their selected declaration, qualification,
object projection, slot and unwind contract. Explicit qualification keeps the
direct call. D1/D0 slot order and D2/D0/D1 definition order are distinct facts;
the final FunctionId schedule preserves both without moving/reparsing IR.

The audit repaired these full ownership paths:

- **Heap arrays:** `finish_allocations` treated an empty user constructor as
  effect-free even for a polymorphic class. `audit-array-vpointer.cpp` compiled
  and validated but crashed on virtual dispatch. The omission proof now also
  requires a non-polymorphic class, preserving each element's required vpointer
  store through the existing counted construction loop.
- **Explicit destructors:** `call_expression` explicitly excluded destructors
  from slot recording. Unqualified `p->~B()` now dispatches D1; `p->B::~B()`
  remains direct. The reducer checks both traces and a further-derived object
  invoking an inherited `destroy` body. Storage is released separately, so no
  object is destroyed twice.
- **Conversion functions:** implicit conversion lowering always used a direct
  symbol. `UserConversion` now records the selected slot; lowering consumes it
  for implicit initialization, casts, result conversions and surrogate calls.
  The class slot index also previously conflated conversions to different
  types. The canonical conversion target is now part of the signature identity.
  A two-slot `int`/`long` reducer checks both dynamic targets and qualification.
- **Operator syntax:** PA12's operator resolver recorded a selected member but
  omitted its virtual slot. It now records the same call fact consumed by
  ordinary call lowering. Assignment and call-operator controls check virtual
  dispatch and the explicitly qualified direct alternative.
- **Global delete:** `::delete` previously entered D0 and therefore called the
  class deallocator despite global selection. The deletion record now selects
  D1 for global deletion and D0 otherwise. Lowering calls the selected global
  deallocator after D1, with one cleanup handler to deallocate on unwind before
  enclosing local cleanup. Null guarding and ordinary class-specific sized
  deletion remain intact. Generalized exception-aware virtual cleanup remains
  outside PA13's handout; the new handler is validated as typed LowIR.
- **Signature provenance:** `virtual_signature` copied a signature header but
  mutated its shared parameter slice. An ordinary `int (*)(B*)` call could
  acquire the virtual receiver's bounded-object promise, even when passed null.
  It now owns a copied compact parameter slice before refinement. The IR check
  verifies the virtual extent and the ordinary pointer's absence of an extent,
  and executes the latter with null.

The C++11 proof for explicit destruction and the one affected course reference
are in [reference-corrections.md](reference-corrections.md). Virtual conversions
follow [12.3.2/1,5](https://timsong-cpp.github.io/cppwp/n3337/class.conv.fct);
global delete selection follows
[5.3.5/9](https://timsong-cpp.github.io/cppwp/n3337/expr.delete#9).
The signature fix preserves LowIR's explicit `object_bytes` promise rather than
inventing a stronger alias/object fact for an ordinary pointer.

### Linkage across translation units

The second TU initially either failed with duplicate object aliases (anonymous
namespace classes) or called the first TU's virtual method (local classes in
same-named static functions). The stage's support cache used a program-wide
ABI key even when the owning C++ entity was internal. Local function metadata
alone did not prevent that identity collision.

`internal_scope` now memoizes the immutable owning scope chain, including
anonymous namespaces and static nonmember functions. Internal support objects
stay in the TU's EntityId maps instead of the external ABI cache; ordinary
methods and deleting entries use the same linkage fact. Combined-program
native labels and lifecycle aliases receive unique stable symbol suffixes.
External header identities still share the canonical ABI graph. The fix covers
vtables, typeinfo/name globals, methods, constructor/base aliases and D0 entries;
no rendered name becomes a semantic key. `check_linkage.py` executes both TUs,
including virtual deletion, and recompiles to check byte-repeatable LowIR.
The existing external-header multi-TU control also passes.

### Supported template demand

For the fixed `template<class T> void consume(T); ... consume(1);` semantic
workload, the parsed template pattern and parameter scope are retained once.
`template_call.cpp` interns the argument pack; `(pattern EntityId, argument-pack
ID)` identifies the supported declaration specialization. The pattern owns its
fixed declaration environment, so this key includes the available semantic
context at this stage. The active/success/failure declaration state is monotonic.
Substitution uses a local binding overlay and dependent-type cache; independent
types are reused. Subsequent calls reuse the selected specialization and typed
conversion facts. Expected deduction/substitution rejection returns a compact
failure instead of rendering diagnostics or throwing per candidate. This trace
ends at the selected semantic declaration, the supported pre-PA14 surface.

## Legality, profitability, invalidation and pipeline bounds

No optional transformation was added by the audit. The repaired decisions are
mandatory virtual-call, initialization and linkage behavior. Comparing those
paths against their incorrect entry outputs would not establish performance
profitability. Common correct programs are compared separately.

| Fact/policy | Legality and whole-pipeline work/growth bound |
| --- | --- |
| Virtual slots and support demand | One declaration scan per completed class; one copy per actually inherited slot/index entry and one visit per demanded slot. Two header words plus one ordinary slot or two destructor slots. No unrelated registry cross-product or global retry. |
| Constructor omission | Only a completed no-argument empty body with no actions/inherited constructor and no polymorphic vpointer work qualifies. Otherwise keep the constructor call. Array IR is independent of runtime extent: 19 and 1,000,000 both produce 106 instructions in the audit control. |
| Dispatch and projection | A selected slot adds a vpointer load, optional constant index and target load; a nonzero nullable base projection adds one bounded branch/join. Conversion/operator facts avoid repeated lookup. No speculative devirtualization or call-site body cloning. |
| Destructor cleanup | D0 expands at most one prepared suffix; larger or bodyful cases call D1 once. Existing destructor suffix expansion is capped at eight actions, then shared blocks bound growth. The 8/64-member control produces 274/1326 instructions and executes both outputs. Global virtual delete adds one local deallocation handler, independent of destructor body size. |
| Arrays/transfers/zero plans | Existing total nested array expansion remains capped at eight elements; larger cases use counted loops or bounded bulk operations. Polymorphic copy/assignment uses prepared subobject actions and preserves destination dynamic identity. |
| Signature refinements | One private parameter slice per demanded virtual member signature, cached by EntityId for the TU. Generic function-pointer parameters remain immutable. The added work/storage is proportional to parameters of emitted distinct signatures. |
| Internal linkage | One cached byte per visited scope, one computation per scope after semantic completion. TU-owned internal support maps and program-owned external ABI keys have distinct lifetimes. Stable labels are a final output decision. |

Completed class, type, layout, exception, selected-call and support facts are
consumed only after their owning completion step. The refined signature and
linkage caches are populated after semantic mutation has ended; no invalidation
by global generation counters is needed. Local lowering state dies with its
function/TU. There is no new fixed-point pass or multiplicative combination of
independent transform budgets. O0 adds no allocator, loop unroller or native
encoding policy. Native loop/call costs are measured through the supplied
backend rather than inferred from fewer LowIR instructions.

## Performance, validation and handoff ledger

[Final performance evidence](final-audit-performance.md) reconciles inherited
measurements, current frozen A/B observations, necessary semantic costs, work
scaling and stage-scoped acceptance. Historical diagnostic thresholds and
misses remain in their original reports; they do not supersede the spec's
current-stage rules. No mandated correctness, comparison or growth requirement
is relaxed.

Final current-state validation:

- `make test-pa13`: **37/37**.
- `make test-report-through-pa13`: **1621/1621**, **13/13** stages, including
  **1584** inherited tests. The read-only supplied Ralph log also contains 1621;
  the prompt's 1645 total does not describe the current fixtures. No fixture
  removal or coverage change was used to reconcile that number.
- `perl scripts/cppgm_file_audit.pl --stage pa13 --paths dev/src`: **pass**, with
  the same three header advisories described above.
- All **eleven** personal executable sources, **15** virtual semantic controls,
  raw lifecycle/demand/cleanup/external-header checks, literal storage controls,
  signature/array-growth checks and internal-linkage/repeatability controls pass.
- The actual driver and all registered sources were rebuilt with ASan/UBSan at
  O1, debug information, frame pointers and non-PIE. After the linkage header
  change, the driver and every lowering translation unit were rebuilt and
  relinked. Final checks set both sanitizers to halt on error. All 37 course
  status/LowIR cases and the personal semantic, native,
  raw IR, signature/array and linkage controls pass without sanitizer findings.
- PA13 requires no separate debug-information or object-inspection target;
  those native surfaces remain outside the current handout.
- `audit_verify.py`: all **33** fixed workloads, **550** final observations,
  frozen binary/source/output identities, observation orders, paired results
  and native outcomes pass. `verify_performance.py` independently verifies all
  **352** historical observations and hashes.
- Every current audit reducer/property fails against frozen entry A and passes
  on final B. This includes the independent signature and internal-linkage
  checks. `git diff --check` passes. Reference review confirms exactly one
  corrected call site, with the source/status/comparison rules preserved.

The first overlapping stage/root test invocation shared `.test_counts`, so its
aggregate counts were not accepted as evidence. Fresh isolated root runs after
both fixes and at exit each report 1621/1621. No harness change was needed.

| Reviewed handoff | Audit disposition |
| --- | --- |
| `9a80791c` semantics | Canonical slot/override facts reviewed; conversion-name identity repaired in `3d39b1cd`. |
| `df2be861` layout/lowering | Storage, calls, constructor/destructor transitions, D0/D1 and ordering traced; missing array/call/delete paths repaired in `3d39b1cd`, internal identity paths in `6c85368f`. |
| `6ca0578c` validator | Contextual integer pointer literals follow PA8's operand contract; floating pointer literals remain rejected. Good/bad personal controls pass. |
| `cf1b9621` evidence | All historical hashes/samples verified; independent current A/B and corrected-path measurements supplement them. Completion claims are superseded by this audit. |
| Final audit implementation | `3d39b1cd`, `6c85368f`: required behavioral and ownership repairs, source-based proofs and executable/property reducers. |
| Consolidation | This audit, compact plan, final performance review and raw/verifying evidence record the completed stage. |

Logs and frozen artifacts reside in `$RALPH_ARTIFACT_DIR/pa13-final-audit/`,
including `final-through.log`, `final-file-audit.log`, `stage-final.log`,
`sanitize-controls.log`, `final-reducers-baseline.json`, the performance/noise/
scaling logs and frozen compiler/LowIR/executable files. Generated objects,
logs and LowIR outputs are not committed. The compact [plan](plan.md) records
final Spec Alignment and acceptance; no PA13 implementation handoff or known
stage defect remains unresolved. PA14 has not been started.
