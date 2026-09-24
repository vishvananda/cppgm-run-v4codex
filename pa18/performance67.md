# PA18 loop 67 performance evidence

Entry `06211ad0` versus final implementation `0b60ca52`, frozen compiler SHA256s
`575cb73b91e50e425345d38e0a7984729c95e04f957943a591bc2a38922603f9` and
`d64395b1809482d2c49643bfd28a6555993d3b73e0a72c151bcb1b034010dc54`.
[Harness](../student.tests/pa18/benchmark67.py) and [all final observations](../student.tests/pa18/loop67-performance.json).
The [pre-ABI](../student.tests/pa18/loop67-performance-before-abi.json) and
[pre-cache/access](../student.tests/pa18/loop67-performance-before-cache.json)
runs are retained, but superseded for acceptance because their implementation
was not the final group. No observations are dropped or credited as a speedup.

PA18/O0 flags: `--emit-lowir -O0`; host build `g++ -std=gnu++11 -Wall -O3`
with test runner enabled. CPU affinity 31. One warmup per binary; four A/A
calibration samples; four ABBA blocks for equivalent correct inputs; six
final-only samples where entry rejects required behavior. Every source, binary,
output hash, wall/RSS observation and telemetry preflight is retained in JSON.
Compiler timings omit telemetry/full validation; preflights explicitly enable
both. The supplied backend bundle is `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.

## Compiler latency and peak RSS

| Workload | Entry ms | Final ms | Paired B/A median (range) | Peak RSS A / B KiB |
|---|---:|---:|---:|---:|
| ordering-600 | 54.15 | 54.84 | 1.007 (0.881–1.030) | 15152 / 15180 |
| ordering-2400 | 211.48 | 210.62 | 1.003 (0.908–1.013) | 43380 / 42772 |
| member-head-600 | 94.61 | 95.35 | 1.005 (1.000–1.020) | 22608 / 22528 |
| member-head-2400 | 384.54 | 387.58 | 1.009 (0.957–1.012) | 73220 / 73632 |
| common-loop-float-1500 | 153.51 | 154.34 | 1.031 (0.967–1.277) | 30248 / 30316 |
| address-600 | — | 90.03 | required new behavior | — / 21112 |
| function-address-600 | — | 62.48 | required new behavior | — / 16852 |
| address-2400 | — | 377.84 | required new behavior | — / 68404 |
| function-address-2400 | — | 241.84 | required new behavior | — / 49244 |

Final startup median: 5.25 ms. The scaled inputs dominate startup.
Tiny runtime-source compiler samples remain in JSON and are diagnostic.
Common paired medians range from 1.003 to 1.031. No compiler speedup is claimed.
The loop/float paired blocks are 1.277, 1.073, 0.967 and 0.989; its raw
medians are 153.51/154.34 ms. This spread does not establish a repeatable
3.1% regression. All outliers are retained. Other common paired medians
increase by at most 0.9%, with necessary semantic/cache bookkeeping and small
code-layout differences. No unprofitable optional transformation is retained.

Representative A/A compiler calibration ranges (ms):

- ordering-600: 53.79–54.29.
- ordering-2400: 206.95–208.90.
- member-head-2400: 381.99–387.73.

## Checked executable runtime and size

| Workload | Entry seconds | Final seconds | Paired B/A median (range) | Payload bytes A / B |
|---|---:|---:|---:|---:|
| runtime-calls | 0.7317 | 0.7321 | 0.988 (0.986–1.026) | 206 / 206 |
| runtime-memory | 0.4163 | 0.4163 | 1.001 (0.999–1.007) | 434 / 434 |
| runtime-floating | 0.4932 | 0.4939 | 1.001 (0.997–1.002) | 230 / 230 |
| runtime-new-address | — | 0.2328 | required new behavior | — / 195 |

Common LowIR and executable hashes are identical A/B; code growth is zero.
Their timing spread measures noise, not an executable performance change.
Volatile loop bounds, runtime computation and checked results prevent a dead
or constant-folded workload. The new function-address workload checks the
required indirect call through a template argument. The backend emits
sectionless ELF: the sizes above are the executable payload after its entry
point, a **text/alignment proxy**, not a claimed `.text` section. Runtime
benchmark sources have no static data. Scaled frontend address programs have
global data, included in their separately recorded executable payloads.

## Scaling, ownership and stage acceptance

- ordering, 600→2400: 3.84× time, 2.82× peak RSS.
- member-head, 600→2400: 4.06× time, 3.27× peak RSS.
- address, 600→2400: 4.20× time, 3.24× peak RSS.
- function-address, 600→2400: 3.87× time, 2.92× peak RSS.

Address telemetry, 600→2400:
- Constant storage identities: 600→2400.
- Class completions: 1800→7200.
- Immutable substitution frames: 4200→16800.
- Typed queries: 1803→7203.

[Repeated-use controls](../student.tests/pa18/address_cache.py) separately check
32, 128 and 512 occurrences: each inspects exactly three candidates total and
executes correctly. Completed address conversion keys contain query identity,
target type, access override and explicit-instantiation access mode. Success
with incomplete prerequisites is not cached. Existing query dependency edges
retain those prerequisites; negative conversion results are not cached here.
This is shared semantic work required by the architecture, not an optional
optimizer or a claimed timing benefit. Linkage classification caches completed
entity/type/scope facts before symbol merging. Work follows required query,
candidate, argument, owner and demand edges; all caches release with the TU and
conversion/candidate scratch releases on return.

Compiler `.text`: **1,837,766 → 1,845,126 bytes**,
+7,360 (0.400%).

PA18 mandates O0 LowIR and no numerical latency/RSS/runtime ceiling. Inherited
PA17 **+15%, +16 MiB, 5.5×** targets remain diagnostics, as classified under
spec §9 in audit 66. All historical measurements remain. Correctness, coverage,
required graph-work bounds and inherited bounded array expansion remain gates.
Final-only costs are necessary semantics, not a comparison to an incorrect
implementation. No material avoidable common regression is established here;
common generated code is unchanged. No optimizer, own native backend, inlining
or growth policy was added. Native allocation/text/runtime optimization and
self-hosting remain PA24–PA34 obligations. Current-stage performance acceptance
passes; independent whole-stage audit remains required.
