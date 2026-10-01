# PA29 compact plan — implementation192 handoff

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `2df00585bd10d4e2e068934394dffc8adb0a47ed`.
Last reviewed commit: `5aaf16d15f8e50925c0b75a5485893a958501b85`.
Entry HEAD: `4becf437ada14c36b01baae1d608428a621a5317`, clean, **399/403**.
Implementation tip: `dca6f1f2`; the following record commit changes no implementation.
Review markers are preserved. This returns control to Ralph, not certification.

## Design/spec alignment

[Implementation192](implementation192.md) completes the scalar floating owner:
exact literal bits → canonical type/precision → constants/conversions/layout →
typed `f16`/`f128` LowIR → native storage, arithmetic and host ABI. GNU and
standardized-width types retain their C++ identities; explicit lexical typedefs
support the captured legacy header profile. Related integer-rounding, full-width
constant-array keys, variadic storage and raw LowIR truth defects are repaired.
There is no parser replay, fixture recognition, production host compilation or
hidden textual phase transport. Helpers use ordinary typed runtime calls.

Literal scanning is linear with a fixed-format limb bound; facts and numeric
indexes belong to the TU. Native legalization runs once in O(input + output),
creates at most six operations per input and releases replaced pools. Constants
charge the existing evaluator. No optional optimization pass is added.
[Prior audit](audit.md), [vector work](implementation191.md) and inherited
measurements remain. One [proved Q-token correction](reference-corrections192.md)
changes type/representation bytes; inputs, statuses and comparison rules remain.

## Validation and performance

[Final checks](../student.tests/pa29/evidence192/validation.json): PA29 **402/403**
(exit 2); PA1–28 **4538/4538** (exit 0); root through PA29 **4940/4941**
(exit 2), only the retained ABI-tag case. File audit passes with four inherited
header warnings. Explicit controls pass **84/84** commands, inherited vector
controls **191/191**, and the independent rational decoder **1642/1642** cases.
O0/O2 execution, host calls/layout/varargs, LowIR roundtrips, raw truth branches,
rejections and ELF/MIR/unwind inspection are covered. [Coverage](../student.tests/pa29/evidence192/coverage.json)
preserves all **403 inputs and 1,707 contract/harness paths**, with only the
proved reference correction. [Progress](../student.tests/pa29/evidence192/stage-delta.json)
is **4→1 failures**, with no new failures or reduced coverage. Source/binary
hashes bind the checks to committed implementation.

[Performance192](performance192.md) retains **640 observations plus 32 launchers**,
four A/A + six ABBA common pairs, six corrected-only scaling controls and
launcher calibration. Compiler latency/RSS and runtime/text are reported together,
with raw spreads and checked outputs. Common images remain byte-identical;
final compile ratios are 1.004–1.017 with every range crossing unity. Scalar
format work/text scales with demand. No benefit is claimed against rejected
entry programs. Spec §9 keeps inherited blanket 15% and zero-growth targets
diagnostic; mandated evaluator/native/inline/time limits remain unchanged.
Later-stage obligations add no PA29 gate, and correctness failures remain counted.

## Remaining group and handoff ledger

| Owner | Failures | Disposition and next boundary |
|---|---:|---|
| Scalar extended floating representation/ABI | 0 | Completed; no known unfinished requirement in this group. |
| Nested-template ABI-tag contract | 1 | Independent extension-contract review; GCC/Clang disagreement reducer and unchanged oracle retained. No waiver. |

The [failure ledger](../student.tests/pa29/evidence192/remaining.json) retains
its identity and proof boundary. Further floating work cannot decide the separate
attribute/naming policy. Its existing reducer does not supply the authorized
proof needed to change the reference; repeating that investigation would not
extend this completed owner. Full PA29/root-through success and whole-stage
review remain required before PA30.

- `f3a80433`: entry ownership/data-flow/complexity/validation plan; review markers.
- `f3c93919`: exact formats, canonical semantics, typed IR/native ABI, controls
  and documented Q-token reference proof.
- `5369931a`: parameter-only floating branch legalization, layout/roundtrip
  controls and reproducible scaling harness.
- `dca6f1f2`: isolated bit-initialized-slot truth repair and regression control.
- Following record commit: final checks, preserved preliminary evidence,
  performance and handoff ledger. Once committed cleanly, complete the
  implementation handoff goal; the whole assignment remains unfinished above.
