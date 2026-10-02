# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: 76d3fcb24059d1557b6dd0e398cf3896931fcca9

Target: PA32 full-stage; implementation remains incomplete. [Audit 210](audit.md)
retains the accumulated review inventory. Implementation 213 entered at
`cec5b40ae0de687d217cc787e1cca342591a7c56`, clean, with 196/219 course cases.
Previous turn: progress (validated integer-loop handoff); no live process on
entry. Neither review marker advances during implementation.

## Design/spec alignment

Source -> canonical semantic facts -> shared typed LowIR -> optimization ->
text view or native MIR/direct ELF. O0 returns before optimizer analysis. O1
owns bounded scalar/CFG/storage/memory/call cleanup and proven effect-free loop
deletion. O2/O3 also propagate closed-world constants; O3 permits bounded full
unrolling. Text never transports facts between production phases.

| Owner | Legality/fallback | Work and growth limits |
| --- | --- | --- |
| Scalar/slots | Typed constants, same-type aliases, mutable snapshots; traps/volatile/effects retained | Each dirty propagation/rewrite <=16*(I+uses+1); no growth |
| Ordinary CFG | Sparse slot entry facts, completed dominance, scoped edge facts, phi-safe bypass | Promotion <=16*(I+O+B), phis <=I, phi operands <=2O; dominance <=32*(I+O+E+B); CSE/bypass <=16*(I+O+B), bypass operands <=2x |
| Calls | Direct graph/escape census, recursion and typed homes, admission/cold-body costs | Depth <=64, work <=32768/caller and <=32*(I+O+P+S+F+1)/unit; growth <=1536/caller (2048 single-use). Mandatory expansion retains 4194304/unit and 262144/caller |
| Private objects (211) | Single-definition origins; union/find byte partitions; exact fields and snapshots; escapes/volatile/unknown overlays retain homes | <=64 bytes/home, <=16 fields; two fixed invocations, O((I+O+S) alpha(S)); each reserves added IR <=8*(I+1), homes <=16S |
| Integer loops (212) | Closed linear loops, widened exact trip/overflow proof; unknown counts/EH/mutable carriers decline | One invocation; candidate work <=16*(I+O+E+1); O3 <=4 trips, <=64 body clones/loop, unsimplified reservations <=256/function and min(4096,2*(I+1))/unit |
| Memory values (213) | Canonical addresses/byte ranges, immutable snapshots, intersected ordinary predecessor facts; unknown predecessors/handlers reset; explicit effects invalidate | One invocation; 32 cells/state; shared <=128*(I+O+E+1) allowance plus linear census and bounded dominance; <=16 recent diamonds, <=64 comparisons/proof; <=16 copy pieces and <=128 bytes; no IR/operand growth |
| Regions/roots/transport | Persistent handler stacks, monotonic no-unwind facts, complete root/escape census; durable debug/section/extent/ABI facts | Retirement <=32*(I+O+B+1); exhaustion retains input; root census linear; one function-local MIR at a time |

I/O/B/E/P/S/F mean instructions/operands/blocks/edges/parameters/slots/functions.
Indexes, address records, state slices and proof buffers have invocation or
function owners and release there. No global cache or rendered-name key.

Memory data flow: typed SSA definition/use census -> address identity and
within-object provenance -> bounded RPO memory states -> reuse, conditional
value phi or adjacent object copy -> one conditional scalar/storage cleanup.
Backedges reset facts; no fixed point or speculative load is introduced.
Functions with calls in cycles reuse the pipeline's existing conservative
admission: new memory analysis is skipped there, retaining ordinary local slot
forwarding. Measured local as well as cross-block reuse prolonged hot values.
CSE numbers commutative operands without forcing a live invariant ahead of a
dying accumulator. One use census preserves that profitable operand order.
Byte-overlap checks use modular target addresses. Plain pointer arithmetic
carries equality, not disjoint-object or readonly-region evidence. Only pairs
of annotated noalias parameters are disjoint. Imported, weak and explicitly
object-named globals conservatively may share ELF storage. Mutable operands,
width conversion, volatile/atomic accesses and unknown effects retain snapshots.
EH registrations, calls that may unwind and handler roots delimit memory states.

Repeated pure diamonds require a dominating completed result, equivalent
conditions/arms and identical memory epochs. Retiring a matched load requires
an exclusive def/use path; shared expressions remain live. Conditional-address
forwarding admits private homes of incoming scalars/literals: external or
instruction-produced values otherwise prolong spill lifetimes. Declined choices
keep their loads. Adjacent integer/pointer transfers require disjoint complete
spans; no floating/I1 representation assumptions. Bulk selection and encoding
share the direct-copy size policy, so only actual clobbers affect carriers.
Small copies without alignment evidence keep scalar chunks to avoid widening
reads across recent scalar stores; aligned aggregates retain vector encoding.

A reused load identity also feeds the scoped unsigned-decrement proof:
`(x-1) >=u x` iff x is zero in the same width. Calls/stores that kill memory
facts prevent transferring the earlier nonzero fact to a new load identity.
Pipeline growth reservations from 211/212 are unchanged; 213 adds no IR. PA33
still owns general allocation quality, including remaining value-phi spills.

## Remaining implementation and handoff boundary

The **17 remaining course failures** belong to these owners:

1. **Pointer loops (6):** empty backward ranges and byte/word fills, twin phis,
   final pointer publication and exit phis. They require dynamic termination,
   range/alias proofs and dynamic fill lowering. Plain `index` supplies no range
   promise (PA8 Memory and Addressing). Integer trip proofs do not establish
   these facts. All outcomes remain required implementation.
2. **Calls (5):** exception-bearing contextual candidates, builtin facts,
   landing cleanup and parent admission after initializer-list accessor cleanup.
3. **Source identity (5):** lifecycle/declaration names, constructor/move and
   lambda facts. Copy/move bulk transfer improves in 213; their naming/lifecycle
   failures remain. Three source debug fixtures and the inherited O0 nested-try
   reducer `call-nested-cleanup.cpp` also remain.
4. **Phi closure (1):** conditional phi threading and partial cross-slot
   promotion in the presence of an unresolved initial load.

213 completes all five external-memory course failures and the memory-backed
nonzero predicate, with conservative profitability, alias and effect guards.
Related source copy/move transfers now use the same bounded copy owner. The
remaining cases need pointer range/termination proofs, exception-aware call
admission, semantic declaration/debug identity, or partial SSA/CFG threading.
Those facts cannot be obtained by extending available-memory intersections or
raising their caps. This is the concrete incomplete implementation boundary;
PA32 itself is not complete.

## Validation and performance evidence

Course: **202/219**, **17 failures**, from 196/219 and 23 failures: six
existing failures removed, none added, unchanged fixtures/comparison rules.
Earlier PAs: **5178/5178**. Through PA32: **5380/5397**, failing only PA32.
File audit passes with four inherited header warnings. Direct debug **5/5**;
source debug remains **0/3**; normal/debug object replay **25/25** each. Ralph's
425 denominator is a different census; the root course count remains 219.

Explicit checks include 90 checked memory cases x four levels x three execution
paths; 32 bulk-size/alignment/carrier cases x four levels x three paths; 2136
unsigned-domain comparisons x four levels; 100/500/1000-function exact work
scaling; dense-CFG budget exhaustion; ELF storage aliases; inherited local,
dataflow, object, call, loop and audit reducers. Source/template memory, loop
and declaration traces pass LowIR validation, two runtime inputs, debug MIR,
and identical direct/replayed ELF at every level. No contract/reference changes.

[Evidence 213](../student.tests/pa32/evidence213/binding.json) binds final code
`5b5512b4`, inputs/flags/binaries, all **3024** A/A + six-ABBA observations
(**1092 final**, 1932 earlier), and **48** short diagnostic observations.
Compilation and checked execution were measured separately, pinned to CPU 2.
Each affected compile contains 1800 kernels; runtime repeatedly calls one with
runtime-varying data. Median paired B/A, final O1:

| Workload | Compiler | Peak RSS A/B KiB | Runtime | Object text A/B bytes |
| --- | ---: | ---: | ---: | ---: |
| Repeated loads | 1.016x | 32904/28428 | 0.581x | 111600/117000 |
| Private scalar choice | 0.938x | 18796/15092 | 0.980x | 122400/127800 |
| Adjacent copies | 0.877x | 17648/15748 | 0.999x | 66600/66600 |
| Repeated diamonds | 0.836x | 29432/25800 | 0.930x | 340200/207000 |

All final load/diamond runtime pairs improve (0.577–0.587x; 0.775–0.946x).
One checked load execution repays compilation of all 1800 kernels. Longer
private-choice pairs improve 0.972–0.987x; copies remain at parity
(0.995–1.003x) while compilation improves. Declined external and loaded-value
choices produce byte-identical objects, runtime medians 0.999/1.001x, compiler
1.042/1.031x. Their attempted bounded analysis is the disclosed cost.

All twelve common template-heavy memory/floating/EH/pruning objects are
byte-identical at O0/O1/O3; compiler medians 0.988–1.006x, runtime
0.993–1.004x. Compiler-component O0 control: paired 1.009x, peak RSS
77584/77524 KiB, text 34322->34275 bytes; no executable entry. All spreads and
outliers remain, including common O0 compiler 0.534x and O1 memory runtime
0.767x pairs; unchanged object hashes prevent treating noise as code gains.

Exploratory loaded-value phis regressed 1.25–1.29x and widened copies 1.71x;
those policies were corrected. Call-cycle reuse regressed the common memory
kernel 1.053x (a partial fix 1.112x); final admission restores the exact original
object. Canonical operand reordering initially grew the load kernel 62->104
bytes; preserving the dying accumulator reduces it to 65 bytes and improves
runtime further. The initial zero-text-growth diagnostic is replaced by an
explicit <=1.05x target: the 3-byte load/private-choice growth buys measured
runtime improvement within the stage's bounded policy. Both original misses
and all revised measurements remain; this is not a fixture or coverage waiver.

Historical evidence remains preserved: [211](../student.tests/pa32/evidence211/binding.json),
[212](../student.tests/pa32/evidence212/binding.json), and audit 210's scalar
2.337x compiler/0.876x runtime measurement. Inherited 2x/1.75x/zero-growth/
10%-runtime targets and 213's compiler/RSS <=1.5x, text <=1.05x are self-selected
diagnostic targets, not extra exit gates. Mandated fixture bounds, correctness,
coverage, bounded work/growth and measured benefit remain required. Historical
misses and exploratory regressions remain in the evidence, not erased.

## Handoff ledger

| Boundary | Completed implementation/evidence | Still required |
| --- | --- | --- |
| Audit 210, `76d3fcb2` | Accumulated audit and four corrections; markers retained | Then 41 course failures/debug closure |
| Implementation 211, `d666b628` | Private aggregate owner; 12 existing failures fixed; measured benefit | Then 29 course failures and four debug failures |
| Implementation 212, `a58a3a97` | Integer-loop owner; six course + one direct debug failures fixed; bounded cloning and measured benefit | Then 23 course + three source debug failures |
| Implementation 213, `5b5512b4` | Memory owner, six existing failures fixed; exact byte/alias/epoch proofs and measured profitability | 17 course + three source debug failures; remaining owners above |

Independent review questions: accumulated private-object partition/escape and
snapshot proofs; affine endpoint/overflow proof and loop reservations; new
memory intersections, byte/provenance/ELF alias exclusions, diamond exclusive
use paths, carrier clobbers and pipeline budgets. These review obligations are
separate from unfinished implementation; neither is waived. Historical verifiers
remain bound to their own code. The 213 verifier checks current bindings,
raw-summary arithmetic, unchanged contracts, reduced failures, review markers
and the committed clean handoff.
