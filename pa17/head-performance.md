# PA17 retained-head performance — loop 49

Entry A is `df0904deaa9a21544ab4c10f3a9e68120740d982`; final B is
`807e0989cc2e83f7dfa8804e9bb4db471ea35af9`. Both binaries, flags, complete
inputs, output hashes, telemetry and raw observations are frozen in
[the final record](../student.tests/pa17/head-performance.json). Reproduce with
`student.tests/pa17/head_benchmark.py A B WORK OUT`.

The harness pins CPU 31 on `Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43`. Compilers use
`--emit-lowir -O0`; host build is `g++ -std=gnu++11 -Wall -O3` with
`TEST_RUNNER_ENABLE`. Each common workload has one warmup per binary, four A/A
samples and four ABBA blocks. New-only workloads have one warmup and six final
samples; the entry rejection is recorded, so no speedup is inferred. Native
code uses the pinned course backend at O0. Compiler and execution measurements
are separate. Every executable returns its checked expected result. Runtime
loops use volatile bounds and dependent arithmetic/memory so startup and dead
work do not dominate. Self-hosting and our own native backend belong to later PAs.

| Common compiler workload | A / B median ms | A / B peak KiB | Paired B/A median (range) | A/A range ms |
|---|---:|---:|---:|---:|
| common-partials-1500 | 110.30 / 110.57 | 22344 / 22864 | 1.0058 (0.9948–1.0093) | 110.44–113.00 |
| common-loop-float-1500 | 153.30 / 153.61 | 30304 / 30164 | 0.9980 (0.9812–1.2021) | 150.78–155.65 |
| common-partials-6000 | 451.17 / 447.01 | 72936 / 73092 | 0.9832 (0.9439–1.0029) | 449.18–454.74 |
| common-loop-float-6000 | 596.44 / 595.25 | 104080 / 104172 | 0.9972 (0.8005–1.0060) | 597.49–600.25 |
| member-qualified-1500 | 286.14 / 289.82 | 49360 / 50380 | 1.0114 (0.9028–1.0240) | 281.78–286.55 |
| member-qualified-6000 | 1205.71 / 1192.32 | 182164 / 185040 | 0.9931 (0.9748–0.9994) | 1205.73–1226.05 |

All common LowIR and executable hashes match exactly. The common compiler
paired medians range from −1.68% to +1.14%; their observed block spreads are
above, and individual outliers remain in the JSON. These are cost observations
for required semantics, not optimization benefits. Peak RSS differences across
common frontend workloads are reported without imposing a new acceptance gate.

| Newly supported compiler workload | Final median ms (range) | Peak KiB |
|---|---:|---:|
| three-head-demand-1500 | 454.53 (450.91–459.85) | 78652 |
| three-head-demand-6000 | 1924.76 (1911.30–1952.15) | 298084 |
| wide-member-head-128 | 713.79 (707.89–721.60) | 124592 |
| wide-member-head-512 | 2911.15 (2889.59–2921.69) | 483284 |
| member-runtime | 6.29 (6.23–6.42) | 5864 |

| Native workload | A / B median ms | Text bytes A / B | Paired B/A median (range) | A/A range ms |
|---|---:|---:|---:|---:|
| runtime-calls | 362.61 / 362.28 | 206 / 206 | 1.0034 (0.9919–1.0127) | 362.13–606.47 |
| runtime-memory | 211.76 / 211.52 | 434 / 434 | 0.9867 (0.9761–1.0065) | 210.13–213.96 |
| runtime-floating | 249.57 / 249.26 | 230 / 230 | 0.9993 (0.9945–1.0007) | 249.18–250.07 |
| member-runtime | rejected / 362.34 | — / 206 | final only | n/a |

Native text is the sectionless executable payload after ELF entry; these
runtime sources have no static data. Peak native RSS is 256 KiB in every
observation. Runtime sources have compiler medians of about 5–7 ms: those compiler timings
are startup-sensitive diagnostics, retained in full but not used for frontend
performance claims. Their native medians are about 211–363 ms.

| Scaling input | Frames | Substitution work | Definition applications |
|---|---:|---:|---:|
| three-head-demand-1500 | 18000 | 24010 | 1500 |
| three-head-demand-6000 | 72000 | 96010 | 6000 |
| wide-member-head-128 | 2400 | 42262 | 300 |
| wide-member-head-512 | 2400 | 158230 | 300 |

The 4× input increase in `three-head-demand`
costs 4.23× compiler time and 3.79× peak RSS.

The 4× input increase in `wide-member-head`
costs 4.08× compiler time and 3.88× peak RSS.

Frame counts follow actual specialization/definition demands and remain fixed
when only head width increases. Width-dependent argument/type work is retained
once per relevant source/substitution identity. Parser receiver queries cache
each completed subtree, avoiding repeated chain walks. These observations
support the implemented ownership bounds; they are not proof of all input shapes.

Compiler `.text` grows from 1,720,006 to 1,740,550 bytes
(+20,544, 1.19%) for the required entity/default/pack semantics.
No optional optimization or native code-growth transform was added. PA17/O0
has no mandated numeric performance ceiling. Existing evaluator limits and
lowering growth policies remain unchanged. Inherited +15%, +16 MiB and 5.5×
targets are diagnostics under spec.md; they cannot override stage-scoped
acceptance. Required correctness and all course failures remain binding.

The [pre-pack campaign](../student.tests/pa17/head-performance-before-pack.json)
is preserved in full. Its later measurements overlapped a compiler rebuild and
controls, so it is diagnostic only and superseded by the final frozen run.
[Earlier cumulative/checkpoint observations](audit-performance.md) are preserved.
No measurement, comparison rule or mandated limit was discarded.
