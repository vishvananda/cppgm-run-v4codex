# PA16 compact plan — implementation 41

Stage base commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`
Last reviewed commit: `7c39a6edbfa43c226036b8a92fe236722ac85dcc`

Target: **PA16 full-stage**. Turn entry: `caac2cda243e4ab71d6f5aca48159f88b9b8da3b`,
93/154 passing. Implementation tip: `f1497ca2`, **146/154** passing: 54 prior
failures resolved and one new output mismatch, a net reduction of 53 failures. Both review markers are preserved.

## Design/spec alignment

Owner: the TU semantic constant evaluator, extended through declarations,
template queries and typed lowering. Canonical aggregate payloads are separate
from storage/subobject identity. Selected initializer/conversion/constructor
plans feed execution; frames own bindings/lifetimes; declarations publish checked
values and relocation facts. Constructor builders support ordered field reads
and self addresses. Call keys include referenced storage versions/liveness.
Arrow selection records feed evaluation, runtime calls, exceptions and cleanup.
Consumed constant values demand literal symbols; literal data is emitted in a
separate slice after other globals, retaining pointer addends.
Class completeness precedes destructor/ABI demand. Explicitly defaulted helpers
retain their established runtime policy; implicit trivial base copies use their
existing representation facts.

Complexity: object interning is proportional to initializer parts; omitted array
ranges remain sparse. Field/path lookup is indexed, array projection logarithmic,
address offsets cached. Dependency traversal visits address-containing children.
Calls retain the 512-depth / 1,000,000-step resource policy. No source replay,
fake frontend nodes, serialized semantic keys or reference-backed implementation.
[Object evidence](object-performance.md) records data flow, costs and validation.

## Remaining implementation groups

| Owner | Unfinished requirement / validation |
| --- | --- |
| Static initialization and O0 emission | Seven unchanged comparisons expect zero storage plus startup initialization where checked constants now emit static data/relocations: base-copy constructor, object reference binding, static string member, unary operator result, reference-parameter identity, static-object alias, function-and-constructor. Reconcile output policy with mandatory constant initialization; rerun these fixtures and prior-through. |
| Class-result ABI and cleanup emission | Dependent nonliteral-result instantiation now has one valid destructor identity, but its indirect result and cleanup structure differ from the empty-class reference. Establish the inherited ABI/output contract; preserve native lifetime behavior and earlier class-value fixtures. |
| Automatic scalar-array initialization | The README requires readonly data plus one copy for every fully constant nonvolatile trivial scalar array. Sixteen PA10–15 oracles still require stores under identical flags. The [existing conflict evidence](storage-performance.md) remains open; no coverage or comparator change is authorized by semantic equivalence alone. |

The static string-member fixture is the one newly mismatching case: its pointer
now appears as static relocation data instead of a startup constructor call. The
new native string-object control verifies pointer contents and constant reads.
This output mismatch still counts as a failure. All 154 course tests and their
references are unchanged.

These are unfinished requirements, not waived audit questions. No reference has
been revised. N3485 [basic.start.init]/2 requires static initialization before
dynamic initialization, but that alone does not prove each exact fixture's
startup-form LowIR incorrect: a startup sequence can implement static
initialization. Any correction still needs the user's reduced-proof protocol.

## Performance acceptance

[Object performance](object-performance.md) preserves **804** new frozen A/A,
ABBA and follow-up observations covering compiler latency/RSS and runtime/text.
All common LowIR and four runtime executable pairs are byte-identical. A measured
scalar fast path improves compiler latency 2.6–3.2% over the first implementation;
final scalar templates cost about 6–8% over turn entry. Large runtime-function
compiler medians are near entry; timing spikes and new-correct-only scaling are
reported without discarding observations. Compiler text grows 56,320 bytes.
No runtime speedup is claimed. Historical unsupported percentage/RSS/scaling
gates remain diagnostic under the stage-scoped spec; resource limits, correctness
and coverage remain mandatory. [Audit evidence](audit-performance.md) preserves
3,512 earlier observations and the prior checkpoint review.

## Handoff ledger

| Checkpoint | Result |
| --- | --- |
| `5048b92b` | Typed constexpr objects, subobject addresses, invocation storage, constructors/conversions and native controls. |
| `a707845c` | Dependent array queries, selected recursive arrows, complete lifetime/ABI demand, pointer identities; extended related controls. |
| `135242c1` | Scalar conversion cost repair; frozen compiler/runtime evidence and noise follow-up. |
| `f1497ca2` | Demand string symbols from consumed constant values; defer literal data to preserve global slices and string addends; native static-string control. |
| Current validation | PA16 **146/154** (exit 2); prior-through **2112/2112** (exit 0); through PA16 **2258/2266** (only the eight listed mismatches); file audit passes with three inherited header warnings; **173 native / 75 rejection** personal controls pass. |

Implementation handoff boundary: the connected object/address execution group
now accepts and validates every successful PA16 input, rejects required invalid
inputs, and has native cross-owner controls. Remaining course failures require a
separate, consistent O0 initialization/ABI policy spanning earlier-stage storage
contracts. More evaluator patches cannot settle those output comparisons;
rewriting goldens now would lack the required proof. This is an incomplete
implementation handoff, not assignment certification or permission to advance.

Independent review remains due for the accumulated change since the preserved
review marker, including cache/lifetime completeness, query adapters and ABI
policy. That review does not substitute for the unfinished implementation above.
The [checkpoint](../student.tests/pa16/objects-checkpoint.json) and
[verifier](../student.tests/pa16/verify_objects.py) bind the code, unchanged course
fixtures, raw measurements and checks. Earlier review history remains in
[audit.md](audit.md); historical plans do not add performance exit gates.
