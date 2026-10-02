# PA32 final whole-stage audit 218

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: dcd298afff36573014188a2c3cee826f2500cbf8

**PA32 full-stage is complete.** The required file audit and through-stage report
pass; all three previously open source-debug failures are fixed. The review
independently reconstructed the current source→semantic→LowIR→MIR→ELF pipeline
and its optimization policies. Earlier checkpoint conclusions were checked
against the implementation, current traces and measurements, not used as a
substitute for that review. The [plan](plan.md) is the compact final design,
operative budget table and handoff ledger. [Evidence 218](../student.tests/pa32/evidence218/README.md)
binds the reviewed code and complete results.

Entry was clean at `5a767f20`. Both entry review markers named `40151904`;
there was no boundary recovery or narrowing. No prior live build/test process
needed resuming. The cumulative range since that checkpoint includes:

| Handoff | Reviewed commits and ownership |
| --- | --- |
| 214 records | `f4f075b8`: previous findings, policies, measurements and open obligations |
| 215 | `0f370e3f`, `7dd38d40`, `3592a12f`, `a2f067df`, `527b84ca`, `1aa04ae6`, `104846c6`, `33561837`: independent slot demands, diamonds, finite pointer ranges, guarded fills, complete runtime-role cache identity and retained experiments |
| 216 | `94eb1dae`, `2691df5a`, `2bace8a0`, `7e04081b`: exact floating facts, contextual inline/no-unwind proofs, private stores/EH joins, readonly representation and bounds evidence |
| 217 | `cc7d3c20`, `940d3396`, `c81197b0`, `5a767f20`: source display identities, complete-object/lifecycle facts, ABI aliases, cold actions, member C-linkage correction and proved even-stride oracle correction |
| 218 correction | `dcd298af`: source-value/debug ownership through lowering and optimization, stable phi input publication and final slot retirement |

The combined changed sources and their callers/consumers were read, including
all 35 implementation paths changed between the previous checkpoint and entry.
The older scalar, aggregate, loop and memory implementations were also reviewed
in final composition. The binding covers the whole current implementation.
No new C++ translation unit was introduced by this audit; inherited additions
are registered in `dev/frontend_source_sets.mk` for their shared consumers.

## Spec Alignment: architecture reconstructed from source

`lowering/driver.cpp` constructs immutable preprocessing buffers, interned token
identities and streaming cursors. The syntax cursor retains bounded lookahead
and deferred language regions; it does not copy complete token streams.
Parser and semantic analyzer cooperate on one slab-backed Ast/fact graph.
Tokens use file/offset and interned identifier identities; node relationships
are compact IDs. Parser checkpoints retain neither owning token strings nor
abandoned complete trees. The source-view tools render this shared substrate.

Semantic scope/name indexes, overload candidates and explicit using/ADL/base
edges constrain lookup to relevant declarations. Rejection and fact states are
structured; rendering is outside hot semantic equality. Template specialization
in `semantic/template_call.cpp` is keyed by canonical pattern and interned
argument pack; the canonical selected pattern includes the parent context.
Explicit-pack lookup has its own index. Immutable parent-linked substitution
frames retain bindings without copying all enclosing scopes. Separate demand
states distinguish unstarted, active, success and expected failure for bodies,
layouts, defaults and other facts. Incomplete queries carry precise dependency
revisions and reverse consumers. Local completion does not cold-clear unrelated
caches or restart all pending work.

Instantiation demands already parsed regions in `template_instantiation.cpp`.
Dependent occurrence overlays establish specialization facts while sharing
nondependent topology and fixed-expression results. Class completion does not
instantiate unrelated bodies. Declaration/source order and emission order are
separate from canonical demand identity. The reviewed paths introduce no text
semantic keys, parser replay, global mutable cache or per-node shared ownership.
Frontend nodes/facts use TU arenas/slabs; local candidate, substitution and
lowering scratch have narrower owners. Their release points are explicit.

`lowering/symbols.cpp` and function-body emission consume the selected entity,
recorded conversions, completed layouts, lifetime actions and distinct ABI
entry IDs. Missing required facts are invariants, without textual recovery.
Source display is one dense declaration/scope census; strings only render
identities. Most-derived class and offset facts travel on known nonreference
object values. Unknown references/loaded pointers remain dynamic. Destructor
effect facts determine unobservable intermediate vptr stores while required
complete/base bodies and actual virtual destruction remain. C-linkage contexts
cannot change class-member linkage; the owner-kind rule follows N3485 7.5/4,
not a mangling repair. Weak inline aliases share body reachability; explicit,
addressed, called and ordinary/internal aliases retain their required roots.

The TU frontend is released after direct construction of typed LowIR. The
optimizer and object path consume that same `Program`. Explicit LowIR input
is parsed/validated once, and audit validation is available; ordinary passes
do not repeatedly serialize/reparse or fully validate unchanged programs.
All object-relevant metadata, including debug copies, section placement,
parameter object extents and runtime roles, is durable in LowIR.

Native `compile_image` selects compact MIR one function at a time, encodes it
and releases transient selection/allocation state before the next. Direct ELF
writing consumes typed symbols, relocations, sections and unwind records.
Inspection MIR describes the actual encoder input. No assembly text or host
compiler implements required output. Benchmark `g++` commands only link objects
produced by this compiler. Cross-function LowIR retention is explicit bounded
optimization/linkage work; it does not retain otherwise-dead frontend graphs,
serialized LowIR and all-function MIR together.

## Representative data traced to the executable

The final checks retain exact commands, phase counters, validated LowIR,
consumed MIR, ELF/disassembly and executions for four independent sources.
Each direct object equals the object reconstructed from O0 textual LowIR at
all four levels. Each executable is checked with two runtime inputs per level.

| Trace | Source/semantic evidence | Lowering, optimization and native evidence |
| --- | --- | --- |
| `audit-trace.cpp`, `Pair<T>` and `demand<T>` | 180 tokens, max pending 47, 637 node/occurrence IDs, two body transitions, four substitution frames, 14 type substitutions/20 hits at every level | Recorded class/lifetime facts lower directly; debug LowIR 146→42 instructions at O1–O3; actual destructor effects and object replay retained |
| `loop-trace.cpp` | 131 tokens, max pending 17, 336 IDs, one template body transition | g0 76→32→29 instructions, one O3 unroll; debug 80→36 retains source snapshots; runtime, locations and native replay pass in both modes |
| `memory-trace.cpp` | 249 tokens, max pending 8, 448 IDs, one body transition | Debug 137→72 instructions, three reused reads; `twice<Pair>` uses one field load and an addition; pre-try capture remains distinct from the post-handler reload |
| `range-trace.cpp` | 174 tokens, max pending 15, 469 IDs, two body transitions | g0 101→40 instructions, two guarded fills; debug 110→58 retains source-value loops; both modes execute/replay correctly |

These traces test fact identity and release boundaries rather than using IR
counts as runtime claims. Native benchmarks provide the separate profitability
evidence below. In particular, the repeated-load kernel follows pointer/type
identity → exact address cell → unchanged memory epoch → reusable value →
scalar/CSE cleanup → native registers. Sixteen memory reads become one, without
spills; text grows 62→65 bytes/kernel. The measured runtime improvement pays for
that growth. External aliasing or a write/call/volatile/atomic/unwind barrier
invalidates the fact before it can suppress a load.

Complete-object base adjustment similarly follows a recorded object class and
offset through the completed layout and typed LowIR address operation. Its
native wrapper has six instructions instead of fourteen. Unknown references
still use dynamic adjustment. The existing source-identity reducer checks
complete/base/member/reference, returned/copy/move objects, actual destructor
and virtual dispatch, overload collisions, lambdas and C-context member demand.

## Findings and full ownership corrections

The outstanding debug defect crossed parser, source lowering, promotion,
propagation, folding and dead-code elimination. Lowering attached guessed
expression locations to mechanical memory/CFG instructions but did not model
named source-value snapshots. Promotion and propagation then eliminated values
needed by the required source-debug output. Statement locations sometimes
pointed at the token after `return`, `break`, `continue` or `goto`.

`dcd298af` fixes the complete path. The parser records statement keywords and
the `for` keyword. Function-owned compact slot/location records identify named
nonreference scalar source storage. Debug lowering creates a typed, explicitly
located `copy` before an eligible nonvolatile scalar store. Copy destinations
use unique display names; the optimizer identifies a snapshot by opcode and
location, never by a name prefix. No private frontend bit is needed after
serialization. Bit-field, atomic, vector, object and volatile paths retain
their own conversion/effect semantics.

Explicit source computations/transfers receive line anchors. Mechanical
loads/stores/addresses/CFG edges do not inherit guessed locations. Arithmetic
anchors begin at the left operand. A named increment on the same source line
as initialization uses that value's originating column in line-table mode;
updates on other lines retain their own file/line. For-initializer bindings
associate with the containing for statement. This is general source-location
policy; no filename, fixture snippet or expected result is recognized.

Scalar integer/floating folding, propagation and DCE retain located copies.
Memory equivalence cannot chase through them. Transform-created aliases clear
locations so they do not falsely create new source snapshots; a genuine
floating store conversion retains its original location and rounding. Inlining
and loop cloning preserve actual source locations and values. Slot promotion
publishes phi operands in stable predecessor source order; swaps consume its
existing work budget and an exhausted private analysis publishes no partial
proof. A final unconditional linear slot census removes retired source homes;
obsolete split-state bookkeeping was removed.

`debug_values.py` checks templates, shadowed variables, loop updates, floats,
keyword locations and explicit located LowIR copies at O0–O3. It executes both
source debug modes and checks eight direct/replayed object pairs. Existing
loop/range traces now cover both modes; g0 retains their profitability checks.
Their former debug-specific “must unroll/fill” assertion was an unsupported
personal pass-choice gate conflicting with required source snapshot retention.
The failed attempt is retained. Runtime, locations, MIR, object equality, g0
quality and direct-IR debug-unroll checks remain enforced; coverage increased.

No course reference changed in 218. The inherited 217 correction was separately
re-reviewed against its [reducer and LowIR proof](reference-corrections.md).
An address decreased by eight preserves its residue modulo eight; unequal
initial/end residues never meet in the 64-bit address ring. No memory access
or C++ source forward-progress premise justifies deleting that LowIR loop.
The corrected reference bundle is pinned there, with the exact two-sidecar
delta. Original input, expected status and comparison machinery remain; finite
loop deletion still has coverage. The current reducer also checks all 65,536
pairs in an 8-bit model. Compiler agreement is not the proof.

## Legality, profitability, invalidation and pipeline bounds

The [plan's implemented budget table](plan.md#operative-work-and-growth-budgets)
records per-owner limits and release points. Review verified their use at
admission, mutation and fallback, including the complete pipeline:

- Scalar work uses dirty users, exact single-definition types and conversion
  snapshots. Integer widths and target floating values are explicit; observable
  floating rounding, NaNs/signed zero and trapping/effect paths are not replaced
  by host arithmetic assumptions. CFG pruning preserves EH/address roots and
  repairs phis. No-unwind facts propagate only to indexed callers, monotonically
  within the unit budget. Ordinary dominance uses iterative traversal and a
  capped dirty worklist; incomplete proofs decline transformations.
- Slot promotion tracks sparse (block,slot) demand, publishing a demand before
  visiting predecessors. Partial/unknown facts propagate without treating an
  unvisited edge as known. EH loads require a proved common entry snapshot.
  Phi/operand growth is preflighted separately. Object splitting unifies byte
  partitions, not storage identity; only private compatible ≤64-byte/16-field
  homes qualify. Padding/representation and escapes are respected. Two fixed
  transactional splits surround immutable call admission without restarting it.
- Call proofs use complete typed target/signature/argument/effect inputs and
  immutable leaf-first summaries, bounded by site/caller/unit work and depth.
  Readonly string facts require an eligible nonalias object and correct implicit
  padding or explicit packed layout. Runtime lookup includes role, full signature
  and parameter promises: same spelling with a different ABI/role is not a hit.
  The fill helper cache lives for one invocation. Unknown mutable operands,
  recursion, ABI conflicts and exhausted contexts retain valid calls.
- Integer loop proofs widen endpoints/trips and check the final update.
  Pointer elimination requires a terminating residue proof; unproved even
  strides remain. Fill formation proves exact multiples, byte representation,
  private/alias facts and zero-trip control before touching memory. It repairs
  entry/exit phis. The shared native fill helper has four MIR instructions and
  nine bytes using `rep stosb`, no frame, and explicit caller clobbers. Full
  unrolling snapshots parallel phis and distinct continuing/exiting comparison
  values. Trip/body/function/unit clone limits all apply before mutation.
- Memory states intersect only completed ordinary predecessors and use exact
  address/width identities plus effect epochs. Imported/weak/object-named globals
  may alias ELF storage. Plain index arithmetic is not a disjointness promise.
  Floating conversions and mutable values retain snapshots; matching diamonds
  cannot cross writes/calls. Copies require complete nonoverlapping integer/
  pointer spans. No speculative load is added. CSE preserves the dying
  accumulator; native bulk encoding and parameter availability share the same
  copy/clobber policy. Loaded-value phis and call-cycle memory reuse decline
  where prior measurements exposed spills; local slot forwarding remains.

The composition is fixed, with linear censuses plus capped analyses over
consumed/produced IR, two finite split reservations, immutable inline admission
and one shared loop-clone reservoir. Cleanup never starts another interprocedural
expansion. Function analyses die/rebuild at their next scheduled boundary;
unrelated facts/caches are not globally invalidated. Forced preparation has a
separate finite unit/caller budget. Output growth is bounded across the pipeline,
not an unlimited product of local growth caps.

Final scale controls verify memory work 3100/15500/31000 for 100/500/1000
functions, contextual proof work 2000/10000/20000, and total range optimization
work 191809/959031/1918083. The 1200-call stress case stops at 32768 caller
work and retains 1047 calls. Dense-memory exhaustion declines reuse and still
validates/executes. The debug cost benchmark's g0 O3 lane reaches exactly 4096
reserved clones, unrolling 128 of 1200 functions; its remainder stays valid.
These are work/growth checks, not substitutes for executable measurements.

## Frozen performance evidence and stage acceptance

[The complete report](../student.tests/pa32/evidence218/performance.md) gives
paired medians/extrema, absolute latency, peak RSS, runtime and text for every
workload. **2156 final samples** cover fixed template-heavy memory/floating/EH/
pruning at O0/O1/O3, a compiler component, scalar O0/O1, objects, loops, memory,
ranges, contextual calls, source layouts/lifetimes and debug mode. Binaries,
flags and input hashes are frozen; CPU 2; A/A calibration followed by six ABBA
blocks. Compilation and checked execution are measured separately. Affected
telemetry is outside timings; common stats flags are symmetric. No concurrent
build/test process ran during the final measurements. All observations and
outliers remain, including **252 intermediate** audit samples.

Affected A baselines precede their owning implementation; B is `dcd298af`.
All **24** final affected B objects are byte-identical to the accepted owner
outputs (214/215/216/217). This verifies interactions in the current complete
pipeline, without assuming that a prior measurement applies to changed code.
Selected paired B/A medians follow; all spreads and absolute times are linked
above. O1 is used except O3 unroll and the explicitly labeled mode comparisons.

| Workload | Compiler ratio | Compiler peak RSS KiB A/B | Runtime ratio | Object text bytes A/B |
| --- | ---: | ---: | ---: | ---: |
| Scalar, final O0/O1 | 2.431 | 10212/15404 | 0.873 | 170400/109800 |
| Private object copies | 1.055 | 17884/20872 | 0.511 | 192000/144000 |
| Exported object copies | 1.151 | 17924/19416 | 0.600 | 188428/159628 |
| Finite loop deletion | 0.862 | 12748/11960 | 0.123 | 128000/28800 |
| Full unroll | 1.166 | 16228/17920 | 0.453 | 153600/151481 |
| Repeated loads | 1.032 | 32432/28512 | 0.582 | 111600/117000 |
| Private scalar choice | 0.946 | 18396/15092 | 0.988 | 122400/127800 |
| Repeated diamonds | 0.885 | 29112/26236 | 0.929 | 340200/207000 |
| Large repeated-value fill | 0.954 | 35468/31316 | 0.010 | 440000/420009 |
| Empty repeated-value fill | 0.962 | 35480/31276 | 0.945 | 440000/420009 |
| Finite pointer loop | 0.786 | 31584/27676 | 0.038 | 360000/70000 |
| Checked contextual call | 2.472 | 9028/20504 | 0.491 | 50535/36000 |
| Nested contextual call | 1.377 | 31784/39280 | 0.204 | 168046/40846 |
| Floating constants | 0.505 | 16092/13336 | 0.956 | 228000/33600 |
| Complete virtual-base object | 0.864 | 24192/22612 | 0.781 | 66099/16865 |
| Shared termination action | 0.965 | 40508/40904 | 0.952 | 170012/164040 |
| Final O3 g0/line-table mode | 1.065 | 26528/25476 | 1.356 | 129856/129600 |

Repeatable runtime gains justify the bounded object/loop/load/call/range work.
Examples: all six repeated-load ratios are 0.575–0.597, object-copy 0.511–0.513,
and unroll 0.450–0.465. Scalar compiler cost 2.397–2.506x buys runtime
0.867–0.883x and 35.6% less text. Context proof costs 2.376–2.611x compile time
and more memory while its checked runtime ratios are 0.413–0.526. They remain
within the documented level's work/growth budgets. No arbitrary ratio threshold
is substituted for that assessment or for the mandated fixture limits.

Private scalar choice's current 0.980–1.015 runtime range does not independently
establish a speed gain; its byte-identical earlier accepted experiment improved
all six pairs (0.949–0.996), with lower compiler latency/memory and the disclosed
three-byte/kernel text cost. Adjacent copies retain identical text/runtime and
reduce compiler work. External/loaded-value choice controls retain identical
objects and bounded attempted-analysis costs; no runtime gain is claimed.
Partial overwrite's mandated store removal retains 400000-byte text, with a
32-byte rather than 16-byte native frame in this allocator; current runtime
0.740–1.010 is noisy, and prior measurement was 0.999. This is not evidence of
a standalone runtime win or a new PA32 allocator gate.

Move and cold termination hot-path instructions are unchanged. Current cold
sharing 0.894–1.018 runtime spread, and the earlier 1.010 median on the same
object, do not establish a speed gain. The retained noinline helper saves 3.5%
text and avoids duplicate cold lowering. The earlier roughly 1% runtime cost
remains disclosed as a changed-image placement constraint, owned by later
machine optimization. The class-member linkage correction has no correct entry
executable; the timed landing variants use equivalent valid C++ linkage.

The explicit debug comparison uses the same correct final compiler with g0
and line-table flags. Required snapshots retain the source-value loop, giving
runtime 1.356 [1.210–1.513], compile 1.065 [0.766–1.148], and similar text.
Native inspection confirms the retained loop versus bounded unrolling. This is
a measured metadata-policy cost, not an optional optimization benefit or an
excuse to weaken debug output. Plain quality and direct-IR debug optimization
remain tested. Better debug-aware machine scheduling/allocation is later work;
PA32 does not require a particular unroll of source debug values.

All **12** same-level common entry/final objects are byte-identical. Compiler
paired medians span 0.938–1.045 and runtime 0.949–1.025; their variations are
not code-generation speedups. All spreads remain, including a 2.281 runtime
pair on identical O3 memory images. Common maximum positive peak RSS difference
is 116 KiB. The fixed O0 `folding.cpp` component has compiler ratio 0.992
[0.937–1.018], 1094.46/1096.67 ms, 77896/77836 KiB, identical 34466-byte text,
and no executable entry. Full self-hosting is PA34, not an omitted PA32 runtime.

Historical verification independently recomputes A/A/ABBA arithmetic and checks
source/artifact/binary hashes for all previously unaudited 215–217 handoffs:
**4564 observations, 642 source bindings, 2047 artifact bindings**. 215's mutable
dev binary paths are explicitly rebound to identical-hash frozen copies. The
older 210–214 evidence remains. Rejected optional loaded-phi, widened-copy and
call-cycle policies retain their historical 1.25–1.29x, 1.71x and 1.053/1.112x
runtime regressions; they were removed or guarded. The earlier 1.453x empty-range
regression is retained, with the corrected zero-trip admission and current
0.945x measurement. No negative observation is erased.

Stage acceptance therefore follows spec.md: retain required semantics/debug and
mandated envelopes, remove/guard measured unprofitable optional policies, enforce
finite work/growth and disclose all four dimensions. Inherited 2x compiler,
1.75x RSS, zero text growth, 10% runtime, and later 1.5x/1.05x/1.25x targets
were self-imposed diagnostics. Their historical misses do not permanently fail
corrected policies; they are not new exit criteria. No requirement, comparison
rule or coverage was weakened by reclassification.

## Final validation, ledger and remaining boundaries

The final implementation was validated sequentially, before frozen measurements:

- `make test-pa32`: **219/219**, including normal object replay **25/25**.
- `make test-report-through-pa32`: **5397/5397**, all **32/32 stages**; earlier
  PA1–31 contribute 5178. The supplied primary log also says 5397/5397 and is
  preserved as `entry-primary.log`; the prompt's 5767 census differs from both
  logs. No course fixtures were removed or changed during this audit.
- `perl scripts/cppgm_file_audit.pl --stage pa32 --paths dev/src`: pass, with
  four inherited substantial-header warnings (`procedural.h`, LowIR `model.h`,
  semantic `analyzer.h` and `model.h`).
- `make -C pa32 test-debuginfo`: **5/5 direct, 3/3 source, 25/25 debug replay**.
- All 32 required command groups in `checks.json` pass: inherited semantic,
  object, loop, memory, floating, contextual, range and native reducers; exact
  work/growth controls; four source/template/native traces; the new snapshot
  control and audit 214's comparison-value reproducer at all levels.

One exploratory command, `make -C pa8 test-debuginfo`, still reports five exact
shape differences: export metadata roundtrip, source g0 add, source debug add,
source throw closure, and machine add. The same set occurs with frozen entry
cppgm/lowiropt; native implementation is unchanged. [PA8's scope](../pa8/README.md)
explicitly places source optimization, object-link/native and DWARF/debugger
work in later assignments. These extra exact source/MIR designs are not PA32
requirements; the owning PA32 debug/object checks pass. Both failure logs and
the runner's initial rejection are bound, not described as passing. The absence
of general DWARF line sections remains an inherited later boundary; LowIR/MIR
locations and required source/serialized object identity are validated here.
PA24/PA32 regression-design directories are likewise outside the stated course
exit criteria. No profiler or allocator-specific diagnostic is an exit gate.

The ledger in [plan.md](plan.md#handoff-ledger) closes every unaudited handoff
since `40151904`, including 215 runtime-cache repairs, 216 readonly/context
proofs, 217 lifecycle/linkage/reference proof and 218 debug ownership. Earlier
checkpoint narratives and measurements remain in their commits/evidence rather
than conflicting with this final state. No unfinished PA32 work remains.

The records verifier binds the reviewed implementation, required results and
all historical attempts; recomputes 2408 current/intermediate and 4564 historical
observations; verifies unchanged fixtures, frozen binaries and owner-object
identity; and checks the final clean records-only handoff. No production source
changes occur after `dcd298af` in the final records commit.
