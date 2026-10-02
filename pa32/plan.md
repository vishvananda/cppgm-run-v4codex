# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: 401519044a2c28130d4085b3bc7c3d0411b5dc8f

Target: **PA32 full-stage**, incomplete: **213/219**, **six failures**.
Implementation 216 entered clean at `335618379eb20ac285cc1df5fb498f498afccccf`
with **208/219**, eleven failures. Ralph's 208/425 uses a different census;
the unchanged root course report supplies the counts above. The prior goal
turn was progress; no live job remained on entry. Both markers are preserved.

This handoff closes contextual calls, exact floating constants, private backing
writes and EH joins. Code: `94eb1dae`, `2691df5a`, `2bace8a0`. Five original course failures
are removed, with no new failure or changes to course tests or references.
[Evidence 216](../student.tests/pa32/evidence216/README.md) binds all checks,
performance observations, binaries, inputs and implementation files.

## Architecture and operative budgets

Source -> canonical typed semantic facts -> shared typed LowIR -> bounded
optimization -> text adapter or native MIR/direct ELF. O0 skips optimizer work;
text remains an explicit adapter. The source catch fix and native floating-store
correction also apply at O0. O1 owns scalar/CFG/storage/call cleanup, finite loop
proofs and contextual inlining; O2 adds closed-world constants; O3 permits bounded
full unrolling. No ordinary machine-allocation optimization is added.

| Owner | Proof/data flow and fallback | Work, storage and growth limits |
| --- | --- | --- |
| Scalars/slots | Typed width, immutable snapshots, actual conversions; floating constants use target representations, exact conversions, signed zeros; unknown NaNs/rounding retain operations | Dirty work <=16*(I+uses+1); no growth; O(1) new floating proof |
| Calls (216) | Immutable callee graph, leaf-first admission; complete typed builtin ABI and readonly object prefix -> site constants -> ordinary reachable paths -> no-unwind proof -> clone -> dead-path/region retirement. Unknowns, recursion, mutable carriers and budget exhaustion retain calls | <=4096/site, shared <=32768/caller and <=32*(I+O+P+S+F+1)/unit; depth <=64; clone growth <=1536/caller (2048 single-use). Forced work <=4194304/unit, <=262144/caller |
| Regions/CFG (216) | Persistent handler stacks; balanced parent catch-entry exits; forward single-predecessor joins preserve handler roots, phi edges, definition order and debug locations | Region work <=32*(I+O+B+1); new joins O(I+O+B), no growth. Existing dominance <=32*(I+O+E+B), CSE/bypass <=16*(I+O+B), bypass operands <=2x |
| Private objects (216) | Existing exact origins/partitions; final unread in-bounds integer/pointer writes may retire only when every use stays private. Volatile, atomic, float conversion, escape and unknown offsets retain homes | <=64 bytes/home, <=16 fields; two splitting invocations, O((I+O+S) alpha(S)); splitting IR <=8*(I+1), homes <=16S. Final retirement is O(I+O+S), no growth |
| Integer/pointer loops | Closed, immutable inductions; exact widened endpoints, odd byte-stride finiteness; guarded contiguous repeated-byte fills, alias-safe reference loads, repaired exit phis; even-stride unknown congruence declines | One <=16*(I+O+E+1) invocation; O3 <=4 trips, <=64 clones/loop, <=256/function and min(4096,2*(I+1))/unit. Fills no block/instruction growth; exit-phi operands <=2x; one typed four-instruction helper/unit |
| Memory values | Exact byte/object facts, completed ordinary predecessor intersections and epochs; unknown writes, handlers, volatile/atomic accesses and backedges invalidate | <=32 cells/state; <=128*(I+O+E+1) shared work plus census/dominance; <=16 diamonds, <=64 comparisons/proof, <=16 copy pieces/128 bytes; no growth |
| Roots/native | Durable ABI/debug/section/extent/runtime identity; preserved copy clobbers and observable floating-store conversions | Linear census; one MIR function through immediate encoding and release; direct ELF |

I/O/B/E/P/S/F denote instructions/operands/blocks/edges/parameters/slots/functions.
Readonly-prefix summaries cost O(G+D), at most 16 byte inspections/data item,
once per scalar invocation/cloner; no object byte-buffer expansion or per-site
rescan. Proof tables/overlays release at their function/site/invocation owner.
The fixed schedule and shared clone reservations bound the complete pipeline;
local rewrites do not restart whole-program inlining or a global fixed point.

Inherited legality/profitability policies remain: plain `index` proves no object
bounds/disjointness; imported/weak/object-named globals may alias; external or
loaded-value phis and new memory reuse in call cycles decline after measured
spill regressions. CSE retains dying-accumulator operand order. These constraints
and the PA33 allocation owner do not excuse avoidable PA32 regressions.

## Remaining implementation and independent review

1. **Pointer congruence (one course failure):** backward eight-byte twin walks
   with independent pointer arguments lack matching modulo-eight residues.
   Establish a contract fact or a reduced, cited reference correction before
   deletion. Odd-stride finiteness does not prove this case.
2. **Source/ABI/lifecycle identity (five course failures):** copy/move constructor,
   lambda, complete/base lifecycle and terminate/declaration identities; lifecycle
   and move storage/instruction envelopes also remain unmet.
3. **Source-debug closure:** all three source-debug lanes still fail relaxed
   comparison. Direct debug 5/5 and object-debug replay pass. These source lanes
   are required implementation work, not waived audit questions.

Independent audit must review accumulated changes since `40151904`, including
215's pointer/phi/helper work and 216's typed constant/site proofs, EH/CFG joins,
floating exception preservation, growth bounds and measured acceptance. Neither
review nor unfinished implementation is waived.

The handoff boundary is the completed call/context/scalar-storage/EH group.
Scope expanded through leaf-parent admission, dead backing writes, nested source
catch regions and both optimizer/native floating-store effects. The remaining
source failures require canonical declaration/ABI/lifecycle producers and their
presentation/debug views; more call-site constant or region rewriting cannot
supply those missing facts. The remaining loop requires an independent contract
congruence proof. This is an implementation handoff, not stage certification.

## Validation and performance acceptance

- Earlier report: **5178/5178**; PA32: **213/219**; through PA32: **5391/5397**.
  File audit passes with four inherited header warnings. Same fixture tree,
  coverage, expectations and comparison rules as entry.
- Normal/debug object replay **25/25** each; direct debug **5/5**. All inherited
  personal reducers, bounds checks and source/template/native traces pass.
  New tests cover constant/unknown/mutable arguments, readonly/builtin ABI
  guards, private/escaped/volatile/out-of-bounds stores, five floating formats,
  dynamic rounding/signaling/inexact flags, nested source EH, debug replay and
  forward-merge phi repair.
- Context scaling: 100/500/1000 sites consume **44,161/216,161/431,161** optimizer
  operations and **2,000/10,000/20,000** context operations. A 1200-call caller
  exhausts exactly **32,768** shared operations and retains valid residual calls.

Frozen entry/candidate measurements: A/A calibration and six ABBA blocks on
CPU 2; **924 final observations** (924 initial observations retained), separate compiler and executable timings, checked
runtime results, retained spreads/outliers. At O1:

| Affected workload | Compiler ratio | Peak RSS KiB A/B | Runtime ratio | Object text bytes A/B |
| --- | ---: | ---: | ---: | ---: |
| Checked access (2400 wrappers) | 2.412x | 9012/20700 | 0.524x | 50535/36000 |
| Floating condition (2400) | 0.506x | 15892/13856 | 0.971x | 228000/33600 |
| Initializer-list parent (2400) | 1.373x | 31600/40112 | 0.192x | 168046/40846 |
| Real landing cleanup (600) | 1.106x | 20820/21256 | 0.998x | 156071/150068 |

Checked-access cloning adds about 55 ms compiler work while halving runtime;
parent cleanup adds about 95 ms while removing 81% of runtime. Both fit the
explicit bounded work/growth policies and reduce native text. Landing cleanup
meets a mandated CFG envelope with 3.8% less text; runtime is within noise, with
no speed claim. Correct floating stores retain required conversions and their
flags; comparing against their former incorrect erasure would be invalid.

Common template-heavy memory/floating/EH/pruning compiler medians are
0.992–1.051x across O0/O1/O3, runtimes 0.993–1.001x. All O0 and memory/pruning
objects are identical; floating/EH optimized text drops 11/14 bytes. O0 compiler
component: 0.988x, 77372/77548 KiB, identical text; no executable entry.
Isolated scheduling outliers remain recorded (including a 7.35x paired runtime
block for byte-identical pruning executables); no samples were removed.
Inherited 2x/1.75x/zero-growth/10%-runtime and 1.5x/1.05x/1.25x targets remain
**diagnostics, not new gates**, per spec.md. All measurements and mandated bounds
remain preserved. [215 evidence](../student.tests/pa32/evidence215/README.md) and
[214 audit evidence](../student.tests/pa32/evidence214/binding.json) remain intact.

## Handoff ledger

| Boundary | Completed | Still required then |
| --- | --- | --- |
| Audit 210, `76d3fcb2` | Accumulated review and four ownership corrections | 41 course failures |
| Implementation 211, `d666b628` | Private aggregate owner and measurements | 29 failures |
| Implementation 212, `a58a3a97` | Integer loops and unit budgets | 23 failures |
| Implementation 213, `5b5512b4` | Bounded memory and actual native clobbers | 17 failures |
| Audit 214, `40151904` | Accumulated review and loop comparison fix | 17 failures; source/debug/EH |
| Implementation 215, `1aa04ae6`; evidence `104846c6` | Scalar/phi and finite ranges; six failures removed | 11 failures; audit pending |
| Implementation 216, `94eb1dae`, `2691df5a`, `2bace8a0` | Contextual calls, scalar facts, backing-store/EH closure; five failures removed | Six course failures, three source-debug failures, independent accumulated review |

Run `python3 student.tests/pa32/context_verify.py` at this records-only boundary.
Historical verifiers stay bound to their own handoffs. The last-reviewed marker
is not advanced by this implementation handoff.
