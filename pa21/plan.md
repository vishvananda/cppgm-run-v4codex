# PA21 implementation plan

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Target: PA21 full-stage. Phase: implementation handoff (loop 102).
Entry: clean; 24/116 pass, 92 fail. Now: **37/116 pass, 79 fail**, no regressions
or reduced coverage. Independent whole-stage audit is still pending.

## Design/spec alignment

RTTI/casts complete for the standalone PA21 group: 18/18 named fixtures pass.
Canonical formation/evaluated facts feed typed LowIR; cached per-TU RTTI and
program ABI identities replace the old sizeof/static-cast interpretation.
Queries preserve demand boundaries, cv/reference/function/array identity,
access, incomplete types and cross-TU linkage. Lowering emits once per identity;
work is bounded by type/dependency edges and produced IR/name bytes. Related
fixes cover function/member-pointer parsing and allocated-copy constructor entry.
[Design, work bounds and runtime limitation](rtti102.md).

## Remaining implementation groups

Counts assign each current failure one primary owner; compositions cross owners.

| Group / owner | Data flow and complexity obligation | Remaining / validation |
|---|---|---|
| Captures / closure facts and construction | Indexed environments select value/class captures and copy/destructor dependencies; checked closure fields feed lowering. Linear in actual captures/dependencies. | 13; explicit/default/this, mutable, template calls and two capture+RTTI fixtures. |
| Initializer lists / initialization and overload selection | Canonical element/conversion plans own backing storage, list ranking and lifetime; lowering consumes those plans once. | 24; braced calls/construction/return, deduction, ranges, scalar/class elements and storage duration. |
| EH and lifetime / cleanup/control flow | Typed active handler and constructed-subobject states own unwind continuations; share only identical complete contexts. Work bounded by emitted control/cleanup edges. | 42; throw/try/catch, subobject failure, argument ownership, conditional temporaries, destructor termination, local-class destructor and `Guard(*this)()` parsing composition. |

Boundary: the remaining RTTI compositions reject at `unsupported value capture`,
before RTTI lowering. Further progress requires closure field/copy/destructor
ownership and template capture demands, or list backing storage/typed unwind
state. Those are new representation/state groups; extending the completed RTTI
paths cannot repair them. All remain required implementation, not audit deferrals.

## Performance evidence

[Protocol/results](performance102.md), [raw observations](../student.tests/pa21/performance102.json).
Frozen baseline/final, A/A and ABBA on equivalent inherited workloads: LowIR and
executables byte-identical. Compiler text +18,624 B (0.89%); heavy template RSS
+204 KiB. No runtime improvement claimed. 800→3200 RTTI specializations produce
800→3200 records, compile 127→482 ms, peak RSS 16,792→49,204 KiB. Pointer-chain
names are charged to produced bytes. Live typeid/cast loops checked. PA21/O0
uses spec §9 bounds; no optional optimization or extra numerical gate. Historical
self-selected ratios are diagnostics; preserved measurements and mandated limits
remain intact. Own native optimization/self-hosting belong to later stages.

## Handoff ledger

Loop 102: `30fe6353` proves/corrects one oracle substitution digit; `9f2181f9`
implements RTTI/casts and related fixes. [Validation](../student.tests/pa21/validation102.json):
`make test-pa21` 37/116 (exit 2); prior-through 3596/3596 (exit 0); file audit
passes (three inherited advisory header warnings). Thirteen old failures resolved;
all fixture inputs/comparison rules preserved. Personal controls: 59/60 with the
supplied freestanding runtime, 48/48 positive host-runtime checks, including the
same-LowIR proof for its one external runtime limitation. Rejections all pass.
No known compiler defect remains open in the completed group. Independent whole-stage
correctness/architecture review is still owed; review markers remain unchanged.
This handoff completes loop 102 implementation work, not PA21 or its audit.
