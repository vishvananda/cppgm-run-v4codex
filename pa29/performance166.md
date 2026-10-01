# PA29 audit166 performance evidence

Final code: `f07f78236eb475648834ed78afbca6c864f64408`. Level: PA29/O0. Required semantic corrections; no new optional transform or speedup claim.

[Manifest](../student.tests/pa29/evidence166/performance-manifest.json) freezes compiler hashes, flags, scripts and CPU affinity 0. The reviewed binary is audit162's accepted `cce8634c`; entry is `3036bd4e`; final is the audited code above. Sizes are **4,009,912 → 4,074,632 → 4,078,728 bytes**. Each common batch has four A/A samples and six ABBA blocks per workload and mode (224 observations). Four affected batches retain eight compilation and eight runtime samples at each of six demand shapes (384 observations total), plus 24 launcher calibrations. Total final observations: **832**, excluding launcher samples.

All workloads check results using runtime input; host linking is outside compiler timing. `/usr/bin/time` records peak RSS; a wall clock measures the invocation. No compiler build or correctness suite overlapped final timing; light review/documentation and external scheduling were uncontrolled. Every observation and outlier remains. Common templates demand 2,400 specializations; loops/calls/memory/floating/exception work remain the fixed earlier inputs. Pruning adds 1,200 unused functions. Self-hosting remains PA34-owned.

## Complete reviewed range

[All observations](../student.tests/pa29/evidence166/cumulative-performance.json). Seconds are medians; paired ratios use each ABBA block’s B/A means. Brackets show the full paired range. RSS is maximum KiB. Every A/B object and executable is byte-identical.

| Input | Compile A/B s | Paired compile [range] | Compiler RSS A/B | Runtime A/B s | Paired runtime [range] | A=B text bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.4957/0.4485 | 0.9815 [0.4187–1.3433] | 29392/29636 | 0.2999/0.1696 | 0.7476 [0.3848–1.3941] | 151633 |
| floating | 0.4368/0.4272 | 0.9034 [0.6528–1.0997] | 29864/29528 | 0.1457/0.1480 | 0.7137 [0.6149–2.2999] | 151474 |
| exceptions | 0.1630/0.1618 | 0.9903 [0.7270–0.9986] | 29660/29372 | 0.2520/0.2508 | 0.9980 [0.9861–1.0022] | 151781 |
| pruning | 0.1975/0.1997 | 1.0115 [0.5769–1.0200] | 34888/35584 | 0.0531/0.0530 | 0.9950 [0.9896–1.0098] | 151633 |

| Input | A/A compiler range s | A/A runtime range s |
|---|---:|---:|
| memory | 0.3660–0.8268 | 0.1509–0.3473 |
| floating | 0.4725–2.0657 | 0.1566–0.5608 |
| exceptions | 0.1608–0.1657 | 0.2550–0.2584 |
| pruning | 0.1965–0.1986 | 0.0523–0.0526 |

## Audit fixes only

[All observations](../student.tests/pa29/evidence166/audit-cost-performance.json). Seconds are medians; paired ratios use each ABBA block’s B/A means. Brackets show the full paired range. RSS is maximum KiB. Every A/B object and executable is byte-identical.

| Input | Compile A/B s | Paired compile [range] | Compiler RSS A/B | Runtime A/B s | Paired runtime [range] | A=B text bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1625/0.1620 | 0.9966 [0.9863–1.0172] | 29560/29632 | 0.0522/0.0524 | 1.0011 [0.9839–1.0075] | 151633 |
| floating | 0.1610/0.1614 | 1.0013 [0.5973–1.0129] | 29332/29524 | 0.0488/0.0489 | 0.9997 [0.9955–1.0041] | 151474 |
| exceptions | 0.1625/0.1609 | 1.0010 [0.5758–1.0109] | 29320/29300 | 0.2509/0.2512 | 1.0017 [0.9639–1.1511] | 151781 |
| pruning | 0.1971/0.1983 | 1.0008 [0.8870–1.0176] | 35556/35588 | 0.0524/0.0525 | 1.0006 [0.9980–5.2006] | 151633 |

| Input | A/A compiler range s | A/A runtime range s |
|---|---:|---:|
| memory | 0.1614–0.1636 | 0.0522–0.0527 |
| floating | 0.1615–0.1715 | 0.0488–0.0492 |
| exceptions | 0.1611–0.1758 | 0.4855–0.5019 |
| pruning | 0.1975–0.2017 | 0.0523–0.0572 |

The audit-only paired compiler ratios are 0.9966–1.0013 and runtime ratios 0.9997–1.0017. Full-range memory/floating blocks show much larger scheduling variation: byte-identical executables nevertheless have widely different timings. That variation does not establish a runtime improvement or regression. Final compiler ratios span 0.9034–1.0115 with broad individual ranges; the preserved preliminary batches were near parity. No repeatable avoidable regression is established. Necessary additional semantic state and all RSS observations are disclosed, without a percent-level guarantee.

## Required capability and scaling

[Assembly](../student.tests/pa29/evidence166/affected163-performance.json), [strings/queries](../student.tests/pa29/evidence166/affected164-performance.json), [evaluation/storage](../student.tests/pa29/evidence166/affected165-performance.json), and [audit interactions](../student.tests/pa29/evidence166/affected166-performance.json) retain all final observations, phase/work counters, input/object/executable hashes and images. Compilation and execution are measured separately. Demand shapes are 600/1,200/2,400 specializations; fixed runtime loops execute 20 million calls (30 million for strings/queries).

The new reference family combines predefined-string queries, a mode-selected reference to a bare value parameter, assembly output and observable temporary destruction. Allocation passes a mode-dependent first extent to a source-defined allocator that records requested bytes; every call checks the expected extent. The entry compiler fails every final audit-interaction checksum ([entry observations](../student.tests/pa29/evidence166/affected-entry.json)). It is therefore not an equivalent affected performance baseline. These are correctness-cost and scaling measurements, not A/B speedups.

| Family / demands | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| register600 | 0.1217 [0.1189–0.1267] | 17440 | 0.2214 [0.2207–0.2218] | 1764 | 61121 |
| register1200 | 0.5761 [0.3468–1.1491] | 27340 | 0.3606 [0.3259–0.6260] | 1760 | 121721 |
| register2400 | 0.8336 [0.5015–2.7694] | 47428 | 0.4567 [0.3163–0.6573] | 1764 | 242921 |
| locked600 | 0.2708 [0.2058–0.5172] | 17448 | 1.5277 [1.3895–2.0391] | 1760 | 69521 |
| locked1200 | 0.3803 [0.1509–0.5467] | 27536 | 0.7119 [0.7000–0.7158] | 1764 | 138521 |
| locked2400 | 0.3013 [0.2984–0.3254] | 47204 | 0.7063 [0.7002–0.7161] | 1760 | 276521 |
| strings600 | 0.0623 [0.0619–0.0627] | 16304 | 0.1747 [0.1741–0.1774] | 1764 | 47933 |
| strings1200 | 0.1166 [0.1154–0.1217] | 25424 | 0.1752 [0.1744–0.2453] | 1756 | 95333 |
| strings2400 | 0.2258 [0.2236–0.2272] | 43936 | 0.1762 [0.1747–0.1824] | 1760 | 190133 |
| queries600 | 0.0667 [0.0658–0.0942] | 17476 | 0.1809 [0.1801–0.1847] | 1764 | 40731 |
| queries1200 | 0.1258 [0.1236–0.1300] | 27408 | 0.1831 [0.1817–0.1874] | 1760 | 80931 |
| queries2400 | 0.2459 [0.2442–0.2495] | 47276 | 0.1838 [0.1826–0.1844] | 1764 | 161331 |
| evaluation600 | 0.0938 [0.0922–0.0973] | 18392 | 0.1718 [0.1708–0.1732] | 1756 | 95873 |
| evaluation1200 | 0.1816 [0.1785–0.1845] | 29384 | 0.1711 [0.1704–0.1728] | 1760 | 191273 |
| evaluation2400 | 0.3593 [0.3579–0.3664] | 52072 | 0.1731 [0.1722–0.1747] | 1764 | 382073 |
| storage600 | 0.2107 [0.2072–0.2146] | 30920 | 0.1346 [0.1339–0.1395] | 1756 | 112057 |
| storage1200 | 0.4248 [0.4194–0.4736] | 57596 | 0.1356 [0.1344–0.1416] | 1868 | 223657 |
| storage2400 | 0.8621 [0.8555–0.8948] | 107336 | 0.1323 [0.1314–0.1328] | 1816 | 446859 |
| reference600 | 0.1932 [0.1892–0.1964] | 31944 | 0.4533 [0.4498–0.4581] | 3816 | 254488 |
| reference1200 | 0.3914 [0.3867–0.4216] | 56744 | 0.4584 [0.4545–0.4653] | 4084 | 508317 |
| reference2400 | 0.7987 [0.7867–0.8104] | 100708 | 0.4533 [0.4514–0.4629] | 4624 | 1015917 |
| allocation600 | 0.1790 [0.1749–0.1853] | 28164 | 0.2561 [0.2503–0.2587] | 1756 | 133164 |
| allocation1200 | 0.3710 [0.3390–0.5385] | 45896 | 0.2508 [0.2502–0.2586] | 1756 | 265764 |
| allocation2400 | 0.7026 [0.6950–0.7317] | 91060 | 0.2491 [0.2476–0.2529] | 2000 | 530964 |

| Batch | Launcher median [range] s | Shortest runtime / launcher median |
|---|---:|---:|
| 163 | 0.0046 [0.0044–0.0057] | 47.8× |
| 164 | 0.0046 [0.0044–0.0051] | 37.7× |
| 165 | 0.0048 [0.0046–0.0052] | 27.3× |
| 166 | 0.0042 [0.0040–0.0056] | 58.3× |

The affected assembly batch also has pronounced scheduling variation, so its final wall times cannot support an exact scaling exponent. Work counters and text remain proportional to demand, with one parsed source recipe per family. All 18 inherited affected inputs, objects and executables are byte-identical to their handoff versions ([image comparison](../student.tests/pa29/evidence166/historical-image-comparison.json)), so those timing shifts do not reflect changed generated code. Strings/queries, evaluation/storage and the final reference/allocation families show approximately proportional compiler time and memory. Required runtime loop work is fixed across demand sizes; code-layout and scheduling differences are disclosed by the full ranges. No latency or runtime speedup is inferred from the varying samples.

[Counter evidence](../student.tests/pa29/evidence166/scaling-counters.json) verifies every sample at each demand size. For N demands, assembly has N body transitions and one recipe; string objects scale as N. Evaluation has N mode observations/values and N+1 mode activations; storage retains the documented 4N observations, 2N values, 3N+2 activations, 3N initialized objects and N reference plans. The integrated reference family has N source strings/reference plans and N+3/N+4 body transitions (three/four demanded name lengths), sharing its single source recipe. Allocation has 2N body transitions (count plus work), with per-expression runtime extent facts. Compiler work is average O(1) per keyed fact and linear in actual recipe/type/output components. No unrelated declaration scan, unbounded search or growth loop is introduced.

## Preserved evidence and acceptance

All **832 preliminary observations and 24 launcher samples** are retained under `evidence166/preliminary-*-performance.json`, with their [manifest](../student.tests/pa29/evidence166/preliminary-performance-manifest.json). That binary preceded the final query-receiver completion invalidation. Preliminary reference inputs used `N+1` and an empty destructor; final inputs demand the bare-argument materialization and observable destruction. Their measurements are not an equivalent-input A/B comparison. Preliminary entry controls are retained too; no previously passing preliminary workload is described as a failed baseline.

The original handoff observations and their manifest review remain in [historical evidence](../student.tests/pa29/evidence166/historical-performance-review.json). No historical measurement was removed. New optional work and code-growth budgets are **zero**. Existing constexpr step/depth, native frame/data/alignment limits and assignment timeouts remain unchanged. Generated image equality on equivalent common workloads rules out hidden code growth there. Required new storage, correct control flow and allocation bytes are measured above.

Historical blanket 15% latency/RSS and zero-growth goals are self-selected diagnostics, as already classified under spec §9; they are not PA29 exit gates. Necessary correctness costs and PA30–34-owned broad hosted runtime, optimization/allocation and self-hosting work do not create new gates. No mandated limit, correctness requirement, fixture or comparison rule is weakened.
