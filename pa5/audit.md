# PA5 independent final architecture audit

Scope: **PA5 full-stage**, Linux x86-64 source-to-AST mode. Independently read
`spec.md`, the handout, `parsing.md`, the shared grammar, testing policy, all
PA5 stage commits, every syntax implementation owner, and the inherited
source/preprocessor/post-token handoffs. Implementation checkpoints were leads,
not proof. The previous goal turn supplied a completed implementation; this
review made progress through source reconstruction, new failing probes, fixes
and current validation.

Stage base: `f8ec565f9`. Implementation handoffs independently reviewed:
`56944a2bc` (plan), `d5e52d04d` (cursor/graph/parser/driver), `6d67335c1`
(remaining syntax), `8365a1124` (prediction indexes and retained facts), and
`7e8d10bf2` (completion ledger). Correctness implementation: `9cfce7949`; final source: `f0a0f614a`
(removes the optional indentation change after isolated profitability testing).
The final source change and both profitability paths were reviewed; no unaudited
PA5 implementation handoff remains.

## Final Spec Alignment

| Spec surface | Actual implementation and conclusion |
| --- | --- |
| Source and streaming (§1) | `Preprocessor` owns immutable `SourceBuffer`s, identifiers, macro facts and expansion storage. `PostTokenCursor` pulls phase-7 values; `syntax::Cursor` copies only retained literal facts and keeps unresolved lookahead in a geometric ring. Consumed tokens are discarded; no successive owning token streams. |
| Canonical identity (§2) | Identifiers are 32-bit IDs in a TU-owned flat table. Syntax uses 32-bit node, location and scope IDs. Names/type-ids/template arguments have structured links; rendered text never keys lookup. Canonical semantic types/declarations are PA6+ surfaces. |
| Lookup (§3) | `Names` indexes `(ScopeId, IdentifierId)` in flat open addressing. Terminal qualified lookup follows the named scope and import/base edges; unqualified lookup additionally visits lexical parents. Qualifiers filter scope-bearing categories. Cycles use reusable scratch visitation/worklists, not retries over unrelated declarations. |
| Templates/demand (§4–5) | Template parameter categories have lexical owners and override immutable spelling hints. Each template body is built once and retained. The delimiter and angle annotations belong to deferred token ordinals; lexical hints are keyed only by immutable identifier identity. No cached semantic lookup answers need invalidation on insertion. Instantiation and demand states do not yet exist. |
| Typed construction (§6) | Parser routines build one graph directly. `DeclaratorFacts` propagates the name, first derived operator and function parameter scope while parsing, avoiding reconstruction from the AST dump or repeated nested-declarator walks. There is no LowIR surface at PA5. |
| Optimization (§7) | No executable transforms, optimization levels, MIR, allocation, ELF or ABI encoding exist yet. Audited retained compiler optimizations are prediction indexing and immutable lexical hints; their legality, owners and bounds are below. No runtime gain is inferred from node counts. |
| Allocation (§8) | Nodes are 32-byte array values with non-owning child/sibling/detail IDs. Locations, literals, decoded bytes and flat name facts grow geometrically. No per-node owning pointer/allocation or recursive destruction. Transient cursor/lookup/rendering buffers have explicit TU or view lifetimes; no accumulated process-global state. |
| Complexity/evidence (§9) | Work is charged to input bytes, tokens, required scope/import edges, nodes and output bytes. Optional fused frontend/emit times, process peak RSS, growth, delimiter/angle/hint work and name/scope counters observe existing work. The final ordinary/stats campaigns are separate. |
| Self-containment (§10) | Driver uses the shared implementation directly. No reference/host compiler, answer cache, subprocess, textual token/AST roundtrip or fixture recognition implements any required result. Registered sources remain in `dev/frontend_source_sets.mk`. |

PA5 deliberately ends at structured syntax. Full type checking, overload
resolution, specialization demand, direct typed LowIR, MIR, native encoding,
executable runtime/text and actual self-hosting are **not applicable yet**.
These are explicit later-stage obligations, not deferred PA5 fixes. The shared
graph must be refined/annotated by later semantics rather than copied into a
second tree or reconstructed by parsing its text view. Attribute convenience
syntax is outside the shared PA5 grammar; no future attribute semantics are
claimed for the parser's current skip adapter.

## Representative end-to-end traces

**Template and declaration.** For `template<class T> struct packet { T items[3];
T at(int i) { return items[i]; } }; packet<int> object;`, immutable source bytes
flow through character/PP/macro cursors and phase-7 conversion. The syntax
cursor records source locations and interned IDs. `template_parameters` enters
a fresh lexical frame and records T's type category. `class_specifier` publishes
a class scope; category lookahead indexes delimiters and skips nested bodies.
`simple_declaration`, `declarator` and expression routines construct the array,
function parameters, return and subscript exactly once. `template_decl` publishes
packet's category/target scope in the enclosing scope. `packet<int>` stores a
NamePart with TemplateArguments and a TypeId; it does not instantiate or replay
the class grammar. `write_ast` traverses the same IDs and renders the requested
view. The direct graph is the PA6 handoff; there is no ELF path to invent here.

**Declarator and scopes.** For `int (*f(int T))(int){return T<2;}`, the inner
function declarator returns its parameter scope and first derived operator.
The outer pointer/return-function suffix cannot overwrite those facts. The
body sees T as a value, so `<` is relational. A callback's nested parameter
scope never escapes into the enclosing function's parameters or body. For
`int C::f(item p)`, the qualified class scope supplies suffix/parameter syntax
categories before parsing, and the same parameter frame owns suffix/body use.
Each branch and loop owns the required condition and controlled-body scopes;
scoped enum values remain in their enum, and anonymous namespace reopenings
reuse one identity per lexical owner.

**Literal/source ownership.** A macro following `#line` carries its physical
file/offset anchor and presumed filename/line through preprocessing to an AST
literal. The graph retains scalar bits or decoded array bytes, element count,
type, numeric prefix and user suffix by value/ID. Post-token scratch may be
destroyed before those values are read. Adjacent strings crossing an include
produce one decoded value, anchored in the first fragment's file; only offsets
from that physical file can extend its range. The API validates the complete
`leftright` value and its exact first-fragment range after this handoff.

**Release boundaries.** `emit_ast` owns PP, post cursor, Ast, syntax cursor and
Parser in one operand-loop scope. The source/identifier owner outlives every
borrower. The Ast survives parsing and supplies the requested output; then all
TU graphs, name facts and source buffers are released before the next operand.
The writer has an explicit traversal stack. Indentation is a temporary string
for each requested view line, charged to output work, not ownership of a graph
node. The isolated reuse experiment below did not justify retaining a change.
Inline syntax rendering follows structured detail links. Repeated primary-file
benchmark operands verify that work counters reset and outputs stay identical.

## Findings and full ownership fixes

1. Scoped enum enumerators, while/do conditions, unbraced branches and nested or
   comma-separated function prototypes leaked name categories. Scope entry and
   restoration now live at their grammar owners. For-loop conditions consume
   the existing structured condition-declaration production.
2. Prediction, parsing and using/alias resolution disagreed on qualified lookup.
   Global scope ID 0 was also treated as unknown. A distinct unknown-scope
   sentinel, common qualified/import lookup and scope-category qualifier filter
   now govern every path. Known `::T` values override T's lexical hint; imported
   `using m::item` resolves; namespace aliases still work when hidden by values,
   as required by the unchanged namespace-shadowing fixture. All typedef
   declarators retain the aliased target scope. Anonymous namespace reopening
   no longer constructs a second category scope.
3. A trailing declarator child was mistaken for a function test, so braced arrays
   and function-pointer objects entered the function-body parser. First-derived-
   operator facts now distinguish objects/functions while parsing, including
   nested declarators and functions returning pointers to functions.
4. Concatenating string fragments from different files combined unrelated byte
   offsets, violating `begin <= end`. Fixed at `PostTokenCursor`, including the
   first-fragment literal-operator split, rather than masking it in the AST.
5. Relocating an unnamed non-type parameter pack left a stale last-child link.
   `Ast::take_first` maintains both list ends and detaches the sibling link in
   O(1). API traversal of the unnamed-pack graph now passes.
6. Optional name/scope counters replace the previously unused name-probe field.
   They charge occupied-slot iterations plus terminal slot access and visited
   scopes; they do not trigger extra lookups. An indentation-buffer optimization
   was independently measured and removed when it failed to establish repeatable
   benefit beyond noise. All candidate/control observations are retained.

Nineteen new personal regressions inspect syntax choices, including relational
operators, declaration categories and exactly one function/two braced objects
in the combined declarator case. The graph checker also runs every successful
course translation unit (181 inputs including companions), all audit probes,
and the independent template-depth/location/literal/lookup tests. No fixture,
reference, harness, timeout, discovery rule or comparison was weakened.

## Compiler transformations and pipeline budgets

| Change | Legality and invalidation | Work, storage and profitability evidence |
| --- | --- | --- |
| Delimiter/angle indexes | Store punctuation matches for immutable deferred token ordinals, including logical halves of `>>`; do not memoize semantic categories. Annotations expire on consumption, and unsuccessful predictions retain ordinary parser decisions. | One delimiter visit per produced token; nested angle work <2x tokens in fixed/API workloads. Storage O(maximum deferred tokens + nesting depth). Historical frozen A/B measurements establish the repeated-scan benefit; final B must retain the bounds. |
| Identifier lexical hints | Computed solely from immutable identifier spelling. Every current binding/parameter category takes priority, so insertion cannot stale a cached semantic answer. | Once per queried identifier spelling; 1,024 repetitions of a 1,024-byte identifier are checked directly. TU-owned flat byte cache. |
| Declarator facts/scopes | Carry syntax-derived name/operator/scope during the sole grammar parse. Scope facts remain owned by the correct parameter/body; no speculative tree or global invalidation. | O(tokens/nodes) construction and O(depth) parser stack. Scope lookup additionally charges only relevant lexical/import edges. Removing unused per-object parameter scopes offsets necessary control/prototype scopes. |
| Indentation reuse (rejected) | The isolated control changes only scratch ownership, with identical AST/output bytes. | Classes: paired +0.83%/-0.60% gains against 0.78% noise. Expressions: +0.30%/+0.14% against 0.28% noise; no RSS benefit. This does not prove repeatable profit, so final source restores the original renderer. |

No fixed-point pass, global retry or optimization-driven code expansion is
present. The whole source/graph pipeline has linear geometric storage plus
language-required macro expansions/lookahead and scope edges; requested deeply
nested indentation can itself be quadratic in nesting depth and is charged to
output bytes. The measured fourfold input/depth envelopes are <6x wall and
<5x RSS +1 MiB; paired regression allowances are 10% wall plus calibrated noise,
15% RSS +1 MiB and 15% host compiler text. These budgets were fixed in the plan
before the final campaign. No executable profitability, ABI/debug preservation
or code growth claim is made at this stage.

## Performance, validation and ledger

The prior campaign's final binary hash was independently matched to frozen A
and its 168 observations, input generator, output hashes, paired budgets and
nested-work bounds reverified. It remains historical evidence. The final
ordinary campaign at `f0a0f614a` has 168 observations, eight startup probes,
24 separate phase/work runs and 28 ordinary/stats calibration observations.
All hashes, exact outputs, protocols, startup ratios, budgets and scaling gates
pass. Host text grows 2.36%; expression latency grows 1.07–1.64% and its largest
RSS grows 2.53%, within the predeclared correctness-work budgets. The smallest
workload is 48.9x startup. Nested angle work remains <2x tokens, and fourfold
nested depth takes 3.898x final wall time. Templates' small regressions and noisy
groups are disclosed; no broad performance improvement or executable gain is
claimed. [Performance evidence](performance.md) contains the full table, paired
spread/noise, instrumentation comparisons and the rejected optimization ledger.

Current-state completion checks:

- `make test-pa5`: **188/188**; `make test-report-through-pa5`: **393/393**, all
  five stages pass. The root report is the required exit gate, not a reused
  checkpoint log.
- `perl scripts/cppgm_file_audit.pl --stage pa5 --paths dev/src`: **62 files pass**.
- Personal core **10**, extended **15**, and audit **19** cases pass. PA2's
  **316** personal cases (7,062 integer combinations) and PA4's **168** pass
  after the shared post-token source-anchor fix.
- Final isolated ASan/UBSan build with leak detection: all **188** unchanged
  PA5 contracts, all **19** audit regressions, and **181 + 19** direct graph
  inputs pass. API tests also check decoded values after cursor destruction,
  source ranges, qualified lookup mutation/cycles and bounded angle/hint work.
- The legacy matching-binary verifier, final/candidate/isolation verifiers,
  standalone isolated-control rebuild/hash comparison and whitespace checks
  pass. No reference/fixture/harness/coverage/timeout changes are present.
- Source fixes are committed as `9cfce7949`; the measured profitability rejection
  is `f0a0f614a`. This final evidence commit consolidates the plan/audit and
  all three independent datasets. No unreviewed handoff, known PA5 correctness,
  self-containment, timeout, file-audit, architecture or performance gate remains.
  Final repository status is checked after committing the evidence.
