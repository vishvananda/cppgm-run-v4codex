# PA29 implementation192 performance and acceptance

Implementation tip: `dca6f1f2`. The [source binding](../student.tests/pa29/evidence192/source-binding.json)
and [recomputation](../student.tests/pa29/evidence192/performance-verification.json)
bind exact inputs, images, flags and raw observations to the committed code.

Entry compiler SHA-256: `697a1cf4f6dcd402d60d756528aaffc7d15aaae3ed55c4287e71b62420e64eb3`.
Final compiler SHA-256: `88661fad89ec2e9caf03996e1516f0b5098f1bd6a411ae54664332d05cd6ec14`.

## Protocol and common workloads

The final run contains **320 observations plus 16 launcher calibrations**.
Both compilers are frozen. CPU affinity is CPU 0; flags are `-O0 -c --stats`,
with g++ host linking. No build/test job or other benchmark overlaps timing.
The shared host is not isolated. Compilation and checked execution are timed
separately. No outlier is discarded or launcher time subtracted.

The [four common workloads](../student.tests/pa29/evidence192/common-performance.json)
use the inherited fixed template-heavy memory/loops, floating-point,
exceptions/calls and dormant-definition pruning sources. Each has four A/A
samples and six ABBA blocks for each mode: 224 observations total. Inputs are
hash-checked against the inherited sources. Every timed program returns its
independently checked result. All four A/B object and executable pairs are
byte-identical, including text size.

Times below are median milliseconds; RSS is maximum compiler KiB. Ratios are
paired B/A medians with all block extrema. The raw data also retains individual
ranges, A/A calibration, runtime RSS, image sizes and all phase counters.

| Workload | Compile A → B ms | Compiler RSS A → B KiB | Compile ratio [range] | Runtime A → B ms | Runtime ratio [range] | Text bytes |
|---|---:|---:|---|---:|---|---:|
| memory | 169.30 → 171.79 | 29936 → 30148 | 1.0083 [0.9926, 1.2992] | 52.37 → 52.48 | 1.0033 [0.9943, 1.0123] | 151,633 |
| floating | 167.87 → 168.64 | 29540 → 29964 | 1.0092 [0.9999, 1.1671] | 49.48 → 49.38 | 0.9975 [0.9909, 1.0112] | 151,474 |
| exceptions | 169.69 → 169.93 | 30052 → 30252 | 1.0041 [0.9894, 1.7204] | 252.06 → 252.48 | 1.0019 [0.9932, 1.0453] | 151,781 |
| pruning | 211.94 → 214.85 | 35604 → 35724 | 1.0167 [0.9506, 1.2505] | 53.09 → 52.79 | 0.9912 [0.9677, 1.0119] | 151,633 |

All paired timing ranges cross unity. Medians are slightly higher for final
compilation, but this does not establish a repeatable latency regression or
speedup. Outliers remain visible (the exception compile ratio reaches 1.7204).
Compiler RSS rises by 212/424/200/120 KiB in these four pairs. The new semantic
carrier and bounded type inspection have measurable storage/work costs; none is
hidden as a supposed optimization benefit. Runtime variation with identical
executable bytes is measurement variation, not changed generated-code quality.

## Corrected-only scalar-format scaling

[performance192.py](../student.tests/pa29/performance192.py) generates six exact
sources, retained in the [owner dataset](../student.tests/pa29/evidence192/owner-performance.json).
Each demands 128/512/2048 distinct function specializations using half or quad.
All demanded calls contribute to a checked integer checksum. An additional
one-million-iteration recurrence uses a seed from argv, performs floating
addition/subtraction/multiplication and conversion, and checks every accumulated
result. O0 and live runtime input retain the work. The entry compiler rejects
all six sources; diagnostics/statuses are preserved. These are corrected-only
costs, not a speedup against invalid output. GCC independently compiles and runs
every source. Eight compile and eight runtime samples per source give 96
observations.

| Workload | Compile median [range] ms | Compiler RSS KiB | Runtime median [range] ms | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| half128 | 26.53 [26.14, 26.74] | 10,620 | 163.33 [162.49, 168.12] | 1,756 | 52,771 |
| half512 | 76.60 [75.67, 77.21] | 18,456 | 163.70 [163.30, 167.24] | 1,884 | 208,291 |
| half2048 | 290.19 [283.58, 293.40] | 50,392 | 164.55 [164.17, 171.39] | 2,460 | 830,371 |
| quad128 | 25.32 [24.61, 27.88] | 9,856 | 194.24 [193.17, 213.07] | 1,760 | 20,868 |
| quad512 | 70.84 [69.01, 76.18] | 17,084 | 195.07 [194.04, 202.29] | 1,760 | 81,540 |
| quad2048 | 257.65 [249.63, 276.62] | 43,956 | 197.72 [196.16, 204.36] | 1,900 | 324,228 |

Compile launcher median is 6.16 ms [6.05, 6.64]. Runtime launcher median is 4.50 ms [4.40, 4.71]. The smallest owner compilation still contains material work beyond startup;
checked owner runtimes exceed 160 ms. No corrected-only timing is interpreted
as an A/B optimization claim.

| Workload | Specializations / body transitions | Semantic fact records | Fact storage bytes | Input IR | Added by legalization | Helpers | Native instructions |
|---|---:|---:|---:|---:|---:|---:|---:|
| half128 | 128 / 128 | 2,931 | 114,104 | 2,112 | 1,678 | 5 | 8,830 |
| half512 | 512 / 512 | 11,379 | 452,088 | 8,256 | 6,670 | 5 | 34,942 |
| half2048 | 2048 / 2048 | 45,171 | 1,742,616 | 32,832 | 26,638 | 5 | 139,390 |
| quad128 | 128 / 128 | 2,931 | 114,104 | 2,112 | 388 | 6 | 4,439 |
| quad512 | 512 / 512 | 11,379 | 452,088 | 8,256 | 1,540 | 6 | 17,495 |
| quad2048 | 2048 / 2048 | 45,171 | 1,742,616 | 32,832 | 6,148 | 6 | 69,719 |

Template binding count stays five and binding work stays 23. Semantic facts,
input/output IR and text grow with demanded specializations. Helper identities
stay bounded at five for half and six for quad across the series. Half emits
more code because its operands are extended to binary32 and its result is
truncated; the baseline x86-64 target has no native half arithmetic requirement.
Quad arithmetic uses one platform helper per operation. These are necessary
representation costs, with at most six emitted operations per input operation,
not an optional transform requiring a claimed runtime win. Scratch/old pools
are released after preparation; ordinary formats allocate no legalization pools.

## Preserved preliminary observations and stage-scoped budgets

The [preliminary common run](../student.tests/pa29/evidence192/preliminary-common-performance.json)
retains 224 observations, and [preliminary owner run](../student.tests/pa29/evidence192/preliminary-owner-performance.json)
retains another 96 plus 16 launchers at `5369931a`. Its [source binding](../student.tests/pa29/evidence192/preliminary-source-binding.json)
and [validation](../student.tests/pa29/evidence192/preliminary-validation.json)
remain separate from the final compiler. Final IR review then found that an
isolated branch consuming a bit-initialized floating slot could bypass target
legalization. The [negative-zero reducer](../student.tests/pa29/evidence192/preliminary-slot-reproducer.json)
returns 1 before the repair and 0 in final controls. The corrected implementation
was revalidated and remeasured rather than silently rebinding old observations.

Preliminary common compile median paired ratios were memory 1.0044, floating 1.0070, exceptions 0.9995, pruning 1.0158; their ranges all crossed unity and their images were also identical. The
preliminary owner runs had wider runtime/compile spreads, retained in full.
Total new retained evidence is **640 observations plus 32 launchers**. All
[performance191](performance191.md), audit190 and earlier measurements remain.

Under current spec §9, inherited blanket 15% latency/RSS and zero-growth targets
are diagnostic rather than additional PA29 exit gates. This change supplies
required semantics and no optional optimizer. Ordinary-format output remains
identical on the paired controls; new-format work/growth stays linear and bounded
per operation. Necessary costs are disclosed, not compared with rejected entry
programs. No repeatable optional gain or avoidable regression is established.

Mandated limits remain unchanged: the one-million-step/depth-512 constexpr
budget, forced-inline depth 64/work 262,144 per caller and 4,194,304 per
preparation, native storage bounds and course timeouts. There is no new profiler
or allocator-diagnostic requirement. PA30–34 broader hosted/runtime, optimizer,
allocation and self-host obligations stay with their stages. The remaining PA29
ABI-tag failure still counts; full stage/root-through success and independent
review are required before advancement.
