# PA20 aggregate and class result performance evidence

Scope: PA20 `--emit-lowir -O0`, with the supplied native backend for execution.
Entry `16ac49da` and implementation `37f8300b` are frozen in the
[400 observations and 40 warmups](../student.tests/pa20/performance99.json).
The [harness](../student.tests/pa20/benchmark99.py) records binary, backend,
input and harness hashes, flags, CPU affinity, every sample, A/A ranges and
four paired ABBA ratios. Compilation and execution are measured separately.
Validation had finished before timing began; no other task-owned compiler job
was running. Every executable checks its result, and runtime loops use a
volatile limit. Startup and specialization executable timings remain explicitly
startup-dominated controls, not runtime profit evidence.

| Workload | Compiler A/B ms | Peak RSS A/B KiB | Runtime A/B ms | Native payload A/B bytes |
|---|---:|---:|---:|---:|
| startup | 9.95 / 10.31 | 5640 / 5720 | 4.98 / 4.82 | 24 / 24 |
| aggregate-specializations-800 | 153.88 / 190.15 | 25936 / 27460 | 4.57 / 4.60 | 75202 / 134401 |
| aggregate-specializations-3200 | 779.97 / 810.74 | 85864 / 90696 | 6.27 / 6.09 | 300802 / 537601 |
| return-specializations-800 | 250.53 / 252.09 | 28952 / 28716 | 4.79 / 4.86 | 86294 / 73494 |
| return-specializations-3200 | 967.95 / 977.79 | 97256 / 100944 | 4.50 / 4.54 | 345494 / 294294 |
| runtime-calls | 8.65 / 8.88 | 6124 / 6008 | 127.10 / 126.37 | 206 / 206 |
| runtime-memory | 9.86 / 10.10 | 5920 / 5988 | 116.88 / 115.09 | 434 / 434 |
| runtime-floating | 10.42 / 10.78 | 6140 / 6024 | 94.30 / 95.02 | 230 / 230 |
| runtime-aggregate | 8.41 / 8.19 | 6152 / 6036 | 48.89 / 60.78 | 208 / 281 |
| runtime-return | 9.79 / 9.85 | 6152 / 6028 | 34.24 / 30.49 | 232 / 201 |

Compiler `.text` grows **2,074,182→2,075,910 bytes**, +1,728 (0.083%). The
supplied sectionless ELF requires the established payload proxy, including
static data; compiler sizes are actual `.text`. Startup/calls/memory/floating
executables are byte-identical between A and B. Changes in those runtime
timings are noise, not an executable speedup or regression.

## Costs, spread and stage acceptance

The aggregate helper shape is required by the unchanged omitted-class-tail
oracle. It adds one call and representation transport to the former ordered
destination path. The two programs are behaviorally equivalent on these pure
constructor workloads, so both are valid timing inputs; A still fails the
required LowIR shape. This is required O0 lowering, not an optional optimizer
whose profitability can justify changing the contract.

The aggregate loop discloses a median **24.3% slowdown** and **73 extra payload
bytes**. Paired runtime B/A is **1.245, 1.260, 1.348, 1.000**; A/A is
47.83–49.22 ms, B is 46.54–65.64 ms. The added call/copy work accounts for the
cost; no runtime benefit is claimed. Helper duplication across occurrences of
the same type was removed before measurement: the helper owns local slots,
not the first caller's semantic temporary. The trace below verifies sharing.
Inlining or relaxing the required O0 comparison would change this stage's
contract; later native optimization is a separate owner.

At 3200 distinct aggregate types, payload grows **236,799 bytes**, IR grows
**80,010→118,410 instructions** (+12 per distinct type), and peak RSS grows
4,832 KiB. Compiler B/A pairs are **1.043, 1.072, 0.892, 0.869**; A/A spans
465.20–777.17 ms and B 650.13–881.73 ms. The 800-type pairs are **0.989, 0.997,
1.119, 1.258**. These mixed/noisy pairs do not establish a persistent latency
regression of the size suggested by the separate 800-type medians. The output
and memory growth are real, linear costs of the required helper representation.

Caller-owned results remove three IR instructions and 16 native payload bytes
per distinct return specialization. The 3200-case IR is **89,605→80,005**;
compiler B/A pairs are **1.067, 0.866, 1.039, 1.000**. The runtime-return pairs
are **0.923, 1.196, 0.904, 0.710**, with A/A 31.67–32.57 ms and B
26.48–73.14 ms. The smaller output is established, but the mixed timing and
outlier prevent a strong runtime-profit claim. All samples remain recorded.

No optimizer, inlining, unrolling, fixed point or work/growth policy was added.
Existing array expansion limit **8** and conservative ordered fallbacks remain.
Historical percentage/text targets remain diagnostic signals under spec §9,
as in [audit 97](performance97.md) and [handoff 98](performance98.md); none of
their observations or mandated bounds is erased. No self-imposed gate is newly
waived here. Necessary contract costs are disclosed, optional duplicate helper
work was removed, and unaffected executable hashes are unchanged.
**Stage-scoped performance acceptance passes for this completed behavior group.**
The remaining parser implementation and independent stage audit are still required.

## Ownership, complexity and source-to-native trace

`semantic/initializer_effects.cpp` summarizes an already completed constructor
by canonical EntityId and checked expressions by occurrence NodeId in the
TU-owned flat index. Unavailable constructor bodies are conservative, uncached
answers. Success/negative completed summaries do not depend on later lookup;
no body demand, parser replay or global invalidation is introduced. `this`,
member/alias reads, volatile fields and unknown effects reject transport.
`list_initialization.cpp` additionally checks selected argument conversions,
trivial copy storage and trivial destruction, recording `InitAction::helper_copy`.
Nontrivial language transfers retain their selected constructor and ordering.

`lowering/aggregate_helpers.cpp` consumes those facts. Representation helpers
are keyed by canonical target and complete parameter shape; language transfer
recipes still include their plan identity. Each helper is emitted once, in
O(fields), with one local slot per parameter and one destination action per
field. The number of helpers is bounded by demanded type/shape identities;
there is no search across unrelated types or cross-function cloning. Caller
temporaries stay with their function; helper slots are ordinary function-local
LowIR identities. TU facts and output pools release with their normal owners.

At 800/3200 aggregate specializations, constructor/expression summary work is
**802/3202**, hits **799/3199**, and candidate work is **12002/48002** (just two
more than entry at either scale). Body transitions remain **800/3200**, despite
each specialization being called twice. Both variants check **802/3202** bodies;
the changed path does not reinstantiate bodies. The result-ABI change consumes
the existing completed copy/move/destructor facts, with no additional queries.

The [trace](../student.tests/pa20/trace99.json) checks two demanded template
bodies, eight checked bodies, five summary work items, two summary hits and
two shared aggregate helpers across three return bodies. Two move-only return
functions consistently use the recorded indirect result convention through
signatures, calls and destination construction. Telemetry-on and ordinary
LowIR are byte-identical and execute successfully. Prior 94, 95, 96 and 98 traces
were also rerun. Backend allocation/encoding and self-hosting remain later-stage
owners; PA20 constructs typed LowIR itself and only the harness calls the
supplied backend.
