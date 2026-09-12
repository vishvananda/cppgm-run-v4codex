# PA13 O0 performance evidence

This preserves the original stage-completion campaign. The
[final audit review](../../pa13/final-audit-performance.md) records the current
implementation and additional measurements; none of these historical samples
or diagnostic misses is removed.

The final implementation is `6ca0578c` (following `df2be861` and `9a80791c`).
Stage entry is `823e929c`. The frozen binaries are:

- A: SHA-256 `46aa205bfdc4f9a11fd28aec8e0cef7714ea110c61e280c4e7b03d9ab91e0ccb`
- B: SHA-256 `35547adc8e82fb94e30a7d687f6bb94e0d9ca4722639dc41423ee696e2513243`

[Full observations](performance.json) and [noise follow-up](performance-repeat.json)
retain 352 wall/RSS observations including warmups. They also retain platform,
CPU affinity, exact flags, hashes of harnesses/backend/binaries, fixed generated
sources, outputs, phase/work telemetry and checked executable outcomes. Frozen
artifacts are under `$RALPH_ARTIFACT_DIR/pa13/`. The earlier pre-validator B is
preserved separately and was not used in these measurements.

## Protocol and acceptance

Both production binaries use the repository's g++ C++11 O3 host build with the
course runner enabled. The compiler workload uses `--emit-lowir -O0`; the
inherited template corpus uses its supported `--emit-semantics` view. Execution
uses the supplied native backend at O0. No host/reference compiler implements
source semantics or LowIR for this compiler.

Each common workload has warmups, four A/A calibration observations and two
ABBA blocks. The noise follow-up uses the identical frozen inputs/binaries and
four ABBA blocks. New virtual behavior has six B-only observations, because A
is incorrect for virtual dispatch/lifetimes. Every common LowIR/semantic output
is byte-identical; every common executable is also byte-identical. All executable
rows check their result, with volatile loop bounds and live loop/call/memory/FP
work. Compilation and execution are timed separately. Audit/telemetry runs are
separate from timed production runs and explicitly add `--stats` and, for LowIR,
`--validate-lowir`.

This stage adds required semantics, with zero optional optimization passes or
speculative growth. Vtable size is two header words plus one slot per ordinary
virtual and two per virtual destructor. A deleting entry expands at most one
prepared suffix; larger or bodyful cases call the complete entry once. There
is no cloned user body in a deleting entry. These explicit work/growth bounds
apply alongside existing bounded PA12 cleanup. The spec supplies no numeric
PA13 latency/RSS/text/runtime ceiling; inherited diagnostic thresholds do not
create one. No speedup claim is made, and no historical observations are dropped.

## Compiler latency and peak RSS

Medians and maximum RSS across timed production observations are shown below.
The full JSON retains both source scales, startup measurements, spread and all
paired observations.

| Fixed workload | A median ms | B median ms | A peak KiB | B peak KiB |
| --- | ---: | ---: | ---: | ---: |
| Calls, 4x | 1700.009 | 1698.408 | 304876 | 308216 |
| Memory/FP, 4x | 1440.154 | 1431.652 | 249132 | 251508 |
| References, 4x | 50.288 | 50.783 | 14532 | 14600 |
| Template semantics, 4x initial | 270.009 | 267.913 | 37692 | 37140 |
| Template semantics, 4x repeat | 267.426 | 265.431 | 37732 | 37128 |
| Classes, 1000 copies | 144.096 | 143.934 | 31580 | 31680 |
| Classes, 4000 copies initial | 583.617 | 622.024 | 110452 | 112412 |
| Classes, 4000 copies repeat | 574.720 | 578.954 | 110452 | 112384 |
| New virtual classes, 1000 copies | — | 375.862 | — | 82904 |
| New virtual classes, 4000 copies | — | 1589.746 | — | 320852 |

The large class initial ABBA ratios were **1.2950 and 1.0052**, motivating the
repeat. Its four ratios were **1.0146, 1.0148, 1.1676, 1.0052**. The high third
repeat block contains one B observation of 0.7606 s; its other B observation is
0.5757 s. These outliers remain in the report. The repeat medians show 0.74%
latency and 1.75% peak-RSS growth. Additional class/member/ABI facts and layout
records explain bounded required storage costs; no optional transform was
retained to obtain an unmeasured benefit. There is no reproducible large
latency regression across the paired blocks.

The initial template ratios were **0.9986 and 0.6203**. Repetition yielded
**0.9979, 0.9902, 0.9885, 1.0011**; the isolated initial apparent speedup is not
claimed as a benefit. The reference workload ratios were **1.0181 and 1.0122**;
the calls-1 ratios were **1.0062 and 1.0170**. These small regressions are
preserved, rather than hidden by aggregate timing. Other paired results and
A/A spreads remain in the JSON.

Fourfold growth of the new virtual corpus gives **4.23x latency**, **3.87x RSS**,
exactly **4x virtual-slot work** (9000 to 36000), **4x demanded classes** (2000
to 8000) and **4x instructions** (143000 to 572000). The token cursor remains
bounded at 32 pending tokens. These measurements support the stated work
ownership; they do not establish an optimization speedup.

Compiler `.text` grows from **980742 to 1018502 bytes**, **+37760 bytes (3.85%)**,
for the virtual semantic, layout, ABI and lifecycle implementation and audit
support.

## Generated execution and text size

| Checked runtime workload | A median s | B median s | A/B payload bytes |
| --- | ---: | ---: | ---: |
| Classes, 12 million iterations | 0.087525 | 0.087529 | 248 / 248 |
| Calls, 96 million iterations | 0.477839 | 0.478460 | 206 / 206 |
| Memory, 64 million iterations | 0.278738 | 0.279018 | 434 / 434 |
| Floating point, 32 million iterations | 0.331003 | 0.332101 | 230 / 230 |
| New virtual lifetimes/dispatch, 12 million iterations | — | 0.301098 | — / 2360 |

Common runtime ABBA ratios are respectively **0.9908/1.0000**,
**1.0042/0.9998**, **1.0015/0.9992**, and **0.9985/1.0089**. Identical executable
bytes make the small timing differences observational noise, not generated-code
profit or regression. Startup is recorded separately (roughly 3.5 ms execution);
no inference is drawn from its short samples. The backend produces sectionless
ELF, so the executable metric is payload after the ELF entry, including support
code and data, rather than claiming a nonexistent isolated `.text` section.

Native optimization and self-hosting remain later-stage evaluation surfaces.
PA13's acceptance is correct bounded O0 lowering, with measured required costs
and preserved common-program output, without additional self-selected gates.
