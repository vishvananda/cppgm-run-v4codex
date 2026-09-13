# PA14 full-stage audit plan

Phase: **audit, open**. Stage base `8af3c149454e4e43e441206e6978f4d1300e079b`;
audit entry `78c2f13e`. PA15 has not started. The authoritative root report is
**314 PA14 + 1621 prior = 1935/1935**, all 14 stages; the supplied 1959 count
does not match the current checked-in report.

The independent review and remaining ownership work are in [audit.md](audit.md).
This plan supersedes checkpoint conclusions without discarding their evidence.

## Spec alignment and next work

The frontend streams tokens into one retained source graph. Compact occurrence
contexts, canonical types/specializations, immutable substitution frames and
indexed scopes support sparse concrete facts. Definition, body, lifetime,
default, layout and emission owners are separate. Typed lowering consumes
selected operations and ABI entries; PA14 ends at O0 LowIR.

The default-fact handoff and newly reviewed body-publication path are covered.
Remaining work, in ownership order:

1. Validate fixed source initializers and braced returns; reuse checked recipes
   in concrete initialization/lifetime consumers.
2. Finish query-only call/condition and temporary-destruction coverage and fact
   reuse; inspect complete keys and context-sensitive invalidation.
3. Close remaining whole-stage identity, demand, storage and typed-lowering
   questions, recording later-stage boundaries explicitly.
4. Freeze final performance evidence, run both required gates, consolidate the
   final audit and commit all intended work with an empty status.

No optional timing target replaces correctness or architecture. No mandated
limit, fixture, reference or comparison rule has been weakened.

## Evidence and handoff ledger

| Increment | Evidence / status |
|---|---|
| Source, signature, lifecycle and virtual owners through `460f495a` | Historical campaigns, 14,896 observations; independently traced in this audit |
| Default slots, demand, list readiness, elision and access through `b3927732` | `default-final-*.json`, 85 checks; 952 additional observations |
| Body and lifetime terminal publication, definition-time statements, fixed return recipes | 51 checks; 349 entry/current and sanitizer comparisons; 64 statement controls per build; nine repeated body-query cases per build |
| Current body performance | [532 observations](../student.tests/pa14/body-audit-performance.md); 16,380 cumulative; all nine executable payload sizes unchanged |
| Personal comparator adapter | Fixed Perl sort-variable shadowing; one allowed top-level order difference; course comparator unchanged |
| Remaining initializer/query ownership | Three reduced accepted-invalid inputs documented in audit; work remains open |

Artifacts: `$RALPH_ARTIFACT_DIR/pa14-final-audit/`. Both required gates passed
for the body change, with 1,266 fixture/reference hashes unchanged. These checks
do not close the remaining whole-stage defects.
