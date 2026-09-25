# PA18 audit 78 performance evidence

Acceptance is **PA18/O0 LowIR**, spec §9. Correct declaration demand, allocation
query semantics and constructor emission are language requirements. No optional
optimizer, native speedup or mandated numeric performance ceiling is introduced.

## Frozen protocol and complete corpus

[`audit78_benchmark.py`](../student.tests/pa18/audit78_benchmark.py) measures the
union of every workload from handoffs 75–77, including both retained parts of
75's interrupted run, and seven audit probes: **38 workloads per complete run**.
Checkpoint A is `0ed4fe5f`; cumulative A is previous reviewed code `8dc4636d`;
B is reviewed fix `82fca940`. All binaries, flags, source texts, hashes, platform,
CPU affinity, work counters and observations are retained in the
[checkpoint](../student.tests/pa18/loop78-performance.json),
[cumulative](../student.tests/pa18/loop78-performance-cumulative.json), and
[focused](../student.tests/pa18/loop78-performance-focused.json) records.
The focused harness is preserved with its hash in the evidence manifest.

Build flags: `g++ -std=gnu++11 -Wall -O3`, test runner enabled. Timed compiler
flags: `--emit-lowir -O0`. Separate preflight enables `--validate-lowir --stats`.
Each equivalent case has one warmup per binary, four A/A samples and four ABBA
blocks. Rejected prior behavior has six B-only samples, never a claimed speedup.
Compiler and executable execution are measured separately. Checked native output
uses the supplied backend at `-O0`, bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`. Timing does not include backend creation.

Checkpoint output comparison is exact for 31 workloads. Two unused-declaration
probes intentionally change their LowIR declaration view: removing unnecessary
class completion leaves an opaque parameter with no fabricated object size.
Their executable hashes are identical. Five new-behavior probes fail entry
preflight and have final-only costs. Cumulative comparison has 17 exact cases,
one equivalent declaration case and 20 prior rejections. Every produced executable
returns its checked result; all equivalent executable hashes match. Compile-only
workloads retain static assertions or established source semantics.

## Compiler latency and peak RSS

Checkpoint A/B values below are median milliseconds and maximum observed KiB.
Ratios are medians of paired ABBA blocks, followed by their full range.

| Workload | A / B ms | Paired B/A (range) | A / B RSS KiB |
|---|---:|---:|---:|
| ordering-600 | 57.05 / 56.34 | 0.967 (0.660–0.989) | 15360 / 15520 |
| ordering-2400 | 241.05 / 224.76 | 1.007 (0.881–1.603) | 43536 / 43736 |
| member-head-600 | 100.15 / 100.50 | 0.975 (0.372–1.012) | 22620 / 22672 |
| member-head-2400 | 416.96 / 421.31 | 0.990 (0.519–2.024) | 72604 / 72760 |
| common-loop-float-1500 | 258.44 / 268.53 | 1.139 (0.832–1.839) | 30224 / 30320 |
| runtime-calls | 8.61 / 8.60 | 0.994 (0.965–1.168) | 5876 / 6100 |
| runtime-memory | 6.44 / 6.33 | 0.971 (0.919–1.067) | 5888 / 6068 |
| runtime-floating | 6.62 / 6.34 | 0.964 (0.949–1.001) | 5928 / 6132 |
| list-argument-600 | 39.98 / 39.83 | 0.999 (0.961–0.999) | 10976 / 11228 |
| dependent-tag-600 | 70.99 / 67.97 | 0.929 (0.928–0.986) | 16340 / 16568 |
| cast-failure-600 | 36.86 / 36.37 | 0.989 (0.958–0.995) | 10712 / 10916 |
| list-argument-2400 | 148.92 / 146.13 | 0.985 (0.975–1.002) | 26828 / 27052 |
| dependent-tag-2400 | 270.62 / 270.48 | 1.000 (0.987–1.020) | 48400 / 48724 |
| cast-failure-2400 | 133.32 / 131.65 | 0.988 (0.942–0.993) | 25772 / 26176 |
| runtime-list-query | 6.57 / 6.39 | 0.977 (0.935–0.979) | 5956 / 5988 |
| inherited-declaration-600 | 55.45 / 55.14 | 1.002 (0.978–1.124) | 14304 / 14444 |
| inherited-query-600 | 61.85 / 60.54 | 0.988 (0.966–1.004) | 15576 / 15708 |
| inherited-declaration-2400 | 217.57 / 218.27 | 1.005 (0.991–1.081) | 40840 / 41136 |
| inherited-query-2400 | 239.30 / 237.20 | 0.987 (0.985–0.994) | 44936 / 45132 |
| inherited-body-160 | 36.42 / 36.02 | 0.990 (0.973–1.103) | 11344 / 11496 |
| inherited-body-640 | 131.70 / 131.32 | 0.991 (0.833–1.020) | 28404 / 28616 |
| runtime-inherited | 6.63 / 6.49 | 0.983 (0.963–0.988) | 5924 / 5960 |
| nested-dormant-600 | 24.06 / 23.39 | 0.976 (0.958–0.989) | 8996 / 9080 |
| nested-layout-600 | 102.55 / 102.75 | 1.006 (0.999–1.034) | 18256 / 18356 |
| nested-invalid-dormant-600 | 23.28 / 23.01 | 0.984 (0.982–0.995) | 8960 / 9228 |
| ambiguous-type-600 | 39.53 / 38.77 | 0.983 (0.971–1.013) | 11736 / 11888 |
| nested-dormant-2400 | 78.93 / 77.14 | 0.981 (0.978–0.992) | 18712 / 18944 |
| nested-layout-2400 | 415.81 / 415.43 | 1.003 (0.984–1.042) | 55544 / 55692 |
| nested-invalid-dormant-2400 | 77.39 / 76.63 | 0.983 (0.974–0.988) | 18508 / 18660 |
| ambiguous-type-2400 | 143.65 / 141.38 | 0.986 (0.933–1.000) | 28520 / 28680 |
| runtime-nested | 6.73 / 6.53 | 0.975 (0.966–0.983) | 5924 / 5896 |
| audit-new-list-600 | — / 36.77 | new behavior | — / 10208 |
| audit-incomplete-600 | 31.15 / 20.26 | 0.650 (0.647–0.655) | 10752 / 8724 |
| audit-deferred-emission-600 | — / 198.61 | new behavior | — / 36880 |
| audit-new-list-2400 | — / 132.46 | new behavior | — / 23732 |
| audit-incomplete-2400 | 110.58 / 63.26 | 0.572 (0.568–0.574) | 25820 / 16516 |
| audit-deferred-emission-2400 | — / 822.58 | new behavior | — / 130148 |
| runtime-audit-forwarding | — / 6.61 | new behavior | — / 6056 |

Checkpoint startup medians are **7.45 / 7.41 ms**. Small runtime-source compilation
samples are startup-limited and are reported as diagnostics. Scaled frontend
workloads dominate startup. The first common-loop sample has a paired median
**1.139**, but its range is **0.832–1.839**; large ordering/member samples likewise
show scheduler noise. All observations remain, including those outliers.

A focused repeat used the same frozen inputs, binaries, flags, affinity and
A/A/ABBA protocol after the complete cumulative batch. It did not reproduce the
common-loop slowdown. This supports no repeatable regression, **not** a general
speedup claim. The cumulative common-loop ratio is **1.003 (1.001–1.017)**.

| Focused checkpoint repeat | A / B ms | Paired B/A (range) | A / B RSS KiB |
|---|---:|---:|---:|
| ordering-2400 | 213.13 / 208.90 | 0.984 (0.970–0.997) | 43564 / 43624 |
| member-head-2400 | 394.09 / 394.37 | 1.006 (0.993–1.035) | 74048 / 74152 |
| common-loop-float-1500 | 153.23 / 152.29 | 0.914 (0.789–1.007) | 30228 / 30320 |

The cumulative table below covers large shared workloads and each affected owner;
the raw record contains both scales, all 38 workloads and every observation.

| Cumulative workload | A / B ms | Paired B/A (range) | A / B RSS KiB |
|---|---:|---:|---:|
| ordering-2400 | 214.29 / 212.82 | 0.997 (0.980–1.006) | 43632 / 43636 |
| member-head-2400 | 402.96 / 400.91 | 1.010 (0.997–1.049) | 73904 / 72700 |
| common-loop-float-1500 | 154.94 / 156.07 | 1.003 (1.001–1.017) | 30208 / 30328 |
| list-argument-2400 | — / 149.84 | new behavior | — / 26940 |
| dependent-tag-2400 | 273.57 / 271.88 | 0.997 (0.966–1.006) | 47668 / 48704 |
| cast-failure-2400 | — / 131.73 | new behavior | — / 26092 |
| inherited-declaration-2400 | 212.49 / 210.30 | 0.993 (0.990–0.997) | 39976 / 41044 |
| inherited-query-2400 | — / 234.84 | new behavior | — / 45044 |
| inherited-body-640 | — / 127.26 | new behavior | — / 28588 |
| nested-dormant-2400 | 405.67 / 77.67 | 0.193 (0.183–0.196) | 56224 / 18868 |
| nested-layout-2400 | 411.37 / 413.99 | 0.991 (0.974–1.017) | 55588 / 55648 |
| ambiguous-type-2400 | — / 140.23 | new behavior | — / 28604 |
| audit-incomplete-2400 | — / 64.25 | new behavior | — / 16444 |
| audit-new-list-2400 | — / 133.54 | new behavior | — / 23648 |
| audit-deferred-emission-2400 | — / 822.62 | new behavior | — / 130064 |

Largest equivalent RSS increases are **404 KiB** checkpoint and **1,068 KiB**
cumulative, alongside reductions on other cases. The accumulated class/template
records implement required declaration and forwarding facts, with linear graph
bounds; no material compiler latency regression repeats across the equivalent
corpus. Newly supported paths disclose their final-only memory/latency costs.

Compiler `.text` is **1,944,070 → 1,945,158 bytes** for this audit,
**+1,088 (0.056%)**; cumulative **1,897,286 → 1,945,158**, **+47,872 (2.523%)**.
These are semantic implementation costs. Generated equivalent executables do not
grow. Required forwarding wrappers and their distinct ABI entries are not optional
inlining or specialization growth.

## Checked runtime and generated size

Volatile bounds and data-dependent checked results prevent dead or folded loops.
The supplied backend emits sectionless ELF, so size is the retained executable
payload proxy, not a claimed `.text` section. These runtime sources contain no
static payload data. Checkpoint values below retain the noisy first observations;
identical executable bytes make those fluctuations environmental observations.

| Runtime workload | A / B seconds | Paired B/A (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 0.982391 / 0.858254 | 0.881 (0.772–1.186) | 206 / 206 |
| runtime-memory | 0.425802 / 0.424450 | 0.964 (0.896–1.021) | 434 / 434 |
| runtime-floating | 0.497582 / 0.499018 | 1.002 (0.995–1.006) | 230 / 230 |
| runtime-list-query | 0.114923 / 0.114899 | 0.991 (0.975–1.002) | 175 / 175 |
| runtime-inherited | 0.115666 / 0.115209 | 0.998 (0.994–1.005) | 187 / 187 |
| runtime-nested | 0.152271 / 0.152162 | 0.998 (0.992–1.006) | 205 / 205 |
| runtime-audit-forwarding | — / 0.062179 | new behavior | — / 216 |

Cumulative runtime medians for calls/memory/floating/nested are respectively
**1.004 / 0.999 / 1.000 / 1.003** B/A, again with identical executable bytes.
The other cumulative runtime cases have no correct prior executable and remain
final-only. No native runtime improvement is claimed from LowIR counts or frontend
speed. Native allocation, spill quality and optimized code are later-stage work.

## Work bounds and acceptance

Removing declaration-only completion reduces the audit's 600/2400-class compiler
latency to **0.650 / 0.572** of entry, with RSS reductions at both scales. It
removes source-member/definition work and never hides a demanded definition.
Accumulated dormant-nested work at 2400 classes is **0.193** of the previous
reviewed compiler, with RSS **56,224 → 18,868 KiB**. Demand/layout workloads retain
approximately equal compiler latency. These source/output-equivalent reductions
follow from the trace and work counters, rather than a smaller-IR assumption.

Fourfold audit input grows allocated-aggregate query time **3.60×**, incomplete
parameter declaration time **3.12×**, and constexpr-then-runtime forwarding time
**4.14×**. Completed forwarding bodies/actions and deferred emission edges are
reused. List tails, nested source regions and completion invalidation retain their
independent graph bounds recorded in [audit.md](audit.md). A later TU refreshes
only an incomplete output signature keyed by function identity; complete signatures
allocate no entries in that index. No whole-program retry or unlimited transform
is introduced.

Historical **+15%, +16 MiB, 5.5×** targets remain self-selected diagnostics. They
were already reclassified by the governing stage-scoped rule; this audit preserves
all measurements and does not reinstall them as exit gates. There is no weakened
mandated limit, correctness rule or test coverage. Required semantic costs and
PA24–PA34 backend/self-hosting constraints create no additional PA18/O0 exit gate.
Performance is accepted for this checkpoint; **32 PA18 correctness/comparison
failures remain**, and stage advancement still requires the through-PA18 report.
