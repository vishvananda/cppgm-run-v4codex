# PA15 checkpoint plan

Stage base commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`
Last reviewed commit: `538cfcb00441f57c0629f6d27fddbad723539479`

Target: **PA15 full-stage**, O0 typed LowIR. Loop 33 starts at `3aff4801`,
**166/177**, with the same **11 failures** as the completed checkpoint audit.
The previous goal turn produced verified audit evidence and repairs (progress).
This implementation turn does not authorize advancing to PA16.

The first review covers every commit from the stage base through `538cfcb0`
(13 handoff commits and three audit commits), including combined source changes
and their interactions. [Audit](audit.md) records the complete range, findings,
architecture trace and ledger. [Verified evidence](../student.tests/pa15/checkpoint-evidence.json)
pins the exact failing set, fixture trees, reviewed sources and check artifacts.

## Reviewed ownership and fixes

Canonical typed arguments retain per-parameter pack boundaries and immutable
substitution frames. Explicit function specializations and function-address
selection now share typed signature deduction, retaining references, return
types and explicit prefixes. ADL and local ABI traversal visit pack elements;
class redeclarations preserve parameter-pack kind. Clearing a deduction binding
repairs its hash probe cluster without invalidating unrelated entries.

Signature substitution publishes expanded-list topology once. Scalar signature
and body consumers bypass pack expansion and its temporary vectors/scans;
empty packs retain their explicit empty-list fact. Source occurrences stay
shared, completion/body/storage facts stay separately demanded, and lowering
consumes typed semantic facts. The audit adds 11 native and 7 rejection controls
plus a collision/removal control for the shared index.

## Remaining required implementation

| Broad group | Remaining obligations |
|---|---|
| Dependent matching and aliases | Four fixtures: dependent bool traits, dependent typename, alias-parameter overloads, and type-equivalent defaults. Establish selected-pattern and dependent alias facts; handout exclusions do not waive checked fixtures. |
| Constant objects and initialization/storage | Six fixtures: aggregate braced casts, dependent conversion operators, constexpr locals after qualified types, static constexpr reference replay/call initializers, and stale function initialization output. Keep constant execution, initializer recipes and storage demand separate. |
| Ordinary source validation | One fixture: unused ordinary member `static_assert`. Validate ordinary bodies independently of emission, including explicit-class members; do not eagerly instantiate unrelated template bodies. |

Keep each group open through its dependent consumers and rejection controls.
Loop 33 sequence: dependent matching and alias consumers, constant objects and
their storage/lowering consumers, then ordinary body validation. Class-pattern
selection belongs to the primary's indexed candidate family; deduction produces
typed bindings consumed by the selected definition. Dependent aliases retain
typed qualified-name queries. Constant execution consumes checked expression
facts, while storage and emission remain separately demanded. Ordinary body
validation belongs to declaration completion, not emission demand. Work must
track family candidates, dependent nodes and emitted actions, with TU-owned
facts and local scratch; no global rescans or source replay. Validate each group
with existing failing fixtures plus explicitly run personal/native/rejection
controls, then required root checks. Freeze A/B binaries and inputs for A/A and
ABBA compiler latency/RSS and executable runtime/text evidence; no optional
optimizer or unsupported numerical performance gate is introduced.
The three earlier handoffs had useful broad ownership boundaries, but splitting
selection from packs without testing their composition missed pack specialization,
ADL and target-signature bugs. Separate scalar performance follow-ups also left
redundant work in the same parameter owner. Avoid another handoff until related
consumers and their interactions have been checked; use no progress quota.

## Validation and stage-scoped acceptance

`make test-pa15`: **166/177**, exit 2, identical 11 entry failures.
`make test-report-through-pa14`: **1935/1935**, exit 0.
`perl scripts/cppgm_file_audit.pl --stage pa15 --paths dev/src`: **pass**, the same
three inherited header warnings. All existing personal suites and the new audit
controls pass. Fixture coverage, references, bundle, comparison rules and
attribution are unchanged.

[Performance](audit-performance.md) preserves 2,380 invocations across 11 frozen
campaigns, including A/A, ABBA, compiler latency/RSS, runtime/text and counters.
Measured compiler tip `034e3b91` has exactly the reviewed tip's `dev` tree.
The avoidable scalar work was removed. PA15/O0 mandates correctness and bounded
work/lifetimes, with no numerical latency/RSS/text ceiling or optional optimizer.
Historical self-selected targets remain diagnostic; all measurements and actual
requirements remain intact. Native optimization, allocation and self-hosting
keep their later-stage ownership. Finish all remaining PA15 fixtures and the
root through report before advancing.
