# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: 76d3fcb24059d1557b6dd0e398cf3896931fcca9

Target: PA32 full-stage; implementation remains incomplete. Audit 210 reviewed
three accumulated checkpoints and its corrections; [audit.md](audit.md) retains
that inventory, findings and evidence. Implementation 211 entered at
`2e1238dc`, with 178/219 course cases. The preceding turn was progress (completed
checkpoint audit); entry process inspection found no live build to resume.

## Design/spec alignment and completed owners

Source -> shared typed LowIR -> optimizer -> text view or native MIR/direct ELF.
O0 returns before optimizer analysis. O1 owns local/scalar/ordinary CFG, bounded
calls and private aggregate homes. O2/O3 also propagate closed-world constants.
Higher levels still owe their remaining fixture objectives.

| Owner | Legality and conservative fallback | Work/growth limits |
| --- | --- | --- |
| Scalar/slots | Typed integer folding, same-type aliases, mutable snapshots; traps, volatile and effects retained | Dirty propagation/rewrite each <=16*(I+uses+1); no growth |
| Ordinary CFG | Sparse demanded slot entries, completed dominance, scoped edge facts, phi-safe bypass; conflicting/EH paths decline | Promotion <=16*(I+O+B), phis <=I, phi operands <=2O; dominance <=32*(I+O+E+B); CSE <=16*(I+O+B); bypass <=16*(I+O), phi operands <=2x |
| Calls | Direct graph/escape census, recursion checks, typed homes, explicit admission/cold-body costs | Depth <=64; <=32768 work/caller; <=32*(I+O+P+S+F+1)/unit; growth <=1536/caller (2048 single-use); reserve actual copy work before mutation |
| Private objects (211) | Single-definition address origins; complete-copy layout components; exact nonoverlapping typed fields; preserve padding, snapshots, full copy spans and debug locations. Escapes, volatile, unknown offsets, conflicting overlays and object-valued inputs retain homes | Objects <=64 bytes, <=16 fields; two fixed invocations; union/find + address users O((I+O+S) alpha(S)), bounded 64-byte partitions; added IR reserved <=8*(I+1)/function/invocation, new homes <=16S |
| Regions/roots | Persistent handler stacks, monotonic no-unwind facts/reverse callers; complete root/escape census | Retirement <=32*(I+O+B+1); inconsistent state/exhaustion retains input; root/argument census linear |
| Transport/native | Source/debug identity, sections/extents and ABI facts in LowIR; one function-local MIR at a time | Deterministic final ELF ordering; object replay retained; allocator remains PA24 policy |

I/O/B/E/P/S/F count instructions/operands/blocks/edges/parameters/slots/functions
at the owning pass. All indexes, byte partitions, flat pools and scope records
have explicit invocation/function owners. Object origins resolve once via users;
copy layouts use union/find, not repeated chain scans. Partition disagreement
invalidates the component; escape invalidates only the affected storage home.
Bulk floating/i1/i128 representations conservatively retain object storage.
New scalar homes feed the existing conversion/snapshot/SSA owners. First split
precedes call costing; second handles cloned homes and forwarded addresses.
There is still one optional call-expansion invocation and no global fixed point.
Unused homes retire after final scalar cleanup, removing obsolete native frames.

Pipeline growth remains linear: each split adds at most 8*(I+1) instructions
and 16S homes, separated by the unchanged bounded cloner; promotion and bypass
retain their independent bounds. Each synthesized instruction has <=2 operands.
Work refusal precedes mutation. Mandatory attribute expansion keeps its distinct
4,194,304/unit, 262,144/caller and depth-64 limits. No limit was relaxed.

## Remaining implementation and handoff boundary

1. **External memory:** alias classes for global/parameter/projected addresses,
   load numbering across ordinary and exceptional joins, conditional-address
   forwarding and adjacent noalias copy coalescing. Private local storage and
   its complete copies are implemented, including padding and branch merges.
2. **Calls/loops:** context-sensitive exception-bearing candidates, builtin and
   query facts, finite-trip/effect proofs, loop deletion/fill/hoisting and
   profitable O3 unrolling/versioning. The initializer-list source fixture still
   needs parent-call admission after nested accessor simplification; its object
   argument remains correctly materialized across the unexpanded call.
3. **Source/debug closure:** binding/start spans and lifecycle/declaration
   identities, plus the inherited O0 nested-try construction failure in
   `student.tests/pa32/call-nested-cleanup.cpp`. Required direct/source debug
   failures remain implementation work. Backend DWARF emission is later work.

211 closes the private-storage owner through object arguments/returns, nested
copy chains, conditional stores/zeroing, scalar promotion and late call costing.
Further related failing fixtures require external alias/effect state and EH
joins, or a new call/loop policy. Extending private-home equivalence to those
addresses would be unsound; extra whole-pipeline sweeps would not supply the
missing proofs. This is the concrete incomplete handoff boundary, not a claim
that the remaining assignment requirements are optional.

## Validation and performance evidence

Current course result: **190/219**, **29 failures**, down from 41; **12 original
failures removed, no new failures**, unchanged fixtures/comparison rules.
Earlier PAs: **5178/5178**. Through PA32: **5368/5397**, failing only PA32.
File audit passes with four inherited header warnings. Debug direct **4/5**,
source **0/3**, debug object replay **25/25**; primary replay **25/25**. These
unchanged debug failures remain required implementation. Ralph's 425 denominator
is a different census; exit code 2 is not a count of failing cases.

Explicit reducers: 43 object cases x four levels x three execution paths,
100/500/1000-home copy chains, transactional growth refusal, source EH/debug
replay, plus inherited local/dataflow/call/audit reducers. The demanded-template
trace again passes typed validation, ELF equality, two runtime inputs and MIR
locations at all four levels. No contract or reference output was changed.

[Evidence 211](../student.tests/pa32/evidence211/binding.json) binds frozen A/B
binaries, inputs/flags, all **588** A/A + six-ABBA observations, equivalent checked
outputs, compiler wall/RSS, runtime/text and validation logs. Median paired B/A:

| Workload, O1 | Compiler | Peak RSS A/B KiB | Runtime | Object text A/B bytes |
| --- | ---: | ---: | ---: | ---: |
| Private copy chains | 1.005x | 17976/20920 | 0.511x | 192000/144000 |
| Staged exports | 1.097x | 17916/19228 | 0.599x | 188428/159628 |

All six runtime pairs improve (ranges 0.511–0.513 and 0.598–0.601); one checked
run repays the full compilation delta, even though compilation contains 1200
kernels and execution uses one. The observed native kernel frame shrinks
80->32 bytes and text 160->120 bytes; both bulk copies disappear. Shared
memory/floating/exceptions/pruning have byte-identical objects at O0 and O1,
compiler medians within noise, and runtime medians 0.994–1.003x. Compiler-source
O0 control: 0.998x, RSS 77284/77552 KiB, identical object; no executable entry.
Raw outliers/spreads are retained, including the 2.023x O0 exception compile pair.

211's diagnostic targets (compiler/RSS <=1.5x, no text growth) are satisfied.
Inherited 2x compiler, 1.75x RSS, zero/1.5x growth and historical 10% runtime
thresholds remain self-selected diagnostics, not extra exit gates. Audit 210's
cumulative scalar 2.337x compiler/0.876x runtime observations remain preserved;
stage-scoped acceptance does not erase them. Bounded work/growth, correctness,
coverage, measured benefit and disclosed regressions remain required.

## Handoff ledger

| Boundary | Completed implementation/evidence | Still required |
| --- | --- | --- |
| Audit 210, `76d3fcb2` | Whole accumulated checkpoint audit and four owner corrections; markers preserved | Then 41 course failures and debug closure |
| Implementation 211, `d666b628` | Private aggregate owner; 12 existing failures fixed; checked bytes/snapshots/EH, bounded growth and repeatable runtime benefit | 29 course failures in the three owner groups above; unchanged debug failures |

Independent review questions: audit the new partition/escape proof, reservation
accounting and interactions with mutable/EH snapshots across the accumulated
implementation range. These are review obligations, separate from the explicit
unfinished implementations; neither is waived. The last-reviewed marker does
not advance during implementation. `python3 student.tests/pa32/objects_verify.py`
checks the current handoff bindings, summaries, unchanged contracts, failure-set
reduction, markers and clean tree. Historical audit evidence stays bound to its
reviewed code, not to this later implementation.
