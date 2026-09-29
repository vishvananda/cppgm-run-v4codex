# PA20 compact plan — implementation handoff 99

Stage base commit: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Last reviewed commit: `882cf5236a8ddb756403105cd50135440920e2a4`.
Target: **PA20 full-stage**. Phase: **implementation handoff; stage incomplete**.
Entry: `16ac49da1339edaa4a5f96bf4efab731a30c65e1`, clean, **140/144**.
Implementation boundary: `37f8300b` (following `a7265134`, `b8b9e1ff`).
The final records commit adds controls/evidence only; no implementation follows
that boundary. [Audit 97](audit.md) and its review markers remain in force.

## Completed owners and spec alignment

| Owner | Data flow and invariants | Complexity / lifetime |
|---|---|---|
| Aggregate initialization | Completed constructor/expression summaries plus selected list conversions → explicit representation-transport fact → shared helper or ordered destination construction. Excludes self-addresses, aliases/member reads, volatile state, nontrivial copies/destruction and unknown effects. No extra language moves. | One summary per completed canonical constructor / expression occurrence; average O(1) lookup, linear fields/arguments. TU-owned flat facts; helper slots are local LowIR identities, independent of caller temporaries. |
| Class value boundary | Completed copy/move/destructor facts → separate parameter and result conventions → signatures/calls/destinations. Small copyable results stay direct; a trivial move alone only permits direct argument transport. | Existing once-per-class facts and demand states; no new query, body demand, cache invalidation or semantic reconstruction. |

These extend the initial aggregate group through nested returns, defaulted
special members, template owners, defaults, lambda results, omitted class tails
and the preserved audit-97 sequencing proof. Constructor summaries reject
`this`; the new native control exposed and verified that correction. Unknown
or not-yet-completed facts retain ordered construction. No token replay, copied
semantic tree, rendered identity key or optional optimizer was added.

One oracle inserted two observable moves after the required two member copies.
The [C++11 proof, reducer and bundle revision](reference-corrections99.md) correct
that `.ref` only. Its source, success status and comparison rules remain intact.
The correction is separate from the two implemented course-shape fixes.

## Validation and performance

[Final validation](../student.tests/pa20/validation99.json): PA1–19 **3452/3452**;
file audit **pass** (same three inherited header warnings); PA20 **143/144**;
through PA20 **3595/3596**. Required failures fall **4→1**, with no new failures.
All 144 fixture sources and comparison rules remain; the coverage manifest
checks 639 files and verifies every stage reference revision against its proof.
Personal controls **303/303**, four explicit ABI checks, five inherited/current
source-to-native traces, all reference reducers and all three repaired required
program executions pass. The independent assignment audit is not implied.

[Performance evidence](performance99.md): frozen entry/final compiler and inputs,
400 observations / 40 warmups, A/A plus four ABBA blocks, compiler latency/RSS
and checked native runtime/payload size. Compiler text grows 1728 bytes (0.083%).
The required aggregate helper costs runtime and output size; repeated helper
occurrences share one body, and work/output remain linear in demanded identities.
Result storage removes copies but noisy timings support no strong speed claim.
Unchanged call/memory/floating controls produce identical native bytes. Existing
expansion bound 8 and stage-scoped acceptance apply; historical diagnostic
percentages waive neither correctness, coverage nor mandated limits.

## Remaining implementation (mandatory)

**Parser / retained declaration environment: one required failure**,
`400-repeated-local-declaration-template-probe.t`. The first declaration changes
`sizeof(local_args)` and thus the selected specialization. The second apparent
declaration is actually a relational expression calling the hidden friend:
its `cmp1` is an integer, not a template. The oracle correctly contains one
local object and an `operator>` call.

`Parser::translation_unit` currently delivers a whole declaration/function to
semantic consumption, while its category table merges specialization member
categories. Resolving this requires semantic specialization and local-entity
facts during grammatical classification (or a retained structural ambiguity),
with one grammar pass and correct lexical scope publication. The completed
initializer/ABI owners run after that decision and cannot supply those facts.
A repair inside them would mask the parser defect; correct work requires a new
parse/semantic cooperation boundary. This is the concrete boundary of this
handoff, not a PA21 deferral or a waived exit criterion. No further known
correctness defect remains in the completed aggregate/value groups.

Independent review questions, separate from that implementation: validate the
transport proof/cache stability, helper sharing across occurrences, the result
ABI envelope and the reference proof; review capture changes from handoff 98
and the cumulative base-to-tip performance interpretation. All remain pending
Ralph's independent review, with no assignment advancement claim.

## Handoff ledger

| Boundary | Completed work / evidence | Unfinished implementation | Independent review |
|---|---|---|---|
| Audit 97 | [Preserved ledger](audit.md#ledger), 121/144 | Original 23 failures | Reviewed through `882cf523`; markers retained |
| Implementation 98, `e75e0d6c`→`a1faea7a` | Reference/this/nested/pack captures and conversion proofs; 140/144, 264 controls; [evidence](performance98.md) | Aggregate initialization, class ABI and retained grammar (4 failures) | Pending; not superseded by this handoff |
| Implementation 99, `16ac49da`→`37f8300b` | Proven class representation transport, separate result ABI, member-copy proof; 143/144, 303 controls, required prior/audit checks and [measurements](performance99.md) | Lookup-sensitive parser/semantic cooperation (1 failure) | Pending cumulative audit; full stage audit still required before advancement |
