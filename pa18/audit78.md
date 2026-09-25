# PA18 checkpoint audit 78

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Previous Last reviewed commit: `8dc4636d23a38f2bbcc8662b88b07f7979715c2d`.
Entry: `0ed4fe5f0c92e32b4f9ac1ff2e4404c89b35a190`.
Last reviewed commit: `82fca940b1849d90deffbaba29ee162946f3e23c`.

**Checkpoint audit passes; PA18 full-stage remains incomplete.** The review
covers every commit since the preceding code tip, all 52 changed implementation
and registration files, and interactions across handoffs 75–77. The previous
record is preserved verbatim in [audit74.md](audit74.md). The
[evidence manifest](../student.tests/pa18/loop78-evidence.json) records full
commit IDs, patch/source hashes, checks, coverage, failure paths and artifacts.

## Complete range

| Commits, chronological | Review and interactions |
|---|---|
| `9fa23653` | Prior audit records, performance observations and retained obligations. |
| `de0f5228`, `6229b49a`, `b87de70e` | Both list/cast implementation commits and their handoff: retained list formation/validation, ordinary narrowing, constant/exception consumers, strings, downcast adjustment, typed ABI adapters and parameter representation. The record commit also corrects benchmark classification and continues its interrupted observations; both original files remain. |
| `e052e939`, `bff709db`, `c8a2aad8`, `2ce99c6a` | All three inherited-constructor implementations and their handoff: canonical proxy/head identity, notional signatures, omitted parameter timing, forwarding transfers/defaults, lifetimes, constexpr execution and exception demand. Reviewed intermediate patches as well as their final combination. |
| `86e142ba`, `b6294394`, `975e6162`, `0ed4fe5f` | Nested declaration/definition separation, deferred source regions, explicit nested instantiation/specialization, indexed ambiguity and alias access, plus the handoff. Reviewed class-demand interactions with both preceding groups. |
| `82fca940` | Three ownership repairs below, independent controls, combined trace and accumulated benchmark harness. Validated before committing; subsequent records change no implementation or test code. |

All **239 historical hashes** checked against their owning code/record revisions
or retained artifacts match. `benchmark75.py` belongs to `b87de70e`, where its
continued-run classification was corrected, rather than the preceding code tip.
That distinction and the initial interrupted observations are preserved. The
preceding goal turn is verified progress from its committed handoff and evidence;
no background process needed resuming at audit entry.

## Findings and repairs

**A — allocation queries lost initializer syntax, object lifetime and immediate
access failure.** `new_query` flattened braces into ordinary constructor-call
arguments. This accepted scalar narrowing and rejected valid aggregate allocation.
The shared call/list consumer also treated the allocated object as a temporary,
rejecting a private/deleted destructor or attributing its exception effect to
allocation. Allocation lookup/access and placement conversions could turn expected
substitution rejection into a hard diagnostic or omit access validation.

The source query now retains its braced child. Its canonical key records allocation
context, and the list plan carries that fact through validation and exception
analysis. Formation and validation remain separate cached queries; the allocation
bit survives validation-key construction. Root destruction is omitted only for the
allocated object; aggregate subobjects and argument temporaries retain their
requirements. Indexed allocation ambiguity, access and selected placement
conversions return structured immediate-context failure. No constructor body is
required by an unevaluated query, including inherited notional signatures.

Controls cover narrowing, aggregate nesting/strings, explicit/private/deleted
constructors, root versus subobject destructors, noexcept, global versus class
allocation, ambiguous allocation, private placement conversion and dormant poison
bodies. Governing rules: N3485 §5.3.4 [expr.new]/9,11,15,17, §8.5.4
[dcl.init.list], and §14.8.2 [temp.deduct]/8. The allocation/destructor distinction
is explicit in [the local draft](../doc/n3485.txt):6504–6520.

**B — declaration presentation forced semantic completeness.** Handoff 75 avoided
entity-table growth during lowering by completing namespace function parameters
in the representation pass. A declaration `void f(A<int>);` consequently demanded
an unused, possibly ill-formed class definition. Nested member classes exposed the
same phase-boundary defect in lowering itself.

Declaration-only parameters now retain their incomplete semantic identity.
The explicit LowIR declaration view uses a pointer with unknown object size,
analogous to its existing incomplete-result convention; it does not invent a
layout or instantiate members. Actual calls and definitions must establish a
complete type in semantics. Complete types continue to use their established
value ABI. This preserves the entity-table boundary without using layout as a
lookup or recovery operation.

A cross-TU control exposed the necessary counterpart: a later declaration may
supply the complete value ABI. A program-owned index keyed by output function ID
marks only incomplete signatures. Later declarations refresh that one signature;
completed signatures are not overwritten by later incomplete declarations. The
same mechanism repairs the inherited incomplete-result case. There is no global
retry, textual key, generation flush or per-function marker allocation for ordinary
complete signatures.

Controls cover poisonous unused template/nested parameters, plain incomplete
classes, later completion/specialization, required definition-time rejection,
and parameter/result/both signatures in either source order. Six multi-TU controls
validate externally declared LowIR; six supply definitions and execute with checked
results. Proof: N3485 §8.3.5 [dcl.fct]/9 limits the completeness requirement to
function definitions, §5.2.2 [expr.call]/4 requires complete by-value call arguments,
and §14.7.1 [temp.inst]/1–2 separates declaration and definition demand.

**C — constexpr constructor actions lost their later emission owner.** A retained
list query could constexpr-evaluate an inherited constructor before runtime use.
Its synthetic actions were completed under the caller's context, so their base
constructor/default/transfer dependencies were not attached to the constructor.
Later emission reused the actions but could produce a call to an undefined symbol.

`constructor_actions` now scopes the current function and body-evaluation depth to
its own declaration identity, restoring both on exit. Existing deduplicated deferred
use edges therefore attach to the synthetic body and activate when it is emitted.
Actions and body checks are not repeated; constexpr-only use still emits only
`main`. Controls cover nested owners, inherited chains, defaults and class-value
copy/move forwarding. N3485 §12.9 [class.inhctor]/8 specifies the forwarding body;
§3.2 [basic.def.odr], §5.19 [expr.const] and §14.7.1 distinguish its semantic
availability from runtime definition demand.

**35/35 semantic audit controls** pass, versus **17/35** on frozen entry.
**12/12 declaration controls** pass, versus **7/12** on entry. Every executable
positive control validates LowIR and runs through the supplied native backend;
rejection controls check exit status. No course fixture, reference, harness,
comparison rule or bundle was changed.

## Architecture and optimization trace

[The combined trace](../student.tests/pa18/audit78_trace.cpp) follows ordinary
`main` and `Outer<int>::Derived` from immutable source through the streaming
preprocessor/cursor, integrated parser/semantic construction, canonical template
and query facts, typed lowering, explicit LowIR output and the supplied backend's
ELF. Nested `Base` and `Derived` definitions are demanded; poisonous `Dormant` is
only declared, including as a function parameter. A constexpr list use establishes
the inherited default before runtime construction; `seed()` increments `effects`
exactly once and execution checks the resulting field is nine. Allocation queries
exercise both narrowing rejection and a dormant deleted root destructor.

The trace has **288 tokens, 419 parsed nodes and 168 occurrence records**. It
performs three nested declarations, two nested definition transitions, one template
body transition and three source body checks. Three deferred use edges are processed
once. Two forwarding-argument records survive. Five definitions are emitted:
`main`, `seed`, the derived forwarding entry and the distinct complete/base ABI
entries of the demanded base constructor. Dormant bodies and the allocation-only
constructor/destructor are absent. LowIR has **42 instructions and 65 operands**;
explicit validation and checked ELF execution pass.

Parsed source regions are retained once. Projection gives compact source/context
occurrences and separate demand roots; substitution never invokes the parser.
Canonical query keys include type, context, operation, children, frame substitutions
and the new allocation context. Class, forwarding and exception states retain
not-started/active/success/failure at their narrow owners. Access recipes survive
aliases which erase type arguments. Lookup traverses indexed lexical/base edges,
and every ambiguity consumer checks the sentinel before indexing entities.

TU vectors/arenas own source, occurrence, type, query, conversion, forwarding and
class facts. Local candidate/argument scratch dies at each operation. Selected
conversions and subobject actions pass directly to typed lowering; no resolution,
synthetic syntax, serialized intermediate phase or whole-program name recovery
was introduced. The driver releases each frontend/lowering TU before advancing.
Only output/linkage identities, including the new incomplete-signature index,
survive across TUs, and die with the output program. Hot expression records remain
36 bytes. No process-global mutable cache or individually owned hot node was added.

Graph controls preserve local completion invalidation: list queries invalidate
three consumers at each 32/128/512-owner scale; nested and inherited completion
controls each invalidate one. Omitted array tails keep constant plan/field counts.
Nested dormant definitions create zero member occurrences; demanded and repeated
nested definitions retain the established linear/one-definition bounds. Forwarding
query-only demand creates no base bodies, and repeated queries reuse one recipe.

The useful fact traced into executable code is the selected constructor and its
parameter/default/object identities. Lowering consumes these once, preserving ABI
entries, access, lifetimes, exception effects and source locations. These repairs
implement required semantics, with no optional optimization or speculative growth.
O0 adds no fixed-point search, inlining, unrolling or new optimization budget.
Unknown layout/effect facts remain conservative. Existing work/growth policies are
unchanged; own native selection, register allocation, debug encoding and self-hosting
belong to later stages. [Performance 78](performance78.md) reports compiler
latency/RSS and checked runtime/size together, with frozen A/A and ABBA evidence
for the complete accumulated corpus and the previous reviewed binary.

## Validation and disposition

| Check | Evidence at reviewed code tip |
|---|---|
| `make test-pa18` | **388/420**, exit 2; exact same **32** entry failure paths: six status failures and 26 LowIR comparisons. |
| Required prior-through command | **2609/2609**, exit 0. |
| PA18 file audit over `dev/src` | Pass, exit 0; same three inherited header-division advisories. |
| Coverage/comparison | All **420 inputs and 1,686 tracked fixture/reference files** unchanged, including across the full review range. |
| Personal controls | **875 inherited + 35 semantic audit + 12 multi-TU controls**, all pass. |
| Accumulated repaired course paths | All **nine** retain checked execution; also execute the cast reducer and the inherited-alias case that remains a required comparison failure. |
| ABI/graph/trace | Seven ABI checks, list/forwarding/nested scaling and completion controls, and validated source-to-ELF trace pass. |
| Performance | Frozen compiler/runtime/RSS/text evidence and PA18/O0 acceptance in performance78.md; historical measurements retained. |

Earlier reference corrections [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md) retain their
reducers, standard proofs and bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
There is no new reference correction in this audit.

Remaining implementation stays in two broad ownership groups in [plan.md](plan.md):
source signatures/member/alias syntax, and ordinary LowIR initialization/result
facts. All 32 course failures and the inherited class-ellipsis/nested-alias-cast
reducers remain obligations. PA18's root through report must pass before PA19.
Splitting list queries, forwarding and nested completion across handoffs left
allocation lifetime, representation and constexpr-emission interactions unchecked
together. That fragmentation and repeated packaging were avoidable; future work
should validate all consumers of each ownership group before handing it off.

| Checkpoint ledger | Range / fixes | Evidence / disposition |
|---|---|---|
| 78, accumulated audit | `8dc4636d` → entry `0ed4fe5f` → code `82fca940`; every commit across three handoffs, with three ownership repairs | PA18 388/420, exact same 32 failures; earlier 2609/2609; file audit/coverage pass; 922 controls; stage-scoped performance accepted. Code tip above is the next review baseline; full-stage remains unfinished. |
