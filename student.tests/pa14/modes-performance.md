# Initialization modes: frozen diagnostic campaign

This campaign compares initializer checkpoint `dc112d7e` with the corrected
copy/direct/list implementation. It does **not** close performance acceptance:
the measured copy-initialization growth identifies redundant destination
materialization metadata, which the next change must remove. All observations
remain evidence, including the pre-correction measurements.

The [main data](modes-performance.json) contain 32 compiler and 11 native
workloads; the [repeat](modes-noise.json) contains seven compiler and two native
workloads. Each has one warmup per binary, four A/A observations and two ABBA
blocks: **728 observations**, **17,724 cumulative**. Binaries, flags, inputs,
outputs, backend and harnesses are frozen by hash. Timing ran without concurrent
agent builds, tests, verification or archive work. Native programs check live
runtime inputs and results; the new copy workload performs eight million
iterations, sixteen million conversions, and checks zero surviving objects and
checksum 48,000,000. Compiler and native execution are timed separately.

Compiler A SHA-256: `c85b790913e7cccb5c62a1f8db26679f9eb16b364facc82d9bc37e2334ce9421`.
Compiler B SHA-256: `669c8cb9d6e9e8d1fb5a090445ecf657438c77ea0473b9ed4852ec610eec8d19`.
Compiler `.text`: 1,368,518 → 1,369,862 bytes (**+1,344 / 0.098%**).
Analyzer: 6,520 → 6,552 bytes for one flat cache; the other 17 measured records
are unchanged. The native text metric is executable payload after the ELF entry,
as documented in the shared harness for the supplied sectionless backend.

| Workload | Main paired latency change | Repeat paired latency change | Main median peak RSS A → B (KiB) | Repeat RSS A → B (KiB) |
|---|---|---|---|---|
| Body, 4,000 statements × 128 instances | +3.04%, +0.52% | +2.61%, −0.88% | 95,390 → 95,438 | 95,560 → 95,632 |
| 1,000 declaration instances | +1.11%, +4.82% | −0.95%, −0.58% | 74,778 → 74,788 | 74,734 → 74,942 |
| Demand, 1,000 × 128 × 4 | −1.32%, −0.46% | +0.12%, −0.81% | 346,240 → 346,136 | 348,738 → 348,942 |
| Copy, 128 instances × 8 initializers | −0.07%, +1.38% | +1.99%, +1.36% | 23,814 → 24,620 | 24,096 → 24,920 |
| Copy, one instance × 4,000 calls | −1.28%, +0.21% | −2.78%, −2.48% | 26,954 → 26,888 | 26,900 → 26,168 |

The 64,000-declaration copy control measured +1.82%/+2.10%; its A/A range was
0.3261–0.3309 seconds. Other A/A examples: body 0.5935–0.6035 seconds (repeat
0.6049–0.6135), declaration 0.5458–0.6492 (repeat 0.5641–0.5667), and 128-instance
copy 0.1294–0.1313 (repeat 0.1334–0.1363). Full spreads and all other workloads
remain in the data. No frontend latency improvement is claimed.

All **11 executables are byte-identical**, with text payloads 2360, 2072, 261,
206, 206, 434, 230, 300, 444, 1456 and 1456 bytes. Copy runtime paired changes
were +0.20%/+1.91%, then −0.79%/+2.70%; the inherited initializer runtime was
−0.51%/−1.02%, then +0.32%/−0.87%. Default runtime includes +4.41%/−0.31% amid
an A/A range of 0.6656–0.7534 seconds. No native runtime improvement is claimed.

Source recipes remain **2M**, concrete uses **2KM**, independent of unrelated
N declarations and Q repeated calls. Candidate visits are unchanged. The new
copy path preserves the scalar source and removes duplicate expression facts,
but materialization adds KM entities/scopes and conversion objects despite an
existing destination: K=128, M=8 adds 1,024 entities and scopes. This is the
identified avoidable cost, not a waived numerical target. The correction must
retain selected conversions, access/destructor checks and destination lifetime.

PA14 O0 introduces no optional runtime transform. Native selection, allocation,
ELF writing, optimization levels and self-hosting remain later-stage owners.
There is no new numerical latency/RSS/text exit gate. Correctness, complete cache
keys, bounded source/concrete work and the required course/file-audit gates remain
mandatory. The next campaign must compare correct implementations and retain
these measurements rather than replace them.

[Validation](modes-validation.json) records 71 passing groups, including both
required gates, 349 entry/current and sanitizer comparisons, both builds' 75
mode controls, 82 initializer controls, 64 statement controls, repeated body and
cache inspections, inherited demand/lifecycle/virtual controls and PA13 ABI checks.
The [proof](modes-proofs.json) preserves 35 entry status defects and three native
failures among 75 controls; its legal basis is the cited local N3485 clauses.
All 1,266 fixture/reference hashes remain unchanged. The interrupted first
validation and initial binaries are preserved separately; no timing used them.
Five default/query reproducers remain open in [the handoff](modes-handoff.json).
