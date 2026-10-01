# Performance180 — callable selection and mandatory inline preparation

Acceptance is PA29/O0 under spec §9. This group implements required callable
semantics and explicit force-inline metadata, not an optional optimization-level
policy. The final [manifest](../student.tests/pa29/evidence180/manifest.json)
binds code tip `f613c554`, compiler hashes, flags, inputs, scripts and evidence.
[Performance179](performance179.md) and all inherited measurements are preserved.

## Protocol and observations

A is frozen entry `259329ee`; B is frozen final `f613c554`. Compilation uses
`-O0 -c --stats` (owner inputs also specify `-std=c++11`). Host linking and
correctness-suite work occur outside compilation/runtime timers. Each equivalent
input has four A/A calibration samples followed by six ABBA blocks, separately
for compilation and checked execution. All samples and spread are retained;
no affinity is set, and external scheduling is uncontrolled.

- [Common data](../student.tests/pa29/evidence180/common-performance.json):
  **224** observations, four inherited 2,400-specialization workloads covering
  loops, calls, memory, floating point, exceptions and dormant declarations.
- [Owner data](../student.tests/pa29/evidence180/owner-performance.json):
  **280** equivalent observations on five forced-inline inputs, plus **48**
  final-only observations on three static-call extension inputs. Entry rejects
  the latter; no speedup is computed against rejection.
- Owner executables consume argv seed 17 and varying loop inputs for **24 million**
  calls. Every result agrees with an independent Python checksum and a separately
  compiled Clang C++11 extension execution. Source/input and image hashes are
  retained. No constant-folded or dead workload is timed.
- All **552 preliminary** observations at `a613b21a` remain in
  [preliminary common](../student.tests/pa29/evidence180/preliminary-common-performance.json)
  and [preliminary owner](../student.tests/pa29/evidence180/preliminary-owner-performance.json),
  with their own binary hashes. Final changes add conservative budget fallback,
  reuse operand scratch, and complete mixed/surrogate callable ranking.

Ratios below are medians of six paired block ratios; bracketed values are the
full paired-ratio range. RSS is maximum timed ABBA compiler RSS. Calibration
RSS and all runtime RSS samples remain in the raw data.

## Equivalent common inputs

| Input | Compile A/B s | Compile B/A [range] | RSS A/B KiB | Runtime A/B s | Runtime B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1606/0.1599 | 0.9846 [0.9760–2.9117] | 29792/30068 | 0.0508/0.0507 | 0.9955 [0.9884–1.0079] | 151633/151633 |
| floating | 0.1599/0.1610 | 1.0093 [0.6925–1.0247] | 29636/29848 | 0.0475/0.0475 | 0.9979 [0.9938–1.0020] | 151474/151474 |
| exceptions | 0.1608/0.1592 | 0.9917 [0.9666–1.6045] | 30024/30232 | 0.2492/0.2491 | 0.9998 [0.9903–1.0210] | 151781/151781 |
| pruning | 0.2058/0.2009 | 0.9781 [0.5677–1.0057] | 35564/35676 | 0.0508/0.0507 | 0.9990 [0.9895–1.0184] | 151633/151633 |

All four common objects and executables are byte-identical
([identity check](../student.tests/pa29/evidence180/elf-comparison.json)). Their
runtime differences do not establish generated-code improvement or regression.
Compiler medians are close; large outliers are retained, including memory's
0.7724-second B sample and pruning's 0.4983-second A sample. No broad compiler
speedup is claimed. Common peak compiler RSS increases by at most 0.93%.

## Equivalent forced-inline inputs

| Input | Compile A/B s | Compile B/A [range] | RSS A/B KiB | Runtime A/B s | Runtime B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| straight600 | 0.0468/0.0483 | 1.0226 [1.0111–5.7906] | 13592/14468 | 0.1274/0.1273 | 0.9955 [0.9601–1.0001] | 37960/30651 |
| straight1200 | 0.0859/0.0883 | 1.0330 [1.0132–1.0398] | 19688/20964 | 0.1276/0.1268 | 0.9889 [0.9846–0.9967] | 75760/61251 |
| straight2400 | 0.1696/0.1762 | 1.0479 [0.8445–1.1138] | 31972/34364 | 0.4150/0.1265 | 0.3048 [0.3045–0.3059] | 151360/122451 |
| branch1200 | 0.1464/0.1790 | 1.2252 [1.2134–1.2743] | 25464/39504 | 0.3941/0.2502 | 0.6368 [0.6326–0.6485] | 198592/204455 |
| floating1200 | 0.1421/0.1548 | 1.0905 [1.0892–1.0990] | 23400/30736 | 0.3167/0.3154 | 0.9963 [0.9939–1.0004] | 183760/193251 |

Straight2400 runs **69.5% faster** in paired blocks, with **19.1% less text**;
branch1200 runs **36.3% faster**, with **3.0% more text**. All six blocks agree
on those gains, and the preliminary runs show the same direction and magnitude.
No cache/hardware attribution is inferred. Straight600 and floating1200 do not
establish a useful runtime gain. Floating1200 adds **5.2% text**; this cost is
explicitly disclosed.

Compiler costs are also real: branch1200 adds **22.5% paired latency** and
**55.1% peak RSS**; floating1200 adds **9.1% latency** and **31.4% RSS**. Mandatory
CFG/value mapping and retained input/output LowIR account for additional work
and storage. The final implementation removes per-instruction temporary-vector
allocation and avoids exception analysis for bodies without regions. Compared
with the retained preliminary run, final paired compile ratios fall from 1.0385,
1.0443, 1.0648 and 1.1197 to 1.0226, 1.0330, 1.0479 and 1.0905 on the straight
and floating inputs. These sequential runs are disclosed evidence of the repair,
not a separately calibrated optimization speedup claim.

The attribute requires eligible local-body substitution even at O0; the weak
replacement course fixture makes this observable. There is no profitability-
driven inlining of unannotated calls to remove. Current conservative scalar/FP
homes and general result storage remain typed O0 lowering; broader allocation
and optional optimizer policy belong to PA32/PA33. This does not excuse a
known avoidable correctness or performance defect in the completed group.

## Calibration and new static-call behavior

| Input | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| common memory | 0.1571–0.1611 | 0.0507–0.0512 |
| common floating | 0.1605–0.1664 | 0.0476–0.0478 |
| common exceptions | 0.1591–0.1641 | 0.2495–0.2629 |
| common pruning | 0.2010–0.2868 | 0.0505–0.0507 |
| owner straight600 | 0.0455–0.0474 | 0.1272–0.1278 |
| owner straight1200 | 0.0838–0.0863 | 0.1271–0.1277 |
| owner straight2400 | 0.1630–0.1675 | 0.4147–0.4159 |
| owner branch1200 | 0.1472–0.1591 | 0.3931–0.3954 |
| owner floating1200 | 0.1413–0.1526 | 0.3156–0.3181 |

| New input | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| static600 | 0.0633 [0.0624–0.0644] | 15752 | 0.1274 [0.1265–0.1285] | 1700 | 42127 |
| static1200 | 0.1195 [0.1176–0.1213] | 24324 | 0.1292 [0.1280–0.1434] | 1908 | 84127 |
| static2400 | 0.2432 [0.2364–0.3760] | 41076 | 0.4223 [0.4205–0.5226] | 1960 | 168127 |

Eight timed launchers: median 0.00294 s, range 0.00269–0.00349 s.
Measured workloads dominate startup. Static2400 takes longer than Static1200 at
a fixed call count; no causal explanation or speedup baseline is asserted for
newly accepted source. All required results are checked before timing.

## Work/storage bounds and acceptance

[Scaling](../student.tests/pa29/evidence180/scaling.json) checks every final owner
compilation's counters. For N=600/1200/2400, straight-line inputs have 15N+236
parsed nodes, 43N+236 total nodes, 17N+43 LowIR instructions, N+3 candidates,
N substitution frames, N expansions, 19N actual expansion units and 35N reserved
units. Native text is 51N−189 bytes. Static-call inputs have 17N+241 parsed nodes,
50N+241 total nodes, 13N+43 LowIR instructions, N+3 candidates and N frames;
native text is 70N−113 bytes. Linked text adds 240 bytes on these inputs.
These equations, together with the bounded algorithm, support work proportional
to actual declarations, candidates and emitted IR, rather than a timing-only
asymptotic claim.

Optional transform work and growth budgets remain **zero**. Mandatory expansion
admits at most **262,144 reserved instruction/operand/slot units per caller**,
**4,194,304 per program**, at **depth 64**. Reservations include boundary copies,
return-region retirement and continuation storage. A nested expansion cannot
spend its parent's reserved completion work. The same units bound additional IR
and slot growth; mapped values and blocks are proportional to that admitted
input/output. Actual work is checked against reservation. The measured maximum
caller reservation is **85,200**, program reservation **85,200**, and actual work
**73,200**. No measured owner input declines a call. Separate cycle, depth,
exponential-growth and dynamic-frame controls prove bounded conservative calls
at the limit, without rejecting otherwise valid input.

The inherited blanket 15% compiler-latency/RSS and zero-growth targets remain
**diagnostic**, as already classified in performance179 under spec §9. All
measurements, including historical misses and current costs, remain preserved.
Required semantics and bounded mandatory work determine current-stage acceptance;
these diagnostic percentages are not new exit gates. No mandated limit is
weakened: generated-element/constexpr/depth/source-identity/native frame/data/
alignment limits and course timeouts remain those recorded in performance179.
Heavy hosted runtime, optional O1–O3 optimization and self-hosting retain their
later-stage owners.

Reproduction, with A/B frozen first and OUT containing the recorded entry data:

```sh
python3 student.tests/pa29/test180.py OUT/controls-ranking B
python3 student.tests/pa29/inspect180.py OUT/inspection-complete180 OUT/controls-ranking
python3 student.tests/pa29/validate180.py student.tests/pa29/evidence180
python3 student.tests/pa27/performance147_common.py OUT/common-final A B
python3 student.tests/pa29/performance180.py OUT/owner-final A B
python3 student.tests/pa29/analyze180.py OUT student.tests/pa29/evidence180
```

The analyzer also expects the preserved preliminary JSON files and frozen entry
image under the basenames recorded in its source. Generated binaries/objects and
raw text logs are outside version control. JSON retains commands, observations,
checked outputs, counters and hashes.
