# PA23 final audit 126

Target: **PA23 full-stage**. Disposition: **accepted for advancement**.
Reviewed range: `f33dd0775073bf5db6665fb2f4f504783159df76..83ba7c58`.
Production implementation: `d9179e848e8b6a5b9ca378a0ee20152404ce166a`;
compiler SHA-256 `979a332fdeb3133af19b09d93bfbf4cb061236b59573e682b2c4a747fda5acaa`.

The previous goal turn made progress: it completed and committed the stage's
remaining lifecycle/ABI/member-pointer owners and validation. Entry was clean;
no prior build or test process was live. This audit read the spec, handout,
testing rules, plans, all stage commits and the final combined changes in all
47 implementation/build-registration files. It reconstructed the underlying
frontend and lowering paths; the checkpoint's conclusions were not substituted
for source review. [Checkpoint124](audit124.md), all implementation handoffs and
historical measurements remain preserved.

## Findings and changes

No additional in-scope production defect was found. Handoff125's claims were
tested across their ownership boundaries, including combinations absent from
its controls. [audit126.py](../student.tests/pa23/audit126.py) adds twelve
independent cases: two virtual bases passed by value together with indirect
results, secondary virtual dispatch and member pointers; indirect calls;
derived value projections; covariant/null member returns; shared construction
order; demanded templates with dormant invalid members; null projections and
crosscasts; externally owned construction RTTI and value-result adjustors in
separate/merged TUs and both orders. **12/12** pass validation and execution.

The compiler, fixture sources, oracles, statuses and comparators are unchanged
by audit126. Changes are the explicit personal controls, fresh evidence and
consolidated final plan/audit/performance records. Two reporting details are
resolved: the root report verifies **3856/3856** fixtures and **22/22** separately
reported focused controls; the supplied **3880** summary is not substantiated
by either the primary log or the fresh repository report. Current Itanium ABI
RTTI Layout is §2.9.4, although
the older correction records label it §2.9.5. Their `#rtti-layout` anchor and
substantive proof are correct. Historical records are not silently rewritten.

## Final Spec Alignment and architecture

| Spec surface | Actual owner, data flow and audited invariant |
|---|---|
| §1 source/parse | `Preprocessor` owns immutable sources and interned identifiers. PP/post/syntax cursors stream into `Parser::translation_unit`, which sends each parsed region to `Analyzer::consume`. The ring retains unresolved lookahead; there are no successive owning token streams or second semantic syntax tree. PA5's specified lexical fallback is subordinate to known name categories. |
| §2–3 identity/lookup | Node/Entity/Type/Scope IDs, canonical argument tuples and `IdIndex` flat tables own equality. Scope-kind/name indexes, base/import edges and candidate filters bound lookup to relevant declarations. Selected member, receiver/conversion and slot facts are recorded. Manglings and LowIR names are final spellings, not semantic keys. |
| §4 templates | `syntax/occurrence.cpp` retains parsed source regions and creates compact `(source,context)` occurrences. Nested classes, bodies and defaults are demand-separated. `substitute_type` immediately reuses nondependent types; `template_expression.cpp` inherits checked fixed facts. Parent-linked substitution frames include specialization, parameter range, parent and argument tuple. No grammar replay, body cloning or enclosing-environment copy. |
| §4–5 demand | Member/body/layout/RTTI/ordinary-table/construction-table states distinguish pending, active, success and failure. `Analyzer::finish` advances queue cursors; definition notification wakes the owning table/member. `demand_vtable` publishes each construction dependency set once. Completing a class does not instantiate its dormant template members. |
| §2,5 shared layout | `virtual_subobjects.cpp` interns virtual anchors and ordinary occurrence paths. All required final-overrider candidates participate before physical view coalescing. `layout.cpp`, `layout_virtual_views` and `layout_lifecycle` publish complete/nonvirtual extents, base offsets, segment-owned negative rows, address points, physical store order and VTT slices. |
| §5 cache validity | Base-path/miss keys use source/target IDs only after class completion; layout of each path is memoized. Containment cache keys both receiver occurrences and lives for one class completion. RTTI finalization follows semantic discovery. Program view keys include table/view, construction keys table/VTT index, and adjustor keys target plus every this/result/virtual-result adjustment. Local completion never clears unrelated caches. |
| §6 lifecycle/ABI | Semantic actions distinguish shared virtual bases and direct nonvirtual bases. Complete entries initialize shared bases once; base entries skip them. Copy sources use their own table, construction destinations use entry context; destruction reverses actions and assignment retains direct-base order. Program signature plans place by-value hidden base addresses after visible parameters, before lifecycle tails; references/pointers have none. |
| §6 direct lowering | `Procedural`, `Program` and `FunctionBuilder` consume typed facts and append typed instructions/data. Invariant failures remain errors. Each base/complete/deleting entry, view, construction segment, adjustor and member callable has an emission identity. `order_lifecycle_entries` scans reserved symbols, not later semantic declarations. LowIR writing is the requested output boundary; no serialize/reparse transport. |
| §8 ownership | TU vectors own source facts, class/slot/view arenas and flat indexes. Class-completion scratch dies after publication. Program `Linkage`/ABI graph and signature plans survive all merged TUs. Hidden argument maps and builder scratch reset per function; temporary table buffers die after publication. The typed LowIR program survives for the explicit writer, then releases. No per-slot owning pointer graph or process-global mutable cache was introduced. |
| §9–10 evidence/self-containment | Opt-in telemetry observes existing work; stats+validation preserve output hashes. Source-path inspection and file audit find no reference/host compilation delegation. Reference executables appear only in observation/validation harnesses. O0 executable work, latency, RSS and text are measured together; later native/self-host requirements are separately owned. |

Production source review includes `semantic/{member,virtuals,virtual_subobjects,
layout,lifecycle_layout,construction,construction_vtables,destruction,
transfer_actions,rtti,rtti_facts}.cpp` and every dependent consumer in the
47-file delta. New `.cpp` files are registered in `dev/frontend_source_sets.mk`.
The three file-audit warnings concern substantial inherited headers, not failed
rules or newly introduced implementation ownership.

## Representative end-to-end traces

**Ordinary declaration.** `value-result-member-call` defines `A : virtual V,
virtual W`, and `D : P, I` overriding `I::read(A)` to return a three-word `R`.
The semantic call selects `I::read`, records D→I displacement and by-value A
materialization. A's layout owns V/W offsets. Signature construction produces
the result address, receiver, A address and two hidden base addresses; the
single call emitter expands that plan. Converting `&I::read` to a D member
pointer changes the receiver displacement and retains the virtual callable.
The callable loads the logical slot and forwards the already prepared arguments
using `emit_raw`, avoiding a second hidden-argument expansion. The table target
adjusts I's receiver back to D and invokes `D::read`.

[ELF trace](../student.tests/pa23/trace126.json) confirms the callable's two loads
and indirect call, plus `_ZThn8_N1D4readE1A` applying `lea -8(%rsi)` before the
call. The indirect result remains in `%rdi`; the receiver is correctly `%rsi`.
No source body is copied into either helper. The test checks both `R::x` and
`R::z`; separate/merged and reversed-TU variants check the same ownership.

**Demanded template.** `template-shared-construction` instantiates `D<6>` from
`A<6>, B<6>`, both virtually deriving from `V<6>`. Parsed patterns and literal
facts remain shared; concrete field/virtual-call facts use the specialization
frame. The invalid dependent `A::dormant` body is neither checked nor emitted.
Shared V identity participates once in layout; A's override wins for that V.
Complete D construction initializes V once, then calls A/B base entries with
their VTT slices and the same physical V address. `read(D<6>)` copies into its
parameter object and receives the hidden V address. Conversion to V and virtual
dispatch consume the published projection/slot facts and return seven.

Its ELF has one weak `_ZTT1DILi6EE`, the expected A/D table entries, and distinct
28-byte adjustors for -16 and -8 receiver displacements required by complete
and construction views. No dormant symbol exists. The source compiles to
validated deterministic LowIR and both supplied native routes execute it.

**Stage boundary.** These traces reach ELF through the PA8/PA23 supplied backend
and authorized host object linking. PA23's production endpoint is typed LowIR
plus its explicit text view. This audit does not claim that this compiler's own
MIR selection, allocator, ELF writer, host member-pointer ABI or debug emission
already exists; those surfaces belong to PA24 and later assignments.

## Optimization, invalidation and pipeline bounds

The useful fact traced here is non-null `this`: C++11 [expr.prim.this] §5.1.1/2
identifies the object whose nonstatic member is executing. `expression.cpp`
records it and `base_projection` preserves it. `pointer_projection` removes a
null branch only for this proof or zero fixed displacement. Arbitrary pointers
retain checks; `null-shared-projection`'s disassembly shows a null branch before
loading the -24 virtual-base row. Covariant pointer results also retain their
null test. No fact crosses a mutable storage write or invents alias/unwind facts.
This is O(1), has zero code-growth budget and adds no analysis. The original
paired runtime benefit and 428→396 text bytes are preserved in
[performance121](performance121.md); the final executable remains 396 bytes.
The fresh ELF trace shows `Base<D>::self()` as an 18-byte function containing
`lea -4(%rdi),%rax` and no conditional branch, confirming the fact reaches
encoding. Its remaining O0 frame setup is visible in the supplied backend's
disassembly; it is not evidence of a student frame-allocation optimization.

| Policy | Legality and fallback | Work/growth bound and invalidation |
|---|---|---|
| Shared-view coalescing | Resolve all final-overrider candidates first; merge only identical physical subobject/type views | Linear imported facts plus physical store sorting O(views log views); class-local scratch, immutable publication; no fixed-point rescan |
| Path/layout reuse | Key completed source/target path; retain unknown/incomplete queries conservatively | Each demanded path layout once, shared tail summary; no global invalidation |
| Member-pointer proof | Every write must preserve displacement; exposure/unknown value disables the proof | 64-step shared budget per proof, memoized owner states; fall back to full callable/displacement extraction; zero growth |
| Member callable / ABI expansion | Needed to implement addressed virtual dispatch and required value-parameter ABI | One callable per program target, at most five logical instructions; one pointer/index per hidden fact, optional address instruction; linear arguments, no source-body copy |
| Lifecycle/VTT | Preserve physical layouts, action order and construction RTTI | Each required segment/row once; nested sub-VTTs omit virtual tails; complete VTT appends each shared sub-VTT once. Construction dependencies publish once; emission scans reserved IDs once |
| O0 cleanup/arrays | Preserve effects, exception paths and destruction order | At most two D0 actions before falling back to D1; destructor suffix expansion capped at eight, larger suffixes shared; array unrolling capped at eight, otherwise loops. No multiplicative unbounded body expansion |

These bounds compose over demanded class facts, emitted table rows, call
arguments and function actions. Distinct class closures and ABI entries can
require more than source-linear output; repeated inheritance paths must not
multiply identical physical facts. There is no optional inliner, loop transform
or O1–O3 fixed-point optimizer in this stage. Actual backend spills/frames in
the ELF trace are recorded, not mistaken for student allocator quality.
[Performance126](performance126.md) assesses all four performance dimensions
and the measured semantic costs, retaining the original evidence and spread.

## Reference and coverage audit

[Correction122](reference-correction122.md) and
[correction125](reference-correction125.md) cover exactly **21** changed `.ref`
files since stage base, **20** since checkpoint124. All 45 fixture sources and
expected statuses, harnesses and comparison rules are unchanged. The pinned
bundle is `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, archive SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.

Independent review read the supplied N3485 rules for shared/ordinary base
subobjects (§10.1/4,6,7), initialization (§12.6.2/7,8,10), construction RTTI
(§12.7/4–6) and null allocation (§5.3.4/13, §18.6.1.3/2). The eighteen
nonpolymorphic virtual-base corrections are one consistent table-based layout
and ABI repair required by the PA23 parameter contract; the two other outputs
change construction RTTI/top and shared-diamond metadata. The prior ordinary
diamond's repeated-base flag remains a separate one-field correction.
[Itanium construction entries](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#vtable-ctor)
and [RTTI Layout](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#rtti-layout)
support the metadata rules. Compiler agreement is not the proof.

Fresh [reference observations](../student.tests/pa23/reference126.json) reproduce
reference exits 1 (nonpolymorphic reference), 2 (construction RTTI), rejection
(indirect virtual-base mem-initializer), 0 (class placement control) and 1
(standard null placement). Student exits are all zero. Standard placement is
linked from an address-taken library function, not replaced by user code.
The inherited repeated-base reducer also passes again. The fixture with
indeterminate scalar virtual members remains a LowIR comparison, not a claimed
defined runtime benchmark. No oracle was changed during this final audit.

## Validation and final ledger

[Validation126](../student.tests/pa23/validation126.json) records exact commands,
observed terminal exits, log/evidence hashes, source digests and individual
personal verdicts. Required results are **45/45 PA23**, **3856/3856** through
fixtures and **22/22** separately reported focused controls, **23/23** stages, and
passing `perl scripts/cppgm_file_audit.pl --stage pa23 --paths dev/src` with
three inherited warnings. All **168** inherited personal cases, **12** new cases
and **44/44** accepted-output roundtrips pass. Telemetry does not change outputs.
PA23 adds no debug/inspection exit target; representative ELF inspection was
performed explicitly for this architecture audit.

The [standalone backend limit](../student.tests/pa23/backend-limit126.json) is
reproduced with reference and student shared-RTTI LowIR: the supplied standalone
runtime returns 1, both hosted objects return 0. Reference private-label repair
is confined to observation scratch. Source behavior and required comparison
remain enforced; retain this backend reproducer for PA24.

| Handoff / commits | Reviewed contribution | Final disposition |
|---|---|---|
| 121–123, `b28e12ce..2bba7273` | Views/covariance, RTTI, lifecycle identities, shared subobjects and segment rows | Final ownership paths independently reconstructed; old measurements preserved |
| Audit124, `36e612a0..7a644d69` | Program-owned thunk/view identities, shared-view growth, completed path reuse; incomplete groups explicitly retained | Fixes remain; all prior failing personal runtime groups now pass |
| `a6036731`, `72477046` | Lifecycle layout/actions, VTT/base context and centralized by-value ABI | Reviewed through every constructor/transfer/cleanup/call consumer |
| `ba150d2c`, `587a6835` | Construction demand/RTTI and member dispatch callables; justified oracle correction | Independent controls, reducer proofs, native traces and unchanged coverage verified |
| `e9ea810c`, `d9179e84` | Bound schedule by reserved emission IDs; publish construction dependencies once | Full earlier-stage report and deep/forest counters verify the completed path |
| `83ba7c58`, audit126 | Handoff evidence, final architecture/performance/validation consolidation | No unaudited handoff remains; PA23/O0 accepted |

No known correctness, self-containment, timeout, file-audit or architecture
defect remains within the PA23 contract. Later native/host/debug/self-host work
and the supplied backend observation are explicit handoffs, not additional
PA23 exit gates. This audit completes PA23; it does not implement PA24.
