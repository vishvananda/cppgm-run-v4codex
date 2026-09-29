# PA18 compact completion plan

Stage base: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Entry: `f5c942ff`; reviewed implementation: `fe4a11f0` (also `34b49cab`).
Target: **PA18 full-stage**. Phase: **final independent audit complete**.
Spec Alignment: **aligned for PA18/O0**.
PA19 has not been started. The previous goal turn was progress; no inherited
live job remained. [Audit 90](audit.md) reconstructs the actual source pipeline;
[audit 89](audit89.md) preserves the entry record unchanged.

## Findings and completed changes

- Nonfinal function packs now contribute only explicit, non-deduced argument
  lanes. Calls, queries, constructors/operators and function-type targets share
  the rule. Nonfinal template argument expansions suppress deduction for the
  whole list; completed types still undergo conversion/target matching.
- Completed specializations map default slots to concrete parameter positions,
  retaining source identity and lazy fact/demand ownership. Defaults before
  packs are accepted; defaults on packs are rejected.
- The 49 standard-derived controls improve **13 → 49** on identical inputs.
  One representative declaration/template trace checks shared body identity,
  one default evaluation, class copy/destruction and canonical indirect results.
- The validation harness builds its ABI API check from current sources, removing
  its dependency on an old scratch executable. The implementation source is
  registered in `dev/frontend_source_sets.mk`.
- The review covers the whole streaming/canonical pipeline, fixed/dependent fact
  sharing, complete cache keys, local invalidation, monotonic demand, typed
  lowering, bounded O0 transformations and allocation/release boundaries.
  Handoffs since checkpoint 86 and audit 89 were reviewed in combined source form.
- No new oracle correction is needed. All 29 historical corrections retain
  documented reducers, cited standard/contract proof and bundle provenance.
  All 420 PA18 sources / 1,686 fixture paths and comparison rules remain.

## Validation and performance

Final reviewed code passes **71 explicit checks**, including **1,605 personal
cases in 37 result-bearing suites**, ten execution traces, ABI/API checks,
scaling/representation inspections and oracle reconstruction. Required commands:

- `make test-pa18`: **420/420**, exit 0.
- `make test-report-through-pa18`: **3029/3029**, **18/18 stages**, exit 0;
  PA10–PA12 also report **22 focused properties**. Earlier stages: **2609/2609**.
- `perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src`: exit 0,
  three inherited header advisories, no errors.

[Frozen performance verification](performance90.md) covers **99 workloads**: all 88 distinct
inputs from audits 82/89 plus eleven nonfinal-pack scaling/runtime cases.
Compiler latency/RSS and checked native runtime/size are reported together,
with A/A calibration, ABBA and spread: **2,563 full-run observations, 308 repeat
observations and 330 preserved interrupted-run observations**. The 89 comparable
inputs retain exact LowIR/native output; ten previously rejected inputs report
final-only costs and proportional work. No consistent compiler slowdown is
established by the targeted repeats; no precise speedup is claimed. Compiler
text grows 7,296 bytes (0.366%). Inherited optional-transform profitability and
all historical measurements remain. Stage-scoped acceptance passes.

Acceptance is spec §9's **PA18/O0** policy. Historical +15%, +16 MiB and 5.5×
targets remain diagnostics, not added exit gates. Correctness, coverage,
mandated limits and work/growth/profitability requirements are unchanged.
Native optimization/debug, hosted varargs and self-hosting remain later stages.

## Exit and ledger

[The evidence manifest](../student.tests/pa18/loop90-evidence.json) binds the
current sources and binaries to checks, controls, traces, coverage and measurements.
`34b49cab` owns the function-pack/default repair and portable validation;
`fe4a11f0` owns the whole-list non-deduced rule. The first repair's passing checks
and interrupted timing remain recorded; all required checks were repeated on
final code. The final documentation/evidence commit consolidates this review.
A clean `git status --short` is verified after committing. There is no remaining
PA18 implementation, unaudited handoff or audit obligation before advancement.
