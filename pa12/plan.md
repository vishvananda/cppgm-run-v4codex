# PA12 implementation plan

Stage base commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Last reviewed commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Target: PA12 full-stage. Phase: implement; incomplete.

## Design/spec alignment

Extend the shared source graph, canonical TypeId/EntityId facts and typed LowIR.
No source/IR replay, reference delegation, new optimizer or later native gate.
The completed group owns canonical ref qualifiers, using-object ranking, deleted
member selection, pointer-reference materialization, delegation edges and union
variant/storage identities. The transfer model now records
implicit/defaulted declarations, deletion, triviality, exception facts and typed
field/base actions. Lowering consumes these once; class-value ABI and lifetime
destinations remain the next shared owner.

| Group / owner | Data flow and complexity | Validation / status |
| --- | --- | --- |
| Member declaration/selection | Canonical qualifier -> indexed declaration -> object/reference conversion; O(parameters + required candidate/base edges) | Focused ref-qualification, using, deletion and pointer-reference fixtures pass |
| Delegation/unions / construction | Selected constructor edges and variant/storage IDs -> existing actions; once per demanded edge/subobject | Delegation/cycles, variant initialization, injection and destructor-boundary fixtures pass |
| Special members / semantics | Per-class declaration/deletion/triviality state -> typed scalar/reference/subobject/unit/prefix actions -> demanded helpers; O(candidates + subobject edges) | Normal copy/move construction and assignment, fallback from deleted defaulted moves, union/deletion rules, base entries, noalias, bounded arrays pass. Remaining combined lifetime and class-value cases |
| Class ABI / lowering | Selected transfer + explicit destination -> value arguments/results; O(expressions + actions) | Remaining: direct/indirect ABI, named return slots, conditional values |
| Lifetime / actions and cleanup | Full-expression/scope identities -> shared cleanup suffixes; O(actions + CFG edges) | Remaining: local/static references, condition declarations, array/temporary controls |
| Conversions / overload engine | Indexed conversion functions -> recorded user/standard sequences -> calls | Remaining: scalar/pointer/class conversions and their combined ref-qualifier cases |
| Allocation / object model | Allocation choice and storage -> construction/destruction actions | Remaining: scalar/array new/delete and nonthrowing-null paths |

## Performance evidence

[Member evidence](performance.md) and [transfer evidence](transfer-performance.md)
retain frozen binaries, A/A and ABBA observations, latency/RSS, runtime/text and
work counters. Final common LowIR/native outputs are unchanged; 4x transfer
inputs use 4.124x time and 3.501x timed RSS, with exactly 4x transfer actions.
An optional scalar-only prefix fold slowed native execution and was removed;
final output matches the field-wise baseline. Required whole-object/storage
prefix shapes retain a documented supplied-backend runtime cost, not an extra
positive-runtime gate. O0 preparation is linear in required candidate/subobject
edges, with no body cloning; array expansion stays capped at eight elements.

## Handoff ledger

- Entry: clean stage base, terminal failure log; no live prior work handle.
  Previous goal-turn progress was unverified; authoritative inspection resumed work.
  Baseline **61/257**, 196 failures; earlier PAs **1327/1327**.
- `4e209677`: member qualification, delegation, unions and destructor boundaries.
  Checkpoint 85/257; failure-set audit found 26 old failures removed and two
  using-overload regressions. Initial measurements are preserved as historical.
- `55d15ba8`: correct using-owner ranking, pointer-reference output and deleted
  member/assignment selection. Final **92/257**, 165 failures: **31 original
  failures removed, no new failures**; comparison coverage is unchanged.
- Serial root reports are required for trustworthy counts: concurrent reports
  share a tally. The invalid concurrent totals were replaced by serial runs.
- `3da4de09`: four bit-field reference comparison sites corrected under the
  authorized exception; [proof and reducers](reference-corrections.md). All
  transfers, other instructions, inputs, sidecars and comparison rules retained.
- `73664568` and the bounded-prefix followup: **123/257**, 134 failures, **31 prior failures removed,
  no new failures** versus 92/257. Serial earlier report **1327/1327**; file
  audit passes with its existing advisory. Seventeen personal source checks and
  two LowIR retype checks pass. Fresh required logs: `/tmp/pa12-transfer-budget-stage.log`
  (exit 2), `/tmp/pa12-transfer-budget-prior.log` (exit 0). `git diff --check`
  passes; benchmarks are complete, with no live test/benchmark processes.
- Concrete remaining boundary: class prvalues still fall into aggregate/scalar
  paths. Passing/returning them requires an explicit destination/result ABI,
  selected transfer and lifetime identity together (direct/indirect calls,
  conditional results, named return slots, references). Transfer helpers are
  now available; those source values are not yet owned by that destination model.
- Ordinary bit-field initializer ordering is an independent contract conflict:
  PA11 `400-bit-field-constructor-member-init` and PA12
  `300-bit-field-copy-semantics` prescribe different orders for the identical
  `Bits` constructor. A general lowering change caused two earlier regressions
  and was removed. Preserve the earlier checks; no unproved reference edit or
  test-dependent compiler branch was introduced. Destructor cleanup boundaries
  and explicit widening in user bodies also remain separate from transfers.
- Handoff: the normal special-member transfer group is committed. The next
  ABI group needs class boundary facts plus destination/lifetime records; current
  `converted`, `call`, parameter prologues and return lowering all assume scalar
  results or aggregate slots. Changing only one of these consumers would emit
  invalid object loads or omit required copies/destruction. That coupled owner
  is the concrete boundary for this incomplete handoff; the full-stage goal
  remains active. Course coverage stays **257**, with **62 stage-base failures
  removed** overall and no new failing fixtures.
