# PA32 final plan and completion record

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: dcd298afff36573014188a2c3cee826f2500cbf8

Target: **PA32 full-stage**. Phase: **audit complete**. The independent review
reconstructed the accumulated compiler, reviewed all handoffs since `40151904`,
and closed the three outstanding source-debug failures. No PA32 implementation
or unaudited handoff remains. The [final audit](audit.md) records findings,
ownership traces, performance acceptance and limitations; [evidence 218](../student.tests/pa32/evidence218/README.md)
binds the reviewed code, all observations and complete check logs.

## Spec Alignment and final design

Immutable source buffers and streaming preprocessing feed one parsed/typed
semantic graph. Canonical names, entities, types, template packs and immutable
substitution frames key facts; scoped dependency worklists demand parsed bodies.
Lowering consumes recorded conversions, layouts, lifetimes and ABI entries into
one typed LowIR Program. The frontend dies at that boundary. Bounded optimization
feeds explicit LowIR views or per-function MIR and direct ELF encoding. Text is
an adapter, never production transport; the compiler does not delegate required
output to reference tools or a host compiler.

O0 bypasses optimization. O1 has bounded scalar/storage/control/call/loop/memory
policies; O2 additionally propagates exact internal-call constants; O3 may fully
unroll proved small loops. Admission separates legality, profitability and
budget. Exhaustion retains conservative valid IR. Mutation rebuilds only the
next scheduled function analysis; no global retry or expanding fixed point.

Located LowIR `copy` instructions now carry explicit source-value snapshots.
They are retained through scalar simplification and memory/slot rewrites, and
survive serialization without frontend side data. Generated aliases do not
invent source snapshots. Parser keyword locations and source computation/value
anchors survive lowering; phi inputs use stable predecessor order. Unused slots
are retired in one final census. Plain compilation has no extra debug copies.

## Operative work and growth budgets

I/O/B/E/P/S/F mean instructions/operands/blocks/edges/parameters/slots/functions
at the documented owner's admission boundary. Fixed linear censuses and output
construction are additional to capped proof work. These are implemented bounds;
fixture outcome/envelope limits remain independently mandatory.

| Owner | Legality, invalidation and fallback | Work, storage and growth |
| --- | --- | --- |
| Frontend/demand | Interned scope/entity/type/pack keys, immutable parent frames, separate active/failure/success facts; targeted query revisions and reverse dependencies | TU slabs and flat indices; one parse/source region; dependent occurrence overlays; release after typed lowering |
| Display/object/lifetime | Dense entity/scope ordinals; completed most-derived class+offset for known nonreferences; unknown references remain dynamic; recorded destructor effects and distinct ABI identities | Linear declaration census/rendering; 4 bytes/entity, 16 bytes/lambda; 16 bytes/transient complete-object Value fact; no IR growth |
| Source debug | Function-owned slot→location index; a located copy before a nonvolatile same-type scalar source store; explicit instruction metadata survives replay | At most one snapshot per eligible store; linear source lowering, O(slots) location records; charged phi input swaps; final linear slot census |
| Scalar/CFG | Single-definition widths/types; exact floating facts preserve observable rounding; effects/EH/address roots retained; dirty use worklist | Each scalar propagation/alias phase ≤16*(I+uses+1); ordinary-flow dominance ≤32*(I+O+E+1); call/EH closure ≤32*(I+O+B+1); conservative partial-proof rejection |
| Slot/object storage | Sparse (block,slot) demand, published before recursive predecessor demand; exact store conversion and EH entry snapshots; private nonescaping compatible byte partitions | Promotion proof ≤16*(I+O+B), separately capped phi/operand growth; homes ≤64 bytes/16 fields; two fixed splits each reserving ≤8*(I+1) growth |
| Calls | Immutable leaf-first admission, exact complete site keys, typed readonly/runtime identity and no-unwind proofs; unknown mutable/ABI facts decline | ≤4096/site, ≤32768/caller, ≤32*(I+O+P+S+F+1)/unit; depth ≤64; clone growth ≤1536/caller (2048 single-use); forced preparation separately ≤4194304/unit and ≤262144/caller |
| Loops/ranges | Widened endpoints/last update; exact pointer residue/multiple proofs; zero-trip guard before memory/calls; parallel phi and comparison snapshots | One invocation, proof ≤16*(I+O+E+1); O3 ≤4 trips/64 clones per loop, ≤256/function, min(4096,2*(I+1))/unit; one typed shared fill helper |
| Memory/CSE | Exact address/type cells and effect epochs; completed predecessor intersections; aliasing/volatile/atomic/call/unwind invalidate; loaded-value/call-cycle spill hazards decline | ≤128*(I+O+E+1) proof; ≤32 cells/state, ≤16 diamonds, ≤64 comparisons/proof, ≤16 copy pieces/128 bytes; no memory-transform IR growth |
| Native/ELF | Compact function MIR, shared bulk-copy clobber policy, typed ABI/relocation facts, actual consumed MIR inspection | Selection/allocation/encoding per function, release immediately; direct sections/symbols/relocations/unwind; no assembly transport |

The fixed pipeline composes finite local passes, two split reservations, one
immutable call admission and one shared loop-clone reservoir. It never restarts
expansion after cleanup. Growth is bounded across the unit, not merely per
callee or loop. Whole-unit LowIR is retained for this explicit bounded work;
frontend graphs, serialized text and all-function MIR are not retained alongside it.

## Validation and performance acceptance

Required final checks pass: `make test-pa32` **219/219**;
`make test-report-through-pa32` **5397/5397**, **32/32 stages**;
`perl scripts/cppgm_file_audit.pl --stage pa32 --paths dev/src` pass with four
inherited header warnings. Required PA32 debug passes **5/5 direct + 3/3 source
+ 25/25 object replay**; ordinary replay is also **25/25**. All 32 required
check commands, personal runtime/replay/native reducers, traces and work/growth
controls pass. The supplied primary log also reports 5397/5397, matching the
final run; the prompt's 5767 census differs from both logs. No course fixture
or comparator changed during audit 218.

The optional earlier PA8 debug-shape probe has the same five failures at entry
and final. PA8 explicitly excludes the later source/optimizer/native/DWARF
surfaces; this probe is preserved as a diagnostic, not substituted for PA32's
required debug check. Details and logs are in the audit/evidence.

[Measurements](../student.tests/pa32/evidence218/performance.md) contain **2156
final observations**, **252 intermediate observations**, and **4564 newly
verified historical observations** from handoffs 215–217. Frozen binaries,
flags, inputs and CPU 2; A/A calibration then six ABBA blocks; compiler and
checked executable timings/RSS separately; object text and native inspection.
All 24 affected final objects match their accepted owner outputs byte for byte.
All 12 common same-level entry/final objects and the compiler-component object
also match. Noise on identical images is not an optimization benefit.

Repeated loads, object copies, finite loops, bounded unrolling, contextual calls,
range fills and complete-object lowering retain measured useful gains. Costs
include scalar O1/O0 compiler 2.431x for runtime 0.873x/text −35.6%; contextual
proof compiler 2.472x for runtime 0.491x; and O3 debug/g0 runtime 1.356x for
required source snapshots. Compiler/memory/text costs and paired spreads are
fully disclosed. Debug snapshots preserve the existing source contract and may
limit loop transforms; g0 quality and direct-IR debug coverage are preserved.

Inherited 2x compiler/1.75x RSS/zero growth/10% runtime, later 1.5x/1.05x/1.25x
ratios and personal debug pass-choice assertions are diagnostic targets, not
additional stage gates. The audit retains historical misses and rejected
policies, enforces actual work/growth and fixture limits, and separates required
semantic/debug costs and later allocator/DWARF/self-hosting work. No mandated
behavior, coverage or comparison rule was weakened.

## Handoff ledger

| Boundary | Completed | Obligations at that boundary; final disposition |
| --- | --- | --- |
| Audit 210, `76d3fcb2` | Independent review and four ownership fixes | 41 course failures; closed by later implementation and re-reviewed in 218 |
| Implementation 211, `d666b628` | Private aggregate storage | 29 failures; whole-stage storage interactions re-reviewed |
| Implementation 212, `a58a3a97` | Integer loops/unit budgets | 23 failures; endpoint/phi/native behavior re-reviewed |
| Implementation 213, `5b5512b4` | Memory and native clobbers | 17 failures; alias/effect/spill admissions re-reviewed |
| Audit 214, `40151904`; records `f4f075b8` | Accumulated architecture/performance and loop comparison fix | 17 course and three source-debug failures; historical narrative preserved in that commit |
| Implementation 215, `0f370e3f..33561837` | Sparse slot facts, diamonds, finite ranges, complete fill runtime identity | 11 failures and review pending; proofs/cache key/measurements reviewed in 218 |
| Implementation 216, `94eb1dae..7e04081b` | Floating constants, contextual calls, private storage/EH, readonly layout | Six course and three source-debug failures; full ownership paths/performance reviewed in 218 |
| Implementation 217, `cc7d3c20..5a767f20` | Displays, complete objects, aliases/linkage/cold actions; proved even-stride oracle correction | Zero course and three source-debug failures; all source/ABI/cache/contract changes reviewed in 218 |
| Final audit 218, `dcd298af` | Source snapshots/locations, stable phi order, slot cleanup; independent whole-stage architecture and performance review | All PA32 requirements pass; no unaudited handoff or unfinished PA32 work |

Run `python3 student.tests/pa32/audit218_records.py verify --clean` to verify the
final records-only handoff, immutable reviewed code, required results, retained
failed attempts, measurements, historical bindings and clean worktree.
