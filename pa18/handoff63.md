# PA18 implementation handoff 63

Implementation boundary: `4a49ea10` (after `5d71a342`, `e84a420e`, `c576e6a5`).
Stage/review base remains `94dcb8ad21664137e87d574e878c14a4a047348a`.
This is an **incomplete implementation handoff**, not a full-stage audit.
At entry the worktree contained no PA18 implementation progress or plan;
continuation was treated as no PA18 progress and resumed from the 154-failure
baseline. This handoff changes compiler behavior and reduces those failures.

## Completed ownership and data flow

| Owner | Implementation and scope |
|---|---|
| `template_ordering.cpp`, `template_deduction.cpp` | Partial ordering has a distinct deduction policy. Canonical source parameter identities are rigid symbols on the argument side; only the parameter-side scratch bindings mutate. Nested cv and specialization arguments use strict matching without derived-class completion. Call comparisons nominate supplied parameters, omit unused defaults and results, retain symbolic packs, and apply reference/cv ties per corresponding type. Address/explicit-specialization contexts compare full signatures. Non-deduced uses still require bindings; unused head parameters do not. |
| Call/constructor/operator selection and `conversion.cpp` | Every viable candidate participates in linear winner selection and verification, including a winner after a tied prefix. Ordinary functions outrank template specializations for addresses; templates use partial ordering. Crossed conversion advantages remain ambiguous. Array qualification comparison descends through dimensions. Implicit object insertion applies to member/nonmember operator comparisons. The selected entity and conversions remain the ordinary semantic call/address facts consumed by lowering. |
| Prototype declarations and query calls | A function signature retains its expansion node; the prototype parameter denotes an element and retains its pack flag. Return-type queries expand empty/nonempty packs through existing immutable frames. Indirect query arity/type rejection returns a structured failed fact. Definition bodies are demanded only after selection. |
| `template_packs.cpp` | Expansion overlays distinguish template arguments from runtime parameter EntityIds. An entity lane never indexes the type arena. The 640-specialization reducer and 2400-specialization benchmark cover the former numeric-identity alias/out-of-bounds failure. |
| `query_abi.cpp` and typed Itanium graph | `sizeof...` retains template/function ordinals or captured arguments and lowers directly to a typed ABI node. Encoder, validator and explicit fact adapters support `sZ` and `sP`, without semantic string keys or an ABI text roundtrip. Grammar proof: `doc/itanium-mangling.txt`, expression productions at lines 544–546. |

Ordering rules are in N3485 [temp.func.order] 14.5.6.2/2–5,
[temp.deduct.partial] 14.8.2.4/3–12, and [over.over] 13.4/2–4.
No contract fixture, reference, comparison rule or required coverage changed.

The ordering cache belongs to the Analyzer/TU. Its complete key contains the
two original TypeIds, supplied-argument count/full-signature context, operator
context and implicit-object owner IDs. It is probed before constructing shapes:
completed lookup is O(1) average; misses visit compared type occurrences plus
the used-parameter graph, with a visited set for that graph. No declaration,
layout or body state is used or invalidated. Scratch vectors/indexes release
on return; canonical shapes, arguments and completed comparisons release with
the TU. Candidate selection is O(C) comparisons, not an all-pairs tournament.
Pack expansion remains proportional to language-required lanes and output.
ABI construction/encoding is proportional to its typed graph/output.

A representative trace is the const-pointer overload: retained `const T*`
pattern → call deduction/substitution → recorded viable conversion → strict
ordering fact → selected specialization → existing body demand → recorded
call/ABI facts → typed LowIR. Neither comparison nor lowering invents a class
specialization merely to compare shapes. `invoke` follows the same path, with
its retained result query consuming prototype element types and body expansion
consuming existing parameter objects. The synthetic virtual-emission course
case and the frozen native workloads exercise these boundaries.

## Validation and remaining boundary

Current PA18: **282/420**, versus **266/420** at entry: **16 existing failures
fixed, zero new failures**, same 420 fixtures. Earlier PAs: **2609/2609**, all
17 stages. The combined root through-PA18 report remains red at **2891/3029**.
The PA18 file audit passes with the three inherited header-division advisories.
Exact commands, source hashes, fixed/remaining cases and outcomes are in
[loop63-evidence.json](../student.tests/pa18/loop63-evidence.json).

All **64 personal controls** pass, versus 21 at entry. They cover cv, references,
arrays, nested aliases, packs, arity/default nomination, address/return contexts,
deleted/unselected bodies, candidate permutations, operators/constructors,
query failure, prototype pack results, ABI size queries and arena growth.
Fifteen selected course fixtures execute successfully through the supplied
backend; the sixteenth is validated LowIR only because its source deliberately
declares `pick`, `pair` and `Wrap` without definitions. The attempted native
link failure is retained in the evidence; no course oracle was changed.
The three array-ordering fixtures execute correctly but retain their course
LowIR initialization mismatches.

PA9 direct API, all 117 contract/serialization cases, personal/final adapter
controls, and ten new pack-ABI tool/roundtrip checks pass. See
[performance.md](performance.md) for frozen compiler and executable evidence,
including the interrupted run that exposed the pack-lane defect.

The next related deduction prerequisite is now in a different owner: immediate
substitution must retain/drop dependent results and defaults with the correct
lexical and specialization context. There are **117 acceptance/rejection**
failures and **21 LowIR comparison** failures left. Of the five remaining
`200-` fixtures, four already pass semantic compilation and fail LowIR
comparison; the fifth requires dependent-base inherited-constructor
publication. Extending the ordering algorithm cannot resolve those failures.
Constructor/conversion-target deduction, explicit/braced deduction,
dependent-name/alias validity and initialization/result metadata lowering
remain implementation work. Completing them requires their own retained
facts and validation groups, rather than more changes to this completed owner.

Independent review remains required for the entire stage range: validate the
ordering context/cache inputs and symbolic directionality, pack/query identity
and ABI graph integration against the full language surface, then reconcile all
remaining whole-stage spec findings. These are review questions, separate from
the known unfinished implementation above. Neither is waived. Preserve the
plan's stage base and last-reviewed markers until Ralph's independent audit.
