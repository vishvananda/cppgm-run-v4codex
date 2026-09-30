# PA29 implementation / handoff155

Target: **PA29 full-stage**. Phase: implementation handoff; stage unfinished.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Entry: clean; 182/403 passing, 221 failing. Exit: **272/403**, 131 failing;
**90 existing failures resolved, no new failures**, unchanged course coverage.

## Design/spec alignment

| Completed owner | Data flow / complexity | Evidence |
|---|---|---|
| Hosted driver/preprocessor mechanics | Build-time config + ordered flags/forced includes → streaming token cursor → PA4 posttoken output or shared source compiler. O(source + expansion/include edges); no runtime host probing. Include search, byte-discarding comments, operand grammars and macro-expanded attribute probes are shared mechanisms. | PA29 preproc, ordering/error controls, earlier PA4 |
| Hosted numeric forms | One validated binary/hex-float/suffix decode → canonical literal bytes → selected cooked/pack constexpr UDL facts. No source-answer substitution. | Literal controls, object execution and LowIR roundtrip |
| Canonical shape/transform/operation queries | Registry → retained typed arguments → interned query fact. Shape O(1), arrays O(rank); conversions/destruction demand ordinary candidate and definition facts. TU-owned caches key canonical types/queries, never printed names. | Shape/cv/rank/aggregate, access, negative conversion/destruction, template controls |
| Shared semantic fixes and inspection | Explicit candidate filtering precedes deduction; invalid reference binding cannot be repaired with a copy; cached default/transfer properties use definition access; late-defaulted exception specifications preserved. Trait ABI operands stay typed. `-c --emit-lowir --validate-lowir` uses the exact hosted `build_source` Program consumed by object emission. | Four source→LowIR→reader→native/MIR executions; PA1–28 |

No hosted-only backend, body replay, ambient mutable cache or textual production
IR roundtrip was introduced. Ordinary names sharing builtin spellings remain
ordinary names unless followed by the intrinsic operand grammar. Feature probes
share builtin registries; unsupported operations remain unadvertised. Exception
feature configuration is independent of source macro redefinition.

## Remaining implementation groups

Exact failures/diagnostics: [owner ledger](../student.tests/pa29/evidence155/remaining.json).
Counts are assigned by the first relevant owner, not independent root-cause proof.

| Owner / failing cases | Required data flow and complexity | Next validation |
|---|---|---|
| Atomic/assembly: 21 | Typed atomic qualifier and memory-order/effect facts → serializable LowIR → native/legal runtime operation. Work bounded by operands/IR, preserve no-unwind facts. | Atomic compile/run, 16-byte alignment, noexcept inspection |
| Scalar/runtime intrinsics: 33 | Registry signature and arity → canonical call/intrinsic → LowIR/C ABI, O(operands). Includes integer, memory, libm, source-location and invoke forms. | Probe, wrong-arity, width/runtime controls, LowIR parity |
| Extended syntax/types/layout: 51 | Retained attributed/type syntax → dependent width/layout/initializer facts → typed lowering. O(tokens + demanded layout edges), no ignored semantics. | Block/nullability/vector/complex/float forms, attributes, designated init, folds/lambdas/bindings, no-unique-address copies |
| Legacy traits/lifetime proofs: 6 | Declaration properties for *all* relevant special members; reference-temporary queries consume binding/materialization/lifetime facts. These are not aliases for ordinary overload viability. | Legacy copy/trivial/default-constructor reducers, temporary binding controls |
| Template demand/hosted ABI: 20 | Existing specialization/demand owners → canonical context/arguments → ABI names and emitted definitions. O(actual candidates/dependencies), no library-name exceptions. | Pack/alias/invocation/default-cache reducers, pretty-function, extern/inline/force-inline and symbol controls |

## Performance acceptance

[Measurements and budgets](performance155.md) apply spec §9 to this stage.
Mandatory semantics were added; new optional optimizer work/growth budgets are
**zero**. The inherited blanket 15%/zero-growth targets remain diagnostics, not
mandated exit gates; historical observations are preserved. Common fixed-input
A/A + ABBA measurements include all four cost dimensions and full spread; new
capability/scaling costs are separate from the failing baseline. No speedup is
claimed. Self-hosting and later-stage optimization budgets remain later owners.

## Handoff ledger

- Commits: initial plan `66bb1c6b`; driver/literal/query group `4a428561`;
  query/access/inspection extension `f8f8f342`; exception metadata `6bf1369b`.
- Final checks: PA1–28 **4538/4538**, explicit controls **77/77**, file audit
  passes with four inherited header-body warnings. PA29 **272/403** still
  returns 2. [Validation](../student.tests/pa29/evidence155/validation.json)
  records hashes and the 21,987 unchanged earlier/current fixture files.
  A four-test alias-name regression was found in validation and fixed before
  handoff. No tests, references or comparison rules were changed.
- Boundary: completed the hosted input/decoding/canonical-query group and its
  shared correctness defects, extending through attribute expansion, ordinary
  alias parsing, protected-base definition access and runtime-feature metadata.
  Further progress now needs the distinct typed operation/type/layout/lifetime
  owners above; extending registry strings or token acceptance cannot implement
  their effects, storage, access or ABI contracts. Those requirements remain
  unfinished implementation, not questions deferred to waive implementation.
- Independent review questions: whole-stage source-to-ELF/spec audit; confirm
  the shorthand `std::is_nothrow_*` fixture's forward-declared-only classes and
  the deleted-copy triviality oracle against applicable C++11 rules. No proof
  bundle or reference correction is claimed; both fixtures still count as
  failures. Review markers remain unchanged; this handoff is not stage approval.
