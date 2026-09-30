# PA25 implementation140

Stage base commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe
Last reviewed commit: 4530fe939e95a45ff6a46d5e53c76807b5bdd352

Target: **PA25 full-stage**. Phase: **implementation handoff**.
Entry HEAD: `4ecb9b413ab864cc1a43b2636ca78e67f6ce8ef8`, **86/101**.
Final: **101/101**; original 15 failures fixed, no new failures or coverage change.
Preserve [audit138](audit.md); this handoff returns control for independent audit.

## Design/spec alignment

Production remains source -> canonical semantic facts -> typed LowIR ->
function-local MIR -> native object -> indexed linker -> ELF. PA25's private
object version is now 3; PA26/27 still own host-compatible objects and metadata.

| Completed owner | Data flow and complexity |
|---|---|
| Earlier driver/class work | Separate/direct/mixed source linking retains indexed definition/fixup demand, typed runtime roles, public/unambiguous RTTI traversal and declaration-owned static initialization. Field ordering is O(fields log fields); emission is linear in bytes. |
| Source exception semantics | Original function-try syntax and handler scopes flow through template binding and lifetime lowering. Constructor initializers and destructor work are protected; completed subobjects unwind before handlers. Constructor handler return is diagnosed; destructor handler return and implicit rethrow retain correct lifetime ownership. |
| Native EH | One pass indexes block-leading clauses; function-owned typed selectors consume them directly. Work is O(blocks + markers). 96-byte registrations preserve state and retain active cleanup until eh.end/resume. Branch label ownership is validated. |
| Exception runtime | Finite typed LowIR support is demanded on retained symbol edges. An 80-byte payload header owns nested catch/rethrow state and destruction/freeing. Matching traverses hierarchy paths with O(depth) stack; null/base/member-pointer conversions use source representations. bad_cast, bad_typeid and bad_alloc share this protocol. Support IR dies after native construction. |
| Floating evaluation | Existing instruction fields carry F80 evaluation with declared F32/F64 storage; typed reader/writer/MIR/encoding preserve the fact. Two fixed-width tests recognize exact power-of-two multiplication, selecting shorter SSE encoding; unknown operands retain F80. Zero new allocations/nodes/frame bytes and zero text-growth budget. |

[Implementation140](../student.tests/pa25/implementation140.md) records state
ownership, legality, fallback and source-to-ELF traces. No host/reference compiler
implements source behavior. No fixtures, references or comparison rules changed.

## Validation and performance

- `make test-pa25`: **101/101**.
- Required prior-through24: **4152/4152**; root through25: **4253/4253**.
- File audit passes with four inherited header warnings; whitespace clean.
- Explicit controls: **74 exception**, **67 class**, **61 driver**, **19 scalar**
  plus 96 wide operand pairs, **122 audit138**, **49 statement**; five reduced
  source controls, valid/invalid deferred-body checks, 120,120 float/double scale
  comparisons, typed runtime/production LowIR roundtrips, MIR and native traces.
- [Validation140](../student.tests/pa25/validation140.json) pins commands,
  statuses, logs, binaries, inventories and exact fixed failure set.
- [Performance140](../student.tests/pa25/performance140.md) preserves **952**
  A/A and ABBA observations. Final template compile median **0.26196 s**, peak
  **15248 KiB**; final/entry ratio **1.042 [0.952, 1.111]**, no compiler speedup
  claim. Exact scaling improves the correct floating runtime by **17.3%**
  (ratio **0.827 [0.824, 0.838]**) and reduces text **377 -> 362** bytes.
  Final/entry floating runtime cost is **4.9% [2.8%, 11.6%]**, text **340 -> 362**;
  remaining general-operation precision traffic is required semantic work.
  New class/cast/allocation/exception baselines include latency, RSS, runtime
  and text. All raw observations, including timing variation, are retained.
- Inherited 15% timing/RSS targets remain diagnostics under spec section 9.
  Correctness, coverage, mandatory native bounds and bounded compiler work
  remain required. The optional scaling selection meets its explicit work and
  zero-growth budgets with repeatable measured benefit.

## Remaining groups and handoff ledger

**Unfinished implementation:** none identified within the completed PA25 groups.
The inherited unused-dependent-local item is closed by the cited C++11 proof and
valid deferred-body control in [deferred-member-proof140](../student.tests/pa25/deferred-member-proof140.md).
The floating contract is implemented; [rounding136](../student.tests/pa25/rounding136.md)
and its unchanged reference evidence remain preserved.

**Independent review:** whole-stage audit must still assess accumulated source
semantics, canonical demand, EH payload/cleanup transitions, RTTI access and
ambiguity, static destination identity, evaluation precision and performance
proofs. These are review questions, not waived requirements or a certification.

| Boundary | Evidence / next owner |
|---|---|
| audit138 | Preserved review marker; 74/101; audit.md and validation138.json. |
| implementation139 | Completed nonthrowing class execution and static polymorphic initialization; 74/101 -> 86/101; validation139/performance139 preserved. |
| implementation140 | Completed source exception ownership/matching/function-try execution and source floating precision; 86/101 -> 101/101. Expanded through class failure services, nullptr/member-pointer conversions and exact scaling. Required prior/current/root/file checks pass. No implementation failure is deferred; Ralph owns the full independent stage audit before advancement. |
