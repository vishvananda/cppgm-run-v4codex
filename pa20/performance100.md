# PA20 retained-angle performance evidence

Scope: PA20 `--emit-lowir -O0`. Frozen entry `2e31ab57` and implementation
`7925464d` are measured by [benchmark100.py](../student.tests/pa20/benchmark100.py).
[Raw evidence](../student.tests/pa20/performance100.json) retains all **316
observations and 34 warmups**, compiler/backend/input/harness hashes, flags,
CPU affinity, A/A calibration, four ABBA blocks, paired ratios and spreads.
Compilation and execution are timed separately. Validation finished before
measurement; no other task-owned build/test ran during measurement.

Both compilers accept the seven equivalent A/B sources and every native program
checks its result. The entry compiler rejects the three newly supported sources;
those have six final-only samples per metric, with no relative benefit claim.
Runtime loops use volatile limits; startup and specialization executables are
explicitly startup-dominated controls. No optional optimization was added.

| Workload | Compiler A/B ms | Peak RSS A/B KiB | Runtime A/B ms | Native payload A/B bytes |
|---|---:|---:|---:|---:|
| startup | 8.91 / 8.76 | 5796 / 5956 | 4.31 / 4.30 | 24 / 24 |
| qualified-specializations-800 | 89.18 / 92.93 | 19904 / 20064 | 3.57 / 3.47 | 22279 / 22279 |
| qualified-specializations-3200 | 362.95 / 367.42 | 61920 / 62144 | 3.77 / 3.79 | 89479 / 89479 |
| new-retained-specializations-800 | — / 60.35 | — / 13252 | — / 3.59 | — / 24900 |
| new-retained-specializations-3200 | — / 213.82 | — / 34916 | — / 3.55 | — / 99300 |
| runtime-calls | 7.67 / 7.05 | 6112 / 6180 | 125.49 / 124.28 | 206 / 206 |
| runtime-memory | 8.76 / 8.10 | 6032 / 6148 | 73.43 / 73.84 | 434 / 434 |
| runtime-floating | 7.12 / 6.86 | 6176 / 6284 | 85.81 / 85.85 | 230 / 230 |
| new-runtime-retained | — / 7.58 | — / 6220 | — / 3676.59 | — / 180 |
| runtime-explicit-relation | 7.71 / 7.62 | 6100 / 6148 | 3693.89 / 3020.83 | 180 / 180 |

Compiler `.text` grows **2,075,910→2,090,694 bytes**, +14,784 (0.712%). Native
sizes use the established sectionless supplied-ELF payload proxy, including
static data; compiler sizes are actual `.text`. **Every A/B native executable
is byte-identical.** The new retained-grammar runtime also produces exactly the
same bytes as the equivalent explicitly spelled relation. Runtime variation,
including the large drift in the last row, is therefore not a code speedup.

## Costs, work bounds and acceptance

The 800-entity compiler pairs B/A are **1.016, 1.034, 1.003, 1.069**. A/A spans
86.52–92.01 ms; B spans 84.84–96.63 ms. At 3200, pairs are **1.025, 0.992,
1.071, 1.034**, A/A 354.85–364.74 ms, and B 356.80–422.77 ms. Medians grow
4.2% and 1.2%; peak RSS grows 160 and 224 KiB. These costs include actual
specialization/member classification before grammatical publication. Queries
visit only the qualified path and reuse canonical class demand/argument facts;
no member-body demand, global registry search or cache invalidation is added.

An initial sparse overlay design would have added a map lookup to every AST
read after an ambiguity. The final implementation instead resolves source
wrappers before publication and guards later mutation with a bit per published
source node. No such overlay or duplicate graph remains. This removes an
avoidable mechanism; no timing claim about the unmeasured prototype is made.

For the new 800/3200-specialization cases, grammar work stays exactly **4 names,
7 parts, 1 interpretation**, while demanded function bodies grow **800→3200**.
Compiler time grows 3.54×, RSS 2.63×, and native payload 3.99× for 4× the
specializations. Source interpretation is shared, and instantiation performs no
grammar replay. A relation needs two new source wrappers; declarator operands
reuse their original structure, with one wrapper for an empty call when needed.
Work and retained storage follow the consumed source/fact/IR identities.

[The trace](../student.tests/pa20/trace100.json) additionally checks two body
specializations across four calls, one source interpretation, hidden-friend
execution, and identical LowIR with telemetry/audit enabled or disabled. Its
40/600-comparison parser controls record **440→6600 angle visits** (exactly 15×
for 15× the comparisons), and 164→2404 cache hits. Failed delimiter probes are
cached only on still-live cursor tokens; consumed tokens release that state.
This prevents repeated scans of an unresolved relational suffix.

These are necessary O0 correctness costs, not optional transforms justified by
runtime profit. There is no new optimizer, code-growth policy or numeric exit
gate. Existing expansion bound 8, correctness, required LowIR shape and coverage
remain intact. The stage-scoped acceptance in `spec.md` applies; historical
diagnostic percentages in inherited plans are not additional mandated limits.
[Evidence 94](performance94.md), [95](performance95.md), [96](performance96.md),
[audit 97](performance97.md), [98](performance98.md) and [99](performance99.md)
remain preserved, including their regressions, classifications and observations.
Their cumulative interpretation remains part of independent whole-stage review.
Student native optimization, debug encoding and self-hosting belong to later
stages; PA20's native measurements use the supplied backend only in test tools.
