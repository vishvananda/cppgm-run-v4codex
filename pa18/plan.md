# PA18 compact completion plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `f2558ff473410f95de7af49469f404ebf8362e34`.
Target: **PA18 full-stage**. Phase: **final independent audit complete**.
Spec Alignment: **aligned for PA18/O0**. PA19 has not been started.
Previous turn: progress (committed the pending pack-prefix repair); no inherited
live job remained. [Final audit](audit.md) and
[evidence manifest](../student.tests/pa18/loop89-evidence.json) record the proof.

## Findings and completed work

- Completed class-result identity, class ellipsis transfer/default/effect demand,
  prvalue storage, branch lifetimes and canonical floating zero were independently
  reviewed across ordinary, retained-template, query, constant and lowering paths.
- Explicit pack prefixes now extend through call, target and nested deduction;
  completed deductions still agree or discard the candidate. The 51 controls
  improve **38 → 51** on identical sources. Scalar explicit arguments no longer
  allocate unnecessary prefix frames; no-argument calls retain packs directly.
- The actual streaming frontend, canonical typed graph, immutable frames,
  separate demand states, localized dependency edges, selected conversion facts,
  typed lowering and owner release boundaries satisfy the relevant spec sections.
  Nine source-to-native traces and inherited work/representation checks pass.
- All **29** accumulated reference corrections have documented reducers and
  cited language/contract proofs. The canonical class-result correction is
  accepted with its direct-call versus indirect-call distinction. No new oracle
  correction, changed input, weakened comparison or removed fixture was needed.

## Validation and performance

Final PA18 **420/420**; earlier stages **2609/2609**; root through report
**3029/3029**, all **18 stages**; **22** separately reported focused properties.
File audit passes with three inherited header advisories. The final explicit
validation contains **65 checks**, including **1,556 personal cases**, nine
execution traces, ABI/scaling/inspection controls and oracle reconstruction.
All **420** PA18 sources and **1,686** fixture paths remain. The audit explains
why the earlier all-tracked-input count of 3053 exceeds the course report.

[Performance 89](performance89.md) freezes entry/final binaries over the union
of audit-86 and handoff-87 corpora plus pack-prefix scaling/runtime inputs:
**54 workloads**, compiler latency/RSS and executable runtime/payload together,
A/A noise calibration, ABBA blocks, checked outputs and all observations retained.
The unnecessary scalar-prefix frames are removed; new required pack deduction
has proportional work. Existing bounded O0 summaries/omissions retain their
profitability evidence and exact output. No new optional transform is introduced.

Acceptance is **PA18/O0**, spec §9. No mandated numeric compiler latency/RSS
ceiling applies. Historical +15%, +16 MiB and 5.5× targets remain diagnostics;
correctness, coverage, work bounds and optional-transform profitability remain
requirements. Native optimization/debug, hosted aggregate-varargs retrieval,
source try/catch and self-hosting remain later-stage ownership.

## Exit and history

Required `make test-pa18`, `make test-report-through-pa18` and
`perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src` pass on reviewed
code. Root reports run sequentially because they share counts. Intended changes
and final evidence are committed; `git status --short` is checked empty at close.
There is no remaining PA18 implementation or audit obligation.

| Checkpoint | Disposition |
|---|---|
| 82 | [Preserved audit](audit82.md): signatures, conversions and result summaries. |
| 83–86 | Arrays, constructors, discard/storage and shared effect/lifetime repairs; [audit 86](audit86.md) preserved verbatim, **417/420** at that checkpoint. |
| 87 | [Handoff](handoff87.md): class boundaries and three proved result-oracle repairs, **420/420**. |
| 88–89 | `56e17b79`, `d2980f04`, `f2558ff4`: extensible pack deduction and demand-only frame creation; whole-stage independent audit, full validation and final performance evidence complete. |
