# PA15 final plan and handoff

Stage base commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`
Last reviewed commit: `dff6a92ba5af15cd21e60c82d98bf62aaf7a3d3e`

Target: **PA15 full-stage**. Loop 36 independently reviewed the accumulated
implementation and every handoff after checkpoint `538cfcb0`. Implementation and
final audit are complete: **177/177 PA15; 2112/2112 through PA15; 15/15 stages**.
The previous turn was progress: completed implementation and frozen evidence.
No PA15 implementation or unaudited handoff remains. PA16 has not been started.

## Final Spec Alignment

The production path retains immutable sources, streaming interned tokens and
one parsed source graph with attached canonical semantic facts. Templates retain
source regions and share fixed facts; dependent occurrences use compact
parent-linked substitution frames and complete pack boundaries. Indexed lookup,
selected declarations/conversions, per-fact demand states and typed ABI identities
feed direct typed LowIR. Text is the requested output adapter. Flat indexes,
slabs and vectors have TU/function owners; no external compiler implements output.

The audit traces explicit and demanded templates, partial-pattern selection,
constant activations, initialization, storage and vtable/body demand through their
actual source owners. [The audit](audit.md) records those traces, the limits and
the distinction between ordinary body validation and emission. Fixture-required
matching, variable constants and limited constant calls remain implemented even
where the broad README lists their general forms as later-stage features.

The final repair rejects volatile value reads in AST and query constant contexts,
prevents publishing a volatile reference's initializer as a reusable value, and
preserves volatile loads in namespace/local-static initialization. Address
formation, array decay, `sizeof`, unselected branches and ordinary runtime reads
remain valid. The new [reducers](../student.tests/pa15/final_audit.py) provide
17 native and 11 rejection controls; ten expose failures in frozen audit entry.

## Performance and acceptance

[Final performance](final-performance.md) records **24 frozen campaigns / 4774
invocations**, including all preliminary/noisy observations. New final campaigns
cover compiler latency/peak RSS, native runtime/text and owner work scaling.
Constant calls retain 512 active calls and 1,000,000 expression visits per root;
unavailable prerequisites do not become permanent activation failures. Array
backing remains limited to 32 bytes/object, with conservative direct stores/zero
loops beyond that. No new optimizer or growth search was added.

PA15/O0 has no mandated numerical latency/RSS/text ceiling. Historical diagnostic
budgets, including PA14's local 64-KiB compiler-text target, do not become an
accumulated PA15 exit gate. Measurements, language/complexity limits and coverage
remain intact. Native selection/allocation/ELF, higher optimization levels and
self-hosting keep their later-stage owners; supplied-backend controls establish
executable behavior at this stage's LowIR boundary.

## Validation and ledger

Fresh `make test-pa15`, `make test-report-through-pa15` and
`perl scripts/cppgm_file_audit.pl --stage pa15 --paths dev/src` pass. File audit has
three inherited header-ownership warnings. All nine personal suites pass,
including LowIR address controls and native volatile-load checks. The actual
root summary is 2112/2112; the incoming 2136 count is not substituted for it.
The final [evidence verifier](../student.tests/pa15/verify_final.py) checks frozen
hashes, sampling order, parity, counters, fixtures, current source and logs.

| Handoff | Final review disposition |
|---|---|
| Checkpoint 32, through `538cfcb0` | Independently retraced source/token/semantic/pack/lowering owners; original 11 failures subsequently resolved. Historical audit/evidence preserved in git and checkpoint JSON. |
| Loop 33, `57f0cd95` / `3c356866` | Matching, dependent aliases, selected environments and immutable pair-ordering cache reviewed; no remaining handoff. |
| Loop 34, `1aa393c9` / `3b2475f8` / `4f4bea42` | Initialization, static addresses, volatile stores, backing profitability and one proven reference correction reviewed; no remaining handoff. |
| Loop 35, `5452ad74` | Constant execution, body validation and static storage reviewed; volatile read defect repaired in `dff6a92b`. |
| Loop 36, `dff6a92b` | Full-stage architecture, correctness, self-containment, performance and required exits verified; final documentation/evidence consolidated. |

The only fixture change across PA15 is the documented
[aggregate-array reference correction](reference-corrections.md). Its source,
status, comparison rules and pinned bundle are preserved; the reducer was rerun
independently. No new reference correction was needed in this audit.
