# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Target: PA18 full-stage. Entry: **266/420** course tests pass (154 failures).

| Owner / group | Design, data flow and work |
|---|---|
| Function deduction and ordering (active) | Canonical type/parameter identities feed directional partial deduction; only participating call parameters (full signature for addresses). Reference/cv and pack tie rules select the declaration once; conversion facts carry it to typed LowIR. Scratch bindings live for one comparison; O(candidate count × compared type edges), no body demand, rendered keys, exception-based rejection or global scans. |
| Immediate substitution / SFINAE (remaining) | Retained dependent query/type → immutable substitution frame → candidate success/discard; defaults, return types, expression validity and declaring-scope identity. |
| Conversion/constructor/explicit deduction (remaining) | Target or call values → deduction → ordinary recorded conversions; preserve demand boundaries and member owners. |
| LowIR mismatches (remaining) | Separate already-correct deduction from initialization, result metadata, and constant lowering differences; preserve comparison rules. |

Spec alignment: extend the existing typed semantic graph, TU interners and
per-comparison scratch; no token replay or semantic text transport. Stage is
O0 LowIR: no optional optimization or native-backend work is introduced.
Performance: freeze entry/final binaries and inputs; A/A calibration and ABBA
compiler latency/RSS on equivalent correct programs, final-only observations
for newly accepted behavior, checked backend execution and text/payload size.
PA18/O0 mandates no numeric latency/RSS ceiling; later native/self-host stages
remain outside this stage. Preserve explicit work bounds and all observations.

Validation: run focused required fixtures and explicit personal controls,
`make test-pa18`, root through-PA17, and the PA18 file audit. Full through-PA18
must pass before advancement. No fixtures, references or coverage are waived.

| Handoff ledger | Implementation | Independent review |
|---|---|---|
| 63 (in progress) | Ordering/address family under implementation; remaining groups above unfinished. | Stage base is the review boundary; this handoff will require independent review. |
