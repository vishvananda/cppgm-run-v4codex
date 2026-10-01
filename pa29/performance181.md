# Performance181 — zero-extent arrays at PA29 / O0

This group implements required semantics and adds no optional optimizer. It
claims no compilation or generated-code speedup. Acceptance follows spec §9;
[performance179](performance179.md) and [performance180](performance180.md),
including their observations and diagnostic targets, remain preserved.

## Frozen inputs and protocol

The [manifest](../student.tests/pa29/evidence181/manifest.json) binds entry
`c0efbf29` (compiler SHA `292b3fe3c141…`) and final code `0e47335f` (SHA
`904c8098efb5…`), scripts, inputs, correctness checks and raw observations.
Flags are `-O0 -c --stats`. Compilation and execution are timed separately;
compiler builds, host linking and initial output checks are outside the timers.
No CPU affinity was set; external scheduling is uncontrolled. All samples,
including outliers, remain in the raw JSON.

- [Common observations](../student.tests/pa29/evidence181/common-performance.json): four fixed 2,400-specialization workloads for memory/loops/calls, floating point, exceptions and unused declarations. Four A/A calibration samples precede six ABBA blocks in each mode: 224 observations.
- [Array observations](../student.tests/pa29/evidence181/owner-performance.json): equivalent one-element-array inputs at 400/800/1600 specializations use the same A/A+ABBA protocol, 168 observations. Newly supported zero-extent inputs at those sizes use eight final compilations and eight executions each, 48 observations. No ratio compares a successful compile with entry rejection.
- Array executables consume argv seed 7. Their specialization probe and ten million varying loop iterations must match independently computed Python results (loop total 5,031,592,612). This checks live work and output before and during measurement. Common outputs are checked by their fixed runner.
- [Preliminary common](../student.tests/pa29/evidence181/preliminary-common-performance.json) and [preliminary array](../student.tests/pa29/evidence181/preliminary-owner-performance.json) data preserve all 440 observations at `f69601ef` (SHA `294d4fa996df…`). The compiler was then repaired to check object extent before narrowing. All 440 observations were repeated for the corrected binary. The old path named `cppgm-final` identifies that earlier frozen hash, now retained as `cppgm-prelimit`; preliminary data does not substitute for final validation.

## Equivalent inputs

Values below use the final binary. Ratios are medians of six paired ABBA block
ratios. RSS is the maximum timed ABBA value; A/A RSS remains in raw data.
Text is the linked executable's `.text` section.

| Workload | Compile A/B median s | Compile B/A [range] | Compiler RSS A/B KiB | Runtime A/B median s | Runtime B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1735/0.1749 | 1.0194 [0.8044–1.0643] | 29468/29764 | 0.0526/0.0524 | 1.0066 [0.9255–1.0321] | 151633/151633 |
| floating | 0.1856/0.1805 | 0.9550 [0.8221–1.5730] | 29644/29844 | 0.0496/0.0493 | 0.9983 [0.9550–1.0330] | 151474/151474 |
| exceptions | 0.1814/0.1801 | 1.0289 [0.9566–1.1536] | 29036/29224 | 0.2789/0.2767 | 1.0125 [0.8488–1.3214] | 151781/151781 |
| pruning | 0.2188/0.2189 | 1.0240 [0.8686–1.6568] | 35680/35904 | 0.0536/0.0526 | 1.0028 [0.9452–1.0479] | 151633/151633 |
| array1-400 | 0.0836/0.0817 | 0.9989 [0.9106–1.0381] | 17496/17744 | 0.1285/0.1283 | 0.9991 [0.9894–1.0109] | 39459/39459 |
| array1-800 | 0.1600/0.1638 | 1.0230 [0.9100–1.2362] | 27480/27820 | 0.1254/0.1251 | 0.9968 [0.9919–1.0093] | 78659/78659 |
| array1-1600 | 0.3086/0.3046 | 0.9881 [0.8621–0.9924] | 47956/48120 | 0.1242/0.1246 | 1.0068 [0.9846–1.0095] | 157059/157059 |

| Workload | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| memory | 0.1752–0.2019 | 0.0525–0.0601 |
| floating | 0.1729–0.1872 | 0.0477–0.0503 |
| exceptions | 0.1782–0.2870 | 0.2704–0.3137 |
| pruning | 0.2123–0.2942 | 0.0519–0.0539 |
| array1-400 | 0.0800–0.0921 | 0.1274–0.1306 |
| array1-800 | 0.1563–0.1649 | 0.1239–0.1282 |
| array1-1600 | 0.2984–0.3257 | 0.1231–0.1262 |

All seven objects and executables are fully byte-identical across A/B;
[image equivalence](../student.tests/pa29/evidence181/image-equivalence.json)
binds those comparisons. Runtime variation therefore does not establish changed
generated-code performance. Compiler peak RSS increases by at most 1.42% on
these equivalent inputs. Paired compile medians range from 0.9550 to 1.0289;
individual blocks have much wider spread, including a 0.5035-second pruning
compile. The measurements do not establish a repeatable compiler speedup or an
avoidable regression. No unfavorable block is dropped or averaged across suites
to conceal its spread.

## Newly correct zero-extent inputs

| Specializations | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| 400 | 0.0770 [0.0754–0.0791] | 17392 | 0.1232 [0.1224–0.1253] | 1392 | 34651 |
| 800 | 0.1490 [0.1458–0.1570] | 27440 | 0.1272 [0.1260–0.1297] | 1408 | 69051 |
| 1600 | 0.3089 [0.2998–0.3296] | 47696 | 0.1276 [0.1265–0.1302] | 1376 | 137851 |

The [eight launcher observations](../student.tests/pa29/evidence181/launchers.json)
have median 0.00303 s and range 0.00294–0.04544 s. Normal workload times exceed
startup, while the retained launcher outlier illustrates scheduling noise.
Positive and zero-extent inputs have different required storage/copy behavior;
their text difference is not an optimization benefit against equivalent output.

The analyzer checks these equations against all 60 final B array compilations;
[scaling counters](../student.tests/pa29/evidence181/scaling.json) also retain a
representative row per input. N is the number of demanded specializations.

| Family | Parsed nodes | Occurrence nodes | LowIR instructions | Native text bytes | Linked text bytes |
|---|---:|---:|---:|---:|---:|
| extent 1 | 15N+381 | 95N | 25N+84 | 98N+19 | 98N+259 |
| extent 0 | 15N+381 | 95N | 21N+80 | 86N+11 | 86N+251 |

Both families have N type-substitution computations, 3N substitution cache hits,
2N substitution frames and N demanded body transitions. Semantic fact storage
is 442,224 / 859,272 / 1,715,240 bytes at N=400/800/1600; vector capacity accounts
for its nonexact doubling. Full-process RSS includes the remaining indices and
backend state. These counters support work proportional to actual semantic
facts and emitted IR on the measured families; wall times alone do not prove
asymptotic complexity. This is not a measurement of every SFINAE signature or
zero-sized class lifetime combination; their algorithms and correctness controls
are documented in [handoff181](handoff181.md).

## Costs, bounds and stage-scoped acceptance

Canonical Type storage stays **40 bytes**: the new absent-bound flag uses existing
padding. PlacementNew facts grow **112 → 120 bytes**, one logical element-count
multiplier per allocation. [Record measurements](../student.tests/pa29/evidence181/record-sizes.json)
retain both values. The count is necessary because a zero byte stride cannot
recover the number of elements requiring constructor/destructor effects.
Completeness and signature checks follow existing typed declaration/substitution
owners; zero-element initialization emits no absent element work. Zero-size bulk
operations evaluate operands and then emit no invalid zero-span instruction.

No optional transform, speculative specialization or retained optimization body
is added. Optional transform work and growth budgets remain **zero**. The
existing **eight-element** unroll bound remains; larger live arrays use counted
loops, including correct unwind cursors. Required effects, alignment, ABI,
serialization and debug/inspection behavior are covered by the final controls.

The inherited blanket 15% latency/RSS and zero-growth targets remain diagnostic
under spec §9. Their measurements are preserved. Required semantic costs and
newly supported output are recorded above; no optional transform is justified by
size alone or speed against rejected input. This reclassification does not relax
correctness, coverage, mandated bounds or an established avoidable regression.
Mandated limits remain 1,048,576 generated elements, 1,000,000 constexpr steps,
depth 512, source-site identity below 2^31, native frame/data limit 0x70000000,
alignment 4096 and unchanged course timeouts. The existing 32-bit object extent
encoding is now checked before narrowing a producer's 64-bit size; semantic
`sizeof` can still represent larger types when no storage is emitted. General
optimization levels, broad hosted runtime and self-hosting remain later-stage
work.

Reproduction is recorded in the manifest and the checked-in runners. Freeze A/B
at the identified source tips, use the recorded basenames in OUT, build the
current tools and seed evidence `entry.json` before running validation. The
analyzer also expects preserved preliminary JSON and final controls/inspection
subdirectories. Validation writes sequential logs under `/tmp/pa29-181`.
Generated objects, executables and logs are not committed.
