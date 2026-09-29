# PA21/O0 performance evidence (loop 103)

[Final raw observations](../student.tests/pa21/performance103.json) retain frozen
binaries, flags, inputs, CPU affinity, backend hashes, warmups, four A/A samples,
four ABBA blocks for equivalent paths, six samples for new paths, every timing,
peak RSS, output hashes, checked results and separately collected work counters.
Reproduce with [benchmark103.py](../student.tests/pa21/benchmark103.py).
Baseline: `f4224e0b`; final implementation: `1c541f84`. Host compiler and flags
match the existing course build. Timing excludes stats and validation; RSS uses
`/usr/bin/time`. The [intermediate observations](../student.tests/pa21/performance103-first.json)
from `e835d6dc` are preserved, before the generated-copy cleanup extension.

## Equivalent inherited paths

All six workloads produce **byte-identical LowIR and native executables**.
Values below are medians. Executable size is the supplied sectionless ELF's
payload proxy, including static data; it is not an isolated `.text` measurement.

| Workload | Compile A → B, ms | Peak RSS A → B, KiB | Runtime A → B, ms | Payload bytes |
|---|---:|---:|---:|---:|
| Startup | 5.774 → 5.559 | 5708 → 5968 | 3.132 → 3.164 | 24 |
| 9,600 auto specializations | 678.723 → 684.344 | 106532 → 106520 | 3.379 → 3.447 | 768062 |
| Live calls | 6.309 → 6.093 | 6020 → 6156 | 126.502 → 124.545 | 206 |
| Live memory | 6.589 → 6.286 | 5972 → 5988 | 74.360 → 73.972 | 434 |
| Live floating point | 6.818 → 6.472 | 6040 → 6196 | 86.615 → 86.460 | 230 |
| Live reference captures | 6.287 → 6.023 | 6020 → 6064 | 25.148 → 25.546 | 188 |

Heavy-template paired compiler B/A ratios are 0.860, 0.994, 1.093 and 1.022;
A/A spans 670–709 ms. Live calls/memory/floating/reference runtime paired ranges
are 0.980–1.002 / 0.989–1.003 / 0.994–1.002 / 0.989–1.044. Startup timings and
short compilations are diagnostics, not benefit claims. The floating compilation
block with ratio 0.397 includes a slow A observation; it remains in the raw data.
Identical executables cannot support attributing runtime differences to this
change. No compile or runtime speedup is claimed and no repeatable avoidable
regression is established. Compiler `.text` grows by 24,832 bytes (1.18%), from
2,109,318 to 2,134,150, for the required semantic and cleanup implementation.

## New behavior

The baseline rejects value captures and is excluded from new-semantics A/B
claims. These are final-only scaling and live execution observations.

| Workload | Compile ms | Peak RSS KiB | Runtime ms | Payload bytes |
|---|---:|---:|---:|---:|
| 800 closure specializations | 115.713 | 24132 | 3.216 | 84862 |
| 3,200 closure specializations | 476.803 | 76672 | 3.436 | 339262 |
| 16 captured fields | 7.282 | 6180 | 3.706 | 411 |
| 64 captured fields | 8.724 | 6692 | 3.678 | 1901 |
| 256 captured fields | 26.978 | 7776 | 5.811 | 8237 |
| 32-element array capture | 13.672 | 6168 | 6.025 | 328 |
| 4,096-element array capture | 13.093 | 6116 | 6.776 | 16600 |
| 4M live value-capture calls | 12.153 | 6320 | 41.854 | 190 |

At 4× specialization count, capture edges and closures grow 800→3200, semantic
expression work 13,605→54,405, and lowering expression work 16,005→64,005.
Latency grows 4.12× and RSS 3.18×. At 16→64→256 fields, recorded capture edges
match field counts; expression work is 52→196→772 and LowIR bytes are
4,718→18,479→75,562. Array-capture LowIR remains 1,386→1,402 bytes at 128×
elements. Native payload growth there comes from the supplied backend's existing
large local-array initialization encoding; this compiler still emits a bounded
copy loop. New startup-sized rows show scheduling noise and establish no speedup.
The runtime loop reads a volatile trip count, varies the captured value, executes
4M calls and checks the accumulated 6M result; runtime work is live.

## Acceptance

Spec §9 applies at PA21/O0. The mandated budgets are O(n) or O(n log n) work in
consumed/produced facts and IR, plus the documented eight-element expansion cap.
There is no optional optimization or added numerical exit gate. Necessary capture
construction and exception cleanup costs are disclosed, not compared against a
rejecting baseline. All earlier measurements remain intact. Historical ratios in
inherited plans are diagnostic targets under stage-scoped acceptance; they do not
replace mandated bounds, correctness or coverage. Own native optimization and
self-hosting retain their later-stage owners.
