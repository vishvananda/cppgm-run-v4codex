# Performance176 — PA29 / O0

This change supplies required declaration, demand and lifetime semantics. No optional
optimization pass or speedup is claimed. The entry compiler cannot produce a correct
executable for the affected workload; those rejected compilations are retained.
Spec §9 therefore permits equivalent comparisons on common correct workloads and
scaling/correct-output evidence for the new capability.

## Frozen protocol

The [manifest](../student.tests/pa29/evidence176/manifest.json) freezes entry/final
binaries, hashes and scripts; final implementation is `1b19e9ac`.
Both compiler and executable timing use wall time separately from host linking.
The [common raw evidence](../student.tests/pa29/evidence176/common-performance.json)
contains AAAA noise calibration and six ABBA blocks per workload/mode (224 samples).
Flags are `-O0 -c --stats`; the inherited inputs contain 2,400 demanded templates
plus checked loops/calls/memory, floating point, exceptions or unused functions.
All samples, outliers, RSS, phase counters, input/image hashes and linker version
are preserved. No affinity or hardware-counter dependency was introduced.
Builds and correctness suites finished before measurement; external scheduling
and concurrent documentation activity were not controlled.

| Workload | Compile A/B median s | Compile paired B/A [range] | Compiler peak RSS A/B KiB | Runtime A/B median s | Runtime paired B/A [range] | Executable text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1704/0.1772 | 0.9999 [0.8326–1.1611] | 29508/29824 | 0.0523/0.0524 | 0.9903 [0.9668–1.0114] | 151633/151633 |
| floating | 0.1660/0.1672 | 1.0154 [0.7160–1.1255] | 29620/29968 | 0.0481/0.0482 | 1.0045 [0.9899–1.0233] | 151474/151474 |
| exceptions | 0.1767/0.1753 | 0.9957 [0.8923–1.3585] | 29544/29436 | 0.2613/0.2637 | 1.0117 [0.8987–1.0829] | 151781/151781 |
| pruning | 0.2074/0.2053 | 0.9948 [0.8532–1.1440] | 35192/35768 | 0.0513/0.0513 | 0.9953 [0.9910–1.0071] | 151633/151633 |

| Workload | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| memory | 0.1635–0.2201 | 0.0514–0.0532 |
| floating | 0.1627–0.1898 | 0.0478–0.0481 |
| exceptions | 0.1637–0.1999 | 0.2536–0.2950 |
| pruning | 0.2008–0.2159 | 0.0505–0.0524 |

Executable text is unchanged on every common workload. Paired compiler medians
range from 0.9948 to 1.0154 and runtime medians from 0.9903 to 1.0117; their spreads
and calibration do not establish a repeatable regression or speedup. Peak compiler
RSS grows at most 1.64% here. These are observations, not a new percentage gate.

## Affected semantics and scaling

[All affected observations](../student.tests/pa29/evidence176/declaration-performance.json)
contain eight compiler and eight runtime samples at 600/1200/2400 specializations
(48 samples). Each class has an excluded member body and an excluded inline static
object initialized by a non-constexpr call. An extern-template class declaration
coexists with the required local body/data demand. Runtime checks initialization
counts and computes 24 million member calls from a `strtol` seed and varying loop
inputs. Every run checks its printed checksum against independent Python arithmetic.
Flags are `-std=c++11 -O0 -c --stats`, and the runtime seed is `17`.

| Specializations | Compile median [range] s | Compiler peak RSS KiB | Runtime median [range] s | Runtime peak RSS KiB | Executable text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | 0.1114 [0.1101–0.1228] | 21484 | 0.1285 [0.1275–0.1345] | 1756 | 82177 |
| 1200 | 0.2329 [0.2251–0.2562] | 35500 | 0.1340 [0.1293–0.1364] | 1792 | 163777 |
| 2400 | 0.4828 [0.4632–0.6462] | 63136 | 0.4508 [0.4410–0.4760] | 2136 | 326977 |

Launcher median is **0.00319 s**, range **0.00310–0.00347 s**. The shortest compiler
and runtime medians exceed 20× that median. At the same total call count the
2,400-function program runs materially slower than the 1,200-function program.
This is disclosed as an O0 generated-code limitation; no cache/profiler attribution
or runtime improvement is claimed. Entry-to-final affected ratios are invalid
because entry rejects these required semantics.

[All-sample counter checks](../student.tests/pa29/evidence176/scaling.json) prove
**N initializer computations**, **N completed-fact hits**, **28N+339 parsed nodes**,
**103N+339 occurrence nodes**, **23N+56 instructions**, and **136N+337 native text
bytes** in all 24 compiler samples. Executable text is **136N+577 bytes**.
Work and storage follow declarations, demanded initializers and actual emitted
operations. No unused member initializer is computed merely to complete a class.
Controls also verify that constant-only queries emit no inline member objects and
`--stats` does not change emitted object bytes.

## Acceptance and budgets

Optional optimization work and growth budgets remain **zero**. Required dynamic
objects add one eight-byte initialization guard; required temporary destructor
registration adds its own eight-byte guard. These identities and emitted helpers
are linear in demanded objects/lifetime actions, with no optional whole-program
pass or fixed-point search. The new facts have one owner and one terminal result.
Necessary semantic costs are distinguished from later optimizer/allocation and
heavy hosted-runtime work. No avoidable regression was established on equivalent
correct inputs. The stage requires these semantics regardless of speedup.

The inherited unsupported blanket **15% latency/RSS** and **zero-growth** targets
remain diagnostic, following [performance175](performance175.md) and spec §9.
No measurements or mandatory limits were removed: generated elements remain
**1,048,576**, constexpr steps **1,000,000**, call depth **512**, packed source-site
capacity **2^31−1**, native frame/data capacity **0x70000000**, alignment **4096**,
and all course timeouts. PA30–34 retain heavier hosted-runtime, optimization and
self-hosting work. Correctness and coverage remain mandatory.

Reproduce with the manifest’s frozen binaries:

```sh
python3 student.tests/pa27/performance147_common.py OUT_COMMON ENTRY FINAL
python3 student.tests/pa29/performance176.py OUT_AFFECTED ENTRY FINAL
python3 student.tests/pa29/test176.py OUT_CONTROLS FINAL
python3 student.tests/pa29/inspect176.py OUT_INSPECTION
python3 student.tests/pa29/validate176.py OUT_GATES
```
