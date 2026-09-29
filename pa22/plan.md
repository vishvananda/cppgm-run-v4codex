# PA22 compact plan — handoff 118

Target: **PA22 full-stage**. Phase: **implementation handoff; stage incomplete**.
Stage base commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
Last reviewed commit: `e90fa3fa514e2990bbe5ea4716252d8c42ae782a`.
Entry HEAD: `0c710f745f1341536992f43b24ba325578a2fad3`, **94/99**.
Handoff implementation: `e10bdd7f`, **95/99**; the nested-owner member-template
storage case is fixed. All 99 cases, references, statuses and comparison rules
are unchanged. The previous goal turn was progress (audit 117's constant-owner
repairs and evidence); it had no live build/test process at entry.

## Design/spec alignment and completed group

Semantics owns a bounded forward proof for local member-pointer storage.
A function request is deduplicated at member application; completed demanded
bodies are visited once, without parsing, instantiation or callee demand.
Local declaration identity plus canonical byte offset identifies storage;
recorded field/base projections distinguish repeated bases and unify equivalent
field paths. Per-occurrence read facts feed typed lowering directly. Calls,
unknown writes, overloaded arrows, control boundaries and temporary destruction
expire facts. Parameters, references, unions, volatile and unsupported forms
retain generic adjustment. Facts do not change target-word truth semantics.

The shared receiver owner also repairs qualified repeated-base field access:
select the qualifier subobject before the field's declaring base. Lowering and
constant evaluation consume that path; dependent queries already use the same
owner. Unqualified ambiguous names and inaccessible qualifiers remain rejected.
No rendered identity, new global cache, textual transport or grammar replay.

Work is bounded by **4096 visits/function**, **64 traversal depth**, and the
existing **64-node value proof** per attempted value. Conservative fallback emits
ordinary correct IR. Function-local flat indexes are released after each body;
only requested-function IDs and proven occurrence IDs survive to lowering.
There is no IR duplication or growth: a proven call removes three operations.
The effect epoch is local to one flow region, not a global cache generation.

## Validation and performance

[Validation](../student.tests/pa22/validation118.json): PA1–PA21 **3712/3712**,
PA22 **95/99**, file audit pass (three inherited header warnings), **94/94**
personal controls plus the ABI target-word truth probe, **95** stable accepted
LowIR roundtrips and **four** preserved rejections. New controls are **36/36**;
the preceding implementation fails the overloaded-arrow call control, repaired
before handoff. The entry compiler also rejects four valid qualified-field
controls. No fixture or oracle correction was made.
The through-PA22 report is **3807/3811**, failing only those four PA22 cases.

[Performance](performance118.md) retains both A/A+ABBA campaigns, frozen hashes,
all observations, compilation latency/RSS, checked runtime and native text.
Flow visits scale **6656→26624** for **512→2048** functions. The initial two
live-loop comparisons improve about 30% with eight improving paired blocks;
the final campaign's noise and regressions are disclosed. Both campaigns emit
identical native work. Six common inputs retain identical LowIR/native text.
Spec §9's PA22/O0 acceptance applies: inherited +15%, +16 MiB and 5.5× targets
remain diagnostic, not gates. Preserve all historical 114–117 evidence and
mandated correctness, comparison, work/growth and initializer-expansion bounds.

## Remaining implementation and boundary

1. **Parameter member values and required shape**: general
   `300-const-member-function-pointer-address-call`, spec
   `300-member-pointer-parameter-variadic-deduction`, and
   `300-overloaded-member-pointer-function-template-deduction`. Unknown parameter
   values require receiver adjustment/target-word extraction. The completed
   local storage proof cannot establish facts for externally callable functions;
   the inverse-adjustment and noncanonical-null ABI controls demonstrate why.
   Resolving these requires a separate sound call-boundary/value owner or a
   reduced, contract/standard-proven oracle correction. No correction is assumed.
2. **Constant-condition materialization/demand**: general
   `300-structured-bool-conditional-member-pointer-dead-branch`. Its remaining
   receiver temporary and static-constant emission differ from the oracle.
   Resolving it requires coordinated receiver effects, lifetime and emission
   demand, beyond the completed local member-storage owner.

These are four unfinished implementation obligations, not waived audit questions.
Further local proof expansion cannot discharge them; crossing those ownership
boundaries requires a new semantic design and validation group. The completed
handoff includes the related qualified-field defect and every discovered flow
effect defect, not just the minimum one-test progress. PA23's virtual inheritance,
polymorphic multiple inheritance and broader RTTI remain deferred. Do not advance
before the full PA22 through report passes and independent audit resolves findings.

## Handoff ledger and independent review

| Range | Status | Evidence / boundary |
|---|---|---|
| Stage base through `e90fa3fa` (114–117) | Independently reviewed | [audit117](audit.md), including both audit fixes; historical measurements preserved |
| `f18dfb62..d649b4b5` | Implemented; independent review pending | Local storage proof, canonical projections, qualified field runtime/constant/template paths |
| `e10bdd7f` | Implemented; independent review pending | Overloaded-arrow receiver effect boundary; 35/36→36/36 controls |

Independent review must assess path identity, conservative effect boundaries,
proof budgets/profitability and the accumulated whole-stage design. Those review
questions are distinct from the four known implementation failures above;
neither category is waived. Review markers remain unchanged for Ralph.
