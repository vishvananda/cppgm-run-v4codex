# PA15 initialization performance evidence

This is PA15/O0 implementation evidence, with no new assignment exit gate.
The measured implementation is `4f4bea42`; compiler A is the frozen entry
`b8379f52`, B is the final bounded implementation. The committed
[raw final campaign](../student.tests/pa15/initialization-performance.json)
records hashes of both binaries, all sources, LowIR/native outputs and the
supplied backend. Compiler `.text` grows from 1,506,694 to 1,512,070 bytes
(5,376 bytes, 0.36%). All executable outputs were checked before timing.

## Protocol and interpretation

Linux x86-64, CPU affinity 31, host build `g++ -std=gnu++11 -Wall -O3` with
`TEST_RUNNER_ENABLE`; compiler flags `--emit-lowir -O0`; supplied native backend
`-O0`. The [harness](../student.tests/pa15/initialization_benchmark.py) freezes
binaries and source inputs, performs one warmup per binary, four A/A samples,
then two ABBA blocks. Compilation and native execution are separate measurements.
Wall time includes subprocess startup and reading the tiny `/usr/bin/time`
usage record; peak RSS comes from that child process, not the Python driver.
Telemetry is a separate `--stats --validate-lowir` invocation and is not timed.
Small compiler inputs take about 6 ms and are startup dominated; no frontend
speed claim is made from those samples. Workloads with 1,000/4,000 concrete
specializations take 0.08–0.90 s. Native loops/calls use volatile runtime inputs
and checked sums, and run for 0.14–1.56 s. Self-hosting belongs to PA34.

Array A/B outputs differ but execute the same checked result. Wide signatures
have identical LowIR; calls/memory/floating controls have identical native bytes.
Derived-class A output omits the base zero; update-query A rejects the input.
Their B-only measurements are absolute correctness costs, not speedups against
an incorrect baseline. Personal native controls cover the repaired behavior.

Native executables are sectionless. Text below is the executable payload from
entry to the first trailing data byte, **including alignment padding**. The
known array data tail is checked byte for byte and reported separately. This
does not confuse total ELF size or readonly data with machine instructions.

## Final compiler observations

Times are seconds: median [minimum, maximum]. RSS is the full KiB range.
Ratios are B/A within each ABBA block; A/A is the initial four-sample range.

| Workload | A wall | B wall | A RSS | B RSS | A/A wall | ABBA B/A |
|---|---:|---:|---:|---:|---:|---:|
| arrays-1000 | 0.13011 [0.12891, 0.13989] | 0.13022 [0.12960, 0.13132] | 23608–23876 | 24920–25088 | 0.12891–0.13054 | 0.9971, 0.9668 |
| derived-1000 | — | 0.07935 [0.07597, 0.08279] | — | 17484–17788 | — | — |
| updates-1000 | — | 0.11376 [0.11225, 0.11732] | — | 21852–22008 | — | — |
| wide-signatures-1000 | 0.21681 [0.21511, 0.22390] | 0.21776 [0.21443, 0.22129] | 48460–48772 | 48636–48776 | 0.21615–0.22239 | 1.0009, 0.9966 |
| arrays-4000 | 0.52464 [0.51977, 0.52718] | 0.52633 [0.52037, 0.70863] | 79280–79396 | 84068–84320 | 0.52393–0.52718 | 0.9950, 1.1840 |
| derived-4000 | — | 0.29933 [0.29858, 0.30536] | — | 54928–55092 | — | — |
| updates-4000 | — | 0.46177 [0.45344, 0.50312] | — | 71060–71332 | — | — |
| wide-signatures-4000 | 0.89791 [0.89076, 1.10666] | 0.90203 [0.89920, 0.91659] | 178324–178660 | 178348–178608 | 0.89076–1.10666 | 1.0124, 1.0063 |
| runtime-calls | 0.00606 [0.00567, 0.00621] | 0.00605 [0.00597, 0.00623] | 5332–5424 | 5264–5456 | 0.00591–0.00613 | 1.0079, 1.0285 |
| runtime-memory | 0.00608 [0.00582, 0.00619] | 0.00606 [0.00599, 0.00616] | 5328–5512 | 5328–5564 | 0.00582–0.00619 | 1.0000, 0.9866 |
| runtime-floating | 0.00628 [0.00592, 0.00637] | 0.00609 [0.00601, 0.00615] | 5432–5688 | 5548–5744 | 0.00620–0.00630 | 0.9588, 1.0006 |
| runtime-array-4 | 0.00593 [0.00582, 0.00608] | 0.00593 [0.00591, 0.00603] | 5300–5452 | 5296–5504 | 0.00582–0.00608 | 0.9958, 1.0032 |
| runtime-array-8 | 0.00591 [0.00572, 0.00636] | 0.00578 [0.00570, 0.00583] | 5348–5528 | 5388–5488 | 0.00599–0.00636 | 0.9984, 1.0046 |
| runtime-array-16 | 0.00601 [0.00577, 0.00615] | 0.00591 [0.00576, 0.00608] | 5336–5476 | 5304–5380 | 0.00600–0.00609 | 1.0018, 0.9898 |
| runtime-array-64 | 0.00634 [0.00628, 0.00656] | 0.00641 [0.00636, 0.00652] | 5368–5556 | 5288–5424 | 0.00628–0.00640 | 1.0093, 1.0018 |

The final arrays-4000 campaign has one 0.70863 s B sample with 0.43 s user
+ 0.08 s system time, producing the 1.1840 block ratio. No observation is
discarded. [The repeated compiler campaign](../student.tests/pa15/initialization-performance-repeat.json)
uses the same frozen inputs/binaries and excludes usage parsing and hash checks
from the timer. It also observes stalls; this is not evidence of a precise
compiler speedup or a stable sub-percent latency bound:

| Repeated workload | A wall | B wall | A RSS | B RSS | ABBA B/A |
|---|---:|---:|---:|---:|---:|
| arrays-4000 | 0.52146 [0.51631, 0.56300] | 0.54264 [0.51798, 0.62547] | 79136–79400 | 84040–84324 | 1.0860, 1.0001 |
| wide-signatures-4000 | 0.89863 [0.88687, 0.96506] | 0.89510 [0.89240, 1.05406] | 178296–178596 | 178340–178560 | 1.0877, 0.9990 |

A prior repeat accidentally included output SHA verification in wall time.
[Its complete observations](../student.tests/pa15/initialization-performance-repeat-instrumented.json)
and [exact harness](../student.tests/pa15/initialization_repeat_instrumented.py)
are preserved; they are not used to claim compiler latency. Its array ratios
are 1.0047/0.9828; wide-signature ratios 1.7912/1.0047 include a 2.40757 s
sample with 0.71 s user + 0.19 s system time. The corrected repeat reports
both its slower first block and near-equal second block without selecting one.
The final campaign shows approximately 6% extra array RSS for required constant
classification and backing records; unrelated wide-signature RSS is unchanged
within the reported ranges. No numerical latency/RSS ceiling is mandated here.

## Final executable observations

| Workload | A wall | B wall | A/A wall | ABBA B/A | Text A → B (bytes) | Data A → B (bytes) | File A → B (bytes) |
|---|---:|---:|---:|---:|---:|---:|---:|
| arrays-1000 | 1.11786 [1.11614, 1.12400] | 0.85880 [0.85623, 0.86005] | 1.11614–1.12092 | 0.7686, 0.7649 | 136118 → 94128 | 0 → 16000 | 136238 → 110248 |
| arrays-4000 | 1.55524 [1.55164, 1.56166] | 1.46507 [1.46094, 1.47081] | 1.55164–1.55623 | 0.9408, 0.9405 | 544118 → 376128 | 0 → 64000 | 544238 → 440248 |
| runtime-calls | 0.30015 [0.29942, 0.30116] | 0.30145 [0.30074, 0.30310] | 0.29942–0.30091 | 0.9987, 1.0082 | 206 → 206 | 0 → 0 | 326 → 326 |
| runtime-memory | 0.17684 [0.17568, 0.17736] | 0.17640 [0.17543, 0.17670] | 0.17671–0.17736 | 1.0020, 0.9981 | 434 → 434 | 0 → 0 | 554 → 554 |
| runtime-floating | 0.20898 [0.20821, 0.20952] | 0.20940 [0.20869, 0.20991] | 0.20821–0.20897 | 1.0009, 0.9995 | 230 → 230 | 0 → 0 | 350 → 350 |
| runtime-array-4 | 0.14791 [0.14758, 0.14817] | 0.15025 [0.14959, 0.15209] | 0.14775–0.14809 | 1.0219, 1.0142 | 226 → 224 | 0 → 16 | 346 → 360 |
| runtime-array-8 | 0.15311 [0.15240, 0.16305] | 0.14291 [0.14229, 0.14319] | 0.15240–0.16305 | 0.9334, 0.9268 | 254 → 232 | 0 → 32 | 374 → 384 |
| runtime-array-16 | 0.20582 [0.20473, 0.21063] | 0.20627 [0.20612, 0.20822] | 0.20473–0.20731 | 1.0036, 0.9949 | 310 → 310 | 0 → 0 | 430 → 430 |
| runtime-array-64 | 0.57464 [0.56977, 0.59989] | 0.57601 [0.57252, 0.57646] | 0.56977–0.57490 | 0.9949, 0.9752 | 750 → 750 | 0 → 0 | 870 → 870 |

## Budget, scaling and preserved rejected policy

The [preliminary campaign](../student.tests/pa15/initialization-performance-preliminary.json)
and [address-validation campaign](../student.tests/pa15/initialization-performance-address.json)
both used unbounded array backing. Each showed about an 8% runtime regression
on the 64-element array through the supplied backend despite much smaller text.
That avoidable policy was removed. The final **32-byte/object** O0 backing limit
keeps the eight-element measured benefit (ABBA ratios 0.9334/0.9268) and returns
16/64-element arrays to byte-identical native output with no backing data.
Validation still covers every constexpr array; omitted bounds retain the
existing zero loops. The limit is a general emitted-storage policy and does not
inspect fixture identity or bypass constant-expression requirements.

The required four-element backing form is about 1.6% slower in the isolated
single-function loop (ABBA 1.0219/1.0142) and adds 14 ELF bytes. This disclosed
cost follows the current fixture contract and supplied backend copy expansion;
it is not a claim that smaller IR always runs faster. With 1,000/4,000 distinct
four-element instantiations, both ABBA blocks improve runtime about 23%/6%
and total ELF size falls about 19%. The eight-element case improves about 7%
at a cost of 10 ELF bytes. Readonly data never exceeds 32 bytes per eligible
object, backed by one TU-owned record. Native backend improvements belong to
PA24 and later; the language behavior and validator are implemented here.

The added validation cache visits 5,000/20,000 initializer actions for
1,000/4,000 four-element arrays: five actions per object. Specialization frames
are 1,000/4,000 and body checks 2,001/8,001. Derived completion uses 1,000/4,000
frames and body checks; update queries use 2,000/8,000. No omitted run is expanded
for semantic validation. Work is O(typed initializer actions + candidates +
emitted data/instructions), keyed by immutable initializer/entity IDs, and
released with the TU. The backing bound and direct-initialization fallback
avoid an unbounded optional duplicate image. Validation is mandatory semantic
work; it does not construct a second syntax evaluator or emit dormant bodies.

This is O0 with no new optimizer, allocator or profiler gate. The retained
policy meets the explicit structural work/storage budget; its limited runtime
benefits, necessary contract cost, RSS growth and wall-time uncertainty are
reported together. Historical diagnostic targets in earlier plans remain
evidence rather than new exit requirements under spec §9. Mandated limits,
correctness, required LowIR comparisons and all existing fixtures are preserved.
Whole-stage audit and the four remaining implementation failures are still due.
