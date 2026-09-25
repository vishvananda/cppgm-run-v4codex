# PA18 checkpoint audit 74 performance evidence

Frozen audit entry `2433d6de`, previous reviewed code `f59e8f67`, and final code
`8dc4636d` compare PA18/O0 LowIR compilation. The reviewed baseline binary is
`/tmp/pa18-loop71/cppgm-entry`, verified against audit 70’s final binary hash.
Both final comparisons use `/tmp/pa18-loop74/cppgm-final`. Build flags are
`g++ -std=gnu++11 -Wall -O3`, with the test runner enabled. Flags, CPU affinity
(CPU 31), platform, binary/source/output hashes, warmups, raw wall/RSS/user/system
samples, context switches and separate telemetry are recorded in the artifacts:

- [Final checkpoint observations](../student.tests/pa18/loop74-performance.json): 36 workloads.
- [Cumulative observations](../student.tests/pa18/loop74-performance-cumulative.json): the same 36 sources.
- [Focused repeat](../student.tests/pa18/loop74-performance-focused.json): seven fixed large frontend sources, resolving scheduling noise.
- [Preserved initial audit run](../student.tests/pa18/loop74-performance-attempt1.json): 34 workloads at `f4b9020b`, before the namespace/raw-parameter follow-up. Its B binary is retained as `/tmp/pa18-loop74/cppgm-f4b9020b`; its original output path was later reused for final B. The recorded hash pins the measured binary.

The [committed harness](../student.tests/pa18/audit74_benchmark.py) takes the union
of all handoff 71–73 benchmark sources, checking source equality for repeated
names, and adds prototype/pack, conversion-exposure and namespace-return families
at 600/2400 declarations plus a checked runtime case. The focused harness is
retained verbatim inside its JSON; it selects seven existing sources and changes
no flags or protocol. No build or validation suite ran concurrently with timing.

Protocol: one warmup per binary, four A/A samples and four ABBA blocks. A baseline
must emit exact equivalent LowIR and executable bytes before a paired comparison.
If it rejects newly required behavior, six final-only samples report necessary
cost without presenting rejection speed as a baseline. Namespace return tests
are new behavior relative to audit entry, but equivalent relative to `f59e8f67`.
Validation and telemetry run separately from timing. All observations are retained;
no outliers are removed. Large frontend runs dominate roughly 5–7 ms startup;
compilation of the small runtime sources remains startup-limited.

## Audit-entry compiler comparison

| Workload | A / B median ms | Paired B/A median (range) | A / B peak RSS KiB |
|---|---:|---:|---:|
| ordering-600 | 56.01 / 56.46 | 0.997 (0.929–1.082) | 15252 / 15256 |
| ordering-2400 | 219.17 / 222.70 | 1.036 (0.872–1.115) | 43100 / 43132 |
| member-head-600 | 103.59 / 102.45 | 0.983 (0.849–0.996) | 22916 / 22984 |
| member-head-2400 | 419.33 / 413.49 | 0.993 (0.933–1.043) | 73644 / 73592 |
| common-loop-float-1500 | 155.43 / 156.00 | 1.004 (0.990–1.613) | 30308 / 30336 |
| runtime-calls | 5.80 / 6.01 | 1.045 (0.995–1.160) | 5848 / 5884 |
| runtime-memory | 6.42 / 6.45 | 0.996 (0.948–1.031) | 5932 / 5836 |
| runtime-floating | 5.83 / 5.80 | 1.001 (0.979–1.007) | 6008 / 6056 |
| member-context-600 | 151.66 / 149.95 | 0.978 (0.952–1.016) | 31176 / 31204 |
| lexical-alias-600 | 168.94 / 164.79 | 0.919 (0.364–0.999) | 32984 / 33000 |
| member-definition-600 | 208.11 / 203.38 | 0.982 (0.663–1.000) | 40052 / 40156 |
| member-context-2400 | 1149.14 / 659.48 | 0.433 (0.288–0.895) | 106900 / 106936 |
| lexical-alias-2400 | 717.92 / 717.18 | 1.000 (0.522–1.401) | 114588 / 114668 |
| member-definition-2400 | 882.47 / 893.68 | 1.054 (0.388–2.018) | 142512 / 142456 |
| runtime-new-member | 6.78 / 6.60 | 0.989 (0.962–1.025) | 5988 / 5988 |
| alias-formation-600 | 75.54 / 75.43 | 0.971 (0.378–1.694) | 17524 / 17532 |
| pack-capture-600 | 195.07 / 194.10 | 0.726 (0.229–1.012) | 34552 / 34612 |
| alias-formation-2400 | 305.95 / 304.34 | 0.905 (0.253–1.013) | 52368 / 52336 |
| pack-capture-2400 | 829.58 / 810.75 | 1.148 (0.382–3.327) | 120160 / 120180 |
| runtime-new-pack | 6.93 / 6.91 | 0.990 (0.972–1.035) | 5912 / 5888 |
| invocation-600 | 139.81 / 139.38 | 0.995 (0.979–4.545) | 26492 / 26516 |
| prototype-this-600 | 94.50 / 93.71 | 0.993 (0.972–1.008) | 20320 / 20352 |
| invocation-2400 | 570.39 / 569.49 | 0.965 (0.285–1.078) | 88652 / 88652 |
| prototype-this-2400 | 369.79 / 369.07 | 1.013 (0.987–1.548) | 64480 / 64488 |
| runtime-invocation | 6.62 / 6.54 | 0.987 (0.974–0.989) | 5848 / 5960 |
| using-hiding-600 | 127.95 / 129.48 | 1.007 (0.999–1.048) | 25828 / 25800 |
| indirect-query-600 | 50.81 / 44.10 | 0.879 (0.851–0.944) | 13320 / 12172 |
| using-hiding-2400 | 520.59 / 531.10 | 1.016 (1.003–1.020) | 85776 / 85576 |
| indirect-query-2400 | 200.77 / 171.73 | 0.855 (0.848–0.865) | 36572 / 31408 |
| audit-prototype-600 | — / 190.71 | new behavior | — / 33720 |
| audit-exposure-600 | — / 76.73 | new behavior | — / 16508 |
| audit-namespace-600 | — / 128.80 | new behavior | — / 24048 |
| audit-prototype-2400 | — / 818.76 | new behavior | — / 117708 |
| audit-exposure-2400 | — / 310.74 | new behavior | — / 48516 |
| audit-namespace-2400 | — / 528.38 | new behavior | — / 79212 |
| runtime-audit | — / 6.39 | new behavior | — / 5812 |

Empty-input medians: 5.66 / 5.53 ms.

## Cumulative compiler comparison

| Workload | A / B median ms | Paired B/A median (range) | A / B peak RSS KiB |
|---|---:|---:|---:|
| ordering-600 | 57.95 / 57.47 | 0.992 (0.984–1.036) | 15192 / 15256 |
| ordering-2400 | 224.13 / 224.60 | 1.011 (0.985–1.028) | 42968 / 43712 |
| member-head-600 | 101.65 / 101.29 | 0.994 (0.963–1.006) | 22876 / 22840 |
| member-head-2400 | 416.39 / 419.70 | 1.008 (0.957–1.131) | 74116 / 73968 |
| common-loop-float-1500 | 161.44 / 162.46 | 1.006 (0.998–1.205) | 30148 / 30308 |
| runtime-calls | 6.11 / 5.92 | 0.968 (0.961–0.973) | 5824 / 5864 |
| runtime-memory | 6.11 / 5.95 | 0.965 (0.951–0.999) | 5860 / 5896 |
| runtime-floating | 6.26 / 6.05 | 0.975 (0.916–0.982) | 6052 / 6012 |
| member-context-600 | 170.23 / 183.98 | 1.089 (0.966–1.157) | 30172 / 31224 |
| lexical-alias-600 | — / 172.98 | new behavior | — / 32980 |
| member-definition-600 | — / 225.02 | new behavior | — / 40164 |
| member-context-2400 | 635.49 / 661.48 | 1.019 (0.946–1.065) | 103832 / 106908 |
| lexical-alias-2400 | — / 751.32 | new behavior | — / 114472 |
| member-definition-2400 | — / 848.07 | new behavior | — / 142476 |
| runtime-new-member | — / 6.45 | new behavior | — / 5808 |
| alias-formation-600 | 69.91 / 72.72 | 1.038 (1.018–1.484) | 17124 / 17460 |
| pack-capture-600 | — / 182.97 | new behavior | — / 34700 |
| alias-formation-2400 | 280.07 / 288.32 | 1.018 (0.966–1.045) | 50948 / 52280 |
| pack-capture-2400 | — / 789.36 | new behavior | — / 121944 |
| runtime-new-pack | — / 6.71 | new behavior | — / 5852 |
| invocation-600 | — / 132.58 | new behavior | — / 26640 |
| prototype-this-600 | — / 88.31 | new behavior | — / 20704 |
| invocation-2400 | — / 556.57 | new behavior | — / 89128 |
| prototype-this-2400 | — / 361.87 | new behavior | — / 65184 |
| runtime-invocation | — / 6.34 | new behavior | — / 5952 |
| using-hiding-600 | 124.18 / 120.65 | 0.968 (0.966–0.981) | 26528 / 25776 |
| indirect-query-600 | 48.53 / 42.67 | 0.878 (0.865–1.463) | 13380 / 12176 |
| using-hiding-2400 | 520.10 / 500.97 | 0.962 (0.945–0.971) | 89600 / 85900 |
| indirect-query-2400 | 188.85 / 174.07 | 0.978 (0.900–1.830) | 36336 / 31364 |
| audit-prototype-600 | — / 185.24 | new behavior | — / 33556 |
| audit-exposure-600 | — / 74.95 | new behavior | — / 16560 |
| audit-namespace-600 | 127.19 / 127.27 | 1.002 (0.993–1.034) | 24276 / 24116 |
| audit-prototype-2400 | — / 788.51 | new behavior | — / 118076 |
| audit-exposure-2400 | — / 299.45 | new behavior | — / 49096 |
| audit-namespace-2400 | 502.34 / 507.20 | 0.999 (0.990–1.026) | 78920 / 79348 |
| runtime-audit | — / 6.33 | new behavior | — / 5988 |

Empty-input medians: 5.68 / 5.56 ms.

## Focused compiler repeat

| Workload | A / B median ms | Paired B/A median (range) | A / B peak RSS KiB |
|---|---:|---:|---:|
| ordering-2400 | 495.48 / 584.50 | 0.972 (0.938–1.143) | 43200 / 43256 |
| member-context-2400 | 1426.30 / 1290.52 | 0.912 (0.889–0.973) | 106916 / 106904 |
| lexical-alias-2400 | 1451.13 / 1441.38 | 0.986 (0.934–1.019) | 114468 / 114512 |
| member-definition-2400 | 1827.20 / 1825.68 | 1.003 (0.990–1.161) | 142764 / 142780 |
| pack-capture-2400 | 1546.73 / 1775.16 | 1.012 (0.927–1.101) | 120172 / 120164 |
| using-hiding-2400 | 534.42 / 532.74 | 1.016 (0.777–1.045) | 85756 / 85752 |
| indirect-query-2400 | 189.91 / 162.65 | 0.832 (0.719–0.872) | 36392 / 31428 |

Empty-input medians: 12.16 / 12.01 ms.

## Checked executable runtime and size

The supplied native backend is pinned to bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, invoked with `-O0` separately from
compiler timing. Runtime inputs are volatile and loop results are checked; every
execution returns zero. It emits sectionless ELF, so the recorded payload is a
code-size proxy for these sources without static data, not a claimed `.text`
section. No equivalent executable grows; all paired executable hashes match.

### Checkpoint

| Workload | A / B median s | Paired B/A median (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 0.740754 / 0.734833 | 0.976 (0.802–1.039) | 206 / 206 |
| runtime-memory | 0.435490 / 0.422728 | 0.731 (0.420–0.993) | 434 / 434 |
| runtime-floating | 0.501228 / 0.498538 | 0.951 (0.673–1.007) | 230 / 230 |
| runtime-new-member | 0.116440 / 0.116952 | 1.005 (1.000–1.008) | 179 / 179 |
| runtime-new-pack | 0.174496 / 0.175284 | 1.000 (0.995–2.083) | 251 / 251 |
| runtime-invocation | 0.127328 / 0.125408 | 0.988 (0.931–0.996) | 191 / 191 |
| runtime-audit | — / 0.138092 | new behavior | — / 215 |

### Cumulative

| Workload | A / B median s | Paired B/A median (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 0.741716 / 0.743257 | 1.000 (0.978–1.005) | 206 / 206 |
| runtime-memory | 0.421400 / 0.420852 | 0.996 (0.995–1.002) | 434 / 434 |
| runtime-floating | 0.551439 / 0.541024 | 0.986 (0.938–1.014) | 230 / 230 |
| runtime-new-member | — / 0.113942 | new behavior | — / 179 |
| runtime-new-pack | — / 0.171534 | new behavior | — / 251 |
| runtime-invocation | — / 0.121967 | new behavior | — / 191 |
| runtime-audit | — / 0.137924 | new behavior | — / 215 |

Compiler `.text`, checkpoint: **1,892,934 → 1,897,286 bytes**, **+4,352 (0.230%)**.
Compiler `.text`, cumulative: **1,866,374 → 1,897,286 bytes**, **+30,912 (1.656%)**.

## Work scaling and semantic costs

Fourfold input growth below uses final checkpoint observations; timing ratios
include scheduler noise, so canonical work counts are reported alongside them.

| Family, 600 → 2400 | B time ratio | Type-substitution work | Frames | Type queries | Signature shapes |
|---|---:|---:|---:|---:|---:|
| ordering | 3.94× | 3005 → 12005 | 1800 → 7200 | 601 → 2401 | 5 → 5 |
| member-head | 4.04× | 4802 → 19202 | 5400 → 21600 | 1204 → 4804 | 3 → 3 |
| member-context | 4.40× | 9616 → 38416 | 5403 → 21603 | 3609 → 14409 | 4810 → 19210 |
| lexical-alias | 4.35× | 16223 → 64823 | 10807 → 43207 | 4211 → 16811 | 4 → 4 |
| member-definition | 4.39× | 16832 → 67232 | 7804 → 31204 | 5415 → 21615 | 7820 → 31220 |
| alias-formation | 4.03× | 4208 → 16808 | 1805 → 7205 | 603 → 2403 | 4 → 4 |
| pack-capture | 4.18× | 19206 → 76806 | 19801 → 79201 | 601 → 2401 | 5 → 5 |
| invocation | 4.09× | 8412 → 33612 | 5405 → 21605 | 2416 → 9616 | 5 → 5 |
| prototype-this | 3.94× | 7808 → 31208 | 4800 → 19200 | 1807 → 7207 | 13 → 13 |
| using-hiding | 4.10× | 13806 → 55206 | 6000 → 24000 | 603 → 2403 | 7 → 7 |
| indirect-query | 3.89× | 5403 → 21603 | 2400 → 9600 | 3606 → 14406 | 3 → 3 |
| audit-prototype | 4.29× | 8405 → 33605 | 4200 → 16800 | 6616 → 26416 | 4811 → 19211 |
| audit-exposure | 4.05× | 0 → 0 | 1200 → 4800 | 603 → 2403 | 0 → 0 |
| audit-namespace | 4.10× | 7200 → 28800 | 1200 → 4800 | 0 → 0 | 5 → 5 |

The final checkpoint's large scheduling outliers are explicit: member-context-2400
A/A spans **0.658–1.995 s**, pack-capture-2400 **0.809–1.685 s**, and
member-definition-2400 **0.868–1.544 s**. Individual paired ratios include
**0.253–4.545** across frontend cases. Even the unchanged runtime-memory source
has an A/A compilation outlier near one second. The focused repeat therefore
uses a fixed set of seven large sources, retaining its own A/A calibration and
all four ABBA blocks. It is not a selection of favorable samples.

The initial final-checkpoint pack-capture paired median **1.148** becomes **1.012**
(range **0.927–1.101**) in the repeat; member-definition **1.054** becomes **1.003**
(**0.990–1.161**), and ordering **1.036** becomes **0.972** (**0.938–1.143**).
Their semantic work counters are unchanged by the audit. These observations do
not establish an intrinsic regression or general speedup. Using-hiding has a
small **1.016** checkpoint median in both final batches; the repeat's paired
range **0.777–1.045** does not resolve that movement reliably. Its candidate,
substitution and graph work are unchanged. Namespace/class key selection is a
required correctness distinction, with no extra pass or optional policy added.

Indirect-query-2400 has paired medians **0.895** in the first audit run,
**0.855** in the final checkpoint, and **0.832** in the repeat. The necessary
abstract-parameter check completes class declarations before conversion
classification. Existing class facts then avoid speculative conversion objects:
candidates fall **9600 → 4800**, conversions **9600 → 7200**, types
**36015 → 16815**, and entities **24009 → 12009**. LowIR remains byte-identical.
This is a measured secondary benefit of consuming required declaration facts,
not an optional optimizer or a claimed runtime improvement. Final checkpoint
peak RSS falls **36,572 → 31,408 KiB** in that case. Elsewhere checkpoint RSS
is effectively flat (maximum observed increase **112 KiB**).

Across the full reviewed range, necessary semantic costs remain visible.
Member-context paired medians are **1.089 / 1.019** at 600 / 2400, with peak RSS
**+1052 / +3076 KiB**. Canonical source signature shapes add **4810 / 19210**
computations while type-substitution work decreases; the complete owner/context
keys are needed for the repaired redeclarations and cross-specialization results.
Alias-formation paired medians are **1.038 / 1.018**, with peak RSS
**+336 / +1332 KiB**. At 2400 uses, type-substitution work rises
**4804 → 16808**, frames **4803 → 7205**, and access checks **0 → 2400**:
transparent results must still validate source formation, defaults and access.
These computations are cached at their canonical recipe/frame owners and scale
with actual uses; deleting them would restore the demonstrated correctness bugs.
The new prototype family also shares source object contexts and exception
queries, with only demanded specializations evaluated. There is no global retry,
unbounded specialization search or retained duplicate parse representation.

Cumulative using-hiding medians are **0.968 / 0.962**, reflecting the previously
reviewed reduction in hidden candidates. The namespace-return family is
**1.002 / 0.999**, with exact LowIR equality against the reviewed baseline.
New lexical aliases, pack captures, invocation and exception/exposure behavior
have no valid earlier executable; their final costs and fourfold work scaling
are reported without comparing against erroneous rejection.

Common executables have identical hashes and payload sizes in every paired
comparison. Cumulative runtime medians are **1.000** for calls, **0.996** for
memory and **0.986** for floating point, with their complete spreads above.
Checkpoint runtime outliers are environmental variation on identical bytes;
no runtime benefit or degradation is attributed to the frontend changes.
The final-only audit runtime has a **215-byte payload** and about **0.138 s**
median checked execution. Its conversion/query/exception requirements were
unsupported at entry. No own native allocator or optimization claim is made.

## Stage-scoped acceptance

Acceptance is **PA18/O0 LowIR** under spec §9. There is no mandated numerical
latency/RSS/runtime ceiling at this stage. The inherited **+15%, +16 MiB and
5.5×** numbers were self-selected diagnostic targets, not assignment or spec
limits; they remain diagnostics in the compact plan. All older observations,
including missed targets and failed attempts, remain intact. Historical misses
do not permanently fail the corrected implementation, and rejection speed is
not an equivalent correct baseline. Correctness, required coverage, precise
fact ownership and bounded work remain mandatory.

The audit introduces required semantic checks and repairs, with no optional
optimization to justify by speculative runtime profit. O0 has no new fixed-point
pass, inlining, cloning or code-growth policy; work follows candidate lists,
canonical type/recipe operands and demanded fact keys. Unknown facts retain
conservative behavior and local demand owns invalidation. Fourfold sources
produce approximately fourfold canonical work, with timing noise disclosed;
new native code appears only for demanded functions. The compiler text increases
are bounded implementation costs, not generated-program growth. Measurements
show no material avoidable regression requiring another implementation change;
small latency differences remain subject to the disclosed host noise. They are
not converted into new unsupported gates. Native selection/allocation budgets,
O1–O3 optimization profitability and self-hosting remain PA24–PA34 obligations.
All 41 remaining PA18 correctness failures are still required work.
