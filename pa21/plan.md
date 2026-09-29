# PA21 compact plan — implementation 108

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `f65eae8d7d434735a0ce981173a347eb8f8a1e59`.
Target: **PA21 full-stage**. Phase: **implementation**.
Entry: clean `a2b9e823800997ebbe57263a6ea2306e529698be`, **101/116**.
Current: **108/116**, **8 remaining**, **7 original failures resolved**.
Earlier PAs: **3596/3596**; through PA21: **3704/3712**. File audit passes.
No source cases or comparison rules removed.
This is an implementation handoff, not whole-stage completion or advancement.

## Design/spec alignment and completed group

The cumulative typed semantic graph feeds the direct LowIR builder. Required
output is generated here; supplied native backends are validation tools only.
The [105 audit](audit.md) and all 102–107 evidence remain applicable records.

| Owner / data flow | Complexity and validation |
|---|---|
| Class returns, cleanup: eligible local identity → caller result storage with callee unwind ownership until successful return. | Linear return edges; cached remaining-prefix/result cleanup. Observable/const/polymorphic results, multiple returns, handlers, and earlier/later throwing destructors execute correctly. |
| Function body, `exception_boundary`: selected call signatures and deallocation boundary → explicit outer termination region and one program-owned adapter. | One scan/rebuild of the current function's contiguous instruction slice; scratch dies immediately. Implicit/explicit `noexcept`, templates, defaults, parameters, subobjects and nested handlers checked. |
| Initializer plans, `aggregate_lifetimes`: completed canonical subobject type/address → immutable reverse cleanup prefix; retire only after aggregate completion. | Exception facts cached by retained plan/constructor identity; suffix facts avoid unnecessary saved addresses. Existing eight-element expansion cap; counted loops above it. Member, nested, copy and array failures, including reverse order and the 8/9 boundary. |
| Object/array/constructor/helper lowering: successful destination → cleanup ownership before argument-temporary destruction. | Rebase only the new temporary suffix; old constructor-failure snapshots remain valid. Default array temporaries end before the next element. Constructor raw-region changes get distinct resume identities. Local, heap, member, delegated and helper failure controls. |
| Semantic helper eligibility: completed constructor actions + proof mode → private-object operand proof and commuting transfer fact. | Cached by canonical constructor plus mode; linear action/operand traversal. Real copies preserve self pointers; aliases, external effects and throwing early transfers retain ordered lowering. |
| Parser/list/LowIR owners: declaration prefix → correct `T(*this)()` expression; empty aggregate plan → checked zero initialization; slot layout → bounded `zeroinit`. | No fixture recognition. Nested local-template fixture, empty switch returns, volatile/pointer/member-pointer initialization and LowIR validation checked. |

[134 execution controls](../student.tests/pa21/ownership108.py) pass, including
failures discovered during the extension from result ownership through partial
construction and default-argument cleanup. [Reference correction 108](reference-corrections108.md)
proves the missing completed-member cleanup using C++11 [except.ctor], a reduced
source and original/revised native execution. Reconstruction never reads student
output. Earlier corrections [102](reference-corrections102.md) and
[106](reference-corrections106.md), bundle identity and all input coverage remain.

## Remaining implementation and concrete boundary

- **Source handler composition/support identity (3):** nested catch-miss,
  guarded-static initializer, and handler-context continuation comparisons still
  need region/handler-exit placement and RTTI support-global pairing changes.
- **Initializer-list backing ownership (2):** global backing-storage duration
  and local backing-array lifetime need retained backing views/addresses and
  matching construction/cleanup region placement.
- **Generated special-member transfer (2):** template-member temporary cleanup
  and shared-call hidden temporary need inherited member/prologue and omitted
  trailing-class helper action changes.
- **Constant-array O0 policy (1):** function-template/local-class specialization
  output uses a readonly aggregate copy where this fixture expects stores.
  Earlier PA17 requires the same readonly-copy shape for a similar ordinary
  array; a global replacement would regress the cumulative contract.

108 completed the destination lifetime group and extended it through aggregate
helpers, small/counting arrays, allocation, constructor members and delegation.
The remaining owners retain different facts: backing-object identities, generated
special-member actions, source-handler continuations and constant-array policy.
Changing the completed prefix or result scheduler cannot supply those facts.
Further fixes require new owner-specific data-flow and contract baselines; this
is the coherent boundary, not a waiver of the eight failures or spec requirements.

Independent review questions: audit helper independence/commutation legality,
exception-fact key completeness, immutable prefix rebasing, result transfer and
constructor-region terminal identity. These differ from the known implementation
failures above. Preserve the review marker; Ralph owns the independent audit.

## Validation and performance

[Validation](../student.tests/pa21/validation108.json) records sequential required
gates, all 116 original inputs, exact failure subsets and 15,048 fixture/harness
hashes. [Through report](../student.tests/pa21/through108.json) retains the final
scope. Explicit personal suites pass **558/559**: all 134 new ownership controls,
six `zeroinit` acceptance/rejection checks and inherited controls except the known
freestanding RTTI backend discrepancy; its host-runtime counterpart passes.
No course failure is waived by this inherited personal-suite distinction.

[Performance](performance108.md) pins A/B binaries, sources, A/A calibration,
four ABBA blocks, compiler latency/peak RSS and checked runtime/size. Twelve
established native outputs are byte-identical. Named-result runtime B/A is
0.476–0.501; default-array runtime 0.624–0.710. Required member-prefix `.text`
growth and noisy compiler observations are disclosed. Work/growth is linear on
the measured 256/1024-function curve. Supplied sectionless ELF payload is labelled
as a proxy; compiler and hosted executable `.text` are measured directly. The
interrupted freestanding RTTI attempt and continuation are preserved. No optional
optimizer is introduced. Historical +15%, +16 MiB and 5.5× diagnostics remain
evidence, not extra PA21 exit gates. Coverage and the eight-element expansion
bound are unchanged. Native optimization/self-hosting belong to later stages.

## Handoff ledger

102: `30fe6353`, `9f2181f9`, `f4224e0b` — RTTI/casts, 37/116.
103: `e835d6dc`, `1c541f84`, `fa079cde` — captures/copies, 49/116.
104: `e2af8963`, `f03b9371`, `511fe9b9`, `3b87e462` — lists/demand, 71/116.
105: audit through `f65eae8d`, recorded by `06b2d989`; 45 failures retained.
106: `9c4f64da`, `301ef6fd`, `e80ad0c7`, `c64e88fb` — EH/lifetimes, 92/116.
107: `b821682b`, `20476a77`, evidence `a2b9e823` — full expressions, 101/116.
108: `efcb52b1` entry/owners; `2e392cec` result/boundary/parser/empty aggregate,
106/116; `a30acab5` construction ownership/defaults/reference proof, **108/116**.
Final evidence/plan commit follows without changing the compiler. Handoff goal:
implementation progress; stage acceptance and independent review remain Ralph's.
