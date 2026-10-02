# PA30 implementation201 performance acceptance

Code tip: `fc8ea8d6`. Frozen A is entry `b0790726`; B is the final compiler.
[Source/binary binding](../student.tests/pa30/evidence201/source-binding.json)
and [ownership trace](design201.md) identify the measured code. Changes supply
required O0 semantic/CFG checks; no optional optimization or speedup is claimed.

## Protocol and stage scope

CPU 2 affinity, flags `-O0 -c --stats`, frozen binaries/inputs, one AAAA noise
calibration and six ABBA blocks per equivalent workload, separately for
compilation and execution. All hashes, observations, phase counters, checked
outcomes and text sizes are retained. Compilation and execution are independent
measurements; argv/argc and checksums keep runtime work observable. Host g++
only links emitted objects. Timing began after builds, reports and controls.

There are **420 observations plus 16 launcher calibrations**:
[common](../student.tests/pa30/evidence201/common-performance.json) 224,
[owner scaling](../student.tests/pa30/evidence201/owner-performance.json) 168,
and [hosted/fixed outcomes](../student.tests/pa30/evidence201/hosted-performance.json)
28. Correct rejection costs are final-only; accepting invalid code is not an
A/B performance baseline. No sample or outlier is discarded.

The **45-second per-compile limit** is mandatory. Historical blanket 15% latency
and zero-growth targets are diagnostics under spec §9; performance195–200 and
all raw observations remain. No numeric RSS percentage is mandated. Existing
language/expansion/native bounds are unchanged. The new reachability proof has
a fixed number of linear passes, deduplicated worklists and function-local
storage: O(instructions + edges), no fixed point or code growth. Isolated joins
avoid further analysis; ordinary complete-return bodies need none. Unknown
integer values retain both edges. This proof neither instantiates constexpr
bodies nor changes IR. Noreturn and unwind remain separate signature facts.

## Equivalent common programs

All A/B objects and executables are byte-identical. The fixed workloads cover
2,400 demanded templates, loops, calls, memory, floating point, exception
cleanup and unused-function pruning.

| Workload | Compile B/A [paired range] | Runtime B/A [paired range] | Compile RSS A/B KiB | Object / executable text bytes |
|---|---|---|---|---|
| memory | 0.987 [0.850, 1.189] | 1.002 [0.955, 1.024] | 29636 / 29620 | 151393 / 151633 |
| floating | 1.013 [0.884, 1.065] | 1.000 [0.986, 1.010] | 29740 / 29856 | 151234 / 151474 |
| exceptions | 1.003 [0.666, 1.034] | 1.013 [0.885, 1.328] | 29188 / 29204 | 151541 / 151781 |
| pruning | 1.006 [0.986, 1.432] | 0.999 [0.991, 1.001] | 35668 / 35664 | 151393 / 151633 |

| Workload | Compile A/B median ms | Runtime A/B median ms | Compile A/A range ms | Runtime A/A range ms |
|---|---|---|---|---|
| memory | 168.84 / 168.44 | 52.73 / 53.11 | 166.45–205.60 | 52.62–53.70 |
| floating | 171.70 / 174.61 | 49.96 / 49.50 | 170.14–179.98 | 49.27–50.14 |
| exceptions | 173.48 / 174.26 | 255.18 / 259.72 | 169.68–177.37 | 250.48–262.61 |
| pruning | 217.79 / 217.73 | 52.46 / 52.45 | 216.41–222.38 | 52.50–52.75 |

Compiler paired medians are close to unity; outliers remain in the ranges.
The exceptions runtime has particularly broad outliers. Identical executable
bytes rule out a generated-code regression on these inputs. These observations
do not establish an avoidable compiler regression, and no noise-derived speedup
is claimed. Required validation costs are measured directly below.

## Completed-owner scaling

Each independent family demands a function template, local-class constant use,
a captured lambda, a constant arithmetic loop, noreturn calls and an exception
handler. All A/B programs pass the same checks and are byte-identical. The main
loop performs 3,000,000 argv-dependent transitions through the selected family;
external function definitions keep the other families' emission work visible.

| N | Compile A/B median s | Compile B/A [paired range] | Peak RSS A/B KiB | Runtime B median s | Runtime B/A [paired range] | Object / executable text bytes |
|---:|---|---|---|---|---|---|
| 64 | 0.0583 / 0.0584 | 0.988 [0.910, 0.995] | 13580 / 13592 | 0.0773 | 1.008 [0.997, 1.009] | 35712 / 35952 |
| 256 | 0.2131 / 0.2159 | 1.015 [0.989, 1.039] | 32056 / 32688 | 0.0770 | 1.000 [0.985, 1.004] | 141696 / 141936 |
| 1024 | 0.8627 / 0.8727 | 1.012 [0.948, 1.044] | 106168 / 105976 | 0.0771 | 1.000 [0.989, 1.003] | 565632 / 565872 |

Every B repetition records **3N** checked fallthrough functions, **274N**
instruction visits and **35N** edge visits. The linear counters and bounded
RSS support the function-local work model. Code growth with N is the same
external definitions in A and B; the checking itself emits no additional code.
The small measured compiler cost buys required rejection and preserves valid
control flow; there is no optional transform to remove or unbounded search.

Compile launcher: median 5.98 ms [5.76, 6.74].

Runtime launcher: median 4.68 ms [4.55, 4.84].

## Repaired outcomes and retained hosted costs

| Fixture | Outcome | Compile median [range] s | Peak RSS KiB | Object text bytes |
|---|---|---|---:|---:|
| 200-local-callable-cross-function-reference-negative | reject | 0.0083 [0.0080, 0.0088] | 7532 | — |
| 400-reachable-missing-return-bad | reject | 0.0080 [0.0077, 0.0082] | 7632 | — |
| 700-hosted-replaceable-operator-new-dynamic-exception-spec | reject | 0.0342 [0.0333, 0.0346] | 8728 | — |
| 600-noreturn-control-convergence | emit | 0.0102 [0.0098, 0.0103] | 7788 | 275 |
| 700-libstdcxx-regex-compiler-member-alias-call | emit | 2.4863 [2.4557, 2.5033] | 177048 | 222302 |
| 600-hosted-recursive-std-function-string-substr | emit | 0.8770 [0.8546, 0.8954] | 54372 | 24188 |
| 700-hosted-map-subscript-piecewise-construct-compile | emit | 0.5859 [0.5835, 0.5886] | 43252 | 7737 |

All expected rejections produce no object. Every successful fixture emits an
identical object in all four repetitions. Maximum observed compile: **2.503 s**,
maximum RSS: **177048 KiB**. The 45-second limit is preserved.

PA30 discards the heavy hosted objects; hosted link/runtime completion belongs
to PA31. Applicable default/capture/control/exception behavior is additionally
checked through direct objects and serialized LowIR execution. Two required
random fixtures still need packed SIMD semantics; these measurements do not
waive those failures, general vector subscripting, or independent review.
