# PA30 implementation200 performance acceptance

Code tip: `37b6729d`. Frozen A is entry `377d92a0`; B is the final compiler.
[Source/binary binding](../student.tests/pa30/evidence200/source-binding.json)
and [ownership trace](design200.md) identify the measured code. These are
required semantic repairs at O0; no optional optimization or speedup is claimed.

## Protocol and stage scope

Flags: `-O0 -c --stats`, CPU 2 affinity. All compiler hashes, fixed input hashes,
wall times, RSS, phase counters, object/executable hashes and text sizes are
retained. Four common workloads use one AAAA calibration plus six ABBA blocks,
separately for compilation and execution. Runtime argc/argv and checked
checksums keep executable work observable. Host g++ only links emitted objects.
The final sequence began after builds, correctness reports and controls ended.

There are **312 observations plus 16 launch calibrations**:
[common](../student.tests/pa30/evidence200/common-performance.json) 224,
[owner scaling](../student.tests/pa30/evidence200/owner-performance.json) 48,
[repaired hosted fixtures](../student.tests/pa30/evidence200/hosted-performance.json)
40. Entry rejects the corrected owner/hosted workloads; those report absolute
final costs, never speedups against failed compilations. No sample is discarded.

The PA30 **45-second per-compile limit** remains mandatory. Historical blanket
15% latency/zero-growth diagnostic targets remain non-gates under spec §9;
performance195–199 and their raw observations are preserved. No numeric RSS
percentage is mandated. Existing generator (1,048,576 elements), evaluator,
inline and native expansion bounds are unchanged. New work is proportional to
selected subobject destruction edges and source bodies; query dependence uses
constant-time declaration flags. No optional code growth or optimization is
introduced. PA31 hosted runtime completion, PA32/33 optimization policies and
PA34 self-hosting remain later-stage work, not additional PA30 exit gates.

## Equivalent common programs

Every A/B object and executable is byte-identical. These fixed workloads include
2,400 demanded templates, loops, calls, memory, floating point, exception cleanup
and unused-function pruning. Runtime/text equivalence establishes that compiler
changes do not trade execution quality for lower compiler work on these inputs.

| Workload | Compile B/A [range] | Runtime B/A [range] | Compile RSS A/B KiB | Object / executable text bytes |
|---|---|---|---|---|
| memory | 1.014 [0.988, 1.166] | 0.999 [0.970, 1.003] | 29680 / 29796 | 151393 / 151633 |
| floating | 0.996 [0.990, 1.411] | 1.003 [0.961, 1.029] | 29696 / 29920 | 151234 / 151474 |
| exceptions | 0.996 [0.713, 1.016] | 1.001 [0.989, 1.018] | 29236 / 29272 | 151541 / 151781 |
| pruning | 0.996 [0.800, 0.999] | 1.003 [0.987, 1.013] | 35788 / 35800 | 151393 / 151633 |

| Workload | Compile A/B median ms | Runtime A/B median ms | Compile A/A range ms | Runtime A/A range ms |
|---|---|---|---|---|
| memory | 168.21 / 169.33 | 52.11 / 52.11 | 169.42–259.53 | 52.09–52.67 |
| floating | 168.19 / 167.76 | 49.21 / 49.28 | 167.48–170.41 | 49.30–49.39 |
| exceptions | 173.04 / 170.64 | 254.07 / 254.47 | 167.64–170.81 | 252.51–257.40 |
| pruning | 208.28 / 207.20 | 52.56 / 52.60 | 210.38–212.78 | 52.58–52.76 |

Compilation has isolated timing outliers, retained in the paired ranges. The
common paired medians and A/A observations do not establish a repeatable
avoidable regression. Generated code is identical, so runtime variation here
is measurement noise. Required new semantics are measured separately below;
no reduced-IR or blanket percentage claim is used to justify added work.

## Completed owner scaling

Each independent source family has an ordinary enclosing class, nested template
body, later-declared nested constructor and member alias sequence generator.
The body is checked in its enclosing complete-class context; a demanded
specialization uses both generated packs to select a delegating constructor.
The executable runs 3,000,000 argv-dependent checked transitions through that
constructor/call chain. Entry rejects the same input before code generation.

| N | Compile median [range] s | Peak RSS KiB | Runtime median [range] s | Object / executable text bytes |
|---:|---|---:|---|---|
| 64 | 0.0585 [0.0581, 0.0603] | 14488 | 0.0690 [0.0685, 0.0694] | 557 / 797 |
| 256 | 0.2147 [0.2114, 0.2840] | 34836 | 0.0692 [0.0688, 0.0694] | 557 / 797 |
| 1024 | 0.8616 [0.8534, 0.8780] | 115956 | 0.0689 [0.0684, 0.0694] | 557 / 797 |

Every repetition records lookup work `133N+49`, type-query work `11N+8`, class
completions `4N+1`, delimiter work `207N+97`, and maximum pending lookahead 132.
All output hashes match across repetitions and N. Increasing type-only families
does not emit additional code. This supports the indexed declaration/fact
ownership and linear work model without introducing broader invalidation.

Compile launcher: median 5.95 ms [5.73, 6.75].
Runtime launcher: median 4.42 ms [4.32, 4.61].

## Repaired hosted compile costs

| Fixture | Compile median [range] s | Peak RSS KiB | Object text bytes |
|---|---|---:|---:|
| 600-hosted-recursive-std-function-string-substr | 0.8590 [0.8382, 0.8700] | 54600 | 24188 |
| 600-shared-ptr-allocator-shadowing | 1.0126 [0.9575, 1.0902] | 57152 | 7377 |
| 700-hosted-function-capturing-lambda-compile | 0.2919 [0.2882, 0.2951] | 24292 | 2159 |
| 700-hosted-function-nullary-base-reentry-compile | 0.8609 [0.8510, 0.8631] | 55824 | 28006 |
| 700-hosted-function-typeid-compare-compile | 0.3037 [0.2973, 0.3079] | 24332 | 1860 |
| 700-hosted-map-subscript-piecewise-construct-compile | 0.5744 [0.5700, 0.6030] | 43184 | 7737 |
| 700-hosted-piecewise-pair-index-sequence-alias | 0.8370 [0.8326, 1.0134] | 54168 | 20556 |
| 700-hosted-result-of-bind-member-template-callable-compile | 0.3456 [0.3202, 0.5516] | 27532 | 1873 |
| 700-hosted-std-result-of-nested-callable-pack-compile | 0.0603 [0.0600, 0.0617] | 11424 | 519 |
| 700-libstdcxx-regex-compiler-member-alias-call | 2.4537 [2.4369, 2.5092] | 177060 | 222302 |

Maximum hosted compile: **2.509 s**, **177060 KiB** peak RSS; all four repetitions of each fixture emit identical objects. The 45-second limit is preserved.

PA30 discards these hosted objects; their link/runtime completion is PA31 work.
Applicable cleanup/allocation/construction semantics are nevertheless checked
by direct object and serialized LowIR execution controls, including a throw
partway through aggregate union initialization. Whole-stage completion remains
blocked by five required fixture failures; performance evidence does not waive
them or the independent review of this implementation delta.
