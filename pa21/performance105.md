# PA21/O0 checkpoint audit performance (105)

[All observations](../student.tests/pa21/audit105-performance.json) come from
[benchmark105.py](../student.tests/pa21/benchmark105.py): fixed source/flags,
frozen binary/backend hashes, one warmup each, four A/A calibration samples and
four ABBA blocks, with compiler and executable timing separate. Affinity pins
one CPU. `/usr/bin/time` measures peak RSS. Telemetry/LowIR validation runs
separately and does not enter the timed commands. Sources, checked exits, every
sample, paired ratios and spreads remain in the record.

Stage A: `ac988ea3`; audit entry A: `3b87e462`; reviewed B: `f65eae8d`.
The first six rows compare stage base with B; remaining rows compare audit entry
with B. The five throwing prefix reducers are excluded from A/B benefit claims:
the entry compiler is wrong there. The normal-path copy loop below checks equal
results on its exercised path and measures the conservative cleanup cost.

Host build uses the course g++ configuration (`-std=gnu++11 -Wall -O3` and
existing entry/tool flags). Timed compiler flags are `--emit-lowir -O0`; the
supplied native backend also uses `-O0`. Compiler ELF `.text` is measured directly:
2,090,694 B stage base; 2,154,950 B entry; **2,155,654 B reviewed**. The cumulative
increase is 64,960 B (3.11%); this audit adds 704 B (0.033%). Native executables
are sectionless ELF: reported payload is an explicit code/data/EH proxy, not
an isolated `.text` claim. Native backend optimization remains a later owner.

## Results

Medians below are milliseconds; RSS is peak KiB. Full ranges, including scheduling
outliers, are in the raw data. Native payload values are A/B bytes.

| Workload | Compiler A/B ms | RSS A/B KiB | Runtime A/B ms | Payload A/B B | Compiler paired B/A range |
|---|---:|---:|---:|---:|---:|
| startup | 10.78 / 10.83 | 5752 / 5912 | 5.50 / 5.61 | 24 / 24 | 0.198–0.998 |
| auto-specializations-9600 | 1394.11 / 1433.57 | 106416 / 106680 | 5.78 / 5.61 | 768062 / 768062 | 0.915–1.060 |
| runtime-calls | 11.72 / 11.67 | 6032 / 6176 | 217.24 / 211.23 | 206 / 206 | 0.946–1.097 |
| runtime-memory | 10.81 / 11.21 | 5992 / 6092 | 129.64 / 127.60 | 434 / 434 | 0.983–1.079 |
| runtime-floating | 11.32 / 10.04 | 6016 / 6220 | 146.05 / 143.53 | 230 / 230 | 0.839–0.928 |
| runtime-reference-captures | 10.52 / 10.00 | 6012 / 6208 | 43.37 / 42.78 | 188 / 188 | 0.912–0.977 |
| fixed-rtti-800 | 199.09 / 178.41 | 22204 / 21080 | 5.21 / 5.03 | 176760 / 176760 | 0.809–0.916 |
| fixed-rtti-3200 | 846.24 / 745.15 | 70708 / 68160 | 5.61 / 5.36 | 704760 / 704760 | 0.812–0.914 |
| list-specializations-3200 | 1067.03 / 1021.01 | 82620 / 82552 | 5.67 / 5.82 | 588862 / 588862 | 0.949–0.995 |
| capture-specializations-3200 | 980.25 / 956.43 | 78576 / 78540 | 5.50 / 5.55 | 339262 / 339262 | 0.982–1.014 |
| runtime-scalar-lists | 12.46 / 12.08 | 6092 / 6192 | 249.24 / 245.44 | 307 / 307 | 0.958–0.987 |
| runtime-class-lists | 13.07 / 12.58 | 6124 / 6188 | 112.37 / 113.83 | 1664 / 1664 | 0.843–1.120 |
| runtime-typeid | 11.90 / 12.07 | 6124 / 6168 | 47.33 / 47.66 | 600 / 600 | 0.928–1.051 |
| runtime-cast | 11.98 / 11.82 | 6092 / 6252 | 78.01 / 78.57 | 936 / 936 | 0.933–1.031 |
| runtime-copy-prefix | 11.80 / 11.48 | 6124 / 6212 | 122.28 / 134.22 | 1952 / 2104 | 0.881–1.035 |
| composition | 14.68 / 14.26 | 6368 / 6440 | 4.69 / 4.71 | 2760 / 2760 | 0.942–1.009 |

Fifteen of sixteen rows have **byte-identical LowIR and native executables**.
Their runtime noise cannot be attributed to a code change. Startup and short
compile timings are diagnostics: startup compiler pairs include 0.198/0.213,
and runtime pairs include 8.778/5.775; these outliers are retained, not discarded.
The 9600-specialization compile A/A range is 1384–1547 ms; ABBA ratios are
0.915, 1.033, 1.049, 1.060. Its median drift is +2.8%, RSS +264 KiB, and semantic
expression/body-transition counts are unchanged. This does not establish an
avoidable work regression beyond the observed noise. No whole-stage speedup is
claimed. Calls, memory and floating-point results remain checked and live.

## Fixed RTTI work

At 800 specializations, RTTI expression records fall **2400 → 3**, semantic
expression work **16010 → 9617**, while LowIR stays **29634 instructions**.
At 3200, records fall **9600 → 3**, semantic expression work **64010 → 38417**,
and LowIR stays **118434 instructions**. Twelve fixed expressions and their
conversions remain shared instead of being re-established for each body.

Compiler medians improve **10.4% / 11.9%** respectively. All four pairs at each
scale favor B: 0.916, 0.809, 0.881, 0.897 and 0.812, 0.877, 0.901, 0.914.
A/A ranges are 197.8–207.4 ms and 814.0–947.3 ms. RSS falls 1124 / 2548 KiB.
The combination of repeatable paired results at both sizes, lower recorded work
and identical output supports a compiler-work benefit, not a native optimization
claim. At 4× demand, reviewed time grows 4.18×, RSS 3.23× and emitted work about
4×; fixed RTTI records remain three.

The current 3200-list and 3200-capture rows retain exactly their entry plans,
objects/edges and instruction counts (147205 and 67205). The earlier
[102](performance102.md), [103](performance103.md), [104](performance104.md)
curves also cover pointer-name output size, captured field/array counts and
scalar/class list lengths. All frozen hashes were reverified in the review
manifest; measurements are preserved rather than replaced by favorable samples.

## Required cleanup cost

The new prefix-unwind controls demonstrate incorrect entry behavior, so no
performance benefit is claimed against that baseline. On the checked normal-path
loop, the repair changes payload **1952 → 2104 B** and median runtime
**122.28 → 134.22 ms** (+9.8%); paired ratios are **1.117, 1.102, 1.095, 1.143**.
A/A is 117.9–120.4 ms, so this cost is repeatable and disclosed.

[LowIR differences and both native disassemblies](../student.tests/pa21/audit105-prefix-cost.json)
locate the cost. The same four-byte member copy still lowers to the same load and
store (0x4004f0/0x4004f3). The corrected copy function additionally installs the
required `eh_cleanup` before the potentially throwing selected member copy and
retires it after success; its exceptional edge calls the first member destructor
then resumes. Zero-offset typed projections add no native instruction. The
supplied freestanding backend grows the frame by 16 bytes and registers an
80-byte handler record (0x4004f6 onward), saving the handler/stack/register state
on the normal path. Cold cleanup code accounts for the additional destructor
call. There is no new constructor call, copied source tree, duplicated copy loop
or optional transform to remove.

The selected user-provided copy has no nonthrowing specification. O0 therefore
retains the conservative exceptional edge. Removing it requires a separate body
effect proof or a different native EH implementation, not omission of completed
subobject cleanup. Native allocator/exception-runtime improvements are owned by
later stages. The external-throw controls verify actual failure cleanup rather
than timing only a nonthrowing surrogate; host code compiles only the driver,
while this compiler supplies the tested source-generated LowIR.

## Acceptance

**Stage-scoped performance acceptance passes for this checkpoint.** The repair
removes duplicate semantic work and an illegal prefix transformation. The required
cleanup cost is measured and explained; no avoidable added loop/call was found.
PA21/O0 has no optional new optimization pass and no handout numerical timing,
RSS, runtime or text ceiling. Spec §9 still mandates consumed/produced-work
bounds, complete fact keys and bounded conservative fallbacks; the eight-element
array expansion cap is preserved. Runtime inputs are volatile/dynamic and their
results checked, so live loops are not dead or constant-folded workloads.

Inherited +15%, +16 MiB and 5.5× thresholds remain self-selected diagnostics,
as already recorded by PA18–20, not exit gates. All historical observations,
mandated limits, correctness and coverage remain. Own native optimization and
self-hosting are not implemented PA21 surfaces. None of these measurements waives
the 45 outstanding required PA21 comparisons.
