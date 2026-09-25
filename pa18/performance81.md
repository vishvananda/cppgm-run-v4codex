# PA18 handoff 81 performance evidence

Acceptance is PA18 `--emit-lowir -O0`, under spec §9. All observations are in
[the final raw record](../student.tests/pa18/loop81-performance.json).
[The initial preflight](../student.tests/pa18/loop81-performance-attempt1.json)
and [the complete pre-refinement run](../student.tests/pa18/loop81-performance-before-demand.json)
remain preserved. No course/reference comparison was changed.

## Protocol and budgets

Frozen A is entry `69247877`; B includes implementation `7b8c98a6`, `4ab09a9b`
and the request refinement `25b89fc2`. Binary hashes, compiler flags, input
sources/hashes, backend hash/bundle, telemetry and every observation are recorded.
The harness pins CPU 31, warms each binary, takes four A/A calibration
samples, then four wall-time ABBA blocks. Compilation and execution are timed
separately. Each native preflight checks exit zero; each timed execution also
checks its result. The affected loops use a volatile bound and checked sums over
24 million calls, preventing dead or constant-folded workloads. Native execution
uses the supplied PA18 backend at O0; no native-code implementation is claimed.

The O0 work budget is one proof per requested, completed scalar conversion,
one return and at most eight parenthesis wrappers. A successful proof retains
one typed scalar fact; request and result indexes are TU-owned flat maps keyed
by canonical function identity. Each use consumes its recorded conversions.
Unproved or over-budget bodies retain ordinary calls. The transform adds no
executable operations or cloned code; selected-arm emission only removes work.
The deferred nonvirtual leaf list is visited once after actual symbol uses.
There is no new fixed-point or whole-IR optimization pass. Explicit-only uses
request no summary and keep their original emission path.

PA18 mandates no numerical compiler-latency/RSS ceiling. Historical +15%,
+16 MiB and 5.5× diagnostics remain non-gates; bounded work, correctness,
coverage and measured profitability remain requirements. Later native
optimization/self-hosting requirements are not imposed on PA18.

## Compiler latency and peak RSS

Medians are milliseconds. Paired ratios use the mean B time divided by mean A
time within each ABBA block; the range retains all four blocks. RSS is the
maximum KiB observed for that binary in the ABBA samples.

| Workload | A / B ms | Paired B/A (range) | A / B peak KiB |
|---|---:|---:|---:|
| ordering-600 | 117.17 / 114.00 | 0.995 (0.974–1.608) | 15284 / 15276 |
| ordering-2400 | 473.49 / 436.13 | 0.948 (0.923–1.047) | 43404 / 43348 |
| member-head-600 | 185.02 / 167.51 | 0.907 (0.875–0.952) | 22608 / 21492 |
| member-head-2400 | 799.46 / 684.03 | 0.775 (0.653–0.899) | 72276 / 68160 |
| common-loop-float-1500 | 350.53 / 357.91 | 1.050 (0.958–1.147) | 30232 / 30376 |
| runtime-calls | 10.35 / 10.17 | 0.921 (0.902–1.003) | 5944 / 6064 |
| runtime-memory | 10.18 / 9.66 | 0.970 (0.917–1.021) | 5900 / 6064 |
| runtime-floating | 9.31 / 9.30 | 1.001 (0.903–1.072) | 5948 / 6192 |
| named-result-600 | 112.34 / 125.97 | 0.953 (0.824–1.166) | 14212 / 13312 |
| retained-result-600 | 201.12 / 203.05 | 0.999 (0.882–1.067) | 15008 / 15020 |
| named-result-2400 | 455.02 / 431.57 | 0.919 (0.664–0.987) | 39160 / 35524 |
| retained-result-2400 | 746.78 / 744.44 | 1.000 (0.922–1.097) | 41980 / 42096 |
| runtime-named-result | 12.35 / 14.08 | 1.091 (0.791–1.371) | 5928 / 6060 |
| runtime-known-branch | 10.34 / 10.31 | 0.986 (0.967–1.049) | 5960 / 5980 |
| runtime-retained-result | 11.47 / 10.61 | 0.906 (0.476–1.010) | 5928 / 6024 |

Startup medians are 13.21 / 13.54 ms. Tiny runtime-source compiler timings are
startup-limited; the 600/2400 declaration and 1500-function workloads carry
more useful compiler-cost evidence. A/A ranges and all scheduling outliers remain
in the raw record. No tiny-source latency speedup is claimed.

The complete final run had substantial scheduling variation: the common-loop
A/A calibration spanned **286.89–352.17 ms**. Its observed paired compiler ratio
was **1.050**, despite zero summary work and unchanged 39,000 instructions.
A [focused confirmation](../student.tests/pa18/loop81-performance-common-confirmation.json)
with the same frozen binaries, flags and source reports **152.66 / 152.36 ms**,
paired **0.995 (0.986–1.182)** and peak RSS **30248 / 30364 KiB**. This does not
reproduce a systematic regression; all samples, including the outlier block,
remain preserved. The 600-case named-result aggregate median is slower in the
loaded full run even though its paired median ratio is below one. No compiler
speedup is claimed from that inconsistent small-workload timing. Explicit-only
600/2400 paired ratios are **0.999 / 1.000** after the request refinement.

## Executable runtime and size

The compiler size is ELF `.text`. The supplied sectionless native output uses
executable payload bytes as a size proxy; it is not a section measurement.

| Workload | A / B ms | Paired B/A (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 1338.17 / 1299.56 | 0.942 (0.854–1.289) | 206 / 206 |
| runtime-memory | 670.89 / 665.71 | 0.995 (0.845–1.048) | 434 / 434 |
| runtime-floating | 750.46 / 791.68 | 1.058 (0.960–1.080) | 230 / 230 |
| runtime-named-result | 290.65 / 212.40 | 0.735 (0.633–0.944) | 222 / 196 |
| runtime-known-branch | 269.54 / 187.10 | 0.707 (0.648–0.762) | 268 / 192 |
| runtime-retained-result | 326.79 / 325.75 | 1.001 (0.951–1.043) | 222 / 222 |

The unaffected floating executable has a **1.058** runtime ratio in the final
run despite byte-identical native output; it was **0.998** in the earlier run.
This scheduling variation is disclosed, not attributed to a generated-code change.
The two affected loops improve in all four final blocks (paired medians **0.735 /
0.707**), as well as in all blocks of the earlier run (**0.675 / 0.675**).

Compiler `.text`: **1,960,070 → 1,967,302 bytes**,
**+7,232 (0.369%)**.

## Scaling, refinement and acceptance

The original member-head corpus also forwards named non-type template constants.
Its initial exact-IR preflight failed because the optimization applied correctly;
both frozen outputs passed native checks. The final harness marks those affected
inputs as executed-equivalent and preserves exact comparison on unaffected inputs.
The failed preflight and its completed observations remain in the first raw file.

The first complete run showed a 1.6% paired compiler cost at 2400 explicit-only
conversion functions. That avoidable work led to the request refinement. Final
explicit-only scaled and runtime workloads have zero summary inspections and
byte-identical LowIR/native output to entry. Both measurements remain available.

| 600 → 2400 declarations | B summary work | A instructions | B instructions |
|---|---:|---:|---:|
| member-head | 600 → 2400 | 10202 → 40802 | 7802 → 31202 |
| named-result | 600 → 2400 | 3613 → 14413 | 1213 → 4813 |
| retained-result | 0 → 0 | 3613 → 14413 | 3613 → 14413 |

The named-result and known-branch loops improve in every paired block and shrink
their native payloads. This repeatable runtime benefit justifies the bounded
frontend proof and small compiler text increase. Candidate and syntax-node counts
are unchanged: the gain comes from consuming established facts during lowering.
Unaffected byte-identical executables provide noise controls; no runtime improvement
is attributed to them. Costs and observed regressions are disclosed above.
The required correctness/coverage checks and these work, growth and profit records
satisfy this group’s PA18/O0 acceptance. The remaining 24 course failures still
prevent whole-stage acceptance.
