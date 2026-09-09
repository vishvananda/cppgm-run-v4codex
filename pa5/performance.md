# PA5 compiler performance evidence

## Independent final audit: exact final source

Frozen A: **`7e8d10bf2`**, binary SHA-256
`a34773b8616e9f327db58bf6a82b8f85753ef30f9bec6bb48e6d7efab6f15c41`.
Frozen final B: **`f0a0f614a`**, SHA-256
`66195a35202947571a0dba98dbe084ac7cf4bcf53c3df38f3c2524f4748346e3`.
These are ordinary `g++ -std=gnu++11 -Wall -O3` dev builds with the course runner.
The exact host compiler/platform, permitted CPU affinity, flags, input hashes,
output hashes and every observation are retained in
[final-audit-performance.json](../student.tests/pa5/final-audit-performance.json).
**Compiler latency and peak RSS are measured below; generated-program runtime
and generated text size are N/A**, because PA5 emits structured syntax only.
Host compiler `.text` is reported separately, not as generated-program text.

The campaign fixes the existing generator's twelve inputs before timing. Four
identical primary-file operands run in one process (eight for nested inputs),
each with fresh TU ownership, to make useful work dominate startup. No builds,
correctness tests or other timing campaigns overlap. Wall time includes process
startup, parsing and AST writes; SHA-256 checking is outside the timed interval.
`/usr/bin/time` records process peak RSS. The runner pins itself and children to
one permitted CPU. Each input has two A/A calibration pairs, B/B and two ABBA
blocks: **168 ordinary observations**, plus **eight startup probes**, **24 separate
phase/work runs**, and **28 calibrated ordinary/stats observations**. Every output
is byte-equivalent, including telemetry-enabled runs. Workload inputs contain
no dynamic predefined macros. Measurements do not skip required syntax work.

Wall/RSS entries are medians of the four observations per variant in paired
blocks. Gains compare the A and B means within each ABBA block; positive is
faster. Noise is the maximum relative difference in the two A/A pairs. Spread
shows all four B paired observations; raw files retain every A, B and calibration
sample as well. Wall values are **per invocation with the listed TU count**.

| Workload | TUs | A / B wall (s) | A / B RSS (KiB) | Paired gains | A/A noise | B wall spread (s) |
| --- | --- | --- | --- | --- | --- | --- |
| classes-1 | 4 | 0.325618 / 0.320932 | 17166 / 17062 | +1.07%, +1.65% | 0.33% | 0.320734–0.321700 |
| classes-4 | 4 | 0.330934 / 0.331128 | 17760 / 17708 | -0.69%, +0.26% | 1.01% | 0.328634–0.335162 |
| declarations-1 | 4 | 0.530992 / 0.530875 | 22720 / 22566 | -0.30%, -0.40% | 3.24% | 0.527254–0.539571 |
| declarations-4 | 4 | 2.178996 / 2.152095 | 78898 / 79882 | +1.75%, +0.79% | 1.48% | 2.146143–2.161309 |
| expressions-1 | 4 | 1.570529 / 1.592929 | 63052 / 63090 | -1.64%, -1.16% | 0.12% | 1.585909–1.596138 |
| expressions-4 | 4 | 6.274296 / 6.352706 | 202734 / 207868 | -1.07%, -1.47% | 0.53% | 6.330366–6.366399 |
| nested-1 | 8 | 0.262063 / 0.263221 | 8520 / 8442 | -0.44%, +0.22% | 10.92% | 0.263046–0.265240 |
| nested-4 | 8 | 1.015853 / 1.026006 | 21494 / 21428 | -0.49%, -1.31% | 0.89% | 1.018723–1.030714 |
| procedural-1 | 4 | 0.857146 / 0.862040 | 33310 / 33426 | -1.66%, +0.29% | 1.68% | 0.854207–0.871855 |
| procedural-4 | 4 | 3.429278 / 3.447533 | 116718 / 116546 | -0.55%, -0.53% | 0.96% | 3.433553–3.459453 |
| templates-1 | 4 | 0.517467 / 0.524514 | 21206 / 21126 | -1.81%, -0.77% | 2.24% | 0.520538–0.527619 |
| templates-4 | 4 | 2.084260 / 2.105228 | 72612 / 72560 | -1.18%, -0.43% | 0.10% | 2.091076–2.110691 |

Host compiler `.text`: **203718 → 208518 bytes (+2.36%)**. The principal disclosed
costs are the expression groups (1.07–1.64% paired latency regressions), larger
expressions' peak RSS (202734 → 207868 KiB, +2.53%) and templates (up to 1.81%
paired slowdown, with smaller-input noise 2.24%). Required body/prototype scope
ownership adds real lookup/storage work; this is necessary syntax correctness,
not an optional executable optimization. Class-1 gains exceed calibration in
both blocks; several other changes are within noise or inconsistent between
blocks. No broad speedup claim is made. Nested-1's **10.92%** calibration noise
is retained and disclosed; it is not evidence of a performance improvement.

Median B startup is **0.005378 s**; the shortest workload is **48.9x** startup,
above the predeclared 25x floor. Fourfold nested input/depth grows B wall
**3.898x**, while angle work **25520 → 102320** follows tokens
**25851 → 102651**. Every delimiter gets one indexing visit; final name/scope,
node, hint and angle counters reset identically for each repeated primary file.
All six families pass the fourfold wall/RSS and pipeline work envelopes. The
class-depth experiment has nearly fixed trailing object count, and output
indentation bytes grow with depth; output cost is disclosed rather than
misreported as repeated parsing. Declaration/template counts scale fourfold;
procedural syntax includes loops, calls, memory access and floating arithmetic.
Actual self-hosting and executable benchmarks await their owning stages.

The predeclared budgets remain **10% wall regression plus A/A noise, 15% RSS
+1 MiB, 15% host text growth, <6x wall and <5x RSS +1 MiB for fourfold input/depth**.
No optimization-driven AST/output growth is allowed. Final hash/output/protocol,
startup, scaling, work and every paired budget pass the executable verifier.

Telemetry is evaluated separately on final B. For declarations-4, ordinary
and stats medians are 2.150349 / 2.152902 s; paired overheads are +1.49% and
-0.24% against 0.52% A/A noise. For templates-4 they are 2.097715 / 2.112070 s,
with +1.02% and +0.30% overhead against 1.36% noise. These observations do not
support a precise or zero-overhead claim. All outputs are identical and RSS
stays within budget. Phase counters are read from existing work and are not
used to claim an unmeasured optimization benefit.

## Profitability decision and retained intermediate evidence

The full candidate at `9cfce7949` also reused one indentation string. Its
[168 ordinary observations and auxiliary runs](../student.tests/pa5/reuse-candidate-performance.json)
remain intact. To isolate this optional change, freeze that candidate and a
control whose only source change restores per-line indentation construction.
The [isolated dataset](../student.tests/pa5/indentation-performance.json) retains
28 ordinary observations, eight startup probes and four phase/work runs on
classes-4 and expressions-4, with identical flags/inputs and exact outputs.
The control SHA-256 is
`956e926bb80a8ca799f4bb64213cd79598346ee536253c7dbfc9c11c81482405`;
the reuse candidate is
`15f6bebfdd2c44e88fee38e947e69b91d71e827cd178ce71bd6e3edb166778a0`.

| Isolated workload | Control / reuse wall (s) | Paired gains | A/A noise | Control / reuse RSS (KiB) |
| --- | --- | --- | --- | --- |
| classes-4 | 0.328909 / 0.328209 | +0.83%, -0.60% | 0.78% | 17710 / 17738 |
| expressions-4 | 6.343431 / 6.331473 | +0.30%, +0.14% | 0.28% | 207866 / 207844 |

Neither workload establishes a latency benefit above noise in both blocks or a
useful peak-memory improvement. Allocation-count reasoning alone does not prove
compiler profit. The optional change was **removed in `f0a0f614a`**, and the
complete final campaign above was rerun on that exact binary. Intermediate
measurements are not presented as final-source results. The inherited delimiter
and lexical-hint optimizations remain: their ownership/legality/bounds are
reviewed in [the architecture audit](audit.md), with historical frozen evidence
below and final work/scaling checks above.

## Reproduction and verification

Build frozen A/B in isolated checkouts of the stated commits with the recorded
host toolchain and ordinary dev flags. Copy executables before timing. Do not
rebuild or run tests concurrently with the measurement process.

```sh
python3 student.tests/pa5/audit_performance.py measure <frozen-A> <frozen-final-B> /tmp/pa5-final.json
python3 student.tests/pa5/audit_performance.py verify student.tests/pa5/final-audit-performance.json <frozen-A> dev/cppgm++
python3 student.tests/pa5/audit_performance.py report student.tests/pa5/final-audit-performance.json
python3 student.tests/pa5/build_indentation_baseline.py /tmp/pa5-indentation-control
python3 student.tests/pa5/audit_performance.py verify student.tests/pa5/indentation-performance.json /tmp/pa5-indentation-control/no-reuse <frozen-9cfce7949>
```

The verifier recomputes the protocol, actual binaries/text sizes, regenerated
inputs, output equivalence, gains/noise, work/TU isolation and budgets. For the
isolation dataset it reports whether any workload proves paired profitability;
this retained experiment correctly reports **not established**. See
[personal validation](../student.tests/pa5/README.md) for the sanitizer/graph
runners. Raw data are the audit evidence; generated binaries, objects, AST dumps
and execution logs remain outside version control.

## Historical implementation campaigns (pre-independent-audit)

The following measurements retain their original scope and candidate hashes.
They were independently verified against the frozen matching binary; their
"final B" means the implementation checkpoint, not `f0a0f614a`.

Baseline A is full-behavior commit `6d67335c1`, already passing 188/188.
Final B is source commit `8365a1124`, including delimiter/angle indexes, cached lexical hints, compact shared
physical/presumed locations and retained user-literal payloads. Both binaries
were frozen before this campaign; flags, toolchain, kernel, hashes and every
observation are in [performance.json](../student.tests/pa5/performance.json).
The first indexed campaign is also retained in [indexed-performance.json](../student.tests/pa5/indexed-performance.json).

The final campaign has **168 observations and eight startup probes**: two A/A
pairs, B/B and two ABBA blocks on each of 12 fixed inputs. The first campaign
has another 140 observations and eight startup probes. No builds or correctness
tests overlapped either campaign. Wall timing includes process startup and the
AST write, excludes hashing; RSS comes from `/usr/bin/time`. All compared outputs
are byte-identical by SHA-256. Frontend/emit phase times and work counts are
retained for every run. AST compilation is measured; **generated-program runtime
and text size are N/A**, and no executable optimization benefit is claimed.

Values below use medians from the paired blocks. Gain ranges are the two paired
block results; negative gains are disclosed regressions. Noise is the maximum
relative difference in the two A/A pairs. All raw observations preserve spread.

| Workload | A / B wall (s) | A / B peak RSS (KiB) | Paired gains | A/A noise |
| --- | --- | --- | --- | --- |
| classes-1 | 0.095395 / 0.087388 | 15660 / 15246 | 8.36–8.71% | 1.88% |
| classes-4 | 0.133640 / 0.090146 | 16006 / 15498 | 32.68–33.20% | 0.53% |
| declarations-1 | 0.140144 / 0.142492 | 17674 / 17612 | -3.37–-1.66% | 0.48% |
| declarations-4 | 0.558030 / 0.565422 | 59076 / 58188 | -1.72–-1.21% | 0.71% |
| expressions-1 | 0.397666 / 0.406060 | 48742 / 47260 | -2.32–-2.04% | 1.15% |
| expressions-4 | 1.570499 / 1.600956 | 183606 / 178074 | -2.01–-1.83% | 0.46% |
| nested-1 | 0.388456 / 0.038617 | 6748 / 6776 | 90.04–90.09% | 2.15% |
| nested-4 | 5.612879 / 0.135206 | 14814 / 15174 | 97.59–97.59% | 0.24% |
| procedural-1 | 0.222268 / 0.222427 | 27042 / 25738 | -0.02–0.21% | 1.95% |
| procedural-4 | 0.880584 / 0.881023 | 95836 / 90946 | -0.32–0.42% | 0.73% |
| templates-1 | 0.142083 / 0.137018 | 15390 / 14646 | 3.07–3.40% | 2.75% |
| templates-4 | 0.558190 / 0.548155 | 50296 / 47410 | 1.12–1.76% | 0.56% |

Host compiler `.text`: **200070 → 203718 bytes (+1.82%)**.
Nested input grows 3.934x in bytes; B wall grows 3.501x and RSS 2.239x.
Angle work grows **25520 → 102320**, tracking tokens **25851 → 102651**.
The API also asserts one delimiter-index visit per token and once-per-identifier
lexical scanning on 1,024 repetitions of a 1,024-byte unresolved identifier.
Median B startup is 0.004291 s; the shortest measured workload is 9.0x startup.

Fourfold workloads scale declaration/function counts or nested template depth.
`classes` varies class depth 128→512 with the same 12,000 trailing objects; its
source grows only modestly and is a separate depth experiment. AST indentation
can grow with nesting; phase times distinguish that requested output from parsing.
`procedural` covers loops, runtime parameter references, calls, array loads and
floating arithmetic at the syntax stage. Actual runtime/self-hosting awaits the
later executable-producing stages; no syntax timing substitutes for that evidence.

Predeclared budgets: each paired block permits ≤10% wall regression plus A/A
noise, ≤15% RSS growth +1 MiB, ≤15% host text growth, and fourfold input/depth
below 6x wall / 5x RSS +1 MiB. **Every budget passes.** Nested/template index
benefits exceed measured noise in both blocks. Declaration/expression slowdowns
are within budget; the strongest reproducible gains are nested-angle and
complete-class lookahead. No improvement is claimed for the inconclusive
procedural workload.

Reproduce/check from the repository root:

```sh
python3 student.tests/pa5/bench_inputs.py /tmp/pa5-inputs
python3 student.tests/pa5/measure.py <frozen-A> <frozen-B> /tmp/pa5-inputs /tmp/observations.json
python3 student.tests/pa5/verify_performance.py student.tests/pa5/performance.json <frozen-8365a1124>
```

The historical verifier checks its matching frozen binary, input generator hashes, complete
protocol, output equivalence, every paired budget, all scaling pairs and the
nested-work bounds. It prints recomputed medians, paired gains and B wall spreads.
Frozen A can be rebuilt from the commit above with the recorded host flags.
Generated binaries and AST outputs remain outside version control.
