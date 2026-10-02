# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: 401519044a2c28130d4085b3bc7c3d0411b5dc8f

Target: **PA32 full-stage**. Course checks **219/219**; **three source-debug
checks remain implementation work**. Independent accumulated review is pending.
217 entered clean at `7e04081b611b89cd46e5b294a963aba891c36073` with **213/219**,
six failures. The previous goal turn was progress (216's committed work); no
live process remained. Both review markers are preserved. Ralph's 213/425
entry census uses a different population; these are root course-report counts.

217 closes declaration displays, complete-object/lifecycle facts, alias emission
and cold termination actions (`cc7d3c20`), extended through a discovered member
linkage defect (`c81197b0`). Five original failures are fixed by code. `940d3396`
corrects the sixth oracle with a reduced [LowIR congruence proof](reference-corrections.md).
No input, exit status or comparison rule changed. [Evidence 217](../student.tests/pa32/evidence217/README.md)
binds checks, frozen binaries, measurements and the exact reference delta.

## Design and operative budgets

Canonical source/semantic graph -> typed LowIR -> bounded optimization -> text
view or per-function MIR/direct ELF. EntityId/SymbolId and typed ABI identities
own lookup and emission; display strings only render them. Earlier source-view
adapters retain their specified presentation. Object-capable emission consumes
the same semantic graph and completed layout facts. O0 skips the optimizer;
semantic address selection and linkage correctness also apply at O0.

| Owner | Data flow and conservative boundary | Complexity, storage, growth |
| --- | --- | --- |
| Display (217) | Canonical declaration census -> scope/name ordinal; parser keeps lambda declarator ordinals, never tokens; collision allocator; separate ABI identity | O(declarations + rendered bytes); flat counters, 4 bytes/entity, 16 bytes/lambda; TU release; no IR growth |
| Object/lifecycle (217) | Nonreference object/member -> most-derived class + offset -> completed base layout; unknown references/loaded pointers remain dynamic. Existing destructor-effect fact removes unobservable vptr stores, retaining required entries | O(1) completed-fact lookup/conversion; 16 bytes/transient Value; no semantic re-resolution or IR growth |
| Roots/cold actions (217) | Weak inline bodies/aliases share lifetime; ordinary/internal aliases, explicit roots, addresses and live calls remain. One noinline termination action/program. Class members keep C++ linkage in C demand contexts | Linear symbol/alias/reference census and deduplicated reachability; invocation scratch; no growth; O(1) linkage rule |
| Scalars/storage/CFG (inherited) | Exact widths/conversions/snapshots, balanced EH, private extents | Scalar dirty work <=16*(I+uses+1); regions <=32*(I+O+B+1); homes <=64 bytes/16 fields, two splits, growth <=8*(I+1); joins linear/no growth |
| Calls (216) | Immutable bodies, leaf-first admission, typed site/readonly and no-unwind proofs | <=4096/site, <=32768/caller, <=32*(I+O+P+S+F+1)/unit; depth <=64; clone growth <=1536/caller (2048 single-use); forced work <=4194304/unit, <=262144/caller |
| Loops/memory (inherited) | Proved endpoints/residues; alias/effect/epoch-aware memory facts; unproved even strides remain | Loops <=16*(I+O+E+1); O3 <=4 trips/64 clones per loop, <=256/function, min(4096,2*(I+1))/unit. Memory <=128*(I+O+E+1), <=32 cells/state, <=16 diamonds, <=64 comparisons/proof, <=16 copy pieces/128 bytes; no memory growth |

I/O/B/E/P/S/F mean instructions/operands/blocks/edges/parameters/slots/functions.
Fixed schedules and unit reservations are unchanged. Inherited details and
measurements remain in [216](../student.tests/pa32/evidence216/README.md) and
[215](../student.tests/pa32/evidence215/README.md), including floating, alias,
spill and loop guards. No global retry, semantic string keys or global cache.

## Remaining implementation and review

**Source-debug closure is unfinished:** O1/O2 source values and O3 loop source
values fail required relaxed comparisons. Direct debug is 5/5; normal/debug
object replay is 25/25 each. This needs source assignment/value identities and
statement locations preserved through promotion, propagation and phi construction.
It is implementation work, not a waived requirement or an audit question.

The completed group expanded from naming through alias roots, complete/base
lifecycle entries, C-context member linkage, cold termination sharing and the
independent loop-oracle proof. Its concrete boundary is the remaining source-debug
value/location producer and optimizer policy, a separate cross-phase data-flow
change. More declaration rendering or layout facts cannot recover eliminated
debug values. This ends implementation 217, not PA32 certification/advancement.

**Independent review remains required** since `40151904`: 215/216 proofs and
acceptance; 217's complete-object propagation, alias roots, linkage, display
collisions, corrected oracle and measured tradeoffs. Neither accumulated review
nor unfinished debug implementation is waived.

## Validation and performance acceptance

Final sequential checks: earlier **5178/5178**, course **219/219**, through PA32
**5397/5397**, file audit pass (four inherited header warnings). All personal
reducers, native/source/template traces and work/growth checks pass. New checks
cover overloads/lambdas, C-context members, dynamic/complete virtual bases,
copy/move/destructor dispatch, alias roots and pointer nontermination. The three
source-debug failures remain. Regression fixtures are outside course exit criteria.
Intermediate overlapping root reports had mixed counters and are exploratory
only; final required reports run sequentially.

Frozen inputs/flags and A/B binaries, CPU 2, A/A plus six ABBA blocks, separate
compiler/executable timings with checked runtime-dependent results:
**868 final observations**, **280 historical observations**, all preserved.

| Affected O1 workload (1200 wrappers) | Compiler ratio | Peak RSS KiB A/B | Runtime ratio | Object text bytes A/B |
| --- | ---: | ---: | ---: | ---: |
| Complete virtual-base object | 0.810x | 23952/22228 | 0.731x | 66099/16865 |
| Move construction | 1.002x | 38336/37912 | 0.981x | 98510/98510 |
| Shared termination action | 0.978x | 40528/40848 | 1.010x | 170012/164040 |

Layout facts reduce the measured native wrapper from 14 to 6 instructions, with
repeatable runtime/compiler/text benefits. Move/landing normal-path instructions
are unchanged; no move speed claim. Cold sharing meets the retained-helper
requirement, saves 3.5% text and 2.2% compiler time, with a disclosed ~1% runtime
cost in the changed image. It adds no hot work; machine placement is a later
owner, not an extra PA32 gate. The C-linkage reducer had no correct baseline
executable; timed landing variants both use C++ linkage.

Common template-heavy memory/floating/EH/pruning at O0/O1/O3 have byte-identical
objects: compiler medians 0.992–1.011x, runtime 0.996–1.007x. O0 compiler component
1.000x, RSS 77544/77888 KiB, identical 34301-byte text, no executable entry.
All spreads/outliers remain, including 4.69x on identical pruning images. The
initial independent-input layout result (1.039x) is retained; the final input
chains results to expose load-to-return latency. Inherited arbitrary ratio,
zero-growth and runtime targets remain diagnostics under spec.md. Mandated
limits, correctness, coverage and measurements remain intact.

## Handoff ledger

| Boundary | Completed | Still required then |
| --- | --- | --- |
| Audit 210, `76d3fcb2` | Review and four ownership corrections | 41 failures |
| Implementation 211, `d666b628` | Private aggregates/evidence | 29 failures |
| Implementation 212, `a58a3a97` | Integer loops/unit budgets | 23 failures |
| Implementation 213, `5b5512b4` | Bounded memory/native clobbers | 17 failures |
| Audit 214, `40151904` | Review and loop comparison fix | 17 failures; source/debug/EH |
| Implementation 215, `1aa04ae6`; evidence `104846c6` | Scalar/phi and finite ranges | 11 failures; review pending |
| Implementation 216, `94eb1dae`, `2691df5a`, `2bace8a0`; evidence `7e04081b` | Calls/scalars/storage/EH closure | Six course and three source-debug failures; review pending |
| Implementation 217, `cc7d3c20`, `940d3396`, `c81197b0` | Source/ABI/lifecycle closure; proved oracle correction | Zero course failures; three source-debug failures and accumulated review |

Run `python3 student.tests/pa32/source_verify.py` at this records-only handoff.
Historical verifiers bind their own commits. Implementation does not advance
Last reviewed commit.
