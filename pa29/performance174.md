# PA29 checkpoint audit174 performance evidence

Reviewed code: `7139ceb5769eea12c6e2e00b54932f09d59c79b5`. Acceptance is **PA29/O0**. This audit fixes
required semantic/lifetime and ABI behavior; it introduces no optional optimizer
transform and claims no speedup.

[Manifest](../student.tests/pa29/evidence174/performance-manifest.json) freezes
binaries, inputs, flags, scripts and environment. Compiler sizes are **4,109,848**
bytes at the previous review, **4,187,928** at audit entry, and **4,188,000** final:
+78,152 (1.902%) over the review range and +72 (0.00172%) for the audit fixes.
All final correctness checks and measurements use the same compiler hash.

There are **1,048** final observations: 224 previous-review/final common samples,
224 entry/final common samples, 448 entry/final affected samples, 96 required-
correctness scaling samples, and 56 follow-up pruning samples, plus **six**
launcher observations. All **168** preliminary common observations on the
pre-ABI-fix binary remain; that run was deliberately stopped when symbol inspection
found another ownership defect, and does not serve as final acceptance evidence.

Host linking is excluded from timed compilation. Flags are `-O0 -c --stats`, plus
`-std=c++11` for affected inputs. g++ 15.2.0 links executables. Inputs are frozen by
hash; common inputs retain PA26/27's 2,400 demanded template functions, loops,
calls, memory, floating point and exception cleanup. Affected inputs retain the
171/172/173 hashes. Every timed execution checks an independently known result
using argc-dependent values. Builds and correctness suites finished before timing;
measurement series ran sequentially. CPU affinity was unset. Process launch,
external scheduling and documentation/evidence work are included or uncontrolled;
no outlier was removed. Self-hosting remains PA34 work.

## Equivalent checked workloads

Each workload/mode has four A/A samples followed by six ABBA blocks. Tables use
median seconds and maximum compiler RSS in KiB; paired ratios are each block's
mean B / mean A, with median and full range. A/A samples are excluded from A/B
medians. Raw records retain every phase time, RSS, work counter and image hash.
Both implementations correctly execute these benchmark programs; the independent
symbol/link tests assess ABI compatibility beyond their same-compiler execution.

### Accumulated review range

A is previous-reviewed `221d6d0e`; B is final. [Raw observations](../student.tests/pa29/evidence174/cumulative-performance.json).

| Workload | Compile A/B s | Paired B/A [range] | Compiler RSS A/B KiB | Runtime A/B s | Paired B/A [range] | Text bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.3160/0.2859 | 0.9589 [0.8051–1.1438] | 29348/29584 | 0.0855/0.0839 | 0.9809 [0.8871–1.0299] | 151633 |
| floating | 0.4489/0.3705 | 0.9642 [0.7953–1.0092] | 29252/29652 | 0.0764/0.0784 | 1.0199 [0.9705–1.0653] | 151474 |
| exceptions | 0.3883/0.3691 | 0.9340 [0.8792–1.0209] | 29228/29704 | 0.5156/0.5293 | 1.0270 [0.9274–1.1251] | 151781 |
| pruning | 0.2512/0.2468 | 0.9825 [0.8252–1.6597] | 35096/34908 | 0.0519/0.0522 | 0.9996 [0.9822–1.0157] | 151633 |

### Audit changes

A is entry `914e1a0a`; B is final. [Raw observations](../student.tests/pa29/evidence174/audit-performance.json).

| Workload | Compile A/B s | Paired B/A [range] | Compiler RSS A/B KiB | Runtime A/B s | Paired B/A [range] | Text bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1674/0.1692 | 1.0020 [0.9308–1.9613] | 29316/29512 | 0.0516/0.0516 | 0.9947 [0.9663–1.0090] | 151633 |
| floating | 0.1750/0.1848 | 1.0318 [0.9724–1.4041] | 29360/29492 | 0.0483/0.0483 | 0.9935 [0.9744–1.0027] | 151474 |
| exceptions | 0.1692/0.1677 | 0.9808 [0.6185–1.1156] | 29300/29500 | 0.2572/0.2532 | 1.0311 [0.9218–1.1171] | 151781 |
| pruning | 0.2089/0.2233 | 1.0746 [1.0193–1.2653] | 34904/35040 | 0.0515/0.0516 | 0.9922 [0.9335–1.0126] | 151633 |

### Affected inherited owners

A is entry `914e1a0a`; B is final. [Raw observations](../student.tests/pa29/evidence174/affected-performance.json).

| Workload | Compile A/B s | Paired B/A [range] | Compiler RSS A/B KiB | Runtime A/B s | Paired B/A [range] | Text bytes |
|---|---:|---:|---:|---:|---:|---:|
| selection1200 | 0.2106/0.1964 | 0.9506 [0.5725–0.9850] | 31952/31964 | 0.1171/0.1187 | 1.0100 [0.9715–1.0287] | 84521 |
| sequence2400 | 0.0342/0.0334 | 1.0045 [0.9265–1.0960] | 12104/12260 | 0.1074/0.1070 | 0.9990 [0.9728–1.0294] | 484 |
| integer-pack2400 | 0.0361/0.0359 | 0.9902 [0.9629–1.0155] | 12576/12716 | 0.1113/0.1111 | 0.9989 [0.9754–1.0554] | 484 |
| runtime1200 | 0.0956/0.0943 | 1.0058 [0.9456–1.9757] | 20280/20676 | 0.3169/0.3158 | 0.9951 [0.9613–1.0420] | 79341 |
| query24000 | 0.1588/0.1603 | 0.9972 [0.9902–1.2868] | 31012/31032 | 0.1452/0.1446 | 1.0024 [0.9870–1.0178] | 491 |
| ordinary1200 | 0.2713/0.2710 | 0.9899 [0.7519–1.1085] | 40344/40532 | 0.3189/0.3214 | 1.0054 [0.9799–1.0418] | 190939 |
| closures1200 | 0.4027/0.4066 | 1.0214 [0.9859–1.1964] | 61792/62000 | 0.3355/0.3336 | 0.9986 [0.9835–1.0033] | 254155 |
| calls1200 | 0.0940/0.0946 | 1.0157 [0.9804–1.6267] | 19992/20028 | 0.1899/0.1894 | 1.0771 [0.8527–1.1889] | 80602 |

All common objects/executables are byte-identical. All equivalent affected text
sizes are unchanged. The two sequence objects retain the same size but acquire
correct pack grouping in their function names; their executable differences are
confined to symbol/string tables and build ID. `calls1200` objects are identical;
its executable differs only in the linker string table. The executable `.text`
bytes match in every one of these cases, verified in
[ELF comparison](../student.tests/pa29/evidence174/elf-comparison.json). Object byte
identity is recorded as a diagnostic, not imposed as a gate against required ABI
corrections. Checked runtime results, machine text and host-link controls remain
mandatory evidence.

All 291 previously existing common work counters remain equal across the review
range; the five newly exposed counters are zero. All 296 common counters are
unchanged across the audit, including the pruning follow-up. Affected counters
are unchanged except one additional canonical ABI argument-pack node in each
sequence generator. [All-sample comparison](../student.tests/pa29/evidence174/counter-delta.json)
checks deterministic facts in every compile observation, not just one chosen run.

The initial audit pruning compile ratio was **1.0746 [1.0193–1.2653]**, so it was
investigated instead of dismissed because it met an arbitrary percentage. An
isolated repeat with the exact binaries, source and flags gives **0.9817
[0.8484–1.0512]**, compile medians **0.2010/0.2005 s**, RSS **34988/35064 KiB**.
Runtime is **0.0510/0.0514 s**, ratio **1.0141 [0.9921–1.0775]**, unchanged text
**151633 bytes**. [All repeat samples](../student.tests/pa29/evidence174/repeat-performance.json)
remain. The discrepancy is not repeatable in these observations and all work
counts are unchanged; no avoidable compiler regression is established. The
selection input's initial improvement is likewise not claimed as a speedup.
Other paired ranges cross 1, with runtime variation even for identical code.

### A/A noise calibration

| Series / workload | Compile range s | Runtime range s |
|---|---:|---:|
| cumulative / memory | 0.2959–0.3361 | 0.0729–0.0850 |
| cumulative / floating | 0.2676–0.4113 | 0.0820–0.0856 |
| cumulative / exceptions | 0.3351–0.3624 | 0.4519–0.5419 |
| cumulative / pruning | 0.4159–0.4860 | 0.0517–0.0546 |
| audit / memory | 0.1615–0.1678 | 0.0514–0.0526 |
| audit / floating | 0.1649–0.2142 | 0.0481–0.0486 |
| audit / exceptions | 0.1700–0.4040 | 0.2514–0.2631 |
| audit / pruning | 0.1998–0.2078 | 0.0519–0.0581 |
| affected / selection1200 | 0.1857–0.2005 | 0.1163–0.1175 |
| affected / sequence2400 | 0.0338–0.0352 | 0.1069–0.1141 |
| affected / integer-pack2400 | 0.0357–0.0378 | 0.1099–0.1129 |
| affected / runtime1200 | 0.0927–0.0943 | 0.3131–0.3769 |
| affected / query24000 | 0.1585–0.1614 | 0.1437–0.1494 |
| affected / ordinary1200 | 0.2581–0.2717 | 0.3130–0.3249 |
| affected / closures1200 | 0.4059–0.4568 | 0.3302–0.3366 |
| affected / calls1200 | 0.0873–0.0888 | 0.1852–0.1860 |
| repeat / pruning | 0.1994–0.2341 | 0.0510–0.0513 |

## Required-correctness costs and scaling

The entry compiler compiles these inputs but executes every one incorrectly.
Their omitted copies or invented captures make entry/final runtime ratios
semantically invalid; [entry results](../student.tests/pa29/evidence174/affected-performance.json)
are retained explicitly. Final measurements use eight samples per size and mode.
`discardN` expands N volatile class operands and verifies all required copy and
destruction effects across roughly 20 million operations. `captureN` demands N
closures whose local declaration queries an enclosing Shape only through decltype,
then checks 20 million runtime calls without capture copies. Required calls and
checks remain live at every size.

| Workload | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| discard600 | 0.0331 [0.0302–0.1324] | 11980 | 0.1968 [0.1888–0.2105] | 3380 | 43810 |
| discard1200 | 0.0577 [0.0547–0.0844] | 16540 | 0.2688 [0.2598–0.2741] | 3624 | 87010 |
| discard2400 | 0.1073 [0.1032–0.1259] | 26088 | 0.4107 [0.3996–0.4737] | 3600 | 173410 |
| capture600 | 0.1540 [0.1469–0.1610] | 27976 | 0.3329 [0.3190–0.3524] | 1420 | 86035 |
| capture1200 | 0.3055 [0.2958–0.4212] | 48160 | 0.3281 [0.3222–0.3432] | 1672 | 171835 |
| capture2400 | 0.6680 [0.6177–0.7136] | 89500 | 0.3313 [0.3193–0.3383] | 1776 | 343435 |

Launcher median **0.00343 s**, range **0.00319–0.02700 s**. The shortest audit
correctness compile median is **9.7×** launcher median, and the shortest runtime
median is **57.5×**. Small sequence compilations are startup-sensitive; the
inherited larger-generator measurements remain the scaling evidence for that
owner. No timing exponent or statistical significance is claimed.

[All-sample scaling assertions](../student.tests/pa29/evidence174/scaling-counters.json),
reproduced by `python3 student.tests/pa29/analyze174.py`, establish measured work:

- `discardN`: **2N−1** fold steps, **5N+11** query work, **8N+92** full-expression
  work, **N+2** regions, four specializations and five body checks. Parsed source
  remains 288 nodes/four retained regions; occurrence projections are **12N+51**.
  Executable text is **72N+610** bytes for the required expanded effects.
- `captureN`: **N** closures, one capture recipe, zero candidates/edges, **2N**
  specializations/body transitions, **2N+4** body checks, **N+3** query work and
  **18N+43** full-expression work. There are three retained source regions;
  demanded occurrence projections are **175N**. Text is **143N+235** bytes.

These counts and text formulas hold at 600/1200/2400 in all eight compile samples.
IR capacity grows geometrically with emitted operations; RSS and text grow with
required expansion/specialization work. Larger discard bodies run slower at equal
operation counts, disclosed above; there is no claim of optimization benefit over
an implementation that illegally omits effects. The O0 policy retains explicit
calls, cleanup and memory accesses. Capture scanning is not multiplied by call
count. Function MIR/frame/disassembly evidence is in the [audit](audit.md).

## Historical plans, budgets and acceptance

[Historical review](../student.tests/pa29/evidence174/historical-performance-review.json)
hashes all raw measurements from implementations171–173: **1,912** main and **44**
launcher observations, including preliminary173 binaries. The reports remain
unchanged: [171](performance171.md), [172](performance172.md), [173](performance173.md),
with the earlier [170](performance170.md) baseline. In particular, implementation173
records ordinary closure object metadata growing **936032 → 1181992 bytes**
(26.28%) for required weak/COMDAT ODR identity, with executable text unchanged at
**190939**. The present audit preserves that valid behavior; it adds no further
ordinary-object growth. Historical generator timings that were too short remain
alongside the larger 6000/12000/24000 samples.

Spec §9's stage-scoped rule applies to those inherited plans as well as this audit.
Historical blanket **15% latency/RSS** and **zero-growth** targets are self-selected
diagnostics, not additional PA29 gates. Required semantic copies, correct capture
storage and ABI metadata are necessary costs; correctness is not traded against
an incorrect faster baseline. No measurement is erased and no mandated limit or
comparison rule is weakened. Equivalent workload work counts and text remain
stable; the isolated pruning check does not reproduce the suspected regression.
No new optional transformation is retained on an unproven benefit.

Optional optimizer work/code-growth budgets are **zero** at this change. Required
fold work is linear in retained expansion/conversion edges; capture work follows
source recipes and actual dependencies; ABI emission follows argument/output size.
Complete canonical keys and explicit monotonic fact states own memoized work;
there is no global retry/invalidation. Scratch ends with the operation, semantic
storage with the TU, and MIR selection/allocation with each function. Existing
bounds remain **1,048,576** generated elements, **1,000,000** constexpr steps and
**512** call depth, native frame/data **0x70000000**, alignment **4096**, and course
timeouts. Failed semantic proof remains conservative IR or the required diagnostic.
Heavier hosted runtime, higher optimization levels, advanced allocation and
self-hosting remain PA30–34 responsibilities, without creating a PA29 exit gate.

Reproduce using the frozen hashes in the manifest:

```sh
python3 student.tests/pa27/performance147_common.py OUT_CUMULATIVE PREVIOUS_REVIEW FINAL
python3 student.tests/pa27/performance147_common.py OUT_AUDIT ENTRY FINAL
python3 student.tests/pa29/performance174.py OUT_AFFECTED ENTRY FINAL
python3 student.tests/pa29/repeat174.py OUT_REPEAT OUT_AUDIT
python3 student.tests/pa29/analyze174.py
```
