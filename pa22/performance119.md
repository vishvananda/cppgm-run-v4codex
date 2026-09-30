# PA22/O0 handoff 119 performance

The change consumes completed conversion-body and object-lifetime facts to omit
an inert empty receiver. It is required to meet PA22's constant-condition LowIR
shape. Unknown parameter adjustment and target-word truth retain their existing
correct implementation; their corrected oracles do not introduce optimization
work. The LowIR validator's width bound is constant work in external/audit
validation, not an added production pass.

## Frozen protocol

The [harness](../student.tests/pa22/benchmark119.py) records sources, flags,
binary/backend hashes, platform, CPU affinity (CPU 0), results and all timings.
Both lanes use `--emit-lowir -O0`. Native code uses the same supplied object
backend and `g++ -no-pie` host link. Each workload warms up each lane, records
four A/A observations, then four ABBA blocks for compiler and executable
separately. Peak compiler RSS uses `/usr/bin/time`. Stats/IR validation and
checked execution precede timing. Builds and course/control suites finished
before timing began. The two affected runtime loops execute 40 million
iterations under volatile bounds and check their computed results.

| Binary | Commit | `.text` bytes | SHA-256 |
|---|---|---:|---|
| A: entry | `17603c8a` | 2279302 | `2c4d610ea33c14d3dcd82a67cee412edb9f73ec558451c0e838b5b31ded8a968` |
| B: final | `321c93db` | 2281222 | `d2cac90074d5f8c4c9a73201e6b3bd1f6274b925146ec8879fe5491b7eda9b65` |

Compiler text grows **1920 bytes (0.084%)**. Frozen artifacts remain at
`/tmp/pa22-119/{entry,final}`. Both binaries are semantically correct for every
measured input. All observations, including the regression below, are retained
in [common](../student.tests/pa22/performance119-common.json) and
[affected](../student.tests/pa22/performance119-affected.json) evidence.

## Measurements

Times are median milliseconds; RSS is peak compiler KiB; native text includes
unchanged hosted startup. The final two columns show the full range of the four
paired ABBA B/A ratios. Startup-sensitive tiny compiler inputs and trivial mains
do not establish speed claims by themselves.

| Workload | Compile ms A/B | RSS KiB A/B | Runtime ms A/B | Native text A/B | Compile paired B/A | Runtime paired B/A |
|---|---:|---:|---:|---:|---:|---:|
| auto-specializations-9600 | 695.38/688.17 | 108288/108324 | 4.19/4.19 | 768286/768286 | 0.833–1.000 | 0.982–1.031 |
| runtime-calls | 5.87/5.89 | 6280/6328 | 122.95/123.14 | 430/430 | 0.999–1.013 | 1.001–1.005 |
| runtime-memory | 6.21/6.19 | 6308/6312 | 73.52/73.61 | 658/658 | 0.958–1.030 | 0.996–1.007 |
| runtime-floating | 6.28/6.32 | 6440/6488 | 86.59/86.44 | 454/454 | 0.988–1.018 | 0.997–1.000 |
| runtime-member | 6.02/6.10 | 6320/6360 | 210.91/210.69 | 488/488 | 0.976–1.018 | 0.997–1.004 |
| member-functions-2048 | 152.23/151.61 | 27660/27732 | 4.09/4.12 | 184632/184632 | 0.900–1.003 | 0.955–1.043 |
| runtime-conditional | 6.40/6.45 | 6352/6368 | 153.95/163.87 | 352/345 | 0.988–1.032 | 1.048–1.068 |
| runtime-member-selection | 6.45/6.51 | 6344/6256 | 276.36/262.79 | 531/523 | 0.987–1.025 | 0.924–0.958 |
| receiver-functions-512 | 30.34/29.55 | 10208/9792 | 3.78/3.76 | 14616/9496 | 0.969–0.986 | 0.990–1.008 |
| receiver-functions-2048 | 105.01/102.20 | 21748/20832 | 3.91/3.93 | 57624/37144 | 0.970–0.980 | 0.978–1.029 |

All six common inputs retain byte-identical LowIR and native text. Small runtime
variations (including the calls median's +0.16%) are not claimed as changes in
execution efficiency. Common peak RSS rises by at most **72 KiB**. The new proof
has zero receiver visits on all six common workloads.

The large affected compile improves **2.7%**, with four improving pairs; A/A is
104.25–106.37 ms, A 104.03–106.81 ms, B 101.64–103.93 ms. It uses **916 KiB less
peak RSS** and emits **20480 fewer native text bytes**. At 512 functions every
compile pair also improves, with **416 KiB less RSS** and **5120 fewer text
bytes**. Runtime of their trivial mains is only a correctness check.

The member-selection loop improves **4.9%** at the median, with paired ratios
**0.945, 0.958, 0.943, 0.924**. A/A is 274.79–283.24 ms. Disassembly confirms
removal of a receiver-address instruction and a byte store on every iteration;
the stack reservation drops **152→136 bytes**, and native text drops eight
bytes. The member call, high-word adjustment extraction and all loop work remain.
This is a repeatable runtime benefit, not an inference from smaller IR.

## Scalar-loop regression and native placement

The simpler scalar loop is **6.4% slower** despite seven fewer text bytes and
stack reservation **32→16 bytes**. All four pairs regress, beyond its A/A range
154.32–155.50 ms. This observation is not dismissed as noise or hidden by the
member-loop gain.

The [placement diagnostic](../student.tests/pa22/placement119.py) links the same
frozen A/B object files at four text addresses, preserving native instruction
work within each lane. Each address has its own warmups, four A/A observations
and four ABBA blocks, with checked results. [All observations and main
disassemblies](../student.tests/pa22/placement119.json) are retained, including
original executable disassembly. This is a runtime-only diagnostic, not a new
compiler performance campaign or a replacement for the default-layout results.

| Text address | Runtime median ms A/B | Paired B/A range |
|---|---:|---:|
| `0x402000` | 156.73/156.70 | 0.982–1.011 |
| `0x402010` | 153.03/164.08 | 1.068–1.101 |
| `0x402020` | 153.33/161.14 | 1.049–1.062 |
| `0x402030` | 152.39/152.43 | 0.995–1.007 |

The slowdown depends on native code placement: two placements eliminate it,
two reproduce it. The precise microarchitectural cause is not established.
Both original loops keep the same real sum/index memory operations and iteration
count; B removes only the unused receiver address/store and changes instruction
placement. PA22 owns semantic facts and LowIR, while native instruction placement
is owned by the supplied backend until PA24. Adding source-unrelated padding or
restoring unnecessary receiver stores would violate the intended pipeline/LowIR
shape. No additional optional pass, loop rewrite or speculative transform was
introduced. The required receiver elision is retained with its measured member
loop/compile benefit and disclosed backend placement limitation. Later backend
work must measure layout choices rather than assume fewer instructions are faster.

## Owners, limits and stage acceptance

`prepare_user_conversion` registers one typed source/conversion-use edge per
prepared scalar conversion, indexed by canonical selected function ID. The
existing completed-function sweep establishes a body summary once, then visits
only that function's registered uses. The receiver proof examines at most **8
nodes/use**, follows only parenthesis edges and reads existing empty-layout and
completed constructor/destructor action facts. Those action effects already have
per-owner monotonic caches; no new instantiation or callee-body analysis occurs.
Unknowns, effects, named receivers, arguments, nonempty layouts, lists and budget
exhaustion retain conservative evaluation. The result is one boolean in the
existing conversion record, directly read by lowering.

Work is **O(uses + existing required action edges)** with at most eight local
visits/use. At 512/2048 independent uses, counters are exactly **512/2048** visits
and **512/2048** omitted receivers. Both live loops use one visit and one fact.
Use edges occupy three 32-bit identities each in a geometric TU-owned vector;
the flat head index and all facts release at TU destruction. There are no
per-node owning pointers, copied trees, rendered keys, global invalidation,
repeated syntax parsing or new whole-program scans. Telemetry merely counts
existing visits; stats and validation preserve byte-identical IR.

Output growth budget is **zero**: a proved occurrence removes storage and
operations, without duplicating a body or control flow. Result-summary wrapper
budget remains eight; all inherited PA22 member-proof/flow and initializer
expansion limits remain unchanged. Correctness controls exercise fallback,
constructor/base/argument effects, volatile arguments, escaping receivers,
conversion effects, destructor timing and exception propagation. This bounded
O0 implementation has measured compiler/RSS and member-loop benefits; no optional
broader optimizer is required.

Spec §9's stage-scoped acceptance applies. There is no mandated numeric PA22/O0
latency/RSS threshold. Inherited +15%, +16 MiB and 5.5× diagnostics remain
non-gating; performance114–118 and their misses are preserved. This reclassification
does not waive correctness, coverage, comparison or work/growth limits. Native
placement, optimization, debug and self-hosting work retain their later owners.
Whole-stage independent review must assess this evidence and the accumulated
implementation before advancement.
