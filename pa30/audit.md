# PA30 final whole-stage audit204

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `4a1e92b82ad62dce9aad0cf54c3dc0db1982fce8`.
Entry: `547b2678`. Target: **PA30 full-stage**. Disposition: **complete**.

This independently reviews the complete stage, including implementation203's
previously unaudited vector/packed target boundary. The 29 entry stage commits,
the cumulative changes in 84 implementation files, and their surrounding shared
owners were inspected. Checkpoint conclusions located evidence; they did not
replace reconstruction of the production path. The prior goal turn was progress:
committed implementation and test/performance evidence. No inherited job was
live at entry. Earlier audit records remain in history at `d9a62584` and
`404e468e`; the implementation203 handoff remains at `547b2678`.

## Findings and repairs

**Oversized LowIR target descriptors were silently truncated.** The new `x86`
validator inspected only the low 64 bits of an integer literal. For example,
`x86 18446744073709551799, %p, 18446744073709551615` selected descriptor 183
(`pause`) after accepting an identity outside the finite target vocabulary.
Negative wrapping identities, wide predicates and wide no-predicate sentinels
had the same problem. The shared validator now rejects wide/negative IDs and
wide or improperly negative predicates before selection. Ordinary `-1` and
unsigned 64-bit all-ones retain their documented no-predicate meaning.

[Eight minimal inputs](../student.tests/pa30/source204/) establish this boundary:
six malformed cases plus two valid sentinel controls. The frozen entry adapter
accepts all six malformed inputs; final `lowir` and `lowir2native` reject all six.
Both valid controls also execute. [Before evidence](../student.tests/pa30/evidence204/reducers-before.json)
binds the exact inputs and entry adapter. The proof is the operation's finite ID
and predicate contract in [design203](design203.md), not agreement with another
compiler. This fixes the owning external-input validator; production descriptors
are already canonical typed facts, so no frontend workaround or encoding change
is needed. The repair is `4a1e92b8`.

**The records were stale.** The prior plan still requested independent audit,
and audit202 still reported two unimplemented cases. The supplied 5,258-test
summary also disagrees with its primary log. That log, the fresh entry run and
the final run all report **5,094/5,094**, including **153/153 PA30 cases** and
**30/30 stages**. Fixture inventory is unchanged; this is count reconciliation,
not removed coverage. The plan now records the final design and acceptance.

Two inspection-script attempts used the wrong runtime boundary: standalone
LowIR allocation names were sent to a host linker, then a hosted exception
program was sent to standalone executable linking. Their outputs are preserved
in `inspection-adapter-mode.json` and `inspection-standalone-mode.json`.
The final explicit adapter uses hosted LowIR, `compile_object(..., true)`, and
the actual hosted `compile_image` MIR view. No compiler fallback, contract change
or unresolved runtime defect is hidden by those script corrections.

## Independent Spec Alignment

| Requirement | Reconstructed ownership and disposition |
|---|---|
| §1 source, preprocessing, parse once | TU-owned immutable source buffers feed `Preprocessor`, `PostTokenCursor` and the bounded-lifetime ring `syntax::Cursor`. Tokens carry interned IDs/locations. `Parser::translation_unit(&sem)` publishes declarations into the shared graph. `predeclare_class` and angle probes inspect cached delimiter/category boundaries; they do not parse bodies again. Split `>>` state and member-template suffix boundaries live on pending tokens. |
| §2 canonical identity | `semantic::Types`, argument packs, entity/scope IDs and open-addressed `IdIndex` own identity. Lookup merges typedefs by canonical designated type, not display spelling. `FactStore` publishes into 1,024-record slabs. `NodePool` stores source nodes once and compact `(source, context)` occurrences. Fixed template type/conversion facts are reused by `reuse_fixed_expression`; concrete object/lifetime identities stay per occurrence. |
| §3 lookup and candidates | Scope/name indexes and explicit using/base/friend/ADL edges restrict lookup to relevant relationships. `select_call` retains compact candidate/conversion slices; deduction checks template shape/arity, ordinary viability checks arity and member/static category before conversions. All required viable candidates are compared, and the selected entity/conversions are published. Expected substitution failures use compact failure/state records; exceptions at final diagnostics are not candidate control flow. |
| §4 demand and templates | `specialize_class` uses `(canonical pattern, interned argument pack)`; a nested pattern already identifies its enclosing specialization/environment. The selected definition has one parent-linked substitution frame. Declaration, body, layout, default, cleanup and emission states remain separate. `Ast::instantiate` projects retained source regions without grammar replay; dormant bodies/defaults/nested classes stay deferred. Class completion does not instantiate unrelated member bodies. Integer-sequence requests retain the 1,048,576 bound. |
| §5 scheduling and validity | `Analyzer::finish` drains cursor-based, deduplicated queues for functions, storage, friends, vtables, defaults and exception facts. Completed/in-progress/failure states prevent duplicate work. `query_dependencies` records class/query reverse edges; class completion invalidates only published dependent consumers and their values, with query-local revisions. No global cache epoch/retry sweep was introduced. Shuffle signatures cache immutable canonical function types; capture keys are `(closure, object)`. All caches are TU-owned. |
| §6 direct lowering | `toolchain::build_source` → `lowering::build_program` → `Procedural` builds a typed `Program`; `compile_object` passes it directly to native preparation/selection. Selected conversions, aggregate cleanup actions, layouts, ABI and noreturn facts are consumed by identity. Text readers/writers are explicit adapters only. Each declaration/ABI entry uses its emission identity. Production does not revalidate the whole unchanged program at every boundary. |
| §7 optimization/native | The inherited forced-inline pass admits only eligible fixed-boundary callees, rejects recursion/unsupported frames, reserves work before mutation, and preserves a call on refusal. Depth ≤64, caller work ≤262,144 and program work ≤4,194,304 remain unchanged. Required vector lowering expands at most eight ordinary lanes before using a loop; packed recipes expand ≤16 lanes; target records occupy 64 bytes and dynamic comparison fallback has ≤32 alternatives. Selection/allocation/encoding have bounded function-local passes; XMM14/15 and R10/R11 are reserved scratch, and masked stores restore RDI. |
| §8 lifetimes and allocation | TU source/identifier, syntax/occurrence and semantic slabs have one lexical owner in `build_program` and die after lowering. Candidate/substitution/query scratch has narrower owners. LowIR pools retain typed bodies for bounded forced expansion and emission; growth is linear in original IR plus reserved expansion work. `Selector::run` placement/liveness/frame state dies after each function is encoded. `HostElf` moves native byte buffers into sections; it does not retain a second encoded image. No owning hot-node shared pointers or process-global accumulating caches were found. The unused legacy `mir_model.h` is not the production MIR. |
| §9 performance/work | Sparse fallthrough facts move Pending → Constant → Varying at most once per transition; indexed users/edges drive a dirty queue, with one conservative cycle seeding step and no IR growth. Unknown/escaped/volatile/noninteger storage remains unknown. Current and inherited measurements, work bounds, runtime and text costs are reviewed below. PA30 applies O0 and the mandated 45-second hosted compile limit. |
| §10 self-containment | The actual path contains no reference/host compiler or assembler delegation, fixture recognition, cached answers or hosted-only lowering route. Build-time host macro/include probing supplies the permitted environment; `host_config` consumes that immutable configuration. Host linking is used only by the explicit object-execution controls. |

The review also followed all accumulated semantic interactions: current
instantiation queries use their source scope; dependent protected access is
rechecked after substitution; friendship grants do not escape their specialization;
first new[] extents remain distinct from canonical element types and consume the
selected class-to-integral conversion; only selected aggregate children demand
destructors; automatic objects cannot cross an ordinary function boundary;
default lambdas own their context; noreturn normal exits and exception edges are
separate. Their positive/negative controls were rerun together, not assumed from
checkpoint pass counts.

## Representative source-to-ELF traces

[interactions.cpp](../student.tests/pa30/source204/interactions.cpp) combines these
owners with `Pipeline<int>::run`: inherited same-type aliases, a dormant invalid
member, integer-sequence alias expansion, protected dependent bases and friend
access, class-completion deferral, converted allocation, nested captures,
exception cleanup, volatile vector storage, signed packing and scalar SSE.
The scalar `n` from the demanded template becomes a typed lane, saturation to
[-128,127] is implemented by widened scalar comparisons/selects, and `addss`
retains its descriptor and preserved upper lanes through LowIR and hosted MIR
to the encoded SSE instruction. A copied volatile vector retains its value when
the original lane is subsequently overwritten. Both runtime input variants check
all results and destructor effects.

The trace has **935 tokens**, maximum pending cursor **194**, **19 semantic
specializations**, **2 function-template body transitions**, **691 original /
696 prepared LowIR instructions**, **789 native instructions** and **4,146 text
bytes**. `dormant<U>` is not demanded. The aggregate's selected `Guard` cleanup
runs once when `Later` throws; the converted allocation's selected conversion
feeds the same array-new ownership path. The fallthrough proof visits 87
instructions and 43 operand/edge units without rewriting code.

A separate inherited `combine<int>::apply` trace follows the fixed forced-inline
callee fact through eligibility and reservation: one expansion costs **17 actual
units / 33 reserved**, while the noinline call and volatile effects survive.
Hosted MIR exposes the real O0 frame/loop traffic: `main`/`apply` have
48/32-byte frames in the inline trace, and `Pipeline<int>::run` has a
352-byte frame including 48 bytes of floating scratch in the combined trace.
The loop still performs its scalar loads/stores and real noinline call. This is attribute-required
expansion; fewer instructions are not claimed as runtime profit. Analyses used
for admission refer to immutable original bodies; new placement and frame facts
are computed after expansion, so stale liveness is never reused.

[Inspection](../student.tests/pa30/evidence204/inspection.json) records 344 commands,
actual LowIR, hosted MIR, disassembly, symbols, CFI, runtime results and counters.
[ELF comparison](../student.tests/pa30/evidence204/elf-equivalence.json) proves equal
text, named relocations, symbol facts and CFI between direct and reconstructed
hosted objects. Only incidental symbol-table ordering differs for the combined
trace. Telemetry on/off objects match. Both explicit LowIR and production objects
carry the facts needed by the native backend.

## Performance acceptance

[Performance204](performance204.md) records current frozen whole-stage A/B
compilation and checked execution, separate hosted A/B runs, A/A calibration,
six ABBA blocks, all observations/spreads, RSS and text sizes. A whole-stage
comparison uses the original stage-base binary, not merely the last checkpoint.
The heavier hosted comparison uses the correct implementation203 baseline.
No build or correctness suite overlaps timing. CPU affinity is fixed; the host
is shared, and outliers are retained.

All four common workload A/B objects and executables are byte-identical.
Compiler paired medians are near unity; runtime variation cannot demonstrate a
generated-code change for identical images. These fixed programs cover 2,400
demanded templates, loops, calls, memory, floating point, exceptions and emission
pruning. Additional final images bind the vector/packed scaling and heavy-header
evidence from [performance203](performance203.md) to this validator-only repair.
The new malformed-input check is outside ordinary production compilation.

Inherited vector snapshots cost **24N text bytes** and approximately **1–2%**
runtime on their equivalent workload; this is disclosed required value-capture
work, not an optional optimizer or a claimed speedup. Removing it would violate
captured-value and volatile semantics. The measured linear counter relations,
fixed lane/record/inline limits and conservative fallbacks remain intact.
Newly accepted workloads have final-only cost evidence when the earlier binary
rejects them; rejection timings are never used as performance baselines.

Historical blanket 15% latency and zero-growth targets remain diagnostics under
spec §9. Their observations are preserved; they do not override stage-scoped
acceptance. The 45-second limit, correctness, comparison rules and coverage stay
mandatory. PA31 hosted-library runtime closure, PA32/33 optimizer-level goals and
PA34 inception remain their own stages. Those boundaries do not defer a known
PA30 defect or excuse avoidable regressions.

## References and final validation

No reference, fixture, harness, discovery rule or timeout changes in audit204.
The complete stage has exactly three documented reference corrections:
[namespace typedefs](reference-correction197.md),
[base typedefs](reference-correction198.md), and
[replacement new](reference-correction201.md). The reduced sources, C++11
[dcl.typedef], [namespace.udir], [class.member.lookup] and [except.spec] proofs,
and pinned bundle revision/hash were independently checked. The 197 provisional
base-lookup rejection is explicitly superseded by proof198. Only the six recorded
output/status sidecars differ from stage base; all test sources and comparisons
are preserved. Compiler agreement is not their proof.

[Final validation](../student.tests/pa30/evidence204/validation.json):

- `perl scripts/cppgm_file_audit.pl --stage pa30 --paths dev/src`: **pass**, with
  four inherited substantial-header organization warnings and no audit errors.
- `make test-pa30`: **153/153**, exit 0.
- `make test-report-through-pa30`: **5,094/5,094; 30/30 stages**, exit 0.
- **439** accumulated personal control commands pass on the final compiler.
- All **286** hardware differential cases passed at entry with two inputs each;
  freshly rebuilt final objects are byte-identical, proving reuse of those
  executions after the validator repair. All six new malformed IR cases reject
  through both external consumers; both valid sentinel controls execute.
- Final source/binary/fixture bindings, performance ordering/recomputed summaries,
  ELF equivalence, check statuses and review markers are mechanically verified
  by [verify204.py](../student.tests/pa30/verify204.py): **2,662 checks pass**.

## Consolidated ledger

| Boundary / code tip | Whole-stage disposition |
|---|---|
| 195 / `32727741` | Friend/elaborated declarations, split angles and class member boundaries; fixed parser ownership/scaling evidence retained. |
| 196 / `2f1595b2` | Current-instantiation identity and qualifier source scope; dependent queries and body demand reviewed. |
| 197 / `94faedf1` | Semantic alias shape and namespace identity; namespace reference proof reviewed. |
| Audit198 / `4a081cb0` | Base alias identity and singleton LowIR role presentation; corrected reference proof and full-range performance retained. |
| 199 / `d6408429` | Runtime array extents, dependent access/friendship, fixed vector values and zero/volatile storage. |
| 200 / `37b6729d` | Selected aggregate-child destruction demand and enclosing template body scheduling; concrete integer-sequence alias identity. |
| 201 / `fc8ea8d6` | Capture boundaries, default contexts, typed noreturn/exception CFG; replacement-new proof. |
| Audit202 / `378d1bd8` | Contextual new[] bound conversion and sparse constant joins repaired; historical open SIMD/vector group now closed by 203. |
| 203 / `247c383a` | Previously unaudited packed/vector completion and serialized target record reviewed end to end; inherited code/runtime costs accepted with their stated scope. |
| Final204 / `4a1e92b8` | External descriptor-width defect repaired; entire stage reconstructed, accumulated controls rerun, final architecture/performance/reference/exit evidence consolidated. |

No unaudited handoff or known required PA30 implementation work remains. The
reviewed code tip is fixed above; the following record commit changes only audit
scripts, plan and evidence. Advancement beyond PA30 was not performed here.
