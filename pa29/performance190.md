# PA29 audit190 performance and stage acceptance

Code tip: `5aaf16d15f8e50925c0b75a5485893a958501b85`.
All measurements precede that commit and bind to the identical validated source
and frozen B binary; [source binding](../student.tests/pa29/evidence190/source-binding.json)
and [recomputed verification](../student.tests/pa29/evidence190/performance-verification.json)
record the hashes. No build/test job overlapped the timing sequence. CPU affinity
is CPU 0, flags `-O0 -c --stats`, host link `g++`. Host compilers only link or
corroborate independent controls; they do not implement compiler output.

## Protocol and comparison scope

There are **656 new observations plus 16 launcher observations**. Compilation
and execution are timed separately with wall time and `/usr/bin/time` peak RSS.
The four common and six inherited owner workloads each use four A/A calibration
samples followed by six ABBA blocks, for both compilation and execution. Every
status and sample, phase counter, paired mean ratio, median and spread is retained.
All ten equivalent pairs have **byte-identical objects and executables**.

- [Common, 224 observations](../student.tests/pa29/evidence190/common-performance.json):
  A is last reviewed `2df00585`, SHA-256
  `a4022c8d9e1a288e70aa4010d1a5d574614d35575f319b56ed088865e0d3c785`.
  Covers memory/loops, floating point, exception/call traffic and dormant-template pruning.
- [Inherited owners, 336 plus eight launchers](../student.tests/pa29/evidence190/existing-owner-performance.json):
  A is audit entry `bac893d4`, SHA-256
  `9d36f0bbcb3ddccc57853370d8003f6103eadbfcd0fe50a171994fccab950294`.
  Covers definition/demand and assertion/specifier contexts at N=600/1200/2400.
- [Repaired owners, 96 plus eight launchers](../student.tests/pa29/evidence190/owner-performance.json):
  eight compile and eight runtime samples per family/size. Entry A fails every
  input; these are **corrected-only measurements**, not a speedup comparison.
  Entry failures and exact generated sources are retained. Clang compiles/runs
  the same inputs as an additional behavioral control.

B in every dataset is SHA-256
`98bbbe6a9acc9f606c564a19c1cba5efae7b52d356f6692439fe62b7980b12b5`.
Inputs and all available frozen binaries were rehashed. ABBA order and paired
ratios were independently recomputed. Common sources come from
`performance147_common.py`; the other generators retain source text in JSON.

## Equivalent implementations: all four dimensions

Times below are medians in milliseconds; RSS is the maximum compiler KiB across
samples. Ratios are median paired B/A, with the complete paired range. Text is
linked executable `.text` bytes, equal in A/B. Full raw A/A and sample ranges,
runtime RSS, object sizes and hashes are in the datasets above.

| Workload | Compile A → B ms | Compiler RSS A → B KiB | Compile paired [range] | Runtime A → B ms | Runtime paired [range] | Text A=B |
|---|---:|---:|---|---:|---|---:|
| memory | 165.44 → 166.09 | 29856 → 30208 | 1.0072 [0.8285, 1.0127] | 52.77 → 52.63 | 0.9979 [0.9919, 1.0049] | 151,633 |
| floating | 165.93 → 165.84 | 29664 → 29872 | 0.9979 [0.9944, 1.0026] | 48.90 → 49.08 | 1.0024 [0.9944, 1.0046] | 151,474 |
| exceptions | 165.58 → 166.22 | 30036 → 30248 | 0.9960 [0.5380, 1.0177] | 250.89 → 251.69 | 1.0083 [0.9882, 1.1831] | 151,781 |
| pruning | 206.51 → 208.39 | 35752 → 35912 | 1.0092 [0.6402, 1.2070] | 52.42 → 52.42 | 0.9998 [0.9949, 1.0057] | 151,633 |
| demand600 | 148.70 → 151.97 | 26232 → 26496 | 1.0067 [0.9912, 1.0254] | 139.60 → 138.43 | 0.9909 [0.9742, 1.0073] | 825 |
| demand1200 | 386.88 → 386.85 | 44716 → 44920 | 0.9928 [0.6757, 1.1596] | 147.49 → 147.64 | 0.9791 [0.9393, 1.0731] | 825 |
| demand2400 | 662.27 → 647.37 | 81452 → 81636 | 0.9665 [0.9111, 1.0505] | 139.44 → 140.28 | 1.0228 [0.9703, 1.1067] | 825 |
| contexts600 | 107.51 → 106.61 | 14268 → 14580 | 0.9869 [0.7677, 1.1200] | 148.80 → 150.97 | 0.9974 [0.9342, 1.0426] | 825 |
| contexts1200 | 137.48 → 136.20 | 21136 → 21344 | 0.9729 [0.8861, 1.1754] | 164.64 → 150.82 | 0.9675 [0.4606, 0.9937] | 825 |
| contexts2400 | 256.12 → 248.70 | 34616 → 34892 | 0.9298 [0.5048, 1.1389] | 139.58 → 142.32 | 0.9588 [0.8758, 1.1293] | 825 |

Every compiler paired range crosses unity. Common compiler paired medians are
0.9960–1.0092; the owner medians are 0.9298–1.0067. Compiler RSS increases by
160–352 KiB on common workloads and 184–312 KiB on inherited owners. These costs
are disclosed, not discarded under the noise label. No repeatable avoidable
latency regression or speedup is established. The apparent runtime improvement
on contexts1200 occurs with exactly identical executable bytes; it is scheduling
variation, not an optimization result. Outliers (including a common A compile
at 451 ms and owner samples with broad ranges) remain in the records.

The inherited-owner launcher median is 4.55 ms, range 4.32–38.19 ms; repaired-owner
launcher median is 5.14 ms, range 4.92–5.41 ms. Common steady runtime is roughly
49–252 ms, inherited-owner runtime 136–151 ms on B; the live workload dominates
ordinary startup. No timing is selected or subtracted to manufacture a gain.

## Repaired-owner work and runtime

`identityN` emits 2N named complex objects with N unique complete typed values,
checks both real and imaginary components through a pointer table and sums all
imaginary parts. Its cache must distinguish a common real component with
N different imaginary identities. `listsN` checks N fixed braced conditions in
undemanded template definitions; no runtime list materialization is needed.

Both families then execute two million complex rotation/add steps with seed
read from argv, accumulate both components, and throw/catch the complex value
through a demanded template every hundred iterations. Python computes an
independent bounded-integer recurrence for the expected result. Runtime work
cannot disappear through folding a constant return; every sample verifies it.

| Workload | Compile median [range] ms | Compiler RSS KiB | Runtime median [range] ms | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| identity600 | 80.61 [79.17, 84.04] | 12588 | 136.06 [127.38, 151.38] | 3704 | 1,899 |
| identity1200 | 142.51 [98.09, 145.01] | 17968 | 161.36 [133.08, 173.31] | 3740 | 1,899 |
| identity2400 | 173.84 [160.74, 293.76] | 28232 | 132.01 [128.01, 168.24] | 3972 | 1,899 |
| lists600 | 46.55 [37.48, 57.35] | 11060 | 134.25 [130.34, 153.37] | 3644 | 1,711 |
| lists1200 | 73.98 [66.29, 90.04] | 14180 | 134.52 [128.16, 171.06] | 3664 | 1,711 |
| lists2400 | 129.01 [121.34, 166.84] | 20540 | 136.18 [129.38, 142.82] | 3692 | 1,711 |

The constant-data cache publishes **600/1200/2400 facts**, despite 2N objects;
key storage capacity is **24,576/49,152/98,304 bytes**. It compares the full typed
64-bit payload on a hash collision. Fixed-list work is **N constant activations,
5N interpreter steps, 2N list plans, N list fields and zero runtime list objects**.
Tokens are 12,874/25,474/50,674 (identity) and 12,838/25,438/50,638 (lists).
These observations support work/storage proportional to distinct facts and
source size. Lists produce identical objects at every N; identity data grows as
required while text stays constant. This is bounded correctness work, with no
new optional transform and no profitability claim against invalid A behavior.

## Inherited evidence and legality/budget disposition

[Historical verification](../student.tests/pa29/evidence190/historical-performance.json)
retains **2,936 observations plus 56 launchers across 14 datasets** from handoffs
187–189, including their preliminary misses and corrected-only measurements.
Their binary/input hashes, ABBA ordering and paired ratios were checked; their
[manifests were checked at the record commits](../student.tests/pa29/evidence190/historical-manifests.json).
Earlier manifests naturally bind the earlier plan contents. They are not
invalidated by updating the current compact plan. No historical samples were
removed, and this report does not replace them with selected recent samples.

The useful optimization fact here is the complete, canonical constant payload:
only equal type/payload/validity may share a static-data fact. Unequal imaginary
components and signed zero retain distinct storage contents. Recorded list
conversions are executed once for their demand, without overload replay or
runtime-temporary construction. Complex arithmetic preserves admitted exceptional
inputs, while finite overflow and zero division still fail constant evaluation.
ABI returns/catches retain typed storage and ordinary COMDAT RTTI identity.
These are semantic requirements at every optimization level, not optional
transformations needing a speculative profitability waiver. O0/O2 controls pass.

The unchanged forced-inline owner checks fixed arity, recursion/depth ≤64,
stack/varargs and exception-region legality before reserving work. It caps each
caller at 262,144 work units and the whole preparation at 4,194,304; a missing
proof or exhausted budget retains a valid call. Source and explicit LowIR enter
that owner once. No work/growth budget was reset or widened. Constexpr retains
1,000,000 steps and depth 512; native frame/data bounds, alignment restrictions,
course timeouts, debug/unwind obligations and all coverage remain unchanged.

Under **spec §9 stage-scoped acceptance**, inherited blanket 15% latency/RSS and
zero-growth targets remain diagnostic, not additional PA29 exit gates. Necessary
correctness storage/RTTI costs are measured above. Broader hosted builds, hosted
runtime, optimizer and allocator objectives remain owned by PA30/31/32/33, with
PA34 inception still required later. No profiler or allocator-specific diagnostic
is introduced as an exit criterion. Correctness, mandated limits, comparison
rules and course coverage have not been weakened. Five PA29 failures still count;
full PA29/root-through success is required before advancing.
