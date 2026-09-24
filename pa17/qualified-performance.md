# PA17 qualified lookup and argument application evidence

Frozen implementation: `df97733f3f0d8c179b3b8dc4eefd8b9f193c3ea6`; entry: `4d1527e774b124ae47574a6c74006af4437cd0dd`. Compiler/source flags, CPU affinity, platform, SHA-256 binaries, all source text, untimed telemetry and raw observations are in [the campaign](../student.tests/pa17/qualified-performance.json). [The separate repeat](../student.tests/pa17/qualified-recheck.json) uses exactly the same binaries, flags and source hashes. Neither campaign is pooled, trimmed or discarded.

Compiler mode is `--emit-lowir -O0`; host build is `g++ -std=gnu++11 -Wall -O3` with the course runner. Each common workload has one warmup per binary, four A/A samples, and four ABBA blocks. New accepted workloads have one warmup and six final-only samples; entry rejection is recorded. Compile and executable timing are separate. Compiler workloads take tens of milliseconds to over a second; the tiny runtime-source compile timings remain in JSON and support no latency claim. Native loops read volatile bounds and check live results. PA24 native encoding and PA34 self-hosting remain later-stage obligations.

All nine common sources produce byte-identical LowIR; the three common runtime executables are byte-identical. Compiler text grows **1,024 bytes (0.0576%)**, from 1,776,454 to 1,777,478 bytes. This increment repairs semantic behavior and adds no optional optimization. No runtime speedup is claimed.

The initial A/A calibration for partials-1500 ranges from 120 ms to 4.85 seconds, while its compiler CPU use stays near 0.1 seconds. Its first two ABBA blocks are likewise disrupted. The repeat covers every common workload. The tables retain both campaigns; wall-time differences within that noise are not evidence of compiler improvement.

Initial compiler observations:

| Workload | A median [range] ms | B median [range] ms | Paired B/A [range] | Peak RSS A/B KiB | A/A ms |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 186.58 [114.45–5678.80] | 174.65 [112.90–283.14] | 0.4907 [0.0560–1.0004] | 22,384/22,516 | 120.08–4852.73 |
| common-loop-float-1500 | 218.92 [170.51–273.74] | 207.03 [156.01–289.42] | 0.9895 [0.7050–1.2480] | 30,188/30,220 | 175.98–263.57 |
| common-partials-6000 | 485.14 [456.11–625.86] | 468.19 [462.05–508.57] | 0.9059 [0.8406–1.0024] | 72,840/73,104 | 450.56–688.74 |
| common-loop-float-6000 | 611.85 [605.57–740.29] | 621.58 [596.56–1696.93] | 1.0368 [0.8994–1.9124] | 104,060/104,200 | 602.19–609.06 |
| member-qualified-1500 | 295.93 [294.46–318.39] | 295.43 [294.38–298.67] | 0.9913 [0.9591–1.0063] | 49,972/50,204 | 297.38–392.64 |
| member-qualified-6000 | 1277.90 [1227.11–1485.96] | 1255.07 [1224.08–1312.32] | 0.9726 [0.9289–0.9997] | 184,212/184,248 | 1220.38–1258.76 |

Repeated compiler observations:

| Workload | A median [range] ms | B median [range] ms | Paired B/A [range] | Peak RSS A/B KiB | A/A ms |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 110.63 [108.87–115.32] | 110.66 [109.14–120.25] | 1.0009 [0.9881–1.0519] | 22,388/22,644 | 107.87–109.94 |
| common-loop-float-1500 | 152.18 [148.84–157.52] | 152.88 [151.11–191.52] | 1.0025 [0.9897–1.1429] | 30,172/30,220 | 151.09–152.65 |
| common-partials-6000 | 458.08 [453.55–483.12] | 460.45 [455.50–462.76] | 1.0017 [0.9630–1.0123] | 72,848/73,060 | 450.52–453.70 |
| common-loop-float-6000 | 604.59 [599.43–634.64] | 612.30 [600.41–789.39] | 1.0014 [0.9895–1.1785] | 104,100/104,180 | 601.42–612.97 |
| member-qualified-1500 | 298.07 [293.78–315.06] | 295.99 [292.91–302.80] | 0.9896 [0.9829–1.0040] | 50,312/50,560 | 294.72–453.90 |
| member-qualified-6000 | 2007.78 [1244.84–3282.50] | 2048.42 [1229.08–3063.29] | 0.9850 [0.9492–0.9930] | 185,052/185,084 | 1235.17–1275.43 |

New accepted compiler workloads (entry rejects each):

| Workload | B median [range] ms | Peak RSS KiB | 4× input latency ratio |
|---|---:|---:|---:|
| inherited-qualified-1000 | 135.75 [131.66–171.81] | 25,904 | — |
| inline-qualified-1000 | 44.66 [43.26–125.94] | 11,836 | — |
| fixed-pack-head-1000 | 89.28 [85.26–342.06] | 20,352 | — |
| inherited-qualified-4000 | 557.46 [548.27–640.50] | 86,928 | 4.11× |
| inline-qualified-4000 | 159.06 [157.55–163.79] | 30,368 | 3.56× |
| fixed-pack-head-4000 | 374.98 [367.14–469.18] | 63,676 | 4.20× |

For 1,000→4,000 specializations, inherited lookup work grows 15,033→60,033, type substitutions 2,000→8,000 and access facts 1,000→4,000, while the retained source remains three regions/52 nodes. Inline-qualified lookup grows 19,016→76,016 and body transitions 1,000→4,000, with two regions/13 nodes. Fixed-head pack substitutions grow 12,005→48,005, lanes 3,000→12,000 and access facts 1,000→4,000, with three regions/33 nodes. These are proportional-demand observations, not bounds on arbitrary recursive template programs.

Executable observations (RSS is 256 KiB throughout):

| Run / executable | A median [range] ms | B median [range] ms | Paired B/A [range] | Text A/B bytes | A/A ms |
|---|---:|---:|---:|---:|---:|
| initial / runtime-calls | 370.72 [362.41–420.81] | 366.02 [359.98–418.47] | 0.9820 [0.9775–1.0091] | 206/206 | 360.88–372.0 |
| initial / runtime-memory | 214.07 [210.97–217.72] | 218.96 [213.23–246.12] | 1.0320 [0.9956–1.1002] | 434/434 | 210.63–225.2 |
| initial / runtime-floating | 250.09 [248.53–251.25] | 249.90 [248.29–254.65] | 1.0004 [0.9963–1.0088] | 230/230 | 248.49–260.26 |
| initial / qualified-runtime | entry rejects | 161.15 [159.54–164.00] | — | 258 | — |
| repeat / runtime-calls | 648.94 [569.99–754.56] | 624.16 [572.44–766.37] | 1.0012 [0.9050–1.0404] | 206/206 | 678.41–727.82 |
| repeat / runtime-memory | 362.79 [346.66–406.15] | 361.83 [354.09–380.67] | 0.9931 [0.9537–1.0353] | 434/434 | 353.12–418.33 |
| repeat / runtime-floating | 400.27 [377.26–426.47] | 421.76 [376.66–444.10] | 1.0446 [0.9915–1.0886] | 230/230 | 401.25–437.55 |

Text measures the executable payload after the ELF entry in the supplied sectionless backend; these runtime inputs have no static data. Exact executable equality establishes that timing variation on common runtime workloads is noise, rather than generated-code change. The new qualified runtime checks inherited type lookup, fixed-head pack application, an inline-qualified call and live arithmetic; it has no correct entry executable for A/B comparison.

The [source-to-LowIR-to-ELF trace](../student.tests/pa17/qualified-trace.json) records source/IR/ELF hashes, checked execution, phase/work counters, decoded instructions and ownership. Parser imports, canonical dependent type paths, immutable substitution/expansion frames and recipe/frame access facts feed recorded ordinary calls and direct typed LowIR. Temporary argument vectors release on return; existing TU arenas/flat indices own lasting facts. No grammar replay, global retry, rendered identity or reference-generated production output is introduced. The supplied native backend remains the explicit PA17 validation boundary.

Work and growth are bounded by the related name/scope graph and the source/emitted argument sequence; class templates repack only after source expansion. Existing constant-evaluation limits (1,000,000 work / 512 depth), initialization caps and conservative lowering fallbacks are unchanged. There is no new optimization pass or code-growth policy. PA17/O0 mandates no numerical latency/RSS ceiling. Historical +15%, +16 MiB and 5.5× targets are diagnostic under spec.md §9; all historical measurements and mandated correctness/coverage remain preserved. The common repeat shows no repeatable avoidable regression; new costs are required semantics, not an optimization claim. Independent review remains open.

Reproduce with `python3 student.tests/pa17/qualified_benchmark.py A B WORK OUT`, then `python3 student.tests/pa17/qualified_recheck.py OUT RECHECK_OUT RECHECK_WORK`, using fresh output paths. Verify the handoff with `python3 student.tests/pa17/verify_qualified.py`.
