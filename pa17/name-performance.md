# PA17 source-name performance — loop 51

Entry A: `0fe13f160c921db3ca68bfefc857b3a6be40cf3f`. Final B: `6afed48ec5ff231ef85df0d8c5b7980f50ef6500`.
Frozen binaries, flags, complete input sources, output hashes, telemetry and all
observations are in [the final record](../student.tests/pa17/name-performance.json).
Reproduce with `student.tests/pa17/name_benchmark.py A B WORK OUT`.

The campaign pins CPU 31 and compiles PA17 `--emit-lowir -O0`, with the same
`g++ -std=gnu++11 -Wall -O3` compiler build and test runner. Common workloads
have one warmup per binary, four A/A observations, then four ABBA blocks.
Newly supported inputs record the entry rejection, one final-binary warmup and
six final observations. Timed compilation excludes stats and validation; the
untimed preflight includes both. No build or test campaign overlapped the
measurements. Native execution uses the pinned course backend at O0 and is
measured separately from compilation.

Every common LowIR hash and corresponding executable hash matches exactly.
Native programs check their results and use volatile bounds and live calls,
memory operations and floating-point arithmetic. Their compilation takes about
6 ms, so those compiler observations remain startup-sensitive diagnostics in
the raw record. Frontend scaling inputs run for roughly 0.1–2 seconds; native
workloads run for roughly 0.2–0.4 seconds. Own native emission and self-hosting
are later-stage responsibilities.

| Common compiler workload | A / B median ms | A / B peak KiB | Paired B/A median (range) | A/A range ms |
|---|---:|---:|---:|---:|
| common-partials-1500 | 109.17 / 110.13 | 22672 / 22396 | 1.0054 (0.9987–1.0260) | 107.75–116.16 |
| common-loop-float-1500 | 150.80 / 152.24 | 30236 / 30280 | 1.0125 (0.8134–1.0208) | 149.62–151.71 |
| common-partials-6000 | 452.06 / 455.40 | 72860 / 72796 | 1.0041 (0.9977–1.0380) | 448.39–453.43 |
| common-loop-float-6000 | 593.82 / 601.74 | 104240 / 104228 | 1.0043 (0.9844–1.0227) | 586.89–595.67 |
| three-head-demand-1500 | 456.21 / 458.85 | 78788 / 78520 | 1.0050 (0.9979–1.0374) | 450.80–459.91 |
| three-head-demand-6000 | 1960.25 / 1959.18 | 296268 / 296692 | 0.9949 (0.9910–1.0044) | 1960.44–1996.09 |
| current-name-demand-1500 | 146.49 / 146.43 | 28616 / 28616 | 0.9969 (0.9594–1.0086) | 145.34–160.34 |
| current-name-demand-6000 | 612.93 / 599.42 | 97712 / 96636 | 0.9732 (0.9665–0.9881) | 602.94–617.39 |

Common compiler paired medians span -2.68% to +1.25%.
Common peak RSS changes range from -1,076 to +424 KiB.
All block ratios and individual observations are preserved; timing variation
is not excluded or converted into an optimization-profit claim. The fixed
current-name workloads are consistent with bounded source checks and reuse of
canonical injected types. The implementation adds required semantic checks,
with no optional generated-code pass or global retry.

| Newly supported input | Final median ms (range) | Peak KiB |
|---|---:|---:|
| parenthesized-value-demand-1500 | 164.72 (163.38–166.68) | 27276 |
| parenthesized-value-demand-6000 | 681.73 (677.74–696.71) | 92304 |

| Native workload | A / B median ms | Text bytes A / B | Paired B/A median (range) | A/A range ms |
|---|---:|---:|---:|---:|
| runtime-calls | 358.50 / 359.29 | 206 / 206 | 1.0013 (0.9992–1.0027) | 357.67–359.78 |
| runtime-memory | 210.32 / 209.87 | 434 / 434 | 0.9974 (0.9941–1.0008) | 210.07–213.16 |
| runtime-floating | 248.77 / 249.06 | 230 / 230 | 1.0014 (0.9988–1.0030) | 248.67–249.32 |

Native peak RSS is 256 KiB in every observation. Text is the sectionless
executable payload after the ELF entry; these runtime inputs have no static
data. Exact executable equality rules out a generated-code regression on
these inputs. Runtime variation is reported independently.

| Scaling input | Type substitution work | Source binding work | Source type work | Occurrences |
|---|---:|---:|---:|---:|
| current-name-demand-1500 | 1500 | 22 | 3 | 105000 |
| parenthesized-value-demand-1500 | 3002 | 34 | 3 | 94500 |
| current-name-demand-6000 | 6000 | 22 | 3 | 420000 |
| parenthesized-value-demand-6000 | 12002 | 34 | 3 | 378000 |

The 4× newly supported demand workload scales to 4.14× latency and
3.38× peak RSS. Source binding/type work remains constant, while substitution
and occurrence work scales linearly. The source interpretation is shared;
instantiation consumes the recorded facts without parsing the body again.

Both intermediate campaigns remain intact:
[before the parser flag](../student.tests/pa17/name-performance-before-prefilter.json)
and [before sharing the view fallback](../student.tests/pa17/name-performance-before-shared-view.json).
The first campaign contains large timing outliers, including loop/float ABBA
blocks. None were removed. The flag avoids inspecting ordinary declarations.
Sharing the view fallback then removed 57,408 bytes of duplicated compiler
code; no timing claim is attributed solely to this layout change.

Final compiler `.text` is 1,768,518 bytes versus 1,745,478 at entry:
+23,040 bytes (1.32%) for the completed semantic group.
PA17/O0 has no mandated numerical ceiling. Required semantic costs are
measured; avoidable dispatch and code growth were removed. No runtime/code-size
benefit is claimed for an optional optimization, because none was introduced.
Existing constant-evaluator work/depth limits and lowering growth/fallback
policies remain unchanged. Inherited +15%, +16 MiB and 5.5× targets are
diagnostic under spec.md, not additional exit gates. Historical
[friend](friend-performance.md), [head](head-performance.md) and
[audit](audit-performance.md) measurements remain unchanged. This evidence
supports the completed group at PA17/O0; whole-stage independent audit remains
required.
