# PA30 implementation199 performance acceptance

Code tip: `d6408429`. Frozen A is entry `d9a62584`; B is the final
implementation binary. [Source/binary binding](../student.tests/pa30/evidence199/source-binding.json)
and [design trace](design199.md) identify the measured implementation.
These are necessary semantic/emission repairs at O0. No optional optimization
or speedup is claimed.

## Protocol and budgets

Flags are `-O0 -c --stats`, affinity CPU 2. Fixed inputs, compiler hashes,
all wall times, peak RSS, phase counters, object/executable hashes and text
sizes are retained. Common comparisons use one AAAA calibration and six ABBA
blocks for compilation and execution separately. Every executable run checks
its result from runtime argc/argv inputs; host g++ only links compiler-emitted
objects. Tests/builds finish before the final measurement sequence.

Final evidence contains **312 observations plus 16 launch calibrations**:

- [Common](../student.tests/pa30/evidence199/common-performance.json): 224, with
  templates, loops, calls, memory, floating point, exceptions and unused emission.
- [Allocation/access scaling](../student.tests/pa30/evidence199/owner-performance.json):
  48 plus 16 launchers, at N=64,256,1024 independent declaration families.
- [Repaired hosted fixtures](../student.tests/pa30/evidence199/hosted-performance.json):
  24, four compiles of each of the six newly passing course inputs.
- [Vector runtime](../student.tests/pa30/evidence199/vector-performance.json):
  16 compile/runtime samples covering initialization, extraction, representation
  conversion, volatile whole-vector reads and assignment.

The last three workloads have no valid entry timing baseline: A rejects the
new semantics. They report final absolute costs, not a comparison to failed
compilation. The small vector source's compile time is startup-dominated;
compiler scaling uses the larger owner/common inputs instead. Runtime loops
perform 3,000,000 checked transitions and dominate process startup.

The mandated **45-second per-compile limit** remains unchanged. Inherited
blanket 15% latency/zero-growth diagnostics are not extra gates under spec §9;
performance195–198 and all observations remain preserved. No numeric RSS
percentage is mandated by PA30. Existing evaluator, native, inline and
per-function reclamation bounds are unchanged. Required initialization stores,
volatile reads and representation copies are measured correctness costs; no
optional transform is retained without profit. Fixed builtins emit at most
eight lane initializations; larger existing vector operations use bounded
loop lowering; static zero vectors use one byte-span data item. These repairs
add no unbounded search or optional code expansion.

## Equivalent common workloads

All A/B object and executable pairs are byte-identical, including text. Paired
B/A medians and complete six-block ranges are below; noise observations are
retained rather than discarded. No generated-work reduction is inferred.

| Workload | Compile B/A [range] | Runtime B/A [range] | Compile RSS A/B KiB | Object text bytes |
|---|---|---|---|---:|
| memory | 0.987 [0.954, 1.009] | 0.997 [0.973, 1.031] | 29976 / 30028 | 151393 |
| floating | 0.931 [0.733, 1.037] | 0.991 [0.961, 1.065] | 29888 / 29908 | 151234 |
| exceptions | 0.963 [0.927, 1.146] | 0.955 [0.684, 1.102] | 30060 / 30152 | 151541 |
| pruning | 0.987 [0.844, 1.290] | 0.998 [0.919, 1.133] | 35716 / 35772 | 151393 |

| Workload | Compile A/B median ms | Runtime A/B median ms | Compile A/A range ms | Runtime A/A range ms |
|---|---|---|---|---|
| memory | 336.99 / 331.26 | 92.19 / 91.48 | 323.09–375.26 | 88.31–95.76 |
| floating | 347.22 / 333.99 | 83.56 / 83.31 | 327.74–398.51 | 83.96–91.62 |
| exceptions | 337.19 / 326.74 | 529.15 / 493.15 | 327.95–418.62 | 416.04–510.49 |
| pruning | 401.83 / 389.58 | 109.71 / 110.52 | 411.97–486.37 | 80.41–97.07 |

## Corrected owner scaling

Each family contains dependent new-array declarations, current-instantiation
friendship, fixed-base signature aliases and one demanded nested class. The
emitted helper consumes those facts and the runtime allocation/access loop
checks an independently computed final state and checksum.

| N | Compile median [range] s | Peak RSS KiB | Runtime median [range] s | Object / executable text bytes |
|---:|---|---:|---|---|
| 64 | 0.1228 [0.1196, 0.1422] | 14424 | 0.1125 [0.1088, 0.1148] | 573 / 813 |
| 256 | 0.5013 [0.4359, 0.8407] | 34620 | 0.1077 [0.1025, 0.1134] | 573 / 813 |
| 1024 | 2.4817 [2.0717, 2.9764] | 114672 | 0.1395 [0.1352, 0.1527] | 573 / 813 |

Every repetition records lookup work `179N+88`, type-query work `12N+3`,
class completions `N`, delimiter work `311N+170` and maximum lookahead 47.
All output hashes match across repetitions and N: growing type-only families
adds no emitted text. Indexed facts and language-required scope/base edges
account for the linear work, without a new global cache or retry policy.

Compile launch calibration: median 9.26 ms [8.15, 9.63].
Runtime launch calibration: median 6.79 ms [5.66, 7.91].

## Vector and hosted costs

The vector runtime uses an argv-dependent seed, vector construction, two
representation casts and two full volatile operand reads each iteration. Its
checksum/final state are computed independently. The early benchmark exposed
invalid zero-vector static data; that defect is fixed in the measured B.
The reduced zero-initialization and volatile controls also pass object and
external LowIR roundtrip execution.

Vector compile median [range]: **0.0126 [0.0101, 0.0192] s**, peak RSS **8088 KiB**. Runtime: **0.1676 [0.1671, 0.1730] s**. Object/executable text: **688 / 928 bytes**. All repeated objects match.

| Repaired fixture | Compile median [range] s | Peak RSS KiB | Object text bytes |
|---|---|---:|---:|
| 600-hosted-fixed-vector-builtins | 0.0213 [0.0206, 0.0224] | 8088 | 364 |
| 600-hosted-fstream-stream-compile | 2.6200 [2.5054, 2.7906] | 68288 | 22301 |
| 600-regex-iterator-difference-alias | 2.8260 [2.0112, 4.8412] | 124532 | 25080 |
| 700-hosted-codecvt-wstring-convert-char16-compile | 1.1253 [1.1129, 1.1804] | 67652 | 25386 |
| 700-hosted-iomanip-setprecision-compile | 1.1685 [1.1640, 1.1987] | 71004 | 22301 |
| 700-hosted-locale-facet-compile | 1.0368 [1.0316, 1.0534] | 65540 | 22301 |

All repaired hosted compiles succeeded with deterministic repeated objects. Maximum measured hosted time **4.841 s**, RSS **124532 KiB**, below the unchanged timeout. PA30 discards these objects; hosted runtime completion remains PA31 work, while the applicable generated-code surfaces above have checked runtime/text measurements.

## Preserved intermediate observations

Two earlier complete common/owner/hosted series (296 observations and 16
launchers each) are retained in `evidence199/preliminary-*-performance.json`
and `pre-zero-*-performance.json`. Their B hashes identify the earlier code;
neither is relabeled final. The first predates volatile snapshots; the second
predates zero-vector storage repair. The later correctness discoveries required
fresh final bindings and measurements. Some early samples overlapped follow-up
work and exhibit large outliers. The final sequence runs without concurrent
compiler builds or reports; all intermediate outliers are still preserved.

Common images are identical in both intermediate series too. Their compilation
paired medians span 0.983–1.017 and runtime medians 0.998–1.009. These observations
and A/A spreads do not establish a repeatable avoidable regression or an
optimization benefit. The current code adds constant-time dispatch checks and
required semantic work; no unrelated scans, optimizer expansions or growing
cache keys were added. The final raw data preserves any timing variance rather
than claiming zero overhead. Whole-stage acceptance remains incomplete because
15 required correctness fixtures still fail, independently of these measured
costs; the performance record does not waive them.
