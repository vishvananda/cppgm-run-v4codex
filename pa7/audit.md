# PA7 independent full-stage audit

Reviewed the actual source through `9cceb7913`, independently of the checkpoint
conclusions. Scope: PA7 procedural semantics plus inherited PA1–6 and the
class/member/template declaration intake exercised by the PA7 contract. Read
`spec.md`, PA7/PA5 handouts, `TESTING_AND_REFERENCES.md`, the stage commits, and
all semantic owners and frontend handoffs. The earlier implementation audit
is preserved in `682532c07`; it was an input to inspect, not completion evidence.

## Final Spec Alignment

| Spec surface | Actual architecture and evidence |
| --- | --- |
| 1: source, stream, parsing | `Preprocessor` owns immutable source buffers, interned identifiers, macro definitions and spelling slabs. `PostTokenCursor` feeds the bounded/deferred `syntax::Cursor`. Delimiter/angle prediction annotates live cursor ordinals; it does not build abandoned trees. Parser declarations call `Analyzer::consume` on the sole NodeId graph. Complete-class category lookahead is a scan, not a grammar replay. PA5's documented lexical fallback remains subordinate to known bindings. |
| 2: identity and facts | TU-owned flat indexes and geometrically grown arrays hold IdentifierId, TypeId, EntityId, ScopeId and NodeId. Types intern structural children/parameter slices. Function families and normalized signatures key redeclarations. Expressions hold value identities; calls select declarations in `Fact::entity`. Operand/argument ranges and incoming parent conversions have distinct ownership. Output spellings never key semantic equality. |
| 3: lookup and overloads | Lookup walks lexical parents and indexed using/inline/base edges, with traversal stamps for cycles. Immutable overload unions preserve using-declaration visibility. Arity filters precede conversions/deduction; a tournament and verification inspect all viable candidates. Complete target qualification signatures participate in ranking. Expected candidate rejection is a compact invalid Conversion, not an exception or formatted string. |
| 4–5: demand and cache validity | Template declarations retain the parsed pattern and parent-linked parameter environment. A key is `(pattern EntityId, canonical argument-pack ID)`; the pattern owns all context used by this declaration-only substitution. Active/success/failure declaration state is separate from emission demand. A specialization-local substitution index and TU TypeId dependence cache reuse closed structure. Member bodies use dormant/queued/active/complete states and a deduplicated queue cursor; sizeof/decltype do not enqueue bodies/emission. |
| 6: semantic handoff | Operators, calls, casts and conditions retain typed conversion facts; constants consume selected entities and operand conversions. The dump reads those facts and source wrappers without lookup, resolution or parsing. There is no LowIR input/output or lowering in PA7. This audit follows the available handoff through the semantic dump; an ELF trace starts only in later assignments. |
| 7: optimization | No source optimization levels, LowIR/MIR transform, instruction selection, allocator or encoder exist here. Mandatory constant evaluation and canonical-fact reuse have explicit owners and bounds below. No executable speedup is inferred from node counts or compiler timings. |
| 8: ownership | Source/spelling storage, identifier tables, graph and semantic arenas have one TU owner in `emit_ast`; reverse destruction releases semantics before the graph and source. Nodes and facts use non-owning IDs, not shared_ptr or individual heap nodes. Candidate/type-construction scratch vectors and flat indexes have operation lifetimes; switch labels have the active switch's lifetime. The explicit dump retains one source graph, not duplicate syntax/semantic graphs. |
| 9: work and performance | Counters observe expression/constant work, candidates, conversions, dependence, demand and canonical types without an extra traversal. Frozen A/B, A/A and ABBA records cover inherited frontend work and PA7 calls, loops/memory/floating point and repeated template demand at 1x/4x. See `performance.md` for current evidence and unchanged budgets. |
| 10: self-containment | Driver dispatch reaches this compiler's preprocessing/parser/semantic code. Rendering is an output adapter. Source inspection and the 79-file audit find no reference/host compiler delegation, cached answers, fixture recognition or textual phase transport in this path. Host g++ builds the compiler and test APIs only. |

Types and dependence/signature caches are immutable by identity. Constant facts
are keyed by a source NodeId in its single analyzed environment, including an
expected-nonconstant sentinel. Later declarations cannot change an already
selected expression. Lookup itself is uncached across insertion, so there is no
incomplete negative key or global cache invalidation. Declaration insertion
updates its own scope/name index; class completion and demand have separate
records. Deterministic printing follows source IDs and demand order, without an
ordered hot semantic map or a whole-program retry.

General class-aware calls, template bodies, general constant evaluation, dependent expression substitution,
ADL/hidden friends, ABI/layout closure, LowIR, native emission and self-hosting
are later milestones, not inferred from this intake. No source-to-executable
surface exists at PA7; generated runtime/text, spill costs, ABI/debug encoding
and executable optimization profitability are not applicable.

## Representative end-to-end traces

1. `int (*fp)(int)=choose`: source bytes become interned tokens; the declarator
   and name create nodes in the same graph. Type construction interns pointer
   and normalized function types. Indexed lookup supplies candidate EntityIds;
   target conversion selects the matching function, records its identity and
   the conversion, and leaves source spelling for the dump. No name serialization
   or second parse appears between these owners.
2. `make()(1)` and `(condition ? left : right)()`: the producing call's selection
   is separate from its returned pointer/reference. A conditional carries a
   known value identity only if both arms agree. The outer indirect call retains
   its callee graph, function-to-pointer conversion, arity and argument facts.
   `(side(),left)()` similarly retains the side-effecting comma expression.
   The API checks that wrappers do not inherit a child's argument range.
3. `unsigned long + long long` on LP64: integral rank, independently of width,
   establishes unsigned long long. Both operand conversions are recorded before
   overload ranking. Constant evaluation consumes the same targets instead of
   rediscovering the common type. For `(true ? -1 : 1u)>0`, the chosen arm is
   converted to unsigned int before comparison. Unchosen division-by-zero arms
   are type-checked but not evaluated. Required constexpr contexts reject
   nonconstant/volatile reads; comma sequences preserve their evaluated operands.
4. `consume(static_cast<void(*)(int)>(&target<int>))`: each declaration pattern
   is parsed once. Explicit and deduced types intern two argument packs and two
   specialization declarations; repeated calls reuse both identities. Closed
   type subgraphs return unchanged, dependent structure uses a local substitution
   index, and emission demand changes once. The API and fixed 1x/4x benchmarks
   test reuse, not a hypothetical fully instantiated template body.
5. `&C::used` versus `sizeof(&C::unused)`: canonical member-pointer identity
   includes the class and function cv. An evaluated selection enqueues the body
   once; processing a body can enqueue a required constructor action by typed
   object/constructor IDs. Unevaluated operands establish the type but do not
   demand a definition. The API leaves an invalid undemanded body dormant and
   verifies that repeated finish/output calls add no work or output.

## Findings and corrections

`2fd1e8333` and `9cceb7913` fix the ownership paths, with 63 independent behavioral probes and
extended fact-level assertions. The frozen pre-audit binary fails 49 of those
63 probes; the audited binary passes all 63. These are personal tests, explicitly
run, and do not change the course contract.

| Finding | Correction / coverage |
| --- | --- |
| Call targets doubled as returned-value identity; conditional/comma callees lost branches or side effects. | Separate call selection from value facts; direct-call dispatch requires a name designator. Preserve indirect callee trees and copy only value properties into transparent wrappers. Returned pointers/references and conditional/comma calls are tested. |
| Operators, casts and conditions lacked persistent conversion decisions. | Contiguous operand conversions, compound read/operand/store sequences, explicit/contextual/discarded conversion kinds and indirect/variadic promotions. API checks parent incoming fields cannot corrupt child ranges. |
| Width-based integer arithmetic and flattened cv bits selected wrong overloads. | Shared promotion/rank rules, wchar_t/char32_t promotions and full qualification-signature comparison. Equal-width signed/unsigned ranks and deep comparable/incomparable cv targets are covered. |
| Constant evaluation repeated lookup/common-type decisions and missed conditional conversion. | Consume resolved entities and operand conversions; convert the chosen conditional arm; preserve short-circuit/comma evaluation and reject nonconstant constexpr/volatile reads. PA6's constant-only surface uses the shared type rules. |
| Pointer operators accepted void/function/incomplete arithmetic and ordered null comparisons; cv union was missing. | Require complete object pointees for arithmetic/subscript, distinguish equality/ordering and construct a canonical composite pointer with required intermediate const. |
| All explicit pointer casts were accepted regardless of cast kind. | Distinguish static, const, reinterpret and C-style legality, preserve cv restrictions, check integer width, and record the selected explicit conversion. |
| Builtin fallback accepted an unresolved name in another namespace. | Recognize the builtin spelling only at its actual unqualified/global name boundary; normal qualified lookup still governs declared functions. |
| Control scopes and switches missed immediate redeclarations, converted duplicate cases/defaults and contextual nullptr conditions. | Explicit Control scope kind, parameter/local boundary checks, per-switch target type and label index, and recorded contextual conversions. Nested legal shadowing and nested switches remain accepted. |
| Unevaluated address/type operands triggered member/template emission demand. | Guard definition/emission demand during sizeof/decltype while retaining declaration/type facts. |

## Compiler work, legality and growth ledger

| Operation | Legality, profitability, invalidation and bound |
| --- | --- |
| Canonical types/signatures | Structural typed keys and immutable records; completed TypeId facts are reusable without invalidation. Work is charged to distinct type structure and parameter slices. Geometric flat-table growth has no per-entry allocation. |
| Type dependence/substitution | Dependence is a pure property of immutable types/template parameter IDs. Transform only dependent types; reuse closed types. One TU dependence fact per encountered type; one local substitution result per encountered dependent type per specialization. Expected declaration failures stay at the complete specialization key. |
| Expressions/constants | One semantic analysis and one constant success/nonconstant result per NodeId. Constant evaluation is mandatory where required and otherwise bounded by visited source edges; it preserves integer width, short circuiting and conversions. No fixed-point scan or IR/code growth. Missing PA7 binary operand facts are invariant failures. |
| Overload selection | Required candidates only; arity/category/shape filters, then argument conversions. Tournament plus verification is linear in viable candidates times argument count and qualification depth. Candidate scratch is released after the operation. No capped search can silently skip a required candidate. |
| Statement/member work | Each statement is visited once. Converted case-label membership is average O(1), released at switch exit. Each member is enqueued/processed once, including recursively discovered constructor dependencies. finish resumes at the queue cursor; no rescan of completed members. |
| Whole frontend | Work is proportional to source/tokens/nodes plus required lookup edges, candidate conversions and demanded facts. The fixed corpus checks expression/constant work <= nodes, stored conversions <= 3*nodes, dependence work <= canonical types, demand processed == queued, and 4x scaling. All retained data is charged to TU memory; no unbounded optimizer or emitted-code growth exists. |

The extra conversion records and rejection checks are required correctness
work, not a runtime optimization claim. Their compiler latency/RSS and host
text cost must pass the existing stage budgets and the separate before/after
PA7 comparison. No budget is relaxed to accommodate the fixes. The direct PA7 loop/memory/
floating-point case costs 4.91–5.29% more compiler latency to preserve required
conversion facts, within budget. Full performance coverage uses the documented
longer AST-declaration recheck; the rejected short samples remain in the record.
All 1,154 audit observations and the exact accepted/superseded scope are in
[performance.md](performance.md).

## Validation and handoff ledger

Source validation at `9cceb7913`:

- `make test-pa7`: 186/186; `make test-report-through-pa7`: 684/684, 7/7 stages.
- `perl scripts/cppgm_file_audit.pl --stage pa7 --paths dev/src`: 79 files pass.
- 63 independent audit probes; original 29 personal cases plus multi-TU isolation.
- ASan/UBSan/leak builds: all 186 PA7 and 105 PA6 fixtures, both PA7 personal
  suites, PA7 conversion/identity/demand API and inherited PA5/PA6 APIs pass.
- Inherited personal checks: PA6 57; PA5 core 10, extended 15, prior audit 19.
- Personal rejection probes require exit 1 and reject sanitizer diagnostics;
  distinct sanitizer exit codes also protect the inherited negative-fixture run.
- No course fixture, reference, harness, handout, root spec or discovery rule
  changed. No generated objects/logs/outputs are included in commits.

| Handoff since PA6 / prior checkpoint | Independent disposition |
| --- | --- |
| `c2837a0e6` | Stage baseline/plan inspected; no implementation to trust as evidence. |
| `ec0c55093` | Procedural expression, conversion, call and statement ownership reconstructed; corrections above close discovered gaps. |
| `ec6b78425` | Member identities, demand and template declaration intake traced through actual source and tested independently. |
| `d5ab80eb4` | Closed-type dependence cache checked for canonical key, immutability, dependent-only work and bounded counters. |
| `682532c07` | Saved final evidence reverified after reproducing the PA6 baseline and recovering the PA7 binary with exact recorded hashes. Historical checkpoint observations remain archival; current frozen campaigns supersede them for the final state. |
| `2fd1e8333` | Cohesive semantic corrections and personal/API tests committed. No earlier PA7 implementation handoff remains unreviewed. |
| `9cceb7913` | Final ownership review keeps cast/builtin forms on their originating nodes; three comma-form probes close the remaining rendering handoff. |
| Final evidence commit | Complete fixed-corpus coverage and longer recheck/direct-PA7 verifiers pass with unchanged budgets; all 1,154 observations retained. Final required gates pass; intended changes committed and clean status checked at handoff. |
