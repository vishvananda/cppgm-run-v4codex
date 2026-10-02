# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: 401519044a2c28130d4085b3bc7c3d0411b5dc8f

Target: **PA32 full-stage**, still incomplete. [Audit 214](audit.md) closes the
complete `76d3fcb2..40151904` review, including all three implementation handoffs
and their interactions. Entry was clean at `86d08255`, with **202/219** course
cases and **17 failures**; the audited code preserves that exact failure set.
Previous implementation turn: progress; no live process remained on entry.

Implementation 215 entered clean at `f4f075b8`, with the same 202/219 course
cases. The previous turn completed audit evidence (progress); process inspection
found no running build/test. Review markers above remain unchanged. Initial
owner group: ordinary control/dataflow and pointer ranges. Typed CFG edges,
single-definition values and slot demand records feed legality proofs; bounded
function-local work and existing growth reservations remain mandatory. Validate
dynamic/zero-trip/alias/export cases, phi snapshots and partial slot facts across
all levels, text replay, native execution and debug; freeze A/B and collect
compiler wall/RSS plus checked runtime/text evidence before acceptance.

## Architecture and operative limits

Source -> canonical typed semantic facts -> shared typed LowIR -> bounded
optimization -> text adapter or native MIR/direct ELF. Production never uses a
text roundtrip. O0 returns before optimizer analysis. O1 owns inexpensive
scalar/CFG/storage/memory/call cleanup and proven effect-free loop deletion;
O2/O3 add closed-world constant propagation; O3 allows bounded full unrolling.

| Owner | Legality and conservative fallback | Work and growth limits |
| --- | --- | --- |
| Scalar/slots | Typed constants, same-type aliases, actual store conversion and mutable snapshots; preserve traps/volatile/effects | Dirty propagation/rewrite <=16*(I+uses+1); no growth |
| Ordinary CFG | Sparse slot facts, completed dominance, scoped width-correct edge facts, phi-safe bypass | Promotion <=16*(I+O+B), phis <=I, phi operands <=2O; dominance <=32*(I+O+E+B); CSE/bypass <=16*(I+O+B), bypass operands <=2x |
| Calls/regions | Direct graph/escape census, recursive and typed-home guards; contextual effects and no-unwind facts | Depth <=64; work <=32768/caller, <=32*(I+O+P+S+F+1)/unit; growth <=1536/caller (2048 single-use). Forced expansion remains <=4194304/unit, <=262144/caller; region retirement <=32*(I+O+B+1) |
| Private objects | Single-definition origins, complete byte partitions and snapshots; escape/volatile/unknown overlays retain homes | <=64 bytes/home, <=16 fields; two fixed invocations, O((I+O+S) alpha(S)); reserve added IR <=8*(I+1), homes <=16S |
| Integer loops | Closed linear loops; exact widened trip/endpoints including final update; unknown/EH/mutable carriers decline | One invocation; candidate work <=16*(I+O+E+1); O3 <=4 trips, <=64 body clones/loop; unsimplified reservations <=256/function and min(4096,2*(I+1))/unit |
| Pointer ranges (215) | Immutable twin inductions; odd byte strides prove finite address cycles; exact contiguous counts permit byte/repeated-byte fills; zero guard precedes reference loads; phi edges retain snapshots | Same loop invocation/work budget; no instruction/block growth; exit-phi operands <=2x original, charged copies; one four-instruction native fill helper/unit |
| Memory values | Exact address/byte identities, immutable snapshots, ordinary predecessor intersections and effect epochs; backedges/handlers/unknown writes reset | One invocation; <=32 cells/state; shared <=128*(I+O+E+1) plus linear census/bounded dominance; <=16 recent diamonds, <=64 comparisons/proof; <=16 copy pieces/128 bytes; no IR/operand growth |
| Roots/native transport | Complete root/escape census; durable debug/section/extent/ABI facts; actual copy clobbers | Linear root census; one function-local MIR through immediate encoding; direct ELF |

I/O/B/E/P/S/F mean instructions/operands/blocks/edges/parameters/slots/functions.
All analysis tables, address/state slices and proof overlays have function or
invocation owners and release there. A fixed schedule composes the individual
reservations; no local rewrite restarts global inlining or a whole-program
fixed point. Exhaustion preserves valid input.

Plain `index` supplies equality, not object bounds or disjointness. Paired
noalias parameters and actual local/global identities supply stronger proofs;
imported, weak and object-named globals may share ELF storage. Memory epochs
invalidate across writes, volatile/atomic operations and exceptional state.
Repeated diamonds require a completed dominating result and exclusive use paths
before retiring matched loads. Small unaligned copies preserve scalar store
forwarding; selection and encoding share direct-copy limits/clobbers.

Profitability remains explicit: external/loaded-value conditional phis and new
memory reuse in call cycles decline; their measured policies prolonged spills.
CSE preserves a dying accumulator's operand order. Private scalar choices and
bounded memory reuse remain enabled with measured benefit. General allocation
quality remains PA33 work, without excusing avoidable PA32 regressions.

Audit 214 fixed the loop overlay: body and backedge uses of a header comparison
observe its continuing truth, while exports observe the later exiting truth.
Both are canonical `i64` values. No new analysis or growth allowance was needed.

## Remaining implementation: three broad owner groups

1. **Pointer congruence (one course failure after 215):** the twin backward
   eight-byte induction has independent pointer arguments and no congruence
   fact. Odd-stride address cycles and contiguous writes do not prove this
   loop terminates. The requirement remains open; no fixture or budget is
   weakened. All other original control/dataflow failures now pass targeted
   checks, including alias-safe guarded fills and partial slot facts.
2. **Contextual calls and EH (5):** exception-bearing candidates, builtin facts,
   landing cleanup and parent admission after initializer-list accessor cleanup.
3. **Source/ABI/debug identity (5):** lifecycle and declaration names,
   constructor/move and lambda facts. Three source-debug failures and the
   inherited O0 `call-nested-cleanup.cpp` reducer also remain required work,
   coordinated with the EH owner.

These are unfinished stage requirements, not deferred audit findings. Close
snapshot/ABI/debug, unit budget and native-profitability interactions within each
owner group before publishing its handoff. Repeatedly splitting those followups
across small handoffs was avoidable fragmentation; the unroll comparison bug
shows why phi-only snapshot checks were insufficient.

## Validated checkpoint and performance acceptance

- Earlier PAs **5178/5178**; PA32 **202/219**, exact same **17 failures**;
  through PA32 **5380/5397**. Coverage, references and comparisons unchanged.
- File audit passes with the same four inherited substantial-header warnings.
- Direct debug **5/5**, source debug **0/3**; normal/debug replay **25/25** each.
- All inherited object/loop/memory/local/dataflow/call/audit reducers pass,
  including alias/EH/escape, modular arithmetic, scaling and budget exhaustion.
  New checks cover **48 cases x four levels x three paths**, plus an undefined
  comparison reducer. Three source/template traces validate LowIR, checked
  execution, debug MIR and identical direct/replayed ELF at O0–O3.

[Evidence 214](../student.tests/pa32/evidence214/binding.json) binds the reviewed
code, commands, binaries and inputs: **1,260 new A/A + six-ABBA observations**
and **4,536 verified historical observations**. CPU 2; compile and execution are
separate; all samples/spreads retained. The audit reports wall time, RSS,
runtime and text size together. Current affected objects equal their accepted
211/212/213 objects byte for byte. Paired current B/A medians:

| Workload / level | Compiler | Peak RSS KiB A/B | Runtime | Object text bytes A/B |
| --- | ---: | ---: | ---: | ---: |
| Private copies O1 | 1.034x | 17880/20896 | 0.512x | 192000/144000 |
| Exported copies O1 | 1.131x | 18072/19380 | 0.599x | 188428/159628 |
| Finite loops O1 | 0.866x | 13092/11764 | 0.122x | 128000/28800 |
| Full unroll O3 | 1.155x | 16648/17800 | 0.452x | 153600/151481 |
| Repeated loads O1 | 1.020x | 32748/28424 | 0.581x | 111600/117000 |
| Private scalar choice O1 | 0.933x | 18928/15052 | 0.988x | 122400/127800 |
| Adjacent copies O1 | 0.878x | 17644/15680 | 0.995x | 66600/66600 |
| Repeated diamonds O1 | 0.873x | 29272/26040 | 0.936x | 340200/207000 |

Declined external/loaded-value choices retain identical objects; attempted
analysis costs 1.044/1.032x compiler time. All 12 common template-heavy
memory/floating/EH/pruning objects remain identical at O0/O1/O3; compiler
medians are 0.991–1.007x. The O0 compiler-component control is 1.005x,
77456/77452 KiB, identical 34275-byte text; it has no executable entry.

Inherited 2x compiler, 1.75x RSS, zero-growth, 10%-runtime and later
1.5x/1.05x/1.25x ratio targets are **diagnostics, not extra exit gates** under
spec.md. Audit 210's 2.337x scalar compile/0.876x runtime and the original
rejected-policy measurements remain preserved. The 3-byte load/choice growth
buys measured benefit. Current repeatable gains and bounded costs support
acceptance; no mandated fixture envelope, correctness, coverage or work/growth
limit is weakened. Later allocator/DWARF/self-hosting work adds no PA32 gate.

## Handoff ledger

| Boundary | Completed work | Still required |
| --- | --- | --- |
| Audit 210, `76d3fcb2` | Accumulated review and four ownership corrections | Then 41 course failures |
| Implementation 211, `d666b628` | Private aggregate owner and measured benefit; 12 failures removed | Then 29 failures |
| Implementation 212, `a58a3a97` | Integer-loop owner, unit budgets and typed snapshots; six failures removed | Then 23 failures |
| Implementation 213, `5b5512b4` | Bounded memory owner, actual clobbers and profitable admission; six failures removed | 17 failures |
| Audit 214, `40151904` | Full accumulated architecture/performance review; comparison-value correction; unchanged progress and clean code/records boundary | Same 17 failures plus source/debug/EH closure in the three groups above |

The code/evidence commit precedes the records-only plan/audit commit. Run
`python3 student.tests/pa32/audit214_verify.py --records` at that boundary.
Historical verifiers remain bound to their own handoffs; the new historical
verifier uses those source commits instead of imposing stale worktree bindings.
