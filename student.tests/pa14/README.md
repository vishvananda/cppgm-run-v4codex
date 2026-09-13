# PA14 personal validation and performance

Run from the repository root:

```sh
python3 student.tests/pa14/check_functions.py
python3 student.tests/pa14/check_fixed_objects.py
python3 student.tests/pa14/check_dependent_objects.py
python3 student.tests/pa14/check_object_reducers.py
python3 student.tests/pa14/check_declaration_types.py
python3 student.tests/pa14/check_demand_regions.py
python3 student.tests/pa14/verify_performance.py
```

`check_functions.py` compiles all twenty-four local `.cpp` sources with LowIR validation,
then runs the generated programs through PA8's supplied native backend. They
cover specialization demand/identity, compatible declarations, lazy class
completion, calls/operators/defaults/references, static function addresses,
overloaded argument deduction, ordinary/template overload sets, local hiding
and converting class references. New sources cover dependent qualified types,
renamed out-of-class/nested definitions, late definitions, class defaults,
explicit class demand, ellipsis conversions and evaluated/unevaluated storage.
The compiler implements the LowIR itself.

`check_sanitizers.py RELEASE SANITIZED` runs all 314 course sources and twenty-four
personal sources through both frozen compilers. It requires equal status,
byte-identical successful LowIR and no ASan/UBSan report. Rejection parity for
incomplete-stage inputs is a memory-safety check, not a course correctness pass.
The current campaign checks 338 inputs with GCC's address and undefined
behavior sanitizers, leak detection and halt-on-error enabled, plus 124 explicit
binding/query/scalar/call/object/default rejection cases. All seven `.t` reducers also
pass release/sanitizer output parity and native execution. The ABI controls
are in `check_queries.py` and `check_value_queries.py`.

The [performance review](../../pa14/performance.md) explains the raw JSON and
acceptance. `benchmark.py A B WORK OUT` uses the fixed PA10 compiler/native
corpus plus PA14 template workloads. New semantic cases are measured only on
the working compiler. `graph_read_benchmark.py B C WORK OUT` isolates the source
graph read change using the frozen preliminary corpus. Inputs, outputs, binaries,
flags, hashes, wall/RSS/context-switch observations, A/A calibration and ABBA
pairings are retained. The verifier checks all artifacts and pairings, including
historical measurements; it does not require a timing threshold.

Frozen binaries and generated inputs/outputs live under
`$RALPH_ARTIFACT_DIR/pa14-entry/`, `pa14-measurements/` and `pa14-dependent/`.
Raw JSON contains their absolute paths and SHA-256 identities. Those external artifacts are required to
rerun hash verification; they are not compiler implementation inputs. No generated
binaries, objects, test logs or `.my*` files are committed.

`definition_benchmark.py A B WORK OUT` freezes the continuation-entry/current
comparison on the previous common corpus plus new nested/member/static definition
scaling and a live out-of-class member-call executable. Its 308 observations live
in `definition-performance.json`; the same verifier checks them alongside the
1,148 historical observations. No numerical timing threshold filters results.


`check_bindings.py`, `check_queries.py`, `check_fixed_expressions.py` and
`check_fixed_calls.py` run definition-time rejection and non-demand controls;
each accepts a frozen compiler path for sanitizer repetition. Fixed calls cover
ADL, indirect/reference parameters, class temporaries/defaults, access, deleted
conversions and unused selected-specialization bodies.

`call_benchmark.py A B WORK OUT` extends the frozen body corpus with fixed-call,
unused-definition and class-conversion scaling plus two checked native loops.
A/A calibration and two ABBA blocks retain every sample and output hash. The
harness and input corpus are frozen before timing; no numerical gate filters
observations or replaces correctness.

`fixed-objects.cpp` covers fixed class/pointer objects, member cv/categories,
virtual/qualified/static calls, reference casts, function-pointer fields and
callable objects. `default-object-identity.cpp` checks repeated default values,
conditional lifetimes, conversions, constructor defaults and array-element
cleanup. Its per-value instance counts tolerate optional return copy elision.
The two reduced `.t` inputs preserve the entry failures independently.

`dependent-objects.cpp` checks template-owned implicit/explicit `this` fields,
mutable/reference members, fixed-base access, bit-field promotion, nested and
out-of-line owners, late forward declarations and prototype parameter queries.
`check_dependent_objects.py` rejects fourteen invalid unused member bodies; the
entry compiler accepted each reducer. `parameter-shape.t` separately checks
earlier parameter names in subsequent `decltype` parameter types for template
members and an ordinary function. `method-parameters.t` covers nested
function-pointer returns, static/nonstatic overloads and deferred enum signatures.

`dependent_object_benchmark.py A B WORK OUT` freezes sixteen compiler workloads
and four checked native loops. Two full campaigns and focused follow-ups retain
specialization-count, repetition, unused-body, out-of-line and inherited control
measurements, including every outlier. `prototype_benchmark.py` records required
prototype-scope costs; `method_parameter_benchmark.py` adds nested declarator
scaling and a checked native loop with a complete prior-output preflight.
Eight new campaigns add 1,162 observations; all 7,266 historical/current
observations verify. Artifacts and reducer proofs live under
`$RALPH_ARTIFACT_DIR/pa14-dependent-objects/`. The verifier checks complete orders,
outputs, unchanged hot record sizes, source/context work equations, fourteen
entry-accepted invalid bodies and four frozen final reducer inputs. Earlier
versions and observations remain preserved.

`object_benchmark.py A B WORK OUT` retains all 27 preceding compiler inputs,
adds six receiver/result scaling inputs and two native loops, and measures
three corrected default-identity inputs on B only. Its 644 observations include
the failing A native identity proof. `object_repeat_final.py WORK OUT` repeats
six noisy cases from that frozen corpus; the interrupted first harness and its
fourteen completed observations remain preserved. `object_view_benchmark.py
A B WORK OUT` isolates immutable conversion-call views on ten correct compiler
inputs and two native loops. All frozen artifacts live in
`$RALPH_ARTIFACT_DIR/pa14-object-facts/`; the verifier retains every campaign.

`signature-facts.cpp` and `declaration-types.cpp` cover raw parameter types,
canonical local/field/alias types, renamed declaration heads, cv/reference
queries, private access, overloads and dependent using-declarations.
`check_declaration_types.py` explicitly runs eight rejection controls, including
five newly rejected unused function-pointer bodies. `declaration_type_benchmark.py`
freezes sixteen inputs and five native loops before AA/ABBA measurements; its
two campaigns preserve 588 additional observations. `declaration_type_evidence.py`
retains positive native and negative compiler proofs. `declaration_layout_probe.cc`
is compiled explicitly by the recorded layout command, outside the native `.cpp`
suite. Both the proof and layout artifacts are checked by the performance verifier.

`demand-regions.cpp` checks independent body, constructor-initializer and default
argument demand, including unused dependent defaults, explicit arguments, repeated
side effects, nested/local classes and renamed definition heads. `default-heads.t`
reduces declaring-head lookup and defaults used before their later definitions.
`check_demand_regions.py` runs twelve rejection controls and that reducer through
the compiler/native backend. Initial-only default declarations follow N3485
[dcl.fct.default]/4–6; the original permissive personal source and its correction
are retained in the artifacts. Course fixtures and references are unchanged.

`region_benchmark.py` freezes 21 compiler workloads and five checked native loops.
Three campaigns retain 1,008 observations, including the intermediate fully-used
body slowdown. `region_cache_benchmark.py` isolates cached source topology from
the already-correct demand implementation with another 70 observations. The
verifier checks all 8,932 observations, source/default work equations, complete
AA/ABBA orders, exact common-correct LowIR/native hashes, unchanged hot records,
current source-region layouts and six newly rejected invalid defaults. Frozen
proofs and binaries live under `$RALPH_ARTIFACT_DIR/pa14-regions/`; historical
layout probes retain their header snapshots, while the new current probe also
checks the live headers. None of these artifacts are compiler inputs.

`region-attributes.t` additionally checks alignment operands and packing on local
classes inside demanded member bodies. `check_object_reducers.py` runs it with
the earlier object/prototype reducers. The entry, final release and sanitizer
compilers preserve identical validated LowIR and checked native output.


`body-values.cpp` checks fixed expression types and selected conversions around
value-dependent sizeof/alignment. `dependent-bounds.cpp` checks dependent array
identity/deduction, renamed declarations, static constants/enumerators, adjustment
of array parameters, scalar casts, and conditional/logical evaluation. The
`value-conversion.t` reducer preserves O0 constant-width decisions for fundamental
and class-template operands. Run `check_body_values.py` and
`check_value_queries.py` explicitly; together they cover 22 rejection cases and
four new ABI forms. Six invalid unused bodies were accepted by the entry compiler;
`value_query_evidence.py` retains both results and C++11 rule references.

`value_query_benchmark.py A B WORK OUT` adds source/repetition/key scaling and two
checked native loops to all 21 preceding inputs. `PREFLIGHT_ONLY=1` checks the
entire corpus without timing. The frozen final campaign records compiler wall/RSS,
executable runtime/payload, A/A calibration and two ABBA blocks. New array cases
rejected by A have B-only observations. `value_query_validation.py RELEASE
SANITIZED WORK OUT` records all 337 parity inputs, 124 rejection controls, six ABI
controls and seven reducer output/native checks. The value-stage layout was recorded with
`value_layout_probe.cc` and frozen transitive headers; prior header snapshots
remain intact. Artifacts live under `$RALPH_ARTIFACT_DIR/pa14-value-facts/`.


`expression-owners.cpp` checks fixed call/default inputs, distinct local and
receiver identities, conditional reference lifetimes, full-expression temporary
cleanup, move-only values and indirect reference calls. `expression-store.cc` is
a separately compiled storage control: immutable properties share source IDs,
concrete bindings and incoming/evaluation state remain independent, conversion
variants are shared, and snapshots survive recursive arena growth.

`expression_owner_benchmark.py A B WORK OUT` retains all 28 value-stage compiler
inputs and adds three source/instance call-input scalings and one checked native
loop. Each full campaign has 32 compiler and eight executable measurements,
each with warmups, A/A calibration and two ABBA blocks.
`expression-owner-performance.json` retains the initial ownership campaign;
`expression-view-performance.json` records the local-view follow-up against the
same entry compiler and inputs. Neither campaign replaces earlier observations.

`expression_owner_validation.py RELEASE SANITIZED WORK OUT ENTRY` records 338
parity inputs, 124 rejection controls, six ABI controls, seven reducers, storage
controls under both builds, and the lifetime program under entry/release/sanitizer
builds. Both validation manifests are retained. `expression_owner_layout_probe.cc`
and 17 frozen transitive headers establish the current property/use records and
public views; historical layout artifacts remain unchanged. The full verifier
also checks current header hashes, ownership/scaling equations and required root
check manifests. Artifacts live under
`$RALPH_ARTIFACT_DIR/pa14-expression-owners/`.
