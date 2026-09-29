# PA20/O0 checkpoint audit 97 — performance evidence

Reviewed code: `882cf5236a8ddb756403105cd50135440920e2a4`.
[Harness](../student.tests/pa20/audit97_benchmark.py),
[accumulated-range observations](../student.tests/pa20/audit97-performance-range.json),
[checkpoint/fix observations](../student.tests/pa20/audit97-performance-fix.json),
[corrected sequencing workload](../student.tests/pa20/audit97-performance-sequencing.json).
Handoffs [94](performance94.md), [95](performance95.md), and [96](performance96.md)
and all their original measurements remain preserved.

## Frozen protocol and boundaries

Compiler flags: `--emit-lowir -O0`; host build: `g++ -std=gnu++11 -Wall -O3`
with the course test-runner support. All three snapshots use the same host
build configuration. The harness records full compiler/source/backend hashes,
input text/hashes, output hashes, CPU affinity (CPU 1), wall time, peak RSS,
user/system time, context switches, checked exits and work counters. Timed
compilation has neither `--stats` nor `--validate-lowir`; their separate output
is checked against ordinary output. No build or correctness suite ran alongside
these measurements. External scheduling noise is retained, not filtered.

Comparable compiler and executable workloads have one warmup per binary,
four A/A observations, and four ABBA blocks. New/repaired behavior has six
final-only observations after warmup. Compilation and execution are measured
separately. Live loops use volatile limits and checked dependent sums; template
executions and startup rows are startup-dominated controls, not runtime profit.
Compiler size is exact `.text`. The supplied native backend emits a sectionless
ELF; its payload size includes code and data and is explicitly a proxy, not an
exact `.text` measurement. Student native encoding/optimization/debug and
self-hosting belong to later assignments.

| Snapshot | Commit | SHA256 | Compiler text bytes |
|---|---|---|---:|
| Stage base | `a9b24ab6` | `1a181c95ee97d646d41c1009780200049e776d67e1d715840d5526b3d10e1d40` | 2005958 |
| Audit entry | `7eca0738` | `6678cf1f0c551ec5277df1ae913d9cc4e6cf1ef1fe741fae63b0439eb306bbb1` | 2063494 |
| Reviewed | `882cf523` | `b2e3c9e77aa370b0c716786b3a38d4a8673a589fe407d1d5b60af4f8e3ddbb73` | 2065734 |

The measured working-source hashes equal the validated source hashes and the
committed code tip. Growth is **59776 bytes (2.98%)** over stage base and
**2240 bytes (0.109%)** for this audit fix. The 23 accumulated-range workloads
retain **696 observations / 76 warmups**; nine checkpoint/fix workloads retain
**360 / 36**; the sequencing workload adds **12 / 2**. Total **1068 / 114**.

After completing those 23 range workloads, the initial harness rejected a new
personal workload before timing: its handwritten expected sum was 43264,
but 500000 cycles of 1+...+8 modulo 65536 is **43136**. The
[attempt/proof](../student.tests/pa20/audit97-benchmark-note.json) is preserved;
only that personal expectation was corrected and measured separately. No
course test or earlier measurement was altered. The sequencing JSON also
records an explicit correction to its default entry-commit metadata, verified
against the actual frozen checkpoint binary hash; all observations are intact.

## Accumulated stage comparison

A is stage base; B is reviewed code. Times are medians in milliseconds; RSS
is maximum observed KiB from the ABBA samples (six samples for B-only rows).
All actual measured programs return the checked result. Rejected baseline
inputs are B-only; equivalent execution alone does not certify that the old
LowIR met every required output-shape rule.

| Workload | Compiler A/B ms | Peak RSS A/B KiB | Runtime A/B ms | Native payload A/B bytes | Paired compiler B/A |
|---|---:|---:|---:|---:|---|
| startup | 6.20 / 6.16 | 5676 / 5864 | 3.14 / 3.16 | 24 / 24 | 1.013, 0.983, 1.010, 0.983 |
| auto-specializations-2400 | 159.64 / 164.61 | 30828 / 31164 | 3.32 / 3.40 | 192062 / 192062 | 1.050, 1.028, 1.255, 1.026 |
| namespace-variable-2400 | 142.75 / 141.81 | 24536 / 24744 | 3.30 / 3.26 | 24 / 24 | 0.990, 0.995, 0.975, 1.006 |
| auto-specializations-9600 | 649.71 / 710.55 | 105204 / 105904 | 4.84 / 5.17 | 768062 / 768062 | 1.027, 1.155, 1.056, 1.118 |
| namespace-variable-9600 | 547.73 / 551.39 | 81596 / 82020 | 3.63 / 3.82 | 24 / 24 | 0.968, 1.042, 1.003, 1.004 |
| runtime-calls | 6.74 / 6.64 | 5940 / 6248 | 124.43 / 124.07 | 206 / 206 | 1.024, 0.882, 0.982, 1.034 |
| runtime-memory | 7.13 / 6.97 | 5888 / 6148 | 74.95 / 74.09 | 434 / 434 | 1.012, 0.992, 0.945, 0.909 |
| runtime-floating | 6.84 / 6.73 | 6152 / 6160 | 86.47 / 86.63 | 230 / 230 | 0.956, 0.911, 0.989, 0.990 |
| new-array-pack | 6.96 | 6056 | 146.09 | 263 | B only |
| new-array-ranges-800 | 180.53 | 21984 | 4.19 | 140861 | B only |
| new-member-ranges-800 | 163.35 | 22760 | 4.33 | 136160 | B only |
| new-array-ranges-3200 | 733.34 | 73136 | 4.74 | 563261 | B only |
| new-member-ranges-3200 | 638.74 | 72940 | 4.72 | 544160 | B only |
| new-runtime-range-array | 8.43 | 6128 | 157.62 | 260 | B only |
| new-runtime-range-class | 9.16 | 6124 | 233.48 | 464 | B only |
| array-members-800 | 246.77 / 247.58 | 28868 / 29096 | 4.03 / 4.02 | 153661 / 176095 | 1.020, 1.020, 1.011, 1.011 |
| closure-entries-800 | 268.38 / 260.38 | 30264 / 31076 | 4.15 / 4.22 | 174462 / 158462 | 0.997, 0.957, 0.866, 0.963 |
| array-members-3200 | 694.54 / 760.81 | 96380 / 97892 | 3.63 / 3.78 | 614461 / 704095 | 1.008, 0.960, 1.121, 0.971 |
| closure-entries-3200 | 943.37 / 1071.04 | 108840 / 105932 | 3.61 / 3.56 | 697662 / 633662 | 0.970, 1.077, 1.130, 0.965 |
| runtime-pointer-closure | 7.52 / 7.02 | 6016 / 6116 | 51.51 / 42.77 | 258 / 208 | 0.969, 0.738, 0.754, 0.979 |
| runtime-immediate-closure | 6.51 / 6.34 | 5972 / 6088 | 45.48 / 47.96 | 192 / 200 | 0.990, 0.947, 1.001, 0.968 |
| runtime-array-members | 6.50 / 6.59 | 6028 / 6156 | 36.52 / 111.14 | 285 / 343 | 0.931, 0.983, 0.937, 1.186 |
| new-runtime-default-closure | 6.40 | 6092 | 1795.03 | 284 | B only |

Eight general-control executables (startup, both auto/namespace sizes, calls,
memory and floating) are byte-identical. Timing differences in those native
files cannot establish generated-code regressions. Across 9600 auto
specializations, required return deduction adds 9600 deductions/demands while
checking each of the 9601 bodies once and classifying just two placeholder
types. The +9.4% compiler median and paired ratios 1.027–1.155 are disclosed
semantic costs over the base's incomplete placeholder implementation, not an
optional optimization benefit. The common by-value path already avoids the
scratch substitution maps removed in handoff 94. The new helper proof does
zero work on this source. No percentage miss becomes a gate under spec §9.

Noise limits quantitative conclusions: that workload's A/A interval is
656.31–669.42 ms while B samples span 683.89–1122.58 ms. Array-members-3200
has A/A 980.25–987.93 ms but B 643.68–1000.40 ms; closure-entries-3200 has
A/A 1119.70–1134.43 ms but B 667.84–1103.54 ms. All paired ratios and raw
observations remain in the JSON; medians alone are not speedup claims.

Pointer-closure payload shrinks 258 to 208 bytes by removing the redundant
wrapper call. Current runtime ratios are **0.847, 2.493, 0.836, 0.823**;
the slow block is retained. Handoff 96 independently measured four pairs
**0.796–0.859**, with the same 50-byte reduction. That repeated benefit and
bounded body-entry demand support this change; the noisy current run does
not justify a new precise percentage claim. Immediate-call payload grows
192 to 200 bytes for required indirect-entry presentation, with no profit claim.

Array-member runtime grows **36.52 to 111.14 ms**, paired **3.034, 3.051,
3.031, 3.100**, and payload grows 285 to 343 bytes. At 3200 specializations,
payload grows 614461 to 704095 bytes. This is the previously documented
mandatory O0 member-array temporary/helper/copy representation required by
`100-aggregate-element-array-member-braced-init.ref`; the old semantically
successful output did not satisfy that shape. It is a real cost, not a speedup
or an optional losing transform. Required optimization of that native work
belongs to later stages; the comparison contract is unchanged.

## Checkpoint-to-fix comparison

A is the frozen 7eca0738 checkpoint; B is reviewed code. **All nine native
files and their payload sizes are byte-identical.** These controls isolate
compiler proof cost from the already-required aggregate/callable changes.

| Workload | Compiler A/B ms | Peak RSS A/B KiB | Runtime A/B ms | Native payload bytes | Paired compiler B/A |
|---|---:|---:|---:|---:|---|
| auto-specializations-9600 | 1397.97 / 1314.49 | 108440 / 108464 | 6.43 / 6.67 | 768062 | 0.887, 0.958, 0.993, 0.996 |
| array-members-800 | 270.52 / 262.31 | 31272 / 29024 | 4.49 / 5.22 | 176095 | 0.931, 1.074, 0.961, 0.997 |
| array-members-3200 | 1538.91 / 1357.27 | 97088 / 98292 | 9.68 / 10.24 | 704095 | 1.076, 0.837, 0.805, 1.025 |
| closure-entries-3200 | 1548.34 / 1507.91 | 106460 / 106580 | 3.46 / 3.47 | 633662 | 0.954, 0.984, 1.113, 0.773 |
| runtime-calls | 6.02 / 5.89 | 6108 / 6184 | 122.91 / 122.43 | 206 | 0.957, 0.994, 0.970, 0.986 |
| runtime-memory | 6.48 / 6.34 | 5936 / 6188 | 73.12 / 73.26 | 434 | 3.866, 0.962, 0.986, 0.979 |
| runtime-floating | 6.49 / 6.38 | 6172 / 6312 | 85.99 / 86.06 | 230 | 0.981, 0.973, 0.977, 0.986 |
| runtime-pointer-closure | 6.89 / 6.73 | 6084 / 6120 | 42.49 / 42.83 | 208 | 0.955, 1.013, 1.002, 0.970 |
| runtime-array-members | 8.60 / 8.50 | 5972 / 6156 | 130.69 / 127.77 | 343 | 0.990, 0.984, 1.004, 1.027 |

The 3200-array proof adds bounded metadata: peak RSS rises 97088 to 98292 KiB
(+1204 KiB, 1.24%). Lower RSS at 800 is retained but not attributed to the
new cache. Compiler timing is noisy: 3200-array A/A is 1036.46–1388.36 ms,
B spans 1094.36–2335.87 ms; closure-3200 A/A is 2447.82–2742.16 ms, B spans
769.05–2110.39 ms. A runtime-memory compiler outlier produces the 3.866 pair
(B reaches 43.84 ms versus a 6.34 ms median). No observations are dropped and
no broad compiler speedup is claimed. Source inspection/counters identify
required linear proof work rather than a new pass or repeated global analysis.

The repaired self-observing array loop fails its checked result at entry and
is therefore excluded from A/B comparisons. Final-only compiler median is
9.67 ms (9.21–9.89), peak RSS 6156 KiB; runtime is 32.85 ms (31.73–40.00),
payload 236 bytes. Seven focused entry reducers likewise fail and now pass.
There is no claim that being faster than a wrong program establishes profit.

## Facts, budgets and acceptance

- For 800/3200 array specializations, initializer-independence work is
  **6400/25600**; array representation classification is **1/1**, with
  **3202/12802** hits. Facts belong to checked occurrences/completed constructors
  or immutable canonical types, using TU-owned flat indexes. No body is demanded
  merely to improve a proof; unknown cases keep ordered initialization.
- Array-range proof work is 2400/9600 on those sizes; member-range proof work
  stays 4/4. Both record n range plans, one fixed recipe and n recipe uses.
  Array bodies check n+1 times, member bodies n+3. Repeated specialization calls
  do not reparse, reselect fixed endpoints or recheck completed bodies.
- Closures check **2n+1** bodies at n=800/3200, with n return deductions and
  two placeholder classifications. Separate object/pointer emissions share
  semantic bodies/statics and own fresh LowIR slots/labels. Growth is at most
  two demanded bodies per closure plus its small conversion entry.
- Helper/source proof work is bounded by requested graph nodes, constructor
  actions and argument edges, once per complete key. There is no fixed point
  or pipeline restart. Helper emission is O(fields), ordinary lowering tracks
  produced IR, existing initialization expansion limit **8** is unchanged, and
  large omitted tails retain their bounded paths. No new optional optimizer,
  inlining or unrolling budget is introduced. Telemetry does no extra analysis.

**Stage-scoped acceptance: passes for this checkpoint.** Necessary semantic
and mandated O0 representation costs are disclosed; profitable wrapper removal
has repeated evidence; the correctness fallback has measured compiler cost and
preserves comparable native output. No unprofitable optional transform was
added. Historical latency/RSS/size percentages are diagnostic observations,
not extra exit gates; no measurement is deleted and no mandated work/growth,
correctness or coverage requirement is weakened. This conclusion does not
complete the 23 remaining PA20 implementation cases or permit advancement.
