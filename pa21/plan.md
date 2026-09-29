# PA21 compact plan — final audit 113

Target: **PA21 full-stage**. Phase: **audit complete**.
Stage base: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Audit entry: clean `f3af4630` (completed implementation 112).
Last independently reviewed implementation: `30aec177a33f45e1cfb07d43a1790862704e8664`.
No PA21 implementation handoff remains unaudited; no advancement to PA22 was
performed in this task.

## Final design and Spec Alignment

The cumulative compiler keeps immutable source buffers, a streaming token cursor
and one parsed source graph with compact canonical semantic facts. Template
specializations use retained regions, immutable substitution frames and demand
states; fixed expressions are reused. Indexed lookup selects declarations and
conversions once. Typed LowIR consumes construction, lifetime, RTTI, capture,
initializer-list and ABI records directly. There is no production text roundtrip
or host/reference implementation delegation.

The [whole-stage audit](audit.md) reconstructs §§1–10 of `spec.md` from source,
traces a nontrivial declaration and two demanded template bodies through LowIR
and supplied ELF execution, and reviews all **42** stage commits / **93** changed
implementation paths. The prior [105](audit105.md) and [109](audit109.md) reviews
are preserved. Current evidence includes plain-versus-telemetry output identity,
precise demand/work counters, native disassembly and one compiler `execve`.

## Completed audit repairs

- Stable-index emission drains nested omitted-aggregate helper requests exactly
  once, without invalidating vector traversal or leaving missing bodies.
- Automatic and heap-array prefixes compose with source catches, enclosing live
  objects, active handlers and default-argument temporaries.
- Typed release actions retain failed-new and throwing-delete storage ownership;
  remaining array elements are destroyed before storage is released. Placement
  arguments are saved once; matching, sized delete, access and body demand stay
  with the semantic owner.
- Implicit runtime declarations no longer inherit an enclosing template head.
  New-array/delete query facts and dependent allocation ABI expressions reach
  lowering and encoding without reconstructing source semantics.
- Dynamic `typeid` blocks aggregate-helper argument hoisting because its operand
  can observe previously initialized members. Static type queries retain the
  proven small form.

The [one reference correction](reference-corrections113.md) supplies missing
failed-constructor release in an inherited PA12 fixture. Its C++11 proof,
reduced original/revised execution, bundle revision and exact reconstruction
are recorded. Sources, exit statuses, comparisons and coverage are unchanged.
All six accumulated reference-revision scripts pass.

## Performance acceptance and budgets

[Performance 113](performance113.md) retains frozen binaries, inputs, flags,
A/A calibration, ABBA samples, paired spreads, compiler latency/RSS, checked
runtime and actual hosted ELF `.text`. Earlier campaigns and noisy observations
are preserved. The five common workloads have identical LowIR and native
instruction bytes. Compiler text adds 16,064 bytes (0.72%); template peak RSS
is 108,212 versus 107,712 KiB. The affected heap executable adds 122 bytes and
32 bytes of frame reservation; its hosted runtime medians are 194.16 / 195.51 ms,
within paired variation. No general speedup is claimed.

The independent growth controls retain each helper once and show proportional
work/output for 512→2048 source-handler functions. Small array/list expansion
remains capped at eight; larger counts use loops. Local completed-body proofs
have bounded per-owner work and conservative fallbacks. Function-local release
pools/caches are reset; complete context identities govern cleanup sharing.

Historical +15%, +16 MiB and 5.5× targets remain diagnostics, not extra exit gates.
That stage-scoped classification preserves every measurement and all mandated
limits, correctness and coverage. Required ownership cost is disclosed; it does
not license an unprofitable optional transform. Student native optimization,
allocator/debug work and self-hosting remain with later stages.

## Validation and ledger

Final [required checks and coverage](../student.tests/pa21/audit113-validation.json):
file audit **pass** (three inherited header-organization warnings),
`make test-pa21` **116/116**, and `make test-report-through-pa21`
**3712/3712**, **21/21 stages**, plus **22** separately reported property checks.
The only contract-path change from entry is the proved PA12 `.ref` revision.

New controls improve **13/46 → 46/46**; **4/4** access/demand controls and a
cross-object executable covering **14** allocation ABI signatures pass.
The [inherited rerun](../student.tests/pa21/validation113.json) records
**687/690** original lanes plus **2/2** hosted lifecycle controls. The three
original failures are retained supplied-backend limitations: one freestanding
RTTI case passes its hosted counterpart, and two lifecycle cases have identical
entry/final LowIR and pass hosted execution. No required failure is waived.

102–104: RTTI/casts, captures and list demand (71/116).
105: checkpoint through `f65eae8d`.
106–108: EH, full-expression and destination/construction ownership (108/116).
109: checkpoint through `f57bdd3b`, protected-jump repair.
110: `6b3aecdb`, `bdfdb31b`, `df6e8299` — handlers/statics/arrays, generated
construction and bounded helper sharing (113/116).
111: `3028366d` — completed scalar-body effect proof (114/116).
112: `e32e9f2f` through `f3af4630` — exhaustive dispatch and reference alignment
(116/116), frozen measurements and validation.
113: `30aec177` — independently reconstructed whole-stage audit and ownership,
query/ABI and optimization-legality repairs; final evidence and consolidation.
All 110–112 handoffs since the last checkpoint are included in this review.
