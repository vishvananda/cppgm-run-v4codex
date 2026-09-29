# PA21/O0 performance evidence (loop 102)

[Raw observations](../student.tests/pa21/performance102.json) preserve frozen
binary/input/backend hashes, flags, CPU affinity, warmups, four A/A observations,
four ABBA blocks for equivalent implementations, six observations for new
semantics, RSS, checked results, output hashes and work counters. Reproduce with
[benchmark102.py](../student.tests/pa21/benchmark102.py). Baseline is `ac988ea3`;
implementation is `9f2181f9`. Host build: g++ with `-std=gnu++11 -Wall -O3` and
the course test-runner definitions. Compilation timing excludes stats/validation;
those run separately. `/usr/bin/time` supplies the primary peak-RSS measurement.

## Equivalent inherited paths

All five workloads produce **byte-identical LowIR and native executables**.
Timings below are medians; the paired column preserves ABBA block ratio ranges.
Startup results are diagnostics, not useful-work speedup claims.

| Workload | Compile A → B, ms | Peak compiler RSS A → B, KiB | Runtime A → B, ms | Runtime paired B/A range | Native payload bytes |
|---|---:|---:|---:|---:|---:|
| Startup | 10.527 → 9.694 | 5500 → 5732 | 5.786 → 5.416 | 0.884–1.019 | 24 |
| 9,600 auto specializations | 1305.474 → 1205.027 | 107828 → 108032 | 5.896 → 5.862 | 0.851–1.074 | 768062 |
| Live calls | 11.423 → 10.571 | 6028 → 6172 | 188.695 → 191.719 | 0.971–1.075 | 206 |
| Live memory loop | 10.982 → 10.269 | 5992 → 6168 | 113.817 → 121.809 | 0.968–1.272 | 434 |
| Live floating loop | 12.600 → 12.136 | 6016 → 6260 | 131.143 → 126.251 | 0.933–1.006 | 230 |

The template compile paired ratios are 0.918, 0.884, 0.929 and 1.021; its A/A
range is 1.158–1.537 seconds. Call/memory/floating runtime A/A ranges are
184.6–220.8 / 103.9–128.7 / 120.5–129.8 ms. Execution differences, including the
memory outlier, cannot be attributed to code changes when executables are
identical. No runtime improvement is claimed. Compiler text grows 18,624 bytes
(0.89%), from 2,090,694 to 2,109,318, for the new semantic/ABI implementation.
No repeatable avoidable compile regression was observed on these workloads.

## New behavior and bounds

The stage-entry compiler lacks correct RTTI semantics; its rejection or
accidental agreement is retained and excluded from A/B benefit claims.

| Workload | Compile median, ms | Peak RSS, KiB | Runtime median, ms | Native payload bytes | RTTI records / hits |
|---|---:|---:|---:|---:|---:|
| 800 distinct class specializations | 127.072 | 16792 | 5.115 | 89688 | 800 / 800 |
| 3,200 distinct class specializations | 481.989 | 49204 | 5.319 | 358488 | 3200 / 3200 |
| Pointer depth 64 | 15.351 | 6644 | 5.047 | 4696 | 65 / 1 |
| Pointer depth 256 | 44.662 | 11780 | 5.599 | 42904 | 257 / 1 |
| 4M dynamic typeid queries | 12.322 | 6180 | 47.056 | 600 | 2 / 2 |
| 4M dynamic casts | 11.326 | 6068 | 77.553 | 936 | 2 / 3 |

The 4× specialization increase gives 4× RTTI records/queries, 3.79× compiler
latency and 2.93× RSS. Pointer records grow linearly with depth; total emitted
names necessarily grow quadratically. Their LowIR grows from 37,032 to 392,871
bytes, and latency grows 2.91×, within produced-output work bounds. Dynamic
loops alternate success/failure and check 2M successful observations, preserving
live runtime work. Other rows' execution times are startup controls.

The supplied freestanding ELF has no sections: its measured executable payload
includes static RTTI data, and is explicitly a proxy rather than isolated text.
Compiler `.text` is measured separately. Raw evidence retains every observation;
no favorable-only sampling or previous evidence replacement occurred.

## Acceptance

Spec §9 applies at PA21/O0. There is no optional optimization or invented
numerical exit gate. The budgets are the mandated O(n) or O(n log n) consumed/
produced-IR work bounds: one RTTI record per type identity, cached dependency
properties, bounded per-query lowering and output-sized ABI names. Necessary
semantic costs are disclosed above. Historical self-selected benchmark ratios
remain diagnostics and do not override current-stage correctness or coverage.
Own native code generation/optimization and self-hosting remain at their assigned
later stages; no performance result for those unimplemented surfaces is claimed.
