# PA28 implementation plan and handoff152

Target: **PA28 full-stage**. Phase: **validated incomplete implementation handoff**.
Stage base commit: `bec9389f62014dfecac8b41d330de142e0904b8c`.
Last reviewed commit: `bec9389f62014dfecac8b41d330de142e0904b8c`.
Review markers remain fixed until independent review.
Turn entry: `bed2be54be95a64037bc09dca8867dab996a9c88`, **91/97**.
Final required behavior: **96/97**; earlier PA1–27 **4441/4441**.
[Validation](../student.tests/pa28/evidence152/validation.json): earlier tests,
file audit (four inherited warnings), 42 personal commands and unchanged
coverage pass; [native inspection](../student.tests/pa28/evidence152/inspection.json) passes.
Five existing failures resolved; no fixture, reference or comparison changes.
Previous turn classification: progress (ten required failures resolved in evidence151).

## Design/spec alignment and groups

| Status / owner | Data flow, complexity and validation |
|---|---|
| Complete, inherited naming/attributes | Canonical tags/effects and typed Itanium naming → ELF; previous implementation/performance151 evidence retained. Naming controls rerun. |
| Complete: lifetime contexts → native cleanup regions | Persistent live prefix and complete handler context → shared cleanup suffixes; explicit region retirement, outer-local cleanup and selector-preserving resume. Existing linear/cached lowering owners; host/private controls pass. Course textual adapter retains its existing ABI convention. |
| Complete: exception-specification facts → host LSDA | Callable/specialization identity → adjusted canonical TypeId set → typed LowIR filters → signed native selectors and ELF type lists. O(k log k) set canonicalization, linear table emission, TU/function release. Permitted/converted/empty specifications, redeclarations, templates and ordered cleanup controls pass. |
| Complete: imported support ownership and static demand | Key-definition owner → imported table/VTT declarations; construction prerequisites only on definition demand. Completed dynamic-class facts prove required vptr initialization without draining unrelated member actions. Existing deduplicated queues and IDs; mixed host ownership and recursive-template scalar/array statics pass. |
| Unfinished: covariant virtual-primary layout (1) | Nearly-empty virtual-primary sharing and inherited mixed vcall/vbase prefixes are missing. Required row is -32; current model supplies -24. Must change primary selection, physical allocation, projections, VTT segments and RTTI coherently; a thunk-name/padding workaround is insufficient. |

[Implementation trace](../student.tests/pa28/implementation152.md) and
[adapter evidence](../student.tests/pa28/adapter152.md) record ownership and limits.
Direct typed source → LowIR → MIR → ELF remains; no production text transport or
external implementation delegation. Native MIR now retains unnamed block IDs.

## Stage-scoped performance

Frozen entry/final binaries, unchanged common template/loop/call/memory/FP/EH
inputs, A/A calibration and six ABBA blocks measure compiler latency/peak RSS
and checked runtime/text size. New filter and imported/static ownership workloads
measure standalone correct-behavior costs because entry behavior is wrong or
unsupported. All intermediate and final observations (**520**) are retained in evidence152;
[performance152](../student.tests/pa28/performance152.md) records all four dimensions.
Paired compile medians are 1.013, .996, 1.028 and 1.011; spreads/noise and
standalone new-feature costs are disclosed.
No optimization speedup is claimed. Common output is byte-identical; required
semantic work is bounded by actual declarations/clauses/regions. Spec §9 governs
acceptance: inherited 15% and zero-growth diagnostics are not mandated gates.
Mandated limits, correctness and coverage remain intact; PA32/33/34 retain their
stage-owned optimization/self-hosting obligations.

## Handoff ledger and boundary

| Commit | Disposition |
|---|---|
| `100de24b` | Recorded entry, ownership groups and validation plan; preserved review markers. |
| `ed85fc94` | EH cleanup and allowed-exception filter implementation; three failures resolved. |
| `017675aa` | Imported VTT ownership and dynamic local-static demand; two more failures resolved. |
| `810ef2d8` | Preserved the course ABI adapter and restored original references; insufficient reference-correction proof was rejected. |
| `5f8483c7` | Made native inspection retain actual block identities. |
| Evidence commit | Final required checks, personal controls, performance and coverage inventory; no further implementation changes. |

The completed owners are EH regions/filters and support-object demand/ownership.
Further covariant work needs a new physical class/table representation spanning
allocation, primary sharing, prefix identity and projections, with mixed host
producer/consumer proofs. It is impractical to extend the completed ownership
and cleanup fixes into that separate representation change in this handoff.

Independent audit remains pending for all PA28 commits: canonical publication,
cache/demand lifetimes, source-to-ELF ownership, adapter equivalence, LSDA filters,
substitution ordering and performance evidence. These questions are separate
from the one known implementation failure. Neither is waived; this handoff
returns control to Ralph and does not certify PA28 or permit advancement.
