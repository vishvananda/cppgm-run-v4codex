# PA21 compact plan — checkpoint audit 105

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `f65eae8d7d434735a0ce981173a347eb8f8a1e59`.
Target: **PA21 full-stage**. Phase: **checkpointAudit complete; implementation remains**.
Entry: clean `3b87e462`, **71/116 pass, 45 fail**. Current: the same **71/116**
and exact **45 failures**, with no coverage reduction. Earlier PAs: **3596/3596**.
PA21 completion and advancement to PA22 still require the unfinished work.

## Reviewed design and findings

[Audit](audit.md) covers every commit from stage base through the reviewed code
tip: all three accepted handoffs, their interactions and the repair. Typed RTTI,
capture and list facts feed direct LowIR; canonical identities and TU/function
owners bound lookup, demand, storage and cleanup. The independent trace follows
a polymorphic declaration and two demanded templates through class-element lists,
captured ranges and RTTI to execution through the supplied PA8 backend.

Audit fixes: retain fixed RTTI recipes instead of rechecking them per
specialization; preserve a destructible copied subobject's identity when bulk
prefix grouping would otherwise hide its unwind cleanup. Five new host-unwind
reducers fail at entry and pass now. Missing RTTI facts are invariant errors.
Historical details remain in [RTTI](rtti102.md), [captures](captures103.md)
and [lists](lists104.md); this audit supersedes their pending-review wording.

## Remaining implementation groups

| Owner | Required next work and validation |
|---|---|
| Source EH semantics | Typed exception-object initialization, throw/rethrow, catch binding/matching and handler exits, including lambda/template bodies. Demand cleanup dependencies after concrete type completion. |
| Lifetime and control-flow integration | Complete construction/destruction, conditional/full-expression and argument/return ownership; context-complete cleanup continuation keys; list backing regions; local-class/recursive-template identity. Preserve reverse partial cleanup and bounded array/prefix work. Resolve every remaining required LowIR comparison. |

The two list fixtures still differ in required EH-region/address/suffix shape;
they join the other 43 failures. Native controls do not waive those comparisons.
Keep the next handoff broad enough to finish shared ownership paths and their
compositions. Separate cleanup/demand follow-ups in 103–104 were avoidable
fragmentation; do not turn each fixture repair into another handoff.

## Validation and performance

[Validation](../student.tests/pa21/audit105-validation.json): prior-through
**3596/3596**, file audit **pass** with the same three advisories, PA21
**71/116** (exit 2), exact failure-set preservation and **116 unchanged inputs**.
The 15048-path contract inventory permits only the independently verified,
historical [RTTI reference correction](reference-corrections102.md).
New composition **16/16** and prefix-unwind **5/5** controls pass alongside
inherited capture/list/native/host controls. One known supplied freestanding RTTI
runtime discrepancy remains recorded and passes with the host runtime.

[Performance](performance105.md) records frozen stage-base/entry/final binaries,
A/A noise and ABBA pairs, compiler latency/RSS, separate checked runtime/payload,
and work counters. Fixed RTTI recipe counts stay three as specializations grow;
the affected template workloads compile 10–12% faster with identical executables.
Required cleanup adds 152 payload bytes and a measured normal-path runtime cost;
no benefit is claimed against an incorrectly unwinding baseline. All 102–104
measurements remain. PA21/O0 uses spec §9 bounds and the existing eight-element
expansion cap; inherited +15%, +16 MiB and 5.5× diagnostic targets add no exit
gates. Own native optimization/ELF and self-hosting retain their later owners.

## Handoff and audit ledger

102: `30fe6353`, `9f2181f9`, `f4224e0b` — RTTI/casts and oracle proof,
37/116; [validation](../student.tests/pa21/validation102.json).
103: `e835d6dc`, `1c541f84`, `fa079cde` — captures/generated copies,
49/116; [validation](../student.tests/pa21/validation103.json).
104: `e2af8963`, `f03b9371`, `511fe9b9`, `3b87e462` — lists/demand,
71/116; [validation](../student.tests/pa21/validation104.json).

| Audit | Range, findings and disposition |
|---|---|
| 105 | `ac988ea3..f65eae8d`: full accumulated range reviewed, RTTI reuse and copied-subobject cleanup repaired, proof/trace/performance verified, prior/file/progress gates pass; 45 failures remain in the two broad groups above. |

Audit records follow the committed code tip without further implementation edits.
