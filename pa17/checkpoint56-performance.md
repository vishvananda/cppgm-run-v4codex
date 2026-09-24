# PA17 accumulated audit performance — loop 56

Final code: `e14b96fa9d4b3376e5922b8ad30093c3c0b0c759`. The [checkpoint campaign](../student.tests/pa17/checkpoint56-performance.json) compares entry `b739e08d` with final across **36 workloads**; the [cumulative campaign](../student.tests/pa17/checkpoint56-cumulative-performance.json) compares the last reviewed implementation `c43e8eb6` with final across **11 workloads**. Both retain every input, hash, preflight, warmup, observation and untimed work counter. The source range is all three accepted handoffs plus both audit fixes.

Host build: g++ `-std=gnu++11 -Wall -O3`, course test runner enabled. Student flags: `--emit-lowir -O0`. CPU 31 affinity, serial compiler/executable timing, no concurrent build or test campaign. Each common case uses one warmup per binary, four A/A observations and four ABBA blocks; final-only cases use a warmup and six observations. `--stats --validate-lowir` preflights are outside timing. Wall time, CPU time, peak RSS and context switches are retained. The tables report median within-block B/A ratios and full spread, not a ratio of independent medians.

Native validation uses the supplied O0 backend, bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA-256 `c3bae4acf3243d5a2fd6a15ef00e2d75d11a7d82715b1e4771f55165d0542490`. It is outside the production compiler. Runtime loops use volatile bounds and check live results; compilation and execution are measured separately. Text is payload after the ELF entry in the sectionless supplied backend; these runtime inputs have no static data. Own native generation and self-hosting belong to later stages.

| Binary | SHA-256 | Compiler .text bytes |
|---|---|---:|
| Last reviewed | `e54376e7412f26025d58faf1f03127adcb6cd8e41f0e45c2b2e938a76bb41e94` | 1,769,670 |
| Checkpoint entry | `fe00a580350630b44b23ef8fc05077488df9920909a429fb04df4994c092e420` | 1,777,478 |
| Final | `8f8dfd898edc50a919518cda43968b08e09f1e4255d128d391cfdef15c2628d7` | 1,777,798 |

Compiler text grows **320 bytes (0.018%)** for the audit fixes and **8,128 bytes (0.459%)** over the full range. This is separate from generated-program text. No optional optimization was added by the audit fixes.

Checkpoint compiler observations (milliseconds; peak RSS in KiB). Tiny runtime-source compiler timings remain in JSON; their roughly 6–7 ms duration is startup-sensitive and supports no latency claim.

| Workload | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Peak RSS A/B KiB |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 181.79 [159.05–323.87] | 181.50 [180.64–184.41] | 0.9992 [0.7096–1.0748] | 185.70–214.14 | 22300/22324 |
| common-loop-float-1500 | 196.49 [162.15–328.06] | 194.76 [153.76–268.39] | 0.8860 [0.7941–1.1110] | 157.96–269.61 | 30176/30212 |
| common-partials-6000 | 469.72 [461.67–731.02] | 468.03 [455.15–560.92] | 1.0001 [0.8307–1.0992] | 466.70–495.62 | 72904/72940 |
| common-loop-float-6000 | 629.62 [608.18–857.16] | 612.97 [600.31–675.99] | 0.9896 [0.8164–1.0359] | 626.77–962.97 | 104140/104208 |
| member-qualified-1500 | 303.74 [298.25–391.76] | 303.91 [298.82–342.19] | 1.0085 [0.8631–1.0527] | 297.77–411.90 | 50456/50352 |
| member-qualified-6000 | 1244.14 [1224.71–1324.15] | 1239.77 [1221.24–1362.14] | 0.9956 [0.9722–1.0186] | 1241.42–1274.20 | 184424/184340 |
| variable-cv-pack-1500 | 70.15 [68.74–72.75] | 71.69 [69.37–75.89] | 1.0305 [0.9903–1.0530] | 69.94–71.43 | 17044/17016 |
| partial-pack-query-1500 | 360.10 [349.60–378.17] | 357.24 [349.96–382.85] | 0.9954 [0.9895–1.0116] | 357.00–383.71 | 65092/65000 |
| member-partial-owner-1500 | 279.17 [277.28–291.16] | 279.11 [275.78–288.49] | 0.9925 [0.9914–1.0197] | 276.63–282.84 | 57108/57076 |
| variable-cv-pack-6000 | 270.32 [266.79–276.00] | 272.49 [269.98–298.97] | 1.0166 [1.0087–1.0593] | 272.53–309.05 | 51388/51464 |
| partial-pack-query-6000 | 1501.97 [1477.51–1539.89] | 1505.80 [1495.50–1958.62] | 1.0057 [0.9946–1.1421] | 1499.83–1579.50 | 244744/244672 |
| member-partial-owner-6000 | 1188.76 [1179.26–1245.51] | 1201.86 [1179.17–1249.68] | 1.0098 [0.9914–1.0145] | 1188.97–1217.54 | 213312/213340 |
| anonymous-members-1000 | 201.86 [182.04–223.97] | 197.99 [179.02–211.94] | 0.9729 [0.9554–0.9963] | 176.72–196.50 | 33952/33960 |
| conditional-constructors-1000 | 149.10 [145.91–150.91] | 149.68 [146.43–274.29] | 1.0158 [0.9949–1.4190] | 151.84–169.07 | 29332/29416 |
| array-values-1000 | 127.49 [125.72–129.62] | 128.65 [126.76–287.66] | 1.0150 [1.0084–1.6365] | 126.75–130.14 | 27056/27020 |
| anonymous-members-4000 | 747.53 [729.34–784.47] | 747.23 [724.82–770.88] | 0.9771 [0.9730–1.0256] | 736.41–798.34 | 118776/118764 |
| conditional-constructors-4000 | 618.90 [604.09–775.39] | 611.91 [598.07–654.50] | 0.9758 [0.8963–1.0203] | 609.57–650.71 | 100932/100928 |
| array-values-4000 | 533.20 [513.69–542.04] | 530.95 [525.46–563.20] | 1.0076 [0.9902–1.0274] | 518.61–628.48 | 91268/91372 |
| inherited-qualified-1000 | 136.65 [131.60–189.81] | 143.91 [131.80–171.00] | 1.0216 [0.8553–1.0990] | 130.75–176.16 | 25952/25916 |
| inline-qualified-1000 | 43.84 [43.01–50.64] | 43.40 [42.52–53.26] | 1.0052 [0.9803–1.0222] | 43.05–63.90 | 11844/11784 |
| fixed-pack-head-1000 | 91.17 [89.82–98.17] | 91.43 [88.84–94.55] | 1.0007 [0.9591–1.0276] | 89.01–92.93 | 20304/20452 |
| inherited-qualified-4000 | 561.90 [544.52–634.44] | 553.99 [544.63–626.47] | 0.9969 [0.9195–1.0664] | 542.49–575.11 | 86852/86856 |
| inline-qualified-4000 | 157.59 [156.08–159.39] | 158.99 [157.16–293.37] | 1.0104 [0.9966–1.4334] | 157.36–160.12 | 30360/30372 |
| fixed-pack-head-4000 | 363.64 [357.90–370.48] | 362.31 [356.72–441.06] | 0.9992 [0.9935–1.1026] | 354.82–364.91 | 63636/63648 |
| partial-nonclass-failure-1000 | entry rejects | 54.33 [53.73–117.05] | — | — | 12408 |
| partial-cv-failure-1000 | entry rejects | 92.02 [91.15–92.61] | — | — | 16876 |
| partial-nonclass-failure-4000 | entry rejects | 208.93 [206.30–213.24] | — | — | 33076 |
| partial-cv-failure-4000 | entry rejects | 366.60 [360.59–602.43] | — | — | 50452 |

All **31 common checkpoint LowIR outputs** are byte-identical. For the 24 nontrivial common compiler workloads, paired ratios range **0.8860–1.0305**, and peak RSS differences range **−104 to +148 KiB**. The 0.8860 loop/float ratio is noisy (A/A 158–270 ms, paired range 0.794–1.111) and is not claimed as a speedup. Variable-cv-pack costs +3.05% at 1,500 and +1.66% at 6,000; all four larger-case pairs are positive. The added type-category/failure checks and cache-state checks are constant work at the existing substitution owner; no duplicated traversal, new optional transform or growing search was found to remove. This bounded semantic cost is disclosed, not called an improvement or excused by a numerical threshold.

The four new compiler inputs are rejected by entry and accepted by final, so they establish semantic cost/scaling rather than A/B speedups. At 1,000→4,000 demands, non-class qualifier latency grows **3.85×**, RSS 12,408→33,076 KiB, substitution work 2,006→8,006 and candidates 1,002→4,002. Cv-failure latency grows **3.98×**, RSS 16,876→50,452 KiB, substitution work 2,006→8,006 and candidates 1,001→4,001. Retained source stays at two regions and 46/21 nodes respectively; neither demands member bodies. The final-only runtime checks a selected primary after candidate failure. These measurements bound the tested workload families, not arbitrary recursive templates.

Cumulative compiler observations from the previous reviewed implementation:

| Workload | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Peak RSS A/B KiB |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 114.16 [112.14–127.71] | 114.45 [110.90–129.14] | 1.0041 [0.9771–1.0407] | 114.45–135.07 | 22240/22580 |
| common-loop-float-1500 | 155.69 [154.19–170.46] | 156.30 [154.74–163.09] | 0.9974 [0.9728–1.0235] | 153.05–194.72 | 30156/30248 |
| common-partials-6000 | 469.63 [453.13–498.67] | 455.12 [448.25–471.83] | 0.9757 [0.9414–0.9913] | 456.12–516.08 | 72824/72972 |
| common-loop-float-6000 | 618.10 [598.46–637.06] | 603.93 [600.07–824.17] | 0.9849 [0.9714–1.1528] | 608.04–611.90 | 104064/104208 |
| member-qualified-1500 | 304.98 [298.19–336.89] | 300.27 [291.32–316.40] | 0.9804 [0.9612–0.9986] | 294.16–298.39 | 50300/50672 |
| member-qualified-6000 | 1245.76 [1224.40–1264.69] | 1250.93 [1225.11–1289.73] | 1.0044 [0.9918–1.0224] | 1227.15–1271.63 | 184936/184580 |

Cumulative common compiler paired ratios range **0.9757–1.0044**, with peak RSS differences **−356 to +372 KiB**. The six LowIR outputs are identical; no compiler speedup is inferred from fluctuations. These measurements close the interaction review across the whole range, supplementing each handoff’s preserved scaling campaign.

Checkpoint executable observations:

| Workload | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Peak RSS A/B KiB | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| runtime-calls | 360.74 [358.57–434.21] | 368.17 [359.10–449.10] | 1.0051 [0.9425–1.1277] | 359.25–371.15 | 256/256 | 206/206 |
| runtime-memory | 211.08 [210.10–232.93] | 211.47 [210.07–237.76] | 0.9991 [0.9913–1.0096] | 210.75–221.66 | 256/256 | 434/434 |
| runtime-floating | 251.03 [249.49–253.68] | 251.67 [249.59–255.42] | 1.0023 [0.9952–1.0094] | 249.55–250.06 | 256/256 | 230/230 |
| constant-boolean-runtime | 200.05 [195.71–214.18] | 201.07 [196.05–211.65] | 1.0026 [0.9703–1.0140] | 196.27–225.49 | 256/256 | 216/216 |
| union-zero-runtime | 87.39 [86.82–96.47] | 87.09 [86.84–88.87] | 0.9946 [0.9517–1.0122] | 87.30–91.26 | 256/256 | 220/220 |
| conditional-runtime | 181.91 [180.17–314.82] | 181.19 [179.87–183.05] | 0.9923 [0.7338–0.9994] | 180.65–186.73 | 256/256 | 274/274 |
| qualified-runtime | 160.98 [160.15–166.12] | 161.10 [160.30–166.66] | 1.0013 [0.9982–1.0032] | 161.15–162.29 | 256/256 | 258/258 |
| partial-failure-runtime | entry rejects | 92.58 [92.41–92.95] | — | — | 256 | 206 |

The seven common checkpoint executables are byte-identical, so their timing differences do not indicate generated-code changes. The eighth is final-only and checks the newly accepted partial-selection behavior. Native RSS is 256 KiB. All executions exit with the checked result.

Cumulative executable observations:

| Workload | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Peak RSS A/B KiB | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| runtime-calls | 378.37 [360.29–505.69] | 381.52 [360.30–694.04] | 0.9675 [0.9112–1.5088] | 358.45–361.15 | 256/288 | 206/206 |
| runtime-memory | 213.86 [210.11–304.65] | 221.07 [213.20–255.86] | 1.0413 [0.8531–1.1051] | 268.08–329.25 | 256/256 | 434/434 |
| runtime-floating | 268.30 [249.22–295.95] | 251.21 [248.30–290.30] | 0.9632 [0.9256–1.0715] | 250.13–256.71 | 256/256 | 230/230 |
| constant-boolean-runtime | 270.85 [192.75–358.19] | 246.66 [196.07–333.69] | 0.9799 [0.8734–1.0201] | 194.77–205.06 | 256/256 | 228/216 |
| union-zero-runtime | 451.39 [434.50–479.77] | 92.61 [86.91–101.57] | 0.2032 [0.1984–0.2171] | 432.11–446.49 | 256/256 | 212/220 |

Calls, memory and floating binaries remain identical to the previous reviewed tip. The constant-boolean case removes a recorded constant branch and saves **12 bytes (228→216)**; the current paired ratio is **0.9799 [0.8734–1.0201]**, with broad timing variation, so no repeatable speedup is claimed. The original selection campaign’s **2.66% slowdown** remains preserved and disclosed. Its required O0 comparison form and the supplied backend’s layout are unchanged by this audit; later native-layout policy is not a PA17 exit gate.

Union zeroing changes a legal 24-byte representation zero into three bounded stores. The cumulative run shows **0.2032× [0.1984–0.2171] runtime** for **eight extra bytes (212→220)**, consistent with publication’s original and repeated roughly 0.20× results. Thus the affected executable benefit is repeatable and its growth is bounded. The semantic plan proves legality; at most eight stores/seven internal address operations are emitted per small region, and larger regions retain the bulk fallback. No expression/IR scan, fixed-point loop, new analysis or invalidation is introduced. Nonconstant/volatile/effectful branch conditions and non-representation zero plans preserve their original actions. Existing 1,000,000-work/512-depth constant-evaluation limits and aggregate eight-element array-expansion budget remain unchanged.

The [final source-to-LowIR-to-ELF trace](../student.tests/pa17/checkpoint56-trace.json) records an out-of-class member partial, renamed enclosing/inner heads, a dependent alias template-template argument, primary fallback for a non-class qualifier, exact partial selection for a class member type, recorded calls/constants and checked machine execution. It includes source/IR/ELF hashes, telemetry and disassembly. Production data remains typed; the textual LowIR/backend boundary exists only for the assignment output and explicit validation.

Preservation: [selection](selection-performance.md), [publication](publication-performance.md) and [qualified](qualified-performance.md) evidence, including their repeats and noisy observations, is unchanged. The [interrupted audit campaign](../student.tests/pa17/checkpoint56-performance-before-cache-validity.json) retains three completed compiler measurements and the next preflight at `d37a1049`; it was deliberately terminated with status 143 when the incomplete-class negative-cache issue was identified. Its binary and outputs remain frozen. It is not pooled with or substituted for final acceptance. Historical hashes/outputs were revalidated in the audit manifest.

Acceptance follows spec.md §9 for **PA17/O0**, which mandates no numerical latency, RSS, runtime or text ceiling. Historical +15%, +16 MiB and 5.5× diagnostic targets remain measurements rather than additional exit gates. The audit introduced constant-sized identity/failure distinctions, corrected an invalid cache lifetime, and found no avoidable duplicated work or unprofitable optional transform to retain. Necessary semantic costs and supplied-backend constraints are explicit. All observations, correctness/comparison rules, coverage and existing work/growth limits are preserved. The 19 remaining course failures are implementation obligations and do not become performance waivers.

Reproduce with `python3 student.tests/pa17/checkpoint56_benchmark.py A B WORK OUT checkpoint` or `cumulative`, using fresh work/output paths. Verify the complete evidence with `python3 student.tests/pa17/verify_checkpoint56.py`.
