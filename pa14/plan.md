# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture work remains.
Stage entry **84/314**; this continuation entry/current **314/314**.
All **230 original failures** are resolved. Coverage and reference/comparison
rules are unchanged. PA15 has not started.

## Design/spec alignment and remaining groups

Canonical declarations/types/arguments feed ordinary semantics and typed LowIR.
The previous transfer/lifetime group completed the course contract. This turn
adds definition-owned fixed scalar expression types, categories, constants and
conversion slices. Concrete occurrences map local declaration identities and
record evaluation/reference/mutation effects without repeating fixed operand
validation. See [ownership and bounds](implementation.md).

| Remaining current-PA14 owner | Data flow, complexity and validation |
| --- | --- |
| Typed template body graph | Extend shared facts to calls/class operations and declaration/return/default conversions. Substitute dependent edges only; replace full-region projection. Validate materialization, access, unused-body legality and source/context provenance with native/scaling checks. |
| Demand/failure graph | Separate layout/default/exception/body states, typed reasons/reverse edges and structured expected failure. Work follows demanded facts/edges; validate cycles, narrow negative-cache keys and unrelated-declaration scaling. |

The fixed scalar group includes arithmetic, assignments, conditions, casts,
sizeof/alignment and subscript expressions, with twelve new unused-body
rejections. Calls require selected-callee/argument facts plus occurrence-owned
materializations and emission demand; simply copying scalar records would lose
those effects. That is the next concrete owner boundary. Entire source regions
still project (76 occurrence nodes per specialization in the new corpus).
Course success does not waive these architecture requirements or defer them.

## Performance evidence

The new frozen A/A+ABBA campaign measures fixed-body specialization reuse and
unused-definition validation alongside prior calls, memory, floating-point and
query workloads. Compiler and executable timing are separate; all observations
and outliers are retained. Current compiler text grows 4,608 bytes (0.38%);
expression/declaration records remain 36/112 bytes. The fixed corpus computes
20 shared expression facts, versus repeated selection before this change.
The three new campaigns retain 1,008 observations; **4,718 total** are verified.
At 4,000 demanded specializations, conversions fall 80,000→24,014 and peak RSS
falls 2,354 KiB. Final 1,000-instance pairs improve 5.1–7.6%; larger timing is
mixed. Required unused-definition checking adds 52.8 ms/6,256 KiB at 4,000
patterns. Calls-4 retains a higher wall median and timing spread; no general
compiler speedup is claimed. All 19 compiler/five native outputs are identical.
See [performance.md](performance.md) for paired results, costs and bounds.

Earlier **3,710 observations** remain preserved. The transfer campaign showed
about 3× reference-move runtime benefit for 12 payload bytes and 12.2–13.7%
late-copy benefit with 44 fewer bytes; its compiler/memory costs remain recorded.
O0 adds no optional optimizer/native backend and has no mandated numeric compiler
threshold. Historical self-imposed gates remain diagnostics; preserve mandated
bounds, correctness, coverage and all evidence.

## Handoff ledger

Entry `c05778ed`: the previous transfer/query turn is **verified progress**,
314/314 with 1935/1935 through-stage validation. Its implementation/evidence
commits and measurements are retained in implementation.md and performance.md.

| Increment | Commit / validation |
| --- | --- |
| Fixed scalar types/conversions and definition-time legality | `b22e683f`: 314/314, through 1935/1935; fourteen native programs and twelve new rejections pass |
| Frozen body performance protocol | `d0030349`, `ced1c0d6`, `973b9928`: exact A/B output and native equality; both preliminary campaigns retained |
| Direct expression-result construction | `d19a1ff7`: through 1935/1935, native/sanitizer checks pass; final frozen campaign |

Release/ASan/UBSan status/output parity passes on **328 inputs**, plus 31 sanitizer
rejections. Parity includes expected rejections, not extra course passes. The
file audit passes with the same three inherited header advisories. Current
frozen binaries, commands/logs and evidence live in
`$RALPH_ARTIFACT_DIR/pa14-body-facts/`; the preceding transfer directory remains
unchanged. Required stage/prior checks, through report, native/sanitizer checks, file audit
and final evidence verification pass. The scalar group is complete; the
call/materialization and dependent-body graph boundary above remains current-stage
work. All intended changes are committed at handoff, with a clean tree.
