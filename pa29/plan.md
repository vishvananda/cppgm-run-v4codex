# PA29 compact plan — audit186

Target: **PA29 full-stage**. Phase: **checkpoint audit complete; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `52070178897f5894edaf2f35d03a734b781979d4`.
Audit entry: `152396e2` (392/403; 11 failures).
Last reviewed commit: `2df00585bd10d4e2e068934394dffc8adb0a47ed`.
Reviewed range: **`52070178..2df00585`**, all three accepted handoffs and fixes.

## Reviewed ownership and repairs

[Audit186](audit.md) reviews every commit, the accumulated source and cross-handoff
interactions. Guide declarations use canonical signature/query identities and
separate declaration facts. Explicit casts retain cv/object/member identity and
base adjustments. Bit-integer widths remain typed through dependent substitution,
constants, exact precision, storage and serialized `i128a8` ABI alignment.
The scalar capacity remains signed 2–128 / unsigned 1–128; larger widths diagnose.

Audit fixes retain prototype `decltype`/`typeid` inquiry facts under complete
default frames, preserve nested unevaluated/evaluated boundaries, and normalize
bit-integer overflow results before representability checks. No repeated name
recovery, fake runtime parameter or production text transport is added. Query
work follows required typed edges; normalization adds at most two shifts.
[The combined control](../student.tests/pa29/source186/integrated.cpp) traces guide,
default/template, cast, RTTI, storage and native emission together.

## Validation and stage-scoped performance

PA29 **392/403**, exactly the entry's **11 failures**; PA1–28 **4538/4538**;
through PA29 **4930/4941**. File audit passes with four inherited warnings.
All **141** explicit controls and **836** inspection commands pass. All **403**
inputs and **1,707** contract/harness paths are unchanged against entry and prior
review. No reference or comparison rule changed. See the audit and
[evidence manifest](../student.tests/pa29/evidence186/manifest.json).

[Performance186](performance186.md): **496** frozen observations plus eight
launchers; eight equivalent object/executable pairs are byte-identical. Compiler
paired medians **0.9953–1.0189**, all paired compiler ranges crossing unity; RSS
increases and noise remain disclosed. Twenty-four scaling checks confirm one
source default binding and one formation/body transition per demanded
specialization; corrected-only runtime/text remain fixed. **1,144** inherited
observations plus 24 launchers are verified and preserved. No speedup is claimed.
Mandatory limits remain enforced; inherited blanket 15%/zero-growth targets are
diagnostic under spec §9, not extra exit gates. Necessary semantic costs and
later-stage optimizer/self-hosting work do not change PA29 acceptance.

## Broad remaining groups and handoff quality

The [11-case ledger](../student.tests/pa29/evidence186/remaining.json) groups numeric
representation/vendor syntax **7**, hosted template/demand and ABI **3**, and
legacy trait contract **1**. Eight need implementation; three independent contract
questions remain counted (nothrow shorthand, invocability and nested ABI tags).
Continue floating/complex representation and ABI, vector/contextual syntax and
hosted template behavior as broad owners. No failure is waived.

Avoid the observed fragmentation: retaining only `sizeof` inquiries and omitting
overflow consumers from scalar integration left shared-owner gaps for this audit.
Complete source/query/default, conversions, constant/runtime, storage/ABI and
adapter boundaries together before handoff. [Audit182](audit182.md), handoffs183–185
and all historical evidence remain retained. Full PA29/root-through success is
required before PA30. The records commit makes no code changes after the reviewed
tip above.
