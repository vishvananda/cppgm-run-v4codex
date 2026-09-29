# PA22 compact plan — implementation handoff 114

Target: **PA22 full-stage**. Phase: **implementation; incomplete handoff**.
Stage base commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
Last reviewed commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
Entry: **22/99 passing, 77 failures**. Current: **47/99, 52 failures**.
No earlier passing PA22 case regressed; the same 99 contract cases remain.

## Design/spec alignment and completed work

`87ee0e07` implements the member-value representation and conversion group:
canonical member/declaration/type identities and recorded base paths feed typed
LowIR `i64` offset-plus-one data pointers and packed `i128` function pointers.
Null initialization, static/constexpr values, local storage, qualification,
base/derived conversions, mixed-owner comparisons/conditionals, `.*`/`->*`, and
contextual truth share that representation. Nonzero and inverse adjustments
preserve null semantics and receiver identity. The LowIR model/reader/writer
support the emitted scalar form directly. Known non-null addresses retain their
fact through ordinary base projection.

A TU-owned, entity-keyed proof records all local writes and address exposure.
Its active/proven/unknown states, shared 64-node budget and conservative fallback
avoid a false assumption that an owner's layout determines every member-pointer
value. Lowering reads the completed fact; it performs no member lookup or
semantic reconstruction. `e6316d05` keeps overloaded address calls out of this
proof. Work/storage are O(relevant objects + writes), with constant-size lowering
per operation and cached canonical base paths. No production text roundtrip,
reference delegation, source replay or global cache was introduced.

The single-vptr `dynamic_cast<void*>` path now preserves null and consumes the
vtable offset-to-top. The explicit O0 view retains the contract's unreachable
cast continuation. Broader virtual layouts remain PA23 work.

## Unfinished implementation and concrete boundary

- Dependent owner types, member-pointer NTTP formation, address substitution,
  deduction, packs and partial specialization remain the dominant failure group.
  `TypeKind::MemberPointer` currently carries a concrete class entity; accepting
  dependent syntax alone cannot preserve canonical owner identity or SFINAE.
  The next coherent change must coordinate type formation/interning, dependence,
  substitution frames, deduction and pack traversal before enabling those cases.
- Dependent `.*`/`->*` callable queries, inherited conversion functions, template
  member lookup/access and ambiguous subobject/type lookup remain unfinished.
- `300-const-member-function-pointer-address-call` still differs because generic
  parameter calls consume the adjustment word, while the reference omits it.
  Reverse casts and unknown parameter values require that adjustment. Contract
  parity needs a sound parameter-value proof or a proved reference correction;
  restoring the layout-only shortcut is incorrect. No oracle has been changed.
- `300-ambiguous-member-nontype-arg-sfinae` still selects the wrong result until
  the NTTP/query group is implemented. All 52 failures are listed in the ledger
  linked below; they are implementation obligations, not waived audit questions.

This handoff finishes local member values/conversions and the single-vptr void
cast, including related receiver, qualifier and truth fixes. Further progress
now crosses the dependent-type/query ownership boundary or requires a separate
parameter-value proof. Those changes cannot be supplied safely as another local
representation edit. The full-stage target remains active for Ralph's next run.

## Performance and validation

[Performance evidence](performance114.md) preserves frozen binaries, inputs,
A/A calibration, ABBA samples, compiler latency/RSS, checked executable runtime
and actual `.text`, plus the analysis-off proof experiment. Required `i128`
backend costs are disclosed. The 64-node proof has measured runtime/text benefit
and linear work growth; no unprofitable optional pass was added. Inherited
numeric diagnostic targets remain diagnostics under spec §9, not extra gates.

[Validation and unchanged contract manifest](../student.tests/pa22/validation114.json):
PA1–PA21 **3712/3712**, file audit **pass** (three inherited header warnings),
`make test-pa22` **47/99**, root through-PA22 **3759/3811**. Existing failures
fall **77 → 52**, with **25 repaired and zero regressions**. Personal controls
improve **9/16 → 16/16**; explicit LowIR roundtrips, target-word truth and bounded
proof growth/cycle controls pass. References, harnesses and coverage are unchanged.

## Handoff ledger and independent review

114 entry: interrupted PA22 work had no surviving process or uncommitted change;
its no-progress state was revalidated before implementation. `cfcbacab` records
the base/review markers. `87ee0e07`, `e6316d05` implement the group above; final
handoff commits retain the experiments, checks and remaining-failure ledger.

Independent review has **not** audited these commits. Review must verify stored
value/alias proof coverage, canonical constant/base identities and the performance
acceptance evidence, then resolve whole-stage findings before advancement.
These review tasks are separate from the known unfinished implementation above.
Both review markers remain unchanged; neither implementation nor audit is waived.
