# PA29 final audit194 performance

Reviewed code: `60c3c356`. [Source binding](../student.tests/pa29/evidence194/source-binding.json)
pins the tested dev tree, every implementation file and compiler SHA-256.
The [common](../student.tests/pa29/evidence194/common-performance.json) and
[owner](../student.tests/pa29/evidence194/owners-performance.json) datasets retain
**448 final observations plus 16 launcher calibrations**. Their exact binaries,
flags, source hashes, images, individual wall times, peak RSS, counters, A/A
samples and paired spreads are preserved. [Recomputation](../student.tests/pa29/evidence194/performance-verification.json)
checks all ABBA orders, ratios and frozen binaries. Eight A/B object and
executable pairs are byte-identical and every timed executable checks its result.

Each compilation/execution series has four A/A observations followed by six
ABBA blocks. Flags are `-O0 -c --stats`, CPU affinity is CPU 0, and g++ only
links the produced objects. No build, correctness suite or other benchmark
overlapped these measurements. The shared host is not isolated. Nothing is
discarded and startup time is not subtracted. Compiler launcher median is
6.09 ms [5.95, 6.44]; runtime launcher is 4.54 ms [4.31, 4.73]. Workloads
dominate those costs.

Times below are ABBA medians in milliseconds. Compiler RSS is maximum KiB.
Ratios are paired B/A medians and all block extrema. Text is equal in A/B.

| Workload | Compile A → B ms | Compiler RSS A → B KiB | Compile ratio [range] | Runtime A → B ms | Runtime ratio [range] | Text bytes |
|---|---:|---:|---|---:|---|---:|
| memory | 190.24 → 184.63 | 29,760 → 29,952 | .9725 [.7415, 1.1281] | 52.66 → 52.70 | 1.0009 [.9741, 1.0098] | 151,633 |
| floating | 171.32 → 170.22 | 29,852 → 30,004 | .9895 [.9750, 1.0201] | 49.53 → 49.47 | 1.0001 [.9961, 1.0104] | 151,474 |
| exceptions | 168.99 → 169.53 | 29,616 → 29,772 | .9999 [.6039, 1.0068] | 251.72 → 251.24 | .9934 [.9826, 1.0144] | 151,781 |
| pruning | 208.74 → 209.60 | 35,672 → 35,756 | 1.0022 [.6314, 1.0257] | 52.83 → 52.64 | .9969 [.9946, 1.1622] | 151,633 |
| vector demand512 | 130.44 → 129.45 | 25,416 → 25,440 | .9985 [.8039, 1.1451] | 94.72 → 93.86 | .9874 [.7416, 1.0792] | 244,888 |
| half512 | 121.59 → 120.05 | 18,424 → 18,484 | .9925 [.9866, 1.0021] | 163.98 → 163.67 | .9986 [.9042, 1.1596] | 208,291 |
| quad512 | 68.74 → 67.89 | 17,080 → 17,076 | .9907 [.9469, .9969] | 194.54 → 195.18 | 1.0014 [.7364, 1.5681] | 81,540 |
| repeated1024 | 128.78 → 132.21 | 24,432 → 24,528 | 1.0261 [.9771, 2.2106] | 69.12 → 69.05 | .9977 [.9585, 1.0660] | 64,734 |

The four common inputs are the fixed inherited template/loop/memory, floating,
exception/call and unused-definition workloads. [performance194.py](../student.tests/pa29/performance194.py)
hash-checks and reuses the exact implementation191–193 owner sources: 512 vector,
half and quad function specializations and 1,024 tagged member definitions.
Their runtime inputs and independently derived checksums keep their loops live.

Seven compiler paired ranges and every runtime range cross unity. Quad compile
pairs are consistently below unity in this run, but the approximately 0.9%
median is small relative to its A/A range, **68.57–111.86 ms**, and there is no
removed compiler work to explain a gain. No speedup is claimed. The tagged
member series retains its 2.2106 outlier and 2.6% median increase; its pairs and
A/A variation do not establish a repeatable regression. Maximum RSS changes
range from -4 to +192 KiB. Runtime variation between identical executable bytes
cannot be attributed to an optimization. There is no new optional transform to
justify or remove.

## Correctness costs, bounds and historical evidence

The repair changes signaling bits in required half/quad literal encoding and
uses exact quad text at an explicit adapter boundary. Both are O(1) work per
fixed-width literal. They add no IR, machine instructions, frame slots, calls,
branches or executable text to equivalent programs. The broken NaN/threshold
programs are correctness reducers, never performance baselines. Corrected
source/adapter disassembly and data sections agree exactly, including the
classification threshold. Raw LowIR global/function ordering may change symbol
table order; byte equality of complete ELF files is not a contract.

The fresh architecture trace retains **236 input instructions**, **12 extended
legalization additions**, **9 helper declarations**, then **one forced inline
call**, **20 actual work units**, **36 reserved units**, and **254 prepared
instructions**. Native emission produces **312 instructions in two live
functions**. Standalone MIR exposes main/apply stack sizes **304/176 bytes**,
each with 48 bytes of floating scratch; host disassembly and unwind records are
also retained. O0 homes, helper calls and memory traffic remain visible costs.
No claim of spill elimination, loop speedup or runtime profit follows from IR
counts. O1–O3 policies/quality beyond this compatibility stage remain PA32–33
work; self-host execution belongs to PA34, explicitly excluded by PA29.

[Historical verification](../student.tests/pa29/evidence194/inherited-performance-verification.json)
recomputes **1,624 observations and 80 launcher samples** from handoffs191–193,
including their preliminary runs. Final datasets retain matching frozen binaries
and source-tree bindings. Some *preliminary* scratch binary paths were reused
by those handoffs: the record explicitly identifies mismatching current bytes.
Their observations and original hashes remain historical evidence, not fresh
byte verification or final acceptance. This limitation does not apply to the
new frozen A/B files. Audit190 and earlier measurements are unchanged.

The audit's own [preliminary common run](../student.tests/pa29/evidence194/preliminary-common-performance.json)
retains another **224 observations**, bound to the first NaN repair before the
quad-threshold discovery. Its frozen binary remains distinct. Final validation
and measurements were rerun after that second repair; no old measurement was
silently rebound. Total new retained observations are **672 plus 16 launchers**.

Under spec §9, inherited blanket 15% latency/RSS and zero-growth targets remain
diagnostic, with all misses and measurements preserved. They do not override
stage-scoped acceptance. Required representation work is bounded; no repeatable
avoidable regression is established. Mandatory limits remain unchanged:
constexpr one million steps/depth 512; vector constant traversal one million
lanes; straight-line vector lowering at most eight lanes, then a counted loop;
extended legalization at most six operations per input; forced-inline depth 64,
262,144 work per caller and 4,194,304 per preparation, reserved before mutation;
native storage/alignment bounds and course timeouts. There is no profiler,
allocator-specific diagnostic or new performance percentage exit gate.
