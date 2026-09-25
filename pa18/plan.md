# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `f6eaf8ab213c90eefffd48f5c47b1ac9a75bce8d`.
Target: **PA18 full-stage**. Phase: **implementation 87 handoff complete; independent audit pending**.
Turn entry: `01b47c20b41f68d3142df6a8691fc905c20341a8`, **417/420**.
Validated implementation: `7000a4de`; course **420/420**, through **3029/3029**.
Previous turn: progress (committed audit repairs/evidence); no live job remained.

## Design and spec alignment

[Handoff 87](handoff87.md) records owners, data flow, complexity and scope.
Class-result passing stays a completed canonical class fact shared by definitions,
direct calls, conversion functions and indirect signatures. Source-dependent
aliases never become ABI keys. Three bundle result-boundary exceptions are
corrected by [proof 87](reference-correction87.md), with executable reducers and
an oracle transformation that reads only entry references. Isolated old direct
calls execute correctly; the demonstrated defect is incompatible access routes
to the same specialized function type. The proof preserves this distinction.

Class ellipsis uses selected value transfers and caller-owned materializations
across LowIR's scalar variadic boundary. Packed conversion facts carry the
representation and unavailable evaluated-transfer state. Unevaluated class
lvalue-to-rvalue conversions suppress copying; type queries retain the source
binding and defer defaults until hypothetical effects or constant/evaluated use.
Prvalues pass their existing result object without an extra copy/move.
Conditional and short-circuit cleanup consumes the same temporary identity.
Floating default zero now uses the canonical floating constant fact.

No parsing replay, textual semantic key, host delegation, global retry or cache
invalidation was introduced. Recipes and canonical class facts are TU-owned;
evaluated storage is contextual. Work follows candidates, actual arguments,
selected defaults and emitted transfers. Existing work/depth bounds remain.

## Validation and performance

Final stage **420/420**, earlier **2609/2609**, through **3029/3029**; file audit
passes with the same three header warnings. The **53** new controls improve
**30 → 53** on identical sources; accumulated personal checks, native reducers,
coverage and proof composition are recorded in [handoff 87](handoff87.md) and
[the evidence manifest](../student.tests/pa18/loop87-evidence.json). All **420**
course inputs and **1,686** fixture paths remain; only three proved oracles change.

[Performance 87](performance87.md) binds the final frozen code to **438** compiler/
native observations across **17** workloads; both preceding runs remain, **1,314**
observations total. Compiler text grows **7,424 bytes (0.374%)**; unaffected
executable output is identical. Required copy costs and complete spread remain.
[Audit 86 performance](performance86.md) and
historical measurements remain intact. Acceptance is **PA18/O0 LowIR**, spec §9:
no mandated numeric compiler latency/RSS ceiling. Historical +15%, +16 MiB and
5.5× targets remain diagnostics. Correctness, coverage, work bounds and optional
transform profitability remain requirements; no new optional optimization or
code-growth policy was introduced. Native payload is reported separately from
compiler `.text`; the sectionless backend output is not isolated native `.text`.

## Remaining implementation and independent review

No unfinished implementation is identified in the completed class-boundary group.
The implementation handoff is complete; this does not certify the full stage.
Native source try/catch, class `va_arg`/host varargs interoperation, native
optimization/debug and self-hosting remain their later stages' responsibilities.
Two exploratory PA21 inputs remain in personal controls as `LATER`, with their
failed observations preserved, and are not counted as PA18 passes.

**Independent review remains required:** whole-stage spec/architecture findings,
all accumulated oracle corrections, especially the canonical class-result proof,
and stage-scoped performance acceptance. These questions are not waived or
represented as unfinished implementation. [Audit 86](audit.md) and
[audit 82](audit82.md) remain unchanged; neither review marker above advances.

## Handoff ledger

Stage entry **266/420**. Earlier history remains in audit 82 and its links.

| Checkpoint | Range / disposition |
|---|---|
| 83–85 | `48c864ab` → `890f810b`; arrays, constructors, discard/storage; **411 → 414 → 417/420**, 23 proved oracle corrections; reviewed/repaired in 86. |
| 86 | `ecc308bc` → `f6eaf8ab`; accumulated audit and shared exception/constant-lifetime fixes; **417/420**, earlier **2609/2609**, file/coverage and **1429** controls pass. Preserved review baseline. |
| 87 | `01b47c20` → `7000a4de`; class ellipsis, prvalues, unevaluated/effect/default demand, branch lifetimes, typed floating zero; three proved result-oracle repairs. **417 → 420/420**, earlier **2609/2609**, file/coverage pass; final evidence linked above. |

Required commands: `make test-pa18`, `make test-report-through-pa17`,
`make test-report-through-pa18`,
`perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src`.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 before the independent full-stage audit is resolved.**
