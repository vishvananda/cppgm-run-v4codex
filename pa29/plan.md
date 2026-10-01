# PA29 compact plan — implementation181

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `667edd800e4e5eb1b1ef92a3108da3bd96c51708`.
Previous reviewed commit: `7139ceb5769eea12c6e2e00b54932f09d59c79b5`.
Entry HEAD: `c0efbf2992a9e58dbaa2d1e7300496012eed5988`.
Code tip: `0e47335f`.

## Design and spec alignment

The completed group is **GNU zero-extent array identity, layout and lifetime**.
A canonical type distinguishes absent and zero bounds. Existing declaration,
substitution, initialization, conversion and ABI owners consume that fact;
production still constructs typed LowIR and direct ELF without text transport.
Related fixes include immediate-context substitution failure, parenthesized
parameter packs, static member definition bounds, zero-sized class effects,
allocation element counts, explicit adapter parity and pre-encoding width limits.
[Handoff181](handoff181.md) records ownership, data flow, complexity and boundaries.
[Audit178](audit.md), [handoff180](handoff180.md) and all review markers remain intact.

## Validation and performance

PA29 **387 → 388/403**: one original failure removed, **15 remain**, no new
failures. PA1–28 **4538/4538**; through PA29 **4926/4941**. File audit passes
with four inherited warnings. All 403 inputs and 1,707 contract/harness paths
remain unchanged. **43** personal controls and **166** inspection commands pass.
Final commands, source hashes and coverage are bound in the
[evidence manifest](../student.tests/pa29/evidence181/manifest.json).

[Performance181](performance181.md) records compiler latency/RSS and runtime/text
size at PA29/O0: **440 final** observations and **440 preliminary** observations
preserved, with A/A+ABBA on equivalent inputs and final-only zero-extent scaling.
No optional optimization is introduced; equivalent benchmark text is unchanged.
Array emission keeps its existing eight-element expansion bound; larger live
arrays use counted loops.
Canonical Type storage remains 40 bytes; each allocation fact adds one eight-byte
logical element-count multiplier. Existing mandated limits remain enforced,
including the corrected pre-narrowing object extent check. Inherited blanket
15%/zero-growth targets remain diagnostic under spec §9.

## Remaining groups and handoff ledger

The [15-case ledger](../student.tests/pa29/evidence181/remaining.json) separates
**12 unfinished implementation** cases from **3 independent contract questions**,
all still failures: extended syntax/types **11**, template demand/hosted ABI **3**,
legacy trait **1**. No reference correction, coverage reduction or waiver occurs.

The completed group covers scalar/class, constant/runtime, template, ABI,
exception, alignment and storage-limit boundaries. The remaining failures need
extended numeric/complex types, vector intrinsics, GNU expression/syntax forms,
library conversion or ABI policy; they do not use the repaired zero-bound facts.
That separate ownership is the concrete boundary for this incomplete handoff.

Commits `e0619de9`, `f69601ef`, `0e47335f` implement and validate the group;
`42a2bac7` records entry scope. The final record commit returns control to Ralph
for independent review. This handoff ends implementation181, not the assignment
or its audit; full-stage success and whole-stage review remain prerequisites
for advancement.
