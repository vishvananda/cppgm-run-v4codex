# PA29 checkpoint158 performance audit

The reviewed code tip is `1ab3499d7046daf5c298d958a8770b413edb3615`. PA29 O0 adds required semantic and lowering
capability, with no new optional optimizer and no speedup claim. This audit
measures both the accumulated stage and its alignment ownership correction.
[Earlier measurements](performance155.md), [scalar costs](performance156.md)
and [layout costs](performance157.md) remain unchanged.

## Frozen evidence protocol

[Manifest](../student.tests/pa29/evidence158/manifest.json) retains compiler and
script hashes, build flags, machine, CPU, affinity and raw-data hashes. Build
flags are `-std=gnu++11 -Wall -O3`; measured flags are `-O0 -c --stats`.
All measurements use CPU 0 and `/usr/bin/time` peak RSS. Host `g++` only links
these benchmark objects. Compiler processes and generated execution are timed
separately; every executable checksum passes before timing. No test or build
process overlaps timing; light review/bookkeeping and external scheduling remain
uncontrolled. All observations, including slow samples, are retained.

| Frozen compiler | Commit | Binary bytes | SHA256 |
|---|---|---:|---|
| stage_base | `2734e5c6` | 3807008 | `79a205ace278949bd6fa118eeb9e35b1ab59a61007fc21f18746a257b7e5f727` |
| entry | `76430506` | 3930992 | `84642180af8de176d1f8496122bf786e1b87935aeb1c37b233f74accfbfc36c3` |
| final | `1ab3499d` | 3935752 | `9ed4d6c3164faee5c4ba7f4478ca45a0cd318b28f051be5d5c88446b09dc1ff7` |

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py /tmp/pa29-158/cumulative /tmp/pa29-155/base /tmp/pa29-158/final
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py /tmp/pa29-158/audit-cost /tmp/pa29-158/entry /tmp/pa29-158/final
PERF_CPU=0 python3 student.tests/pa29/performance158.py /tmp/pa29-158/affected /tmp/pa29-158/entry /tmp/pa29-158/final
```

## Equivalent common workloads

Each comparison has 224 observations: four A/A observations, then six ABBA
blocks per workload and mode. Inputs retain the inherited checksums and 2,400
demanded templates; loops consume runtime `argc`. Paired ratios divide B's block
mean by A's. Times below are sample medians, RSS is maximum KiB; intervals retain
all paired blocks. Both comparisons produce byte-identical A/B objects **and**
executables for all four workloads. Runtime variation cannot indicate a code
improvement or degradation here. These are equivalent successful baselines.

### Stage base → reviewed code

[All observations](../student.tests/pa29/evidence158/cumulative-performance.json).

| Workload | Compile A / B s | Paired compile B/A [range] | Compiler RSS A / B | Runtime A / B s | Paired runtime B/A [range] | Text A = B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.15385 / 0.15767 | 1.0279 [0.8688, 1.0413] | 29012 / 29204 | 0.05287 / 0.05272 | 1.0005 [0.9852, 1.0104] | 151633 |
| floating | 0.15594 / 0.16108 | 1.0107 [0.7044, 1.0656] | 29172 / 29432 | 0.04916 / 0.04925 | 1.0027 [0.9940, 1.0208] | 151474 |
| exceptions | 0.15893 / 0.16362 | 1.0335 [0.5621, 1.0590] | 29556 / 29664 | 0.25157 / 0.25098 | 0.9968 [0.9940, 1.0159] | 151781 |
| pruning | 0.19202 / 0.19795 | 1.0282 [1.0164, 1.2993] | 34692 / 35444 | 0.05253 / 0.05250 | 0.9988 [0.9945, 1.0161] | 151633 |

| Workload | A/A compile range s | A/A runtime range s | Runtime peak RSS A / B KiB |
|---|---:|---:|---:|
| memory | 0.15141–0.15653 | 0.05230–0.05255 | 1760 / 1884 |
| floating | 0.15408–0.15917 | 0.04875–0.04993 | 1760 / 1764 |
| exceptions | 0.15923–0.16301 | 0.25063–0.27068 | 3932 / 3936 |
| pruning | 0.19088–0.19469 | 0.05243–0.05289 | 1764 / 1764 |

### Audit entry → reviewed code

[All observations](../student.tests/pa29/evidence158/audit-cost-performance.json).

| Workload | Compile A / B s | Paired compile B/A [range] | Compiler RSS A / B | Runtime A / B s | Paired runtime B/A [range] | Text A = B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.15681 / 0.16284 | 1.0279 [1.0071, 1.3044] | 29024 / 29256 | 0.05223 / 0.05232 | 0.9996 [0.9902, 1.0079] | 151633 |
| floating | 0.15797 / 0.16216 | 1.0296 [0.6321, 1.0474] | 29004 / 29556 | 0.04954 / 0.04963 | 1.0038 [0.9956, 1.1722] | 151474 |
| exceptions | 0.16465 / 0.16747 | 1.0273 [1.0069, 1.0369] | 28996 / 29520 | 0.26375 / 0.26365 | 0.9931 [0.9869, 1.0217] | 151781 |
| pruning | 0.22243 / 0.20526 | 1.0075 [0.6986, 1.1019] | 34704 / 35468 | 0.05320 / 0.05347 | 0.9972 [0.6517, 1.2881] | 151633 |

| Workload | A/A compile range s | A/A runtime range s | Runtime peak RSS A / B KiB |
|---|---:|---:|---:|
| memory | 0.15693–0.15970 | 0.05233–0.05248 | 1760 / 1760 |
| floating | 0.15475–0.16024 | 0.04958–0.04972 | 1756 / 1760 |
| exceptions | 0.16275–0.23703 | 0.26210–0.26923 | 3920 / 3928 |
| pruning | 0.20179–0.33388 | 0.05269–0.05434 | 1868 / 1760 |

Cumulative paired compiler medians are 1.0107–1.0335; the audit-only medians are
1.0075–1.0296. Some block ranges are noisy, but the small added cost is visible
in multiple workloads. It is disclosed, not called a speedup or hidden behind
a noise waiver. The storage fix adds a four-byte TypeId to shared expression
properties (Expression view 36 → 40 bytes), a sparse declaration map, and
constant-time propagation checks. Common-workload parsed nodes, source
occurrences, body transitions, query/candidate work and native instruction
counts are unchanged. No repeated specialization, new whole-graph walk or
optional pass explains this cost; the missing required storage fact does.
Peak compiler RSS rises by 108–752 KiB cumulatively and 232–764 KiB for the
audit comparison. Compiler binary growth is 128,744 bytes cumulatively and
4,760 bytes for the audit fix. The compact typed storage fact avoids both a
second expression graph and per-node allocation.

## Newly correct alias-storage workload

[All 48 observations](../student.tests/pa29/evidence158/affected-performance.json)
include six launcher calibrations. The entry binary rejects the layout assertion;
its failure latency is not an equivalent correct performance baseline. B has
eight compile and eight runtime observations at each size. A dependent class
retains an aligned `using` member alias and an overlapping empty member. Every
specialization contributes to a checked result; 60 million runtime calls use
`argc`, with an independently computed checksum.

| Classes/functions | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | 0.12948 [0.12666, 0.13153] | 24560 | 0.36176 [0.35340, 0.36741] | 1756 | 46130 |
| 1200 | 0.25309 [0.25064, 0.26269] | 41604 | 0.36065 [0.35404, 0.37707] | 1764 | 91730 |
| 2400 | 0.51342 [0.50816, 0.52863] | 75528 | 0.35719 [0.35366, 0.36790] | 1760 | 182930 |

Launcher median is 0.00436 s (0.00427–0.00515); minimum compilation exceeds it
29× and runtime 81×. Doubling demand approximately doubles compilation and
incremental memory. At 600/1200/2400 specializations, class completions, body
transitions, substitutions and empty-layout work are exactly 600/1200/2400;
query work is 1810/3610/7210, native instruction count 9053/18053/36053.
The separate [array-bound inspection](../student.tests/pa29/evidence158/inspection.json)
keeps empty-layout work at 2, retained positions at 3, fallbacks at 1 for
100/10,000/1,000,000 elements. These work counters describe scaling, not speedup.

[Native inspection](../student.tests/pa29/evidence158/native-inspection.json)
shows the actual O0 cost: `work<7>` has a 16-byte frame, stores its int at
`rbp-16` and its long at `rbp-12`, reloads them, adds offset 4 and returns.
The empty member causes no overlapping zero store. `storage.cpp` main has a
64-byte frame with three boolean merge homes; the alignment template has no
stack allocation. These loads/stores are not claimed optimized away.

## Stage-scoped acceptance and budgets

The current measurements support accepting the bounded cost of retaining
required semantics. There is no optional transform to justify by runtime profit
or remove. Newly introduced optional optimization work and code-growth budgets
are **zero**; required builtin expansions/layout/ABI output have input- and
demand-bounded costs described in [the architecture audit](audit.md).
Inherited address folding and private reload carrying keep their existing
legality checks, linear work and 64-instruction/three-carrier window. The
64-position empty-layout summary cap, constexpr million-step/depth-512 limits,
existing frame/offset/alignment constraints and course timeouts remain intact.

Historical blanket 15% latency/RSS and zero-growth targets are self-selected
diagnostics, not PA29 exit criteria under spec §9. All historical observations
remain; no correctness or mandated limit is waived. General O1–O3 allocation/
loop improvements belong to PA32/33, self-hosting to PA34. PA29 requires selected
hosted acceptance and lightweight interop, not broad PA30 headers or PA31 hosted
execution. The README explicitly excludes runtime vector lowering; its required
scalar `vector_size` width/layout and unused-wrapper validation remain open.
PA29 itself remains unfinished at 317/403; accepting these measurements does
not accept its remaining language or ABI failures.
