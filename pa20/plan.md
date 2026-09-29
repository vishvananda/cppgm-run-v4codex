# PA20 compact plan — implementation handoff 98

Stage base commit: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Last reviewed commit: `882cf5236a8ddb756403105cd50135440920e2a4`.
Target: **PA20 full-stage**. Phase: **implementation handoff; stage incomplete**.
Entry: `e75e0d6ce1e26a2d50bc0857cd620974c6ee451d`, **121/144**.
Implementation boundary: `a1faea7a` (following `f2bfec21`, `b669fc55`);
subsequent records commit contains validation/performance evidence only.
The [checkpoint audit 97](audit.md) and its review markers remain in force.

## Completed owner and spec alignment

Closure occurrence plus enclosing specialization owns canonical closure and
call-operator identities. TU-owned flat `(closure, declaration)` capture indexes
publish one reference/this pointer field per capture, with explicit lexical
forwarding edges for nested closures. Explicit parameter packs consume retained
parameter-entity sequences, including empty packs. Checked expression/receiver
facts carry capture IDs through fixed template-fact reuse. Lowering consumes
those IDs/layouts directly for initialization and loads, with ordinary special
members for copies. There is no syntax replay, name reconstruction, broad cache
invalidation or duplicate body checking. Captureless conversion entries remain
available only for empty capture lists; invalid C++11 lists are rejected.

Work follows actual capture edges, parameters and expressions: average O(1)
completed capture lookup, linear field construction, required lexical traversal
for first nested demand. Facts live through TU lowering; emitted storage belongs
to each function. Existing deduction, range and initializer-ordering owners are
preserved. Source-to-native tracing verifies repeated specialization demand,
mixed local/this fields, nested rebinding and copying.

Two adjacent conversion oracles were independently proved wrong and corrected:
implicit closure→pointer→wrapper needs two user-defined conversions; constructor
template deduction must retain the closure class. [Proofs, reducers and bundle
revision](reference-corrections98.md) preserve the original sources and comparison
rules. Positive direct-wrapper/pointer and constructor-type controls remain.

## Validation and performance

[Final evidence](../student.tests/pa20/validation98.json): PA1–19 **3452/3452**;
file audit **pass**, with the same three inherited header warnings; PA20
**140/144**; through PA20 **3592/3596**. Failures fall **23→4**: **17 real capture
implementation fixes**, plus the two proved conversion corrections. All 144
sources and comparison rules remain; the coverage manifest checks 639 files.
Personal controls **264/264** (206 inherited, 58 capture/composition controls),
three inherited source-to-native traces, and the [new capture trace](../student.tests/pa20/trace98.json)
pass. Ten repaired required programs match reference executable results; one
fixture declares but does not define `std::forward`, and both native attempts
report that unresolved external. Its required LowIR comparison passes.

[Performance acceptance and all observations](performance98.md): frozen binaries,
A/A and ABBA, compiler latency/RSS plus checked executable runtime/payload size.
Comparable native files are byte-identical; new captures have final-only cost
measurements. Counters scale 3200→12800 capture edges at 800→3200 specializations.
No optional optimizer or work/growth limit changes. Historical percentages are
diagnostics under spec §9; no mandated limit, correctness or coverage is waived.

## Remaining implementation (mandatory)

| Owner | Remaining work | Failures |
|---|---|---:|
| Aggregate initialization/helper ABI | Class-member copy construction and omitted-class-tail helper representation, preserving audit 97's destination sequencing proof | 2 |
| Class lifecycle/value ABI | Dependent-owner completion and direct versus indirect result decisions | 1 |
| Parser/retained declaration environment | Repeated local declaration versus relational-expression/template-name probe | 1 |

This handoff finishes capture environments and their constructor-conversion
composition, including explicit packs beyond the initially failing fixtures.
No known capture implementation defect is left open. Continuing these four
failures requires different owners: aggregate constructor plans and helper
signatures, class transfer/return ABI classification, or lookup-sensitive grammar
probing. Capture identities cannot establish their needed facts. Those are
concrete new implementation groups, not related capture follow-ups or PA21
deferrals; they remain required before stage completion/advancement.

Independent review questions: verify nested forwarding/occurrence isolation,
pack and special-member composition, the two standard proofs and performance
interpretation across the full base-to-tip range. This handoff does not perform
or waive that independent review, nor supersede the audit-97 marker.

## Handoff ledger

| Boundary | Completed implementation/evidence | Unfinished implementation | Independent review |
|---|---|---|---|
| Audit 97 | [Preserved audit ledger](audit.md#ledger), 121/144 | Original 23 failures | Reviewed through `882cf523`; markers retained |
| Implementation 98, `e75e0d6c`→`a1faea7a` | Reference/this/nested/pack capture ownership; conversion proofs; 140/144, 264 controls, required prior/audit checks, performance evidence | Four failures in the three owners above | Pending Ralph's independent audit; no advancement claim |
