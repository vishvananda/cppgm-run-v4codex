# PA14 personal validation and performance

Run from the repository root:

```sh
python3 student.tests/pa14/check_functions.py
python3 student.tests/pa14/check_fixed_objects.py
python3 student.tests/pa14/check_dependent_objects.py
python3 student.tests/pa14/check_object_reducers.py
python3 student.tests/pa14/verify_performance.py
```

`check_functions.py` compiles all eighteen local `.cpp` sources with LowIR validation,
then runs the generated programs through PA8's supplied native backend. They
cover specialization demand/identity, compatible declarations, lazy class
completion, calls/operators/defaults/references, static function addresses,
overloaded argument deduction, ordinary/template overload sets, local hiding
and converting class references. New sources cover dependent qualified types,
renamed out-of-class/nested definitions, late definitions, class defaults,
explicit class demand, ellipsis conversions and evaluated/unevaluated storage.
The compiler implements the LowIR itself.

`check_sanitizers.py RELEASE SANITIZED` runs all 314 course sources and eighteen
personal sources through both frozen compilers. It requires equal status,
byte-identical successful LowIR and no ASan/UBSan report. Rejection parity for
incomplete-stage inputs is a memory-safety check, not a course correctness pass.
The current campaign checks 332 inputs with GCC's address and undefined
behavior sanitizers, leak detection and halt-on-error enabled, plus 82 explicit
binding/query/scalar/call/object rejection cases. All four `.t` reducers also
pass release/sanitizer output parity and native execution. The ABI controls
remain part of `check_queries.py`.

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
