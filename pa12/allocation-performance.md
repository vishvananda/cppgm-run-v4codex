# PA12 allocation, aggregate-copy and alias evidence

These groups add required O0 semantics and representations; no runtime
optimization gain is claimed. PA12 emits its own typed LowIR and uses the
supplied native backend. Its allocator and native optimization costs belong to
that supplied backend at this stage, not to a new PA24 acceptance gate.

## Frozen protocol

The harnesses take A binary, B binary, scratch directory and JSON destination:
[allocation](../student.tests/pa12/allocation_benchmark.py),
[aggregate](../student.tests/pa12/aggregate_benchmark.py), and
[final](../student.tests/pa12/allocation_final_benchmark.py).
They pin CPU 0; preserve platform, source/tool hashes and flags; validate LowIR;
execute each output; and require byte-identical common LowIR/native programs.
Timed compilation is `--emit-lowir -O0`; validation/statistics and supplied-backend
`-O0` construction are outside compilation timing. Every common workload uses
warmups, four A/A observations and two ABBA blocks. Newly supported inputs use
a warmup plus six absolute observations because A rejects their semantics.

| Binary | Commit | SHA-256 |
| --- | --- | --- |
| A | `70556b3d` | `86228da122723158ea55c38c1739e2469164a441fac6412767a0a47b2c544219` |
| Allocation B | `d2db8706` | `817c8940b2e28d7cbb376d1ae80f4b382eac87d2e5b10fe9084892c802d9a61c` |
| Aggregate B | `ffbc7095` | `9575867c694fa3b289113402e2baa12d35289a7be3b84b39d783fdd392360e0a` |
| Final C | `eed7d552` | `8912589fee00ca2192e1df62ed6e5e944abc8f24270e60ac4e6ed27252997784` |

All observations remain in [allocation JSON](../student.tests/pa12/allocation-measurements.json),
[aggregate JSON](../student.tests/pa12/aggregate-measurements.json), and
[final JSON](../student.tests/pa12/allocation-final-measurements.json).
Their paths identify retained binaries and scratch sources/outputs under `/tmp`.
The [final output check](../student.tests/pa12/allocation-final-output-check.json),
reproduced with [this harness](../student.tests/pa12/verify_allocation_outputs.py),
checks all nine earlier allocation workloads against C: validated LowIR and
native output are byte-identical. Thus their recorded executable measurements
also describe final output, without rerunning unchanged heap workloads.

## Compiler and executable results

Times are median seconds; RSS is the median of measured process peak KiB.
Individual peaks, times, ranges and telemetry remain in the JSON files.
Native text uses the established supplied sectionless ELF metric: executable
payload after entry. The compiler metric is its ELF `.text` section.

| Final common workload | Compiler A / C | Peak RSS A / C | Paired C/A | Native bytes A = C |
| --- | --- | --- | --- | ---: |
| 1000 namespaces | 0.24780 / 0.24876 | 49402 / 49320 | 1.010 / 1.000 | 168056 |
| 4000 namespaces | 1.01410 / 1.02221 | 185300 / 183734 | 0.955 / 1.009 | 672056 |
| Twelve-million-call loop | 0.00578 / 0.00576 | 4752 / 4764 | 1.010 / 0.974 | 323 |

The final common loop runs in 0.30851/0.30838 seconds, with paired ratios
0.987/0.991 and A/A range 0.30490–0.30895. The initial allocation campaign had
a noisy 0.30897/0.36273 runtime median and paired ratios 1.001/1.077 despite
identical executable bytes. The aggregate campaign measured 0.30816/0.30960
and 1.002/0.994. All observations are preserved; none supports a repeatable
runtime change. Final compiler A/A ranges are 0.24546–0.25653 and
1.01167–1.01908; a later A outlier of 1.13474 explains the low first paired
4000-case ratio. Final median common compiler costs are approximately 0.4%
and 0.8%, with no common memory growth.

| Newly supported workload | Compiler seconds | Peak RSS KiB | Native text bytes |
| --- | ---: | ---: | ---: |
| 1000 scalar-allocation namespaces | 0.18551 | 36338 | 224100 |
| 4000 scalar-allocation namespaces | 0.75370 | 132700 | 896100 |
| 1000 array-allocation namespaces | 0.28891 | 54568 | 704616 |
| 4000 array-allocation namespaces | 1.16024 | 203238 | 2816616 |
| 1000 aggregate-copy namespaces, final | 0.62528 | 122350 | 553080 |
| 4000 aggregate-copy namespaces, final | 2.60557 | 474972 | 2212080 |
| 1000 destructor-alias namespaces, final | 0.28556 | 52844 | 532608 |
| 4000 destructor-alias namespaces, final | 1.15252 | 197532 | 2128608 |

The namespace programs execute checked results, but their approximately 3 ms
runtimes are startup dominated. Separate loops use volatile bounds, checked
nonconstant checksums and explicit aborts for violated lifetime/reference facts:

| Runtime workload | Iterations | Compiler seconds / peak RSS | Runtime seconds (range) | Native bytes |
| --- | ---: | --- | --- | ---: |
| Scalar heap lifetime | 1200000 | 0.00574 / 4734 | 7.67420 (7.61328–7.68838) | 424 |
| Variable-length array lifetime | 300000 | 0.00613 / 4854 | 1.99424 (1.98316–2.00544) | 1424 |
| Aggregate/reference/global-array copies | 12000000 | 0.00663 / 4922 | 0.18119 (0.18012–0.18234) | 736 |
| Alias destruction and reconstruction | 12000000 | 0.00615 / 4834 | 0.35570 (0.35294–0.35765) | 1240 |

The supplied-backend heap programs also show high process peak RSS:
4800000 KiB for scalar allocation and 1199872 KiB for arrays. These measurements
are disclosed without attributing a speed benefit or silently excluding native
costs. The student compiler's emitted allocation/deallocation calls are retained.
The initial aggregate campaign independently measured 0.63506/2.60661 compiler
seconds and 0.18076 runtime seconds; its full observations are preserved.

Compiler text grows 897158 -> 916806 -> 917958 bytes: final growth is
20800 bytes (2.32%) for all three behavior groups. Common executable text stays
identical. New executable costs are semantic costs, not optional-transform
profitability evidence or extra performance exit gates.

## Owners and explicit work/growth bounds

Allocation/deallocation families use canonical operator identity plus the
scalar/array bit. Resolution visits required class/global candidates once and
retains selection, conversions, extent, stride, cookie, constructor and destructor.
The first bound executes once. Heap loops have constant emitted structure,
independent of the number of elements or dimensions; the cookie stores leaf
count. Partial-construction cleanup destroys the constructed prefix in reverse
and passes the original allocation pointer to the recorded deallocator. The
required LowIR comparisons validate these exception regions; personal execution
checks normal lifetime, null and sized-deallocation behavior.

A constant-return bound proof inspects one completed return and caches its
constant; it permits source-width arithmetic only when size/overhead cannot
wrap. Other dynamic extents widen before multiplication. Constant total-size
overflow rejects. Empty constructor omission checks an empty no-argument body
with no subobject actions; it has zero generated-code growth. Mixed default
scalar/array functions preserve distinct addresses with at most one extra
two-instruction adapter per singleton LowIR allocation/free role.

Aggregate appertainment retains one selected whole-class conversion instead of
recursing into fields when the source initializes the complete subobject.
The source call's type/callee stay intact. Source-declared copy/move types retain
their ordinary O0 global initialization instead of entering the existing early
scalar-field policy. Alias destruction searches only the object class and the
expression-context scope paths, then validates canonical type identity.

Measured allocation records grow 1000 -> 4000; scalar candidate visits
3001 -> 12001; array candidate visits 4001 -> 16001. Final aggregate conversion
objects and value-initialization records each grow 3000 -> 12000, initializer
actions 13000 -> 52000 and candidates 27001 -> 108001. Final alias lookup work
is 73010 -> 292010. These counters support proportional work alongside the
owner/data-flow bounds. Existing local-array expansion remains capped at eight
across dimensions. No body cloning, global retry, speculative optimization or
additional positive-runtime gate is introduced; inherited measurements remain.

Validation at `eed7d552`: PA12 226/257, earlier 1327/1327, 38 personal source
checks and file audit pass. Twenty-four entry failures are removed, no new
failures appear, and coverage is unchanged. All required new/delete comparisons
pass. The 31 remaining fixtures and member-pointer survivor control remain
required work. No fixtures, references or comparison rules changed here.
