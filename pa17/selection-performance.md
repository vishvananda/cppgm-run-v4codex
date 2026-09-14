# PA17 selection and argument performance — loop 53

Frozen entry `65b02f17` → code `0a720134`. The [complete campaign](../student.tests/pa17/selection-performance.json) preserves every source, binary/backend hash, warmup, A/A sample, ABBA block, wall/CPU time, peak RSS and untimed telemetry. Build: g++ `-std=gnu++11 -Wall -O3`, test runner enabled. Student compilation: `--emit-lowir -O0`; CPU 31 affinity, serial samples, no concurrent build/test campaign. One warmup per binary, four A/A samples and four ABBA blocks; final-only workloads have one warmup and six samples. Untimed `--stats --validate-lowir` preflights precede timing. No observations were discarded.

Supplied native backend: O0, bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA-256 `c3bae4acf3243d5a2fd6a15ef00e2d75d11a7d82715b1e4771f55165d0542490`. PA17 produces typed LowIR; the explicit adapter/backend invocation is validation. Own native emission and self-hosting remain later-stage requirements.

| Compiler | SHA-256 | .text bytes |
|---|---|---:|
| Entry | `e54376e7412f26025d58faf1f03127adcb6cd8e41f0e45c2b2e938a76bb41e94` | 1,769,670 |
| Final | `16b3783f192303ee0badbe50da7c5552bb091a3d3daec634767169c050e36ffd` | 1,772,678 |

Compiler text grows 3,008 bytes (0.170%). These bytes are separate from generated-program text. The change implements required selection, substitution, rejection and LowIR behavior; it adds no optional optimization pass.

Compiler times below are milliseconds; RSS is KiB. Paired B/A is the median of the four within-block mean ratios, with their full range. It is not the ratio of separate medians.

| Workload | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Peak RSS A/B KiB |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 111.97 [110.16–116.31] | 112.20 [110.18–118.53] | 1.0001 [0.9869–1.0430] | 109.58–122.17 | 22,424/22,532 |
| common-loop-float-1500 | 155.03 [153.13–159.46] | 154.96 [152.73–164.09] | 1.0017 [0.9932–1.0168] | 151.54–153.99 | 30,172/30,300 |
| common-partials-6000 | 455.84 [446.23–490.50] | 456.83 [450.21–459.86] | 1.0022 [0.9619–1.0133] | 451.18–469.49 | 72,832/73,040 |
| common-loop-float-6000 | 602.25 [597.25–620.85] | 606.34 [600.24–702.37] | 1.0154 [1.0009–1.0799] | 598.36–612.61 | 104,096/104,208 |
| member-qualified-1500 | 295.68 [288.99–333.24] | 293.44 [288.20–297.00] | 0.9813 [0.9265–1.0027] | 292.46–295.32 | 50,260/50,192 |
| member-qualified-6000 | 1234.76 [1227.55–1269.57] | 1242.91 [1216.00–1290.38] | 1.0009 [0.9962–1.0251] | 1213.32–1248.82 | 185,868/185,792 |

Every common LowIR output is byte-identical. The largest paired compiler increase is 1.54% on loop/float-6000, with separate medians +0.68% and an included B outlier of 702.37 ms. The paired member-qualified-1500 improvement is not claimed as a compiler speedup: A includes a 333.24 ms sample, and the larger case is essentially neutral. RSS changes are between −76 and +208 KiB. Exact matching uses a local scratch cache when there is no pack/enclosing frame, preserving the inexpensive ordinary path. No repeatable material compiler-work regression is established by these common workloads.

| Newly supported workload | Final median [range] ms | Peak RSS KiB | 4× input latency growth |
|---|---:|---:|---:|
| variable-cv-pack-1500 | 70.30 [69.21–70.87] | 17,056 | — |
| partial-pack-query-1500 | 358.85 [351.61–380.24] | 64,904 | — |
| member-partial-owner-1500 | 282.04 [277.10–284.69] | 57,004 | — |
| variable-cv-pack-6000 | 269.39 [265.29–272.10] | 51,300 | 3.83× |
| partial-pack-query-6000 | 1537.96 [1513.88–1601.08] | 240,792 | 4.29× |
| member-partial-owner-6000 | 1211.71 [1191.14–1219.01] | 213,256 | 4.30× |

Entry rejects these six inputs, so they are semantic-cost/scaling evidence, not A/B speedups. Variable initializers and candidate visits grow from 3,000 to 12,000; source parameter checking remains 57 visits. Partial-query candidates grow from 3,002 to 12,002, queries from 9,013 to 36,013, while retained source regions/nodes remain 7/231 and source checks remain 100. Member partials apply exactly 1,500/6,000 out-of-class definitions through 1,500/6,000 edges, with 61 source checks and retained regions/nodes 5/143 at both sizes. Substitution frames and projected occurrences grow with demanded facts. No broader function bodies are demanded by these static assertions. These are scoped measurements, not bounds on arbitrary template recursion.

| Executable | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Text A/B bytes |
|---|---:|---:|---:|---:|---:|
| runtime-calls | 360.50 [358.59–365.57] | 359.61 [358.33–361.50] | 0.9969 [0.9958–0.9971] | 358.05–364.95 | 206/206 |
| runtime-memory | 210.83 [209.82–212.83] | 211.22 [209.83–212.97] | 1.0026 [0.9936–1.0057] | 211.04–218.45 | 434/434 |
| runtime-floating | 250.29 [248.93–265.94] | 249.80 [248.71–252.10] | 0.9945 [0.9663–0.9980] | 249.76–250.93 | 230/230 |
| constant-boolean-runtime | 193.54 [192.82–194.50] | 196.36 [194.60–217.00] | 1.0266 [1.0127–1.0593] | 193.73–194.88 | 228/216 |

All executions check a live result using volatile loop bounds. Native RSS is 256 KiB throughout. Text measures sectionless executable payload after the ELF entry; these runtime inputs contain no static data. Calls/memory/floating outputs are byte-identical. Their compiler samples (about 6 ms) remain in JSON but are startup-sensitive and support no latency claim.

The constant-boolean case changes required O0 lowering from a branch on a recorded boolean constant to a jump, matching the two repaired course comparisons. It saves 12 native bytes but is **2.66% slower** by paired runtime (all four block ratios exceed 1). This is not an optimization profit claim. The retained disassembly shows the supplied backend removes `mov/test/jne/jmp`, moves main from payload offset 0x58 to 0x4c, and preserves its loop operations, one call per iteration, 72-byte main frame and 16-byte callee frame. Layout changes are observable; the measurements do not prove their microarchitectural cause. The required LowIR form cannot be reverted while preserving the course comparison. Backend layout/encoding policy belongs to PA24+, and PA17 cannot tune the supplied backend. Both outputs execute the same checked result. Compiler medians for this source are 6.432/6.461 ms, peak RSS 5,800/5,856 KiB; their startup cost prevents a finer inference.

Ownership and budgets: primary/entity/typed-argument keys own selection and alias facts for the TU. Exact matching is proportional to the candidate pattern and expanded arguments; only the primary’s indexed candidates participate. Choosing and verifying a winner takes O(C) pair comparisons, cached by immutable entity pairs (at most O(P²) distinct ordering pairs across a primary’s P patterns). Coverage sorting is O(n log n) only for competing template-template shapes. Parent-linked frames retain outer and selected head identities without copying visible environments. Expected missing alias/type members return an empty probe result at the candidate owner. Canonical queries memoize concrete values and preserve symbolic pack counts. Nondependent source facts remain shared; local scratch maps die at the match boundary and retained tables die with the analyzer. The [trace](../student.tests/pa17/selection-trace.json) records renamed heads → member demand → `sizeof(char)`/`sizeof(int)` constants → typed LowIR → checked ELF.

Boolean condition lowering consumes one recorded semantic constant in O(1), with no expression scan, speculative evaluation, iteration, invalidation or code growth. Nonconstant/volatile/effectful forms retain ordinary lowering. Existing 1,000,000-work/512-depth constant-evaluation limits, eight-element array-expansion cap and counted-loop fallbacks remain unchanged. ABI, cleanup, floating-point and debug obligations are unchanged.

PA17/O0 has no mandated numerical performance ceiling. The inherited +15%, +16 MiB and 5.5× values remain diagnostic targets under spec.md §9; historical measurements are preserved. Required correctness costs and the supplied-backend runtime regression are disclosed rather than waived or called improvements. The correct common outputs, bounded owner/data flow and measured scaling support stage-scoped acceptance. All 28 remaining course failures remain required implementation. Earlier [checkpoint evidence](checkpoint52-performance.md), including cumulative measurements, is unchanged.

Reproduce: `python3 student.tests/pa17/selection_benchmark.py A B WORK OUT` with fresh work/output paths. Verify the handoff with `python3 student.tests/pa17/verify_selection.py`.
