# PA15 checkpoint plan

Stage base commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`
Last reviewed commit: `538cfcb00441f57c0629f6d27fddbad723539479`

Loop 35 entry: `d705aafc4845cca2ad645b8c7ea45194936c9180`, clean,
173/177; implementation now **177/177**, through PA15 **2112/2112**, and 24
native + 18 rejection execution controls pass. Performance and final evidence
are still pending in this working checkpoint. Previous turn: progress (committed initialization work and verified
reports). Implement the remaining three owners together: checked constant-call
execution (selected call/conversion -> immutable activation -> integral result),
static member definition/storage (concrete class -> indexed member definition ->
storage demand), and ordinary body validation (complete-class queue -> checked
body, independent of emission). Retain template-body laziness and source parsing
once. Work must track demanded bodies/activation expressions and indexed edges;
cache keys include function, arguments and object/environment facts. Validate all
four existing failures, personal positive/rejection/native controls, stage and
prior reports, file audit, and frozen compiler/runtime measurements. Constant
execution extends only the fixture-required subset; general PA16 execution stays
with its owning stage. Independent whole-stage review remains outstanding.

Target: **PA15 full-stage**, O0 typed LowIR. Loop 34 enters at `b8379f52`,
**169/177**, and implements **173/177**: three existing failures fixed in code,
one proven reference defect corrected, four failures remain. No input, exit
expectation, comparison rule or fixture coverage was removed. The preceding
goal turn was progress, evidenced by its committed matching changes and report.
This is an incomplete implementation handoff, not a whole-stage audit or approval
to advance. Preserve [the accumulated audit](audit.md) and both markers above.

## Completed owners and spec alignment

Initialization requires class completion before reading aggregate/constructor
properties. The same prerequisite protects omitted aggregate elements. Derived
value initialization now includes the base and consumes the existing cached zero
plan, including ABI-specific member-pointer representations and volatile stores.
No member bodies are instantiated merely to classify a class as an aggregate.

Retained body queries now model builtin prefix/postfix increment and decrement,
including cv, categories, pointer completeness and invalid enum/const/bool cases.
The existing selected candidate/conversion facts own these results; postfix's
integer operand is part of its canonical query key. The replay fixture was an
increment-query rejection, not stale static constexpr storage.

Constexpr array declarations validate their typed initializer plan once. Scalar
constants, static-duration addresses, nested aggregates, strings and repeated
omitted elements use existing static-value/type facts. Failed scalar constant facts,
temporary/automatic/thread addresses and nonliteral elements are rejected.
Volatile subobjects retain observable stores. Each qualifying local object has
at most one readonly backing image per TU. **O0 backing budget: 32 bytes/object**;
larger arrays retain direct initialization after measured copy regressions.
The LowIR validator treats symbol values as addresses and checks declared storage
types at load/store operations. Text remains an explicit adapter, not a phase
transport. [The aggregate reference correction](reference-corrections.md) includes
a nontrivial-copy reducer, C++11 proof, bundle revision and original/revised hashes.

Work is O(initializer actions + emitted data/instructions + required candidates).
An omitted run is one action, not a bound-sized validation loop. TU-owned flat
indexes key plan classification by immutable action ID and backing by object ID;
failed classification is retained, with no global invalidation or retry. Backing
data is bounded by 32 times the number of qualifying objects. Existing source
regions, canonical types, substitution frames and direct typed lowering are reused;
all new maps and backing records release with the TU. No optimizer was introduced.

## Remaining required implementation and boundary

| Owner | Required unfinished behavior |
|---|---|
| Constant execution | Two fixtures: constexpr conversion of a constant object into a bool template argument; constexpr function call in a static member initializer. Execution must consume checked bodies and selected conversions with complete activation/environment keys. |
| Member storage demand | One fixture: publish the defined static constant required by the concrete class-object output. Storage/definition demand must remain separate from dormant nonvirtual member bodies. |
| Ordinary body validation | One fixture: reject a false static assertion in an unused ordinary member. Check ordinary and explicit-class bodies independently of emission, retaining dependent template-body laziness. |

The initializer/type-completion, array storage and retained-update-query group is
complete, including nested/omitted/volatile/address/lifetime controls and the
reference reducer. Further repairs require execution frames or body-validation,
storage and emission dependency facts. Initialization actions contain neither;
forcing dormant bodies or evaluating unchecked syntax would violate the spec.
These are concrete separate owners, not additional matching/initializer cases
that can be completed with the same visitor. They remain implementation duties.

Independent review remains outstanding for the combined stage changes after
`538cfcb0`, including constant/storage interactions and whole-stage architecture.
The implementation checks and reference proof do not waive that review or any
remaining fixture. No new independent architecture investigation was opened.

## Validation, performance and handoff ledger

Required reports: PA15 **173/177** (exit 2; failures **8 -> 4**), prior PA1–PA14
**1935/1935** (exit 0), file audit pass with three inherited header warnings.
Personal initialization controls: **23 native + 13 rejection + 3 LowIR**. Also run
explicitly: 26 value groups, 10 constant groups, 24 specialization native + 13
rejections, 27 pack native + 8 rejections, 15 matching native + 7 rejections,
11 checkpoint native + 7 rejections. Root reports run sequentially because their
shared count sink mixes concurrent invocations; the mixed observations are kept
in scratch and are not used as coverage evidence. No PA15 native/debug gate is
added; its required LowIR validator remains active.

[Initialization performance](initialization-performance.md) records all preliminary,
address-validation and final frozen A/A/ABBA latency/RSS/runtime/text observations,
scaling counters, budgets and the measured reason for the 32-byte backing limit.
PA15/O0 has no mandated numeric latency/RSS/text ceiling. Historical measurements
and diagnostic targets in [matching evidence](matching-performance.md),
[audit performance](audit-performance.md) and prior handoff artifacts are retained.
Native backend optimization and self-hosting remain later-stage responsibilities.

| Handoff | Implementation / independent review boundary | Evidence |
|---|---|---|
| Checkpoint 32 | Reviewed through `538cfcb0`; matching, constants/storage and ordinary validation unfinished | 166/177; prior 1935/1935; [audit](audit.md) |
| Loop 33 | Matching/parser and ordering/environment ownership; reviewed marker unchanged | 169/177; [matching handoff](../student.tests/pa15/matching-handoff.json) |
| Loop 34 | `1aa393c9` initialization/query/array facts; `11e75a50` proven reference correction; `3b2475f8` storage-duration checks; `4f4bea42` measured O0 backing bound; four implementation failures and independent review remain | 173/177; prior 1935/1935; [verified handoff](../student.tests/pa15/initialization-handoff.json) |
