# PA28 implementation handoff153

Target: **PA28 full-stage**. Phase: **validated full-stage implementation handoff**.
Stage base commit: `bec9389f62014dfecac8b41d330de142e0904b8c`.
Last reviewed commit: `bec9389f62014dfecac8b41d330de142e0904b8c`.
Review markers remain fixed until independent review.
Turn entry: `fefbf8a7`, **96/97**. Implementation: `764721cd`, **97/97**.
Previous goal turn: progress (five required failures resolved in evidence152).

## Design/spec alignment

| Group and owner | Data flow, complexity and validation |
|---|---|
| Complete, inherited naming/effects | Canonical entities and attributes → typed Itanium ABI → ELF. Naming151 controls rerun; evidence151 retained. |
| Complete, inherited EH/ownership | Canonical exception sets and persistent lifetime regions → typed filters/LSDA; precise imported VTT and local-static demand. Exceptions152 and ownership152 controls rerun; evidence152 retained. |
| Complete, virtual-primary layout | Class/base IDs → primary selection, nearly-empty/storage claims and prefix rows → projections, covariant thunks, RTTI, VTT/construction segments and host table groups. Graph/claim/row work follows actual identities; fixed tails are memoized, physical ordering O(v log v). No textual transport, lookup replay in lowering, optimizer pass or host implementation delegation. |

[Implementation trace](../student.tests/pa28/implementation153.md) documents
owners, publication/lifetimes, ABI proof and the explicit course/host layout
policies. Related displaced primaries, inherited receivers, VTT order, dynamic
base tail padding, alignment and imported table groups were completed together.
No known implementation failure remains in this group.

## Validation and performance

PA28 **97/97**, PA1–27 **4441/4441**, root through-PA28 **4538/4538**.
[Validation evidence](../student.tests/pa28/evidence153/validation.json) records all checks.
File audit passes with the same four inherited header warnings. Explicit naming,
EH, ownership and bidirectional host controls pass. Native inspection validates
LowIR, requires fixed/virtual thunk ABI names, and proves native-view/ordinary
object byte equality. Coverage remains 97 anchors and 20,288 tracked contract
paths; no tests, references, status sidecars or comparison rules changed.

[Performance evidence](../student.tests/pa28/performance153.md) retains 520 observations from frozen entry/final compilers,
fixed common inputs, A/A calibration and six ABBA blocks. Compiler latency/RSS
and checked runtime/text size are measured together; newly correct virtual
primary/secondary behavior has standalone measurements. Common objects and
executables are byte-identical. Paired compile medians range from .962 to 1.023;
substantial host contention and all outliers are disclosed. New feature costs:
0.7322 s / 68,188 KiB compilation, 0.1240 s runtime /
1,009,166 text bytes. No optimization benefit is claimed. Spec §9
stage-scoped acceptance applies: inherited 15%/zero-growth
diagnostics are not mandated gates. All measurements, mandated limits,
correctness and coverage are preserved. PA32/33 optimization and PA34
self-hosting remain later-stage obligations.

## Handoff ledger and review boundary

| Commit/evidence | Disposition |
|---|---|
| `fefbf8a7` and earlier PA28 commits | Inherited validated 96/97 handoff; evidence151/152 and their complete ledgers remain in history and student.tests. |
| `3f7bae40` | Recorded entry and layout owner before stage edits; preserved review markers. |
| `a5c53a88` | Completed the required layout failure and related physical sharing, prefix, receiver, VTT and table-group behavior. |
| `764721cd` | Preserved ordinary secondary vptr stores in base entries; constructor/destructor dispatch control proves the correction. |
| Final evidence commit | Required checks, explicit controls, native inspection, fixed performance observations and coverage inventory. |

The implementation boundary is the completed host virtual-primary/layout group;
no unfinished implementation is being recategorized as review. Whole-stage
independent audit remains pending for all PA28 commits: canonical publication,
demand/cache lifetimes, ABI layout policy, RTTI/table/VTT ownership, EH filters,
naming/substitution, source-to-ELF tracing and performance evidence. Neither
these questions nor whole-stage spec requirements are waived. This handoff
returns control to Ralph for its full audit and does not authorize advancement.
