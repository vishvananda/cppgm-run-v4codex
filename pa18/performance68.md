# PA18 loop 68 performance evidence

Entry `047215cf` versus final implementation `bd1d7b4c`, with frozen compiler
SHA256s `d64395b1809482d2c49643bfd28a6555993d3b73e0a72c151bcb1b034010dc54` and
`4a7204679e553cf8702b4dbfd32f306a0dd6077355b12bb19a56de3c6b674a32`.
[Harness](../student.tests/pa18/benchmark68.py), [all final observations](../student.tests/pa18/loop68-performance.json),
and [pre-filter observations](../student.tests/pa18/loop68-performance-before-filter.json).
The pre-filter run is preserved but superseded for acceptance; the final run
includes filtering base primary identity before allocating candidate bindings.

PA18/O0 flags: `--emit-lowir -O0`; host `g++ -std=gnu++11 -Wall -O3` with
`TEST_RUNNER_ENABLE`. CPU affinity 31. One warmup each; four A/A calibration
samples; four ABBA blocks for equivalent correct inputs; six final-only samples
where entry rejects required behavior. Telemetry/full validation run only in
preflights. Source, compiler, LowIR and executable hashes, wall time, peak RSS
and all observations are retained. Supplied native backend bundle:
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.

## Compiler latency and peak RSS

| Workload | Entry ms | Final ms | Paired B/A median (range) | Peak RSS A / B KiB |
|---|---:|---:|---:|---:|
| ordering-600 | 56.52 | 56.42 | 0.993 (0.923–1.029) | 15100 / 15184 |
| ordering-2400 | 214.03 | 213.72 | 0.994 (0.981–1.004) | 42672 / 42756 |
| member-head-600 | 98.06 | 96.89 | 0.995 (0.985–1.338) | 22796 / 22904 |
| member-head-2400 | 411.03 | 414.76 | 1.018 (0.981–1.106) | 73620 / 73744 |
| common-loop-float-1500 | 154.60 | 153.05 | 0.995 (0.957–1.005) | 30224 / 30136 |
| partial-head-600 | — | 45.82 | required new behavior | — / 13096 |
| base-fallback-600 | — | 80.94 | required new behavior | — / 19840 |
| partial-head-2400 | — | 173.00 | required new behavior | — / 35468 |
| base-fallback-2400 | — | 331.21 | required new behavior | — / 61580 |

Final startup median: **6.06 ms**. Scaled inputs dominate startup.
Tiny runtime-source compilation samples are retained in JSON as diagnostics.
Common paired medians range from 0.993 to 1.018. No compiler speedup is claimed.
The member-head-2400 blocks range from 0.981 to 1.106; its raw medians are
411.03/414.76 ms. The earlier pre-filter paired median was 1.004. These
observations, including every outlier, do not establish a repeatable 1.8%
regression. The implementation adds only required deduction/conversion work;
no optional transform or repeated global search is retained.

A/A compiler calibration ranges (ms):

- ordering-600: 56.30–56.83.
- ordering-2400: 215.26–222.47.
- member-head-2400: 392.14–400.68.

## Checked executable runtime and size

| Workload | Entry seconds | Final seconds | Paired B/A median (range) | Payload bytes A / B |
|---|---:|---:|---:|---:|
| runtime-calls | 0.7303 | 0.7205 | 0.988 (0.982–0.993) | 206 / 206 |
| runtime-memory | 0.4194 | 0.4183 | 0.997 (0.996–1.001) | 434 / 434 |
| runtime-floating | 0.4999 | 0.4972 | 0.994 (0.987–1.006) | 230 / 230 |
| runtime-new-partial | — | 0.1144 | required new behavior | — / 173 |

Common LowIR and executables are byte-identical A/B: zero generated-code growth.
Timing differences measure noise, not optimizer benefit. Volatile runtime loop
bounds and checked results prevent a dead/constant-folded workload. The new
partial-head workload performs array-reference calls with runtime data.
The backend emits sectionless ELF: payload after entry is a **text/alignment
proxy**, not a `.text` section measurement. Runtime sources have no static data.
Scaled frontend-only inputs are not executed; the separate runtime controls
and 56 semantic controls establish behavior.

## Scaling, telemetry and acceptance

- ordering, 600→2400: 3.79× time, 2.82× peak RSS.
- member-head, 600→2400: 4.28× time, 3.22× peak RSS.
- partial-head, 600→2400: 3.78× time, 2.71× peak RSS.
- base-fallback, 600→2400: 4.09× time, 3.10× peak RSS.

Selected final telemetry (600→2400):

- partial-head, `semantic_type_substitution_work`: 2408→9608.
- partial-head, `semantic_substitution_frames`: 602→2402.
- partial-head, `semantic_type_query_work`: 1202→4802.
- partial-head, `semantic_candidate_work`: 600→2400.
- base-fallback, `semantic_type_substitution_work`: 3609→14409.
- base-fallback, `semantic_substitution_frames`: 2402→9602.
- base-fallback, `semantic_type_query_work`: 604→2404.
- base-fallback, `semantic_candidate_work`: 1800→7200.

Compiler `.text`: **1,845,126 → 1,846,790 bytes**, +1,664 (0.090%).

PA18 mandates O0 LowIR and no numerical compiler/RSS/runtime/text ceiling.
Inherited PA17 **+15%, +16 MiB, 5.5×** targets remain diagnostics under spec §9;
all historical observations remain. Correctness, required coverage, graph-work
bounds and inherited bounded array expansion remain gates. Final-only costs
are necessary semantics, not a speed comparison to the rejecting implementation.
Base search follows explicit visited edges; only matching primary identities
allocate trial bindings. Immutable partial frames and substituted queries use
existing complete typed cache keys. Scratch dies with candidate resolution and
canonical facts with the TU. No optimizer, own native backend, code growth
policy or optional transform was added. Later native optimization and
self-hosting remain PA24–PA34 obligations. Current-stage performance acceptance
passes; independent whole-stage audit remains required.
