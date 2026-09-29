# PA18 checkpoint audit 86

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Previous review baseline: `ecc308bc5ee33ed40fa773f7981961f3018a5867`.
Entry: `890f810bf203639318acc91597273ecdc592c911`.
Last reviewed commit: `f6eaf8ab213c90eefffd48f5c47b1ac9a75bce8d`.

**Checkpoint audit passes; PA18 full-stage remains incomplete.** The review
covers every one of the 17 accumulated commits across handoffs 83–85 and the
validated audit repair, including the combined changes to **39 implementation
and registration files**. [Audit 82](audit82.md) is preserved verbatim. The
[evidence manifest](../student.tests/pa18/loop86-evidence.json) binds full commit,
patch, source, binary, coverage, check and performance identities.

## Complete accumulated range

| Commits, chronological | Reviewed scope |
|---|---|
| `48c864ab` | Audit 82 records, frozen evidence and retained stage obligations. |
| `e9108b5a`, `5bd58396`, `7b2e89bd`, `89e98f11`, `ac354fad`, `09a77fca` | Array bound completion and query substitution, constant images, static member declaration matching, bounded expansion/progress checks, complete fixture preservation, initializer-form enforcement, parenthesized clause correction, proofs and handoff. Reviewed the intermediate clause-boundary defect and its correction. |
| `e4a4f2c3`, `cc4cd563`, `b4d66361`, `7a7ce959`, `f953e42a` | Ownership plan, empty helpers, local-specialization roots, delegation entry propagation, empty transfer legality/lifetimes, scalar widening, required zeroing proof, measurements and handoff. |
| `53252879`, `37c7c832`, `f1fae6ad`, `076eccdd`, `890f810b` | Discarded source forms, volatile class copies/lifetimes, storage/discard proof, functional void queries, fixed-recipe publication/reuse, complete initial/final performance observations and handoff. |
| `f6eaf8ab` | Audit repairs, 74 focused controls, combined source-to-native trace and reproducible cumulative validation/performance/evidence harnesses. This is the reviewed code tip; subsequent changes are records only. |

The preceding goal turn supplied committed implementation and evidence, so it
was progress. Entry process inspection found no inherited live compiler/test job
to wait on or restart. The fresh entry run reproduced **417/420**, with exactly
three failing fixture paths. Exit status 2 is the make status, not a count of
test failures.

## Findings and repairs

**A — discarded copy effects stopped before the exception owner.** Handoff 85
checked volatile class copies and lowered their lifetimes, but ordinary
`expression_nonthrowing` ignored the retained conversion. Void queries discarded
the selected recipe; built-in comma queries kept only validity. Consequently
`noexcept((void)a)`, `noexcept(void(a))` and comma/conditional variants could be
true even when the required copy, destructor or copy default argument could throw.

The ordinary exception consumer now reads the NodeId-owned conversion. Query
void conversions retain their checked copy by conversion identity; comma queries
retain it in a sparse QueryId index. Constructor effect checking consumes the
existing default-argument recipes and conversions. It neither repeats selection
nor materializes a temporary or demands a body. Conversion records are passed by
value across potentially growing arenas. Completion invalidation clears only the
affected query's retained discard and its two exception-cache variants; unrelated
queries stay warm. Query/context/frame identities and source-fixed recipe reuse
remain unchanged.

The proof is N3485 §5 [expr]/11 and §4.1 [conv.lval]/2 (the temporary copy),
§5.3.7 [expr.unary.noexcept]/3 and footnote 80 (implicit calls), and §12.2
[class.temporary]/3 (destruction), in [the supplied draft](../doc/n3485.txt).
Controls distinguish throwing copies, destructors and default expressions or
conversions from noexcept counterparts, reference-returning calls and mixed
conditionals. Ordinary expressions, instantiated bodies and dependent signature
queries all use the same effects. Poison bodies remain undemanded.

**B — constant-array eligibility ignored nonliteral temporary destruction.**
Handoff 83 correctly removed a blanket syntax-based exclusion of class
constructions from the PA16 constant-array policy. Its replacement exposed an
incomplete constant-evaluation proof: `(Marker(), 3)` could be classified as
constant even when `Marker::~Marker()` changed a global. List and implicit
reference-bound constructor arguments to constexpr calls had the same hole.
The resulting readonly image/copy omitted required destructor effects.

The shared constant evaluator now checks canonical literal-type facts when
producing a class prvalue and when consuming a recorded conversion temporary.
Named object initialization remains separate. No initializer syntax scan,
class-name exception or new optimization is added. Nonliteral temporaries keep
ordinary initialization, calls and full-expression cleanup; genuinely constant
scalar arrays retain the required image/copy. The literal-type fact is memoized
at the class owner. Short-circuited and unevaluated operands remain unexecuted.

N3485 §5.19 [expr.const]/2 restricts constructor calls in core constant
expressions to literal classes, §3.9 [basic.types]/10 requires a trivial
destructor for a literal class, and §12.2 [class.temporary]/3 requires temporary
destruction. The runtime reducers check constructor/copy/destructor counts and
array values; constexpr rejection controls check the proof boundary. Explicit,
list, implicit converting, nested, member and template consumers are covered.

**C — cast prediction consumed parenthesized braced construction as a type-id.**
The expanded lifetime controls exposed `(T{}, value)` taking the C-style cast
branch, which then expected `)` at `{`. The lexical predictor now recognizes
that `{` after a type name starts a functional construction. It uses one existing
lookahead result, adds no checkpoint or grammar replay, and preserves abstract
pointer/reference cast handling. N3485 §5.2.3 [expr.type.conv] and §5.4 [expr.cast]
define the two forms. Earlier syntax tests and positive ordinary/template brace
controls pass.

The final audit controls improve **23/74 → 74/74**, with **1355** inherited
controls preserved: **1429** total. Exploratory invalid constexpr-member probes
and intermediate results remain in `/tmp/pa18-loop86`; the manifest compares the
same final 74-source corpus on entry and reviewed binaries. No existing control
or course fixture was removed or weakened.

## Architecture and optimization trace

[The combined trace](../student.tests/pa18/audit86_trace.cpp) follows an ordinary
`Value` declaration and demanded `run<2,3,5,11>` through array-bound inference,
query effects, constructor delegation, constant-data publication and lifetimes.
It preserves a readonly image for the constant long array, ordinary execution
for the effectful scalar array, one selected volatile copy, both required
destructors, and base-constructor entry identity. Student LowIR validates and
the supplied backend's ELF exits zero. All seven inherited traces also pass.

The trace records **302 tokens**, **437 parsed nodes**, **256 contextual
occurrences**, one template body transition, two delegation-entry work items,
one concrete discarded materialization and zero query-completion invalidations.
Two discard selections belong to distinct source-use and exception-query facts.
It emits **nine functions, 161 instructions and 237 operands**.

Immutable sources feed the streaming preprocessor/post-token cursor and
integrated parser/semantic consumer. Parsed templates remain source records;
instantiation uses immutable frames and source/context occurrences. Array bound
completion reuses checked explicit initializer actions, rejects non-consuming
clauses, and republishes the completed canonical type. Query substitution reads
that declaration identity. Omitted array tails remain compressed. Static member
matching indexes canonical element/signature identity and checks supplied bounds;
no mangling or rendered type becomes a semantic key.

The constructor entry worklist propagates each of three monotonic bits over the
selected delegation edge at most once, after demand completion; it does not
retry unrelated bodies. Empty-layout shortcuts require valid implicit trivial
transfers and no user destructor or volatile source. The aggregate requirement
excludes base subobjects from that shortcut. Local-specialization roots use the
existing canonical scope/type locality facts. Discard source forms are one
packed semantic bit, composed from immediate checked children after overload
selection; lowering consumes it with value category instead of rebuilding a
syntax classifier. The 15 inherited fixed-discard scaling checks retain one
selection, N recipe uses and N distinct materializations.

Sources, semantic/constant/query facts and flat indexes are TU-owned. Worklist
scratch is operation-local; function lowering resets its transient state at the
existing boundary. The driver releases parser, semantic and lowering owners
before the next input. The program/linkage owners survive through the explicit
LowIR write. There is no internal textual IR roundtrip, hot per-node owning
graph, process-global mutable cache, global invalidation or fake frontend node.
The new handoff-85 source `semantic/discarded` is registered in the tool list;
the audit adds no implementation translation unit.

The useful bounded omission is an aggregate action group with no actions:
legality comes from the checked group, profitability removes a helper body/call,
and the budget is one O(1) head check with zero code growth. Object storage and
required value-initialization zeroing remain. Nonempty/effectful groups retain
the ordinary path. The cumulative benchmark measures actual checked executable
work through final supplied-backend encoding, not just IR counts. The array
image/copy is mandated by PA16 rather than an optional transformation. Its
constant proof now includes temporary-lifetime legality before publication.
Existing eight-lane initialization, eight-wrapper named-result summaries, and
constant-evaluation work/depth budgets remain unchanged. No new fixed-point,
inlining, code-cloning or allocator pass is introduced.

## Reference proof, validation and disposition

All **23** accumulated corrections reproduce byte-for-byte from the independent
transformers. [Proof 83](reference-correction83.md) covers 15 required PA16 array
images (including two PA17 fixtures), constant union initialization and rejection
of an empty unknown-bound array. [Proof 84](reference-correction84.md) restores
five required PA11 one-byte zeroing operations in three oracles. [Proof 85](reference-correction85.md)
removes two dormant static definitions and one forbidden discarded-reference
load. The copied scalar values, complete extents, original call/deduction paths
and all other oracle bytes remain. The pinned bundle remains
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`. Reduced observations and every revised
success oracle validate; the null-reference course source is not used as proof
of defined C++ behavior. No new reference correction is made in audit 86.

| Required check / evidence | Reviewed code result |
|---|---|
| `make test-pa18` | **417/420**, exit 2; exactly the same three entry failure paths. |
| `n=18; if [ "$n" -le 1 ]; then echo '===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====='; else make test-report-through-pa$((n - 1)); fi` | **2609/2609**, exit 0; PA1–PA17 all pass. |
| `perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src` | Exit 0; the same three inherited header-division advisories. |
| Coverage and comparison | **420** unchanged stage inputs and **1686** retained fixture paths. All PA1–PA18 fixture paths preserved; only the 23 proved accumulated corrections change bytes. No course or comparison change in the audit. |
| Personal controls | **1355 inherited + 74 audit = 1429**, all pass; 34 PA16 initialization controls also pass. |
| Structure / execution | Array-storage, object-entry, volatile-access, fixed-recipe, ABI, summary/budget and inherited scaling inspections pass; eight source-to-native traces pass. |
| Performance | [Performance 86](performance86.md): full frozen cumulative A/A/ABBA corpus, checked output equivalence, compiler latency/RSS and executable runtime/size, with all historical observations retained. |

Acceptance is **PA18/O0 LowIR**, spec §9. Historical **+15%, +16 MiB and 5.5×**
targets remain diagnostics, not exit gates. PA18 mandates no numerical compiler
latency/RSS ceiling. Required semantic/contract costs and later native/debug/
self-hosting work add no gate. Correctness, coverage, mandated limits, work
bounds and profitability of optional omissions remain requirements; an incorrect
baseline is never accepted as a faster implementation.

Remaining implementation is grouped in [plan.md](plan.md): coherent class-result
ABI across definitions/calls/indirect signatures (the three course mismatches),
and the inherited class-ellipsis representation. All remain obligations; PA19
still requires a passing root through-PA18 report. Fragmenting the array,
constructor and discarded-expression work into repeated handoffs left shared
exception and constant-lifetime consumers unchecked together. That fragmentation
was avoidable. Future groups should cover each fact's semantic, query, constant,
lowering and emission consumers before packaging another handoff.

| Checkpoint ledger | Range / fixes | Evidence / disposition |
|---|---|---|
| 86, accumulated audit | `ecc308bc` → entry `890f810b` → code `f6eaf8ab`; every commit across 83–85 plus shared exception/constant-lifetime and brace-prediction repairs | PA18 **417/420**, identical three failures; earlier **2609/2609**, file/coverage pass; **1429** controls; cumulative stage-scoped performance assessed. The reviewed code tip above is the next audit baseline; full-stage remains unfinished. |
