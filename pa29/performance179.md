# Performance179 — local decomposition at PA29 / O0

This change implements required semantics; it adds no optional optimizer and
claims no speedup. Acceptance follows spec §9 at the current stage. Earlier
[performance178](performance178.md) and its inherited measurements are retained.

## Frozen inputs and measurements

The [manifest](../student.tests/pa29/evidence179/manifest.json) binds source tip
`bf487a4195ccc9f744015f8d4ea8a115fd54d246`, entry compiler SHA `022c1bc2829a…`,
final SHA `0f3c56cbde93…`, scripts, exact inputs, outputs and evidence hashes.
Common flags are `-O0 -c --stats`; affected workloads add `-std=c++11`.
Compilation and checked executable execution are timed separately; build, host
link and correctness-suite work occur outside these timers. No CPU affinity was
set. External scheduling is uncontrolled, and all spread/outliers are retained.

- [Common data](../student.tests/pa29/evidence179/common-performance.json): four fixed 2,400-specialization inputs covering loops/calls/memory, floating point, exceptions and unused declarations; AAAA calibration followed by six ABBA blocks per mode, 224 samples.
- [Affected data](../student.tests/pa29/evidence179/owner-performance.json): class/reference, array and destructor-bearing class decomposition at 600/1200/2400 specializations, eight compilations and eight executions each, 144 samples. Entry rejects every input, so these are final-only observations, without a speed ratio against rejection.
- Affected executables consume argv seed 17 and varying loop inputs for 24 million calls. All results match independent Python checksums and GCC C++17 executions. Destructor workloads additionally require 48 million destructor calls. The common runner checks its fixed outputs.
- [Preliminary common data](../student.tests/pa29/evidence179/preliminary-common-performance.json): all 224 earlier observations at `bcef72ad` remain preserved with their original binary hashes. The [first affected attempt](../student.tests/pa29/evidence179/preliminary-owner-attempt.json) stopped before workload timing because of wide-reference narrowing; that shared bug was repaired. That attempt did not persist its launcher timings, and no statistics are inferred from them.

## Equivalent common inputs

Ratios are medians of six paired block ratios, not ratios of unpaired medians.
RSS is the maximum timed ABBA value; calibration RSS remains in raw data.

| Workload | Compile A/B median s | Compile B/A [range] | Compiler RSS A/B KiB | Runtime A/B median s | Runtime B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.3741/0.3681 | 0.9795 [0.9243–1.0632] | 29548/29672 | 0.0960/0.0956 | 0.9864 [0.9543–1.1011] | 151633/151633 |
| floating | 0.4251/0.4109 | 0.9730 [0.9128–1.1086] | 29500/29636 | 0.0825/0.0843 | 1.0109 [0.9548–1.0471] | 151474/151474 |
| exceptions | 0.4441/0.4047 | 0.8634 [0.7495–1.0360] | 29944/30004 | 0.2563/0.2540 | 0.9980 [0.9748–1.2832] | 151781/151781 |
| pruning | 0.2171/0.2260 | 0.9912 [0.8814–1.2649] | 35700/35800 | 0.0510/0.0511 | 1.0012 [0.9911–1.0131] | 151633/151633 |

| Workload | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| memory | 0.3055–0.3372 | 0.0847–0.1083 |
| floating | 0.3890–0.4731 | 0.0822–0.0945 |
| exceptions | 0.3747–0.4508 | 0.5308–0.5867 |
| pruning | 0.2099–0.2676 | 0.0511–0.0524 |

Compiler peak RSS increases by at most 0.46% on these equivalent inputs.
All four common objects and executables are fully byte-identical; the
[image check](../student.tests/pa29/evidence179/elf-comparison.json) verifies that
identity. Runtime differences therefore do not establish changed generated-code
performance. Scheduling variation is large, particularly for exception timing;
neither compiler speedup nor an avoidable regression is established by these runs.

| Preliminary workload | Compile B/A [range] | Runtime B/A [range] |
|---|---:|---:|
| memory | 0.9955 [0.8992–1.4983] | 0.9991 [0.9148–1.0572] |
| floating | 1.0093 [0.9763–1.1336] | 0.9950 [0.9842–1.0063] |
| exceptions | 0.9838 [0.5922–1.0571] | 1.0003 [0.9447–1.0054] |
| pruning | 0.9942 [0.9833–1.1992] | 1.0042 [0.9781–1.0240] |

Preliminary samples use the earlier, explicitly identified compiler and do not
replace final validation. No unfavorable observation was removed.

## Newly correct decomposition

| Workload | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| class600 | 0.1016 [0.1003–0.1027] | 21012 | 0.5737 [0.5492–0.6559] | 1900 | 94360 |
| class1200 | 0.2099 [0.2029–0.2136] | 34560 | 0.5705 [0.5483–0.6322] | 1972 | 188560 |
| class2400 | 0.4356 [0.4030–0.5128] | 62072 | 0.5757 [0.5545–0.6400] | 2220 | 376960 |
| array600 | 0.1022 [0.0996–0.1052] | 21252 | 0.2074 [0.2028–0.2480] | 1912 | 108976 |
| array1200 | 0.2953 [0.2366–0.4629] | 34960 | 0.3942 [0.2918–0.4175] | 1976 | 218176 |
| array2400 | 0.6457 [0.5374–0.6899] | 62540 | 0.5773 [0.4824–0.6794] | 2208 | 436576 |
| lifetime600 | 0.0863 [0.0841–0.0888] | 18592 | 0.6947 [0.6824–0.7112] | 1884 | 78860 |
| lifetime1200 | 0.1658 [0.1636–0.1710] | 30160 | 0.9652 [0.9596–0.9723] | 1936 | 157460 |
| lifetime2400 | 0.3403 [0.3355–0.3478] | 53080 | 1.0483 [1.0408–1.0649] | 1964 | 314660 |

Launcher median 0.00332 s, range 0.0030–0.0504 s; all eight are retained.
Affected workloads dominate launcher time. The larger array and lifetime
specialization sets take longer at a fixed call count; this is disclosed without
a cache/profiler attribution or an unsupported speed claim. They emit distinct
O0 function bodies and required copies/destructors. There is no equivalent
correct entry output against which to assert a new runtime regression.

[Scaling analysis](../student.tests/pa29/evidence179/scaling.json) checks every
one of the 72 affected compilations against these measured equations:

| Family | Parsed nodes | Occurrence nodes | LowIR instructions | Native text bytes | Shapes / members / projections |
|---|---:|---:|---:|---:|---:|
| class | 15N+306 | 73N | 38N+43 | 157N-80 | 1 / 2 / 2N+2 |
| array | 15N+278 | 70N | 42N+43 | 182N-464 | 1 / 0 / 2N+2 |
| lifetime | 15N+354 | 60N | 27N+65 | 131N+20 | 1 / 2 / 2N+2 |

Substitution frames and demanded-body transitions each equal N. Native text
plus 240 bytes equals executable text on these inputs. Binding array capacity
grows geometrically; the recorded binding-storage counter covers those vectors,
not all semantic index allocations. Full process RSS covers the latter as well.
Class shapes retain two field identities once; arrays retain no member list.
The source pattern plus N demanded functions explains the 2N+2 projections.
Counter equations substantiate work proportional to actual facts/emitted IR;
wall-time samples alone do not prove asymptotic bounds.

## Budgets and stage-scoped acceptance

Legality follows the typed initialization and destruction recipes described in
[handoff179](handoff179.md): copies/moves, references, cv and bit-field rules,
default arguments and exceptional cleanup remain observable. No optional
optimization pass, speculative specialization or retained optimization body was
added. Optional transform work and growth budgets remain **zero**. Array-copy
emission has an explicit **eight-leaf** bound, then one counted loop with a
completed-prefix cursor; required runtime visits every element. Constant copies
consume the existing evaluator budget. Canonical shape caching and typed
projections avoid repeated member discovery or synthetic syntax.

The inherited blanket 15% latency/RSS and zero-growth targets remain diagnostic
under spec §9. All measurements are preserved. Necessary semantic storage/code
is measured above; no optional unprofitable transform is being retained. This
does not relax correctness or excuse an established avoidable regression.
Mandated capacities remain 1,048,576 generated elements, 1,000,000 constexpr
steps, depth 512, source-site identity below 2^31, native frame/data limit
0x70000000, alignment 4096 and unchanged course timeouts. General optimizer
levels, broad hosted C++ runtime and self-hosting remain owned by later stages.

Reproduction (freeze A/B first and seed OUT entry.json from evidence179):

```sh
python3 student.tests/pa29/test179.py OUT/controls-complete B
python3 student.tests/pa29/inspect179.py OUT/inspection-complete
python3 student.tests/pa29/validate179.py student.tests/pa29/evidence179
python3 student.tests/pa27/performance147_common.py OUT/common-complete A B
python3 student.tests/pa29/performance179.py OUT/owner-complete A B
python3 student.tests/pa29/analyze179.py OUT
```

The analysis also expects retained preliminary files and frozen images with
the basenames recorded in its source. No generated object or executable is
committed; JSON contains commands, samples, counters and hashes.
