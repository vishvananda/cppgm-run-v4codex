# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture work remains.
Stage entry **84/314**; continuation entry **297/314**; current **314/314**.
All **230 original failures**, including all **17 continuation-entry failures**,
are resolved. Coverage and reference/comparison rules are unchanged.

## Design/spec alignment and remaining work

Canonical declarations/types/arguments feed ordinary semantics and typed LowIR.
The query/binding work now reaches transfer, layout, lifetime and ABI consumers:
reference binding actions, empty payload handling, late-defaulted copy facts,
return destinations, base entries, local type identities and reentrant constants.
Static vptr facts and bounded deleting entries consume prepared constructor and
destruction actions. Calls/operators share emission-use facts; O0 expressions
retain source conversion/condition provenance. See [ownership](implementation.md).

| Remaining current-PA14 owner | Data flow, bounds and validation |
| --- | --- |
| Typed template body facts | Shared fixed types/conversions and dependent-only checking; replace whole-region semantic projection. Work must track pattern facts plus genuinely substituted facts. Extend query/bound/unused-body reducers and scaling checks. |
| Demand/failure graph | Finer layout/default/exception/body states, typed reasons/reverse edges and structured expected failure. Validate cycles, narrow negative-cache keys and unrelated-declaration scaling. |

The transfer/lifetime group was extended through every remaining course failure.
Further action-list or lookup patches cannot replace whole-body fact projection:
that needs a typed body graph spanning occurrence, expression and demand owners.
This is the concrete incomplete boundary. Passing the course suite does not
waive these `spec.md` requirements or defer them to PA15.

## Performance evidence

[Performance review](performance.md) preserves **3,710 verified observations**,
including 1,050 new A/A+ABBA observations and all outliers. Earlier campaigns remain frozen; they exposed unused deleted-copy preparation,
and the final volatile-return boundary fix has its own repeated measurement.
The negative-fact correction removes 4,000 entities/scopes and 8,000 unused actions from
the 4,000-specialization B workload. Final fourfold transfer input gives 4.11×
wall, 3.71× RSS and exactly 4× instructions/actions; fixed binding work stays 23.
Reference-move runtime is about 3× faster for 12 payload bytes; late-copy runtime
improves 12.2–13.7% and loses 44 bytes. Compiler text grows 6,784 bytes (0.56%).
The largest transfer compile retains a 1.5% higher wall median and 3,246 KiB
higher peak RSS median than entry. No general compiler speedup or isolated
negative-fact speedup is claimed. Declaration/expression records remain
**112/36 bytes**; array expansion and deleting-entry sharing retain their bounds.

O0 adds no optional optimizer or native backend, and has no mandated numeric
compiler threshold. Historical self-selected gates remain diagnostics; all
measurements, correctness, coverage and mandated bounds remain requirements.

## Handoff ledger

Continuation entry `c05778ed`: previous turn is **verified progress**, with
314/314 course tests and the architecture boundary retained. The next owner is
definition-owned scalar expression facts: binding establishes fixed operands,
ordinary semantics publishes types/conversion slices once, and specialization
projects only declaration identities and operand consumption. Work is O(pattern
expressions + demanded operand occurrences), with no repeated fixed conversion
selection. Validate unused-body legality, shadowed/nested locals, volatile and
reference effects, course/native/sanitizer parity, and frozen A/A+ABBA scaling.

Entry `2c80bc70`: previous query/binding turn is **verified progress** (297/314).

| Increment | Commit / validation |
| --- | --- |
| Transfers, return slots, recursive layout and local ABI | `c5c4328a`: 306/314; prior 1621/1621; native reducers pass |
| Lifetime entries, expression provenance, retained syntax | `2818c8e2`: 314/314; prior 1621/1621; thirteen native programs pass |
| Emission facts in declaration padding | `6fb57328`: through report 1935/1935; 327 output/status checks unchanged |
| Known-deleted transfer completion | `177f7545`: through report 1935/1935; native/sanitizer checks pass |
| Volatile return eligibility | `c7507efd`: non-volatile source rule; reduced native failure fixed; through/native/sanitizer checks pass |

Final required PA14/prior checks and file audit pass. Thirteen native programs,
12 binding rejections and nine query/ABI checks pass. Release/ASan/UBSan parity
passes on **327 inputs**, plus 19 sanitizer rejection checks. Parity includes
expected rejections; it is not a count of extra passing course tests. File audit
has the same three inherited header advisories. Exact commands/statuses, frozen
binaries, layouts and progress ledger: `$RALPH_ARTIFACT_DIR/pa14-transfer/`.
Final evidence verification passes. All implementation and evidence changes are
committed at handoff; the tree is clean. This is verified course-contract
completion with the explicitly listed current-stage architecture work remaining.
