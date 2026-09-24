# PA17 final performance audit — loop 62

Frozen implementation: `12140852`. [Audit-entry observations](../student.tests/pa17/final-performance.json) compare `0970f3f0` to final; [whole-stage observations](../student.tests/pa17/final-stage-performance.json) compare the PA17 base `21748547` to final. Each campaign retains 16 fixed workloads: the loop-61 common/closure/cleanup corpus verbatim plus exception-closure scaling and runtime controls. Historical reports and all their raw observations remain unchanged.

Build: g++ `-std=gnu++11 -Wall -O3`, course runner enabled; measured invocation `--emit-lowir -O0`. One allowed CPU (31), serial compiler/runtime measurements after all test/build jobs completed. One warmup per binary, four A/A calibration samples, then four ABBA blocks. Entry-invalid cases use six final-only observations after correctness preflight, never treating rejected behavior as fast. `--stats --validate-lowir` and supplied-backend translation/execution are outside compiler timing. Every binary, source, harness, command, flag, wall/user/system observation, context switch, RSS, output hash and counter is retained.

Native backend: pinned bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, O0. Runtime inputs use volatile bounds and checked live arithmetic/call/memory/floating effects. Sectionless ELF code size is payload minus typed global bytes (**code plus alignment**), with raw payload/data/file sizes retained; it is not an ELF `.text` section. The compiler size is its actual `.text`. Own native backend and self-hosting remain later-stage surfaces.

| Compiler | SHA-256 | .text bytes |
|---|---|---:|
| Stage base | `faa771f2230c9df4424910df9d542b2daad4994aceb0abcb2dd031d4401431a8` | 1,669,510 |
| Audit entry | `89859de908e8af110a1e4cb85d830d1547e7b81cf413075f5b2abd0dbf338144` | 1,813,958 |
| Final | `4436d458cdd896585e5cb7065bf5e751644288ad9fc4cd4b09e576c3434ea4ca` | 1,814,470 |

Final compiler text adds **512 bytes (0.028%)** over audit entry and **144,960 bytes (8.68%)** over the stage base. This covers the completed entity model and correctness fixes; no optional optimization pass was added.

## Audit entry → final

Compiler time is ms, RSS is KiB. Ratios are the median of four within-block B/A means, not ratios of independently pooled medians. The A/A and pair ranges explicitly show host noise.

| Compiler workload | A/B median ms | Paired B/A [range] | A/A ms range | Peak RSS A/B |
|---|---:|---:|---:|---:|
| common-partials-1500 | 182.57 / 174.67 | 0.9143 [0.7886–1.0666] | 191.46–257.90 | 22,692/22,668 |
| common-partials-6000 | 804.18 / 820.80 | 1.0491 [0.5976–1.5861] | 590.91–844.95 | 73,228/72,944 |
| common-loop-float-1500 | 220.55 / 265.98 | 1.0035 [0.8838–1.2839] | 258.78–296.00 | 30,268/30,268 |
| common-loop-float-6000 | 1133.19 / 1020.48 | 0.8480 [0.8038–1.0030] | 1263.31–1438.39 | 104,216/104,164 |
| regions-600 | 78.64 / 79.42 | 1.0293 [0.9249–1.3208] | 82.52–105.73 | 15,900/15,900 |
| closures-600 | 147.60 / 138.51 | 0.9615 [0.8297–0.9875] | 133.97–152.47 | 23,360/23,328 |
| regions-2400 | 427.65 / 417.71 | 0.9063 [0.8469–1.0353] | 339.00–391.43 | 46,084/46,036 |
| closures-2400 | 1408.96 / 1187.91 | 0.9270 [0.6120–1.0981] | 1318.72–1549.63 | 73,728/73,828 |
| exception-closures-600 | — / 244.58 | final only | — | —/23,508 |
| exception-closures-2400 | — / 1071.10 | final only | — | —/77,236 |

| Executable workload | A/B median ms | Paired B/A [range] | A/A ms range | Code+alignment A/B |
|---|---:|---:|---:|---:|
| runtime-calls | 868.90 / 861.91 | 0.9978 [0.9549–1.0520] | 1173.76–1224.05 | 206/206 |
| runtime-memory | 825.93 / 804.73 | 0.9987 [0.9559–1.1010] | 670.14–860.86 | 434/434 |
| runtime-floating | 822.02 / 825.07 | 0.9950 [0.9665–1.0280] | 813.00–1044.10 | 230/230 |
| runtime-regions | 1328.03 / 1317.34 | 1.0058 [0.9736–1.0215] | 1196.56–1367.70 | 1020/1020 |
| runtime-closures | 1217.25 / 1239.38 | 0.9904 [0.9783–1.0686] | 1073.46–1171.28 | 316/316 |
| runtime-exception-closures | — / 1106.28 | final only | — | —/316 |

Startup medians A/B: 7.75/7.12 ms. Runtime-source compilations remain startup-sensitive diagnostics in the JSON, not compiler-speed evidence. All native samples report 256 KiB peak RSS.

## Whole-stage base → final

Compiler time is ms, RSS is KiB. Ratios are the median of four within-block B/A means, not ratios of independently pooled medians. The A/A and pair ranges explicitly show host noise.

| Compiler workload | A/B median ms | Paired B/A [range] | A/A ms range | Peak RSS A/B |
|---|---:|---:|---:|---:|
| common-partials-1500 | 219.06 / 221.14 | 0.9968 [0.9624–1.0603] | 188.47–220.42 | 21,908/22,756 |
| common-partials-6000 | 944.71 / 994.24 | 1.0276 [1.0094–1.1292] | 824.28–897.37 | 71,680/73,072 |
| common-loop-float-1500 | 381.37 / 375.36 | 1.0256 [0.9218–1.0463] | 344.44–395.24 | 29,772/30,208 |
| common-loop-float-6000 | 628.83 / 659.85 | 1.0737 [0.8334–1.2653] | 1306.71–1465.22 | 103,996/104,172 |
| regions-600 | 58.89 / 63.82 | 1.0935 [1.0749–1.1917] | 58.41–62.31 | 15,764/15,840 |
| closures-600 | — / 118.18 | final only | — | —/23,228 |
| regions-2400 | 227.91 / 246.63 | 1.0714 [0.8437–1.1101] | 227.52–233.31 | 46,672/46,076 |
| closures-2400 | — / 469.13 | final only | — | —/73,784 |
| exception-closures-600 | — / 128.10 | final only | — | —/23,532 |
| exception-closures-2400 | — / 533.02 | final only | — | —/77,264 |

| Executable workload | A/B median ms | Paired B/A [range] | A/A ms range | Code+alignment A/B |
|---|---:|---:|---:|---:|
| runtime-calls | 719.92 / 717.76 | 0.9994 [0.9910–1.0024] | 721.24–751.33 | 206/206 |
| runtime-memory | 420.01 / 417.56 | 0.9945 [0.9813–1.0013] | 416.61–418.97 | 434/434 |
| runtime-floating | 495.66 / 496.26 | 1.0001 [0.9976–1.0066] | 494.30–497.01 | 230/230 |
| runtime-regions | 1047.13 / 772.69 | 0.7353 [0.7125–0.7433] | 1042.60–1085.94 | 1132/1020 |
| runtime-closures | — / 678.67 | final only | — | —/316 |
| runtime-exception-closures | — / 680.58 | final only | — | —/316 |

Startup medians A/B: 11.75/11.60 ms. Runtime-source compilations remain startup-sensitive diagnostics in the JSON, not compiler-speed evidence. All native samples report 256 KiB peak RSS.

## Interpretation and acceptance

All **13 common audit LowIR outputs** and **five common runtime binaries** are byte-identical. The compiler ratios vary from 0.8480 to 1.0491 on the nontrivial common corpus, with exceptionally wide pairs (partials-6000: 0.5976–1.5861). A/A spreads are similarly large. No general compiler gain, regression, or runtime improvement is established by this noisy campaign. The stage campaign shows visibly changing host conditions within the run; never compare unpaired absolute times across campaigns. No samples were discarded.

Counter comparison supplies the source-level check against avoidable work: audit common-partial, loop and region semantic work counts are unchanged. At 2400 plain closures, canonical signature publication adds only two distinct signature-normalization facts and one type probe in total, not per-body rechecking; all other work/entity/scope counts agree. The complete-key signature cache shares the work. There is no extra transform to remove. Required exception closures were rejected by entry and are final-only.

Across the whole stage, partials-6000 costs 2.76% paired compiler time / 1392 KiB peak RSS; canonical argument slices rise 18,005 → 30,005 and signature work 6003 → 18,005, while type substitution work falls 18,004 → 18,002. Complete source-head/frame identity and exact partial checking account for these linear facts; no repeated class/body computation is introduced. Loop-float-6000 has unchanged semantic work, +176 KiB RSS and a noisy 1.0737 ratio (0.8334–1.2653); it does not demonstrate a repeatable regression. These observations retain the necessary stage costs without turning old diagnostic targets into gates.

The full-expression grouping has a repeatable executable outcome: stage runtime ratio **0.7353**, code **1132 → 1020 bytes**, consistent with loop 61’s independent **0.7400/0.7452** observations on the identical workload. Larger cleanup compilation costs **1.0714** paired over stage base, with RSS **46,672 → 46,076 KiB**. Grouping required regions avoids repeated runtime enter/exit while preserving destruction order and live suffixes. This is measured runtime evidence, not inference from fewer IR nodes. Audit-entry/final region binaries are identical. Common calls/memory/floating stage executables are also identical.

In the steadier final-only stage samples, 600 → 2400 closures scale **3.97×** in compiler time; exception closures scale **4.16×** and **3.28×** in peak RSS. Their counts are exactly 600/2400 closures and 1801/7201 body checks. Exception work is **3000/12,000**; template source binding stays **46**. At 2400 instances, plain and exception closures both use **40,812 entities and 28,811 scopes**, confirming parameter identity reuse. Additional exception facts add about **3480 KiB** over the plain corpus. Different source inputs do not establish a speed comparison; these counts establish work growth. Both closure runtime forms have code size **316 bytes**, checked results, and final-only observations when base/entry lacks the behavior.

PA17/O0 has **no mandated numeric latency, memory or code-size ceiling**. Existing eight-lane lowering expansion, constant-evaluation work/depth limits, memoized effect walks and O(L log L) closure numbering remain binding. Historical +15%, +16 MiB and 5.5× targets are diagnostic choices under spec §9, not extra exit criteria. All historical misses and observations are preserved. Necessary semantics and later-stage constraints do not authorize avoidable regressions, weakened tests or missing behavior; source/counter review found none left in the audited stage.

The earlier [entity](entity-performance.md), [heads](head-performance.md), [selection](selection-performance.md), [publication](publication-performance.md), [transfer](transfer-performance.md), [queries](query-performance.md), [storage](storage-performance.md), [checkpoint](checkpoint60-performance.md) and [closure/cleanup](handoff61.md) campaigns retain their workloads, A/A/ABBA samples, gains and disclosed regressions. Their frozen hashes are in the final ledger. The final comparisons supplement, rather than overwrite, this evidence.
