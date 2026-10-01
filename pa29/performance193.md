# PA29 implementation193 performance and acceptance

Code tip: `60db24f6`. [Source binding](../student.tests/pa29/evidence193/source-binding.json)
and [recomputation](../student.tests/pa29/evidence193/performance-verification.json)
pin exact binaries, inputs, flags, images and all raw observations.

Entry SHA-256: `88661fad89ec2e9caf03996e1516f0b5098f1bd6a411ae54664332d05cd6ec14`.

Final SHA-256: `3ff6daafb3ff6c4dd23f6445619f78e3d169e1e90589800cccf1615a08c5c689`.

## Protocol and equivalent common workloads

**440 observations plus 16 launchers** are retained. Frozen binaries run on CPU 0,
with O0, object emission and `--stats`; g++ performs only host linking. The owner
series also explicitly selects C++11. No build, correctness suite or other
benchmark overlaps timing. This shared host is not isolated. Compilation and
execution are measured separately; no outlier is discarded and no launcher time
is subtracted. Controls independently prove stats-on/off object equivalence.

The [common dataset](../student.tests/pa29/evidence193/common-performance.json)
uses the unchanged, hash-checked inherited memory/loop, floating, exception/call
and dormant-definition pruning inputs, each with 2,400 demanded specializations.
Each mode uses four A/A observations and six ABBA blocks. All four A/B object
and executable pairs are byte-identical, including text. Runtime checksums
consume live inputs. No optimization benefit is claimed.

Comparison medians and RSS use the ABBA samples; A/A observations remain
separate calibration. Times are median milliseconds, RSS is maximum compiler KiB, ratios are paired
B/A medians with all block extrema. Raw data retains individual ranges, A/A
calibration, runtime RSS, phase counters and object/executable sizes.

| Workload | Compile A → B ms | Compiler RSS A → B KiB | Compile ratio [range] | Runtime A → B ms | Runtime ratio [range] | Executable text bytes |
|---|---:|---:|---|---:|---|---:|
| memory | 169.45 → 172.97 | 29712 → 29560 | 1.0592 [0.9967, 1.2023] | 52.71 → 53.48 | 1.0047 [0.9937, 1.0344] | 151633 |
| floating | 278.76 → 277.03 | 29800 → 29832 | 0.9938 [0.9847, 1.0016] | 56.88 → 56.80 | 0.9965 [0.9862, 1.0116] | 151474 |
| exceptions | 278.11 → 284.52 | 29272 → 29172 | 1.0584 [0.8595, 1.7482] | 316.70 → 310.67 | 0.9836 [0.8747, 1.1007] | 151781 |
| pruning | 219.74 → 218.77 | 35736 → 35604 | 0.9854 [0.7274, 1.2291] | 73.49 → 73.91 | 1.0039 [0.9937, 1.1857] | 151633 |

Every paired range crosses unity. Shared-host variation is substantial in the
exception and pruning series, including an exception compile ratio of 1.7482.
These observations establish neither a repeatable regression nor a speedup.
Runtime variation with identical executable bytes cannot be attributed to a
changed generated-code policy. All observations remain available.

## ABI-owner scaling and semantic costs

[performance193.py](../student.tests/pa29/performance193.py) generates retained,
source-hashed inputs in the [owner dataset](../student.tests/pa29/evidence193/owner-performance.json).
Each demands 256/1,024/4,096 nested class member definitions. All demanded results
contribute to a checked sum. A five-million-iteration recurrence calls one member
with an argv seed and checks its final value and accumulated sum, preventing
constant-folded or dead timing. Every object is checked for the expected number
of tagged/untagged symbols before timing its checked executable.

Repeated-tag definitions are correct equivalent A/B inputs: all three object and
executable pairs are byte-identical. Each mode again has four A/A samples and six
ABBA blocks. Necessary metadata selection is measured even when the resulting
ABI spelling is unchanged.

| Workload | Compile A → B ms | Compiler RSS A → B KiB | Compile ratio [range] | Runtime A → B ms | Runtime ratio [range] | Executable text bytes |
|---|---:|---:|---|---:|---|---:|
| repeated256 | 59.82 → 60.53 | 11852 → 11756 | 0.9797 [0.3910, 1.0350] | 69.24 → 69.21 | 0.9998 [0.9944, 1.0130] | 16350 |
| repeated1024 | 131.89 → 132.79 | 24328 → 24316 | 0.9897 [0.8514, 1.0694] | 69.62 → 70.01 | 1.0036 [0.9755, 1.0398] | 64734 |
| repeated4096 | 514.84 → 530.58 | 75644 → 76068 | 1.0065 [0.9384, 1.1634] | 69.44 → 69.35 | 1.0001 [0.9525, 1.0072] | 258270 |

All owner paired ranges also cross unity; the 256-member series contains an
A-side latency outlier (paired ratio 0.3910). It is retained and is not interpreted
as an optimization win. The 4,096-member compiler RSS difference is disclosed in
the table, including the linear snapshot-index cost.

Suppressed-tag definitions are corrected-only measurements, with eight compile
and eight runtime observations per size. Entry output has the wrong ABI policy
as established by the reduced matrix and required fixture. It is not a valid
performance baseline for these inputs. These rows measure cost, not speedup.

| Workload | Compile median [range] ms | Compiler RSS KiB | Runtime median [range] ms | Runtime RSS KiB | Executable text bytes |
|---|---:|---:|---:|---:|---:|
| suppressed256 | 57.15 [53.96, 59.45] | 11844 | 73.92 [68.57, 74.91] | 1764 | 16350 |
| suppressed1024 | 128.14 [124.44, 137.16] | 24320 | 68.79 [68.36, 69.17] | 1764 | 64734 |
| suppressed4096 | 535.58 [514.19, 1127.01] | 74948 | 70.14 [69.74, 136.42] | 1760 | 258270 |

Compile launcher median 8.18 ms [7.70, 9.34]. Runtime launcher median 5.47 ms [5.32, 5.98]. The measured owner work dominates startup; no startup adjustment is applied.

| Members | Definition signature checks | Definition applications / edges | Semantic fact records | Fact storage bytes | LowIR instructions | Native instructions |
|---|---:|---:|---:|---:|---:|---:|
| 256 | 1 | 256 / 256 | 5478 | 265488 | 2878 | 3658 |
| 1024 | 1 | 1024 / 1024 | 21606 | 1011264 | 11326 | 14410 |
| 4096 | 1 | 4096 / 4096 | 86118 | 3973536 | 45118 | 57418 |

The repeated/suppressed variants have the same work counts and text at each size.
One source-signature match serves every instance. Definitions, facts and code
scale with actual demanded members; there is no retry of unrelated prototypes.

## Stage-scoped acceptance and budgets

This is required semantic/ABI work, with no optional optimization pass or new
search. Two flat indexes add one record per selected source prototype and one
per instantiated member. A completed effective-tag lookup is O(1) average; each
attribute list is consumed only at publication/inheritance or name encoding.
For an index with at most n live records over its lifetime, 16-byte slots occupy at most
`16 * max(32, 4*(n+1))` bytes; during geometric growth, the old allocation is at
most half the new one and is immediately released. These TU-bounded facts do
not retain extra source, syntax, LowIR or MIR representations. Tag selection
adds **zero IR instructions and zero executable text** for equivalent programs;
all seven A/B image pairs verify the stronger byte-identity property.

No avoidable repeatable regression or optional gain is established. Necessary
metadata costs are bounded and disclosed. The inherited blanket 15% latency/RSS
and zero-growth diagnostic targets remain nonbinding under spec §9; they are not
new PA29 gates. Historical measurements in [performance192](performance192.md),
[performance191](performance191.md) and the [prior audit](audit.md) are preserved.

Mandated limits remain unchanged: one-million-step/depth-512 constexpr limits,
forced-inline depth 64/work 262,144 per caller and 4,194,304 per preparation,
native storage bounds and course timeouts. Later broad hosted/runtime,
optimization/allocation and self-host benchmarks remain with PA30–34. None of
these distinctions waives correctness, coverage or independent whole-stage
review. The current stage and through report now pass completely.
