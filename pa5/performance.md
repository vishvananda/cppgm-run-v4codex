# PA5 compiler performance evidence

Baseline A is full-behavior commit `262b0b61f`, already passing 188/188.
Final B includes delimiter/angle indexes, cached lexical hints, compact shared
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
python3 student.tests/pa5/verify_performance.py student.tests/pa5/performance.json dev/cppgm++
```

The verifier checks the actual final binary, input generator hashes, complete
protocol, output equivalence, every paired budget, all scaling pairs and the
nested-work bounds. It prints recomputed medians, paired gains and B wall spreads.
Frozen A can be rebuilt from the commit above with the recorded host flags.
Generated binaries and AST outputs remain outside version control.
