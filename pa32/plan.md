# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: 76d3fcb24059d1557b6dd0e398cf3896931fcca9

Target: PA32 full-stage; implementation remains incomplete. [Audit 210](audit.md)
retains the accumulated review inventory. Implementation 212 entered at
`a7fe2af07e6d1a80942a10df46750830af021a2d`, clean, with 190/219 course cases.
Previous turn: progress (private-storage handoff); entry inspection found no
live build. Neither review marker advances during implementation.

Implementation 213 entered at `cec5b40ae0de687d217cc787e1cca342591a7c56`,
clean, with 196/219 course cases (23 failures). Previous turn: progress
(validated integer-loop handoff); no live process on entry. Initial group:
external memory facts, conditional addresses and adjacent noalias copies.
Owner: function-local canonical address/byte-range and available-value records;
data flow: typed LowIR identities and effects -> conservative alias proofs ->
bounded ordinary-edge intersections -> load reuse/conditional value phis/copies.
Use a linear work allowance and conservative reset on cycles, EH boundaries,
unknown effects and exhaustion; no global cache or IR serialization. Validate
alias overlap, mutable snapshots, volatile/atomic and EH barriers, object replay,
existing failures and checked A/A + ABBA compiler/RSS/runtime/text evidence.

## Design/spec alignment

Source -> canonical semantic facts -> shared typed LowIR -> optimization ->
text view or native MIR/direct ELF. O0 returns before analysis. O1 owns cheap
scalar/CFG/private-storage/call cleanup and proven effect-free loop deletion.
O2/O3 also propagate closed-world constants; O3 permits bounded full unrolling.
Text never transports facts between production phases.

| Owner | Legality/fallback | Work and growth limits |
| --- | --- | --- |
| Scalar/slots | Typed constants, same-type aliases, mutable snapshots; traps/volatile/effects retained | Each dirty propagation/rewrite <=16*(I+uses+1); no growth |
| Ordinary CFG | Sparse slot entry facts, completed dominance, scoped edge facts, phi-safe bypass | Promotion <=16*(I+O+B), phis <=I, phi operands <=2O; dominance <=32*(I+O+E+B); CSE/bypass <=16*(I+O+B), bypass operands <=2x |
| Calls | Direct graph/escape census, recursion and typed homes, admission/cold-body costs | Depth <=64, work <=32768/caller and <=32*(I+O+P+S+F+1)/unit; growth <=1536/caller (2048 single-use). Mandatory expansion retains 4194304/unit and 262144/caller |
| Private objects (211) | Single-definition origins; union/find copy partitions; exact nonoverlapping fields and snapshots. Escapes, volatile, unknown offsets/conflicting overlays retain homes | <=64 bytes/home, <=16 fields; two fixed invocations, O((I+O+S) alpha(S)); each reserves added IR <=8*(I+1), homes <=16S |
| Integer loops (212) | Linear entry/header/body/latch/exit, unique definitions, widened exact trip/overflow proof. Mutable carriers, EH, branching bodies, source-later entry definitions and unknown counts decline | One invocation, function-local indexes; candidate work <=16*(I+O+E+1), shared across failed walks; O3 <=4 trips, <=64 body clones/loop; reserve unsimplified snapshots/bindings too, <=256/function and min(4096,2*(I+1))/unit |
| Regions/roots/transport | Persistent handler stacks, monotonic no-unwind facts, complete root/escape census; debug/section/extent/ABI facts stay in LowIR | Retirement <=32*(I+O+B+1); exhaustion retains input; root census linear; one function-local MIR at a time |

I/O/B/E/P/S/F mean instructions/operands/blocks/edges/parameters/slots/functions.
All indexes, partitions, candidate vectors and mapping overlays have invocation
or function owners. Temporary operand/candidate buffers are reused; no global
cache or rendered-name semantic keys. Final spelling allocation is lazy.

Loop data flow: ordinary dominance + one def/use census -> affine induction,
normalized predicate and exact count -> effects/escaping values -> admission ->
transactional rewrite. Arbitrarily long pure loops use analytic final values,
never trip simulation. O3 snapshots parallel phi edges with fresh typed IDs,
including variadic ABI carriers; instructions keep their original locations.
Every executed volatile access, call and arithmetic operation stays ordered.
Zero-trip bodies are skipped without speculating loads or effects. The original
header keeps its identity, so exit-phi edges survive. Disjoint candidates share
budgets; replacement of a closed linear body preserves the remaining dominance
facts. No loop or whole-pipeline fixed point is introduced. One conditional
scalar/CFG cleanup consumes the changed IR and invalidates the local analyses.

Pipeline growth remains bounded by the existing two object splits and call
cloner, plus the loop reservation above; downstream promotion/bypass bounds
remain unchanged. Refusal precedes IR/value mutation. O0/ABI/backend policy is
unchanged; PA33 still owns allocation quality and DWARF is later work.

## Remaining implementation and handoff boundary

The 23 remaining course failures group by semantic owner (some source cases
also need memory improvements):

1. **External memory (5):** global/parameter/projected alias classes, load
   numbering over ordinary/EH joins, conditional addresses, adjacent noalias
   copies and diamond values.
2. **Pointer loops (6):** empty backward ranges and byte/word fills, including
   twin phis and final pointer publication. These need dynamic range/termination
   and alias proofs, plus dynamic fill lowering. Plain `index` does not supply
   a range fact (PA8 Memory and Addressing); a constant-trip integer proof cannot
   establish these properties for arbitrary pointers. These required outcomes
   remain implementation work, not waived or declared erroneous references.
3. **Calls (5):** exception-bearing contextual candidates, builtin facts,
   landing cleanup and parent admission after initializer-list accessor cleanup.
4. **Source identity (5):** lifecycle/declaration names, constructor/move and
   lambda facts. **Source debug:** three binding/start-span fixtures remain.
   The inherited O0 nested-try reducer `call-nested-cleanup.cpp` also remains.
5. **Edge facts/phi closure (2):** nonzero-underflow memory facts and conditional
   phi threading/cross-slot survivor promotion.

212 completes the integer-loop owner through zero/one/many trips, signed and
unsigned/inverted exits, narrow bounds, final exported values, parallel phis,
volatile/call/floating bodies, multiple disjoint loops, source lowering and
serialized replay. All failing course cases requiring that owner now pass.
Further nearby pointer cases require the distinct proofs above, not another
integer-loop sweep or larger clone budget. External memory and source identity
also require separate state owners. This is the concrete incomplete handoff
boundary; the assignment is not complete.

## Validation and performance evidence

Course: **196/219**, **23 failures**, from 190/219 and 29 failures: six existing
failures removed, none added, unchanged fixtures/comparison rules. Earlier PAs:
**5178/5178**. Through PA32: **5374/5397**, failing only PA32. File audit passes
with four inherited header warnings. Direct debug improves **4/5 -> 5/5**;
source debug remains **0/3**; normal/debug object replay **25/25** each. Ralph's
425 denominator is a different census; command exit status 2 is not a case count.

Explicit checks: 78 loop cases x four levels x three execution paths; 346
finite-domain interpreter cases x four levels; 154 long/infinite candidates;
100/500/1000-function scaling and unit/function/body reservation refusal;
variadic/debug/EH guards; inherited local/dataflow/call/object/audit reducers.
Both the inherited declaration/template trace and new demanded-loop trace pass
LowIR validation, two runtime inputs, MIR locations and direct/replayed ELF
identity at all levels. No reference or contract was changed.

[Evidence 212](../student.tests/pa32/evidence212/binding.json) binds final code,
inputs/flags, binaries, all **812 final** A/A + six-ABBA samples and the **112
exploratory** samples, compiler wall/RSS, runtime/text, disassembly and checks.
Median paired B/A (1600 kernels compiled; one checked kernel executed):

| Workload | Compiler | Peak RSS A/B KiB | Runtime | Object text A/B bytes |
| --- | ---: | ---: | ---: | ---: |
| Finite loop, O1 | 0.859x | 13148/11800 | 0.123x | 128000/28800 |
| Four-trip volatile loop, O3 | 1.152x | 16564/17828 | 0.452x | 153600/151481 |

Every runtime pair improves (0.122–0.123x; 0.438–0.484x). One checked execution
repays the full unroll compilation delta (0.208s saved vs 0.018s added). Native
kernel frames fall 16->0 bytes, text 80->18 and 96->83 bytes; backedges and
phi spills disappear. The O3 unit cap admits 163/1600 kernels, retaining the
rest conservatively. Common template-heavy memory/floating/EH/pruning objects
are byte-identical at O0/O1/O3: compiler median ratios 0.991–1.022x, runtime
0.995–1.001x. Compiler-source O0 control: 0.995x, RSS 77556/77564 KiB, identical
object, no executable entry. All spreads/outliers remain, including selfhost
0.765–1.243x paired noise and the O0 EH 0.531x pair.

The new diagnostic targets (compiler/RSS <=1.5x, text <=1.25x) are met. These
and inherited 2x/1.75x/zero-growth/10%-runtime targets are self-selected
measurements, not additional exit gates. [211 evidence](../student.tests/pa32/evidence211/binding.json)
and audit 210's scalar 2.337x compiler/0.876x runtime result remain preserved.
Stage-scoped acceptance still requires correctness, coverage, bounded work/
growth and measured benefit; it does not erase historical regressions.

## Handoff ledger

| Boundary | Completed implementation/evidence | Still required |
| --- | --- | --- |
| Audit 210, `76d3fcb2` | Accumulated audit and four corrections; markers retained | Then 41 course failures/debug closure |
| Implementation 211, `d666b628` | Private aggregate owner, 12 existing failures fixed, measured benefit | Then 29 course failures and four debug failures |
| Implementation 212, `a58a3a97` | Integer-loop owner, six course + one direct debug failures fixed, bounded cloning and measured runtime benefit | 23 course + three source debug failures; remaining owners above |

Independent review questions: audit accumulated private-object partition/escape
and snapshot proofs; new affine endpoint/overflow proof, parallel typed phi
snapshots, retained dominance across disjoint rewrites and pipeline reservations.
These review obligations are separate from unfinished implementation; neither
is waived. `python3 student.tests/pa32/loops_verify.py` verifies current bindings,
raw-summary arithmetic, unchanged contracts, failure-set reduction, review
markers and clean handoff. Historical verifiers stay bound to their own code.
