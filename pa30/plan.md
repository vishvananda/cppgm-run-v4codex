# PA30 compact implementation plan — implementation201

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `4a081cb05b25638be7a759882f67d4d8ae97eb6a`.
Target: **PA30 full-stage**. Phase: **implementation handoff; stage incomplete**.
Implementation entry HEAD: `b0790726de6c67722826833b395b76853e051e2d`.
Code tip: `fc8ea8d6`. Both review markers are preserved.

## Design/spec alignment

[Design201](design201.md) records owner, data flow, complexity and validation.
Automatic object use follows selected declaration/function/capture identities,
including dormant default arguments and nested lambdas. Noreturn attributes
survive parsing and specialization as typed call-boundary facts. Non-void
completion checks the actual LowIR CFG, preserving main, constant loops,
labels, switches, noreturn normal exits and distinct exceptional exits.
Function-local worklists, bounded integer proofs and parent-linked EH regions
run in O(instructions + edges), with no IR rewrite, repeated semantic resolution,
parse replay, global retry or optional optimization. New sources are registered.
[Reference correction](reference-correction201.md) proves the one incompatible
replacement-new expectation; all source fixtures and comparison rules remain.

## Validation and performance

[Reports](../student.tests/pa30/evidence201/validation.json): earlier PAs
**4941/4941**, file audit passes (four inherited warnings), PA30 **151/153**,
through30 **5092/5094**. [Delta](../student.tests/pa30/evidence201/stage-delta.json):
**five → two existing failures**, comprising two implementation fixes and one
proven reference correction; no new failure, removed case or narrowed check.
Ralph's entry summary 148/154 (six failures) disagrees with the primary log and
153-case inventory. Both are preserved; progress also holds against that count.
Explicit validation: **111** current controls, **245** inherited controls,
and **101** source/LowIR/object trace commands pass.
[Verifier](../student.tests/pa30/verify201.py): **1283** checks bind current
sources, binaries, fixtures, outcomes and performance. The previous turn is
classified as verified progress; no inherited process was live at entry.

[Performance201](performance201.md): frozen A/B binaries, A/A calibration,
six ABBA blocks, all 420 observations and 16 launcher calibrations; compiler
latency/RSS and checked runtime/text together. Common and owner A/B images are
byte-identical; owner work scales as 3N functions, 274N instruction visits and
35N edge visits. Maximum hosted sample: **2.503 s / 177048 KiB**.
The **45-second per-compile limit** is mandatory.
Historical blanket percentage/zero-growth diagnostic targets remain non-gates
under spec §9; all earlier measurements are preserved. No optimization speedup
is claimed. Later-stage runtime/optimization/self-hosting work remains scoped
to PA31–34, without waiving PA30 correctness or architecture requirements.

## Remaining implementation

| Group | Required work |
|---|---|
| Packed SIMD (two failing fixtures) | Both random inputs stop at `__builtin_ia32_packsswb`. Implement typed builtin signatures, saturation and lane semantics, then subsequent header operations. No stubs or unused-result invention. |
| General vector expressions | Inherited `pending199/vector-subscript.cpp` still fails. Recorded again in evidence201/pending.json; this is unfinished implementation. |

Do not advance until the full root through30 report passes.

## Handoff ledger

| Work | State and evidence |
|---|---|
| Checkpoints195–197 | Reviewed/repaired by [audit198](audit.md); historical measurements and proofs retained. |
| Implementation199/200 | Committed prior repairs; independent delta review remains pending on Ralph's schedule. |
| `acffe94c`, `fc8ea8d6` | Complete automatic-object/default/capture and control-flow groups, extended through inherited noreturn, constant-loop, handler and default-lambda defects. |
| `3d02099c` | Reference exit-status correction, reduced reproducers, preprocessing and C++11 proof; the runtime and matching-declaration positive controls remain. |
| Handoff boundary | Validated group complete. Remaining packed arithmetic/vector lvalues have separate typed intrinsic, saturation, lane, storage and native lowering owners. Further changes to function ownership or control reachability cannot implement them. No known defect in this completed group is deferred as a review question. |
| Independent review | Implementation199–201 deltas await Ralph's audit schedule. Whole-stage architecture/correctness review is separate from the explicit implementation work above; neither is waived. |

This implementation handoff does not certify the whole assignment.
