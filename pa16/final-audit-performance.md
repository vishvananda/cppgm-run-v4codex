# PA16 final architecture-audit performance

Final source: `95cdc3d8ec4117d69c53a32bceddcf7de6230abb`; entry: `241c6870`.
Frozen A is the validated entry compiler. Final B is the committed audit repair.
Both use `g++ -std=gnu++11 -Wall -O3`, TEST_RUNNER_ENABLE; measured frontend
flags are `--emit-lowir -O0`. Affinity is CPU 31 on Linux x86-64. Binary hashes,
inputs, outputs, backend hash and telemetry are in the raw records below. The
supplied PA8 native backend uses `-O0` and implements no frontend/lowering work.

## Protocol and preservation

- [Initial campaign](../student.tests/pa16/final-performance-initial.json): 476 observations, including warmups; binary `d813110d…` and all trial costs remain preserved.
- [Intermediate repair campaign](../student.tests/pa16/final-performance-pre-cast.json): 476 observations on binary `65997d7c…`, plus [66 DAG observations](../student.tests/pa16/final-dag-performance-pre-cast.json).
- [Final campaign](../student.tests/pa16/final-performance.json): 476 observations on binary `faa771f2…` after the final short-circuit legality repair.
- [Final 32-TU DAG campaign](../student.tests/pa16/final-dag-performance.json): 66 observations on those same final A/B binaries.
- Total: **1,560 observations**, plus all prior stage measurements unchanged. [Main producer](../student.tests/pa16/final_benchmark.py) and [DAG producer](../student.tests/pa16/final_dag_benchmark.py) are retained.

All tables below use the final binary. The short-circuit follow-up adds 64 compiler
text bytes and three controls; it changes no fixed benchmark LowIR. The earlier
campaigns are preserved as separate observations, not pooled into final medians.

Each common phase has one warmup per binary, four A/A observations, then four
ABBA blocks. New behavior rejected by A has one B warmup and six B observations;
it is not a speedup comparison. Compilation and execution are timed separately.
Inputs/flags are fixed before timing; preflight validates LowIR and output
equivalence. Stats/validation are absent from timed invocations. Every execution
checks its result. No trial is removed, including scheduler/context-switch
outliers. Tables use the eight samples per binary from ABBA; A/A samples are
calibration only. RSS is the median of per-invocation peak RSS, in KiB. Ranges
are observed minimum–maximum, not confidence intervals. Pairs are B/A means
within each ABBA block. Sub-10-ms frontend cases are startup diagnostics only.

## Compiler latency, memory and text

| Fixed workload | A / B median ms | A / B peak RSS KiB | Four B/A pairs |
| --- | --- | --- | --- |
| template-1000 | 22.77 / 22.85 | 7396 / 7486 | 1.0118, 1.0014, 0.9976, 0.9955 |
| template-4000 | 73.53 / 74.43 | 13024 / 12906 | 1.0030, 1.0184, 1.0059, 0.9997 |
| memory-float-1000 | 102.81 / 102.38 | 21774 / 21648 | 0.9936, 1.0107, 0.9836, 0.9992 |
| memory-float-4000 | 405.44 / 406.67 | 70410 / 70218 | 1.0095, 1.0016, 1.3094, 0.9870 |
| array-sharing-1000 | 98.92 / 100.58 | 20328 / 21490 | 0.9896, 1.0175, 1.0187, 1.0145 |
| array-sharing-4000 | 392.45 / 388.18 | 64590 / 64532 | 0.9997, 0.9826, 1.2768, 0.9798 |
| final-results-1000 | 29.35 / 29.18 | 9312 / 9234 | 0.9888, 0.9903, 1.0146, 0.9898 |
| final-results-4000 | 102.05 / 102.06 | 20652 / 20666 | 0.9878, 1.0203, 1.0007, 0.9996 |
| shared-object-dag-14 | 8.31 / 7.70 | 5816 / 5956 | 0.9304, 0.9324, 0.9418, 0.9378 |
| shared-object-dag-18 | 16.14 / 8.26 | 6172 / 6216 | 0.4941, 0.5130, 0.5191, 0.5108 |
| shared-object-dag-22 | 121.51 / 8.89 | 6180 / 6188 | 0.0754, 0.0714, 0.0733, 0.0724 |

| Fixed workload | A range ms | B range ms | A/A range ms |
| --- | --- | --- | --- |
| template-1000 | 22.66–22.99 | 22.60–23.07 | 22.76–23.22 |
| memory-float-1000 | 101.35–104.51 | 101.68–104.30 | 101.99–105.49 |
| array-sharing-1000 | 98.25–105.33 | 99.80–101.47 | 98.54–104.44 |
| final-results-1000 | 29.04–29.80 | 28.95–29.60 | 28.79–29.86 |
| template-4000 | 73.04–75.12 | 73.28–75.80 | 73.47–74.23 |
| memory-float-4000 | 400.96–408.33 | 399.67–657.17 | 400.88–410.74 |
| array-sharing-4000 | 387.20–396.72 | 385.11–612.95 | 385.86–517.28 |
| final-results-4000 | 100.94–104.32 | 100.71–105.02 | 101.16–101.89 |
| shared-object-dag-14 | 7.99–8.44 | 7.58–8.02 | 8.01–8.17 |
| shared-object-dag-18 | 15.97–16.43 | 7.94–8.42 | 15.81–16.95 |
| shared-object-dag-22 | 120.13–125.68 | 8.49–9.38 | 120.75–125.94 |

Compiler `.text`: **1,652,870 → 1,669,510 bytes** (+1.01%).
New constant-query/cast/reference correctness paths and traversal bookkeeping
account for code cost; no optional executable transform is added. The 4,000-case
template median costs about 1.2%; final-result medians are effectively equal.
We disclose small costs rather than claim universal improvement. The 1,000-array
RSS increase is 1,162 KiB while the 4,000-array case is comparable; geometric
allocation/RSS granularity makes this unsuitable for a per-node claim.

The first campaign had 385.70→400.86 ms for 4,000 array cases, with three
non-outlier pairs near +2–4%. This exposed avoidable eager constexpr-array value
construction. Literal/value-init plans now prove legality without constructing
duplicate values. The intermediate repair measured 386.08→385.75 ms; final A/B
measure 392.45→388.18 ms and 64,590→64,532 KiB. Every earlier cost and
outlier remains recorded. Final array measurements include a slow B block, also
visible in the ranges/pairs rather than discarded. No broad array speedup is claimed.

New bit-field/template-query behavior is rejected by A. B measurements:

| Cases | B median ms | B range ms | Peak RSS KiB |
| --- | --- | --- | --- |
| 1000 | 49.64 | 49.26–51.43 | 12040 |
| 4000 | 188.70 | 187.13–220.19 | 31756 |

Median growth is 3.80x for 4x input, with 2.64x peak RSS. This is a diagnostic
of new required semantic work, not an A/B profitability claim or an invented
scaling gate. The intermediate campaign's large scheduler outliers remain in
its own record and are not replacement samples for this final measurement.

## Startup-amplified shared-object traversal

A DAG value with two identical children at each level used to revisit shared
objects during dependency collection. Final traversal deduplicates object IDs
as well as storage IDs. Depths 14/18/22 visit 16/20/24 dependencies in each TU;
the key still includes all reachable storage versions/liveness. Both compilers
produce identical LowIR. Per-query work follows distinct values and edges.

The single-TU B samples are near startup. The fixed follow-up repeats the same
source in 32 TUs in one invocation (removing the repeated main definition). Each
TU is parsed/evaluated independently and released; linkage/LowIR remain program
owned. The B interval is now 39–59 ms, compared with roughly 6 ms process startup
in the small runtime-source controls. All 32 TU telemetry records are retained.

| Depth × 32 TUs | A / B median ms | A / B peak RSS KiB | Four B/A pairs |
| --- | --- | --- | --- |
| shared-object-dag-14-32tu | 54.50 / 39.05 | 6158 / 6154 | 0.7169, 0.7179, 0.7211, 0.7119 |
| shared-object-dag-18-32tu | 291.29 / 48.15 | 6066 / 6184 | 0.1641, 0.1673, 0.1741, 0.1656 |
| shared-object-dag-22-32tu | 3660.74 / 56.04 | 6376 / 6338 | 0.0153, 0.0155, 0.0152, 0.0155 |

| Depth × 32 TUs | A range ms | B range ms | A/A range ms |
| --- | --- | --- | --- |
| shared-object-dag-14-32tu | 53.78–57.18 | 38.66–40.62 | 54.14–56.61 |
| shared-object-dag-18-32tu | 287.76–294.09 | 47.51–50.99 | 288.02–293.89 |
| shared-object-dag-22-32tu | 3641.75–3736.38 | 55.71–58.66 | 3696.62–3785.45 |

The depth-22 median improves 3.66 s→56.04 ms (about 65x) in all four pairs,
with essentially unchanged peak RSS. This is a compiler dependency-traversal
benefit on the affected workload, not a general compiler or runtime speedup.

## Executable runtime and size

The calls/memory/floating workloads use the inherited fixed PA10 sources at
factor 12. Result destruction runs 24 million iterations; automatic array copying
runs 8 million. Volatile bounds and checked final values prevent dead work.
All five A/B LowIR outputs and generated executables are **byte-identical**.
No runtime improvement or code-size optimization is attributed to this audit.

| Runtime workload | A / B median ms | A / B range ms | Four B/A pairs | Code+alignment / data / file bytes (both) |
| --- | --- | --- | --- | --- |
| runtime-calls | 360.14 / 360.80 | 358.85–380.42 / 359.08–378.79 | 1.0414, 1.0035, 0.9873, 0.9729 | 206 / 0 / 326 |
| runtime-memory | 211.13 / 211.80 | 209.87–212.54 / 210.02–213.73 | 0.9982, 1.0025, 1.0059, 1.0050 | 434 / 0 / 554 |
| runtime-floating | 249.26 / 249.86 | 248.67–257.92 / 249.13–251.59 | 0.9854, 0.9949, 1.0045, 1.0009 | 230 / 0 / 350 |
| runtime-results | 176.88 / 177.30 | 176.10–194.30 / 176.41–181.79 | 1.0179, 1.0065, 0.9536, 1.0000 | 944 / 8 / 1072 |
| runtime-copy | 220.17 / 226.25 | 212.72–227.05 / 215.97–304.79 | 1.0548, 1.0494, 1.1876, 1.0011 | 276 / 160 / 556 |

The supplied backend emits sectionless ELF. Reported code+alignment is payload
after entry minus typed global-data bytes; it is not falsely labeled a `.text`
section. File bytes include ELF headers. Native peak RSS observations are 256 KiB
in this measurement environment; they are retained as observed rather than used
as a claim about all mapped memory. Runtime A/A ranges and tiny-source compile
latency/RSS remain in the raw records. The runtime-copy distribution includes
a slow B block and faster A samples; equality of executable bytes prevents
attributing those timing differences to frontend changes.

## Stage-scoped acceptance and inherited evidence

PA16/O0 mandates correct constant semantics and constant scalar-array copying,
with bounded frontend/lowering work. It specifies no percentage/time/RSS/text
ceiling. The historical +15% latency, +16 MiB RSS, 5.5x scaling and earlier PA14/15
text targets are diagnostics
under spec.md, not additional exit criteria. The first array regression was
resolved, necessary correctness costs are explicit, and no unprofitable optional
optimizer remains. The 512-call/1,000,000-step limits and eight-element expansion
cap are preserved; neither course coverage nor comparison rules changed.

[Scalar/floating](performance.md), [storage](storage-performance.md),
[validity](validity-performance.md), [checkpoint audit](audit-performance.md),
[objects](object-performance.md), [initialization](initialization-performance.md)
and [result/member](result-performance.md) retain all earlier measurements.
The audit validates those historical snapshots rather than requiring old binaries
or old plan markers to describe the current compiler. Their runtime evidence for
prior output changes is kept separate from this audit’s identical-output checks.
Own native optimization, MIR/allocation/debug and self-hosting belong to later
PAs; those ownership constraints create no extra PA16 gate.
