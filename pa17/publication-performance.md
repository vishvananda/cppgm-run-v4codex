# PA17 publication and construction performance — loop 54

Frozen entry `3f1f6f0b` → code `5c5756b7`. The [complete campaign](../student.tests/pa17/publication-performance.json) retains every input, binary/backend hash, warmup, A/A observation, ABBA block, wall/CPU time, RSS and untimed work counter. Build: g++ `-std=gnu++11 -Wall -O3`, test runner enabled. Student invocation: `--emit-lowir -O0`. CPU 31 affinity, serial samples; no concurrent build/test campaign. One warmup per binary, four A/A samples and four ABBA blocks; final-only workloads use one warmup and six samples. `--stats --validate-lowir` preflights are outside timing. No observations were discarded.

The first campaign had large bidirectional timing excursions, including common-loop-float-6000 A at 1,834 ms and member-qualified-1500 B at 1,174 ms. A [separate recheck](../student.tests/pa17/publication-recheck.json) preserves repeated observations on the identical binaries and inputs; it does not replace the initial observations. No common compiler or byte-identical executable speedup is claimed.

Supplied native backend: O0, bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA-256 `c3bae4acf3243d5a2fd6a15ef00e2d75d11a7d82715b1e4771f55165d0542490`. The backend is used only to validate source-generated LowIR. Own native emission and self-hosting remain later-stage requirements.

| Compiler | SHA-256 | .text bytes |
|---|---|---:|
| Entry | `16b3783f192303ee0badbe50da7c5552bb091a3d3daec634767169c050e36ffd` | 1,772,678 |
| Final | `181bc044df898d92b829a599282f9e55da4f3359e68a901552c23de73936e189` | 1,776,454 |

Compiler text grows 3,776 bytes (0.213%), independently of generated-program text. This is required declaration, specialization, materialization and LowIR behavior; no optional optimization pass is added. Times below are milliseconds; RSS is KiB. Paired B/A is the median of within-block mean ratios, with their complete range.

| Initial common compiler workload | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Peak RSS A/B KiB |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 114.56 [111.59–188.82] | 122.35 [111.55–186.73] | 1.0153 [0.9597–1.0597] | 183.36–185.95 | 22,536/22,596 |
| common-loop-float-1500 | 274.19 [152.88–287.21] | 276.51 [153.27–282.38] | 1.0088 [0.9976–1.0654] | 276.22–281.12 | 30,280/30,212 |
| common-partials-6000 | 455.61 [449.03–471.41] | 457.42 [449.12–496.01] | 1.0157 [0.9835–1.0418] | 456.22–497.99 | 73,064/73,104 |
| common-loop-float-6000 | 985.20 [617.57–1834.18] | 723.39 [596.36–1136.35] | 0.7933 [0.7591–0.9716] | 596.51–600.61 | 104,188/104,204 |
| member-qualified-1500 | 519.97 [418.88–570.90] | 544.42 [299.03–1174.03] | 1.1298 [0.8324–1.5876] | 500.97–630.44 | 50,228/50,360 |
| member-qualified-6000 | 1230.21 [1199.13–1254.72] | 1237.83 [1218.48–1294.94] | 1.0139 [1.0023–1.0466] | 1238.45–1656.83 | 184,904/185,180 |

All common LowIR outputs are byte-identical. The noisy initial ratios range −20.67% to +12.98%; the repeated cases below range −3.86% to +4.28%. RSS changes in the initial six cases range −68 to +276 KiB. The repeated large member workload has a +4.28% paired cost (−0.91% in separate medians; the paired statistic captures a different ordering of the noisy samples); this is disclosed, not called a speedup. The implementation already copies nondependent explicit facts and shares canonical condition queries; there is no optional speculative work to remove. New declaration/access checks and fact storage remain bounded and below the inherited diagnostic targets. Timing alone does not identify a further avoidable regression.

| Repeated common compiler workload | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Peak RSS A/B KiB |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 215.95 [209.49–243.65] | 217.89 [208.38–235.99] | 0.9801 [0.9399–1.0217] | 212.38–246.15 | 22,524/22,640 |
| common-loop-float-1500 | 354.57 [336.21–382.26] | 349.37 [324.59–366.46] | 0.9614 [0.9384–1.0358] | 338.91–366.86 | 30,280/30,224 |
| common-loop-float-6000 | 1251.69 [1213.94–1297.91] | 1281.58 [1212.42–1370.80] | 1.0238 [1.0119–1.0504] | 1220.40–1263.42 | 104,148/104,204 |
| member-qualified-1500 | 585.55 [549.56–650.53] | 578.53 [552.06–617.75] | 0.9959 [0.9147–1.0118] | 587.93–624.51 | 50,620/50,376 |
| member-qualified-6000 | 3240.26 [1409.64–3462.93] | 3210.81 [2844.14–3479.66] | 1.0428 [1.0024–1.4010] | 2365.51–2748.80 | 184,824/185,084 |

| Newly supported compiler workload | Final median [range] ms | Peak RSS KiB | 4× input latency growth |
|---|---:|---:|---:|
| anonymous-members-1000 | 175.54 [173.51–195.28] | 33,904 | — |
| conditional-constructors-1000 | 146.24 [144.47–147.95] | 29,412 | — |
| array-values-1000 | 126.34 [124.94–126.57] | 27,336 | — |
| anonymous-members-4000 | 719.63 [713.57–730.03] | 118,824 | 4.10× |
| conditional-constructors-4000 | 613.52 [602.28–712.28] | 100,688 | 4.20× |
| array-values-4000 | 531.83 [529.68–534.29] | 91,192 | 4.21× |

Entry rejects these six inputs. They establish semantic cost and scaling, not A/B speedups. For 1,000→4,000 specializations, anonymous member publication grows 6,000→24,000 and lookup work 22,019→88,019; retained template regions/nodes remain 2/50. Conditional constructor query work is 3,003→12,003 and body transitions 1,000→4,000, with retained regions/nodes 4/52. Array expression pack traversal remains six source visits; concrete lanes grow 3,000→12,000, retained regions/nodes remain 3/37 and bodies are demanded once per specialization. No all-program retry or grammar replay is added. These measurements do not bound arbitrary template recursion.

| Executable | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Text A/B bytes |
|---|---:|---:|---:|---:|---:|
| runtime-calls | 375.02 [363.47–410.83] | 362.96 [359.83–389.46] | 0.9561 [0.9232–0.9871] | 360.91–368.03 | 206/206 |
| runtime-memory | 210.56 [209.74–211.58] | 210.22 [209.85–211.63] | 1.0001 [0.9993–1.0005] | 210.44–212.55 | 434/434 |
| runtime-floating | 249.32 [248.55–251.88] | 249.68 [248.64–252.79] | 1.0006 [0.9995–1.0061] | 248.60–251.72 | 230/230 |
| union-zero-runtime | 441.09 [434.63–446.69] | 87.66 [87.17–88.44] | 0.1989 [0.1971–0.2008] | 431.11–446.65 | 212/220 |
| conditional-runtime | entry rejects | 181.22 [180.83–181.58] | — | — | 274 |
| union-zero-runtime repeat | 441.37 [431.36–443.82] | 86.28 [85.72–88.55] | 0.1977 [0.1953–0.1986] | 429.86–441.85 | 212/220 |

Every executable checks a live result with a volatile loop bound. Native RSS is 256 KiB throughout. Text measures executable payload after the ELF entry in the supplied sectionless backend; these runtime inputs have no static data. Calls/memory/floating binaries are byte-identical. Their compiler timings (~6 ms) are startup-sensitive and retained in JSON without a latency claim. The final-only conditional executable checks both constructor and call behavior.

Selected-union zeroing changes `zeroinit 24x8` into three scalar stores, as required by the repaired anonymous-member fixture. In the supplied backend, repeated observations show about 80% lower runtime for eight added text bytes (3.77%). This is a measured benefit on the affected workload, not an optimization-level claim. Legality follows the semantic zero plan: only a selected aggregate child whose recorded semantic plan permits all-zero representation initialization enters this case; other child-plan kinds retain their own actions. The existing limit is at most eight stores and seven address calculations per small region; larger regions retain a bulk operation. There is no IR scan, fixed-point transform, new analysis or cache invalidation. Existing initializer expansion limits, 1,000,000-work/512-depth constant limits and loop fallbacks remain intact.

The [source-to-LowIR-to-ELF trace](../student.tests/pa17/publication-trace.json) records a class specialization with an out-of-class constructor template, renamed outer/inner parameters and a conditional explicit query. Typed head/frame/query identities feed recorded construction/conversion actions and the LowIR builder. Template source regions remain shared, projection retains context IDs, concrete list lanes are not reprojected, and TU-owned vectors/flat indices retain facts until TU release. Temporary candidate/substitution vectors release at their owner return. The trace includes untimed counters, source/IR/executable hashes, execution and decoded machine code. The explicit backend boundary is validation; production phase transport remains typed.

PA17/O0 mandates no numerical performance ceiling. Historical +15%, +16 MiB and 5.5× values remain diagnostic targets under spec.md §9; no mandated limit, correctness test or coverage is weakened. Required semantic costs, the noisy first run and the repeated +4.28% compiler cost are preserved. Existing [selection](selection-performance.md) and [checkpoint](checkpoint52-performance.md) measurements remain unchanged. All 22 remaining course failures remain implementation obligations; independent review is still required.

Reproduce with `python3 student.tests/pa17/publication_benchmark.py A B WORK OUT`, then `python3 student.tests/pa17/publication_recheck.py OUT RECHECK_OUT RECHECK_WORK`, using fresh output paths. Verify with `python3 student.tests/pa17/verify_publication.py`.
