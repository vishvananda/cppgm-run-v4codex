# PA21 implementation plan

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Target: PA21 full-stage. Phase: implementation (loop 104).
Loop 104 entry: clean at `fa079cdeffb19a1a69e92ff56f8bc154c4047c36`;
49/116 pass, 67 fail. Previous handoff made verified progress (twelve resolved
failures); no compiler/test process remained live on this entry.
Current work: initialization owns canonical list element/storage plans;
overload selection and deduction consume those plans, and lowering consumes
checked element conversions and explicit backing-array lifetime facts. Extend
through calls, constructors, assignment, returns, ranges and storage duration.
Work must follow elements/candidates and emitted cleanup edges, with no repeated
semantic reconstruction. Validate required fixtures, personal rejection/runtime
controls, prior-through and file audit; freeze entry/final compilers for §9
A/A and ABBA latency/RSS plus checked native runtime/size evidence.
Loop 104 checkpoint: 71/116 on the second stage run; 22 original failures
resolved. Prior-through now 3596/3596; file audit passes (three inherited
advisories). Personal list controls 48/48 and external-unwind controls 27/27
pass, including constexpr/nested storage and host-linked local-static atexit.
Implementation continues: required class-list lifetime LowIR comparisons,
scaling/evidence and related defects still need resolution before handoff.
Loop 103 entry: clean at `f4224e0b`; 37/116 pass, 79 fail.
Now: **49/116 pass, 67 fail**; twelve old failures resolved, no new failures or
coverage reductions. Independent whole-stage audit remains pending.

## Design/spec alignment

RTTI/casts: standalone group complete in loop 102. Typed formation/evaluated facts
feed cached RTTI and ABI identities; lowering emits once per identity.
[Ownership, bounds and external runtime limitation](rtti102.md).

Closures: indexed capture edges retain mode, source type, forwarding field and
checked conversion/default/destructor dependencies. Explicit/default copy and
reference, `this`, mutable/cv, nested, array, template and RTTI compositions use
this shared path. Template `decltype` preserves hypothetical capture cv without
storage demand. Typed lowering constructs fields once and records completed
subobjects for reverse unwind cleanup; generated closure copies share the fixed
class-transfer path. [Design, bounds and validation boundary](captures103.md).

## Remaining implementation groups

Each current failure has one primary owner; compositions cross owners.

| Group / owner | Data flow and complexity obligation | Remaining / validation |
|---|---|---|
| Initializer lists / initialization and overload selection | Canonical element/conversion plans own backing storage, list ranking and lifetime; lowering consumes those plans once. | 24; braced calls/construction/return, deduction, ranges, scalar/class elements and storage duration. |
| EH and lifetime / source handlers and cleanup control flow | Typed active handler, exception-object and constructed-subobject facts own continuations; share only identical complete contexts. Work bounded by actual control/cleanup edges. | 43; source throw/try/catch (including two lambda compositions), subobject failure, argument ownership, conditional temporaries, destructor termination and local-class/recursive-template cleanup. |

Handoff boundary: closure environments, capture initialization and generated
copies—including partial construction—are implemented and validated. The two
remaining lambda fixtures enter unsupported source `try` checking. Their catch
bindings, exception objects, matching/rethrow and handler-exit facts require the
source-EH representation shared with the other EH failures. List backing storage
and overload ranking likewise have a separate owner. Further closure field/copy
changes cannot supply these missing semantic states; both groups remain required
implementation, not independent-review deferrals.

## Performance evidence

[Protocol/results](performance103.md), [final raw observations](../student.tests/pa21/performance103.json)
and [preserved intermediate observations](../student.tests/pa21/performance103-first.json).
Frozen A/A + ABBA: all six equivalent inherited workloads have identical LowIR
and executables. Compiler text +24,832 B (1.18%); heavy-template RSS essentially
unchanged. No speedup claimed. 800→3200 closures yields exactly 4× capture edges,
4× expression work, 116→477 ms compilation and 24,132→76,672 KiB RSS.
32→4096 array elements retain constant-sized copy-loop LowIR. Checked live calls,
memory, floating-point and capture loops include native runtime/payload evidence.
PA21/O0 uses spec §9 bounds and eight-element expansion; no optional optimization
or extra numerical gate. Historical self-selected ratios remain diagnostics.
Own native optimization and self-hosting retain their later-stage owners.

## Handoff ledger

Loop 102: `30fe6353` proves/corrects one oracle substitution digit; `9f2181f9`
implements RTTI/casts; `f4224e0b` records 37/116 and prior-through 3596/3596.
[Preserved validation](../student.tests/pa21/validation102.json) and
[performance](performance102.md); independent whole-stage review was still owed.

Loop 103: `e835d6dc` implements typed captures and construction lifetimes;
`1c541f84` completes generated-copy unwind cleanup.
[Validation](../student.tests/pa21/validation103.json): `make test-pa21` 49/116
(exit 2); prior-through 3596/3596 (exit 0); file audit passes with three inherited
advisory header warnings. Through-PA21 also reports 3645/3712. Personal controls:
64/64 capture controls, 58/58 inherited controls, 13/13 external-unwind controls.
Required fixture inputs, references and comparison rules are unchanged this loop.
No known defect remains open in the completed group. The two unfinished groups
above and independent whole-stage correctness/architecture review are both still
required; neither is waived. Review questions include capture/query key completeness,
retained-template cv/demand and lifetime-state ownership under the stage-wide spec.
Review markers remain unchanged. This completes
loop 103 implementation handoff, not PA21 or its independent audit.
