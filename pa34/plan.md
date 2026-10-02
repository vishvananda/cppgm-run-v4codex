# PA34 inception

PA34 adds no language feature or mode. Preserve the shared typed pipeline,
PA1–PA33 contracts and exact canonical object/binary comparisons (spec §§1–10).
[Corrections and reducers](fixes.md); [ownership audit](audit.md).

## Architecture Review

Audit 222 independently checks the complete PA34 delta from `99ea7d9d`,
including shared source ownership, source-to-ELF and demanded-template paths,
optimization legality/invalidation/budgets, every reduced failure and reference
proof, canonical generation dependencies, and frozen performance observations.
The review checks existing evidence against its frozen source identities and
reproduces required checks. The full findings, commit/reducer review and
optimization proof are in [audit 222](audit.md).

## Final Architecture Review

The final architecture is immutable sources/interned IDs → streaming cursors →
cooperating parser/semantic graph → typed LowIR and bounded optimization →
function-owned MIR/selection/allocation → direct ELF. Retained source topology,
canonical type/pattern/argument/frame keys, separate demand states and reverse
query dependencies avoid grammar replay, semantic reconstruction and global
retry. Frontend slabs/pools die after lowering; bounded interprocedural LowIR
work owns its bodies; per-function MIR/scratch dies after encoding. No text
roundtrip, alternate PA34 compiler mode or external code generation exists.

The trace follows Packet<T>/demand<T>, recorded lifetime/ABI facts and the
typed strlen identity through legality admission to consumed MIR and encoding.
At O0–O3, source/replay objects and generation work counters agree; telemetry
on/off does not alter objects. Undemanded invalid members remain deferred.

Optimization policy and budgets remain those of PA32/PA33, recorded in the
audit table: dirty scalar/dataflow worklists; two finite object splits; immutable
optional inline admission (4096/site, 32768/caller, 32*(I+O+P+S+F+1)/unit,
depth 64, growth 1536 or 2048/caller); forced preparation 262144/caller and
4194304/unit; one loop reservoir min(4096,2*(I+1)), at most 256 clones/function;
near-linear placement and ≤14 ABI register moves; strlen prefixes ≤8/function,
128/unit and 64 reserved bytes/site. Exhausted budgets or missing proofs retain
valid conservative code. There is no expanding pipeline fixed point.

Audit corrections wire all PA33 driver checks to the self compiler, place the
mixed-auto reducer at its PA20 owner, and verify every timed common object
after stopping the clock. The reference proof now explicitly includes C++11
conditional-inclusion rules. Production sources and compiler images are
unchanged from the validated handoff. Every stage commit/reducer is accounted
for; the audit leaves no unresolved architecture or layer-divergence finding.

## Status

No outstanding divergences or required checks. Frozen A/A + six ABBA blocks
cover compiler sources, templates, memory, floating and EH workloads. All
observations, paired latency/RSS, runtime/text and stage-scoped acceptance are
in [the historical evidence](../student.tests/pa34/evidence/performance.md)
and [independent evidence 222](../student.tests/pa34/evidence222/performance.md)
(spec §9). All 532 historical observations recompute; 476 fresh observations
retain compiler latency/RSS and checked generated runtime/text together.

Fresh seed/self compiler-source medians are 2.1718/4.1474 seconds, peak RSS
77,852/87,896 KiB, and paired latency 1.981 [1.582–2.205]. Component text is
34,466 bytes in both. Common compiler paired medians are 2.365–2.604; generated
executables are identical across generations, so their runtime spread is noise.
Per-workload runtime, RSS, text and noise ranges remain in evidence 222.

## Completed canonical validation

- `perl scripts/cppgm_file_audit.pl --stage pa34 --paths dev/src`: pass.
- `make test-report-through-pa33`: 5454/5454 pass.
- PA34 self through PA5, PA8 and PA33: pass; final self PA1–PA33 census 5454/5454.
- Pptoken inception: exact match.
- `make inception CXX=g++ CPPGM_HOST_CXX=g++`: exact cppgm++ match.
- Native debug 11/11 in seed/self, self native driver 18/18, final O0–O3 source/template trace
  and 400-call reducer at the original 8 MiB stack: pass. Trace objects and work
  match across seed/self, including 24 generated-program executions.

## Acceptance

No new performance ratio gate. Preserve 900/3600-second compile limits, 8 GiB
command RSS and existing optimization work/growth budgets. Diagnostics isolate
miscompilation, reproducibility and stack/code-quality defects at their earlier
owners. No production resource limit was raised and no valid failing input was
rewritten. Final builds use canonical objects; probe links are diagnostic only.

The [historical binding](../student.tests/pa34/evidence/binding.json) and
[audit binding](../student.tests/pa34/evidence222/binding.json) record source,
canonical object/binary and evidence hashes. Full artifacts live under
`$RALPH_ARTIFACT_DIR/pa34-221` and `pa34-222`. Crashes, object/IR differences,
failed reducers, frozen binaries and every measurement remain archived. Earlier
IR was losslessly gzip-compressed to reclaim 840 MiB without losing evidence.
