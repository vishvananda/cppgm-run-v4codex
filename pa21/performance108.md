# PA21/O0 destination lifetime evidence (108)

[Raw observations](../student.tests/pa21/performance108.json) and the
[driver](../student.tests/pa21/benchmark108.py) pin entry `a2b9e823` and final
implementation `a30acab5`, compiler/backend hashes, complete source strings,
`--emit-lowir -O0`, host/tool versions and CPU affinity. Each workload uses one
warmup per binary, four A/A observations and four ABBA blocks. Compilation and
execution are timed separately, with `/usr/bin/time` peak RSS and checked exits.
All observations, paired ratios and spreads remain in the raw record. Telemetry
runs separately from timing. No compiler change occurred during measurement.

Times are median milliseconds; RSS is peak KiB. Size is the sectionless supplied
backend's ELF payload proxy, including data/EH tables, except **member-prefix**
and **construction-functions**, which use actual ELF `.text` after supplied
object-backend compilation and host linking. Compiler size is actual `.text`.

| Workload | Compile A/B ms | RSS A/B KiB | Runtime A/B ms | Native size A/B bytes |
|---|---:|---:|---:|---:|
| startup | 10.69 / 10.75 | 5724 / 5928 | 7.48 / 7.70 | 24 / 24 |
| auto-specializations-9600 | 1506.33 / 1583.47 | 108060 / 108216 | 6.36 / 6.06 | 768062 / 768062 |
| runtime-calls | 10.30 / 9.74 | 5996 / 6196 | 210.43 / 215.39 | 206 / 206 |
| runtime-memory | 13.91 / 13.47 | 6012 / 6068 | 123.98 / 121.81 | 434 / 434 |
| runtime-floating | 11.19 / 10.67 | 6236 / 6316 | 143.38 / 139.31 | 230 / 230 |
| runtime-reference-captures | 10.81 / 10.66 | 6016 / 6188 | 37.72 / 38.03 | 188 / 188 |
| runtime-class-lists | 12.19 / 11.90 | 5996 / 6192 | 99.90 / 104.85 | 1664 / 1664 |
| composition | 16.90 / 16.86 | 6488 / 6444 | 6.18 / 6.26 | 2760 / 2760 |
| runtime-logical-temporaries | 11.73 / 12.32 | 5980 / 6156 | 253.93 / 246.07 | 1232 / 1232 |
| runtime-private-scalar-member | 11.20 / 11.12 | 6024 / 6184 | 338.42 / 328.19 | 1104 / 1104 |
| logical-functions-256 | 57.34 / 57.96 | 10624 / 10796 | 5.13 / 5.34 | 120040 / 120040 |
| logical-functions-1024 | 202.57 / 189.23 | 24256 / 24404 | 9.18 / 9.09 | 477928 / 477928 |
| runtime-nrvo | 11.00 / 11.23 | 5980 / 6188 | 199.26 / 97.01 | 1024 / 280 |
| runtime-member-prefix | 11.37 / 11.72 | 5996 / 6168 | 116.74 / 116.40 | 1008 / 1098 |
| runtime-array-defaults | 10.41 / 10.72 | 5996 / 6248 | 218.32 / 147.09 | 1808 / 1712 |
| construction-functions-256 | 60.46 / 63.90 | 10152 / 10536 | 8.47 / 8.61 | 33512 / 55528 |
| construction-functions-1024 | 208.65 / 228.40 | 23400 / 23544 | 10.01 / 9.55 | 131816 / 219880 |

## Ordinary code and compiler cost

The twelve established workloads through `logical-functions-1024` have
byte-identical native executables across A/B. The 9,600-specialization compile
median changes +5.12%, but paired B/A spans **0.955–1.079**, A/A is
**1540–1786 ms**, and observed A/B ranges overlap. This does not establish a
repeatable regression or improvement. Full-expression query work stays
**144005**, regions **0**, instructions **115205**, with peak RSS +156 KiB.
Short-source compiler times include startup and are not speedup evidence.

Compiler `.text` is **2,190,854 → 2,212,422 bytes**, +21,568 (+0.985%). The
added code owns actual required termination, result transfer, partial aggregate
cleanup and constructor-default lifetime behavior. It introduces no optimizer,
extra parser, rendered identity key, reference delegation or IR text roundtrip.

## Affected generated programs

All runtime loops use volatile bounds, consume their results and check final
object counts. A/B executions are equivalent and correct for the measured
nonthrowing inputs; exception behavior is checked separately by 134 personal
controls and the reference reducer.

- **Named result:** six million calls. Runtime paired B/A **0.476–0.501**,
  median **199.26 → 97.01 ms**, payload **1024 → 280**. Preserving the named
  result in caller storage removes redundant copies/destructions while retaining
  failure cleanup. This is a repeatable measured benefit of the required result
  ownership change; compile pairing **0.907–1.102** is startup-sensitive.
- **Array defaults:** 200,000 constructions of 32 elements. Paired runtime
  B/A **0.624–0.710**, median **218.32 → 147.09 ms**, payload **1808 → 1712**.
  The shared typed prefix replaces nested raw array/default temporary regions;
  counters become complete before temporary cleanup. Compile paired B/A is
  **0.942–1.045**. The workload uses ordinary default construction, whose entry
  execution is correct; it does not time the broken omitted-aggregate path.
- **Member prefix:** two million constructions. Runtime paired B/A
  **0.993–1.059** does not show a speedup. `.text` **1008 → 1098** is the
  required cost of cleaning completed members if a later copy fails. Median
  runtime remains **116.74 / 116.40 ms**. The entry failure case is deliberately
  excluded from timing and documented by the independent reference proof.

The [initial observations](../student.tests/pa21/performance108-initial.json)
and [exact initial driver](../student.tests/pa21/benchmark108_initial.py) retain
the interrupted attempt: the supplied freestanding backend rejected duplicate
RTTI object labels in member-prefix before execution. The continuation reuses
completed observations unchanged and uses the supplied object backend plus
host runtime for the three affected rows. No reference frontend compiles the
workload sources. This backend limitation is not a compiler performance claim.

## Work/growth budgets and stage acceptance

The explicit group envelope is linear work/storage in consumed semantic edges
and emitted IR: one exception fact per complete constructor/initializer key,
one independence fact per constructor/proof mode, one saved address per live
completed subobject that can require unwind cleanup, shared immutable suffixes,
and one function-sized scratch instruction slice for termination insertion.
Raw constructor-region changes allocate distinct resume identities; caches
already include that identity. Temporary suffix rebasing leaves published
constructor-failure states unchanged. The existing **eight-element expansion
cap** is preserved; larger arrays use counted loops, not bound-sized output.

At **256 / 1024** construction functions, final exception work is
**1536 / 6144 = 6n**, full-expression work **3862 / 15382 = 15n + 22**, and
instructions **10843 / 43099 = 42n + 91**. Native `.text` is
**55528 / 219880 = 214n + 744**, versus entry **128n + 744**; the additional
**86n** bytes implement previously missing unwind prefixes. These are required
semantic costs, not an optional optimization justified solely by IR bounds.
Final compiler medians scale **63.90 → 228.40 ms** (3.57× for 4× input); RSS
**10536 → 23544 KiB**. Pairing has retained outliers (256 functions up to
**1.837**, 1024 up to **1.290**); no favorable-sample compiler claim is made.
Single-call execution of these curves is a startup control only.

Spec §9 acceptance is scoped to PA21/O0. There is no mandated numeric latency,
RSS or text ceiling for this stage, and this increment adds no optional native
transform. Required semantic bounds, correctness and all course cases remain
binding. Historical +15%, +16 MiB and 5.5× diagnostics are preserved as evidence,
not invented exit gates. Native optimization and self-hosting remain later-stage
work. This accepts the completed behavior group's evidence only; **eight required
PA21 failures** still prevent whole-stage acceptance.
