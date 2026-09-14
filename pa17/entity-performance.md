# PA17 entity-group performance evidence

Final compiler source: `f6dbacdeb0e34afe459b8657c47d52f4b7fe91cd`. Entry: `21748547`.

Frozen binaries, flags, source text/hashes, telemetry, every A/A and ABBA
observation, RSS, result checks and executable hashes are in
[final measurements](../student.tests/pa17/entity-performance.json). The
[initial campaign](../student.tests/pa17/entity-performance-initial.json) and
[pre-coverage-demand campaign](../student.tests/pa17/entity-performance-pre-coverage-demand.json)
remain preserved; they are historical evidence, not final acceptance gates.

Each common workload has one warmup per binary, four A/A samples and four ABBA
blocks on one pinned CPU. Source workloads take about 0.11–0.60 seconds; native
loops take 0.21–0.36 seconds. Tiny runtime-source compilation is reported but
its approximately 6 ms duration is startup-sensitive. Runtime loop bounds are
volatile and the resulting arithmetic/memory values are checked. Compilation
and executable timing are separate. The supplied backend is only a validation
consumer of student-generated LowIR. No self-hosting/native-backend claim is made.

| Compiler workload | A / B median ms | A / B peak KiB | Median paired B/A |
|---|---:|---:|---:|
| common-partials-1500 | 109.53 / 109.81 | 22200 / 22208 | 1.0007 |
| common-loop-float-1500 | 151.58 / 152.19 | 30236 / 30208 | 1.0061 |
| entity-pack-alias-1500 | rejected / 140.47 | — / 26532 | not comparable |
| common-partials-6000 | 445.06 / 452.10 | 72396 / 72932 | 1.0195 |
| common-loop-float-6000 | 596.33 / 592.10 | 103564 / 103520 | 0.9856 |
| entity-pack-alias-6000 | rejected / 571.03 | — / 90384 | not comparable |
| runtime-calls | 6.13 / 6.15 | 5804 / 5696 | 0.9995 |
| runtime-memory | 5.92 / 5.93 | 5744 / 5776 | 0.9978 |
| runtime-floating | 5.81 / 5.93 | 5812 / 5768 | 1.0110 |

| Native workload | A / B median ms | Text bytes A = B | Median paired B/A |
|---|---:|---:|---:|
| runtime-calls | 359.86 / 359.57 | 206 | 0.9991 |
| runtime-memory | 210.53 / 211.10 | 434 | 1.0031 |
| runtime-floating | 249.61 / 249.47 | 230 | 0.9987 |

Compiler `.text`: 1,669,510 → 1,701,830 bytes
(+32,320, 1.94%). Common LowIR and executable bytes are identical.
Native text measures the sectionless executable payload after ELF entry; these
inputs contain no static data. Runtime peak RSS is 256 KiB in both binaries.

The 6,000-case common partial workload retains a roughly 2% compile-time cost
and 536 KiB peak-RSS increase for stricter deduction/retained argument facts.
The smaller common partial workload is approximately equal. No added optional
optimization is present. Inspection found avoidable coverage traversal for a
single viable candidate; final code demands it only for competing candidates.
Observed timing ranges overlap, so this change is not claimed as a speedup.
Loops/floating compilation remains comparable; native differences are noise
between identical bytes. All observations, including outliers, remain in JSON.

The newly supported 1,500/6,000 entity-pack-alias workloads compile in
140.47/571.03 ms (4.07x for 4x input), with peak RSS 26,532/90,384 KiB.
They use compile-time assertions and have no executable entry point. Entry
rejects both, so their necessary semantic work is not an A/B optimization result.

PA17/O0 has no mandated numerical latency, RSS, runtime or text ceiling.
Inherited +15%, +16 MiB and 5.5x targets are diagnostic observations under
spec.md stage-scoped acceptance. Existing evaluator work/depth limits remain
unchanged. Correctness, coverage and comparison rules were preserved; this
handoff accepts the bounded semantic cost and makes no runtime-profit claim.
