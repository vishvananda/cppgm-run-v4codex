# PA18/O0 performance — checkpoint audit 66

Frozen stage base `94dcb8ad`, checkpoint entry `0619704c`, and reviewed tip
`3a883d10` were measured using [audit66_benchmark.py](../student.tests/pa18/audit66_benchmark.py).
[Checkpoint observations](../student.tests/pa18/loop66-performance.json) and
[cumulative observations](../student.tests/pa18/loop66-cumulative-performance.json)
retain every source byte/hash, binary/backend/harness hash, flag, CPU affinity,
warmup, timed observation, A/A range, paired ABBA ratio, telemetry and checked
native output/size. Each run covers the same 25 workloads, the union of the
fixed handoff 63–65 corpora plus two completion/recomputation scales.

Protocol: one warmup each, four A/A samples, four ABBA wall-time blocks.
Compile time and peak RSS (`/usr/bin/time`) are separate from executable runtime.
Telemetry, full LowIR validation and native construction are untimed. Newly
required behavior rejected by A has six B-only samples; an incorrect/rejecting
compiler is not treated as an equivalent performance baseline. Common LowIR and
executables are byte-identical before accepting timing. Volatile loop bounds
and checked results retain runtime calls, memory and floating-point work.

[The first checkpoint run](../student.tests/pa18/loop66-performance-attempt1.json)
is preserved in full. A disk-usage inventory overlapped its early samples, which
had unusually broad spread. The complete clean repeat above uses the same frozen
binaries/inputs. No competing build, test or storage scan ran during the clean
cumulative and checkpoint runs. Historical handoff measurements remain unchanged.

## Checkpoint compiler measurements

All times below are median milliseconds; ranges are paired B/A block ratios.
RSS is maximum observed KiB. A is checkpoint entry, B is the reviewed compiler.

| Workload | A ms | B ms | Paired B/A median (range) | RSS A / B KiB |
|---|---:|---:|---:|---:|
| ordering-600 | 54.61 | 54.03 | 0.990 (0.981–0.994) | 15228 / 15180 |
| nested-cv-600 | 56.37 | 55.53 | 0.986 (0.984–0.995) | 15308 / 15316 |
| pack-result-600 | 124.21 | 124.27 | 0.999 (0.994–1.002) | 25476 / 25468 |
| ordering-2400 | 211.34 | 210.86 | 0.995 (0.905–1.001) | 43380 / 43352 |
| nested-cv-2400 | 217.89 | 217.49 | 0.994 (0.917–0.999) | 43928 / 43924 |
| pack-result-2400 | 514.66 | 511.04 | 0.993 (0.978–1.004) | 85060 / 85312 |
| common-loop-float-1500 | 153.85 | 152.58 | 0.995 (0.660–0.997) | 30208 / 30248 |
| explicit-discard-600 | 46.87 | 46.87 | 0.997 (0.988–1.000) | 13952 / 13952 |
| incomplete-size-600 | 41.06 | 41.05 | 0.999 (0.984–1.003) | 12820 / 12836 |
| explicit-discard-2400 | 178.66 | 179.65 | 1.007 (0.995–1.047) | 38240 / 38260 |
| incomplete-size-2400 | 152.11 | 152.07 | 1.004 (0.990–1.071) | 33424 / 33436 |
| conversion-600 | 64.06 | 63.79 | 0.994 (0.992–1.014) | 16616 / 16616 |
| conversion-defaults-600 | 75.89 | 75.49 | 0.999 (0.985–1.000) | 18512 / 18872 |
| member-head-600 | 96.84 | 96.34 | 0.995 (0.988–0.998) | 22680 / 22600 |
| conversion-2400 | 249.53 | 248.90 | 1.003 (0.993–1.333) | 49348 / 49340 |
| conversion-defaults-2400 | 295.07 | 295.37 | 0.996 (0.924–1.004) | 57028 / 57024 |
| member-head-2400 | 392.66 | 392.39 | 0.997 (0.991–1.078) | 72840 / 73252 |
| completion-rewarm-600 | — | 32.27 | new required behavior | — / 9404 |
| completion-rewarm-2400 | — | 109.99 | new required behavior | — / 20400 |

Startup B median is 5.46 ms. Scaled frontend workloads dominate startup;
compilation of the tiny runtime sources is diagnostic only (all observations
are retained in JSON). No compiler speedup is claimed. Paired median increases on
common scaled inputs are at most about 0.7%, while other inputs vary slightly
downward. The local revision lookup is confined to failed consumers and
invalidated queries; no common successful lookup scans dependency edges.

The full paired ranges retain outliers in both directions, including the
conversion-2400 block at 1.333 and loop/float block at 0.660. These do not
establish a repeatable change; no observations were removed.

Representative A/A compiler ranges in ms:

- ordering-2400: 210.48–214.56.
- pack-result-600: 123.41–144.93.
- explicit-discard-2400: 176.51–178.75.
- member-head-2400: 391.39–395.72.

## Cumulative compiler comparison

| Common workload | Stage base ms | Reviewed ms | Paired B/A median (range) | RSS A / B KiB |
|---|---:|---:|---:|---:|
| ordering-600 | 56.04 | 55.23 | 0.991 (0.978–1.020) | 15036 / 15156 |
| ordering-2400 | 216.55 | 212.64 | 0.980 (0.973–1.018) | 43244 / 43344 |
| common-loop-float-1500 | 153.51 | 152.64 | 0.995 (0.991–0.998) | 30120 / 30248 |

The cumulative common comparison shows no repeatable regression. The other
scaled semantic inputs are rejected at stage base and retain B-only necessary
costs in the cumulative JSON. Their correct checkpoint entry implementations
are compared A/B above; no missing semantics is credited as an optimization.

## Checked executable runtime and size

The checkpoint A already implements all six runtime controls. Medians and
paired ranges are below; all A/B LowIR, executable hashes and sizes are identical.
The supplied backend emits sectionless ELF; bytes after the entry point are the
available **text/alignment payload proxy**, not a claimed section `.text` size.
These six runtime sources have no static data.

| Workload | A seconds | B seconds | Paired B/A median (range) | Payload bytes A = B |
|---|---:|---:|---:|---:|
| runtime-calls | 0.7148 | 0.7137 | 0.999 (0.998–1.000) | 206 |
| runtime-memory | 0.4167 | 0.4170 | 1.000 (1.000–1.002) | 434 |
| runtime-floating | 0.4947 | 0.4942 | 0.999 (0.997–1.000) | 230 |
| runtime-new-ordering | 0.2246 | 0.2243 | 0.999 (0.995–1.001) | 173 |
| runtime-new-substitution | 0.2246 | 0.2249 | 1.002 (0.955–1.012) | 177 |
| runtime-new-conversion | 0.2326 | 0.2329 | 1.000 (0.997–1.001) | 181 |

These runtime differences measure noise in identical executables. A/A ranges
and every observation are retained; they are not generated-code speedup claims.
The cumulative run also verifies the three common loop/call/memory/floating
executables as identical to stage base; its three new semantic runtime controls
have six final-only samples and the same reviewed payload sizes shown above.

## Scaling, budgets and stage acceptance

- ordering, 600→2400: 3.90× time, 2.86× peak RSS.
- nested-cv, 600→2400: 3.92× time, 2.87× peak RSS.
- pack-result, 600→2400: 4.11× time, 3.35× peak RSS.
- explicit-discard, 600→2400: 3.83× time, 2.74× peak RSS.
- incomplete-size, 600→2400: 3.70× time, 2.60× peak RSS.
- conversion, 600→2400: 3.90× time, 2.97× peak RSS.
- conversion-defaults, 600→2400: 3.91× time, 3.02× peak RSS.
- member-head, 600→2400: 4.07× time, 3.24× peak RSS.
- completion-rewarm, 600→2400: 3.41× time, 2.17× peak RSS.

Completion-rewarm controls hold the number of invalidations at one while
pending class/query edges scale linearly (also checked at 32/128/512). This
exercises failed alias/signature prerequisites after an independent consumer
recomputes the shared query. The new failed-consumer slot is reused on retry;
only invalidated query identities acquire revisions. There is no global epoch.
Ordering comparison misses visit nominated types/queries; completed comparisons
use canonical O(1)-average lookup. Specialization/frame/edge storage releases
with the TU, and local comparison/worklist scratch releases on return.

Compiler `.text`: stage base 1,814,470 → checkpoint 1,836,422 →
reviewed 1,837,766 bytes. Audit growth is 1,344 bytes
(0.073%); cumulative growth is 23,296 bytes
(1.284%). Common generated-code growth is zero.

PA18 requires O0 LowIR, not native output or optimized LowIR. There is no
mandated numeric latency/RSS/runtime ceiling. The inherited PA17 plan explicitly
classifies **+15%, +16 MiB and 5.5×** as diagnostic targets under spec §9; that
classification is preserved, with all historical observations. No required
correctness, coverage, graph-work or emission/growth bound is relaxed (including
the inherited bounded array expansion policy). New semantic costs are necessary
correctness work, measured above; they do not excuse avoidable regressions.
No unprofitable optional transform remains because none is introduced in this
range. O0 retains required calls, effects, ABI and ordering; the constant-reference
change has a C++11 proof. Native allocator/spill/loop policies, native text/runtime
optimization budgets and self-host evaluation remain PA24–PA34 ownership.

The current stage-scoped performance acceptance passes.
