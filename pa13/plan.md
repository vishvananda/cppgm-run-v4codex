# PA13 implementation plan and handoff

Stage base commit: `823e929cdc74fedbb487973dcad33b8a4dce18f4`
Last reviewed commit: `823e929cdc74fedbb487973dcad33b8a4dce18f4`
Target: **pa13 full-stage**. Phase: **implementation complete**.
Entry: **2/37**; final: **37/37**, **1621/1621 through PA13**.
Both original review markers are preserved for Ralph's independent audit.

## Design/spec alignment

| Owner | Data flow and complexity | Validation |
| --- | --- | --- |
| Semantic class completion | Interned name/signature → inherited slots → final/override/pure/covariant/exception facts. Flat indexes; work follows actual declarations and slots. | All rejection fixtures; 15 personal semantic controls. |
| Layout / ABI | Canonical class and function IDs → vptr/base offsets, local discriminators, typed vtable/RTTI support; one emission per demanded identity. | Root/base/local/header fixtures; multi-TU execution. |
| Calls | Selected member + qualification + slot + unwind contract → direct/indirect LowIR; no repeated resolution. | Objects, pointers, references, temporaries, dereferenced pointer fields. |
| Lifetimes / deletion | Prepared actions → vptr writes, identity-preserving transfers and selected deallocation; D0 expands at most one suffix or calls D1 once. | Six native sources; raw D2/D0/D1 order and bounded cleanup controls. |

[Implementation trace and audit evidence](implementation.md) describe ownership,
release points and the literal-address validator correction. No fixtures,
references, coverage or comparison rules changed. O0 emits typed LowIR directly;
MIR/native optimization, templates, member pointers and generalized RTTI remain
with their later owning stages. The supplied backend is only a test boundary.

## Performance acceptance

[Report](../student.tests/pa13/performance.md): frozen A/B binaries, fixed input
and output hashes, A/A calibration, ABBA blocks, separate compiler/runtime runs
and 352 retained observations. Every comparable output/executable is identical.
Compiler text grows **37760 bytes (3.85%)**. The large common-class repeat has
**0.74% median latency / 1.75% peak-RSS growth**; isolated timing outliers and
small regressions remain disclosed. No optional transform or speedup claim.
Fourfold virtual-corpus growth gives **4.23x latency / 3.87x RSS / 4x slot work
and IR**. Native runtime/payload sizes and startup noise are reported together.
No unsupported inherited diagnostic target becomes a PA13 gate; mandated
correctness, coverage and explicit work/growth bounds are preserved.

## Handoff ledger

| Group | Commit / result |
| --- | --- |
| Stage entry / plan | `823e929c`, `f16d91dd`: 2/37 |
| Canonical virtual slots and semantic checks | `9a80791c`: 12/37; inherited suite and file audit pass |
| Polymorphic layout, ABI, calls, lifetimes and deletion | `df2be861`: 37/37; through-stage 1621/1621; personal execution/order/demand/growth/multi-TU controls pass |
| Contextual literal pointer validation | `6ca0578c`: 37/37 and 1621/1621; good/bad LowIR reducer; actual ASan/UBSan compiler passes all stage and personal controls |
| Final evidence | Frozen performance and repeat campaigns, verified hashes, implementation trace and this compact handoff |

Required `make test-pa13`, `make test-report-through-pa13`, and the PA13 file
audit pass. The file audit has three advisory header-division warnings and no
failures. Personal controls run explicitly; all 352 evidence observations and
frozen hashes verify. Full logs/artifacts: `$RALPH_ARTIFACT_DIR/pa13/`.

Handoff reason: **full stage completed; no remaining PA13 behavior groups**.
The previous goal turn completed PA12, and this turn removes all 35 PA13 entry
failures while preserving the 37-test stage and every inherited test.
