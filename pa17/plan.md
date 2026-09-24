# PA17 compact plan — implementation, loop 57

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `e14b96fa9d4b3376e5922b8ad30093c3c0b0c759`

Target: **PA17 full-stage**, implementation still incomplete.

Loop 57 entry: `6915296626992eb1ed20f6ca6e8c25a028c0335f`, clean;
324/343, 19 failures. Previous turn: verified progress (committed audit fixes
and evidence). Entry compiler and failure log frozen in
`scratch/implementation57/`. Review markers above are preserved.

Implementation sequence and ownership:
- Semantic conversions/transfer actions own immediate stores, memberwise copies,
  and complete base paths; typed lowering consumes the recorded actions. Work
  follows converted expressions, subobjects and selected path edges.
- Semantic initialization/publication owns static definitions, addresses,
  constant reads and automatic array effects; storage lowering consumes those
  facts once per entity. Extend into this related group as transfer findings
  support it; preserve bounded array work/growth and earlier contracts.
- Query/candidate/closure demand and exception-region scheduling remain distinct
  unfinished owners. No independent review requirement is waived.

Validate with focused course cases, explicitly run personal controls, full PA17,
PA1–PA16 and file audit. Freeze final compiler/inputs for A/A + ABBA latency/RSS
and executable runtime/text observations. PA17/O0 has no mandated numerical
ceiling; preserve existing bounded-work limits and document semantic costs.

Validated implementation increment: PA17 **330/343** (six original failures
closed); PA1–PA16 **2266/2266**; 27 new controls pass. Typed base paths now retain
qualification boundaries and reject ambiguous conversions without candidate
exceptions; scalar/memberwise transfer fixes extend through automatic array
materialization. Cumulative personal controls and frozen performance campaign
remain pending. The remaining storage-demand, query/closure and exception-region
owners are implementation work, separate from independent review of this increment.

Historical reviewed checkpoint: The accumulated
review covers `c43e8eb6..e14b96fa`: all three accepted handoffs, their interactions,
and both audit fixes (10 commits, 37 implementation paths). Entry `b739e08d`
and final pass **324/343**, with the exact same **19 failures** and all 343
course inputs unchanged. PA1–PA16 pass **2266/2266**; **465/465 personal controls**
pass (439 inherited, 26 new; entry fails 11 new controls). No fixtures,
references, bundle revisions or comparison rules changed. The preceding turn
was progress, verified from committed implementation and checkpoint logs.

| Reviewed ownership | Findings and disposition |
|---|---|
| Partial selection, template arguments and dependent queries | Reviewed exact tuples, shared class/variable selection, renamed enclosing heads, query argument packs and failure states. Fixed bare template entities accepted as types, cv-qualified failure becoming a valid type, and non-class/non-template candidate failures escaping as diagnostics. Canonical lookup category travels through substitution, source-head normalization and ABI views. Completed failures are cached by immutable type/frame; active-class misses stay uncached. |
| Member publication, construction and qualified source paths | Reviewed anonymous injection/access/storage/base offsets, conditional explicit query ownership, array temporary/list-lane identity, current-instantiation ambiguity and early inline visibility. Fixed-position expansions normalize after expansion. All three handoffs' review questions are closed by source inspection, cumulative controls and performance evidence. |
| Typed O0 lowering and lifetime boundaries | Reviewed constant boolean branches and selected-union zero stores, their legality, bounded work/growth and conservative fallbacks. Required runtime/text differences and backend constraints remain disclosed. No optional optimizer added. |

[Audit](audit.md) and [performance evidence](checkpoint56-performance.md) bind the
range, source-to-LowIR-to-ELF trace, checks and frozen A/B observations. PA17/O0
has no mandated numerical ceiling. Historical +15%, +16 MiB and 5.5× targets
remain diagnostic under spec.md §9; all observations and existing constant-work,
initialization-growth, correctness and coverage limits are preserved. Earlier
[selection](selection-performance.md), [publication](publication-performance.md)
and [qualified](qualified-performance.md) campaigns remain intact.

| Remaining broad owner | Failures | Required work |
|---|---:|---|
| Query/candidate demand and closure entities | 4 | Recursive ADL/class-selection dependencies, ambiguous-operator expected failure, and lambda callable/body/lifetime ownership. |
| Static storage and initialization | 9 | Static/member publication, local guards, static/dynamic references and addresses, constant reads, and array evaluation/initialization effects. |
| Scalar/object transfer and adjustment | 4 | Constant integer conversion, memberwise copies, full base adjustment and empty-object value initialization. |
| Exception cleanup scheduling | 2 | Required call-region boundaries and cleanup ordering. |

All 19 cases remain unfinished implementation, with exact case membership in the
[evidence manifest](../student.tests/pa17/checkpoint56-evidence.json). Do not advance
to PA18. Avoidable fragmentation split selection, publication and qualified
argument semantics across three handoffs; review exposed shared category/failure
bugs across those boundaries. Future increments should close a broad owner with
its declaration, substitution, demand and lowering interactions together.

| Loop / phase | Audit ledger |
|---|---|
| 56 / checkpointAudit | `c43e8eb6..e14b96fa`; complete accumulated review; dependent-category/failure fixes; PA17 324/343, unchanged 19 failures; earlier 2266/2266; 465 controls; file audit pass; stage-scoped performance evidence accepted. Four broad implementation groups remain. |

Run `python3 student.tests/pa17/verify_checkpoint56.py`. Records follow the
reviewed code tip without further implementation changes. Historical handoff
verifiers describe their frozen tips; [loop 52](audit-loop52.md) is preserved.
