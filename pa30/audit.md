# PA30 accumulated checkpoint audit198

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `4a081cb05b25638be7a759882f67d4d8ae97eb6a`.

Audit scope is the entire first-review range `27029f9..c0b26910`, plus both
validated audit fixes through the code tip above. The stage base and last-review
markers agreed at entry; history confirms the PA29 boundary. No range was
narrowed to the latest handoff. This audit completes the checkpoint review,
not PA30: the same 21 course failures remain required implementation work.

## Complete accumulated range

| Commit | Reviewed content and disposition |
|---|---|
| `e7fd16e3` | Establishes PA30 stage boundary and authoritative baseline, 105/153. |
| `32727741` | Parser class predeclaration bounds, friend elaborated types, split-angle cursor/name/type probes and diagnostic location. Reviewed all nine changed parser files together. |
| `c67da7c8` | Parser handoff: 114/153, reducers, source/binary bindings, scaling and fixed benchmarks. |
| `b7b1436a` | TypeValue queries use the canonical injected type in construction, including partial/nested current instantiations. Explicit applications retain their actual specialized type. |
| `2f1595b2` | Dependent qualified member queries use the current instantiation's retained source scope, preserving qualifier/access facts. |
| `407fcdc0` | Current-instantiation handoff: 129/153, object/adapter controls and performance evidence. |
| `6bf7812b` | Partial-pattern exact argument comparison uses memoized semantic shapes after deduction/substitution; alias access and failed-substitution obligations remain candidate-local. |
| `94faedf1` | Namespace type/namespace alias convergence. Canonical namespace behavior is correct; the comment and restriction preserving base-typedef ambiguity required the repair below. |
| `c0b26910` | Identity handoff: 132/153, PA6 reference proof, evidence verifier and remaining-owner ledger. The deferred class-base proof was not accepted as resolved. |
| `88c25337` | Audit repair: normalize base typedefs by canonical type identity, with reduced proof, corrected PA30 output sidecars and positive/negative controls. |
| `4a081cb0` | Audit repair: preserve singleton LowIR roles across source presentation; add combined controls and reproducible audit tooling. This is the reviewed code tip. |

The combined diff, not only the individual commits, was reviewed. Historical
source hashes are checked at their original commits (following wrapper symlinks),
not against the newer tree. The initial 197 verifier passed 725 checks before
changes. Final [bindings](../student.tests/pa30/evidence198/source-binding.json)
and [verification](../student.tests/pa30/evidence198/verification.json) bind the
complete range, current implementation, contract surface and retained evidence.

## Findings and repairs

1. **Base typedef identity incorrectly depended on declaration scope.**
   `merge_lookup` excluded class-owned aliases from the namespace correction,
   deliberately retaining a false ambiguity for two public aliases of `int`.
   C++11 normalizes type declarations before merging class lookup sets; the
   two sets here are equal. `88c25337` fixes the shared owner with constant-time
   TypeId comparison, excludes alias templates, rejects missing type facts,
   and retains a declaration for access checking. Positive controls cover
   fundamental/class types, base order, qualification and dependent bases;
   negatives still diagnose different types/entities/templates and private aliases.
   [Reference proof198](reference-correction198.md) supplies the unchanged
   reducer, exact C++11 rules and bundle binding. Compiler agreement is not proof.
2. **Ordinary namespace `main` gained an entry role on LowIR roundtrip.**
   The combined checkpoint program compiled and ran directly but the external
   adapter rejected duplicate singleton roles. Lowering already recorded the
   correct role and ABI entity; its presentation allocator nevertheless used
   the implicit entry spelling `@main` for the first namespace function.
   `4a081cb0` prevents ordinary functions from receiving the adapter's three
   implicit singleton spellings. The existing collision allocator, SymbolId,
   explicit roles and ABI object names still own their respective facts. No
   reader/comparison convention is relaxed. A focused reducer includes both
   namespace functions, a static member and a colliding ordinary display name.
   [Before/after trace](../student.tests/pa30/evidence198/trace.json) reproduces
   the old adapter rejection and verifies direct/adapted execution after repair.

No additional defect remains in the reviewed changes. Ordinary negative lookup
and candidate-substitution failures remain compact results; diagnostics are
rendered on final rejection. No library-name/fixture special case, host compiler
delegation, new global cache, grammar replay, whole-program retry or production
text adapter was introduced. New files are personal controls/evidence, not
implementation translation units; no source-set registration is needed.

## Architecture and ownership trace

The nontrivial `identity::both` declaration and demanded
`construction::pointer<int>::reset` in [interactions.cpp](../student.tests/pa30/source198/interactions.cpp)
exercise all checkpoint groups together. Namespace using/type identity also feeds
partial selection using `decltype(value)`; the program executes the selected
constructor, swap and qualified member operations.

| Spec surface | Concrete ownership and evidence |
|---|---|
| Source / parser (§1) | `Preprocessor` owns immutable TU sources; `PostTokenCursor` feeds the ring `syntax::Cursor`. Interned IDs and source locations enter the retained graph. `predeclare_class` performs category/delimiter lookahead, not another grammar parse. Assignment function bodies no longer escape their class; friend elaborated declarations leave namespace creation to semantic friendship. |
| Split-angle lifetime (§1,8) | The live opening token owns its absolute closing ordinal and split bit. A half-`>>` terminates the inner name/type probe before suffix classification. Consumption releases the token; splitting occurs after its inner opener is consumed and preserves outer cached endpoints. No cache invalidation scan or additional allocation. Token size remains 40 bytes. |
| Canonical facts (§2–4) | Injected types cache by class identity plus immutable source parameter slice, including renamed heads. Current-instantiation lookup visits lexical parents and matches TypeIds. Partial matching substitutes first; signature-shape IDs erase alias wrappers recursively while retaining cv, values, packs and template identity. Source access obligations are checked separately before accepting the candidate. |
| Lookup (§3) | Indexed lexical/using/base edges visit relevant scopes with traversal stamps. Namespace targets and type IDs converge in O(1) per pair; variables, distinct templates and different types do not. Overload/partial candidates and selected conversions remain recorded semantic facts, not rendered names. |
| Demand / caching (§4–5) | `specialize_class` interns `(pattern, arguments)`; the pattern fixes its lexical environment. Substitution frames key specialization, parameter slice/width, parent and supplied arguments. `complete_class` observes Active/Success/Failure states. Retained source regions project compact occurrence/context identities; they share source nodes, defer bodies/defaults and never reparse tokens. `reuse_template_type` substitutes only dependent type facts. Member/body/default queues use entity/fact identities and monotonic cursors, with precise definition dependencies. |
| Direct lowering (§6) | Parser `consume` and Analyzer construct facts against one source graph. `Procedural` consumes selected declarations, construction/call/conversion/layout facts into typed `Program`. The driver passes that program directly to native preparation and ELF writing. Text emission/parsing occurs only in explicit inspection/adapter controls. The role repair makes those views preserve existing facts. |
| Allocation / release (§8) | Source/node pools and compact occurrence arrays belong to the TU; semantic facts use slabs and flat ID indexes. Candidate binding indexes are local; source/query/frame/shape caches die with Analyzer. `build_program` releases each TU frontend after lowering. Typed function bodies support bounded native preparation; per-function MIR/selection state dies after encoding, while the ELF image retains only output/linkage data. |

The combined trace records 1,802 tokens, maximum cursor lookahead 119,
28 completed classes, 81 type-query computations, 17 signature-shape computations
with eight hits, and 27 deferred regions of which 24 are demanded. It emits
381 source LowIR instructions and 342 native instructions. Direct hosted text
is 1,726 bytes; the standalone adapter adds its startup and has 1,746 bytes.
Both execute successfully. Telemetry on/off objects are byte-identical.
These counters corroborate the ownership review; counts alone are not its proof.

## Optimization legality, cost and encoding

[optimization.cpp](../student.tests/pa30/source198/optimization.cpp) carries the
merged dependent `value_type` into `combine<int>::apply`. Its selected `scale`
callee has an explicit `always_inline` attribute and a typed, nonthrowing body;
`offset` stays a real noinline call with volatile effects. A dormant member
using `U::missing` is neither instantiated nor emitted.

The existing `lowir/force_inline` owner admits only supported fixed-arity bodies,
checks active recursion/depth and frame semantics, and reserves work before
mutation. Declines retain valid calls. Immutable-body cost/return-region facts
need no unrelated invalidation. The pipeline limits remain 64 nesting levels,
262,144 reserved work per caller and 4,194,304 per program. Both source objects
and the explicit adapter use the same native preparation once. This is required
attribute expansion, not an optional runtime-profit hypothesis or a new pass.

The final trace performs one expansion, charges 17 actual units against 33
reserved, and grows 59 input instructions to 64 prepared instructions.
The volatile load/store survive. MIR places the value live across `offset` in
preserved `rbx`; `apply` has a 32-byte frame, `main` 48 bytes, with no scratch
area. The O0 loop still loads/stores its local counter and sum; this cost is
reported rather than hidden behind a smaller IR claim. Typed instruction
locations and CFI reach direct encoding. The trace retains LowIR, consumed MIR,
ELF symbols/disassembly and unwind frames, plus checked execution. Standalone
MIR includes the original `scale` body; hosted demand prunes it after expansion
(366 hosted text bytes versus 420 standalone bytes including startup).

No optional transform is added by PA30 or this audit. The existing local native
selection windows require adjacent, single-use, nonvolatile loads; carrier live
ranges extend through their consumers. Unknown effects retain conservative
operations. Selection/allocation uses function-local intervals/call epochs and
encoding consumes that same MIR. The inherited budgets and fallback paths have
not changed. Runtime/text evidence accompanies compiler latency/RSS in
[performance198](performance198.md); PA32/33's later level objectives are not
invented PA30 exit gates.

## Acceptance, references and remaining work

[Validation](../student.tests/pa30/evidence198/validation.json) retains exact
commands, complete outputs, statuses and hashes. The specified prior-through
command passes **4941/4941**. The specified file audit passes with its four
inherited substantial-header warnings. `make test-pa30` returns 2 with
**132/153** and the **same 21 failing fixture identities** as entry;
`make test-report-through-pa30` reports **5073/5094**. Thus stage progress is
preserved without compensating for regressions using new passes. The supplied
154 count is reconciled to the primary log and 153 discovered fixtures.

All test inputs, discovery, comparison scripts and timeouts are unchanged across
the accumulated range. Only the three documented PA6 namespace output sidecars
and two documented PA30 base-alias output sidecars differ. The prior
[namespace proof197](reference-correction197.md) is valid for its namespace case;
its explicitly deferred base interpretation is superseded by proof198. The
reference bundle itself is unchanged. The reference-policy exception is used
with proofs, not a weaker comparator or fewer cases.

The 107 explicitly run control commands include every checkpoint reducer with
the documented base-expectation correction, and 34 trace commands include the
old adapter failure and final execution. The final [verifier](../student.tests/pa30/verify198.py)
passes **1,218 checks** of current and historical source/binary bindings, all
original case identities, allowed reference changes, raw observations and
acceptance outputs. Seven additional checks bind the supplemental hosted A/B
series, its images, inputs, protocol and limits in `hosted-ab-verification.json`.

Remaining work is grouped in [the compact plan](plan.md): nine prerequisite/
callable/constructor failures, eight dependent constant/access failures and four
emitted-code/declaration-contract failures. The three handoffs preserved progress
but separated tightly related parser/semantic/identity questions and deferred a
resolvable base-lookup contradiction. Future handoffs should finish broad owner
groups with their interactions rather than stop after individual fixture gains.

## Audit ledger

| Audit | Reviewed range / code tip | Findings and disposition | Checks / remaining |
|---|---|---|---|
| 198 | `27029f9..c0b26910`, extended through `4a081cb0` | Base typedef identity and LowIR role presentation repaired in `88c25337` / `4a081cb0`; reference proof, cumulative architecture and stage-scoped performance reviewed. | Prior 4941/4941; file audit pass; PA30 132/153 with unchanged 21 failures/153 cases; 107 controls and 34 trace commands. Broad remaining groups stay required; no stage advancement. |
