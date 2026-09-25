# PA18 handoff 76 performance evidence

Acceptance: **PA18, unoptimized LowIR (`-O0`)**, spec §9. No optional optimization
or mandated numeric ceiling was introduced. Compiler cost implements the required
candidate/forwarding facts; no runtime optimization benefit is claimed.

## Protocol

[`benchmark76.py`](../student.tests/pa18/benchmark76.py) freezes entry `b87de70e`
and final `c8a2aad8` binaries, flags and source texts. Raw warmups, A/A samples,
four ABBA blocks, paired ratios/spreads, peak RSS, platform/CPU, binary/backend
hashes and output checks are preserved in
[loop76-performance-final.json](../student.tests/pa18/loop76-performance-final.json).
The earlier `bff709db` measurements remain unchanged in
[the first batch](../student.tests/pa18/loop76-performance.json); the final batch
follows the omitted-type demand fix.
Each equivalent workload has one warmup per binary, four A/A observations and
sixteen ABBA observations. Sources rejected at entry have six final-only samples;
comparing failed compilation with correct compilation would be misleading.

Compiler flags: `--emit-lowir -O0`; build: `g++ -std=gnu++11 -Wall -O3`, test runner enabled.
Preflight separately enables `--validate-lowir --stats`. Execution uses the supplied
native backend at `-O0`, pinned bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
Every executable exits zero. Equivalent LowIR and executable hashes match exactly.

## Compiler latency and memory

| Workload | A / B median ms | Paired B/A median (range) | A / B peak RSS KiB |
|---|---:|---:|---:|
| ordering-600 | 55.86 / 55.73 | 1.005 (1.002–1.331) | 15288 / 15288 |
| ordering-2400 | 214.62 / 215.64 | 1.001 (0.955–1.013) | 43744 / 43788 |
| member-head-600 | 100.61 / 101.16 | 1.018 (0.989–1.328) | 22784 / 22964 |
| member-head-2400 | 396.58 / 390.84 | 0.983 (0.971–0.996) | 73772 / 73064 |
| common-loop-float-1500 | 151.60 / 152.04 | 1.006 (0.987–1.065) | 30336 / 30272 |
| runtime-calls | 5.76 / 5.72 | 0.996 (0.978–1.020) | 6000 / 6080 |
| runtime-memory | 6.03 / 6.10 | 1.021 (1.006–1.034) | 6052 / 6056 |
| runtime-floating | 5.99 / 6.08 | 1.017 (0.979–1.046) | 6128 / 6144 |
| inherited-declaration-600 | 54.58 / 54.11 | 0.996 (0.982–1.009) | 14488 / 14700 |
| inherited-query-600 | — / 59.46 | new behavior | — / 15944 |
| inherited-declaration-2400 | 213.55 / 211.59 | 0.990 (0.924–0.999) | 40248 / 39972 |
| inherited-query-2400 | — / 234.75 | new behavior | — / 44680 |
| inherited-body-160 | — / 35.30 | new behavior | — / 11580 |
| inherited-body-640 | — / 131.02 | new behavior | — / 28580 |
| runtime-inherited | — / 6.33 | new behavior | — / 6080 |

Startup medians: **5.48 / 5.38 ms**. Small runtime-source compilations are
startup-limited; the scaled frontend workloads dominate startup. All observations
and A/A spreads remain in the raw record. No general compiler speedup is claimed.
The largest equivalent peak-RSS increase is **212 KiB**, on inherited-declaration-600.
Timing variation is visible in the paired ranges; equivalent measured workloads
show no repeatable material regression requiring a new optimization.

Compiler `.text`: **1,924,614 → 1,941,318 bytes**, **+16,704 (0.868%)**.
Generated equivalent executables do not grow. New behavior necessarily emits
the derived wrapper and demanded base body once per specialization; this is a
language implementation cost, not a code-growth optimization.

## Checked runtime and size

Volatile bounds and data-dependent loop results prevent folded/dead workloads.
The supplied backend emits sectionless ELF; payload bytes are the same recorded
code-size proxy used in earlier handoffs, not a fabricated `.text` measurement.

| Workload | A / B median seconds | Paired B/A median (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 0.720137 / 0.717106 | 1.000 (0.967–1.053) | 206 / 206 |
| runtime-memory | 0.418807 / 0.419571 | 0.996 (0.982–1.008) | 434 / 434 |
| runtime-floating | 0.495792 / 0.495072 | 1.000 (0.996–1.006) | 230 / 230 |
| runtime-inherited | — / 0.116167 | new behavior | — / 187 |

Common executable bytes are identical, so their runtime differences are
environmental observations. The inherited-constructor runtime has no correct
entry executable for the same source. No native runtime gain is claimed.

## Work bounds and acceptance

At 600 → 2400 classes, inherited declaration/query compiler medians grow
**3.91× / 3.95×** for fourfold input. At 160 → 640 demanded bodies, time grows
**3.71×**. The twelve independent graph controls at 32/128/512 establish:

| Use | Forwarding argument records | Base template bodies |
|---|---:|---:|
| Declarations only | 0 / 0 / 0 | 0 / 0 / 0 |
| Noexcept queries | 32 / 128 / 512 | 0 / 0 / 0 |
| Body demand | 32 / 128 / 512 | 32 / 128 / 512 |
| Repeated query, one specialization | 1 / 1 / 1 | 0 / 0 / 0 |

Recipe construction is linear in the selected parameter/default edges. Class
completion works on the inherited candidate set; notional default-omission
signatures count their actual parameter edges. Specialization identity lookup
and completed forwarding lookup are O(1) average. One template body per demanded
base specialization is checked; derived wrappers reuse typed actions. There is
no optional transform, iteration-to-fixed-point, or unlimited cloning policy.

Historical **+15%, +16 MiB, 5.5×** diagnostic targets remain non-mandated; all
earlier measurements in records 63–75 remain intact. Correctness, coverage and
current-stage graph bounds remain gates. Later native optimization/self-hosting
costs belong to PA24–PA34. This stage-scoped acceptance does not excuse PA18’s
35 remaining course failures or replace independent review.
