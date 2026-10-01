# PA29 checkpoint audit170 performance evidence

Code: `221d6d0e4930da05db2913bdf5f50d808f89c744`. Acceptance is PA29/O0; these changes add required semantics, not an optional optimizer pass. No speedup is claimed.

[Manifest](../student.tests/pa29/evidence170/performance-manifest.json) freezes binaries, flags, scripts and platform. Input hashes and every wall-time/RSS sample are retained in the linked series. Compiler bytes: reviewed **4,078,728**, entry **4,109,760**, final **4,109,848** (+88 audit bytes).

There are **1,384 successful observations**: 224 cumulative common, 224 audit-only common, 840 equivalent affected, and 96 audit capability samples, plus six launcher observations. Equivalent comparisons use four A/A samples then six ABBA blocks per workload/mode. The capability families use eight samples per size/mode; entry rejects them, so their timings are costs with no correct A baseline. Host linking is outside compiler timing. No builds or correctness suites overlap timing; documentation/evidence collection and external scheduling are uncontrolled. Every outlier is retained.

## Cumulative: previous reviewed binary versus final

[All observations](../student.tests/pa29/evidence170/cumulative-performance.json). Times are median seconds, RSS is maximum KiB; brackets give the full ABBA paired-ratio range. All A/B objects and executables are byte-identical.

| Workload | Compile A/B s | Compile ratio [range] | RSS A/B | Runtime A/B s | Runtime ratio [range] | Text bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1670/0.1686 | 1.0039 [0.9091–1.1925] | 29740/29920 | 0.0708/0.0715 | 0.9989 [0.9858–1.0160] | 151633 |
| floating | 0.1618/0.1620 | 1.0644 [0.9884–1.3248] | 29368/29544 | 0.0491/0.0490 | 0.9996 [0.9812–1.0414] | 151474 |
| exceptions | 0.1999/0.2092 | 1.0877 [0.8179–1.2521] | 29792/29824 | 0.2523/0.2531 | 1.0058 [0.9939–1.0165] | 151781 |
| pruning | 0.2050/0.2034 | 0.9921 [0.9810–1.2567] | 35424/35540 | 0.0709/0.0707 | 0.9975 [0.9908–1.0055] | 151633 |

| Workload | A/A compile range s | A/A runtime range s |
|---|---:|---:|
| memory | 0.1639–0.2321 | 0.0701–0.0711 |
| floating | 0.2407–0.2589 | 0.0489–0.0497 |
| exceptions | 0.2639–0.2725 | 0.2511–0.5131 |
| pruning | 0.2009–0.2029 | 0.0524–0.0740 |

## Audit fixes: entry versus final

[All observations](../student.tests/pa29/evidence170/audit-performance.json). Times are median seconds, RSS is maximum KiB; brackets give the full ABBA paired-ratio range. All A/B objects and executables are byte-identical.

| Workload | Compile A/B s | Compile ratio [range] | RSS A/B | Runtime A/B s | Runtime ratio [range] | Text bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1629/0.1632 | 1.0006 [0.9860–2.0017] | 28972/29080 | 0.0528/0.0528 | 0.9989 [0.9959–1.0311] | 151633 |
| floating | 0.1632/0.1631 | 0.9966 [0.6943–1.1856] | 29064/29116 | 0.0945/0.0918 | 0.9703 [0.9351–1.0159] | 151474 |
| exceptions | 0.4007/0.3870 | 0.9787 [0.9315–1.0274] | 29192/29296 | 0.5238/0.5314 | 0.9616 [0.7833–1.2789] | 151781 |
| pruning | 0.4046/0.4108 | 1.0139 [0.9744–1.1715] | 35412/35552 | 0.0871/0.0872 | 1.0047 [0.9563–2.2901] | 151633 |

| Workload | A/A compile range s | A/A runtime range s |
|---|---:|---:|
| memory | 0.1611–0.1627 | 0.0527–0.0533 |
| floating | 0.1627–0.1661 | 0.0957–0.0991 |
| exceptions | 0.4036–0.4278 | 0.6128–0.7777 |
| pruning | 0.4331–0.4853 | 0.0880–0.0922 |

## Affected owners and capability scaling

[All observations](../student.tests/pa29/evidence170/affected-performance.json). Layout, wrapper demand, designators, scalar-array mutation and blocks use the unchanged handoff sources, verified by hashes. Every A/B image and every non-time work counter are identical. The table reports final costs for all sizes; paired ratios apply only to equivalent A/B inputs.

| Input | Final compile [range] s | Compile B/A [range] | Compiler RSS | Runtime [range] s | Runtime RSS | Text bytes |
|---|---:|---:|---:|---:|---:|---:|
| layout600 | 0.2033 [0.1965–0.2624] | 1.0093 [0.8188–1.1849] | 21484 | 0.1762 [0.1707–0.1911] | 1760 | 38321 |
| layout1200 | 0.4123 [0.3772–0.5358] | 0.9824 [0.9336–1.0532] | 35656 | 0.1808 [0.1622–0.3302] | 1760 | 76121 |
| layout2400 | 0.7813 [0.7653–0.8945] | 1.0004 [0.9737–1.0482] | 63224 | 0.1785 [0.1519–0.2289] | 1760 | 151721 |
| wrappers600 | 0.1343 [0.1279–0.1982] | 1.0073 [0.7625–1.2017] | 15840 | 0.3884 [0.3640–0.4873] | 1760 | 38355 |
| wrappers1200 | 0.2573 [0.2334–0.5043] | 0.9923 [0.8920–1.2396] | 24476 | 0.4074 [0.3638–0.5148] | 1756 | 76155 |
| wrappers2400 | 0.5401 [0.4740–0.6540] | 0.9678 [0.8251–1.0422] | 41672 | 0.4021 [0.3572–0.4411] | 1756 | 151755 |
| designators600 | 0.3689 [0.2972–0.4304] | 0.9701 [0.7380–1.0398] | 19060 | 0.2530 [0.2187–0.3660] | 1764 | 56921 |
| designators1200 | 0.4120 [0.3683–0.4342] | 1.0241 [0.9383–1.0639] | 31328 | 0.1667 [0.1171–0.2460] | 1756 | 113321 |
| designators2400 | 0.3417 [0.3320–0.5513] | 0.9808 [0.9071–1.3046] | 55300 | 0.1196 [0.1173–0.1269] | 1764 | 226121 |
| mutation600 | 0.1412 [0.1384–0.6112] | 1.0002 [0.9764–2.5993] | 26164 | 0.1424 [0.1356–0.1542] | 1760 | 70121 |
| mutation1200 | 0.2864 [0.2724–0.2956] | 0.9999 [0.7031–1.0180] | 45120 | 0.1471 [0.1381–0.1603] | 1884 | 139721 |
| mutation2400 | 0.6206 [0.5634–0.7011] | 0.9593 [0.8871–1.1312] | 83212 | 0.1597 [0.1413–0.2210] | 1760 | 278921 |
| blocks600 | 0.0871 [0.0681–0.1062] | 0.9808 [0.8838–1.0561] | 15604 | 0.1578 [0.1506–0.3089] | 1756 | 41424 |
| blocks1200 | 0.1851 [0.1213–0.6609] | 1.0129 [0.9986–2.4440] | 23996 | 0.2461 [0.2424–0.7896] | 1760 | 82224 |
| blocks2400 | 0.2290 [0.2238–0.5097] | 0.9966 [0.4476–1.6203] | 40660 | 0.1491 [0.1488–0.1502] | 1760 | 163824 |
| pointers600 | 0.0739 [0.0732–0.0756] | capability | 14980 | 0.1032 [0.1029–0.1132] | 1760 | 33521 |
| pointers1200 | 0.1392 [0.1378–0.1397] | capability | 23164 | 0.1031 [0.1026–0.1037] | 1760 | 66521 |
| pointers2400 | 0.2754 [0.2713–0.8515] | capability | 38200 | 0.1037 [0.1033–0.1059] | 1760 | 132521 |
| queries600 | 0.0683 [0.0678–0.0698] | capability | 15168 | 0.1662 [0.1646–0.1677] | 1760 | 55875 |
| queries1200 | 0.1302 [0.1284–0.1321] | capability | 23064 | 0.1657 [0.1654–0.1714] | 1760 | 111075 |
| queries2400 | 0.2566 [0.2541–0.2616] | capability | 38252 | 0.1669 [0.1650–0.1720] | 1760 | 221475 |

Launcher median **0.0085 s**, range [0.0076–0.0097]. The shortest affected median runtime is **12.1×** launcher median. All kernels depend on runtime argc and check independently calculated sums; the demand calls and 20-million-iteration loops remain executable work.

The pointer capability requires N distinct constant evaluations whose changed pointer fields reach a later-mutated pointee. The vector-query capability substitutes lane types through list queries and emits scalar size results. Neither needs runtime vector operations. The entry binary rejects all six sources, as recorded alongside the final measurements.

[All-sample counter assertions](../student.tests/pa29/evidence170/scaling-counters.json) show unchanged common/inherited work. Scalar-array mutation remains exactly **22N+24** evaluation steps, **2N+7** addresses and **2N** dependency work. The new pointer family has **36N+5** steps, **6N+3** addresses and **10N+1** dependency work. These bounds measure complete reachable storage keys without freezing unrelated scalar arrays. Canonical vector-query facts reuse eight lane shapes across N demanded functions. Storage and output grow with actual demands; timing noise does not support an exact exponent claim.

## Acceptance and historical evidence

Cumulative floating/exception paired compiler ratios are 1.0644/1.0877, with paired ranges crossing 1 and substantial outliers. Audit-only paired compiler medians span 0.9787–1.0139. Byte-identical runtimes also vary with scheduling. These observations disclose regressions and uncertainty; they establish neither a repeatable runtime change nor an avoidable compiler regression. No optional transform is accepted on timing noise. Required storage-key correctness and vector-query validation cannot be dropped for speed.

[Historical review](../student.tests/pa29/evidence170/historical-performance-review.json) verifies all **912 final handoff observations**, their frozen binary/input hashes and common-image equality. All preliminary handoff measurements remain untouched. This preserves the complete cumulative evidence rather than replacing it with the latest comparison.

Additional optional optimization work/growth budgets are **zero**. The pointer index adds two link IDs per sparse overlay and one head per storage owner; insert/removal is O(1), retirement is amortized over removed paths, and traversal visits only actual address-bearing dependencies. Values and links belong to the activation frame and die with it. Query initialization consumes canonical lane counts in O(explicit lanes) with repeated zero tails. Existing 1,000,000-step/512-depth evaluator, native frame/data 0x70000000 and 4096 alignment limits, and course timeouts remain unchanged.

Historical blanket 15% latency/RSS and zero-growth targets remain diagnostic, self-selected targets under spec §9. Their measurements are preserved; they do not add PA29 exit gates. Correctness, coverage and mandated limits remain requirements. Broader hosted runtime, optimizer/allocation and self-hosting evidence retain PA30–34 ownership.
