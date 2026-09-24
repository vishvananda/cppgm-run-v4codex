# PA18/O0 performance evidence — handoff 65

Frozen entry `52d97223` and final tree `4eaf273d` (implementation `644dd88b`).
[Raw observations](../student.tests/pa18/loop65-performance.json) retain binary,
backend and harness hashes, build/compiler flags, CPU affinity, generated sources,
untimed telemetry, compiler wall time/peak RSS, and checked native hashes/sizes.
[The harness](../student.tests/pa18/benchmark65.py) uses one warmup per binary,
four A/A samples and four ABBA blocks. Newly accepted programs have six final-only
samples because the entry compiler rejects them. Validation, telemetry and native
construction occur outside timing. No competing build/test ran during timing.

| Compiler workload | Entry ms | Final ms | Paired B/A median (range) | Peak RSS KiB A / B |
|---|---:|---:|---:|---:|
| ordering-600 | 56.07 | 55.31 | 0.977 (0.964–0.988) | 15120 / 15180 |
| conversion-600 | — | 64.26 | new behavior | — / 16552 |
| conversion-defaults-600 | — | 75.54 | new behavior | — / 18408 |
| member-head-600 | — | 97.38 | new behavior | — / 22728 |
| ordering-2400 | 217.53 | 217.99 | 1.000 (0.912–1.016) | 43280 / 43196 |
| conversion-2400 | — | 254.12 | new behavior | — / 48920 |
| conversion-defaults-2400 | — | 298.57 | new behavior | — / 56400 |
| member-head-2400 | — | 393.31 | new behavior | — / 73096 |
| common-loop-float-1500 | 155.88 | 154.76 | 0.988 (0.983–1.006) | 30208 / 30156 |
| runtime-calls | 6.17 | 5.90 | 0.958 (0.951–0.969) | 5836 / 5888 |
| runtime-memory | 6.74 | 6.57 | 0.970 (0.886–0.984) | 5844 / 5796 |
| runtime-floating | 6.14 | 6.29 | 0.992 (0.972–1.004) | 5916 / 5884 |
| runtime-new-conversion | — | 6.29 | new behavior | — / 5724 |

Final startup median is 5.66 ms; the scaled workloads dominate startup. Tiny
runtime-source compilation timings are diagnostic. Common workload paired
medians show no material compiler regression in this run; no speedup is claimed.
A/A ranges are 55.44–56.60 ms (ordering-600), 212.75–213.81 ms (ordering-2400),
and 154.22–160.41 ms (loop/float). All observations are retained, including the
258.9 ms ordering-2400 entry sample, the 97.85 ms defaults-600 final sample and
the 293.40 ms conversion-2400 final sample. Required new semantics have no valid
entry output against which to claim a compilation or runtime improvement.

| Executable workload | Entry seconds | Final seconds | Paired B/A median (range) | Payload bytes A / B |
|---|---:|---:|---:|---:|
| runtime-calls | 0.7170 | 0.7185 | 1.000 (0.998–1.017) | 206 / 206 |
| runtime-memory | 0.4199 | 0.4179 | 0.998 (0.994–1.003) | 434 / 434 |
| runtime-floating | 0.4942 | 0.4949 | 1.003 (0.999–1.004) | 230 / 230 |
| runtime-new-conversion | — | 0.2345 | new behavior | — / 181 |

Common LowIR and executables are byte-identical; runtime variation is measurement
spread. Runtime A/A ranges are 0.7154–0.7192 s (calls), 0.4162–0.4309 s (memory),
and 0.4943–0.5024 s (floating). The new conversion loop takes 0.2323–0.2358 s.
Volatile bounds and checked results preserve observable calls, memory, floating
point work and conversion execution. The supplied backend writes sectionless
ELF: executable payload after entry is the available text/alignment proxy for
these programs with no static data. Compiler/runtime timing is kept separate.
All other generated executables also run with checked results; their sizes and
hashes are retained, including 52842→211242 bytes for conversion-600→2400 and
55242→220842 bytes for member-head-600→2400. Loop/float is a compile-only corpus.

600→2400 final scaling is 3.94× time / 2.85× RSS for common ordering, 3.95× /
2.96× for conversion, 3.95× / 3.06× for conversion defaults, and 4.04× / 3.22×
for member heads. Type-substitution work grows 1803→7203 (conversion),
4209→16809 (defaults), and 5402→21602 (heads). Head query work grows 1204→4804;
prefix/substitution frames grow 4200→16800. Declaration parameter checks stay
at 21 with 600→2400 retained-check reuses. These observations support linear
work in specialization count for fixed candidate/head/type shapes; they do not
establish a constant bound for arbitrarily nested heads or lexical environments.

Work/storage bounds: conversion lookup follows indexed candidates and inherited
class paths, then required return-type deduction, immediate substitution and
ordering edges. Candidate-local bindings release on return; canonical entities,
types, ordering keys and immutable frames belong to the TU. Each retained head
prefix adds one binding without copying earlier prefixes. Binding lookup follows
the required lexical frame chain; substitutions reuse typed per-frame facts.
No mutable environment, global invalidation, rendered semantic key or unselected
body instantiation is introduced. Constant addresses consume selected conversion
facts; lowering emits the declared conversion type through typed ABI nodes.

Compiler `.text` grows 1,831,302→1,836,422 bytes (+5,120, 0.28%). Common generated
code growth is zero. PA18/O0 mandates no numerical latency/RSS/runtime ceiling.
This is required semantic work with explicit graph bounds, not an optional
optimization; no runtime benefit is claimed to justify extra optimization work.
Prior self-selected thresholds remain diagnostics under spec §9. Preserve
[earlier evidence](performance.md) and [loop 64 evidence](performance64.md);
no mandated limit, correctness requirement or coverage is waived. Native
optimization and self-host performance remain owned by PA24–PA34.
