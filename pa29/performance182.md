# Performance182 — accumulated PA29 checkpoint audit

Acceptance is PA29/O0 under spec §9. Code tip `52070178897f5894edaf2f35d03a734b781979d4` fixes array decomposition qualifiers and moves mandatory inline preparation to its single native owner. It adds no optional transform and claims no speedup. Historical [performance179](performance179.md), [performance180](performance180.md) and [performance181](performance181.md) remain unchanged.

## Protocol and frozen evidence

The [manifest](../student.tests/pa29/evidence182/manifest.json) binds the tested source and frozen final compiler (`27de74d246ac…`). [Common data](../student.tests/pa29/evidence182/common-performance.json) compares the previous reviewed compiler (`022c1bc2829a…`, identical source at `667edd80` and documentation-only `cd283a59`) with final B. [Owner data](../student.tests/pa29/evidence182/owner-performance.json) compares audit entry A (`904c8098efb5…`, `9211517d`) with final B. These are different, explicitly named baselines.

Common flags are `-O0 -c --stats`; owner flags additionally specify `-std=c++17`. Compiler and executable timings are separate wall-time observations with peak RSS from `/usr/bin/time`. Build, host linking and initial correctness checks are outside timing. Equivalent inputs use four A/A samples followed by six ABBA blocks for each mode. Corrected cv-array inputs use eight final compilations and eight executions per size; entry rejects their required type assertions, so rejection is not a performance baseline.

There are **440 final observations** (224 common, 216 owner) and **eight launcher observations**. All inputs, hashes, arguments, outputs, counters and samples are retained. No affinity is set; external scheduling remains uncontrolled. Owner programs take argv seed 17, execute 12 million varying calls, and match independent Python checksums and GCC executions. Common inputs retain the fixed 2,400-specialization frontend load with live loops/calls/memory, floating point, exceptions and dormant declarations. Launcher median is 0.00359 s, range 0.00330–0.00425 s; measured work dominates startup.

## Equivalent implementations

Ratios are medians of six paired block ratios, with their full range. RSS is maximum ABBA compiler RSS. Full calibration RSS and runtime RSS are retained in JSON. Text is linked executable `.text`.

| Input / baseline | Compile A/B s | Compile B/A [range] | RSS A/B KiB | Runtime A/B s | Runtime B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory / review | 0.1738/0.1755 | 1.0121 [0.7935–1.2074] | 29700/29740 | 0.0525/0.0528 | 1.0038 [0.9656–1.2431] | 151633/151633 |
| floating / review | 0.1682/0.1714 | 1.0263 [0.8911–1.0714] | 29756/29820 | 0.0482/0.0483 | 0.9922 [0.9855–1.0046] | 151474/151474 |
| exceptions / review | 0.1742/0.1707 | 1.1001 [0.8569–1.4166] | 29268/29288 | 0.2588/0.2544 | 0.9716 [0.8816–1.0996] | 151781/151781 |
| pruning / review | 0.2136/0.2138 | 0.9816 [0.7645–1.0184] | 35888/35972 | 0.0521/0.0519 | 1.0016 [0.9730–3.4794] | 151633/151633 |
| straight2400 / entry | 0.1863/0.1854 | 1.0043 [0.9377–1.1409] | 33916/33764 | 0.0677/0.0669 | 0.9892 [0.9361–1.0642] | 122451/122451 |
| branch1200 / entry | 0.2021/0.1899 | 0.9209 [0.7422–1.0659] | 41604/41692 | 0.1300/0.1289 | 0.9936 [0.9391–1.1213] | 204455/204455 |
| floating1200 / entry | 0.1639/0.1691 | 1.0254 [0.9524–1.0694] | 30556/30600 | 0.1641/0.1620 | 0.9938 [0.9759–1.0127] | 193251/193251 |

| Input | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| memory | 0.1669–0.2019 | 0.0525–0.0540 |
| floating | 0.1652–0.2101 | 0.0482–0.0494 |
| exceptions | 0.1607–0.1689 | 0.2600–0.3147 |
| pruning | 0.2051–0.2061 | 0.0508–0.0519 |
| straight2400 | 0.1762–0.1958 | 0.0654–0.0661 |
| branch1200 | 0.1885–0.2268 | 0.1284–0.1294 |
| floating1200 | 0.1599–0.1661 | 0.1609–0.1636 |

All seven equivalent object pairs are byte-identical. Common executables are also identical; the three owner executables differ only in `.strtab` (the linker’s local object-file name), with identical executable, data and unwind sections. The [section comparison](../student.tests/pa29/evidence182/image-equivalence.json) records every section. Runtime timing differences therefore do not establish changed generated-code performance.

The common exception compile paired median is 1.1001, while its unpaired final median is lower; paired ratios cross unity (0.8569–1.4166), and both directions of scheduling outlier remain visible. Pruning runtime retains a 0.3077 s final outlier and a 3.4794 paired ratio despite identical images. These data do not establish repeatable avoidable regressions or a speedup. Peak compiler RSS rises at most 0.24% on common inputs and 0.22% on equivalent owner inputs. Nothing was dropped to obtain a passing aggregate.

## Newly correct array qualifiers

| Specializations | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | 0.0966 [0.0952–0.1008] | 20688 | 0.0994 [0.0974–0.1117] | 1908 | 103744 |
| 1200 | 0.2170 [0.1843–0.3083] | 34060 | 0.1116 [0.1067–0.1233] | 1964 | 206944 |
| 2400 | 0.4225 [0.3884–0.5729] | 60700 | 0.2719 [0.2681–0.3176] | 2252 | 413344 |

[Scaling](../student.tests/pa29/evidence182/scaling.json) checks all 24 final cv-array compilations: parsed nodes = 15N+284; projected occurrences = 76N; total nodes = 91N+284; input and prepared LowIR instructions = 36N+43; prepared operands = 57N+71; native text = 172N+304 (linked text adds 240 bytes). There is one canonical binding shape, no member sequence, and 2N+2 projections. Binding-vector capacities grow geometrically: 32,792 / 65,560 / 131,096 bytes. Existing specialization frames and graph ownership remain unchanged.

The 2,400-specialization runtime is larger at the same call count; this is disclosed without a hardware/cache attribution. It emits distinct required O0 bodies and copies. Entry did not implement the same type semantics, so no runtime ratio against its rejected input is meaningful. The audit does not infer performance from IR size alone.

## Legality, budgets, inherited costs and disposition

Array qualifiers follow the hidden-object type rule; retaining volatile qualifiers also preserves volatile loads/stores. The ten-element control consumes one bounded copy loop; copies of zero-sized classes still invoke their selected constructors/destructors. No optional copy elimination or stronger alias/effect promise was introduced.

Mandatory inlining is required by [LowIR force_inline](../pa8/lowir.md), including at O0. Its legality checks, immutable input-body maps, EH/phi remapping, recursion/depth/frame fallbacks and admission costs remain unchanged. Source LowIR now represents input to that transform. Both direct and serialized paths expand exactly once at native preparation, preserving the same mandatory weak-body behavior and avoiding a second allowance at serialization. The source view is roundtripped separately from the prepared view; source/adapter machine code, relocations, symbols, counters and runtime must agree.

Limits remain **64 nesting levels**, **262,144 reserved units/caller**, **4,194,304/program**; each unit bounds instruction/operand/slot work and associated proportional values/blocks. The growth control reserves 2,113,063 program units, at most 262,143 per caller, with 497,214 actual units and 64 conservative declines. Depth and recursion controls also retain valid calls. Final equivalent owner maxima are 85,200 reserved and 73,200 actual units; no benchmark call is declined. Optional-transform work/growth budgets remain zero.

Telemetry follows its owner: `instructions`/`operands` now describe pre-preparation source LowIR; `prepared_instructions`/`prepared_operands`, inline counters and `preparation_ms` describe the shared native stage. Total driver time/RSS includes both. Historical counters are preserved with their original meanings. Stats-on/off objects agree. No analysis is triggered solely for reporting.

The [historical review](../student.tests/pa29/evidence182/historical-review.json) verifies 53 manifest artifacts and all **2,576** retained performance observations (1,360 final, 1,216 preliminary). In particular, performance180’s mandatory branch expansion cost of 22.5% compile latency / 55.1% RSS and floating expansion’s 5.2% text growth remain disclosed alongside their runtime results. They are required attribute costs, not optional transforms retained without profit. The audit repair preserves those objects.

Inherited blanket 15% latency/RSS and zero-growth targets remain diagnostic under spec §9, not extra exit gates. No measurement, required behavior, coverage or mandated limit is waived. Existing generated-element, constexpr step/depth, source identity, object-width, native frame/data and alignment checks remain enforced. General O1–O3 optimization, broad hosted-library runtime and self-host compilation retain PA32/33, PA30/31 and PA34 scope; they add no PA29 completion gate.

Reproduction (freeze the named binaries first):

```sh
python3 student.tests/pa27/performance147_common.py OUT/common REVIEW FINAL
python3 student.tests/pa29/performance182.py OUT/owner ENTRY FINAL
python3 student.tests/pa29/test182.py OUT/controls FINAL
python3 student.tests/pa29/inspect182.py OUT/inspection OUT/inspection180/ir-object
python3 student.tests/pa29/validate182.py OUT
```
