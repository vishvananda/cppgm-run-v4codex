# PA29 compact plan — implementation193

Target: **PA29 full-stage**. Phase: **implementation**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `2df00585bd10d4e2e068934394dffc8adb0a47ed`.
Last reviewed commit: `5aaf16d15f8e50925c0b75a5485893a958501b85`.
Entry HEAD: `71b9f44aa662154c0c072fd9e4df6510324fce09`, clean, **402/403**.
Review markers are preserved; this is implementation, not whole-stage review.
Previous turn classification: progress (implementation192 completed floating
representation/ABI, 399→402; its documented evidence remains intact).

## Design/spec alignment and remaining group

The sole failure belongs to **effective ABI attributes of retained template
member definitions**. The checked contract requires suppression of a declaration
ABI tag on an out-of-class nested member definition. GCC/Clang disagreement is
already established; no C++11 proof permits changing that oracle. Implement the
hosted compatibility policy in semantic declaration/definition ownership, with
canonical entity facts consumed by the existing typed ABI graph and ELF emitter.
No spelling/fixture recognition, mangled-string rewriting or alternate lowering.

Trace source attributes → matched member prototype → instantiated definition →
effective ABI fact → typed mangler → symbol/relocation. Extend controls across
nested/direct/template members, inline/out-of-line definitions, explicit tags,
address/call uses and definition/demand order. Selection must reuse the existing
indexed prototype edge; work follows selected members/tags, with TU-owned compact
indexes and no global retries or per-use ancestor scans.

## Validation and performance plan

Freeze entry/final binaries and sources. Explicit personal controls will check
symbols and host-linked behavior, including ordinary unaffected tags and the
LowIR adapter. Run `make test-pa29`, the exact prior-through command, root through
PA29 and file audit. Preserve all 403 inputs and contract/harness paths.

Measure compiler wall latency/peak RSS plus executable runtime/text on fixed,
checked workloads with A/A calibration and ABBA observations. Separate equivalent
correct common workloads from corrected ABI behavior; no optimization benefit is
claimed. Preserve mandated budgets and inherited evidence. Spec §9 keeps unsupported
blanket 15%/zero-growth diagnostic targets nonbinding; later-stage requirements
are not extra PA29 gates. [Prior audit](audit.md), [floating implementation](implementation192.md)
and [performance192](performance192.md) remain available.

## Handoff ledger

| Owner | Unfinished implementation | Independent review |
|---|---|---|
| Effective nested-member ABI tags | Resolve remaining fixture and related ownership/order cases; validation/performance pending. | Verify general compatibility policy and complete source-to-ELF trace. |
| Prior PA29 groups | No new defect established at entry. | Whole-stage audit of the unreviewed implementation191–193 range remains required. |

Handoff requires no retained stage failure, earlier-stage/file-audit success,
recorded evidence, coherent commits and a clean tree. Whole-stage audit is not
waived by a successful implementation handoff.
