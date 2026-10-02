# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: 76d3fcb24059d1557b6dd0e398cf3896931fcca9

Target: PA32 full-stage. Checkpoint audit 210 is complete; PA32 implementation
is still incomplete. The reviewed range includes all three accumulated
checkpoints and the audit fixes, not just the last handoff. See
[audit.md](audit.md) for the commit inventory, architecture traces, findings,
performance tables and single audit ledger row.

## Completed owners and bounds

Source -> shared typed LowIR -> optimizer -> text view or native MIR/direct ELF.
O0 returns before optimizer analysis. O1 owns local/scalar/ordinary CFG and
bounded calls; O2/O3 also propagate closed-world constant arguments. Level
policy does not substitute for the remaining fixture objectives.

| Owner | Proof and fallback | Work and growth limits |
| --- | --- | --- |
| Scalar/slots | Typed integer folding, same-type aliases, mutable snapshots, traps/volatile/effects retained | Dirty def/use propagation and rewriting each <=16*(I+uses+1); no instruction growth |
| Ordinary CFG | Sparse demanded slot entries; completed dominance; scoped edge facts; phi-safe bypass; exceptional/conflicting paths decline | Promotion <=16*(I+O+B), phis <=I, added phi operands <=2O. Dominance <=32*(I+O+E+B); CSE <=16*(I+O+B); bypass <=16*(I+O), phi operands <=2x |
| Calls | Direct graph/escape census, recursion checks, typed argument/result homes, explicit admission and cold-body costs | Depth <=64; <=32768 charged work/caller; <=32*(I+O+P+S+F+1)/unit; body growth <=1536/caller (2048 single-use); actual copy work reserved before mutation |
| Regions/roots | Persistent handler stacks, monotonic no-unwind facts and reverse caller edges; complete root/escape census | Retirement <=32*(I+O+B+1); inconsistent state/exhaustion retains valid input. Root and argument census linear in consumed pools |
| Transport/native | Debug/source identity, sections/extents and ABI metadata are LowIR facts; one function-local MIR at a time | Deterministic final ELF ordering; both object replay lanes retained; allocator remains PA24 policy |

I/O/B/E/P/S/F denote instructions/operands/blocks/edges/parameters/slots/functions
at the owning pass. Bodies, flat ID indexes, pools and scope-undo records have
explicit invocation/function owners. Optional cloning runs once. The fixed
pipeline contains bounded scalar and structural sweeps, not a whole-unit fixed
point. All original pools plus admitted clones are linear in input size;
promotion adds at most I instructions/2O operands, and bypass has its separate
2x phi-operand bound. Mandatory attribute expansion retains its separate
4,194,304/unit and 262,144/caller work limits and depth 64 at object preparation.
These enforceable limits were not relaxed by the audit.

Audit 210 closed four interactions: width-sensitive edge/diamond proofs;
shared scalar/edge/call replacement rules for implicit switch and variadic/
by-address carriers; floating store conversion during promotion; and definition
locations on retired declarations. The explicit 119-case reducer and demanded
template/ELF trace cover these corrections. No contract/reference changes.

## Remaining implementation, broadly grouped

1. Memory and aggregate proofs: projected-object alias/lifetime state, scalar
   replacement, copy/load forwarding, exceptional memory state, and subsequent
   call simplification. Independent object homes remain correct; eliminating
   their copies requires these proofs.
2. Context-sensitive calls and loops: guarded exception-bearing candidates,
   constant/builtin query facts, trip/effect proofs, deletion/fill/hoisting and
   profitable O3 unrolling/versioning. Extra scalar/inlining rounds cannot
   replace these owners.
3. Source/debug closure: binding/start spans, lifecycle/declaration identities,
   and the inherited O0 nested-try construction failure in
   `student.tests/pa32/call-nested-cleanup.cpp`. The required direct/source debug
   failures remain implementation work. MIR locations are transported; the
   inherited backend has no DWARF line-section emitter.

Retain cohesive owner groups through legality, profitability and validation.
The three distinct checkpoint groups were reasonable, but their small debug,
snapshot, admission and repeated-analysis followups fragmented avoidably across
handoffs. Complete those interactions before the next evidence handoff.

## Acceptance and evidence

Current root course result: **178/219**, **41 failures**, exactly the entry failure
set and coverage. Earlier PAs: **5178/5178**. File audit: pass, four inherited
header warnings. Required debug target: direct **4/5**; independently completed
source **0/3**, debug object replay **25/25**. Primary nodebug replay **25/25**.
Ralph's supplied 178/425 census is a different denominator; `2` is the root
make exit code, not two failing fixtures. This is no PA33 advancement.

[Evidence binding](../student.tests/pa32/evidence210/binding.json) retains frozen
binaries/inputs/flags, A/A plus six ABBA blocks, compiler wall/RSS, checked
runtime/text, all samples and log hashes. 210 verified **4,844** inherited and
**1,148** new observations. Current scalar O1/O0: compiler **2.337x**, RSS
**10,332/15,388 KiB**, runtime **0.876x**, object text **170,400/109,800 bytes**.
All six runtime pairs improve; about eight measured executions amortize the
whole 70.84 ms compilation delta. Same-level audit controls at O0 and O1 have
identical objects and compiler medians within noise of entry.

The inherited 2x compiler, 1.75x RSS, zero/1.5x text-growth and historical 10%
runtime thresholds are **self-selected diagnostic targets**, not additional
course/checkpoint exit gates. The cumulative scalar 2x miss is retained and
explained in the audit; no threshold was silently raised and no sample removed.
The spec's stage-scoped acceptance governs the inherited plans: required
correctness/IR envelopes/coverage, bounded work/growth, equivalent checked
outputs, repeatable affected-workload benefit and disclosed regressions remain.
The earlier unprofitable call/loop policies stay disabled. PA33 allocation,
whole-compiler self-hosting and DWARF emission add no PA32 checkpoint gates.

Explicit verification:
`python3 student.tests/pa32/verify_audit.py` checks current source/artifact
bindings, historical source tips, every paired summary, unchanged contracts and
failure set, the reviewed code marker and clean records-only boundary.
