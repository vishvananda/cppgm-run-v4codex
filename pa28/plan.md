# PA28 implementation plan and handoff151

Target: **PA28 full-stage**. Phase: **validated incomplete implementation handoff**.
Stage base commit: `bec9389f62014dfecac8b41d330de142e0904b8c`.
Last reviewed commit: `bec9389f62014dfecac8b41d330de142e0904b8c`.
Review markers remain fixed until independent review.

Entry **81/97** → final **91/97**: ten existing failures resolved, six remain.
Earlier PA1–27 **4441/4441**; file audit passes with four inherited header warnings.
Final [validation](../student.tests/pa28/evidence151/validation.json) records the
required commands, exact remaining/resolved fixtures, binary/source hashes and
unchanged coverage. Fifteen explicit personal checks pass. No contract/reference
correction or harness/comparison change was made.

## Design/spec alignment and groups

| Status / owner | Data flow, complexity and validation |
|---|---|
| Complete: declaration attributes → semantic facts → shared Itanium graph | Interned source tags, sparse EntityId tag heads and membership, canonical callable effects, class/template publication and strongest redeclarations. Typed names feed ordinary/special symbols and ELF. O(actual tags), O(k log k) tag sorting, TU release. Required raw-symbol checks plus host-built consumer and LowIR effect inspection pass. |
| Complete: template/query/local identities → ABI graph | Parameter ordinals, dependent class pack lists, __decay query facts, local unnamed ordinals and lambda prefix substitutions. Existing query/graph caches and source occurrences; no grammar replay or rendered-name keys. Required naming fixtures, decay assertions and ABI adapter controls pass. |
| Unfinished: virtual completion/layout/projections (3) | Covariant virtual result row -24 vs required -32; external auxiliary vtable views; unavailable lazy template-base RTTI prerequisite. Own consistent address points, support objects and precise class-demand edges before changing output. |
| Unfinished: lifetime/EH regions and host LSDA (3) | Rethrow outer-local cleanup, resume past later lexical local, dynamic exception-specification unexpected handling. Own active lifetime/handler state and host filter facts; validate runtime and unwind interactions. |

[Implementation evidence](../student.tests/pa28/implementation151.md) traces
owners and lifetimes. Direct typed source → LowIR → MIR → ELF, canonical identity,
precise demand and bounded work remain; no external implementation delegation.

## Stage-scoped performance

[Performance151](../student.tests/pa28/performance151.md) retains **744 observations**,
including all pre-packing runs. Final frozen A/A + six ABBA blocks cover compiler
latency/RSS and checked runtime/text size on four fixed workloads. A/B common
objects and executable text are identical; Entity remains **120 bytes**. Paired
compiler medians are .991, 1.015, .996 and .997, with full spread disclosed.
New naming behavior has standalone costs because A cannot compile it.

No timing speedup or optional runtime transform is claimed. Tags have linear
storage and bounded sorting; common code growth is zero. Spec §9 keeps inherited
15%/zero-growth diagnostics distinct from mandated limits. Existing limits,
correctness and coverage stay required; PA32/33/34 acceptance stays stage-owned.

## Handoff ledger and boundary

| Commits | Disposition |
|---|---|
| `b7743532` | Recorded entry/review markers before implementation edits. |
| `2a87fba0` | Completed naming/attribute group; 10 required failures resolved. |
| `ef7f93de..e58612a4` | Sparse tag storage and packed effect byte; final compiler revalidated and remeasured. |
| Evidence/plan commit | Required checks, personal controls, performance records and coverage manifest; no further compiler changes. |

This handoff ends at the naming/attribute boundary. The covariant spelling is
already faithful to its semantic row; remaining fixes require physical vtable
layout, class-completion scheduling, or EH/LSDA state and new runtime proofs.
They cannot be completed by extending name encoding or declaration attributes.

Independent review remains pending for all PA28 commits: semantic publication,
cache/lifetime and source-to-ELF ownership, substitution ordering and performance
evidence. Those review questions are separate from the six known implementation
failures above. Neither is waived; PA28 has not passed its full-stage exit.
