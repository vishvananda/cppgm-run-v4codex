# PA18 performance evidence 79

Acceptance is PA18/O0 LowIR under spec §9. This is required semantic work, with
no new optimization pass or optional transform. Current-stage graph bounds and
correctness remain mandatory; there is no mandated numerical latency/RSS ceiling.
Historical +15%, +16 MiB and 5.5× diagnostic targets remain non-gates for the
reasons and measurements retained in [performance 78](performance78.md). Earlier
observations are preserved; native optimization/self-hosting remain PA24–PA34.

Frozen A is entry `ef0e43c0`; B contains implementation `50407fc1`. Both batches
use the same binary hashes. [Main observations](../student.tests/pa18/loop79-performance.json)
and [explicit-member observations](../student.tests/pa18/loop79-performance-members.json)
retain all samples, source text/hashes, flags, CPU affinity, platform, telemetry,
checked exits and output hashes. The main harness version is committed in
`42312227`; its member-argument extension is in `00738e66`.

Build: `g++ -std=gnu++11 -Wall -O3`, test runner enabled. Timed flags:
`--emit-lowir -O0`. A separate preflight uses `--validate-lowir --stats`.
Each comparable workload has a warmup per binary, four A/A noise samples and
four ABBA blocks. New behavior uses six B-only observations after a warmup;
rejected A compilations are never timed as a performance comparison. The
supplied native backend uses `-O0`, bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`; backend construction is outside compiler
and executable timings. Every emitted executable checks its result. Runtime
loops use volatile bounds and data-dependent calls/arithmetic/memory, preventing
a constant-folded or dead timing workload.

All ten equivalent sources have byte-identical LowIR and, where emitted,
byte-identical executables. Eight newly supported sources have final-only costs.
Compiler size is actual `.text`; supplied sectionless executable sizes below
are payload proxies, as in earlier evidence, not claimed section measurements.

## Compiler latency and peak RSS

Times are median milliseconds; paired ratios are medians of ABBA block ratios,
with the full block range. RSS is maximum observed KiB. All outliers remain.

| Workload | A / B ms | Paired B/A (range) | A / B RSS KiB |
|---|---:|---:|---:|
| ordering-600 | 54.94 / 54.94 | 0.994 (0.989–1.008) | 15196 / 15356 |
| ordering-2400 | 210.95 / 214.09 | 1.019 (0.901–1.067) | 43636 / 43752 |
| member-head-600 | 95.62 / 97.17 | 1.007 (1.002–1.051) | 22620 / 22656 |
| member-head-2400 | 394.44 / 394.43 | 0.981 (0.921–1.048) | 73604 / 73676 |
| common-loop-float-1500 | 154.42 / 153.39 | 0.992 (0.786–1.094) | 30288 / 30436 |
| runtime-calls | 5.90 / 5.73 | 0.977 (0.962–0.980) | 5832 / 6052 |
| runtime-memory | 5.85 / 5.67 | 0.961 (0.951–0.984) | 5896 / 5932 |
| runtime-floating | 6.18 / 6.00 | 0.966 (0.955–0.978) | 6036 / 6144 |
| first-signature-600 | — / 65.65 | new behavior | — / 13844 |
| first-member-600 | — / 129.49 | new behavior | — / 27300 |
| qualified-alias-600 | 66.61 / 66.21 | 0.990 (0.975–1.000) | 16108 / 16172 |
| first-signature-2400 | — / 249.64 | new behavior | — / 38188 |
| first-member-2400 | — / 542.80 | new behavior | — / 93052 |
| qualified-alias-2400 | 260.99 / 257.13 | 0.978 (0.919–0.993) | 45792 / 45788 |
| runtime-first-signature | — / 6.54 | new behavior | — / 6080 |
| member-arguments-600 | — / 90.83 | new behavior | — / 19524 |
| member-arguments-2400 | — / 369.76 | new behavior | — / 60204 |
| member-arguments-runtime | — / 6.63 | new behavior | — / 6016 |

Main startup medians are 5.43 / 5.31 ms. The small
runtime-source compiler samples are startup-limited diagnostics. Scaled sources
dominate startup. Among the large equivalent cases, paired ratios range from
0.978 to 1.019 with mixed noisy blocks; peak RSS increases by at most 160 KiB.
Ordering-2400 adds 3.15 ms at the median (paired +1.9%); the other large
equivalent cases are within their noisy baseline. This small observed cost is
reported with the required signature/query tracking. No compiler speedup is claimed.
The new signature/member costs are required semantics and have no correct A
counterpart. No optional transformation is using those costs as an exemption.

## Executable runtime and size

Runtime A/B are median milliseconds; ranges below are full paired block ranges.
Payload sizes are bytes. All native results are checked and no implementation
uses the reference backend except the explicitly invoked course/personal harness.

| Workload | A / B ms | Paired B/A (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 714.99 / 714.08 | 0.998 (0.996–1.000) | 206 / 206 |
| runtime-memory | 417.11 / 417.30 | 1.001 (1.000–1.002) | 434 / 434 |
| runtime-floating | 494.98 / 495.93 | 1.002 (0.999–1.003) | 230 / 230 |
| runtime-first-signature | — / 59.41 | B only (59.21–78.12 ms) | — / 177 |
| member-arguments-runtime | — / 78.24 | B only (77.75–78.57 ms) | — / 219 |

Equivalent executable bytes are identical; runtime ratios (0.998–1.002) are
measurement variation. Compiler `.text` grows from **1,945,158 to 1,958,534 bytes**,
**+13,376 (0.69%)**, for the required parser, signature and query
logic. Source workloads with many distinct demanded functions retain proportional
O0 code size; no unrolling, inlining or code-duplication policy was introduced.
All source/output sizes, including non-runtime probes, remain in the raw records.

## Work bounds and acceptance

The budget is one first-signature record per redeclared entity, one cached shape
per argument/query key, immutable substitution-frame facts, source-local head
maps and raw-parameter traversals only where signature semantics differ.
Parser work adds a binding alongside each existing open-angle entry. Explicit
member arguments use the existing typed argument and expansion caches. ABI
projection reads a selected declaration and encodes it through existing facts.
These are source/demand bounds, with no whole-program retries or new optimizer
work allowance. Translation-unit vectors/indexes own retained facts; local maps
are discarded after their operation.

At 600 → 2400 declarations, first-signature type substitution work is
7,800 → 31,200; query work is 4,209 → 16,809; signature-shape work stays 9.
For first-member bodies, substitution is 12,014 → 48,014 and query work is
6,625 → 26,425. Retained signature work stays 2, prototype matching stays 1,
source parameter checking stays 81, and concrete signature uses are 1,200 →
4,800. This follows demanded facts, without replaying source checks per body.
The member-argument counters and all earlier completion/region/definition scaling
checks are retained in the evidence manifest. Their costs are also linear in
explicitly demanded instantiations.

Required correctness, unchanged coverage apart from the proved reference error,
all earlier PAs, bounded work and the measured absence of optional regressions
satisfy this group's PA18/O0 performance acceptance. The remaining 27 course
failures still prevent full-stage acceptance.
