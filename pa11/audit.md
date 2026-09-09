# PA11 independent final audit

The audit starts at `0773e4d8`, reconstructs the implementation since stage base
`a97e14d4`, and reviews the actual source, handout, LowIR contract, spec and all
21 stage commits. Checkpoint conclusions were leads, not completion evidence.
Final implementation: `9c0a9c4e`, including the ownership fixes in `b262971d`
and temporary-zero correction in `cd09b606`. The final action-identity naming
cleanup produces a byte-identical compiler to the frozen `cd09b606` candidate.
No later assignment is advanced.

## Spec Alignment

PA11 owns the nonvirtual object model and O0 source-to-LowIR generation. Native
execution uses the supplied PA8 backend only in personal test/measurement
harnesses. Native MIR, allocation, ELF writing, optimized levels, template body
instantiation and self-hosting remain at their handout stages. There is no PA11
requirement to build a native backend or pass later debug/profiler checks.

| Spec | Inspected ownership and actual flow |
| --- | --- |
| §§1–2 source and graph | `SourceBuffer::bytes` is immutable. The TU preprocessor owns buffers, interned identifiers and macro expansion slabs. `PostTokenCursor` and the syntax ring cursor stream tokens; complete-class category lookahead retains only its deferred region and skips nested delimiters. `translation_unit` calls `Analyzer::consume` before the next region. The sole NodeId graph retains source locations; type/entity/expression facts extend it without a copied semantic tree. PA5's required lexical hints remain subordinate to known declaration categories. LowIR production now omits joined string display spellings; linkage consumes decoded literal bytes. |
| §§2–3 identity and lookup | `Types` interns structural kind/cv/child/entity/parameter keys. Flat `IdIndex` owners index scope/name/kind, signatures, fields, using edges and friendships. Ordinary lookup follows lexical/import/base edges; ADL follows argument-associated classes, bases, enclosing and inline namespaces. Candidate arrays deduplicate identities, filter shape/arity, then compare all required conversions. Expected candidate failure is a compact invalid conversion; selected access failure diagnoses at the semantic boundary. |
| §§2,4–5 class facts and demand | Class layout and constructor/destructor actions are separate facts. Member demand uses Dormant/Queued/Active/Complete and one cursor over deduplicated entities. Completing a class preserves parsed method bodies without resolving unrelated bodies. Layout observes in-progress state; completed size/alignment/offset/bit-storage facts are reused. Inherited forwarding preserves local signatures and base/complete entry demands. No global retry or invalidation sweep occurs. |
| §§4–5 inherited template surface | `template_call.cpp` keys declarations by pattern EntityId and interned canonical argument pack; the pattern owns its fixed environment. Short-lived substitution bindings are overlays. Dependence memoization reuses nondependent TypeIds; declaration Active/Success/Failure and emission demand are distinct. The supported declaration trace does not replay template grammar or claim later template-body generation. |
| §§5–6 initialization facts | Semantic source/type plans own explicit actions and omitted ranges. Shared value-initialization shapes use `(0, TypeId)`; contextual validation uses `(TypeId, effective access ScopeId)`, with base/complete emission demand retained separately. Qualifiers are propagated before publication. Default constructor conversions belong to the selected member. Static constructor summaries and object results have separate ctor and `(NodeId, TypeId)` owners after semantic completion. Lowering's removed field/syntax fallbacks now report missing required aggregate plans as invariants. |
| §§6–7 typed lowering and ABI | `Procedural` constructs typed PA8 records through `FunctionBuilder`; its local append checks instruction shape, owner and terminator invariants. PA9's typed ABI graph supplies entity keys and mangled export views. Per-TU EntityId maps and program-owned linkage IDs preserve one emission per distinct base/complete entry. Strings, startup/finalization and TLS helpers have separately reserved collision-free symbols. The production path calls no LowIR reader, ABI fact reader, host compiler or reference binary. |
| §§6–8 lifetimes and storage | `check_jumps` records immutable lifetime prefixes and checks jump ancestry using DFS intervals. Cleanup suffix interning keys state plus terminal; return terminals identify control contexts and save return values before convergence. Constructor/destructor actions preserve subobject order and exception cleanup. Function builders and cleanup/overload/substitution scratch have bounded owners; geometric graph/fact/IR pools use IDs, without owning per-node children or recursive destruction. TU frontend state dies after lowering; program LowIR and typed linkage live until the explicitly requested text output is written. |
| §§9–10 observation and self-containment | Separate `--stats` runs observe source/cursor, candidate/conversion, specialization, demand, action/lifetime, static-cache, linkage and IR-pool work. Timing omits telemetry and full validation. `--validate-lowir` audits the completed typed program once. The only process launcher in the linked support surface is the course test runner dispatching requested student-tool invocations; it does not implement compiler results. Legacy MIR placeholders are not an active PA11 backend. |

## Representative data flow

`audit-default-conversion.cpp` has namespace arrays and local arrays whose
constructor takes `const Box& = 5`. Parsing creates the source default expression
once. Semantic selection identifies `A::A`, records the conversion through
`Box::Box(int)`, a materialized object identity and its destructor, and owns the
conversion range on the default call. Object actions retain the chosen
constructor. Lowering consumes that range, creates the temporary's slot, calls
the selected Box constructor and passes its address to A. A's member action
loads the recorded reference field. Each array element destroys its default
argument temporary before constructing the next element. Complete/base ABI
entries, function signatures and symbols are typed records. The supplied
backend validates/executes that source-generated LowIR; checked counters prove
one live temporary at each constructor and none left afterward.

`audit-global-bitfields.cpp` declares a global before an earlier dynamic reader
and defines its constant aggregate initializer later. Semantic layout records
storage width, shift, signedness and offset for each field. The initializer plan
retains field identities and converted constants. Static lowering patches only
the represented bits, preserving overlapping ordinary scalar bytes and padding;
its scan/split window is at most eight bytes and never allocates a whole class
byte image. Structured global data is available before startup. The reader
checks signed, unsigned, mixed ordinary/bit-field and 64-bit storage values.

`audit-nested-range.cpp` follows an omitted element to the semantic shared value
shape, its nested array range and the existing zero-value proof. Lowering can
emit one exact bulk zero for each eligible span. Volatile subobjects instead
retain typed scalar stores in a counted loop; unions retain their first active
member. Nested repetitions share an eight-element expansion budget. The
32/million-element checks have equal instruction counts; 2/4/6 array dimensions
produce 102/266/430 instructions, rather than exponential duplication.

Repeated `consume(1)` in the fixed template semantic benchmark reuses the
same `template<class T> void consume(T)` pattern, `int` argument pack, substituted
function declaration and demand identity; nondependent `void` is shared. Its
trace ends at the inherited declaration/demand surface. There is no supported
PA11 template-body-to-ELF path to audit. Non-template executable traces above
continue through the external native test boundary, whose hash, ELF payload and
checked execution results are recorded with performance evidence.

## Findings and changes

- Omitted aggregate fields were reconstructed in three lowering adapters. A
  two-element array of classes containing 10000 integers emitted **60009**
  instructions. Semantic value shapes now cover nested arrays and classes;
  lowering consumes them directly. The reducer emits **13** instructions.
  The expansion bound applies across dimensions, not independently at each
  dimension. Static omitted data remains a bounded zero span.
- The omitted union fallback visited every variant, and `zero_value` admitted
  unions. Plans now identify the first active member and bulk runtime zero
  requires absence of union/volatile boundaries.
- Aggregate initialization lost enclosing volatile qualification; helper keys
  also erased cv. Semantic action types propagate qualifiers and helpers retain
  the complete type and recorded field action. Ordinary and volatile helpers
  coexist correctly in one TU.
- A class-only value-initialization cache skipped access checks in later scopes.
  The structural shape remains shared, while contextual success has a complete
  typed key. A friend initialization followed by an unrelated caller now rejects.
- Namespace and TLS arrays without explicit initializers missed dynamic
  constructor scheduling. They now use the same recorded object constructor and
  bounded array lowering as locals; first-use TLS guards remain intact.
- Static bit-fields were emitted as independent scalar fields, corrupting
  packed values. Bounded bit patching fixes the global data owner, including
  overlaps with ordinary fields and initialization before dynamic readers.
- Default constructor arguments bypassed selected user-defined conversions.
  The member now retains the conversion range, and scalar, local-array,
  aggregate-array, namespace-array and TLS construction consume it. Array
  default-argument temporaries are destroyed between elements.
- Early static constructor summaries ignored unused argument effects. Every
  selected argument, including defaults, must now have a static value after its
  conversion. Otherwise the ordinary dynamic path evaluates all arguments.
- Constructor initializers naming unrelated classes or inherited fields were
  silently dropped. Semantic ownership validation now requires a direct member
  or the supported direct base.
- LowIR production built unused joined literal display strings, and temporary
  value construction emitted preliminary zero twice. The display adapter is
  disabled for this mode, linkage uses literal facts, and `construct` alone owns
  the preliminary zero. The temporary reducer checks one `zeroinit` and its result.

The initialization rules are checked against [N3485](../doc/n3485.txt):
§8.5.1/7,9–10 (omitted members, references and arrays), §8.5.1/15–16 (unions),
§3.6.2/1–3 (static/thread initialization, constant-before-dynamic ordering and
permitted early initialization), §12.2/3–4 (temporary destruction, including
array default arguments), and §12.6.2/2 (initializer ownership).
[PA8 memory rules](../pa8/lowir.md#memory-and-addressing) require volatile scalar
initialization markers and distinguish preliminary class storage zero.
No new reference corrections were needed. The seven earlier narrowly proved
corrections, reducers, cited rules and bundle revision were reviewed in
[reference-corrections.md](reference-corrections.md); their source inputs,
sidecars and comparison rules remain unchanged.

## Legality, profitability and budgets

PA11 has one O0 lowering policy, without an optional optimization pass pipeline.
Bulk zero consumes an exact omitted range plus completed type/layout facts;
it cannot cross explicit actions, volatile or union boundaries, or constructor
work. The alternative is bounded loops/scalar operations, not speculative
strengthening of unknown facts. Static byte zero spans have no runtime volatile
access semantics. Early constructor constants require an effect-free body,
allowed scalar field actions, preserved conversion types, and static values
for *all* arguments. Failure retains dynamic calls. Cleanup sharing requires
identical lexical suffix and control/terminal identity.

These decisions happen while constructing IR; no existing instruction analyses
need invalidation. Type/initializer caches are TU-owned and used after relevant
facts are established; no global generation counter clears unrelated facts.
Runtime repetition may grow with object size, while compiler work and generated
control flow track action graph and bounded expansion. Explicit initializer data
necessarily scales with produced data. There is no optional unrolling, inlining,
allocation search or hidden higher optimization level. Later native spill,
selection and encoding quality is outside PA11 ownership; measured executable
results still expose consequences of the emitted LowIR.

The [final performance record](final-audit-performance.md) reports compiler
wall time/RSS/text and executable runtime/text together, including A/A noise,
ABBA pairs, all raw observations and the retained initial candidate. The spec's
stage-scoped rule governs acceptance. Historical 1.10x latency, 1.20x RSS plus
16 MiB, 128 KiB text and scaling thresholds remain diagnostics; they are not
additional course exit gates. The eight-element implementation expansion bound,
correct semantics, coverage and required comparisons are preserved.

## Validation and handoff ledger

Fresh `make test-report-through-pa11`: **1327/1327, 11/11 stages**;
PA11 **302/302**, including all four focused controls. The supplied state said
1351 tests; both the retained primary log and fresh checkout execution report
1327. No test or comparison was removed to obtain that count.
`perl scripts/cppgm_file_audit.pl --stage pa11 --paths dev/src` passes with the
existing Analyzer-header advisory. `student.tests/pa11/check.py` validates and
executes 17 programs and rejects ten invalid programs. `audit_check.py` adds
eight executing reducers, two rejections, four range checks and three nested
expansion checks. All run explicitly; none changes course discovery.
The actual registered driver/phases were also built with ASan/UBSan using
`student.tests/pa10/build_sanitizer.py`; the complete audit reducer/range suite
passes with `CPPGM_AUDIT_COMPILER` selecting that build. No sanitizer report is
accepted as an ordinary rejection. Frozen artifact verification checks four
campaigns, 336 compiler observations, 96 runtime observations and 24 executable
outcomes, and confirms the checkout compiler matches the final frozen binary.
The final exit-check logs are `final-file-audit.log` and `final-through.log`
under `$RALPH_ARTIFACT_DIR/pa11-final-audit`; explicit reducer and artifact
verification logs are `final-audit-check.log` and `final-audit-verify.log`.

The earlier plan's review pointer still named the stage base. This audit closes
all handoffs: member ABI (4113c34d), construction/defaults (17897014–cd054d80),
lifetime/noexcept (2f886c18–55a21dac), selection/access/ADL/operators
(165af40e–88e4ebe1), layout/initializers (09480f23–786ed29e), and inherited/value
construction, ABI/TLS/literals and narrowing (f5918ce9–0773e4d8). New audit fixes
are `b262971d` and `cd09b606`, followed by action-identity naming cleanup in
`9c0a9c4e`; the initial frozen campaign is retained under its own binary hash.
Final measurements and their verification complete the audit ledger. No PA11
implementation handoff is deferred to PA12.
