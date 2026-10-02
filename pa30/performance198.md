# PA30 audit198 performance acceptance

Reviewed code: `4a081cb05b25638be7a759882f67d4d8ae97eb6a`.
The changes correct parsing, dependent types, lookup and LowIR presentation.
There is no optional optimization or speedup claim. Measurements apply to PA30
at O0; hosted link/runtime completion belongs to PA31, optimization-level goals
to PA32/33 and inception to PA34.

## Protocol and acceptance

The frozen stage-base compiler A is `27029f9` (retained by handoff195); B is the
final audit compiler. The final hosted A/B follow-up instead uses checkpoint
`c0b26910`, which already accepts those heavy sources. Source/binary hashes,
flags `-O0 -c --stats`, CPU affinity 2, all wall-time observations, RSS and work
counters are retained. Compilation and executable execution are timed separately.
Each equivalent comparison has one AAAA calibration and six ABBA blocks.
Executable inputs use runtime argc/argv; each run checks its result. Host g++
only links compiler-produced benchmark objects. No reference compiler implements
any measured output.

Current datasets contain **660 observations plus 16 launch calibrations**:

- [Whole-stage common](../student.tests/pa30/evidence198/common-performance.json):
  224 observations across templates, loops, calls, memory, floating point,
  exceptions and emission pruning.
- [Unchanged common repeat](../student.tests/pa30/evidence198/common-repeat-performance.json):
  224 observations to investigate noisy first-series compilation, especially pruning.
- [Corrected base owner](../student.tests/pa30/evidence198/owner-performance.json):
  48 final-only compile/runtime observations and 16 launchers at three scales.
  A rejects these valid inputs; failed compilations are not timing baselines.
- [Hosted range](../student.tests/pa30/evidence198/hosted-performance.json):
  four successful compilations of each of all 27 accumulated repaired fixtures (108).
- [Hosted contemporaneous comparison](../student.tests/pa30/evidence198/hosted-ab-performance.json):
  56 compilations of two large equivalent sources, investigating the difference
  between current and historical absolute times. Exact reproduction code is
  retained in that record as well as flags, inputs and every result.

The PA30 **45-second per-compile limit** remains mandatory and unchanged.
There is no mandated numeric RSS percentage in the handout. Inherited blanket
15% latency and zero-growth targets remain diagnostic under spec §9; their
historical observations are preserved in performance195–197 and the PA29 audit.
They cannot become extra exit gates. This does not waive correctness, coverage,
actual avoidable regressions, or the existing evaluator/inline/native bounds.
No optional unprofitable transform is retained. Required type identity costs
are measured below; they do not license unrelated performance regressions.

## Equivalent full-stage workloads

All eight A/B object/executable pairs (four workloads, two series) are
byte-identical, including text. The table reports paired B/A median and full
block range. Compile/runtime absolute times and A/A spreads remain in the raw
records, including outliers; they are not discarded or combined into a claimed
speedup.

| Workload | Initial compile B/A [range] | Repeat compile B/A [range] | Repeat runtime B/A [range] | Repeat compile RSS A/B KiB | Object text bytes |
|---|---|---|---|---|---:|
| memory | 0.952 [0.728,1.009] | 1.003 [0.936,1.017] | 1.004 [0.985,1.018] | 29636 / 29652 | 151393 |
| floating | 0.952 [0.651,1.027] | 0.996 [0.988,1.004] | 1.003 [0.996,1.012] | 29648 / 29800 | 151234 |
| exceptions | 1.027 [0.989,1.633] | 1.032 [0.958,1.174] | 0.989 [0.729,1.167] | 30040 / 30112 | 151541 |
| pruning | 1.068 [0.808,1.636] | 1.006 [0.929,1.065] | 0.977 [0.884,1.059] | 35560 / 35632 | 151393 |

Repeat compile A/B medians in milliseconds are 274.34/274.38, 272.44/270.88,
238.72/249.38 and 224.77/223.08 respectively. Repeat runtime medians are
70.32/70.54, 56.35/56.36, 387.06/340.78 and 54.29/53.61 ms.
Initial pruning's +6.8% paired median is not reproduced (+0.6% on repetition).
The exception compile median remains slightly positive (+2.7%, +3.2%), with
both series spanning unity and substantial A/A/nonstationary timing. The
review found no added optimizer work, scans, allocations or growing cache key;
required constant-time semantic checks remain. These measurements do not
establish a distinct avoidable slowdown. No claim of zero measurement variance
or guaranteed zero overhead is made. Identical executable bytes exclude a
changed generated-code workload on these controls.

## Corrected owner scaling and machine costs

The base workload defines N independent sets of aliases and three demanded
class specializations per set. It uses the merged type in an emitted helper,
then performs 3,000,000 runtime-dependent transitions from argv seed 7 with
independently computed final-state and checksum checks.

| N | Compile median [range] s | RSS KiB | Runtime median [range] s | Object text bytes |
|---:|---|---:|---|---:|
| 128 | 0.0701 [0.0692,0.0708] | 14676 | 0.0665 [0.0664,0.0668] | 411 |
| 512 | 0.2593 [0.2554,0.2626] | 37588 | 0.0666 [0.0663,0.0671] | 411 |
| 2048 | 1.1485 [1.1030,1.2479] | 129600 | 0.0678 [0.0672,0.0738] | 411 |

Every compile records lookup work `73N+60`, completions `3N`, delimiter work
`165N+134`, and maximum cursor lookahead 10. Equal types use O(1) canonical
comparison at the existing merge owner. There is no new cache or invalidation.
All repeated object hashes match. Output text remains constant as type-only
families grow. Compiler launch median is 6.21 ms [5.68,6.73]; runtime launch
median is 4.86 ms [4.81,5.09]. These are correctness/scaling observations,
not a speed comparison against the rejecting entry implementation.

The separate optimization trace in [audit.md](audit.md) retains actual LowIR,
MIR, ELF, frame/unwind data and checked execution: one attribute-required
expansion costs 17 units against 33 reserved, within the unchanged 262,144
per-caller and 4,194,304 program limits. Volatile effects and noinline calls
survive. The real O0 loop traffic and 48/32-byte main/apply frames are reported.
No optional transform's profitability is inferred from those IR counts.

## Final hosted costs and timing investigation

All 27 repaired course inputs were measured four times with B. Every fixture's
objects are identical across repetitions. Maximum observed compile time is
**2.806 s**, and maximum RSS is **88,592 KiB**. Per-fixture observations include
text size, source/output hashes and counters. The course reports separately
exercise all 153 cases with the unchanged timeout and comparison rules.

Current absolute times for some headers were higher than the historical
measurements, so a contemporaneous A/B comparison used the frozen checkpoint
compiler and final audit compiler on two large affected sources. Both accept
identical inputs, with identical output objects/text:

| Fixture | Compile median A/B s | B/A paired median [range] | RSS A/B KiB | Text bytes |
|---|---|---|---|---:|
| const-unordered-map-find | 1.0134 / 1.0032 | 1.001 [0.807,1.032] | 62668 / 62660 | 18847 |
| map-iterator-operator-lookup | 1.3778 / 1.3727 | 0.990 [0.780,1.006] | 83164 / 83036 | 27127 |

The first workload's A/A block itself spans 2.167–3.870 s before its A/B samples
settle near one second; the second's A/A range is 1.366–1.384 s. Thus the
historical/current absolute differences do not demonstrate an audit regression;
matched current comparisons show no repeatable increase. All noisy observations
are retained. The largest final B observation in this follow-up is 2.358 s.
Hosted executable runtime is outside PA30's discarded-object contract; checked
runtime/text measurements above cover the applicable generated-code surfaces.
