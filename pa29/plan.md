# PA29 compact plan — implementation191 handoff

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `2df00585bd10d4e2e068934394dffc8adb0a47ed`.
Last reviewed commit: `5aaf16d15f8e50925c0b75a5485893a958501b85`.
Entry HEAD: `32c1f5f43cde4e09f623f0e5d606a51d170e8203`, clean, **398/403**.
Implementation tip: `f542b3768ea22d35a2a2e18b87f5169472dbdf20`.
The record commit follows this tip without implementation changes. Review markers
are preserved; this handoff returns control to Ralph, not stage certification.

## Design/spec alignment

[Implementation191](implementation191.md) completes the vector/intrinsic owner:
retained type-operand syntax → canonical queries/deduction and recorded
conversions → constant/static/runtime values → typed LowIR → native ABI/object.
It adds comparison masks, lane conversion/reduction and representation bit casts;
packed bool vectors and GNU/Clang vector ABI identities survive every adapter.
ABI vendor-expression nodes preserve typed operands and have reader/writer checks.
Fixed and dependent template initializers and constructor members reach the same
initializer-plan owner. No parser replay, fixture recognition, production host
compiler delegation or hidden textual phase transport is introduced.

Facts live in the TU with canonical keys and existing completion invalidation.
Function temporaries/MIR have bounded lifetimes. Per-operation lowering emits at
most eight lane bodies or one counted loop; wider work/storage follows required
lanes. Constant evaluation charges the existing evaluator. Related array/copy,
side-effect, static storage, template-definition, member and ABI defects found
while extending this owner are repaired. The [prior audit](audit.md), earlier
ledgers and [three inherited reference proofs](reference-corrections189.md) remain;
this turn changes no contract fixture, reference or harness.

## Validation and performance

[Final checks](../student.tests/pa29/evidence191/validation.json): `make test-pa29`
**399/403** (exit 2); PA1–28 **4538/4538** (exit 0); root through PA29
**4937/4941** (exit 2), only the four retained PA29 failures. File audit passes
with four inherited header warnings. Explicit personal controls pass **191/191
commands**, covering O0/O2, checked runtime, rejection/template cases, source
LowIR/roundtrip/native MIR, ABI fact serialization and ELF/unwind inspection.
[Coverage](../student.tests/pa29/evidence191/coverage.json) preserves all **403
inputs and 1,707 contract/harness paths**. [Progress](../student.tests/pa29/evidence191/stage-delta.json)
is **5→4 failures**, no new failures or reduced coverage. Source/binary hashes
bind the checks to the committed implementation.

[Performance191](performance191.md) retains **544 observations plus 32 launchers**,
including the pre-repair observations. Final evidence has A/A and six ABBA blocks
on four equivalent pairs plus six corrected-only owner scaling controls. All
four common object/executable pairs are byte-identical; every paired timing range
crosses unity. Compiler RSS increases 88–348 KiB on common workloads. Demand work
scales with demanded facts; 16/64/256-lane controls keep 5,711 LowIR instructions
constant. Compiler latency/RSS, runtime/text, raw spreads and checked outputs are
reported together; no optimization benefit is claimed against invalid entry code.
All inherited measurements remain. Under spec §9 the inherited blanket 15% and
zero-growth targets remain diagnostic. Mandatory evaluator/native/inline/time
budgets, correctness and coverage are unchanged; later-stage obligations add no
PA29 gate.

## Remaining groups and handoff ledger

| Owner | Cases | Required next boundary |
|---|---:|---|
| Binary16/binary128 representation and ABI | 3 | Genuine formats across syntax/literals, constants, typed IR, storage, operations and calls/returns; unfinished implementation. |
| Nested-template ABI-tag contract | 1 | Independent extension/contract proof after the existing GCC/Clang-disagreement reducer; oracle and failure retained, no waiver. |

The [failure ledger](../student.tests/pa29/evidence191/remaining.json) preserves
all identities and dispositions. Full PA29/root-through success and the whole
stage audit remain required before PA30. Further vector changes do not supply
the new floating representations or resolve the separate ABI-tag contract;
starting either requires a distinct broad owner investigation/implementation.

- `599e3ad6`: entry ownership/data-flow/validation plan, preserved review markers.
- `66179d47`: typed intrinsic/vector semantics, constants, LowIR/native and ABI graph.
- `37d03290`: representation edge cases, packed static values and distinct vector ABI.
- `f542b376`: scaling-discovered fixed/dependent initializer and member repairs;
  completed controls and reproducible owner benchmark.
- Following record commit: final checks, performance, preservation and handoff
  evidence. The implementation handoff goal is complete once this record is
  committed cleanly; the whole assignment remains unfinished as listed above.
