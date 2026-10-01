# PA29 compact plan — implementation191

Target: **PA29 full-stage**. Phase: **implementation; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `2df00585bd10d4e2e068934394dffc8adb0a47ed`.
Last reviewed commit: `5aaf16d15f8e50925c0b75a5485893a958501b85`.
Audit entry: `bac893d42bba792ab2ce3c868a69cd183acb73bc`, clean, **398/403**.
The preceding implementation turn made verified progress; no live work needed
continuation at audit entry. `fail (2)` was exit status; five failures is the
preservation baseline. The record commit follows the code tip with no code edits.

## Design/spec alignment

[Audit190](audit.md) reviews every commit and the combined source across all
three accepted handoffs since review186: complex values/ABI, contextual syntax,
and template-definition/constant-condition behavior. Its
[range manifest](../student.tests/pa29/evidence190/range.json) records all 14 commits
through the repair and 82 combined implementation/build paths.

Audit repairs complete the static constant-data key, propagate valid complex
infinity/NaN inputs, emit extension RTTI, keep complex continuations typed,
execute retained source/query list recipes through one constant owner, and read
the writer's special-value suffixes. No grammar replay, global retry, fake
runtime materialization or textual production transport is added. Canonical
facts live in the TU; function MIR dies after encoding. The demanded integrated
template trace crosses all three handoffs through host ELF and unwind output.

Three inherited reference corrections are accepted on
[reduced C++11/contract proofs and pinned bundle revision](reference-corrections189.md).
No fixture/reference/harness changes occurred in audit190. All original inputs
and comparison rules remain. Undefined library templates are not invented.

## Validation and performance

`make test-pa29`: **398/403**, exactly the same five failures, exit 2.
PA1–28: **4538/4538**, exit 0. Root through PA29: **4936/4941**, only PA29 fails.
File audit passes with the four inherited header-division warnings. Explicit
controls187/188/189/190 pass **470 commands**; full-phase AST/LowIR/native/ELF
inspection passes **82 commands**. All **403 inputs and 1,707 contract/harness
paths** are preserved since entry; three proven exit-status corrections are the
only changes since the previous review. Source and binary hashes bind validation
to the reviewed code tip. Failure identity is unchanged, so progress passes.

[Performance190](performance190.md) retains **656 new observations plus 16
launchers**, frozen binaries/inputs/flags, A/A calibration and six ABBA blocks on
correct equivalent pairs, and corrected-only scaling on the repaired owners.
Ten equivalent object/executable pairs are byte-identical. All four dimensions,
noise/outliers, bounded fact growth and **2,936 historical observations plus 56
launchers** are retained and checked. No speedup or repeatable avoidable regression
is established. Mandatory evaluator/native/inline/time budgets are unchanged.
Inherited 15% latency/RSS and zero-growth targets are diagnostic under spec §9;
necessary semantics and later PA30–34 obligations create no additional PA29 gate.

## Remaining broad groups

| Owner | Cases | Next complete boundary |
|---|---:|---|
| Binary128/half representation and ABI | 3 | Genuine formats, constants/operations, typed IR, storage and calls/returns. |
| Vector expressions and type-operand intrinsics | 1 | Deduction, conversions/reduction/comparison and required typed lowering. |
| Nested-template ABI-tag contract | 1 | Independent reducer and extension/contract proof; retain the current oracle and failure meanwhile. |

The [failure ledger](../student.tests/pa29/evidence190/remaining.json) preserves
all five cases. Full PA29/root-through success remains required before PA30.
The three handoffs reduced failures 11→5 (three by proven reference correction).
Complex work omitted static-key/RTTI/continuation consumers; constant-condition
work omitted fixed-list recipe evaluation. Avoid that fragmentation by finishing
an owner across source/query/demand, constant/static/runtime/ABI and adapter
boundaries before handoff. Keep the remaining groups broad. Historical handoffs,
[prior audit](audit186.md), all measurements and the cumulative audit ledger remain.

## Implementation191 active work

Entry HEAD: `32c1f5f43cde4e09f623f0e5d606a51d170e8203`; clean, 398/403.
Previous turn: verified progress (audit190 repaired owners and recorded evidence).
Review markers above are preserved. Baseline failures/coverage are those in
`evidence190/remaining.json`; no existing fixture or comparison will be removed.

First owner group: vector expressions and type-operand intrinsics. Trace retained
syntax/type operands → canonical vector types and deduction → recorded operand
conversions/intrinsic identity → ordinary typed LowIR → native object. Complete
comparison, conversion/reduction and bit-cast semantics used by the hosted
fixture, with related validation, template demand, constant/query and adapter
consumers examined together. Work/storage must track source and vector lanes;
no token replay, named-type special cases or hidden backend channel.

Validation: explicit personal positive/negative/template/runtime/LowIR controls,
required PA29 and PA1–28 reports, file audit; freeze entry/current binaries and
inputs for A/A and ABBA compiler latency/RSS plus executable runtime/text on
correct equivalent controls, and corrected-only measurements where entry fails.
Floating formats remain unfinished implementation. ABI-tag mismatch remains an
independent contract question; neither is waived. Handoff boundary remains open
until the complete owner is validated and related defects are resolved.
