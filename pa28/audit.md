# PA28 independent final audit154

Target: **PA28 full-stage**. Audited stage base: `bec9389f62014dfecac8b41d330de142e0904b8c`.
Audit entry: `03575afb57a5f9cdfde1857349f32501e493c1bf`.
Final implementation reviewed: `abfa68e7` (including correction `a3714525`).
The handoffs151–153 were implementation records, not checkpoint approvals.
All of their implementation owners and the cumulative source-to-object path
were independently inspected in this audit. No advancement to PA29 occurred.

## Findings and disposition

**Fixed:** virtual override checking compared only throwing/nonthrowing status.
It accepted `B::f() throw(int)` overridden by `D::f() throw(double)` or an
unrestricted `D::f()`. The correction checks the allowed types using the C++11
handler-matching rules, including pointer qualification and public unambiguous
base matching. It handles concrete/dependent members, all overridden bases,
and implicit destructor type sets. It does not instantiate bodies to decide
declaration compatibility. [Reducer, standard proof, ownership and controls](../student.tests/pa28/exception-override154.md).

**Consolidated:** the new semantic check initially repeated an identical handler
compatibility query across 400 specializations. `abfa68e7` moves completed
positive/negative answers to the analyzer's flat `(exception TypeId, handler
TypeId)` cache. All relevant class base edges are fixed before these queries;
lexical access privileges never affect handler matching. Work/hit telemetry
observes existing work. Scratch traversal and per-class callable-pair indexes
retain their shorter lifetimes. Earlier measurements remain preserved.

**Evidence correction:** the supplied state summary said 4701/4701. Both the
supplied primary log and fresh root reports actually say **4538/4538**, across
**28/28** stages; PA28 has **97/97** anchors. This audit records the command's
actual count. No fixture or test-discovery change was made to obtain it.

**Performance classification:** inherited blanket 15% latency/RSS and zero-growth
criteria are diagnostic targets, not PA28 requirements. Spec §9's stage-scoped
acceptance governs them. Necessary ABI/layout/EH costs remain measured; mandatory
correctness, coverage, timeouts, representation and work limits are unchanged.
No optional optimization or unprofitable transform was introduced by PA28.

## Final Spec Alignment: actual architecture

| Spec surface | Independently inspected owners and conclusions |
|---|---|
| §1 source and grammar | `preprocess/source.h`, token and post-token cursors, `syntax/cursor` and `parser::translation_unit`, then `lowering/driver`. Source buffers are immutable and TU-owned. Tokens borrow source spellings or intern identifiers. Ring lookahead feeds a parser which calls the semantic consumer for each declaration. Source ambiguity is resolved before publication; no successive owning token/AST transports. |
| §§1–2 source-faithful semantic graph | `syntax/occurrence.cpp`, `semantic/fact_store.h`, expression/type fact owners. Source nodes are shared; `(context, source)` occurrence identities project source topology. Slab-backed facts and flat indexes publish semantic records only when needed. Template projection cannot call the grammar parser or rewrite published source nodes. |
| §§2–3 identity and lookup | `semantic/model.cpp` interns typed shapes/parameter slices; `lookup`, `overload`, `query_call` use scope/name/kind, callable shape, canonical argument and selected-declaration IDs. Base/using/ADL edges restrict traversal. Filters precede expensive deduction; required candidates remain considered. Expected query failure is a typed result. Mangled/rendered strings are output only. |
| §§4–5 demand and caches | `template_class`, `template_call`, `template_instantiation`, `template_type_facts`, `template_definition_environment`, `query_dependencies`, `declaration::finish`. Parsed bodies/defaults are retained. Complete keys include pattern/argument identities and immutable parent-linked substitution frames. Nondependent facts are reused. Declaration, body, layout, defaults, exception, support-object and emission demands are separate. Monotonic queues and precise reverse completion edges replace global retry/flush. |
| PA28 naming/effects | `syntax/attributes`, `semantic/native_attributes`, `lowering/local_abi`, `symbols`, Itanium graph/encoder. Tags use source/TU pools and sparse entity heads; effects occupy existing entity padding. Canonical pattern edges propagate late effects. Typed local/unnamed/lambda contexts and template parameter identities reach ABI encodings; no string-based semantic reconstruction. |
| PA28 layout and virtual dispatch | `virtual_primary`, `virtual_storage`, `virtual_subobjects`, `virtuals`, `layout`, `lifecycle_layout`. Primary choice, nearly-empty/storage claims, subobject identity, vcall/vbase rows, final overriders, covariant result projection and physical view/VTT order have explicit owners. Virtual paths deduplicate by identity; fixed tails memoize; physical order is O(v log v). Layout completion precedes exposing covariant slots to callers. |
| PA28 support ownership and RTTI | `semantic/rtti*`, `construction`, `demand_vtable`, then `lowering/virtuals`, `construction_tables`, RTTI and lifecycle entries. A key declaration leaves vtable/VTT external; only actual definition demand establishes construction dependencies. Dynamic local statics require vptr work without prematurely draining synthetic bodies. RTTI base/access/virtual-offset facts share the same finalized layout as casts and tables. |
| PA28 exceptions | `exception_specification`, `dynamic_exceptions`, `exception_override`, `destructor_exception`, source/cleanup/full-expression lowering, native clause/region/table owners. Allowed types are canonical adjusted slices. Persistent lexical/live prefixes retain cleanup and catch exit state. Typed filters reach signed selectors, LSDA action/filter tables, type relocations and the host unexpected boundary. No name-based filter recovery or textual EH transport. |
| §§6–8 typed lowering and native emission | `lowering/driver`, `Procedural`, `toolchain/driver::source`, `compile_object`, `native/driver::compile_image`, `HostElf`. Typed Program is built directly. Its required symbol, support-object and function pools survive into object demand/selection; source/parser/analyzer storage dies after TU lowering. Per-function MIR/placement/EH vectors die after encoding. Native byte lanes move into ELF sections; sections stream once. No assembly or external compiler implements output. |
| §§8–10 ownership/telemetry/self-containment | Slabs/vectors and flat ID tables own hot facts; no per-node shared ownership, ordered hot maps, process-global accumulating caches or duplicated textual IR in production were found. Program storage dies when source compilation returns its object; object buffers die after writing/linking. Stats report existing counts/times, with text inspection explicitly opt-in. Host tools occur in harness checks and authorized final links. |

The two ABI policies are explicit: course LowIR/private-runtime presentation
keeps its established contract; `-c` selects host layout/runtime once. The
presentation-specific EH registration convention in `adapter152.md` was
reviewed against the production path and preserved references. Inspection does
not silently substitute the private ABI for host behavior.

The layout review checked primary selection, inherited prefix positions,
VTT ordering and construction views against the
[Itanium ABI §§2.4–2.6](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#layout).
Its §3.2.5 permits alternative thunk adjustment mechanisms when the required
vcall offsets are present; fixed entry adjustments therefore do not by
themselves imply a host-dispatch defect. Host EH review used the
[exception ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi-eh.html), alongside
checked-in N3485 [except.spec], [except.handle], [except.throw] and
[class.base.init]. No reference correction was needed.

## Representative end-to-end traces

The existing covariance input was traced from `ReturnDerived::self` and
`VirtualDerived::self` through class completion, physical layout, typed table
rows and `BaseAdjustment`, direct LowIR thunk construction, MIR selection,
encoding and relocatable ELF. Its fixed result adjustment is **32**; its virtual
result uses the recorded **-32** row, guarded for null pointer returns. That
same layout feeds vptr stores, RTTI offsets and construction/VTT entries.
The audited object has the required `_ZTch0_h32_...` and `_ZTch0_v0_n32_...`
identities. [Current inspection](../student.tests/pa28/evidence154/inspection.json)
validates the typed IR, verifies MIR image/ordinary-object byte equality,
inspects unwind/relocations and executes the host-linked object. It observes
33 functions, 607 MIR instructions, 1056 aggregate frame bytes and 3540 code bytes.

The new [trace source](../student.tests/pa28/trace154.cpp) combines an attributed
`Root154` declaration, shared virtual primary, demanded `Value154<7>` class and
`sample154<Root154>` function, virtual covariance, RTTI and an unused invalid
dependent member body. `Root154`'s tag enters its canonical entity and ABI graph;
`Value154<7>` uses a canonical template/argument key and retained source region.
Its fixed `int` field facts are reused while dependent members are projected.
The unused member remains undemanded and absent from the object. The function's
`pure` attribute reaches the published `effects=readonly` call boundary.

`Value154<7>::self` records a virtual result row of **-40** (this hierarchy adds
`read` to the prefix). Its generated thunk tests the returned pointer, then
loads `[vptr-40]` and applies that dynamic displacement. `read`'s finalized field
offset **8** becomes `[rdi+8]` in MIR and native encoding; constant template
argument **7** becomes an integer immediate. The emitted ABI names retain
`B5audit` and the dependent function signature. [Trace evidence](../student.tests/pa28/evidence154/template-trace.json)
validates typed LowIR, compares ordinary/view objects, inspects symbols,
relocations and unwind, and executes both nonnull and null covariant results
plus a dynamic cast. It observes 19 functions, 367 MIR instructions, 496
aggregate frame bytes and 1993 code bytes.

Actual O0 costs remain visible: `run154` has a 16-byte frame and stores/reloads
its root and joined return value; the covariant thunk has a 16-byte frame with
result homes. `sample154` and `read` have zero stack allocation, retaining a
frame pointer. This audit does not interpret fewer IR nodes as runtime profit.

## Optimization legality, profitability, invalidation and budgets

| Existing stage policy | Proof, limit and fallback reviewed |
|---|---|
| Semantic/ABI publication | One completed fact per full key; required graph/slot/row work follows actual entities and ABI output. Local queries do not drain broader body work. New handler-match cache is TU-owned with immutable type/base inputs; class-local callable pairs deduplicate repeated virtual slots. Structural exception-set comparisons are required language work over declared sets, with exact typed lookup first. |
| Constant evaluation | Existing one-million-step and depth-512 limits remain. An exhausted proof does not manufacture a constant. No PA28 change widens these limits or bypasses effects/lifetimes. |
| Address/load selection | `parameter_slots::folds` proves a literal address displacement or a single adjacent nonvolatile integer use of an allowed opcode/width. It extends the carrier's last use/call/block facts before allocation. The field-at-8 trace reaches actual encoded addressing. Unknown/disallowed uses retain ordinary materialization/load; no FP reassociation or memory-effect motion. |
| Private reload carrying | `carry.cpp` makes two linear interval scans and tries at most three carriers over at most 64 instructions. Private single-store 64-bit homes, same-block uses, explicit effects and hidden-clobber exclusions establish legality. It replaces executable reload work without growth; failure keeps valid memory operations. No fixed-point full rescans. |
| Allocation/flow/encoding | Fixed register pools and interval/last-use facts account for call clobbers and preserved registers. Monotonic six-bit incoming-parameter flow changes at most six times per edge. Phi edge ordering is O(e log e); encoding is linear in emitted bytes. Frame limit `0x70000000`, global alignment 4096 and representable branch/ELF offsets remain. |
| Pipeline-wide growth/lifetime | No PA28 inlining, specialization cloning, versioning or unrolling pass. Required ABI thunks/tables/filter lists are bounded by typed demand and output. Native transient storage releases per function. Bounded selector windows/flow plus linear lowering/encoding and actual graph work give additive budgets, not nested whole-program fixed points. |

O0 is the measured PA28 policy. O1–O3 optimization acceptance belongs to PA32/33;
PA34 owns self-hosting. Hosted source/header work belongs to PA29–31. Those
future obligations remain recorded; they are not PA28 exit gates or excuses for
an existing PA28 correctness/work defect. Existing selected instruction debug
identities survive folds; the stage adds no debug rewrite, and production MIR
inspection reflects the block identities actually encoded.

## Performance and validation

[Final performance review](final-audit-performance.md) covers compiler latency,
peak RSS, checked executable runtime and text bytes together. Frozen A/B flags,
inputs and hashes, four A/A observations and six ABBA blocks per workload/mode
are retained. Both original and cache-refined observations remain; no outlier
was discarded. Earlier evidence151/152/153 remains unchanged, including
incorrect-baseline outcomes and standalone costs for newly supported behavior.

[Final validation manifest](../student.tests/pa28/evidence154/validation.json)
records exact commands, statuses, hashes and required-output tails. It includes
`make test-pa28` (**97/97**), `make test-report-through-pa28` (**4538/4538**,
28/28 stages), file audit, 33 new declaration controls, naming/effect controls,
exception cleanup/filter controls, imported ownership/CRTP controls,
bidirectional host virtual-primary controls and both native traces. The file
audit passes with the four inherited substantial-header warnings; no audit
failure or timeout is present. These warnings do not hide missing phase owners.

All **20,288** tracked PA1–28 contract paths, **97** PA28 anchors, fixture/status
files, harnesses, comparison rules and the pinned reference bundle are unchanged
from the stage base. Bundle revision:
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`. No reduced-oracle exception was used.
Implementation source registration and `git diff --check` pass. Final artifact
commit/status verification is performed after this record is written.

## Complete handoff ledger

| Commits since last reviewed stage base | Final audit disposition |
|---|---|
| `b7743532`, `2a87fba0`, `ef7f93de`, `e58612a4`, `bed2be54` | Naming/attributes/decay, sparse tags and packed effect byte; source, canonical publication, ABI encodings, controls and evidence151 reviewed. |
| `100de24b`, `ed85fc94`, `017675aa` | EH filter/lifetime ownership, imported VTT and dynamic-local-static demand; semantic to native/ELF ownership and evidence152 reviewed. |
| `810ef2d8`, `5f8483c7`, `fefbf8a7` | Course presentation/reference preservation, actual MIR block identities and 96/97 handoff reviewed; original reference files retained. |
| `3f7bae40`, `a5c53a88`, `764721cd`, `03575afb` | Primary storage/prefix/VTT group plus secondary vptr-store correction and 97/97 evidence153 reviewed end to end. |
| `a3714525`, `abfa68e7` | New dynamic-override correctness fix and canonical handler-match cache; standard proof, 33 controls, workload and telemetry reviewed. |
| This consolidation | Final plan, independent architecture/trace review, performance measurements/acceptance and validation ledger. |

No PA28 implementation handoff remains unaudited, and no known unresolved
correctness, self-containment, timeout, architecture or required-check defect
remains in the reviewed stage. Historical intermediate failures and measurements
are retained as history, not silently recategorized as current passes.
