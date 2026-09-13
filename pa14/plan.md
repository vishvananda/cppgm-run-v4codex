# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original **84/314**, current **314/314**: all 230 original failures resolved,
with unchanged fixtures, references and comparison rules. PA15 has not started.
The connected default-fact group from `460f495a` is complete.

## Design/spec alignment

| Owner / data flow | Complexity and validation |
| --- | --- |
| Inherited source/signature/definition facts | Retained source IDs → immutable frames → concrete facts. Existing key, use, terminal-failure and source-sharing controls remain intact. |
| Default declaration → checked expression/conversion | Immutable slots plus concrete function-specialization identity own active/success/failure. Copy-initialization is checked once in the declaration environment. Seven probes repeat 10,000 requests. |
| Checked default → required definitions | Separate terminal registration state; typed member, specialization, storage and nested-default edges. A use inside a type query still needs the separate initializer; a sizeof inside it remains unevaluated. Three probes cover registration failure, later body failure and specialization isolation. |
| Class completion/list selection → default recipe | Root queue intervals provide complete-class context; local-class template defaults stay lazy. Candidate ranking ignores unneeded defaults; selected constructors validate and consume them once. Fixed calls and lists preserve the declaring access context. |
| Recipe → per-call object/lifetime | Consumers copy conversion recipes and own materialization. Deferred template copies are checked even when elided; ordinary elided copies gain no unused emission entry. Repeated native conversion/lifetime controls and 349 exact parity inputs. |
| Inherited lifecycle/virtual facts → ABI emission | Separate properties, actions, effects, transfer and definition demand; typed reasons and precise reverse edges. Existing public failure/availability, cross-TU lifetime and owning PA13 ABI controls pass. |

The default group expanded through definition demand, list/class readiness,
ordinary elision emission, and two caller-context access failures. These share
the checked-default producer/consumer path and are fixed together. No reference
correction, host-compiler production dependency or new implementation source list
entry is needed.

## Remaining groups and concrete boundary

**Unfinished implementation:** general member/function-body terminal publication
and emission ownership, including direct source bodies outside the template
worklist. These have function/definition keys, statement scopes, and full body
failure intervals; a default's parameter-slot fact cannot own them. Extending
this cache to cover those paths would conflate expression validation, body
checking and output state. The next increment must trace those producers and
consumers before changing their scheduling. This is the concrete boundary of
this completed group, not an external blocker or a minimum-progress stop.

**Independent review questions:** finish the source/prototype binding and retained
type/query audit. Declaration-environment constraint coverage is distinct from
consuming an already checked default; remaining findings are not waived by the
course pass. Full-stage architecture remains open. PA12 member pointers and
unsupported later template features add no gate; the inherited pure-virtual
personal input remains an IR control because of the supplied backend boundary.

## Performance acceptance

**15,848 observations**: 14,896 preserved plus 476 preliminary and 476 final.
Final 17 LowIR/eight executable hashes match. Varying N/K/M/Q gives **K** checked
and demanded defaults and **3K** dependency visits in the workload, independent
of unrelated declarations and call count. General budgets are one preparation
per complete key, one registration/visit per required edge, geometric TU storage,
O(required arguments) materialization and zero generated growth on comparable
correct inputs. No optional runtime transform or mandated PA14/O0 numerical
latency/RSS/compiler-text ceiling is introduced; historical diagnostic targets
remain diagnostics under the stage-scoped rule.

Wide repeated-default compilation improves **14.67%**, repeated **14.03%**;
candidate visits fall **132,069 → 4,101**. Compiler `.text` grows **0.602%**.
Fact/edge payloads are **20/12 bytes**, ListPlan stays **68**, Analyzer grows
**6216→6312**, and the other 17 measured records stay unchanged. Wide-input RSS
medians rise 312/846 KiB; no broad memory gain is claimed. Other frontend costs,
including repeated demands **+0.54%/+1.75%**, all paired outliers and identical-byte
runtime variation remain in [performance.md](performance.md). Timing is isolated
from builds, tests, proofs, layout probes and verifiers; all 24 live headers and
archived observations are verified.

## Handoff ledger

| Increment | Commit / evidence |
| --- | --- |
| Inherited source, failure, virtual and lifecycle owners | Through `460f495a`; 14,896 preserved observations |
| Immutable declaration slots and checked default conversions | `c6588684`; seven public probes |
| Separate dependency demand; list/class readiness and local-class deferral | `37f3cd22`; type-query, unused-body, nested-default and class controls |
| Elided copy definition/emission distinction | `decd2e52`; 349 exact entry/current outputs and two copy controls |
| Consume recipes in the declaring access context | `b3927732`; fixed-template-call and list-constructor reducers |
| Final correctness, storage and performance | `default-final-*.json`, `default-proofs.json`; 85 checks, 952 new observations |

Validation: **314 stage / 1621 prior / 1935 through**, file audit, **349**
release/sanitizer and entry/current parity inputs, **35 existing native programs**,
**25 new source controls (21 native)** under both compilers, **20 new public query
runs**, inherited rejection/lifecycle/virtual/failure and PA13 ABI checks. All
**1266 fixture/reference hashes** are unchanged. Artifacts:
`$RALPH_ARTIFACT_DIR/pa14-default-facts/`. Intended code is committed; this ledger
separates the completed default group from the remaining full-stage work.
