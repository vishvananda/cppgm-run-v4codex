# PA18 implementation handoff 76

Entry `b87de70e` (**383/420**). Implementation commits `e052e939`, `bff709db`, `c8a2aad8`.
This is an implementation boundary; independent review of handoffs 75–76 and
whole-stage acceptance remain open. Stage/review markers in `plan.md` are retained.

## Completed ownership group

Inherited constructor templates retain their source head and a typed edge to the
base constructor. Class completion publishes the derived candidates after local
signatures are available. Constructor characteristics and local hiding follow
N3485 §12.9 [class.inhctor]/1–5; omitted default parameters produce notional
signatures, rather than inheriting default arguments. Implicit default/copy/move
construction remains a separate operation. See [the local standard](../doc/n3485.txt),
lines 15365–15474, and the forwarding rule in /8, lines 15475–15482.

Deduction uses the retained base parameter identities, defaults and substitution
environment. Each `(derived candidate, canonical argument pack)` specialization
has a distinct declaration, and points to the corresponding canonical base
specialization. A failed base substitution retains its completion prerequisite.
The derived body is a synthetic forwarding action, never a projected base body.
For a notional signature, omitted parameter types/access checks remain dormant;
forming the full base specialization belongs to forwarding-definition or exception
specification demand, as required by [class.inhctor]/8 and §14.7.1 [temp.inst]/2–3.
The final five controls catch both premature rejection and
missing diagnostics when that forwarding is actually selected.
Access, explicitness and deletion are taken from the corresponding base entity;
partial ordering consumes the inherited template signature.

Selected forwarding recipes record parameter transfers and omitted defaults.
Ordinary lowering, noexcept and constant evaluation consume those records.
Class values invoke their selected move/copy, including defaulted transfer
arguments; synthetic parameters own their destruction. Default argument work
stays in the base declaration's lexical environment. Constant execution applies
the same transfers, including constexpr copies/moves with observable field values.
Noexcept includes forwarded transfers, defaults and the derived subobjects.
The constructor action initializes the nominated base, and the usual actions
handle other subobjects. No source replay, fake syntax, external implementation,
new comparison tolerance or reference correction is introduced.

Owners and data flow:

| Owner | Stored facts / consumer |
|---|---|
| Class completion (`inherited_constructors.cpp`) | Derived candidate identity, signature, source-head and base-constructor edge; consumed by ordinary deduction. |
| Specialization (`template_call.cpp`) | Monotonic declaration state and derived/base specialization identity; no independent base-body parsing. |
| Forwarding (`inherited_forwarding.cpp`) | One recipe per concrete constructor, selected transfer/default conversions, explicit not-started/active/success/failure state; unavailable prerequisites reset only that recipe. |
| Constructor definition (`construction.cpp`) | Concrete parameter identities and demand for selected transfers/defaults; subobject actions retain the base target. |
| Constant/noexcept/LowIR consumers | Read the retained forwarding edge and conversions; lowering does not rerun overload resolution. |

All records are translation-unit-owned vectors/compact identities; work follows
candidate parameter edges, demanded specialization/forwarding facts and subobjects.
Default omission may require several candidate signatures; construction is linear
in their total parameter edges (the language-required notional candidate set).
No scan over unrelated declarations or global cache invalidation was added.
Each concrete forwarding recipe is built once. Source template bodies remain
shared; query-only use does not request their definitions.

## Validation

- Required PA18 suite: **385/420**, failures **37 → 35**, no new failures.
  Original repairs: `200-inherited-constructor-template-forwarding` and
  `400-partial-specialization-inherited-constructor-template`.
- Earlier stages: **2609/2609**. File audit passes with the same three inherited
  header advisories. All 420 inputs and 1,686 tracked fixture/reference files, and comparison rules,
  are unchanged. The prior 1,712 count included 26 generated `.check*` files;
  those observations remain, but are not tracked contract coverage.
- **60/60** new semantic/native/rejection controls, compared with **17/60** at
  entry; **718/718** inherited semantic controls pass. Intermediate failures and
  their repairs remain in the recorded logs.
- All three inherited-constructor course sources validate and execute with the
  supplied PA18 backend, including the remaining comparison failure below.
- Twelve graph controls at 32/128/512 classes: declaration-only use creates zero
  forwarding arguments/bodies; noexcept queries retain N forwarding arguments
  and zero bodies; execution retains N arguments and N base-template bodies;
  repeated queries of one specialization retain exactly one argument and no body.
  Three inherited completion-invalidation controls also pass.
- Frozen compiler/executable evidence and stage acceptance: [performance 76](performance76.md).
  Authoritative hashes, commands, results, controls and remaining failure paths
  are in [loop76 evidence](../student.tests/pa18/loop76-evidence.json).

## Handoff boundary and unfinished implementation

The scope was extended beyond candidate participation through default omission,
implicit defaults, ordering, access/deletion, constexpr/noexcept, value forwarding,
transfer defaults and destruction. These consumers now have executable positive
and rejection controls; the two repaired original failures establish progress.

`500-inherited-constructor-template-member-alias-pack` changed from rejection to
valid, executable LowIR but remains a required comparison failure. Its residual
diff is the private tag constructor's object-root metadata and zeroing an empty
tag temporary. Those are ordinary constructor-emission and zero-initialization
policy facts, outside the completed inherited candidate/forwarding owner. It is
not counted as a course pass. Reworking those policies also interacts with the
older pending scalar-array pooling, class-result, bool and discarded-load forms;
a case-specific suppression would not be a coherent repair.

The **35** remaining cases comprise **9 rejections and 26 LowIR mismatches**.
Declaration timing (lazy nested classes), ambiguous member lookup, explicit member
template/alias participation and ordinary LowIR facts remain implementation work.
Completing another of those groups requires a separate declaration-state or
lowering-policy change, rather than another consumer of this forwarding recipe.
This is the concrete boundary for this incomplete handoff, not a waiver of PA18.

Independent review should verify canonical proxy/head identity across inherited
chains, completion retry ownership, notional signature formation, forwarding
lifetime/ABI facts and interactions with handoff 75. These review questions are
separate from the known 35 implementation failures. Neither category is waived.
Do not advance until the through-PA18 report passes and the independent audit
resolves whole-stage findings.
