# PA28 handoff153 performance

Final implementation: `764721cd` (layout implementation `a5c53a88`).
Frozen binary SHA-256:

- A: `67e6bdd42e6185b8f483f2ca3796a35aafa18155ddc28a739431ba2cc2bdf7f2`
- B: `d472892b7a45a7cf5c62ba272e34d0636b78f6f8b1e8007fd83c036890f69ca9`

## Protocol and scope

[Common observations](evidence153/common-performance.json) use the unchanged
PA26/27 inputs: 2,400 demanded function templates with runtime loops, memory,
calls, floating point and exceptions, plus an unused-declaration workload.
CPU 0 affinity, `-O0 -c --stats`, four A/A observations and six ABBA blocks
per mode/input. Compilation and execution are separate; host linking is outside
timing. Runtime argc and checked checksums prevent dead workloads. No compiler
build or correctness suite ran concurrently.

[New primary input](evidence153/primary-input.cpp) demands 400 class/function
template instances with virtual primary sharing, covariant returns, ordinary
secondary bases, and secondary virtual dispatch inside base constructors.
It checks object size, pointer identity, construction dispatch and a checksum
over 3,000,000 runtime-controlled pairs of virtual calls/projections.
Entry A returns 2 because its layout is incorrect; its speed is not compared
with B. Twelve compile and twelve runtime observations record correct support.
[All final affected observations](evidence153/primary-performance.json) are retained.

## All four dimensions

Seconds are medians; RSS is maximum KiB. Common medians exclude A/A calibration.

| Input | Compile s A/B | Compile KiB A/B | Runtime s A/B | Executable text bytes A/B |
|---|---:|---:|---:|---:|
| memory | 0.8819 / 0.8007 | 29040 / 29184 | 0.1798 / 0.2442 | 151633 / 151633 |
| floating | 0.6144 / 0.5834 | 29140 / 29376 | 0.0874 / 0.0920 | 151474 / 151474 |
| exceptions | 0.5469 / 0.5591 | 28984 / 29344 | 0.8480 / 0.8325 | 151781 / 151781 |
| pruning | 0.3188 / 0.3700 | 34680 / 34856 | 0.0719 / 0.0723 | 151633 / 151633 |

| Input | Paired compile ratio (range) | Paired runtime ratio (range) | Compile A/A s | Runtime A/A s |
|---|---:|---:|---:|---:|
| memory | 1.001 (0.824–1.153) | 1.165 (0.519–3.577) | 0.3299–35.1715 | 0.1756–0.4070 |
| floating | 1.016 (0.872–1.231) | 1.028 (0.912–1.103) | 0.5402–1.5363 | 0.0921–0.1155 |
| exceptions | 1.023 (0.959–1.165) | 0.969 (0.894–1.076) | 0.5303–0.6336 | 0.7121–0.8213 |
| pruning | 0.962 (0.727–1.176) | 1.004 (0.995–1.024) | 0.6615–0.7754 | 0.0717–0.0740 |

New primary/secondary workload: compile **0.7322 s** (0.7084–0.9690), **68,188 KiB** peak;
runtime **0.1240 s** (0.1222–0.1272), **5,328 KiB** peak;
**1,009,166 executable text bytes**, 6,757,528 object bytes.

## Noise, work and acceptance

Scheduling noise dominates some observations. A/A includes a 35.17-second
compile outlier. A one-second [CPU sample](evidence153/cpu-calibration.json)
found every allowed CPU at least 84% busy and CPU 0 at 99%. No sample is
discarded. The 16.5% paired memory-runtime increase is disclosed despite
identical executable bytes; it cannot demonstrate a generated-code regression.
These measurements do not establish a precise speedup or slowdown.

[Byte comparisons](evidence153/comparison.json) prove every common object and
executable is identical between A and B, including instructions, frames and
spills. [Work counters](evidence153/counter-comparison.json) retain all non-time
differences. Common work is unchanged; the virtual-class sentinel storage
reflects the added typed metadata fields.

The change implements mandatory ABI semantics. No optional transform or
optimization benefit is claimed. Prefix/claim work follows actual graph
identities, cached path suffixes and emitted rows; physical table ordering is
O(v log v). TU-owned compact records and ID indexes release with the analyzer;
native state releases per function. Ordinary secondary vptr initialization
adds the required store per physical subobject. Added rows, VTT entries and
thunks are bounded by ABI facts. Common output retains zero code growth.

Spec §9 stage-scoped acceptance governs PA28 O0. Inherited blanket 15% latency/
RSS and zero-growth diagnostics were self-selected, not mandated exit gates.
No mandated work, frame, encoding or ELF limit is relaxed. No optimizer policy
is changed. PA32/33 profitability and PA34 self-hosting remain later-stage
obligations. Necessary ABI costs and scheduling noise do not create a new
PA28 gate; no optional unprofitable work is retained.

## Preservation and reproduction

All **520 observations** remain: 224 common + 24 primary before the final
secondary-store correction; 224 final common + 24 final simple-primary; and
24 final expanded primary/secondary measurements. The earlier simpler input
is [preserved](evidence153/intermediate-primary-input.cpp), along with its
[intermediate measurements](evidence153/intermediate-primary-performance.json),
[final measurements](evidence153/final-simple-primary-performance.json), and
[intermediate common measurements](evidence153/intermediate-common-performance.json).
The expanded input specifically measures the final base-entry store behavior.
Earlier evidence151/152 is untouched.

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py OUT_COMMON A B
PERF_CPU=0 python3 student.tests/pa28/performance153.py OUT_PRIMARY_SECONDARY A B
```
