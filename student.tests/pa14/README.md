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

`check_functions.py` compiles all thirty-three local `.cpp` sources with LowIR validation,
then runs the generated programs through PA8's supplied native backend. They
cover specialization demand/identity, compatible declarations, lazy class
completion, calls/operators/defaults/references, static function addresses,
overloaded argument deduction, ordinary/template overload sets, local hiding
and converting class references. New sources cover dependent qualified types,
renamed out-of-class/nested definitions, late definitions, class defaults,
explicit class demand, ellipsis conversions and evaluated/unevaluated storage.
The compiler implements the LowIR itself.

`check_sanitizers.py RELEASE SANITIZED` runs all 314 course sources and 33
personal sources through both frozen compilers. It requires equal status,
byte-identical successful LowIR and no ASan/UBSan report. The current signature
publication campaign checks 347 inputs with leak detection and halt-on-error
enabled, plus 166 required rejection controls and one optional unused-default
diagnostic. Seven inherited and four signature `.t` reducers also pass
release/sanitizer output parity and native execution. Direct initialization,
expression/fact storage and lifetime controls remain included. The six ABI
controls are in `check_queries.py` and `check_value_queries.py`.

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


`definition_demand_benchmark.py A B WORK OUT` retains the full 32-input expression
corpus and adds N/K/Q scaling and a live member-call loop. The resulting 37 compiler
inputs and nine native programs record 644 observations. `definition_demand_evidence.py`
freezes twelve entry-accepted invalid-definition proofs, four exact LowIR/native
controls and the current 17-header layout probe. `definition_demand_validation.py`
records 342 sanitizer parity sources and 136 rejection controls, including the
new `check_definition_demands.py`. `verify_definition_demands.py` checks these
artifacts and is included by the cumulative verifier (11,158 observations).

`definition-demands.cpp`, `definition-signatures.cpp`, `definition-overloads.cpp`
and `definition-parameters.cpp` cover repeated/late demand, overload selection,
renamed nested alias heads, body parameter cv/array/function forms, dependent
noexcept and nested function-pointer returns. Source matching uses N3485
[class.mem]/1, [class.mfct]/1–2, [except.spec]/3–4 and [basic.def.odr]/1; host
agreement supplements those rules. No course/reference output changed.

`definition-demand-handoff.json` records the final command statuses and preserves
initial failures and intermediate binaries. Its separate Massif profiles diagnose
the disclosed native RSS increase on a retained call-input case; they are not
timing observations. The full frozen campaign and both profiles remain under
`$RALPH_ARTIFACT_DIR/pa14-definition-demands/`. Performance acceptance and remaining
special-member/declaration/lifetime/demand ownership are documented in the plan.

`special-signatures.cpp` and `injected-signatures.cpp` execute distinct conversion
targets, constructor/copy/move/assignment signatures, renamed nested heads, raw
aliases, returned references and late defaulted special members. Run
`check_special_signatures.py` explicitly for 21 invalid definitions; entry accepted
20 of them. `special_signature_evidence.py` records those C++11 rule proofs, both
new positive programs and the current transitive layout probe. Entry rejected
both positive programs; the frozen injected-head intermediate and final compiler
accept them and produce identical checked native output.

`special_signature_benchmark.py A B WORK OUT HEAD_BASELINE` retains all 37 previous
inputs and adds constructor N/K/Q scaling, nested-head scaling and a live native
loop. The two nested-head cases compare the correct intermediate compiler against
the final direct-application compiler. Other cases compare continuation entry
against final. `PREFLIGHT_ONLY=1` performs output checks without timing. The full
campaign measures 44 compiler inputs and ten executables, with one warmup each,
four A/A samples and two ABBA blocks per campaign: 756 observations.

`special_signature_compare.py` and its Perl adapter use the unchanged course
validator and canonicalizer, validating both compiler outputs in student mode.
Five new constructor outputs differ in local slot suffixes but have equal
canonical LowIR; 39 outputs match byte for byte. The adapter does not use the
generated-projection fallback. All ten executable hashes match. The initial
byte-only and reference-order preflight failures are preserved; no fixture,
reference or course comparison rule changed.

`special_signature_validation.py` records 344 release/sanitizer parity sources,
157 rejection controls, six ABI controls, seven native reducers, thirty native
programs and store/lifetime controls. `verify_special_signatures.py` verifies the
proofs, baseline identities, measurements, work equations and frozen layouts; the
cumulative verifier includes this campaign. Earlier layout probes remain frozen
snapshot evidence. `special-signature-handoff.json` preserves command statuses,
failed initial probes and intermediate binaries under
`$RALPH_ARTIFACT_DIR/pa14-special-signatures/`.


`local-declaration-facts.cpp` checks local/nested/shadowed class and enum identities,
raw/decltype/reference aliases, layout and bounds, copies and destruction across
function specializations. `direct-initializer.t` isolates literal cast operands
in direct initialization while preserving possible function declarations.
`fact-store.cc` explicitly verifies absent reads allocate no records and references
remain valid across optional-index and record-slab growth.

`declaration_fact_validation.py RELEASE SANITIZED WORK OUT ENTRY` records 345
release/ASan/UBSan parity inputs, 157 rejection controls, six ABI controls, seven
inherited reducers, 31 native programs and the initializer/store/lifetime controls.
`declaration_fact_evidence.py` retains the two positive C++11 proofs, entry
rejections, the correct declaration intermediate and the current 18-header layout
probe. Earlier probes keep their immutable snapshots. No course fixture or
comparison rule changes.

`declaration_fact_trial.py A B WORK OUT` retains two isolated three-input campaigns
for sparse records and then grouped writable views (84 observations total).
`declaration_fact_benchmark.py A B WORK OUT` retains all 44 prior inputs, adds four
local declaration N/K/Q scalings and a checked live native loop. The full campaign
has 49 compiler inputs and eleven executables, with warmups, A/A calibration and
two ABBA blocks (840 observations). `PREFLIGHT_ONLY=1` checks the entire frozen
corpus without timing. `verify_declaration_facts.py` is included by the cumulative
verifier and checks hashes, every observation, work/storage equations, proofs,
current layouts and required checks. Frozen binaries, outputs and failed initial
probes live under `$RALPH_ARTIFACT_DIR/pa14-declaration-facts/`.


`signature-publications.cpp` executes raw cv/array/function parameters, trailing
queries, static/nonstatic prototypes, late private/nested defaults and non-static
initializers, including initialization order and overridden dependent failures.
`enum-signatures.cpp` exercises member/local/anonymous enum identity, renamed
heads, nested definitions and overload separation. The personal native catalog
now contains 33 programs. `check_signature_publications.py BINARY` explicitly
runs nine required rejections, an optional early-diagnostic observation and four
reduced native programs. N3485 [temp.decls]/2, [temp.res]/8 and [temp.inst]/1,12–13
justify retaining the unused private-default case as optional and demanding
rejection in a separate call using the default. The initial stricter harness and
its host acceptance remain frozen.

`signature_publication_evidence.py A B WORK OUT CLASS_USE_BASELINE` preserves six
C++11/native proofs and the current 18-header layout probe. The first and two
late-member reducers fail under entry; the correct class-use intermediate and
final compiler execute them. `signature_publication_validation.py RELEASE
SANITIZED WORK OUT ENTRY` records 347 parity sources, 166 required rejections,
one optional diagnostic, six ABI controls, eleven native reducers and inherited
initializer/store/lifetime controls (118 recorded checks).

`signature_publication_benchmark.py A B WORK OUT` retains all 49 previous inputs,
adds four signature N/K/Q scalings and a checked live native loop. The complete
54-input/12-native campaign has 924 observations: one warmup each, four A/A
samples and two ABBA blocks for each compiler/runtime campaign. The new timing
sources place their initializer/default target before uses so both frozen
compilers are correct. Late-member fixes have separate correctness proofs;
rejected entry programs are never treated as valid timing baselines.
`PREFLIGHT_ONLY=1` checks output equivalence without timing. The unchanged course
comparison adapter is available for presentation differences.
`verify_signature_publications.py` checks the full evidence and live layouts;
older probes keep immutable snapshots. Artifacts, including the original raw
parameter failure and correct qualified-enum intermediate, are under
`$RALPH_ARTIFACT_DIR/pa14-signature-publications/`.

The full campaign's wide local-declaration case changed from 11-second A/A
samples to 25–31-second A/B samples, affecting both frozen compilers.
`signature_publication_repeat.py PARENT WORK OUT` repeats that unchanged case
with equivalent output checks and the same 14-observation protocol. It persists
each completed sample. The original observations remain in the full campaign;
this repeat investigates the apparent regression without changing production.
