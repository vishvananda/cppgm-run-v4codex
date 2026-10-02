# PA34 inception

PA34 adds no language feature or mode. Preserve the shared typed pipeline,
PA1–PA33 contracts and exact canonical object/binary comparisons (spec §§1–10).
[Corrections and reducers](fixes.md); [ownership audit](audit.md).

## Status

No outstanding divergences or required checks. Frozen A/A + six ABBA blocks
cover compiler sources, templates, memory, floating and EH workloads. All
observations, paired latency/RSS, runtime/text and stage-scoped acceptance are
in [the evidence](../student.tests/pa34/evidence/performance.md) (spec §9).

## Completed canonical validation

- `perl scripts/cppgm_file_audit.pl --stage pa34 --paths dev/src`: pass.
- `make test-report-through-pa33`: 5454/5454 pass.
- PA34 self through PA5, PA8 and PA33: pass.
- Pptoken inception: exact match.
- `make inception CXX=g++ CPPGM_HOST_CXX=g++`: exact cppgm++ match.
- Native debug 11/11, self native driver 18/18, final O0–O3 source/template trace
  and 400-call reducer at the original 8 MiB stack: pass. Trace objects and work
  match across seed/self, including 24 generated-program executions.

## Acceptance

No new performance ratio gate. Preserve 900/3600-second compile limits, 8 GiB
command RSS and existing optimization work/growth budgets. Diagnostics isolate
miscompilation, reproducibility and stack/code-quality defects at their earlier
owners. No production resource limit was raised and no valid failing input was
rewritten. Final builds use canonical objects; probe links are diagnostic only.

The [binding](../student.tests/pa34/evidence/binding.json) records source,
canonical object/binary and evidence hashes. Full artifacts live under
`$RALPH_ARTIFACT_DIR/pa34-221`. Crashes, object/IR differences,
failed reducers, frozen binaries and every measurement remain archived. Earlier
IR was losslessly gzip-compressed to reclaim 840 MiB without losing evidence.
