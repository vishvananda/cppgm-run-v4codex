# PA29 handoff156 performance evidence

This increment implements mandatory runtime builtin semantics, with no optional
optimization and no speedup claim. PA29 remains incomplete. Historical evidence
in [handoff155](performance155.md) is unchanged.

## Frozen protocol

A is turn-entry `6afc84f76c09cf336a4302af368d5b6ea44c0da0`; B includes
implementation through `81a9f67f`. SHA256s are respectively:

- A: `4e4873df66898a2bb16a06304569616f1fec412178b4ced3d5ab01cd6206d385`.
- B: `5631b69d1adb2c54a8b13120e5dd073ef705b34a3df462690dfdce357b8a0052`.

[Manifest](../student.tests/pa29/evidence156/manifest.json) records sizes, machine,
build flags and paths. Compiler build is `-std=gnu++11 -Wall -O3`; measured
invocations use `-O0 -c --stats`. CPU affinity is `taskset -c 0`. No build or
course test ran during timings; external scheduling remains uncontrolled.
`g++` only links emitted objects. Inputs, objects and executables are hashed;
outputs are checked before timing. Compiler execution and generated program
execution are timed separately with `/usr/bin/time` peak RSS.

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py /tmp/pa29-156/common /tmp/pa29-156/base /tmp/pa29-156/final
PERF_CPU=0 python3 student.tests/pa29/performance156.py /tmp/pa29-156/affected /tmp/pa29-156/base /tmp/pa29-156/final
python3 student.tests/pa29/controls156.py
```

## Equivalent correct common workloads

[All 224 observations](../student.tests/pa29/evidence156/common-performance.json).
The inherited fixed memory/call, floating, exception and pruning benchmarks each
include 2,400 demanded templates and check runtime-input-dependent results.
Each mode runs four A/A observations then six ABBA blocks. Paired ratios divide
B's block mean by A's. Seconds below are sample medians; RSS is maximum KiB.

| Workload | Compile A / B s | Paired compile B/A [range] | Compiler RSS A / B | Runtime A / B s | Paired runtime B/A [range] | Text bytes A = B |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.15667 / 0.15726 | 1.0088 [0.9855, 1.0571] | 29416 / 29684 | 0.05224 / 0.05257 | 1.0008 [0.9826, 1.0722] | 151633 |
| floating | 0.15956 / 0.16025 | 1.0033 [0.6887, 1.1126] | 29612 / 29780 | 0.04913 / 0.04906 | 1.0005 [0.9926, 1.0094] | 151474 |
| exceptions | 0.15796 / 0.15685 | 1.0022 [0.9888, 1.0222] | 29456 / 29732 | 0.25191 / 0.25155 | 0.9995 [0.9806, 1.1325] | 151781 |
| pruning | 0.19298 / 0.19302 | 1.0016 [0.7409, 1.0108] | 34980 / 35224 | 0.05269 / 0.05285 | 1.0039 [0.9962, 1.0133] | 151633 |

Every common object and executable is byte-identical A/B. Runtime variation
therefore cannot show generated-code improvement. Paired compiler medians show
0.16–0.88% overhead; peak compiler RSS rises 168–276 KiB. The compiler binary
itself grows from 3,848,944 to 3,900,504 bytes for required builtin capability.
These are measured mandatory costs, not an optional optimization tradeoff.
All ranges/outliers remain in the raw evidence; no universal time bound is inferred.

A/A compile ranges by table order are .15580–.19363, .15855–.16624,
.15667–.25755 and .19057–.19496 seconds. Runtime A/A ranges are
.05200–.05234, .04861–.05095, .24957–.25366 and .05272–.05310.

## New capability costs

[All 48 observations and launcher calibration](../student.tests/pa29/evidence156/affected-performance.json).
A rejects the new bit-count calls; its failure latency is not a performance
baseline. B takes eight compiler and eight runtime samples at each size.
Each demanded template executes fixed/generic width-aware bit counts. The runtime
executes six million calls whose inputs depend on argc and whose checksum is
independently computed in Python; work cannot be timed after constant folding.

| Demanded functions | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Executable text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | 0.40378 [0.26677, 0.85492] | 22228 | 0.32512 [0.30213, 0.47577] | 1760 | 251338 |
| 1200 | 0.49563 [0.46746, 1.23513] | 37768 | 0.36147 [0.29763, 0.62098] | 1956 | 502138 |
| 2400 | 1.07709 [0.83345, 1.38533] | 69204 | 0.32430 [0.25720, 0.40370] | 2472 | 1003738 |

Launcher median is 0.00900 seconds; even the shortest compiler/runtime sample
exceeds it by over 28×. Scheduling noise is substantial (some compiler samples
are over 3× the minimum); the full spread is preserved. Work counters are less
ambiguous: body transitions are 600/1200/2400, candidate work 1804/3604/7204,
type-query work 610/1210/2410, LowIR instructions 42048/84048/168048,
and native instructions 58259/116459/232859. Signature work remains 9 and retained
source regions remain 2 at all sizes. Memory and code size follow demanded work.
This establishes bounded implementation costs, not a runtime optimization win.

## Stage budgets and ownership

Builtin vocabulary is immutable and bounded. TU-owned canonical signature keys
include intrinsic kind, all operand/result types and optional arity; declaration
insertion cannot change a completed compiler-intrinsic signature. Expected
signature failures return an empty candidate without exception unwinding. Selected
conversions, effects and operations are consumed once by constant evaluation or
typed lowering. Body instantiation continues using retained source nodes.

Runtime bit counts use O(log width) arithmetic with width ≤128; constant counts
visit ≤128 bits. A 128-bit overflow product uses four 64×64 partial products;
smaller products use one 128-bit operation. Exact NaN storage adds bounded
integer storage/load work without extending the LowIR grammar. No fixed-point
optimizer or extra pipeline pass was added. **New optional optimization work
and code-growth budgets are zero**; existing backend budgets remain unchanged.
Required semantic code growth is reported above. Selection/allocation quality
improvements remain PA32/PA33 ownership, not an extra PA29 exit gate.

The inherited 15%/zero-growth targets are self-selected diagnostics, not mandated
PA29 limits, as already documented in handoff155 under spec §9. No mandated
limit, correctness case, comparison rule or coverage requirement was weakened.
Self-host performance remains outside the PA29 handout and is neither certified
nor waived by these measurements.

[Controls](../student.tests/pa29/evidence156/controls.json) cover exhaustive small
integers, 128-bit widths, 1,944 overflow oracle cases, negative arity/types,
side effects, current rounding modes and static/runtime floating payloads.
[Inspection](../student.tests/pa29/evidence156/inspection.json) checks a demanded
template and exact-bit constants through source LowIR, reader, native MIR and
execution, with no builtin call or terminate boundary in that IR. [Validation](../student.tests/pa29/evidence156/validation.json)
records required checks and unchanged fixtures. Telemetry equivalence is recorded
in [telemetry](../student.tests/pa29/evidence156/telemetry.json).

Semantic references for the extensions are the [GCC overflow contract](https://gcc.gnu.org/onlinedocs/gcc-14.1.0/gcc/Integer-Overflow-Builtins.html)
(infinite-precision arithmetic followed by a result-type representability test),
the [GCC builtin contract](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)
(argument evaluation for hints), and the [LowIR constant/storage contract](../pa8/lowir.md#constants-and-copies).
No reference outputs were corrected.
