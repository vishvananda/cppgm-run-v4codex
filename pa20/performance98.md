# PA20 capture environment performance evidence

Scope: PA20, `--emit-lowir -O0`; native execution uses the supplied backend.
No optimization pass, inlining, unrolling or optional code growth was added.
The new work is required capture semantics and pointer-field initialization.
Historical percentage targets remain diagnostics under spec §9, as established
by [audit 97](performance97.md); all its observations and mandated limits remain.

[Final observations](../student.tests/pa20/performance98.json) freeze entry
`e75e0d6c` and implementation `a1faea7a`, compiler/input/backend hashes, flags,
CPU affinity, every warmup and sample. [Initial](../student.tests/pa20/performance98-initial.json)
and [pack-completion](../student.tests/pa20/performance98-pack.json) measurements
are retained, including noise and intermediate compiler hashes. Each complete
run has 288 observations and 32 warmups: one warmup per binary, four A/A
calibration observations, four wall-time ABBA blocks for compiler and native
execution separately; new behavior has six final-only samples. No rejected
entry program is used as a faster baseline. All executables check their result;
volatile loop limits keep runtime workloads live. Startup/scale executable
timings are explicitly startup-dominated controls, not runtime-profit claims.

| Final workload | Compiler A/B ms | Peak RSS A/B KiB | Runtime A/B ms | Native payload A/B bytes |
|---|---:|---:|---:|---:|
| startup | 8.34 / 7.78 | 5548 / 5912 | 4.21 / 4.19 | 24 / 24 |
| closure-entries-800 | 170.47 / 174.43 | 30800 / 31064 | 3.53 / 3.56 | 158462 / 158462 |
| closure-entries-3200 | 787.07 / 974.18 | 106484 / 102768 | 5.44 / 5.67 | 633662 / 633662 |
| runtime-calls | 7.57 / 6.98 | 5888 / 6004 | 132.31 / 132.36 | 206 / 206 |
| runtime-memory | 7.70 / 7.09 | 5932 / 6028 | 74.35 / 74.82 | 434 / 434 |
| runtime-floating | 10.04 / 8.91 | 6060 / 6144 | 101.48 / 100.27 | 230 / 230 |
| new-capture-specializations-800 | — / 206.14 | — / 36580 | — / 3.70 | — / 158462 |
| new-capture-specializations-3200 | — / 995.33 | — / 128456 | — / 4.05 | — / 633662 |
| new-runtime-capture | — / 9.04 | — / 6072 | — / 68.58 | — / 196 |
| new-runtime-nested-this | — / 9.22 | — / 6064 | — / 90.16 | — / 282 |

Compiler `.text` grows 2,065,734 to 2,074,182 bytes (+8,448, 0.41%). All six
comparable native files are byte-identical, including the two closure scales.
The sectionless supplied ELF format requires a native payload proxy (includes
static data); compiler size measures actual `.text`. Hashes and sizes accompany
all observations. No native speedup or slowdown is inferred from timing changes
in byte-identical executables.

The final 3200-closure compiler result is noisy and slower in three paired
blocks: B/A **1.190, 0.985, 1.246, 1.100**. Its A/A range is 700.61–1013.83 ms;
B spans 756.42–1093.14 ms. The earlier pack run gives **1.023, 0.811, 0.938,
0.992**, and the initial run **0.985, 0.970, 1.062, 1.043**. All are retained;
this is not evidence for claiming either a compiler speedup or a precise
persistent regression. Required validation ran concurrently with these broad
runs, which limits their timing conclusions. The final 800-closure pairs are
**1.043, 1.003, 0.601, 1.026**; its A/A spans 167.92–205.16 ms. Both compiler
and native spreads, not just medians, remain in the JSON.

## Ownership, work and budgets

- Each closure occurrence (including specialization context) has a canonical
  entity. A TU-owned flat index maps `(closure, original declaration)` to one
  capture field; object zero denotes source `this`. Nested capture demand
  establishes only necessary lexical forwarding edges. Repeated lookup is
  average O(1); initial nested demand tracks required ancestor edges.
- Explicit packs consume the existing canonical parameter-entity sequence,
  including zero lanes. Capture-list checking is linear in explicit captures
  and declared parameters; captureless lambdas return before scratch indexes.
  Bodies are retained and checked once; no source replay, copied environment,
  global retries, semantic string key or process-global cache is introduced.
- Expression/receiver facts record capture IDs. Lowering consumes those IDs and
  recorded field layouts; it never resolves a captured name or searches the
  enclosing declaration graph. Closure construction is O(captured fields),
  copies use the existing special-member facts, and reads use constant-count
  pointer projections/loads. There is no fixed point or repeated layout pass.
- At 800/3200 captured specializations, counters report **1600/6400 closures**,
  **3200/12800 capture edges**, **2401/9601 checked bodies**. Each specialization
  is called twice without rechecking. Compiler memory is bounded by those
  demanded facts and produced IR. The generated payload scales 158462→633662.
- The [source-to-native trace](../student.tests/pa20/trace98.json) independently
  checks two demanded specializations, five closures, ten capture edges and
  nine bodies, including copied closures and a mixed local/this environment.
  Telemetry-on and telemetry-off output is byte-identical; executable mutations
  and identities are checked. All earlier 94–97 traces and controls remain.
- No transform budgets change: O0 has no new optimization, no duplicated
  callable body for a capturing closure, and no added pipeline-wide growth
  policy. Native optimization and self-hosting remain later-stage work.

## Isolated repeat and acceptance

After validation and the broad benchmark handles both completed, the same final
frozen binary repeated the largest affected workloads, with no other task-owned
compiler job live. [All 52 observations and six warmups](../student.tests/pa20/performance98-isolated.json)
are retained. Closure-entries-3200 compiler A/B medians are **1398.22/1320.52 ms**,
RSS **106440/102604 KiB**, paired B/A **0.888, 1.040, 0.965, 1.007**. A/A spans
1223.45–1410.88 ms; B spans 1229.04–1488.54 ms. Runtime A/B is **5.37/6.48 ms**
(startup-dominated), with identical native hashes and **633662-byte** payloads.
The new capture case is **1920.32 ms**, **128280 KiB**, **7.51 ms** runtime,
**633662 bytes**; its compiler range 1508.59–2381.23 ms discloses the remaining
system noise. Entry rejects that case, so no relative profit is claimed.

Across four retained runs there are **916 observations / 102 warmups**. The
isolated paired results do not reproduce a systematic slowdown, while their
higher absolute times demonstrate why separate-run medians are not comparable.
The code has no extra captureless parameter scan, no duplicate semantic body
work, and no optional transform whose cost could be removed. Added capture
fields/edges and their O0 loads/stores are required semantic work; necessary
compiler text growth is disclosed. Native hashes rule out hidden generated-code
regressions on equivalent controls.

**Stage-scoped performance acceptance passes for this behavior group.** There
is no runtime-profit claim for a new optimization, no new unbounded work/growth,
and no unsupported percentage gate. The four remaining PA20 failures and the
independent whole-stage audit remain mandatory; these measurements waive neither.
