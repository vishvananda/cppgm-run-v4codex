# PA32 checkpoint audit 214

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: 401519044a2c28130d4085b3bc7c3d0411b5dc8f

Reviewed range: `76d3fcb2..40151904`, including the complete accumulated entry
range `76d3fcb2..86d08255` and the audit correction. Both entry markers named
`76d3fcb2`; no boundary recovery or narrowing was necessary. The prior
implementation turn was progress: 213 committed memory ownership and evidence.
Process inspection found no live build/test process to resume on entry.

This completes the checkpoint audit. PA32 full-stage implementation remains
incomplete: **17/219 course failures** and **three source-debug failures** remain
required work. The entry report's status **2** is make's exit status, not a count
of failing tests; its **202/425** is a different census. The authoritative root
course run is **202/219**, and its exact failure set is preserved.

## Complete range and ownership review

| Boundary | Every accumulated commit reviewed | Combined implementation and interactions |
| --- | --- | --- |
| Previous audit records | `2e1238dc` | Prior review boundary, accepted policies and inherited obligations |
| Private objects, 211 | `d666b628`, `a7fe2af0` | Object address/use census, union/find byte partitions, escape and representation guards, scalar snapshots, call-admission interaction, slot retirement |
| Integer loops, 212 | `22b4956c`, `a58a3a97`, `cec5b40a` | Ordinary-flow shape/dominance, widened trip proof, analytic deletion, parallel phi snapshots, source ordering, unit/function reservations and debug/ABI cloning |
| Memory, 213 | `96005658`, `d4ae8856`, `5b5512b4`, `86d08255` | Address provenance, joins and effect epochs, copies/diamonds, unsigned decrement facts, CSE operand costs, shared selection/encoding clobber policy and measured admissions |
| Audit correction | `40151904` | Continuing versus exiting comparison values in the loop substitution owner; executable reducers and complete evidence verification |

All **15 changed implementation paths** were inspected in final combined form:
`frontend_source_sets.mk`; LowIR `object_splitting`, `loop_simplify`, `loop_trip`,
`memory_values`, `edge_facts`, `local_cse`, `optimizer`, `folding.h`, `model.h`
and the loop/memory headers; native `bulk_encoding`, `bulk_policy.h` and
`parameter_flow`. New translation units are registered for both optimizer and
source-driver consumers. The intervening commits' source and evidence changes
were reviewed, including the later profitability and budget corrections, not
just their latest snapshots. Personal tests only gained coverage. No course
fixture, reference, contract, harness or comparison rule changed.

## Finding and correction

**Full unrolling deleted a comparison still used as a value.** The overlay
mapped the loop header's comparison to its original temporary during cloned
body execution. Only afterward did it install the exit result. If the result
was not exported, serialized output contained an undefined temporary. If it
was exported, later propagation could make every body use the final exit truth.
A three-trip loop storing its comparison to a volatile global therefore either
failed object validation or produced the wrong result.

The loop owner now installs the continuing arm's canonical `i64` truth before
cloning bodies and taking backedge snapshots, and installs the exiting truth
afterward. The proof follows [LowIR comparisons](../pa8/lowir.md#unary-and-binary-operations)
(`cmp` produces `i64` 0/1), [parallel phi edges](../pa8/lowir.md#control-flow-value-merges),
and [observable volatile accesses](../pa8/lowir.md#memory-and-addressing).
This is one bounded overlay assignment, with no added IR, growth allowance,
analysis or stronger arithmetic promise. Existing instruction debug locations
and side-effect order remain attached to their clones.

[audit214.py](../student.tests/pa32/audit214.py) checks 48 cases: zero through
five trips, both continuing branch directions, stores, backedge phis,
accumulation, calls and exported exit comparisons. Every case runs at O0–O3
through direct source-IR object compilation, optimized-text replay and the native
executable path. A separate unexported reducer checks the undefined-temporary
failure. Frozen entry binaries fail all three O3 execution paths and reject the
unexported output; the reviewed binaries pass. Debug MIR also retains the
cloned locations. No reference correction or bundle revision was necessary.

## Architecture and fact traces

The source traces are [audit-trace.cpp](../student.tests/pa32/audit-trace.cpp),
[loop-trace.cpp](../student.tests/pa32/loop-trace.cpp) and
[memory-trace.cpp](../student.tests/pa32/memory-trace.cpp). Their current commands,
telemetry, validated LowIR, consumed MIR, ELF views and executions are frozen
under the binding's `checks/{trace,loop-trace,memory-trace}` directories.

- `lowering/driver.cpp` owns immutable preprocessing buffers, identifier
  interning, streaming `PostTokenCursor`, the bounded syntax cursor, shared
  `Ast` and semantic analyzer. Parser and semantics cooperate on that graph.
  No complete token/syntax/semantic copy was introduced by these handoffs.
- `semantic/template_call.cpp::specialize` uses canonical pattern and interned
  argument-pack IDs; the selected pattern supplies its immutable parent
  substitution frame. Declaration/body fact states distinguish active, failed
  and completed demand. Incomplete dependencies use their own query revisions.
  `template_instantiation.cpp` demands a parsed body once; occurrence overlays
  and fixed-expression facts reuse source topology and nondependent work.
  The nontrivial `Pair<T>`/`demand<T>` trace has 180 tokens, maximum pending 47,
  637 node/occurrence IDs, five specialization records and **two** template body
  transitions, with 14 type-substitution operations and 20 hits at every level.
- `lowering/symbols.cpp` maps selected entities and distinct ABI entries to
  symbol/function IDs. `function_body` requires recorded body facts and consumes
  conversions, layouts and lifetime actions; `DebugScope` owns temporary source
  context. TU preprocessing/semantic storage dies after direct typed lowering.
  The optimizer consumes the same `Program` passed to `compile_object`.
- Function-local optimizer ID tables, use chains, state slices, partition
  arrays, dominator scopes and substitution overlays release at their natural
  function/invocation boundary. Names are presentation identities, not keys for
  rediscovering semantics. Changes rebuild the next scheduled local analysis;
  no process-global cache, semantic reconstruction, text roundtrip, global retry
  or per-local-change full-program fixed point was added.
- Native selection owns one compact MIR function, encodes it immediately and
  releases it before the next. The host ELF writer directly emits sections,
  symbols, relocations and unwind records. Text and MIR are explicit inspection
  adapters. All three traces produce byte-identical direct/replayed objects at
  all four levels and pass two independently checked runtime inputs per level.
  Required LowIR/MIR locations survive; inherited absence of DWARF line sections
  is recorded, not turned into a new PA32 gate.

The object trace falls from 144 to **58** LowIR instructions at O1–O3, splitting
three objects into six fields while preserving three destructor effects. The
loop trace falls from 76 to **31** at O3, with one unroll reserving **28**
instructions; its repeated template demand has one body transition. The memory
trace falls from 132 to **71** with three reused reads. `twice<Pair>`'s two
field reads become one load and one addition, while `capture` retains the
pre-try snapshot and the post-handler reload across possible writes/unwind.

The repeated-load benchmark follows the full fact path: typed single-definition
pointer -> exact address/width cell -> unchanged memory epoch -> reusable stable
value -> scalar cleanup -> canonical CSE numbering that preserves the dying
accumulator -> native registers. Disassembly confirms **16 memory reads become
one**, with no spills, and text changes **62 to 65 bytes** per kernel. Its
runtime measurement below pays for that growth. Unknown writes, calls, volatile
and atomic operations reset or invalidate the corresponding facts. Native bulk
encoding and parameter availability now share exactly the same direct-copy
size policy; the 32 boundary reducers check live GP/FP carriers across it.

## Legality, profitability, budgets and conservative outcomes

The [plan](plan.md) retains operative bounds. Object splitting admits only
single-definition local address chains and complete compatible byte partitions;
unknown uses escape their home. Complete copies join layout constraints without
joining storage identity. Padding is retained, and floating/I1/I128 bulk
representations decline splitting. A transaction reserves growth before mutation.
Two fixed invocations surround call admission; they do not restart inlining.

Loop proofs require a closed linear body, completed ordinary dominance,
single-definition carriers and exact widened endpoints, including the last
update. Unknown counts, overflow, exceptional CFGs, mutable carriers and unsafe
source ordering retain input. Analytic deletion requires nontrapping pure work
and available exports; O3 cloning separately applies trip/body/function/unit
profitability and growth limits. The comparison-value correction completes the
same overlay's value semantics; it changes none of these admission rules.

Memory states use exact identities plus modular byte ranges. Plain `index`
conveys equality, not disjointness or readonly provenance. Only paired noalias
parameters or proven distinct homes/globals justify disjointness; imported,
weak and object-named globals may share ELF storage. Joins intersect completed
ordinary predecessors; unvisited backedges/handler entries reset facts. Stores
normalize to their actual width, floating conversion and mutable values retain
snapshots, and effect epochs delimit repeated-diamond equivalence. Removing a
matched load requires an exclusive def/use path. Adjacent copies require
complete nonoverlapping integer/pointer spans. No load is speculated.

Admission remains separate from legality: external/loaded-value phis and new
memory reuse in call cycles decline because their measured spill/lifetime costs
were unprofitable. Local slot forwarding remains available. Unaligned small
copies use scalar chunks to preserve store forwarding; aligned copies retain
vector forms. The earlier 1.25–1.29x loaded-value-phi, 1.71x widened-copy and
1.053x/1.112x call-cycle regressions remain recorded, with their policies removed.

Work remains linear or capped: function-local use lists/union-find, one loop
invocation, one bounded RPO memory sweep and a 16-diamond window; dirty
instruction/block worklists in inherited scalar/CFG cleanup. All invocations
form a fixed schedule. Object growth is reserved against consumed input before
each of two invocations, optional inlining has unit reservations, loop growth
is shared across the unit, and memory adds no IR/operands. Their composition is
bounded without repeated expansion. Exhaustion retains valid conservative IR.
Current 100/500/1000-function memory work is exactly 3100/15500/31000;
dense-CFG exhaustion is exercised. Loop tests exercise the 256/function and
4096/unit ceilings, body refusal, variadic type snapshots and debug cloning.

## Performance evidence and stage-scoped acceptance

[Binding](../student.tests/pa32/evidence214/binding.json) freezes **573 source
bindings**, **315 artifacts**, commands, binary/input/output hashes and all
**1,260 new observations**. Artifacts are retained in
`/home/vishvananda/work/private/v4codex/artifacts/pa32-214`.
Each axis has A/A calibration followed by six wall-time ABBA blocks, CPU 2.
Compilation and checked execution are separate. Runtime input and independently
checked results keep the workloads live; host g++ only links compiler-produced
objects. Affected telemetry is measured separately; common telemetry flags are
identical on A and B. All observations, spread and outliers are retained.

Affected A baselines are pre-211 for objects, pre-212 for loops, and pre-213 for
memory; B is the reviewed compiler. Every B affected object is byte-identical
to that owner's accepted handoff, verifying interactions across all later passes.
O1 is used except O3 for full unrolling. Ratios below are paired B/A medians
and ranges; times are separate sample medians.

| Workload | Compile ratio [range] | Compile ms A/B | Peak RSS KiB A/B | Runtime ratio [range] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| Private copies | 1.034 [1.026–1.090] | 149.02/154.98 | 17880/20896 | 0.512 [0.511–0.514] | 357.83/183.40 | 192000/144000 |
| Exported copies | 1.131 [0.899–1.253] | 126.61/143.85 | 18072/19380 | 0.599 [0.597–0.602] | 355.88/212.86 | 188428/159628 |
| Finite loop deletion | 0.866 [0.837–0.873] | 89.12/77.09 | 13092/11764 | 0.122 [0.122–0.124] | 1146.54/140.30 | 128000/28800 |
| Full unroll | 1.155 [1.146–1.163] | 123.71/143.27 | 16648/17800 | 0.452 [0.442–0.469] | 377.79/171.10 | 153600/151481 |
| Repeated loads | 1.020 [1.001–1.075] | 242.08/247.93 | 32748/28424 | 0.581 [0.577–0.608] | 145.11/84.16 | 111600/117000 |
| External choice control | 1.044 [1.034–1.067] | 115.99/121.29 | 15948/15976 | 0.997 [0.956–1.026] | 81.97/81.81 | 147600/147600 |
| Private scalar choice | 0.933 [0.930–0.956] | 127.79/119.82 | 18928/15052 | 0.988 [0.949–0.996] | 84.60/83.71 | 122400/127800 |
| Loaded-value choice control | 1.032 [0.931–1.043] | 145.42/150.80 | 17480/17576 | 1.000 [0.973–1.003] | 84.49/84.52 | 135000/135000 |
| Adjacent copies | 0.878 [0.863–0.894] | 100.92/88.50 | 17644/15680 | 0.995 [0.978–1.018] | 82.30/82.32 | 66600/66600 |
| Repeated diamonds | 0.873 [0.830–0.896] | 301.34/263.22 | 29272/26040 | 0.936 [0.921–0.942] | 105.10/97.24 | 340200/207000 |

Objects, loops, repeated loads, private scalar choices and diamonds improve in
all six runtime pairs. Copy coalescing improves compilation with runtime at
parity and unchanged text size. Declined choice controls keep byte-identical
A/B objects; their 3.2–4.4% compile cost is bounded attempted analysis, with no
runtime claim. The added compile work for each profitable expensive transform
is repaid by one checked representative execution in these workloads. These
measurements do not imply a universal speedup.

Common controls compare frozen entry/current compilers at the same level:

| Level / workload | Compile ratio | Compile ms A/B | Peak RSS KiB A/B | Runtime ratio | Runtime ms A/B | Identical object text bytes |
| --- | --- | --- | --- | --- | --- | --- |
| O0 memory | 1.004 | 168.93/169.22 | 30080/29952 | 1.002 | 52.25/52.28 | 151393 |
| O0 floating | 0.991 | 168.71/167.57 | 30756/30688 | 1.001 | 48.85/48.92 | 151234 |
| O0 exceptions | 0.993 | 169.61/168.21 | 30716/30628 | 1.011 | 249.22/251.81 | 151541 |
| O0 pruning | 0.997 | 209.19/208.96 | 36432/36396 | 0.998 | 52.35/52.33 | 151393 |
| O1 memory | 1.001 | 193.33/194.48 | 30024/29960 | 0.999 | 48.84/48.92 | 124959 |
| O1 floating | 0.995 | 203.71/203.55 | 30840/30636 | 1.005 | 48.10/48.31 | 124867 |
| O1 exceptions | 1.003 | 202.84/203.75 | 30648/30520 | 1.002 | 250.31/250.41 | 125067 |
| O1 pruning | 1.007 | 236.43/237.90 | 36236/36304 | 1.001 | 48.97/48.73 | 124959 |
| O3 memory | 0.999 | 192.20/193.43 | 29992/29968 | 0.990 | 48.70/48.75 | 124959 |
| O3 floating | 0.995 | 203.86/204.11 | 30720/30688 | 1.001 | 48.19/48.16 | 124867 |
| O3 exceptions | 0.994 | 201.92/201.93 | 30648/30648 | 1.007 | 250.43/251.39 | 125067 |
| O3 pruning | 1.000 | 236.59/236.05 | 36364/36364 | 1.001 | 48.93/48.99 | 124959 |

All 12 A/B objects are byte-identical; runtime variation is not attributed to
code changes. The largest positive RSS difference is 68 KiB. Raw common compile
outliers, including 0.598 and 1.467 pairs, remain disclosed in the JSON spreads.
The compiler-owned `folding.cpp` O0 control is 1.005 [0.999–1.024],
1075.56/1088.60 ms, 77456/77452 KiB, with identical **34275-byte** text.
It has no executable entry, so runtime is inapplicable; full compiler self-hosting
remains PA34 work. The correctness reducer's broken entry output is never used
as a performance baseline.

[Historical verification](../student.tests/pa32/evidence214/history.json) checks
211/212/213 at their actual committed handoffs, including dereferenced source
symlinks, binary/artifact bindings and recomputed A/A/ABBA arithmetic: **4,536
observations**, 1,643 source bindings and 1,104 artifact bindings. Original failed
policies and 48 short diagnostic observations in 213 remain in their original
files; no sample was discarded. Earlier audit 210's 2.337x scalar compile cost,
0.876x runtime and 35.6% text reduction remain accepted evidence.

**Stage-scoped interpretation:** inherited 2x compiler, 1.75x RSS, zero-growth,
10% runtime and later 1.5x/1.05x/1.25x ratio targets are self-selected diagnostics,
not additional exit gates. The historical object verifier's ratio assertions
check its frozen experiment; they do not impose those gates on this audit.
Current measured benefits, exact work/growth limits and removal of unprofitable
optional policies support acceptance. The three-byte load/choice growth is
measured and justified; a historical miss does not permanently fail a corrected
policy. No mandated fixture envelope, correctness condition, comparison rule or
coverage requirement is weakened. Later allocator/DWARF/self-hosting constraints
create no extra PA32 gate, and do not excuse avoidable regressions.

## Validation and remaining work

- `make test-report-through-pa31`: **5178/5178**, exit 0.
- `perl scripts/cppgm_file_audit.pl --stage pa32 --paths dev/src`: exit 0,
  the same four substantial-header warnings.
- `make test-pa32`: **202/219**, exit 2, exact **17-failure set unchanged**.
- `make test-report-through-pa32`: **5380/5397**, only those PA32 failures.
- Required debug: direct **5/5**; source **0/3**, each source lane run explicitly
  despite the aggregate target stopping at its first failure. Normal and debug
  object replay each pass **25/25**.
- Explicit personal checks: 43 object, 78 loop, 90 memory, 32 bulk-carrier,
  506 local, 134 dataflow, 63 call and 119 earlier-audit execution cases across
  four levels and their native/replay paths; 346 finite-domain loop executions,
  154 long/infinite loop guards and 2,136 modular comparison cases; scaling,
  exhaustion, alias/EH/escape/debug/ABI guards; the new 48 cases plus reducer.
- `audit214_verify.py --records` verifies current source/evidence/artifact
  bindings, exact progress preservation, measurements, bounds and the final
  records-only commit/clean worktree boundary.

Remaining work is grouped broadly in the plan: control/dataflow closure
(pointer loops, fills and partial phi/storage promotion); contextual calls and
EH/builtin admission; source declaration/lifecycle/ABI/debug identity. The
inherited O0 nested-cleanup reducer also remains in that source/EH work.
These are incomplete stage requirements, not unresolved audit findings or waivers.

The owner groups were useful, but repeatedly publishing a transform before its
unit budget, value snapshot, debug and native profitability interactions closed
caused avoidable handoff fragmentation. The unroll comparison defect is another
example: phi snapshots alone did not cover all header-produced values. Future
handoffs should close those interactions as one owner group before publishing.

| Audit | Reviewed range and code tip | Findings/evidence | Result and remaining owners |
| --- | --- | --- | --- |
| 210 | `e82bf415..76d3fcb2` | Four typed/snapshot/debug corrections; accumulated evidence; full narrative retained in `2e1238dc` | 178/219, 41 failures; memory/aggregate, calls/loops, source/debug closure |
| 214 | `76d3fcb2..40151904`; Last reviewed commit above | Comparison-value overlay corrected; all three handoffs and 15 implementation paths reviewed; 5,796 verified A/A/ABBA observations; prior/file/progress checks | Checkpoint audit complete; 202/219, identical 17 failures; control/dataflow, calls/EH, source/ABI/debug closure |

The code and evidence tip was validated and committed first. The following
records commit changes only this audit and the compact plan; no code edit follows
that reviewed tip.
