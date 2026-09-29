# PA20 compact plan — checkpoint audit 97

Stage base commit: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Last reviewed commit: `882cf5236a8ddb756403105cd50135440920e2a4`.
Target: **PA20 full-stage**. Phase: **checkpoint audit complete; stage incomplete**.
Implementation 98 entry: `e75e0d6ce1e26a2d50bc0857cd620974c6ee451d`,
121/144 passing. Capture environments are the initial shared owner (17 failures):
closure occurrence plus enclosing specialization owns pointer fields and indexed
capture edges; checked expressions publish field/receiver identities for direct
typed lowering. Nested closures forward those edges through lexical parents.
Work/storage must track captures and actual lexical edges, with no syntax replay
or name lookup in lowering. Validate required fixtures, executable local/this,
nested/pack/copy/access controls, and frozen compiler/native performance evidence.
Extend into adjacent conversion/lifecycle groups where the same facts suffice.
Reviewed the entire base-to-tip range: all ten implementation commits across
handoffs 94–96, their four boundary/evidence commits, and audit fix `882cf523`.
The following records-only commit is outside that code boundary.

## Findings and validation

The [audit](audit.md) reconstructs ownership, cross-handoff interactions,
reference proof and remaining scope. The audit fixed delayed aggregate stores:
later clauses could read an earlier scalar/array member before a helper wrote
it. Semantic initialization now publishes a cached storage-independence proof;
lowering uses ordered destination construction when that proof is unavailable.
Single-argument helpers and proved empty scalar-field constructors preserve the
earlier ABI shapes. Seven new executable reducers failed at entry and pass now.

[Required checks and all controls](../student.tests/pa20/audit97-validation.json):
PA1–19 **3452/3452**, file audit **pass** (three inherited header warnings),
PA20 **121/144**, through PA20 **3573/3596**. The **same 23 failures** remain;
there are no additional failures and all 144 fixtures/comparison rules remain.
Personal controls **206/206**; three inherited source-to-native traces and two
additional encoding/containment inspections pass. No reference was changed by
this audit; the eleven proved handoff-95 corrections were reconstructed and
their original/revised outputs executed again.

[Performance](performance97.md) retains **1068 observations / 114 warmups**,
frozen stage-base/checkpoint/final binaries, A/A calibration and ABBA pairs.
Compiler text grows 2240 bytes over the checkpoint. All nine comparable
checkpoint/fix executables are byte-identical. Initializer proof work is
linear (6400/25600 for 800/3200 array specializations); canonical array
classification remains one. Required PA20/O0 helper costs, deduction costs,
RSS changes and timing noise are disclosed. Historical percentage targets are
diagnostics, not additional gates under spec §9. Required limits, correctness
and coverage are unchanged; native optimization/self-hosting remain later work.

## Remaining implementation groups

| Group | Required work / ownership | Current failures |
|---|---|---:|
| Capture environments and composition | Closure occurrence/enclosing specialization owns local/this fields, nested rebinding, special members, access and pack contexts | 17 |
| Aggregate construction ABI | Complete member copy/omitted-class construction and lifetime/helper contracts; preserve the newly proved initializer ordering | 2 |
| Constructor conversion integration | Complete overload/deduction and constructor argument representation for closure/template/wrapper conversions; prove any reference correction first | 2 |
| Retained declarations and lifecycle | Complete local declaration/probe environments and dependent-owner lifecycle facts | 2 |

Deduction, array-expression completion, typed ranges and captureless callable
entries are reviewed implementations, with the controls above retained for
future changes. Remaining groups are mandatory PA20 work, not PA21 deferrals.
Do not advance until `make test-pa20` and `make test-report-through-pa20` pass.

The three handoffs cover real owner boundaries, but their ten code commits
include avoidable fragmentation: range arithmetic/destination corrections and
the array classifier follow-up should have been integrated with each owner's
composition/performance validation. Finish and validate the broad groups above
instead of treating each exposed interaction as a new milestone. The audit's
single ledger row is in [audit.md](audit.md#ledger).
