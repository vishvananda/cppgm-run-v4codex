# PA27 final plan and disposition

Target: **PA27 full-stage**. Phase: **final audit complete**.
Stage base: `f833cf1ff361529147361cada33eca62e55330cf`.
Audit entry: `9f5e095d450519d8660a29d4629ec5b8f5bcddde`.
Reviewed code tip: `2fba819daa6e8a79a38dc0d35a3f8f942ff01bb7`.

Required checks pass: **4441/4441 tests, 27/27 stages**, PA27 **158/158**,
section properties **3/3**, and file audit with four inherited header warnings.
The entry's reported 4604 total differs from both its primary log and the fresh
root report; 4441 is the measured total, with unchanged coverage.
[Final validation](../student.tests/pa27/evidence150/validation.json) records the
commands, statuses, log hashes, binary and all changed implementation hashes.

## Final design / Spec Alignment

The compiler owns the complete source → streaming interned tokens → integrated
syntax/semantic graph → typed LowIR → per-function MIR → direct ELF path. Host
linking remains the assignment boundary. Text views are inspection adapters.
Canonical identities, source-owned template regions, parent-linked substitution
frames, separate demand states and precise completion dependencies implement
spec §§1–6. TU arenas/flat indexes and per-function native release implement
§8; no accumulated process-global cache or duplicate production text graph is
introduced. The [independent audit](audit.md) reconstructs these owners and traces
actual source, facts, MIR, relocations and executed objects.

| Completed ownership group | Result |
|---|---|
| ELF/object demand | Direct sections and relocations; alignment and addends; lazy COMDAT relocation members; imported-data/function GOT addresses; retained base entries and addressed locals; omitted unused locals. |
| Semantic ABI/linkage | Canonical mangler inputs, substitutions and template parameter identity; suppression, aliases, internal support objects, archive references and TLS roles. |
| Construction/storage/constants | Selected anonymous storage paths, partial unwind, immutable default contexts and reachable-reference validity; semantic field projections and activation-owned receiver groups shared by every consumer. |
| Hosted fixture integration | Shared include search, GNU macro/builtin prerequisites, canonical atomic signatures, typed dependent aliases/friends and selected-signature access recipes. The checked-in hosted stream fixture remains required and passes. |
| Final audit repair | Both header-probe operators require the existing conditional-expansion context, including macro argument prescans and replacement rescans. Fourteen focused commands pass. |

## Performance acceptance

[Performance150](../student.tests/pa27/performance150.md) reports compiler latency,
peak RSS, runtime and text size together for eight fixed workloads. Final evidence
contains **416 observations**; another 416 before the context repair and all
2904 historical observations remain. Frozen A/B, four A/A runs and six ABBA blocks
apply to every supported pair. Newly supported hosted behavior has measured
costs and a host result control, without an invalid failing-A comparison.

No precise timing improvement is claimed. Executable instruction comparisons
explain the required GOT change and 30,816-byte unused-function removal;
construction/signature objects are identical, as are constant objects/text.
No optional transform was added. Spec §9 makes inherited blanket 15% latency/RSS
and zero-growth targets diagnostic; required correctness, coverage, timeouts,
constant limits and native work/growth bounds remain unchanged. PA32/33 optimizer
policies and PA34 self-hosting remain later-stage work.

## Consolidated ledger

| Boundary | Disposition |
|---|---|
| 145–147, through `7dbb0678` | Object, naming and storage handoffs independently re-reviewed as a cumulative implementation. |
| 148, through `cbe7871e`; record `226b8866` | Access/projection/constant receiver repairs preserved; [checkpoint audit](audit148.md) archived unchanged, including its then-unresolved 157/158 result. |
| 149, `d1b017e5..9f5e095d` | Previously unaudited hosted integration and diagnostics now reviewed through all owners; required fixture resolved without coverage changes. |
| 150, code `2fba819d` | Whole-stage review of 23 commits / 81 implementation paths, conditional header-probe repair, new cross-object trace, complete performance/validation evidence. |

Only the previously proved [overlay145 reference correction](reference-corrections.md)
exists in the whole-stage contract diff. No fixture, reference, harness or
comparison change occurred during this audit. No unaudited handoff, known required
implementation defect or PA27 blocker remains. The final audit/evidence commit
contains no subsequent compiler edits.
