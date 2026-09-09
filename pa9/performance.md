# PA9 final performance evidence

**All 42 unchanged checks pass in each final experiment (84/84).** Final B is
`0e2fcf0ff`; whole-stage A is first-correct `2b19ba07a`, and audit-delta A is
completed checkpoint `8195487d1` (implementation `b43bc1ff7`). Compiler latency,
peak RSS and compiler text are reported together. PA9 emits ABI names; generated
executable runtime/text and executable optimization profitability are **N/A**.

## Frozen protocol and provenance

The [protocol](../student.tests/pa9/final-audit-protocol.md) was committed before
audit changes/timing; the separate whole-stage comparison was specified before
any final observations. Each experiment freezes binaries, flags, and eight
inputs: unique template values, dependent expressions, long modifiers and
independent local/template cases, each at 1x/4x sizes. The final binary was
committed before timing. Both historical A binaries were rebuilt from their
commits and their hashes match the original checkpoint evidence.

Build: g++ 15.2.0, `-std=gnu++11 -Wall -O3`, default shared test runner
(entry `-Dmain=test_runner_real_main`, runner `-DTEST_RUNNER_ENABLE`). The raw
records include actual compile/runner configuration, `dev` tree IDs, CPU 0
pinning, platform, all binary/input/output hashes and counters. No builds/tests
ran concurrently with timing. Each workload uses AAAA noise calibration then
two wall-time ABBA blocks. All timed output digests are checked against equal
A/B outputs. Observations last 0.21–2.16 seconds, well above process startup.

- First-correct A: `78784463b5cbd4e73bf7b69d6827314f968e1c2b6eace34d1cc6c3a2ae94ccf8`.
- Checkpoint A: `0ee66201ab6965fd5c8c524d9dbdc0a77131b9ef38ec47a107f9b6eda263b78c`.
- Final B: `5a179d13982dd4e1377e22f9297e6f6041224d979d5c6e4edcc7df757da2c3c0`.

## Whole-stage results

[Final whole-stage data](../student.tests/pa9/final-stage-performance.json)
retains all 96 observations. Wall/RSS columns are medians of the four ABBA
observations for each binary. Changes are means of the two paired block ratios;
brackets give their range. This experiment directly tests the original stage
budgets, without combining ratios from historical runs.

| Workload | A / B wall (s) | Paired change [block range] | A / B peak RSS (KiB) |
| --- | ---: | ---: | ---: |
| Templates, 96k | 0.371 / 0.393 | +4.90% [+4.04, +5.76] | 30186 / 30452 |
| Templates, 384k | 1.541 / 1.701 | +9.19% [+6.65, +11.73] | 107972 / 113482 |
| Expressions, 72k | 0.340 / 0.338 | -0.55% [-1.63, +0.53] | 29790 / 31066 |
| Expressions, 288k | 1.446 / 1.566 | +6.79% [+6.78, +6.80] | 112946 / 117662 |
| Modifiers, 9.6k cases | 0.480 / 0.299 | -37.83% [-38.69, -36.97] | 8500 / 3952 |
| Modifiers, 38.4k cases | 1.899 / 1.176 | -38.07% [-38.38, -37.77] | 23092 / 3962 |
| Batch, 24k cases | 0.213 / 0.224 | +4.96% [+4.33, +5.59] | 5458 / 3558 |
| Batch, 96k cases | 0.839 / 0.877 | +5.87% [+4.68, +7.07] | 11600 / 3692 |

Modifiers improve **37.8–38.1%** on paired means; every individual block improves
37.0–38.7%, far beyond their A/A ranges (0.478–0.484 s and 1.880–1.905 s).
This repeats the earlier iterative/streaming improvement. Other workload
regressions of **4.9–9.2%** are disclosed and remain within 25%; the small
expression case's −0.55% result is not a speedup claim. No gain is attributed
solely to fewer graph nodes or substitutions.

Fourfold B inputs scale wall **3.91–4.63x** and single-case RSS **3.73–3.79x**;
batch/modifier RSS stays **1.00–1.04x**. All four measured work counters
(intern requests/probes, substitution lookups and emitted nodes) stay within
4.5x. Optional statistics observe existing work; only phase clocks/aggregation
are enabled by `--stats`, outside the timed comparisons.

## Audit delta and noise

[Final audit-delta data](../student.tests/pa9/final-audit-performance.json)
retains another 96 observations with the identical final B and inputs. This
comparison isolates the audit repair bundle relative to the completed stage.

| Workload | Paired change [block range] | A/A wall range (s) |
| --- | ---: | ---: |
| Templates, 96k | +2.59% [-0.32, +5.50] | 0.380–0.415 |
| Templates, 384k | -0.54% [-2.06, +0.98] | 1.619–1.690 |
| Expressions, 72k | +4.31% [+4.17, +4.45] | 0.335–0.370 |
| Expressions, 288k | -10.45% [-20.03, -0.86] | 1.449–1.485 |
| Modifiers, 9.6k cases | +5.89% [+5.86, +5.91] | 0.278–0.321 |
| Modifiers, 38.4k cases | +5.75% [+4.87, +6.63] | 1.105–1.113 |
| Batch, 24k cases | +0.84% [+0.33, +1.35] | 0.218–0.222 |
| Batch, 96k cases | +3.28% [+0.37, +6.18] | 0.852–0.867 |

No audit speedup is claimed. In particular, the large-expression −10.45%
paired mean is affected by an A outlier of 2.157 s (A observations span
1.477–2.157 s); its two block ratios disagree materially. The observation is
retained. Positive paired changes are **0.8–5.9%**; the added local validity
checks have bounded work and pass both delta and whole-stage budgets. Delta
RSS is nearly unchanged (largest median increase: 124 KiB).

## Text, legality and budgets

Compiler text: first-correct A **172418 bytes**, checkpoint A **265664**, final B
**274663**. Audit growth is **8999 bytes (8.8 KiB, 3.4%)**; whole-stage growth is
**102245 bytes (99.85 KiB, 59.3%)**, within the original **102400-byte** budget
by 155 bytes. The large total includes the reusable serializer and telemetry;
it is disclosed rather than presented as generated-code size. Shared tag
ordering and borrowed spellings reduced duplicated compiler code while retaining
all required behavior. No generated-program text is produced or measured.

Frozen gates: wall ≤1.25x A; RSS ≤1.20x A +16 MiB; fourfold wall ≤5.5x,
measured work ≤4.5x, RSS ≤5x; compiler text growth ≤100 KiB. The extra 42nd
check repeats the total-stage text bound even in the checkpoint comparison.
Construction checks immediate edges only; canonicalization rewrites a bounded
local shape. Substitution is mandatory grammar, so there is no profitability
choice, optimizer level or generated-code growth. The [audit](audit.md)
records proof ownership, immutable-cache validity, recursion bounds and release.

## Retained evidence and reproduction

All **576** observations across six reports remain separate:

- `initial-performance.json` and `performance.json`: original checkpoint runs,
  including the first failed compiler-text budget and its correction.
- `pre-prefix-audit-performance.json` and `pre-prefix-stage-performance.json`:
  192 successful audit observations on `f2353211e`. A final completion probe
  then found the malformed specialization-prefix crash. These runs are retained
  as historical evidence and do not certify the final implementation.
- `final-audit-performance.json` and `final-stage-performance.json`: 192 final
  observations on `0e2fcf0ff`, all 84 gates pass.

No sample was discarded, no budgets were relaxed, and independent experiments
are not pooled. The [personal README](../student.tests/pa9/README.md) provides
rebuild and reproduction commands. The verifier recomputes ratios, medians,
spread and every gate, validates raw ordering and hashes, and checks that the
current `dev` source tree/binary still matches final B. Historical mode checks
older frozen experiments explicitly without claiming they are current.
