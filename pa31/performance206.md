# PA31 final performance review

Frozen whole-stage A: `c0566ded`, SHA256
`2f91ab5c876290d0072f70645d48d6831b445773d22c6359166877112914627f`.
B: `d5bd5aee`, SHA256
`710fae53af4a786daa744d7c780b560acc49b01f9c92d029fd75b3f05b05b3da`.
The audited compiler is byte-identical to B. There is no audit code change to
invalidate the implementation's measurements.

## Protocol and complete evidence

[Performance205](performance205.md) retains 544 workload observations and 32
launcher observations, including allocation N=64/256/1024 and final-only
list/inheritance costs through N=1024. Their input/image/binary bindings and
summaries were independently verified; none is discarded or superseded as raw
evidence. Rejected/crashing A workloads remain final-only cost measurements.

Audit206 adds **280 workload observations and 16 launcher observations**, using
the same fixed common and hosted stream inputs, binaries, `-O0 -c --stats` flags
and CPU 2 affinity. Each compile/runtime series has four A/A observations followed
by six ABBA blocks; paired ratios use each block's mean B divided by mean A.
Compilation and generated execution are measured separately, with no concurrent
build, course test or control run. Every executable checks its live runtime-input
result. The common workloads retain 2,400 demanded templates plus loops, calls,
memory, floating point, exceptions and unused-body pruning. The hosted workload
constructs/checks/destroys 200,000 streams. Host g++ links the student objects.

All observations, phase/work counters, image identities and summary spreads are
preserved in [common](../student.tests/pa31/evidence206/common-performance.json)
and [hosted](../student.tests/pa31/evidence206/hosted-performance.json) records.
[verify206.py](../student.tests/pa31/verify206.py) recomputes ordering, summaries,
image bindings and hashes, including the inherited evidence. No timing sample
is trimmed or startup-subtracted. The machine is shared.

## Fresh results

Times below are medians in milliseconds. Paired ratios summarize ABBA blocks,
so they need not equal ratios of the individual medians.

| Workload | Compile A/B ms | Compile B/A [range] | Runtime A/B ms | Runtime B/A [range] | Compile peak RSS A/B KiB | Object / executable text A → B |
|---|---|---|---|---|---|---|
| memory | 399.94 / 425.63 | 1.046 [0.692, 1.119] | 94.82 / 98.08 | 1.002 [0.785, 1.018] | 29664 / 29764 | 151393 / 151633 → same |
| floating | 382.05 / 387.81 | 0.990 [0.976, 1.051] | 84.16 / 86.40 | 1.008 [0.766, 1.075] | 29768 / 29936 | 151234 / 151474 → same |
| exceptions | 171.01 / 170.58 | 0.997 [0.964, 1.860] | 251.36 / 252.99 | 1.004 [0.988, 1.384] | 29684 / 29784 | 151541 / 151781 → same |
| pruning | 206.95 / 208.74 | 1.002 [0.993, 2.855] | 52.39 / 52.38 | 1.000 [0.998, 1.009] | 35652 / 35764 | 151393 / 151633 → same |
| stream1 | 1052.52 / 1060.90 | 1.005 [0.925, 1.018] | 118.29 / 117.85 | 0.996 [0.837, 1.014] | 66028 / 66052 | 28652 / 28892 → 28613 / 28853 |

| Workload | Compiler A/A ms | Runtime A/A ms |
|---|---|---|
| memory | 394.75–783.38 | 163.04–173.94 |
| floating | 419.59–467.15 | 78.75–170.50 |
| exceptions | 174.53–455.91 | 248.77–252.21 |
| pruning | 207.57–330.77 | 52.20–52.67 |
| stream1 | 1040.53–1229.97 | 116.50–116.83 |

Launcher calibration is 5.66–6.52 ms for compiler help and 4.52–38.48 ms for
`/bin/true`, including wrappers; the runtime launcher outlier remains in the
record. Workload loops dominate ordinary launcher cost, but shared-host noise
limits small-change inference. Common A/B objects and executables are identical.
Stream text is 39 bytes smaller after removal of unwanted allocation adapters.
The trace, not text size, establishes the ownership correction's legality.

The common compiler paired medians range from 0.990 to 1.046 and runtime medians
from 1.000 to 1.008. All fresh paired ranges cross unity, with substantial outliers
including pruning's 2.855 block. Together with A/A variation, the earlier runs
and unchanged common images, this establishes neither a repeatable avoidable
regression nor a runtime speedup. No speedup is claimed. The stream compile median
ratio is 1.005 and runtime ratio 0.996, likewise not evidence of a small gain/loss.

## Stage-scoped acceptance and work budgets

There is no optional transform added by PA31 or this audit. The semantic repairs
are required for valid hosted behavior. Required list/inheritance work remains
proportional to declaration/plan edges: implementation205 measured list exception
work **22N+3**, inherited exception work **6N**, and N template body transitions
for both families. The list family's maximum recorded RSS is **71,944 KiB**.
The fresh common maximum compile sample is **0.975 s**; hosted maximum is
**1.230 s**, including A/A. All stay below the unchanged **45-second** hosted limit.

The [source audit](audit.md) checks actual pipeline budgets: mandatory forced-call
admission reserves at most 262,144 units per function, 4,194,304 per Program,
depth 64, and retains valid calls when ineligible or exhausted. Selection uses
bounded local folds, call-clobber facts and per-function placement. The combined
hosted trace uses only 2,405 actual / 4,847 reserved expansion units. Its frame,
indirect loop calls and exception cleanup are inspected; fewer IR nodes are not
used as a substitute for runtime evidence.

Inherited blanket 15% latency and zero-growth thresholds are diagnostic targets,
not stage exit gates under spec §9. Their historical observations remain intact.
The rationale is explicit: compare correct equivalent implementations, preserve
mandated limits and reject demonstrated avoidable costs; do not compare success
against rejection or require later optimizer/inception outcomes at PA31. PA32/33
optimization policies and PA34 self-host execution retain their stage boundaries.
Correctness, fixture coverage, comparison rules and timeout limits are unchanged.

Reproduce the fresh runs with `PERF_CPU=2` and
`student.tests/pa27/performance147_common.py OUT A B`, then
`PERF_CPU=2 PERF_FAMILIES=stream student.tests/pa31/performance205.py OUT A B`.
The validation record gives the frozen paths and artifact locations.
