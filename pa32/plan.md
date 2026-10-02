# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: 401519044a2c28130d4085b3bc7c3d0411b5dc8f

Target: **PA32 full-stage**, still incomplete: **208/219**, **11 failures**.
Implementation 215 entered clean at `f4f075b8` with 202/219 and 17 failures.
The previous goal turn completed [Audit 214](audit.md) (progress); process
inspection found no running job. Both review markers remain unchanged.

This handoff closes partial slot/diamond cleanup, finite odd-stride pointer
walks and guarded contiguous fills, including aliasing, zero trips, exported
values, exit phis, ABI/debug transport and measured profitability. Code is in
`0f370e3f` and `7dd38d40`; bound validation/evidence is in `3592a12f`.

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

## Remaining implementation and independent review

1. **Pointer congruence (1 course failure):** twin backward eight-byte induction
   with independent pointer arguments. Their modulo-eight residues may differ;
   plain `index` supplies no matching-residue fact. The required envelope is
   still unmet. Establish a contract guarantee or a proved reference correction
   before further deletion; the current finite-byte proof cannot justify it.
2. **Contextual calls/EH (5):** exception-bearing candidates, builtin facts,
   landing cleanup and parent admission after initializer-list accessor cleanup.
3. **Source/ABI/debug identity (5):** lifecycle/declaration names, constructor,
   move and lambda facts. Three source-debug failures and the inherited O0
   `call-nested-cleanup.cpp` reducer remain required work with the EH owner.

These are unfinished implementation requirements. Independent audit must also
review the accumulated changes since `40151904`, including the modulo-address
proof, exit-phi edge repair, covered-load retirement, helper ABI and performance
acceptance. The even-stride contract question needs independent resolution;
listing it here waives neither implementation nor review.

The concrete handoff boundary is the completed scalar/phi and finite-byte-range
owner. Further related deletion lacks the needed congruence fact. The other
failures require contextual EH and source semantic/ABI producers, which cannot
be repaired by more range rewrites. The scope was extended through zero-trip
profitability, arbitrary exit phis, native clobbers and source/template replay
before this boundary.

## Validation and stage-scoped performance acceptance

- Required earlier report: **5178/5178**. `make test-pa32`: **208/219**;
  through PA32: **5386/5397**. Six original failures removed, no new failure,
  identical contract tree, references and comparison rules.
- File audit passes with four inherited header warnings. Direct debug **5/5**,
  source debug **0/3**; normal and debug object replay **25/25** each.
- All inherited personal reducers and bounds checks pass. New tests cover 19
  control cases, 41 pointer cases and 440 fill assertions at four levels through
  direct, replay and native execution, plus 64 live exit phis, GP/FP clobbers,
  malformed helper ABI, source/template debug replay and work scaling.

[Evidence 215](../student.tests/pa32/evidence215/README.md) binds source, commands,
binaries, inputs and all observations. A/A calibration plus six ABBA blocks,
CPU 2, separately measure compilation and execution. There are **1,288 bound
observations**, of which **1,092 support current acceptance**. Final compiler
measurements use A/E; runtime A/D observations are reused only after checking
byte-identical final A/E objects and executables for every affected workload.
Paired final/baseline medians at O1:

| Workload | Compiler | Peak RSS KiB A/E | Runtime | Object text bytes A/E |
| --- | ---: | ---: | ---: | ---: |
| Reference fill, 1024 bytes | 0.962x | 36236/32464 | 0.009x | 440000/420009 |
| Reference fill, 8 bytes | 0.963x | 36340/32472 | 0.798x | 440000/420009 |
| Empty reference range | 0.958x | 36308/32344 | 0.949x | 440000/420009 |
| Zero fill, 1024 bytes | 0.914x | 34440/31160 | 0.043x | 405000/355009 |
| Finite pointer walks | 0.766x | 29616/26748 | 0.039x | 360000/70000 |
| Boolean diamonds | 0.849x | 34660/34984 | 0.891x | 465000/260000 |
| Partial slot facts | 1.079x | 31760/31648 | 0.999x | 400000/400000 |

The rejected policy's 1.453x empty-range slowdown remains recorded; caller
zero guards resolve it. Partial promotion is a mandated IR outcome with no
claimed runtime gain: native text is unchanged, while the current allocator
uses a 32-byte instead of 16-byte frame for separate original/phi homes. Its
bounded 8% compiler cost and later allocator constraint are disclosed.

All twelve common template-heavy memory/floating/EH/pruning objects remain
byte-identical at O0/O1/O3; compiler medians are 0.987–1.015x. The O0 compiler
component is 0.987x, 77476/77508 KiB, identical 34275-byte text; runtime is
inapplicable because it has no executable entry. All spreads and scheduling
outliers remain recorded. The first contended validation attempt also remains
recorded; the completed unchanged-code run passes all earlier tests.

[Audit 214 evidence](../student.tests/pa32/evidence214/binding.json) preserves
1,260 audit observations and 4,536 verified historical observations. Inherited
2x/1.75x/zero-growth/10%-runtime and later 1.5x/1.05x/1.25x ratio targets remain
**diagnostics, not extra gates**, under spec.md. No mandated envelope, work or
growth budget, correctness requirement or coverage is weakened. PA33 allocation
and later self-hosting requirements add no PA32 gate.

## Handoff ledger

| Boundary | Completed work | Still required |
| --- | --- | --- |
| Audit 210, `76d3fcb2` | Accumulated review and four ownership corrections | Then 41 course failures |
| Implementation 211, `d666b628` | Private aggregate owner and measured benefit | Then 29 failures |
| Implementation 212, `a58a3a97` | Integer loops, unit budgets and snapshots | Then 23 failures |
| Implementation 213, `5b5512b4` | Bounded memory, actual clobbers and profitable admission | Then 17 failures |
| Audit 214, `40151904` | Accumulated architecture/performance review and comparison correction | Same 17 failures; source/debug/EH closure |
| Implementation 215, `7dd38d40`; evidence `3592a12f` | Scalar/phi and finite pointer-range group; six failures removed; zero-trip cost corrected; ABI/debug/performance closure | 11 course failures and source-debug/EH work above; independent audit still due |

Run `python3 student.tests/pa32/ranges_verify.py` at this records-only boundary.
Historical verifiers remain bound to their own handoffs. This returns a validated
implementation checkpoint to Ralph; it does not certify the assignment or
advance the last-reviewed marker.
