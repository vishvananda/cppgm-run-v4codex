# PA29 handoff157 performance evidence

This increment implements required declaration/storage alignment, empty-member
layout and member-designator `offsetof`. It adds no optional runtime optimization
and makes no speedup claim. PA29 remains incomplete. Prior evidence in
[handoff155](performance155.md) and [handoff156](performance156.md) is preserved.

## Frozen protocol

A is turn-entry `0bad8c207712b2077afc78f209a469c55b07847b`; B contains the
implementation in `a33d1086`. Compiler SHA256s are:

- A: `5631b69d1adb2c54a8b13120e5dd073ef705b34a3df462690dfdce357b8a0052`.
- B: `84642180af8de176d1f8496122bf786e1b87935aeb1c37b233f74accfbfc36c3`.

[Manifest](../student.tests/pa29/evidence157/manifest.json) records build/measured
flags, binary sizes, machine, affinity, scripts and hashes. Build flags are
`-std=gnu++11 -Wall -O3` with the recorded GCC system defaults; measurements use
`-O0 -c --stats` and `taskset -c 0`. Compiler execution and generated execution
are measured separately; `/usr/bin/time` supplies peak RSS. Host `g++` only links
the emitted benchmark objects. Inputs, objects and executables are hashed and
all executable checksums pass before timing. No builds or course/explicit tests
run during timings; light evidence bookkeeping overlaps common timings and
external scheduling is uncontrolled.

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py /tmp/pa29-157/common /tmp/pa29-157/base /tmp/pa29-157/final
PERF_CPU=0 python3 student.tests/pa29/performance157.py /tmp/pa29-157/affected /tmp/pa29-157/base /tmp/pa29-157/final
python3 student.tests/pa29/inspection157.py
```

## Equivalent common workloads

[All 224 observations](../student.tests/pa29/evidence157/common-performance.json).
The inherited memory/call, floating, exception and pruning inputs contain 2,400
demanded templates each. Each workload/mode runs four A/A observations and six
ABBA blocks. Ratios divide B's block mean by A's. Times below are sample medians,
RSS is maximum KiB, and bracketed ratio intervals retain every block.

| Workload | Compile A / B s | Paired compile B/A [range] | Compiler RSS A / B | Runtime A / B s | Paired runtime B/A [range] | Text bytes A = B |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.19411 / 0.19089 | 0.9652 [0.7710, 1.2371] | 29416 / 29432 | 0.05343 / 0.05324 | 1.0014 [0.9808, 1.1567] | 151633 |
| floating | 0.17099 / 0.17544 | 1.0117 [0.5142, 1.2164] | 29576 / 29232 | 0.05633 / 0.05633 | 0.9999 [0.9279, 1.0481] | 151474 |
| exceptions | 0.25481 / 0.25398 | 0.9411 [0.7668, 1.0122] | 29516 / 29140 | 0.27110 / 0.26571 | 0.9896 [0.9718, 1.0287] | 151781 |
| pruning | 0.20780 / 0.21438 | 1.0044 [0.7002, 1.5354] | 35040 / 34948 | 0.07229 / 0.07250 | 1.0014 [0.9902, 1.0123] | 151633 |

Every common object and executable is byte-identical A/B, so runtime differences
cannot indicate generated-code improvement. Paired compilation medians span
0.9411–1.0117, with large block spread and overlapping A/A noise. This evidence
does not establish a repeatable compiler speedup or a material regression.
Peak compiler RSS changes by +16, −344, −376 and −92 KiB respectively; these
small process-level differences do not establish a memory optimization either.
Compiler binary size grows from 3,900,504 to 3,930,992 bytes (30,488 bytes) for
required semantic capability. No outlier was discarded.

| Workload | A/A compile range s | A/A runtime range s | Runtime peak RSS A / B KiB |
|---|---:|---:|---:|
| memory | 0.16250–0.19973 | 0.07307–0.07387 | 1764 / 1760 |
| floating | 0.16902–0.26759 | 0.04924–0.05051 | 1764 / 1764 |
| exceptions | 0.25355–0.25628 | 0.26657–0.27669 | 3936 / 3932 |
| pruning | 0.20147–0.27331 | 0.07222–0.07344 | 1760 / 1764 |

## New capability costs

[All 48 observations and launcher calibration](../student.tests/pa29/evidence157/affected-performance.json).
A rejects the new source; failure latency is not a valid performance baseline.
B takes eight compiler and eight runtime samples at each size. Each template
constructs an aligned class with an overlapping empty member and computes a
member offset. Six million calls consume runtime inputs; Python independently
computes the checked checksum. Demanded functions and their class specializations
remain observable through the checked `demanded(argc)` result.

| Classes/functions | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Executable text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | 0.10807 [0.10696, 0.11249] | 20948 | 0.04032 [0.03982, 0.04114] | 1756 | 46130 |
| 1200 | 0.21337 [0.20998, 0.22179] | 34632 | 0.04035 [0.04011, 0.04048] | 1752 | 91730 |
| 2400 | 0.43506 [0.42921, 0.47605] | 62480 | 0.04070 [0.04036, 0.04140] | 1764 | 182930 |

Launcher median is 0.00474 seconds, range 0.00451–0.00540. Minimum compiler time
exceeds that median by 22× and minimum runtime by 8×. New-capability work counters
are proportional to demand: class completions, body transitions, substitutions
and empty-layout work are 600/1200/2400; type-query work is 1810/3610/7210;
native instruction counts are 9053/18053/36053. These are necessary implementation
costs, not an optimization profit claim.

[Structural inspection](../student.tests/pa29/evidence157/inspection.json) also
holds empty-layout work at 2, retained empty positions at 3 and fallback count
at 1 as an array bound grows 100 → 10,000 → 1,000,000. These short compilations
establish representation/work behavior, not wall-time performance. Source LowIR,
reader roundtrip, native MIR/emission and execution pass; enabling telemetry
leaves object and LowIR bytes identical.

## Owners, complexity and stage budgets

- Alignment operands are parsed once. Raw type storage decorations and pending
  expression queries survive substitution; semantic signatures erase them.
  Existing type/frame cache keys include the immutable decorated type. Alignment
  attribute work follows attribute operands and demanded substitutions. GNU
  class/object alignment is a minimum; typedef alignment can decrease storage
  alignment, consistent with [GCC's type attribute contract](https://gcc.gnu.org/onlinedocs/gcc-15.1.0/gcc/Common-Type-Attributes.html).
- Empty layout records exact `(class, byte offset)` pairs. Small class summaries
  contain at most **64** positions; larger shapes retain their canonical graph.
  Conflict queries inspect relevant addresses/subobject paths. Arrays locate the
  relevant element arithmetically instead of expanding array bounds. Per-layout
  flat indexes and monotone frontiers bound repeated identical-shape trial work;
  larger heterogeneous footprints can require more conflict-path work to satisfy
  the ABI's same-type address constraints. The fallback saves representation
  space without substituting an ABI-incompatible guessed layout.
- `offsetof` records a canonical type/member/index query chain. Lookup/layout and
  access facts are computed by semantic owners; constants and typed lowering
  consume offsets/strides. Work follows path length, actual lookup candidates and
  demanded layouts. Runtime indices evaluate once with ordinary effects and
  constexpr evaluation uses the current activation's values.
- TU arenas own type/query/field facts; the empty-placement indexes live for one
  class layout. No source replay, production text roundtrip or global retry was
  added. Explicit LowIR text roundtrip above is an inspection adapter only.

Optional optimization work/growth budgets for this increment are **zero**. The
64-position cap is a local representation limit, not a new assignment timing
gate. Necessary semantic cost, PA32/33 optimizer work and PA34 self-hosting remain
separate stage responsibilities. Historical blanket 15%/zero-growth targets
remain diagnostics under spec §9; their measurements are preserved, and they do
not create unsupported exit gates. No mandated limit, correctness or fixture
coverage has been weakened. Whole-stage PA29 acceptance still requires its
remaining implementation and audit.
