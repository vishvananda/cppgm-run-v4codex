# PA16 loop 39: declaration and exception facts

This is PA16 `--emit-lowir -O0`, with the supplied native backend used only for
validation. No optional optimizer was added. The change establishes required
semantic facts; it makes no runtime speedup claim. Existing scalar/storage
measurements remain in [performance.md](performance.md) and
[storage-performance.md](storage-performance.md).

## Frozen evidence and acceptance

[Harness](../student.tests/pa16/validity_benchmark.py),
[first campaign](../student.tests/pa16/validity-performance-initial.json), and
[repeat](../student.tests/pa16/validity-performance.json) preserve **476 timed
observations including warmups**. Both campaigns use frozen A (`c2ccf2d5`) and B
(`3a836964`) binaries, input hashes, `-O0`, host build flags, backend bundle/hash,
CPU affinity and all observations. Each common workload gets one warmup per
binary, four A/A samples, then two ABBA blocks. New dependent exception inputs
are rejected by A; only B's six successful observations are timed/comparable.
There is no claim that rejecting a program faster is an improvement.

All common LowIR and all four native binaries are byte-identical. Native loops
use volatile bounds and checked results; their 0.21–0.51 s runs dominate startup.
Compiler scaling inputs take approximately 0.04–0.49 s at the larger size; the
small runtime sources' roughly 6 ms compiler measurements are retained as noisy
startup diagnostics, not evidence of compiler throughput.

PA16/O0 has no mandated numeric latency, RSS or text ceiling. The inherited
+15% compiler wall / +16 MiB RSS / <=5.5x work diagnostic targets remain diagnostic;
the spec's stage-scoped acceptance governs. Required constexpr limits remain
512 active calls and 1,000,000 executed steps per root. Added work follows actual
signature parameters, class subobject edges, expression/query/initializer edges,
and demanded exception specifications, with memoized facts and no global retries.
There is no optional code-growth budget to spend: emitted executable work is
unchanged on the equivalent corpus. Self-hosting/native optimization remain
later-stage responsibilities.

## Compiler cost

Times are medians in milliseconds; RSS is the maximum KiB in that campaign.
Numbers below show the repeat; the first campaign and every outlier remain in
its JSON. Paired ratios include both ABBA blocks in both campaigns.

| Input (4,000 units) | A / B ms | A / B peak KiB | Paired B/A range |
|---|---:|---:|---:|
| Template constant calls | 69.14 / 69.22 | 13,112 / 13,076 | 0.978–1.013 |
| Memory/float functions | 396.27 / 397.86 | 70,664 / 70,996 | 0.768–1.038 |
| Literal classes and runtime member calls | 480.67 / 492.17 | 83,948 / 83,924 | 1.022–1.052 |
| Default-constructor exception queries | 151.43 / 152.38 | 25,724 / 25,952 | 0.999–1.605 |
| Dependent exception specifications (B only) | — / 248.99 | — / 36,768 | Not comparable |

Compiler `.text` grows from **1,555,206 to 1,578,566 bytes**: **23,360 bytes
(1.50%)** for required semantics. The class-heavy case consistently costs about
2–5% in paired compiler time, with at most 96 KiB extra peak RSS in these runs.
That is the measured cost of structural validity and canonical cv facts, not an
optimization tradeoff. The new dependent workload grows from 64.8 ms/13,288 KiB
to 249.0 ms/36,768 KiB for 4x source in the repeat.

Two first-campaign observations need qualification: one A memory compile took
647 ms among roughly 396–405 ms neighbors; one B default-query compile took
333 ms among roughly 150–155 ms neighbors. Their paired ratios were 0.768 and
1.605. The repeat has default-query pairs 0.999/1.004 and memory pairs
1.038/1.000. A/A ranges, context switches, CPU times and full spreads are retained;
these isolated wall-time excursions are not a repeatable implementation regression
or a claimed speedup. No sample was removed and no mandated limit was relaxed.

## Executable results and work bounds

| Runtime workload | Executable payload bytes, both A and B | Paired runtime B/A, all blocks |
|---|---:|---:|
| Calls / integer loop | 206 | 0.992–1.010 |
| Memory loop | 434 | 0.997–1.007 |
| Floating point | 230 | 0.997–1.001 |
| Literal class construction and member call | 248 | 1.000–1.008 |

These sectionless backend executables have no static data: payload after the
ELF entry is the recorded code-plus-alignment size, not a fabricated `.text`
section. Equal binary hashes give stronger evidence of unchanged executable
work than the small timing differences. Native RSS is 256 KiB throughout.

Existing-work counters scale exactly 4x from 1,000 to 4,000 units: literal
validity 6,000 -> 24,000, default exception work 5,000 -> 20,000, dependent
exception work 7,000 -> 28,000. The latter establishes 4,000 contextual exception
facts with **zero template body transitions and zero body checks**, despite
repeated queries. Graph nodes and facts are TU-owned flat vectors/indexes;
prototype/constructor scratch is invocation-local and released on return.
No second syntax graph, serialized semantic transport or host compiler delegation
was introduced. Full LowIR validation is explicit in benchmark preflight and
personal controls, separate from uninstrumented timings.
