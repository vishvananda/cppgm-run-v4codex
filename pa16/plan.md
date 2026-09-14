# PA16 compact plan — implementation 41

Stage base commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`
Last reviewed commit: `7c39a6edbfa43c226036b8a92fe236722ac85dcc`

Target: **PA16 full-stage**. Implementation entry: `caac2cda243e4ab71d6f5aca48159f88b9b8da3b`,
93/154 passing, 61 failures. Both review markers above are preserved.

## Active connected group

Owner: the shared semantic constant evaluator. Add compact TU-owned typed
object values and storage/subobject identities; execution frames own local
bindings. Parsed initializer plans, selected conversions and constructor actions
feed evaluation; declarations publish checked values for queries and lowering.
Calls key completed results by function, converted arguments and receiver path.
Projection uses indexed field/base identities and bounded array ranges; work
tracks consumed initializer actions and executed expressions under the existing
512-call / 1,000,000-step bounds. No textual semantic keys or grammar replay.

Implement constructors, aggregates, references/pointers, member/operator calls,
conversions and their declaration/storage obligations together. Validate with
unchanged course fixtures, explicit personal cross-owner/native/rejection controls,
prior-through and file audit. Freeze entry/final binaries and inputs for A/A plus
ABBA compiler latency/RSS and executable runtime/text evidence; new-correct cases
have no speedup comparison against rejected entry programs.

Implementation ledger (not an audit waiver): typed object/address execution now
passes **135/154** unchanged PA16 fixtures, prior-through **2112/2112**, file audit
and **17 native / 8 rejection** personal object controls. Existing scalar/storage/
validity controls and final frozen performance remain to rerun. Six semantic
failures remain in recursive arrow/builtin calls, pack-array queries, subobject
projection and const template parameter resolution; emission identity and static
initialization presentation also remain. Required constant initialization exposes
old dynamic-init references: prove/document each correction before changing any
oracle. Constructor-local field reads, self addresses, scope lifetimes and mutable
reference-call keys now have explicit personal controls. This is an implementation
increment, not the handoff boundary.

The accumulated checkpoint audit is complete;
stage implementation is still **93/154, 61 failures**. Entry `57bac58d` had the
same 61 failures. No passing fixture was lost; all 154 fixtures are unchanged.
This turn made progress: reviewed three handoffs, repaired three defects,
validated the result and froze performance evidence.

## Reviewed implementation

The review covers every commit from the stage base through the code tip above,
all 39 changed implementation paths and their interactions. [audit.md](audit.md)
records the commit groups, ownership traces, proofs and one checkpoint ledger row.
Scalar execution uses temporary frames and canonical completed-call keys;
floating payloads and immutable array ranges have TU owners. Static initialization
is a declaration fact; lowering consumes typed initializer plans, persistent
storage/lifetime records and structurally interned readonly data. Literal-type and
exception facts use declaration/query identity and precise completion demand.

The fixes share specialization context across exception/default/body demand,
leave unavailable constructor probes retryable, and validate implicit conversions
before accepting constexpr initializers. The x87 arithmetic policy is retained;
C++11 permits its excess precision.

## Remaining implementation

- **Object, address and lifetime evaluation:** general class/array values,
  subobject identity/bounds, constructors/base/member initializers, references,
  callables, conversions and complete invocation keys. Finish the connected
  initializer/declaration obligations, including rejecting invalid class-valued
  constexpr initializers.
- **Publication and lowering of those facts:** class/static-pointer constants,
  member projections, demanded static definitions, dependent runtime result
  destruction/ABI emission and object-dependent exception specifications.
- **Ordinary automatic constant arrays:** the README requires a readonly copy,
  while 16 PA10–15 fixtures require stores under identical flags. Both forms
  preserve C++ behavior; a wrong-reference proof is absent. Keep this full-stage
  obligation and the [conflict evidence](storage-performance.md) open without
  weakening coverage or comparison rules.

Avoid more narrow handoffs inside the object/initializer/lifetime group.
The earlier scalar/storage/validity split missed exception-to-body ownership;
future controls should exercise those owners together through storage/emission.

## Acceptance and validation

[Audit performance](audit-performance.md): **854** new frozen observations,
plus **2,658** preserved historical observations. All four performance dimensions
are covered at PA16/O0; no optional optimizer was added. Historical +15% latency,
+16 MiB RSS, 5.5x scaling and earlier text targets are diagnostic, not exit gates.
The 512-call and 1,000,000-step evaluator resource bounds remain unchanged.
Native optimization, encoding and self-hosting retain their later-stage owners.

Fresh `make test-pa16`: **93/154**, exit 2; exact prior-through command:
**2112/2112**, exit 0; through PA16: **2205/2266**, only the same 61 failures.
File audit passes with three inherited header warnings. Personal controls:
**148 native / 66 rejection**, all pass. The [record](../student.tests/pa16/audit-checkpoint.json)
and [verifier](../student.tests/pa16/verify_audit.py) bind code, unchanged contract,
logs, binaries and accumulated evidence. PA17 advancement still requires
completing PA16 and passing its full through report.
