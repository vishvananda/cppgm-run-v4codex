# PA9 performance evidence

The final implementation passes all **41 frozen budget checks**. PA9 emits ABI
names, so generated-program runtime and text size are **not applicable**.
The text sizes below describe the compiler executable itself. No runtime or
code-quality claim is made for generated programs, and earlier compiler phases
are unchanged.

## Protocol and provenance

Baseline A is the first correct encoder, `df7dbb00a`, rebuilt independently from
that commit. Its binary exactly matches the originally frozen binary:
`78784463b5cbd4e73bf7b69d6827314f968e1c2b6eace34d1cc6c3a2ae94ccf8`.
Final B is `ea87f4ba7`:
`0ee66201ab6965fd5c8c524d9dbdc0a77131b9ef38ec47a107f9b6eda263b78c`.
Both use g++ 15.2.0, `-std=gnu++11 -Wall -O3`, and the default shared batch
runner build configuration. No LTO, different optimization flags or alternative
mangling implementation is used.

Eight fixed workloads cover unique template values, dependent expressions,
long modifier chains and many independent local/template cases, each at two
sizes differing by four. Each workload runs AAAA noise calibration followed by
two ABBA blocks, pinned to CPU 0 after builds and correctness checks finish.
Compiler wall time and `/usr/bin/time` peak RSS are measured separately from
output comparison. Every timed result matches the frozen output digest.
The final workloads take about 0.2–1.9 seconds per observation; process startup
is small relative to these workloads. All 96 final observations, input/output
and binary hashes, timings, RSS values, phase counters and block spreads are in
[performance.json](../student.tests/pa9/performance.json).

The initial 96-observation run is retained in
[initial-performance.json](../student.tests/pa9/initial-performance.json).
It passed latency, RSS and scaling checks but failed the compiler text-growth
budget: 296585 versus 172418 bytes. Sharing the serializer's joining code and
iterating its linear type dependencies reduced final text to 265664 bytes.
Final timings use fourfold longer inputs; the initial run is not pooled with
the final run and no observations were removed.

## Results

Wall change is the mean of two paired ABBA block ratios; wall/RSS columns are
medians of the four observations per binary in those blocks. All observations,
including slower outliers, remain in the estimates.

| Workload | A wall (s) | B wall (s) | Paired wall change | A / B peak RSS (KiB) |
| --- | ---: | ---: | ---: | ---: |
| Templates, 96k | 0.364 | 0.378 | +6.37% | 30198 / 30454 |
| Templates, 384k | 1.551 | 1.666 | +6.40% | 107976 / 113356 |
| Expressions, 72k | 0.324 | 0.333 | +1.87% | 29804 / 31064 |
| Expressions, 288k | 1.414 | 1.458 | +3.40% | 112944 / 117558 |
| Modifiers, 9.6k cases | 0.478 | 0.284 | −40.75% | 8494 / 3944 |
| Modifiers, 38.4k cases | 1.897 | 1.119 | −41.11% | 23102 / 3682 |
| Batch, 24k cases | 0.213 | 0.219 | +2.67% | 5454 / 3566 |
| Batch, 96k cases | 0.840 | 0.861 | +1.92% | 11592 / 3692 |

The modifier improvement repeats in both final ABBA blocks (40.3–41.2% faster)
and is much larger than A/A spread. The smaller regressions are disclosed;
there is no claim that the whole change speeds every workload. Sparse
substitution storage removes work proportional to unrelated translation-unit
facts; direct API counters independently verify that invariant. Streaming case
output explains the bounded batch/modifier RSS as the number of cases grows.
No per-change attribution beyond these algorithmic facts is inferred from the
bundle comparison.

Fourfold inputs scale final wall by **3.93–4.41x**. Single-case peak RSS scales
**3.72–3.78x**; batch/modifier RSS is approximately constant (**0.93–1.04x**).
Intern requests/probes, substitution lookups and emitted-node counts satisfy
the fixed 4.5x work bound.

## Budgets and reproduction

Budgets were committed before timing and unchanged after the first failure:
paired compiler wall ≤1.25x A; peak RSS ≤1.20x A +16 MiB; compiler text growth
≤100 KiB; fourfold wall ≤5.5x, measured work ≤4.5x, and peak RSS ≤5x.
Final compiler text grows **93246 bytes (91.1 KiB, 54.1%)**, including the added
serializer and telemetry, within that absolute budget. Output name bytes are
identical on A/B workloads. There are no generated-code transformations or
executable work/growth budgets in this assignment.

The [personal README](../student.tests/pa9/README.md) reproduces builds, fixtures,
sanitizers and timing. `verify_performance.py` recomputes all gates from raw
observations, verifies frozen/current binaries and inputs, and confirms that
subsequent commits do not alter implementation sources.
