# PA29 checkpoint audit158

Target: **PA29 full-stage**; audit complete, implementation unfinished.
Stage base / previous review: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Audit entry: `764305061a4d88a8bb9088216ac31c2ab8395a08`.
Last reviewed commit: `1ab3499d7046daf5c298d958a8770b413edb3615`.
This first PA29 audit covers the whole stage boundary, all three accepted
handoffs and their interactions. It does not narrow the range to handoff157.
The records commit follows the reviewed code tip and changes no implementation.

## Findings and fixes

**Lost alias storage facts.** `using I __attribute__((aligned(1))) = int`
discarded its alignment operand. Alias-template substitution also consumed the
erased entity signature. The fix parses the operand once, binds its dependence
and substitutes the retained source type. The parser, alias and template owners
now use the same alignment representation as typedef declarations.

**Lost expression storage facts.** A decorated pointee's alignment survived on
its declaration but vanished through `*&x`, indexing, pointer arithmetic, casts
and function results. Expression semantic equality must still use canonical
language types; storing the decoration there would break type identity and ABI
matching. The fix adds an optional TypeId in shared expression properties and a
sparse declaration index. `storage_types.cpp` propagates those facts through
builtin operations and selected/indirect calls. Function-template specialization
substitutes its raw result separately and verifies that callable identity is
unchanged. Raw Name/Parameter inputs participate in immutable query keys, and
query/cast results expose canonical expression types plus their storage fact.
No source spelling, whole-declaration scan or lowering-time lookup repairs a
missing fact. [Entry regressions](../student.tests/pa29/evidence158/entry-regressions.json)
and [34 final controls](../student.tests/pa29/evidence158/controls.json) cover
concrete/dependent aliases, identity, fields, parameters, arrays, casts, return
paths, execution and LowIR roundtrips. Both declaration and query paths were
reviewed together; fixing only the direct `alignof(variable)` path was insufficient.

**Incorrect deleted-copy reference.** The original negative trivial/POD assertions
contradict C++11's definition of user-provided and trivial special members. Only
its expected exit status changes to rejection. The original source, all three
assertions, comparison rules and discovery stay intact. The [reducer and clause
proof](reference-correction158.md) cite N3337 and the pinned bundle revision;
positive controls separately exercise every property. This explains the one
extra course pass, independently of the ownership fixes. The forward-declared
`std::is_nothrow_*` shorthand oracle remains unchanged: the ordinary-namespace
undefined-template reducer alone cannot prove the original's behavior under
[namespace.std]. It remains recorded as unresolved, not replaced by recognition
of a library spelling.

**File ownership and performance classification.** The first local extraction
exceeded the file audit's function-size limit; moving storage propagation into
its registered source owner resolved it. Final file audit passes without a new
waiver. The inherited blanket 15% and zero-growth targets are diagnostics under
spec §9; all measurements and mandatory constraints remain. The README's vector
runtime exclusion also governs inherited plans. This does not waive the required
vector width/layout representation or any of the 86 failing fixtures.

## Every commit reviewed

[Range manifest](../student.tests/pa29/evidence158/range.json) retains full hashes,
per-commit implementation paths/diff hashes and the final combined diff hash.
The review included changes subsequently amended by another handoff.

| Commit | Reviewed content and interaction |
|---|---|
| `66bb1c6b` | Stage baseline, spec/owner plan and unchanged review boundary. |
| `4a428561` | Hosted import/probes, numeric parsing, builtin registry and canonical traits; driver-to-frontend configuration. |
| `f8f8f342` | Ordinary trait identifiers and definition access; constructor/destructor/transfer query context and first increment corrections. |
| `6bf1369b` | Exception/RTTI metadata across driver, preprocessing and feature probes; consistency with actually enabled modes. |
| `6afc84f7` | Handoff155 controls, benchmarks and residual owner ledger; no stage completion implied. |
| `5716fcfd` | Scalar/runtime ownership plan and stage-scoped performance interpretation. |
| `3611c078` | Typed libm/memory/integer/floating builtins, selected signatures, constant evaluation and lowering effects. |
| `81a9f67f` | Overflow/hints and exact NaN storage; semantic/lowering agreement and externally serializable LowIR. |
| `0bad8c20` | Handoff156 validation and performance, including unfinished structured intrinsic groups. |
| `076139b6` | Attribute/layout ownership plan, required representations and controls. |
| `a33d1086` | Attributes, aligned storage, empty overlap and offsetof across parsing, dependence, layout, initialization, transfer and ABI; gaps fixed above. |
| `76430506` | Handoff157 evidence and 316/403 result; all residual groups carried forward. |
| `1ab3499d` | Audit ownership fixes, reducers, explicit harnesses and proved reference correction; final code reviewed and validated. |

## Architecture and end-to-end ownership

| Spec surface | Trace, owner, lifetime and conclusion |
|---|---|
| §1 source/grammar | `lowering/driver.cpp` creates TU-owned Preprocessor, PostTokenCursor, syntax Cursor, Parser and Analyzer. Immutable buffers/file offsets and interned identifiers feed bounded streaming lookahead. Integrated `translation_unit(&sem)` publishes one source-faithful graph; attribute operands and retained template bodies are parsed once. Hosted `-E` writing is an explicit output adapter, never production transport. |
| §§2–3 canonical facts/lookup | Type/entity/scope/query IDs and flat indexes remain primary keys. Builtin registry entries select typed signatures; ordinary names still use lexical lookup. Type queries carry access/definition context and structured failure. Raw storage decoration has a separate owner from canonical signatures; no mangled/rendered string becomes a semantic key. Existing overload filters retain required candidates and selected conversions. |
| §§4–5 templates/demand | `template_call`, `template_entities`, `template_instantiation`, type/query and dependency owners key specialization by pattern, canonical arguments and immutable substitution frame/context. Signature, body, layout, defaults, exceptions and emission states remain distinct. Source occurrences project shared parsed regions; fixed facts are reused, dependent facts substituted. Raw alias/return types enter the existing complete type/frame keys. Recursive demand sees active states; completion edges wake actual consumers, with no new global retry/flush. |
| §§6–7 semantic → native | `Procedural` directly builds typed Program. Selected builtin kind/signature, layout offsets, overlap permission, alignment and lifetime actions are consumed without re-resolving source names. `compile_object` calls `native::compile_image`, per-function Selector/encoder and HostElf; no assembly roundtrip or host compiler implements output. Full LowIR validation is used for external inputs and requested audit views. |
| §8 allocation/release | TU slabs/pools and flat indexes own graph, query, expression and sparse declaration facts. Shared expression properties gain one TypeId, not per-node heap ownership. Layout placement indexes and substitution scratch have local lifetimes. TU frontend state dies after lowering each input; Program retains typed functions/symbols needed by demand/emission. Per-function MIR and selection temporaries die immediately after encoding. Program/object buffers then release at their driver boundaries. No accumulating process-global cache was introduced. |
| §§9–10 evidence/self-containment | Phase/work telemetry reads existing counters. Object and LowIR bytes are identical with stats enabled. Reference tools are observations only; hosted macro/include discovery and final linking use the handout's authorized host boundary. No fixture recognition, cached answers or compiler delegation was added. |

The declaration trace is `ByteInt a[4]` in
[storage.cpp](../student.tests/pa29/controls158/storage.cpp). Its parsed alias
operand becomes a decorated TypeId and storage fact; semantic identity remains
`int`. Field/global metadata yields typed LowIR `layout=16x1`; native data and
ELF consume that layout. The pointer initializer records a relocation to `a`.
`a[1]` is offset 4, independently of storage alignment 1. It executes correctly
through both ordinary host object emission and the explicit LowIR reader/native
adapter. The test also verifies canonical identity rather than treating an
alignment decoration as a new C++ type.

The demanded template trace is `Holder<16>` plus `alignment<char>()` in the
same source. The member alias `Row` retains its dependent alignment operand;
substitution uses the pattern/argument/frame key, completes one class layout
and records row offset 16, size/alignment 32/16. Its initializer and loads consume
those facts. The function body shares its parsed source and fixed pointer fact,
substitutes the type query and emits the selected ABI identity
`_Z9alignmentIcEmv` once. The native code returns the two alignment contributions,
and the call relocation retains that specialization name. Source facts are gone
before object selection; no lowering lookup of `Row` or parsing of its name occurs.
[Native evidence](../student.tests/pa29/evidence158/native-inspection.json) and
[roundtrip controls](../student.tests/pa29/evidence158/controls.json) retain hashes
and actual encoded accesses. The scaling workload extends the same demand path
to 600/1200/2400 class/function specializations with matching work counters.

## Optimization legality, profitability and budgets

No optional transform is introduced in the accumulated PA29 range. The new
operations are required language/builtin lowering; their semantic cost is not
presented as an optimization profit. Existing native selection remains relevant.

| Path | Legality, work/growth, invalidation and fallback |
|---|---|
| Integer/overflow builtins | Width/signedness and selected builtin kind are established semantically; each argument evaluates once. Bit reductions have logarithmic work in supported width (at most 128 bits); wide overflow uses fixed-size widened arithmetic/lanes. Result truncation and the overflow predicate are distinct. Expansion is bounded per operation, with no speculative loop/clone transform. |
| Runtime/floating builtins | Registry signatures retain exponent/pointer/integer-return distinctions and all argument effects. Libm operations lower to their actual C ABI functions. Exact NaN bits use ordinary integer storage/load, visible to the LowIR writer/reader; no hidden native-only payload or FP reassociation. Intrinsic nonthrowing facts do not erase throwing argument evaluation. |
| Hints | Required evaluation is retained; unsupported optimization promises do not invent stronger alias/alignment facts. No hinted inlining/versioning growth or claimed runtime gain is added. |
| Empty-member layout/zeroing | Exact `(type, offset)` summaries cap at 64 positions; larger shapes use address-directed graph queries and arithmetic array indexing. Placement respects same-type address constraints. Recorded overlap permission suppresses only the invalid overlapping empty zero store; required constructor/destructor effects remain. Layout facts are immutable after completion. Missing overlap proof keeps ordinary storage behavior. |
| Address folding/native carrying | Semantic field offset 16 becomes an address displacement, then the existing selector checks literal/single adjacent nonvolatile integer uses and updates carrier last-use/call/block facts. Disallowed uses retain materialization. Private reload carrying remains two linear scans, at most three carriers over 64 instructions, with single-store/same-block/effect/clobber checks and zero growth. No global invalidation or fixed-point rescan. |
| Pipeline budget/ABI/debug | New optional work/growth is zero. Required output grows with actual demand and per-op expansion; layout follows relevant subobject paths, not expanded array bounds. Native fixed register/interval/parameter-flow and linear encoding policies remain. Source/block identities and ABI/EH records survive selection; no new debug rewrite is introduced. Existing constexpr one-million-step/depth-512, native frame `0x70000000`, alignment 4096 and representable branch/ELF limits remain. These implementation limits are preserved, not new timing gates. |

Actual executable cost is inspected, not inferred from IR count. In the alias
workload `work<7>` has a 16-byte frame with stores/reloads at `rbp-16` and
`rbp-12`, then adds the recorded member offset 4. In the trace main, a 64-byte
frame includes three boolean merge homes. O0 has not eliminated those costs.
Common memory/call, floating, exception and pruning executables are byte-identical
between stage base, audit entry and final code. [Performance158](performance158.md)
reports compiler latency/RSS and executable runtime/text together, all A/A and
ABBA observations, scaling and launcher calibration. The storage-fact correction
costs about 1–3% in paired compiler medians; no runtime benefit is claimed.

## Validation and residual work

[Validation manifest](../student.tests/pa29/evidence158/validation.json):
`make test-pa29` is **317/403**, exit 2; PA1–28 are **4538/4538**, exit 0;
file audit exits 0 with the same four inherited header-body warnings.
The exact failure-set comparison has **zero new failures** and one resolved
reference mismatch. All **403** PA29 source fixtures are byte-identical to the
stage base; all reference outputs except the proved one-line status correction
and the bundle manifest are unchanged. Harnesses/comparison/discovery are intact.
New controls pass **34/34**; inherited controls pass **77/77**, **40/40**,
**53/53**; inspection passes **10/10**, including byte equality and compact
million-element layout work. Final build and `git diff --check` pass.

The [compact plan](plan.md) and [fixture ledger](../student.tests/pa29/evidence158/remaining.json)
retain five broad groups: atomics/assembly **21**, extended syntax/types/layout
**37**, legacy traits/lifetimes **5**, template demand/hosted ABI **19**,
structured intrinsic contexts **4**. Numeric suffix recognition still lacks true
extended precision; code alignment, dependent offsetof ABI signatures and
class-convertible indices also require their proper owners. These are unfinished
behavior, not accepted implementations or permission to recognize fixtures.
Full PA29 acceptance still requires its complete root through report.

Handoff fragmentation was partly avoidable. The three broad implementation
surfaces were useful, but ordinary-name/access and feature-publication followups,
then scalar/NaN followups and repeated evidence-only checkpoints increased the
review burden. The storage defect crossed declaration, template and expression
owners because those paths were not closed together. Future handoffs should
cover a coherent owner group from source to lowering and validation, retaining
one compact residual ledger rather than splitting each small repair.

## Audit ledger

| Checkpoint | Reviewed range | Findings / disposition | Evidence / result |
|---|---|---|---|
| 158 | `2734e5c6..1ab3499d` (entry `76430506`, three handoffs) | Fixed alias/expression/query/template storage ownership; proved deleted-copy status correction; inherited performance gates classified under spec §9; no optional optimizer added. | PA29 317/403, no new failures, 403 sources retained; PA1–28 4538/4538; file audit and explicit controls pass; all four performance dimensions recorded. Checkpoint audit complete; 86 implementation failures remain. |
