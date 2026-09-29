# PA21/O0 accumulated checkpoint performance (109)

[Primary observations](../student.tests/pa21/audit109-performance.json) compare
last-reviewed `f65eae8d` with audit repair `bd6bfd71`. The latter compiler is
unchanged at the final review marker. SHA256, sources, flags, CPU affinity,
backend/tool identities, warmups, four A/A samples and four ABBA blocks are
retained. Compilation and execution are timed separately; `/usr/bin/time`
records peak RSS. Results are checked before timing, with volatile bounds and
consumed runtime results. Telemetry runs separately from ordinary timing.

The [initial run](../student.tests/pa21/audit109-performance-initial.json) stops
when the old compiler rejects source `throw` in member-prefix. That rejection
is not a speedup baseline. The exact [initial driver](../student.tests/pa21/benchmark109_initial.py),
[continued driver](../student.tests/pa21/benchmark109_continued.py) and
[diagnostic driver](../student.tests/pa21/benchmark109.py) all match their executed
hashes. The continuation preserves completed samples unchanged and uses the
frozen checkpoint-108 entry compiler for the three starred rows. Historical
106–108 evidence covers those rows' earlier implementation costs.

Times below are median milliseconds; RSS is peak KiB. Native size is the
sectionless supplied backend's **ELF payload proxy**, including static data and
EH tables. Starred rows instead report actual hosted ELF **`.text`** from the
supplied object backend plus host linker. Compiler size is actual `.text`.

| Workload | Compile A/B ms | RSS A/B KiB | Runtime A/B ms | Native size A/B bytes |
|---|---:|---:|---:|---:|
| startup | 6.64 / 6.56 | 5712 / 5932 | 3.61 / 3.58 | 24 / 24 |
| auto-specializations-9600 | 714.54 / 751.69 | 106676 / 106832 | 3.81 / 3.88 | 768062 / 768062 |
| runtime-calls | 8.19 / 7.92 | 6020 / 6164 | 131.24 / 133.51 | 206 / 206 |
| runtime-memory | 7.20 / 6.69 | 6032 / 6248 | 73.67 / 74.58 | 434 / 434 |
| runtime-floating | 6.96 / 6.79 | 6228 / 6356 | 86.27 / 86.75 | 230 / 230 |
| runtime-reference-captures | 6.65 / 6.36 | 6016 / 6212 | 25.97 / 25.66 | 188 / 188 |
| runtime-class-lists | 7.45 / 7.31 | 6016 / 6232 | 62.97 / 63.66 | 1664 / 1664 |
| composition | 8.50 / 8.59 | 6276 / 6444 | 3.42 / 3.44 | 2760 / 2760 |
| runtime-logical-temporaries | 7.09 / 6.76 | 6024 / 6188 | 131.28 / 152.30 | 508 / 1232 |
| runtime-private-scalar-member | 6.77 / 6.48 | 6020 / 6060 | 112.57 / 228.83 | 356 / 1104 |
| logical-functions-256 | 36.32 / 35.00 | 10660 / 10564 | 3.54 / 3.60 | 68316 / 120040 |
| logical-functions-1024 | 119.66 / 113.96 | 24940 / 24260 | 3.61 / 3.73 | 272604 / 477928 |
| runtime-nrvo | 8.77 / 8.28 | 6016 / 6280 | 112.36 / 54.35 | 1024 / 280 |
| runtime-member-prefix * | 9.47 / 9.37 | 6188 / 6160 | 112.47 / 112.16 | 1098 / 1098 |
| runtime-array-defaults | 9.42 / 9.31 | 6028 / 6188 | 146.26 / 97.10 | 1808 / 1712 |
| construction-functions-256 * | 50.60 / 50.88 | 10116 / 10252 | 7.23 / 7.23 | 55528 / 55528 |
| construction-functions-1024 * | 121.30 / 117.61 | 23316 / 23456 | 7.09 / 7.14 | 219880 / 219880 |

## Compiler cost and noise

The first template block is **714.54 → 751.69 ms** (+5.20%), with paired B/A
**1.041–1.177** and A/A **689–712 ms**. A repeat gives **676.06 → 699.46 ms**
(+3.46%), pairing **1.011–1.079**, A/A **674–729 ms**. The independent
[diagnostic continuation](../student.tests/pa21/audit109-performance-diagnostics.json)
then gives **782.44 → 747.28 ms** (−4.49%), pairing **0.897–1.037**. All three
sets remain; none is selected as the preferred answer. Together they do not
establish a repeatable compiler regression or speedup.

Isolating the audit repair against checkpoint entry gives pairing **0.947–1.128**
and A/B medians **776.57 / 808.09 ms**, with ranges reaching **1550 / 1524 ms**.
Separately collected instrumented phase medians are frontend **1082.84 / 1041.29**,
lowering **335.55 / 345.59**, writer **108.88 / 115.65 ms** against the last-reviewed
compiler. Their ambient slowdown is visible and they are not mixed with the
ordinary observations or claimed as a speedup. No counters/profiler dependency
was required.

The long template source has identical semantic work counts, **144005**
full-expression queries, **zero lifetime records/regions**, and **115205** IR
instructions. Peak RSS differs by **156 KiB** in the first blocks, **136 KiB**
in the first repeat. Ordinary semantic specialization work has not multiplied;
new completed-fact lookups for defaults/final consumers and function boundaries
remain bounded. Inspection found no new global retry, broad invalidation or
optional optimizer on this workload. The earlier empty-record regression
removed in 106 stays removed. Short-source compilation is startup-sensitive;
the memory workload's **0.390–4.510** compiler pairing outliers are preserved.

Compiler text grows **2,155,654 → 2,213,958 bytes**, **+58,304 (+2.70%)** across
the accumulated range. The audit repair itself adds **1536 bytes (+0.069%)** to
checkpoint entry. This implements required source EH, destination ownership,
termination and protected exits. No numeric text/latency/RSS ceiling is mandated
by PA21. The measured variation is disclosed rather than used to invent one.

## Generated programs and profitability

The eight established sources from startup through composition produce
**byte-identical executables** across the full range. Loops, calls, memory,
floating point, captures and class lists execute checked live workloads;
startup, template scaling and composition timings are explicitly startup controls.
Runtime differences between identical bytes are noise, not optimization profit.
The three starred rows have **byte-identical `.text` sections** between checkpoint
entry and final; whole hosted ELF hashes differ, so no whole-file identity is
claimed. Section hashes are retained in the review manifest.

- **Named result:** six million calls; paired runtime B/A **0.461–0.509**,
  **112.36 → 54.35 ms**, payload **1024 → 280**. The selected eligible local
  occupies caller storage while keeping failure cleanup until successful return.
  This shows repeatable benefit without extra code growth; 134 ownership controls
  separately check observable/const/polymorphic results and throwing cleanup.
- **Array defaults:** 200,000 constructions of 32 elements; paired runtime B/A
  **0.582–0.706**, **146.26 → 97.10 ms**, payload **1808 → 1712**. Completed-prefix
  ownership replaces nested raw default-temporary regions. Earlier execution is
  correct for these nonthrowing inputs; failure behavior is checked separately.
- **Required full-expression regions:** logical temporaries grow **508 → 1232**
  bytes with runtime pairing **1.154–1.277**; private scalar-member cleanup grows
  **356 → 1104**, pairing **1.777–2.138**. These are real O0 costs, not speedups.
  The old runtime outputs happen to be correct on these nonthrowing inputs, but
  omit required PA21 LowIR regions/result ownership. Restoring that old shape
  restores course failures. Native elimination/frame policy belongs to later
  stages; semantic/contract obligations cannot be removed for timing.
- **Completed member prefixes:** the final incremental check has equal hosted
  text, **1098 bytes**, pairing **0.997–1.006**. Earlier **1008 → 1098** required
  cleanup growth is retained in performance108; its original failing exception
  behavior is proved by the reducer, not timed as a valid baseline.

Helper commutation still needs independent operands, a nonthrowing single-argument
transfer, completed constructor facts and no external effects. Unknown proofs
retain ordered construction. No optional native transform, inlining or growth
search was introduced; smaller IR alone is never used as profitability evidence.

## New jump semantics, bounds and acceptance

The old compiler's invalid jump output cannot be a performance baseline. New
final-only controls contain **256 / 1024** independent functions combining
`break`, caught throws and handler `continue`. They execute both paths and check
three destructions. Instructions are **17945 / 71705 = 70n + 25**, regions
**512 / 2048 = 2n**, full-expression work **4623 / 18447 = 18n + 15**, and host
text **68512 / 272800 = 266n + 416**. Compile medians are **46.76 / 240.19 ms**,
with all four observations retained (1024 ranges **180.66–281.79 ms**); peak RSS
**14032 / 38056 KiB**. The diagnostic rerun also retains its observations; no
favorable sample is substituted for the original curve.

A volatile-bounded 100,000-call loop takes **124.92, 128.45, 129.99, 183.51 ms**,
checks **150,000 destructions**, and uses **724 bytes** of hosted text. Compiler
samples are **7.19–7.53 ms**, peak RSS **6216 KiB**. Hosted text excludes the
shared exception runtime. All source lowering comes from the student compiler.

The pipeline envelope remains linear in semantic edges and emitted IR, except
O(b log b) ordering of completed blocks. Cleanup suffixes are interned by their
complete context/terminal, array expansion is capped at **eight elements**, and
larger arrays use counted loops. Audit jumps add one sparse canonical destination
fact and traverse only exited contexts while emitting required cleanup. TU caches
and function-local state have bounded owners; nonthrowing boundary scratch dies
after one function-slice rewrite. There is no fixed-point or unbounded transform.

Spec §9 stage-scoped acceptance applies at **PA21/O0**. Historical **+15%,
+16 MiB, 5.5×** targets remain diagnostics, with all earlier measurements preserved.
Required semantic costs and later native/self-hosting constraints do not add
exit gates; correctness, coverage, comparisons and mandated work/growth limits
remain binding. This evidence supports the checkpoint audit. **Eight required
PA21 comparisons remain unfinished**, so it does not certify stage completion.
