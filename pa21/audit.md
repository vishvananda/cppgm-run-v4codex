# PA21 checkpoint audit 105

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Audit entry: `3b87e462701e268eeaaaa9c9ddeb4594ec74439b`.
Last reviewed commit: `f65eae8d7d434735a0ce981173a347eb8f8a1e59`.
Target: **PA21 full-stage**. Phase: **checkpointAudit complete**.

The checkpoint passes its progress-preservation gates. PA21 remains **71/116
passing, 45 failing** and must not advance to PA22. This audits the accumulated
implementation and its unfinished integration; it does not certify full-stage
completion. Earlier stages pass **3596/3596**.

## Range and review

This is the first PA21 review; both inherited markers pointed to the stage base.
All **11 entry commits**, their combined changes and the audit repair were
reviewed: **12 commits / 64 implementation paths**. The
[manifest](../student.tests/pa21/audit105-review.json) records full commit IDs,
subjects, source hashes, frozen binaries and entry reproducers. Entry was clean,
with no surviving compiler/test process. The preceding accepted handoffs are
progress, proved by commits and validation rather than an assumed live wait.

| Commits | Reviewed contribution and interactions |
|---|---|
| `79204189`, `30fe6353` | Stage boundary/plan and reduced RTTI oracle correction. |
| `9f2181f9`, `f4224e0b` | RTTI query/ABI identity, evaluation, casts, allocated-copy entry choice, parser prediction and evidence. |
| `e835d6dc`, `1c541f84`, `fa079cde` | Captures/forwarding/cv, selected copy/default recipes, partial construction and generated-copy unwinding, including ordinary classes. |
| `e2af8963`, `f03b9371`, `511fe9b9`, `3b87e462` | List selection/deduction/query recipes, backing arrays, constexpr/range/lifetime use, helper dependencies and inline checking versus emission. |
| `f65eae8d` | Fixed RTTI recipe reuse and copied-subobject lifetime preservation, controls and trace/measurement harnesses. |

Every implementation diff was inspected with its owner/callers. Follow-up commits
were also reviewed individually to distinguish intermediate defects from final
behavior. The 102–104 design/evidence records remain historical; this audit
supersedes their pending-review wording.

## Findings repaired

**Repeated fixed RTTI checking.** `template_expression.cpp` excluded dynamic
casts and most typeid forms from retained facts. A function containing three
nondependent RTTI operations created 600 RTTI expression records across 200
specializations. The fixed-expression owner now retains checked nondependent
type/operand recipes; dependent types and local-class identities still substitute.
Reuse evaluates runtime operands only when required and applies recorded RTTI
demand separately. That example now creates **3 records**. Missing RTTI facts
raise an invariant error rather than returning the zero record. Controls check
static unevaluated operands, exactly-once dynamic calls, captured operands and
dependent local identities; the larger curves verify bounded recipe reuse.

**Bulk prefix copying erased destructor obligations.** Loop 103 added cleanup
after generated subobject actions, but prefix coalescing could replace a nonempty,
trivially copied class with a storage action. If that class had an observable
destructor and a later copy threw, its completed destination subobject was never
destroyed. Ordinary classes, base subobjects, closures and captured arrays were
affected. All five reduced host-unwind controls fail at entry and pass now.

The shared `prepare_transfer` owner now preserves nontrivial destructor boundaries
in potentially throwing construction. Nonthrowing construction and assignment
retain their policy. Typed field/type/cleanup identities reach lowering without
fake frontend nodes or post-hoc symbol searches. N3485 [except.ctor] 15.2/1–2
([local text](../doc/n3485.txt:21482)) requires reverse destruction of completed
subobjects when initialization throws; a trivial copy does not waive that rule.

## Architecture trace and ownership

The [source](../student.tests/pa21/audit105_trace.cpp) constructs a polymorphic
declaration and two demanded `inspect<N>` specializations containing class-element
lists, value captures, ranges and RTTI/casts. Three calls reuse the two bodies and
check values and final live count. The
[trace](../student.tests/pa21/audit105-trace.json) preserves full LowIR, statistics,
native hash/payload disassembly and compiler syscalls.

- Immutable buffers and the streaming cursor feed one retained source graph.
  Scope prediction does not parse abandoned trees; template occurrence contexts
  reuse parsed regions. The trace has **489 parsed nodes**, **2 body transitions**,
  **52 inherited expression facts**, **2 closures / 4 capture edges**.
- Types/declarations/occurrences and parent-linked template environments have
  compact identities. Query/list/transfer validation has distinct active/success/
  failure states. Fixed RTTI decisions are shared, while runtime object identities
  stay occurrence-specific. **1 list plan / 2 list objects / 1 representation**
  distinguishes the recipe from backing storage for each specialization.
- Lookup uses indexed lexical/associated scopes and candidate sequences. The
  language-mandated standard library declarations are resolved in namespace std;
  list aliases and fields then use identities. Arbitrary library field names,
  explicit user bodies and other namespaces' similarly named classes are preserved.
- `(closure, object)` capture edges retain source cv, mode, forwarding and checked
  conversions. List plans retain element conversions and backing/pointer/count
  identities. Nested/static/reference retention uses explicit lifetime owners;
  copying the wrapper does not extend backing lifetime again.
- Checking, layout, definition, vtable and emission demands remain separate.
  Mandatory unused inline bodies are checked without emitted entries. Deduplicated
  queues visit required dependencies, with no new global retries or cache-generation
  invalidation. Lowering-only RTTI completeness/linkage caches see the completed
  semantic TU and cannot become stale from later source declarations.
- Typed lowering consumes conversions, layout, lifetime and ABI facts directly.
  The trace has **3 RTTI expression records / 2 emitted RTTI types**. ABI identities
  merge external symbols across TUs; rendered manglings are output, not keys.
  Compiler syscalls show one `execve` and no reference/host-compiler invocation.
- TU arenas/vectors own semantic facts and indices; shorter-lived candidate
  buffers and function partial-construction/cleanup caches have explicit release
  or reset boundaries. Program-owned LowIR/linkage survive TUs for the output view.
  This range introduces no per-node owning pointer or process-global mutable cache.

The authorized supplied backend consumes that LowIR and produces the checked
**2760-byte executable payload**. Ordinary and instrumented/validated LowIR are
identical. Disassembly retains expected O0 calls, frame loads and loop branches;
no spill/allocator improvement is claimed. Student MIR, allocation, ELF encoding
and self-hosting remain PA24–34 owners, not completed PA21 implementations.

## Legality, profitability and budgets

The transfer audit traces constructor/noexcept and destructor facts through prefix
selection, typed actions, cleanup IR and supplied encoding. The repaired legality
test retains ordered memberwise transfer and reverse cleanup when coalescing would
discard observable destruction. No profitability claim can justify that omission.

Existing copy/helper policies still require recorded storage-independence,
volatility, transfer and lifetime proofs. List backing elements construct in final
slots without aggregate transport dependencies. Completed summaries are owner-keyed;
unknown proofs keep conservative actions. RTTI sharing removes duplicate semantic
work without deleting runtime operations or strengthening alias/effect promises.

Work follows source nodes, demanded facts/candidate edges and output. RTTI properties
are cached by type; name bytes count toward produced output. Captures, list elements
and transfer prefixes use linear traversal. Cleanup retirement memoizes reachable
states and nested backing retention uses a worklist. Array construction/copy/
destruction retains the **eight-element expansion cap**, then loops. No new fixed
point, inlining, unbounded growth/search or global optimization was introduced.
The unfinished general EH owner must still carry complete try/handler context;
these native controls do not certify its remaining source/comparison requirements.

[Performance](performance105.md) records A/A and ABBA compiler latency/RSS,
checked runtime/payload and work counters. Historical 102–104 measurements and
frozen binary hashes were preserved and verified. The inherited **+15%, +16 MiB,
5.5×** targets remain diagnostics under spec §9, as in
[PA20's acceptance](../pa20/final-audit-performance.md); they introduce no new
exit gate. Correctness, coverage and mandated work/growth bounds remain gates.

## References and validation

The sole accumulated reference change is **pa21-rtti-name-102**. Its
[reducer/proof](reference-corrections102.md), bundle revision and reconstruction
from original bytes were verified. N3485 [expr.typeid]/4 and [temp.type]/1 preserve
template identity; the [Itanium substitution rules](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-compression)
place components before containing structures. The fixture must name `vector`
(`S4_`), not `allocator` (`S5_`). The one changed name byte and matching ABI
metadata match the manifest; the reduced host/native check passes. No new oracle
correction, test/status change or comparison relaxation was made.

[Final validation](../student.tests/pa21/audit105-validation.json):

- `make test-report-through-pa20`: **3596/3596**, exit 0, with separate earlier
  focused controls also passing.
- `make test-pa21`: **71/116**, exit 2, the exact **45-failure set unchanged**.
  No new failure is offset by another passing test.
- `perl scripts/cppgm_file_audit.pl --stage pa21 --paths dev/src`: exit 0; the
  same three advisory header-body warnings, no new diagnostic or limit change.
- All **116** PA21 input identities, required statuses and comparison rules remain.
  The **15048-path** earlier/current contract inventory differs from stage base
  only in the proved historical RTTI reference.
- Personal controls: composition **16/16**, new unwind **5/5**, captures **64/64**,
  inherited captures **58/58**, capture unwind **13/13**, lists **51/51**, list
  unwind/static finalization **25/25**, host RTTI **48/48**. Freestanding RTTI is
  **59/60**: the already documented supplied-runtime public-base-inside-private-
  derived discrepancy remains and passes with the host runtime. Both observations
  are preserved; this is no waiver of a required fixture or compiler behavior.

## Remaining work and ledger

Two broad groups remain: source exception-object/handler/rethrow semantics, and
complete lifetime/control-flow integration across construction, destruction,
arguments, conditional temporaries, lists and local-class/template identities.
The latter includes the two recorded list LowIR failures; all 45 failures remain
required work. Native success does not waive relaxed LowIR comparison.

Feature boundaries were useful, but separate cleanup/demand follow-ups
(`1c541f84`, `f03b9371`, `511fe9b9`) left avoidable fragmentation across shared
owners. The next handoff should finish EH/lifetime paths and their capture/list/
template compositions with integrated validation, rather than split each
fixture-shaped repair into another handoff.

| Audit | Range and disposition |
|---|---|
| 105 | `ac988ea3..f65eae8d`; all accumulated handoffs reviewed; RTTI reuse and copied-subobject cleanup fixed; proof, trace and performance verified; earlier/file/progress gates pass; 45 unchanged failures remain implementation work. |

The marker names the committed code tip. These records follow without further
implementation edits, providing an unambiguous baseline for the next audit.
