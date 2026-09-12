# PA13 implementation and completion evidence

This is the stage-completion checkpoint record at `cf1b9621`. The subsequent
[independent final audit](audit.md) supersedes its completion conclusion and
records the repaired paths, reference correction and final evidence. The
historical measurements and validation below are retained as checkpoint facts.

The stage is O0 C++11 source-to-LowIR polymorphism over PA12's single-inheritance
object model. PA24 owns native encoding; the supplied backend is used only by
independent execution and performance harnesses. No reference output, required
fixture, comparison rule or coverage bucket was changed.

## Semantic and lowering trace

For `Derived : Base` with `int f() const override`, the streaming parser retains
one declaration node. `virtual_declaration` records the canonical function
shape without its result type. `complete_virtuals` uses the interned name and
shape to select the inherited slot, checks final/static/covariant/exception
constraints and publishes the selected declaration in the derived slot.
Destructors use a common signature identity and adjacent complete/deleting
slots. Covariance checks the actual class chain and access context, including
pointee cv, without rendered names as keys.

Class, member, expression and ABI relationships remain compact IDs in TU-owned
vectors. Each polymorphic class owns a flat signature index and compact slot
sequence; work tracks actual declarations and inherited/emitted slots. The
member demand queue computes bodies/actions once. Vtable demand is monotonic,
triggered by construction/destruction or a defined key function, and traverses
only required virtual entries. Ordinary unused bodies remain undemanded.

Class layout reserves the first vpointer at byte zero or shares the primary
base's vpointer. Recorded base conversions contain the complete byte offset;
nullable pointer conversion branches before nonzero adjustment. Calls record
both the selected member and virtual slot; explicit qualification records a
direct call. Lowering consumes these facts and the selected exception contract,
without repeating lookup or overload resolution.

Typed globals carry the offset-to-top, RTTI support relocation and ordered
slots. RTTI support identities use PA9's ABI graph. Function-local classes carry
the enclosing function identity and source discriminator, avoiding collisions
between overloads, functions and blocks. Program-owned linkage shares support
roles and class ABI entities across translation units.

Constructor/destructor actions write the class's address point at the correct
lifetime transition. Copy construction initializes the new vpointer; assignment
preserves the destination's dynamic identity. A deleting entry records its
selected deallocator and complete size. Empty bodies expand at most one prepared
suffix; other bodies/suffixes call the complete entry once, preserving exception
cleanup with linear work. The existing larger destruction suffixes remain
bounded. Exception specifications query completed subobject types independently
of body demand, avoiding premature empty-action conclusions.

Function IDs remain stable. A validated emission schedule groups each class's
base/deleting/complete destructor entries without moving or reparsing function
IR. It visits each entry once and is separate from slot order. Function-local
builder state is released after emission; semantic caches die with the TU, and
only canonical ABI/linkage and typed LowIR survive to the program writer.
Telemetry counts virtual declarations, inherited/emitted slots and demand
transitions without additional semantic queries.

## Validation scope

- `make test-pa13`: 37/37, removing all 35 stage-entry failures.
- `make test-report-through-pa13`: 1621/1621, including 1584 inherited tests.
- `perl scripts/cppgm_file_audit.pl --stage pa13 --paths dev/src`: pass, with
  three advisory header-division warnings; no file audit failures.
- Six independent executable sources cover lifetime dispatch, qualification,
  copies/assignment identity, nullable base adjustment, overloaded slots,
  covariance, local identities and sized virtual deletion.
- Fifteen semantic controls include cv/ref signatures, final/override, access,
  covariance and destructor exception specifications.
- Raw IR controls verify D2/D0/D1 ordering, unused body demand, multi-TU identity
  and cleanup growth. Increasing destructor fields from 8 to 64 produces 274
  and 1326 total instructions; both programs execute with checked results.
- The actual compiler built with ASan/UBSan passes all 37 course status/LowIR
  checks, the six native sources, semantic controls and raw IR controls.

The audit-mode literal-address correction follows the LowIR scalar-operand and
memory/addressing rules in [PA8's contract](../pa8/lowir.md), and the explicit
`load ptr 0` nonnull branch in PA13's `300-class-specific-sized-delete.ref`.
The compiler's validator previously rejected that contract form before reaching
its unreachable branch. Literal operands now receive pointer context while
floating pointer operands still fail. A reduced good/bad personal LowIR control
checks both outcomes. This changes the implementation validator, not references
or comparison tolerance.

## Performance and scope

[Performance evidence](../student.tests/pa13/performance.md) reports all four
required dimensions with frozen binaries/inputs, A/A calibration, ABBA blocks
and checked outputs. No optional optimization is introduced, and no speedup is
claimed against PA12's incorrect handling of virtual programs. Native-backend
optimization, template demand, generalized RTTI and self-hosting remain scoped
to their owning later assignments; they are not additional PA13 gates.

Artifacts and full logs reside under `$RALPH_ARTIFACT_DIR/pa13/`. The benchmark
JSON preserves every observation and binary/input/output hash. The compact
[plan and ledger](plan.md) retain both original review markers.
