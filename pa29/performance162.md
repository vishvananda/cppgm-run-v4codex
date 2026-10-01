# PA29 audit162 performance evidence

Reviewed code: `cce8634c3c835cf6d5e8f4fa5fea0db213959718`. O0 current-stage acceptance; no optional optimization or speedup claim.

The [manifest](../student.tests/pa29/evidence162/performance-manifest.json) freezes four binaries, scripts, flags, CPU and all observations. The prior reviewed binary matches audit158’s final hash. The entry binary matches implementation161. Compilation and checked execution are timed separately, with CPU affinity 0, wall clocks and `/usr/bin/time` peak RSS. Runtime argc and checked checksums keep the loops live; the common workloads demand 2,400 templates.

Two final common experiments each retain AAAA calibration plus six ABBA blocks for four workloads in both modes (224 observations each). Each affected experiment has eight compilation and eight execution samples for six demand shapes (96 observations), plus six launcher calibrations. All **1,088 preliminary and final observations** remain in the linked JSON files. The preliminary binary predates the cv/identity fix and is not final-endpoint evidence.

No compiler build or correctness suite overlapped timing. Light review/evidence activity and external scheduling were uncontrolled. The final file audit overlapped the final two unaligned2400 execution samples; those observations remain included. Timing spreads, including large outliers, are reported rather than filtered.

## Equivalent implementations

### Previous review → final

[All observations](../student.tests/pa29/evidence162/cumulative-performance.json). RSS is maximum KiB; timings are medians. Brackets contain the full range of paired B/A block ratios. Objects and executables are byte-identical for every A/B pair.

| Workload | Compile A / B s | Paired compile [range] | Compiler RSS A / B | Runtime A / B s | Paired runtime [range] | Text A = B |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.16208 / 0.16171 | 1.0014 [0.8294, 1.0986] | 29748 / 29752 | 0.05257 / 0.05240 | 0.9972 [0.9551, 1.0020] | 151633 |
| floating | 0.19256 / 0.18970 | 0.9828 [0.8461, 1.1528] | 29828 / 29860 | 0.04902 / 0.04899 | 0.9972 [0.2739, 1.0070] | 151474 |
| exceptions | 0.16141 / 0.16178 | 1.0133 [0.5856, 1.0401] | 29768 / 29840 | 0.25051 / 0.26017 | 1.0207 [0.9668, 1.0282] | 151781 |
| pruning | 0.22987 / 0.19913 | 0.9299 [0.6298, 1.1916] | 35356 / 35112 | 0.06363 / 0.06050 | 1.0032 [0.9033, 1.0570] | 151633 |

| Workload | A/A compile range s | A/A runtime range s | Runtime RSS A / B KiB |
|---|---:|---:|---:|
| memory | 0.34641–0.51784 | 0.05247–0.05262 | 1760 / 1884 |
| floating | 0.16835–0.20279 | 0.04879–0.04938 | 1756 / 1756 |
| exceptions | 0.15796–0.18512 | 0.25247–0.26916 | 3936 / 3932 |
| pruning | 0.19549–0.33176 | 0.05167–0.06727 | 1756 / 1760 |

### Audit entry → final

[All observations](../student.tests/pa29/evidence162/audit-cost-performance.json). RSS is maximum KiB; timings are medians. Brackets contain the full range of paired B/A block ratios. Objects and executables are byte-identical for every A/B pair.

| Workload | Compile A / B s | Paired compile [range] | Compiler RSS A / B | Runtime A / B s | Paired runtime [range] | Text A = B |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.16667 / 0.16051 | 0.9927 [0.7985, 1.2264] | 29768 / 29832 | 0.05207 / 0.05205 | 0.9994 [0.9941, 1.0587] | 151633 |
| floating | 0.19216 / 0.17729 | 0.9835 [0.7763, 1.5120] | 29864 / 29828 | 0.05991 / 0.05982 | 1.0013 [0.9679, 1.0104] | 151474 |
| exceptions | 0.25643 / 0.25714 | 1.0016 [0.9085, 1.2493] | 29908 / 29844 | 0.25217 / 0.25115 | 0.9881 [0.7372, 0.9995] | 151781 |
| pruning | 0.19553 / 0.19554 | 0.9953 [0.9928, 1.0057] | 35060 / 35140 | 0.05247 / 0.05239 | 1.0012 [0.9870, 1.0063] | 151633 |

| Workload | A/A compile range s | A/A runtime range s | Runtime RSS A / B KiB |
|---|---:|---:|---:|
| memory | 0.25778–0.27496 | 0.05728–0.07456 | 1760 / 1760 |
| floating | 0.16172–0.26513 | 0.04894–0.06033 | 1760 / 1760 |
| exceptions | 0.25711–0.29709 | 0.25093–0.25191 | 3932 / 3936 |
| pruning | 0.19382–0.19508 | 0.05212–0.05240 | 1756 / 1760 |

Cumulative paired compiler medians span **0.9299–1.0133**; audit-only medians span **0.9835–1.0016**. Several blocks have large timing variation. These data do not justify an exact percent latency claim. Equal native bytes rule out a generated-code explanation for runtime timing differences. The new code adds no whole-program pass; existing work counts on common inputs remain stable. There is no repeatable avoidable performance regression established by these observations.

## Corrected behavior costs

The entry binary fails seven new reducer checks, including wrong results and misaligned native crashes. Its affected execution is not a correct equivalent baseline. [Final affected observations](../student.tests/pa29/evidence162/affected-performance.json) measure the corrected implementation alone. Snapshot work combines a trait on a demanded class, atomic bool compound updates, a reference snapshot and pointer-like member invocation. Unaligned work exercises declared aligned(1) fields through scalar libatomic store/load/CAS recipes. Each executable checks all demanded functions and two million runtime calls.

| Workload / demanded functions | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| snapshot600 | 0.21177 [0.21051, 0.21618] | 34400 | 0.15192 [0.15149, 0.15242] | 1760 | 214182 |
| snapshot1200 | 0.44727 [0.42772, 0.47897] | 62220 | 0.15286 [0.15197, 0.15491] | 1888 | 427782 |
| snapshot2400 | 0.88842 [0.87471, 0.90240] | 116656 | 0.15265 [0.15099, 0.15399] | 2256 | 854982 |
| unaligned600 | 0.10720 [0.10590, 0.10975] | 19980 | 0.17195 [0.16991, 0.21895] | 1756 | 228521 |
| unaligned1200 | 0.20968 [0.20513, 0.21300] | 32972 | 0.17460 [0.17283, 0.21979] | 2008 | 456521 |
| unaligned2400 | 0.42304 [0.41830, 0.42616] | 58552 | 0.17227 [0.17048, 0.21769] | 2480 | 912521 |

| Workload | Body transitions | Type-query work | Legacy type / member work | Native instructions |
|---|---:|---:|---:|---:|
| snapshot600 | 600 | 7219 | 600 / 600 | 52862 |
| snapshot1200 | 1200 | 14419 | 1200 / 1200 | 105662 |
| snapshot2400 | 2400 | 28819 | 2400 / 2400 | 211262 |
| unaligned600 | 600 | 611 | 0 / 0 | 45053 |
| unaligned1200 | 1200 | 1211 | 0 / 0 | 90053 |
| unaligned2400 | 2400 | 2411 | 0 / 0 | 180053 |

Launcher median is 0.00453 s [0.00424, 0.04485]. Measured workloads dominate startup. Demand doubling approximately doubles compiler latency, incremental memory, work counts and text. Canonical type count for the runtime fallback workload stays at 33. These measurements establish bounded required work, not runtime optimization profit.

## Acceptance and retained evidence

The compiler grows from 3,935,752 bytes at the previous review to 4,009,912 (+74,160), including +4,256 over audit entry. All equivalent common generated text sizes are unchanged. New required capability has explicit per-function costs; source operands evaluate once and CAS retry is runtime contention, not compiler fixed-point work. Native selection requires supported width and sufficient recorded alignment; other layouts use the runtime ABI.

No new optional transformation is introduced: new optional work/growth budgets are **zero**. Existing constexpr 1,000,000-step/depth-512 limits, native frame/data bounds, global alignment bound and assignment timeouts are retained. Atomic order strengthening and strong implementation of weak CAS retain the documented conservative contract. The added reference temporary and boolean computation are necessary semantics, and unsupported native alignment is a correctness defect, not a performance option.

Historical blanket 15% latency/RSS and zero-growth targets remain self-selected diagnostics under spec §9. They add no exit gate; all older and preliminary measurements remain available. PA29 does not own broad hosted runtime, general optimization/allocation or self-hosting acceptance: those remain PA30–34 work. No required PA29 behavior, coverage, comparison or mandated limit is waived.

Preliminary datasets: [cumulative](../student.tests/pa29/evidence162/preliminary-cumulative.json), [audit cost](../student.tests/pa29/evidence162/preliminary-audit-cost.json), [affected](../student.tests/pa29/evidence162/preliminary-affected.json). Earlier evidence remains in [159](performance159.md), [160](performance160.md) and [161](performance161.md).
