# PA15 final whole-stage audit — loop 36

Stage base commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`
Last reviewed commit: `dff6a92ba5af15cd21e60c82d98bf62aaf7a3d3e`

Audit entry: `d97445d4`. Result: **PA15 full-stage complete**, with one
cross-owner correctness repair. The fresh course report passes all 15 stages;
no implementation failure, timeout, self-containment defect or unaudited PA15
handoff remains. This review reconstructed the source pipeline and semantic
owners; checkpoint conclusions were evidence to verify, not substitutes for
source review. The prior checkpoint record remains in git at `3aff4801` and in
[checkpoint-evidence.json](../student.tests/pa15/checkpoint-evidence.json).

## Final Spec Alignment and architecture

| Spec surface | Actual ownership and reviewed data flow |
|---|---|
| §§1,8: sources and parser | `preprocess/source.h` retains immutable bytes/file IDs. `PPTokenCursor` borrows spelling; identifiers enter the TU intern table. `PostTokenCursor` holds one PP lookahead and literal scratch. `syntax::Cursor` discards consumed ring entries; deferred class lookahead is delimiter-indexed. `Parser::translation_unit` calls `Analyzer::consume` for each parsed declaration. |
| §§1,2,4: graph and instantiation | `syntax/ast.h` stores source nodes once. `occurrence.cpp` caches source-region IDs, projects occurrence edges and defers bodies/defaults. It does not copy or reparse template grammar. `FactStore` uses 1024-record slabs; `ExpressionStore` shares fixed properties while uses retain bindings, receivers and incoming conversions. |
| §§2–4: identity and lookup | Types, entities, scopes, queries and argument slices have compact IDs. Value arguments encode canonical type/bits; pack arguments preserve nested boundaries. `template_call.cpp` separates explicit prefixes from completed specialization keys. `lookup.cpp` visits lexical and indexed using/base edges; candidates are filtered by arity/shape, then required candidates are checked. Rendered names and ABI text are presentation, not equality keys. |
| §§4,5: facts and invalidation | `template_type_facts.cpp` keys substitution by pattern/head, specialization, parent and lane; overlays contain changed bindings. Specialization declaration/body, layout, defaults, definitions, vtables and emission have distinct states/queues. `finish()` advances queue cursors; it does not restart every pending entity. Definition traversal keys include the member and selected definition head, permitting later definitions without clearing unrelated caches. |
| §§3–5: selection | `template_specialization.cpp` publishes an explicit selection on the existing incomplete canonical entity; a completed primary is not silently reinterpreted. `class_pattern_selection.cpp` searches the primary's candidate list, verifies the exact substituted argument tuple and caches ordering by both immutable pattern IDs. Selection creates only the selected environment; two linear passes identify/verify the winner. |
| §§6,8,10: lowering and ownership | `lowering/driver.cpp` constructs `lowir_model::Program` directly from semantic facts. `Procedural` consumes recorded call/conversion, initializer/lifetime, layout and ABI facts. `symbols.cpp` keys cross-TU linkage by typed ABI entities. TU frontend state releases after lowering; function scratch resets per function; output IR/minimal linkage remain through writing. No production parser/writer roundtrip, host/reference invocation, per-node shared ownership or mutable process-global cache occurs in these owners. |

The source audit includes accumulated literal handling, signature substitution,
pack expansion, index removal, partial matching, initialization, constant calls
and body/storage changes, not only the final patch. New implementation sources
are registered in `dev/frontend_source_sets.mk`; the final repair adds none.
The file audit's three inherited header warnings are advisory ownership findings;
they do not hide per-node allocation or a missing stage implementation.

### Representative declarations and demands

`template<> constexpr int f<2,3>()` in the final controls travels from interned
source tokens to a parsed explicit declaration, canonical integral argument pack,
selected function entity and checked constant body. `C<f<2,3>()>` requests that
selected call, converts its integral result to the parameter type, then uses the
canonical value argument as the class key. The explicit function remains strong;
ordinary primary instantiations use the existing weak emission policy. Selected
body/linkage facts feed typed LowIR and its explicit text output.

For demanded `sum<1,2,3>()`, the source body is retained once. Its complete value
pack identifies the specialization; `template_pack_expansion.cpp` discovers the
source expansion's parameters once and creates lane frames for `N...`. Expanded
initializer edges refer to occurrences in those frames. Initialization establishes
one typed array plan; constant validation visits plan actions. The local array
keeps its own address identity while lowering can copy a legal small readonly
backing into that storage. The loop and call remain executable. The native
control checks the sum and a second specialization, through the supplied backend.

For `C<int,char>` with a virtual `f()`, class completion creates its signature,
layout and vtable demands; the vtable requests the selected body even without a
source direct call. Its unrelated nonvirtual invalid dependent body stays dormant.
For `C<const Tag*>`, candidate matching selects the more specialized const-pointer
pattern; immutable pair ordering is computed twice for 4000 requests, with 7998
hits. Dependent aliases retain their typed template arguments through both lookup
and overload conversion. These are language-required computations, not optimizer
searches. Source and output order are independent of demand identity.

Ordinary and explicit-class member bodies are checked in the enclosing complete-
class queue interval regardless of emission. Implicit class-template bodies remain
demanded separately. Namespace objects request their concrete class's visible
static constant definitions once through indexed definition/storage edges. Late
definitions satisfy those requests; storage does not instantiate unrelated bodies.

### Constant fact, legality and final instructions

`constant_execution.cpp` establishes a checked body independently of emission.
Activations use body identity, canonical scalar argument IDs and typed receiver
identity; specialized body identity includes its environment. Expression values
are keyed by activation/node and never published as parameter-dependent source
constants. Active/success/failure states prevent duplicate recursive work. Missing
definitions and exhausted evaluation budgets remain unavailable rather than
permanent activation failures. Controls cover retry after a definition and a deep
request followed by a shallow request. Body checking runs with activation values
suspended, preserving definition-time semantics.

The final volatile repair traces a negative fact end to end. A volatile parameter
or referent retains its cv-qualified expression type even after scalar argument
canonicalization. AST evaluation refuses its lvalue-to-rvalue conversion; typed
name/value queries do likewise. Reference declarations do not publish their
initializer as a constant readable through a volatile glvalue. Static initializer
classification distinguishes forming an address from reading through that address.
The latter remains a dynamic initializer. Lowering consumes the existing type and
emits `load volatile i32`, including in namespace and local-static initializers.
The supplied backend executes these programs. Unselected conditional branches,
`sizeof`, address formation and volatile-array decay do not falsely acquire reads.

This is a legality repair, not an optional optimization. It introduces no analysis
cache or invalidation, no semantic lookup in lowering, and constant work per
existing evaluation/initializer visit. The source-fact publication fix prevents
stale reusable values rather than clearing all constants after a declaration.

## Findings and changes

1. **Volatile constant reads and lost initialization effects — fixed in
   `dff6a92b`.** Frozen entry accepted eight invalid constant-expression reducers
   and omitted volatile loads in two dynamic-initializer controls. The repair
   spans `constant.cpp`, `template_value_facts.cpp`, `type_builder.cpp` and
   `static_initializer.cpp`. The governing rule is N3485 5.19 [expr.const]/2,
   [non-volatile glvalue requirements](../doc/n3485.txt:7222); this is semantic
   proof, not compiler agreement. Seventeen native and eleven rejection controls
   now pass. Required fixtures were not changed.
2. **Stale audit records and verifier scope — consolidated.** The old audit
   ended at 166/177 and the plan at an unaudited 177/177 handoff. The checkpoint
   measurement-only verifier also compared historical harness hashes to the
   later edited working file. `verify_final.py` resolves exact retained git/file
   contents by hash and verifies historical campaigns without imposing their
   old live-binary/marker assertions on a corrected final compiler. Original
   verifiers, handoffs, preliminary observations and measurements are preserved.
3. **Inherited reference correction — independently confirmed.** The only
   changed fixture is `100-aggregate-functional-braced-cast.ref`. Its
   [reducer and C++11 proof](reference-corrections.md) demonstrate that a raw
   relocation of nontrivial array elements bypasses required construction and
   breaks their self-pointers. The fresh reducer returns 0 for student output
   and 1 for pinned reference output. Source/status, full comparison rules,
   coverage and bundle revision `c2f713cd70d06170632bfde3e75dd6fe1aa44d98` are
   unchanged. There is no new reference correction in loop 36.

## Performance, work and growth acceptance

[Final performance](final-performance.md) verifies all four dimensions against
frozen binaries, flags and sources: compiler wall time/peak RSS, checked native
runtime and executable text. It preserves **4774 invocations in 24 campaigns**,
including 644 new final observations/warmups. A/A calibration precedes two ABBA
blocks. Repeats retain the original observations; small startup-dominated compiler
samples support no latency claim. Identical native bytes support parity, not a
runtime speedup inferred from timing noise or fewer IR nodes.

Required constant execution has 512 active calls and 1,000,000 expression visits
per root. Caches retain only demanded TU facts. Pack work follows actual lanes,
dependent nodes and produced IR, without a fixed partition cap. Matching/order
work follows candidates and compared pairs. Ordinary lowering remains linear or
near-linear in consumed/produced facts; no global fixed-point scan, inlining,
unrolling or higher-level growth policy is added. Telemetry observes existing
work in separate, untimed `--stats --validate-lowir` runs.

Small-array backing is a bounded lowering policy: static storage addresses only,
trivial destruction, no volatile subobject copies, at most 32 bytes per backing
object. Each entity emits once, so the whole pipeline adds at most 32 bytes times
the number of eligible demanded objects. Larger or noncopyable plans use ordinary
stores/zero loops. Earlier unbounded backing regressed large-array runtime despite
smaller text; its measurements are preserved and that policy was removed. The
retained eight-element case shows measured benefit, while the four-element
contract form's small runtime cost is explicitly disclosed. No mandated comparison
or language behavior was weakened for performance.

Spec §9's stage-scoped acceptance applies to inherited plans too. PA15/O0 has no
numerical latency, RSS or code-size exit ceiling. PA14's local 64-KiB diagnostic
compiler-text budget is not an accumulated-stage gate; its operation-specific
allocation/target-growth constraints and historical measurements remain intact.
Required semantic costs are reported, and no unprofitable optional transform is
being excused as correctness work. Native MIR allocation/spills/direct ELF,
O1–O3 budgets and self-hosting have no PA15 production surface; their later owners
remain explicit rather than fabricated measurements or extra exit gates.

## Validation and audit ledger

Fresh checks at the reviewed code: `make test-pa15` **177/177**, root
`make test-report-through-pa15` **2112/2112**, and file audit **pass** with three
inherited header warnings, all exit zero. The root command covers all 15 stages;
its actual count supersedes the incoming 2136 status claim. No earlier fixture,
handout, test/comparison script or coverage rule was changed. LowIR validation is
active; PA15 requires no later-stage debug/inspection/native compiler gate.

All nine explicit personal suites pass: constants (10 groups), values (26),
specializations (24 native/13 rejection), packs (27/8), matching (15/7),
initialization (23/13 plus three LowIR controls), execution (24/18), checkpoint
(11/7), and final audit (17/11). The fresh reference reducer and source/fixture
hash checks supplement those tests. [Final evidence](../student.tests/pa15/final-evidence.json)
records reviewed commits/files, code tree/binary, campaign hashes and current logs.

| Review boundary | Disposition in this final audit |
|---|---|
| Checkpoint 32, `8000f3c8..538cfcb0` | Independently retraced accumulated canonical value/pack/literal/ABI and lowering paths; original failures closed by later owners. |
| Loop 33, through `b8379f52` | Matching/alias parser facts, exact class selection, ordering reuse and selected environments reviewed. |
| Loop 34, through `d705aafc` | Initialization/layout/zero plans, retained update queries, array addresses, volatile stores, reference proof and backing budgets reviewed. |
| Loop 35, through `d97445d4` | Constant execution and its caches, ordinary validation versus emission, static definition/storage edges reviewed. |
| Loop 36, through `dff6a92b` | Volatile owner repair, representative cross-feature controls, whole-stage benchmarks, preserved historical evidence and fresh required exits complete. |

No PA15 handoff or required implementation group remains unaudited. The next
assignment may extend these owners; this audit makes no claim to implement its
additional language or backend requirements.
