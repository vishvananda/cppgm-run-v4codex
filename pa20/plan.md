# PA20 compact plan — implementation handoff 100

Stage base commit: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Last reviewed commit: `882cf5236a8ddb756403105cd50135440920e2a4`.
Target: **PA20 full-stage**. Phase: **implementation handoff; stage tests pass**.
Entry: `2e31ab57e7ba3371ca5c59057ede55b435d4a666`, clean, **143/144**.
Implementation boundary: `7925464d` (following `db0f82f1`). Final records only
follow this boundary. [Audit 97](audit.md) and its markers remain in force.
Previous goal turn: **progress**, established by the aggregate/value changes
and their validation. No previous task process remained live at entry.

## Design and spec alignment

| Owner | Data flow and invariants | Complexity / lifetime |
|---|---|---|
| Retained grammar / declaration environment | Parser category alternatives and angle locations → actual statement scope → canonical specialization/member lookup → selected source grammar → ordinary typed facts/LowIR. The first local declaration changes the next lookup; the apparent second declaration becomes the hidden-friend comparison. | Linear names/parts/operand structure plus existing canonical demand. Source interpretation precedes publication, is shared across specializations and cannot mutate a published region. No replay, copied tree, textual key or lowering reconstruction. |
| Delimiter prediction | Specialization categories remain alternatives independent of source order. A failed angle probe marks all its still-open prefixes on the live cursor. | Linear scanning; successful/failed lookahead is reused. Cursor-owned state disappears with consumed tokens. Lexical hiding remains separate from specialization alternatives. |
| Inherited completed PA20 owners | Deduction, arrays, range lifetime/operations, lambda captures/callable entries, aggregate transport and separate argument/result ABI facts feed typed lowering. | Prior ownership, demand and expansion bounds remain; evidence and review obligations in handoffs 94–99 are preserved. |

Inspection replaced the proposed body-scheduling change with the existing
source-ambiguity boundary. The implementation extends that owner through
specialization order, aliases, lexical/control scopes, functions, members,
lambdas, template reuse, operator precedence, multiple arguments and ordinary
operand forms. Interpretation uses the shared operator precedence table.
Unknown/dependent facts retain the existing dependent-language checks; no
specialization reparses grammar. No fixture or reference was changed this turn.

## Validation and performance

[Final validation](../student.tests/pa20/validation100.json): required PA20
**144/144**; exact prior-through command **3452/3452**; through PA20
**3596/3596**; file audit **pass**, same three inherited header warnings.
Required failures fall **1→0**, with no new failures and unchanged coverage.
The manifest checks all 639 fixture/contract/harness files; all 144 sources,
statuses and comparisons are unchanged since entry. Historical reference
revisions still match their preserved reducers and proofs.

Explicit personal controls **344/344** (303 inherited plus 41 new), four ABI
checks, six source-to-native traces, historical reference proofs/executions
and both versions of the repaired required program pass. The new trace checks
one grammar interpretation reused by two specialization bodies, four calls,
source-publication invariants and telemetry-independent output.

[Performance evidence](performance100.md): frozen entry/final/input hashes,
316 observations/34 warmups, A/A and four ABBA blocks, compiler latency/RSS and
checked native runtime/payload. Qualified workloads show 1.2–4.2% median
compiler growth and 160–224 KiB peak-RSS growth; compiler text grows 0.712%.
Every equivalent A/B native executable is byte-identical. New grammar work is
constant across 800/3200 body specializations; 40/600 negative angle controls
show exactly proportional visits. No runtime optimization benefit is claimed.
No new numeric gate replaces the stage-scoped acceptance or mandated bounds.

## Remaining implementation and independent review

**Unfinished implementation:** none identified under the shipped PA20 contract;
no required failure or owner defect is deferred to PA21. This boundary completes
the implementation handoff, not the assignment's independent audit.

**Independent review, still mandatory:** review source grammar publication and
projection, category alternatives/lexical hiding, argument reassociation and
work bounds; review handoff 98 capture changes and handoff 99 transport/cache,
helper sharing, result ABI and reference proof; resolve cumulative whole-stage
architecture/performance findings before advancement. Passing tests does not
waive these questions or change the preserved review markers.

## Handoff ledger

| Boundary | Completed work / evidence | Unfinished implementation | Independent review |
|---|---|---|---|
| Audit 97 | [Preserved ledger](audit.md#ledger), 121/144 | Original 23 failures | Reviewed through `882cf523`; markers retained |
| Implementation 98, `e75e0d6c`→`a1faea7a` | Capture environments/conversions; 140/144, 264 controls; [measurements](performance98.md) | Aggregate/value and retained grammar (4 failures at that boundary) | Pending cumulative review |
| Implementation 99, `16ac49da`→`37f8300b` | Aggregate transport/result ABI/member-copy proof; 143/144, 303 controls; [measurements](performance99.md) | Retained grammar (1 failure at that boundary) | Pending cumulative review |
| Implementation 100, `2e31ab57`→`7925464d` | Semantic angle interpretation and related grammar; 144/144, through 3596/3596, 344 controls, six traces, file audit and [measurements](performance100.md) | None identified under the course contract | Full independent stage audit required; no advancement claimed |
