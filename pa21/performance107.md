# PA21/O0 full-expression implementation evidence (107)

[Raw observations](../student.tests/pa21/performance107.json) and [driver](../student.tests/pa21/benchmark107.py) freeze entry `14225657` and implementation `20476a77`, compiler/backend hashes, sources and `--emit-lowir -O0` flags. One warmup per binary, four A/A observations, four ABBA blocks, pinned CPU, checked exits and separately collected telemetry. Compilation and execution are timed separately; `/usr/bin/time` supplies peak RSS. All observations and paired spreads are retained.

Times below are median milliseconds; RSS is peak KiB. Native size is the supplied sectionless backend’s ELF payload, including static data and EH tables; it is a proxy, not isolated `.text`. Compiler size is actual ELF `.text`.

| Workload | Compile A/B ms | RSS A/B KiB | Runtime A/B ms | Payload A/B bytes |
|---|---:|---:|---:|---:|
| startup | 9.92 / 9.84 | 5708 / 5988 | 5.52 / 5.65 | 24 / 24 |
| auto-specializations-9600 | 1353.24 / 1374.83 | 108060 / 108180 | 6.53 / 6.45 | 768062 / 768062 |
| runtime-calls | 9.36 / 9.23 | 6028 / 6192 | 182.39 / 181.08 | 206 / 206 |
| runtime-memory | 9.84 / 9.91 | 6028 / 6260 | 116.77 / 114.94 | 434 / 434 |
| runtime-floating | 11.63 / 11.56 | 6236 / 6264 | 144.34 / 143.67 | 230 / 230 |
| runtime-reference-captures | 10.38 / 10.47 | 6216 / 6248 | 36.44 / 36.39 | 188 / 188 |
| runtime-class-lists | 15.29 / 15.24 | 6252 / 6248 | 93.58 / 94.95 | 1664 / 1664 |
| composition | 14.38 / 13.30 | 6252 / 6436 | 5.54 / 5.35 | 2760 / 2760 |
| runtime-logical-temporaries | 11.04 / 10.65 | 5996 / 6244 | 271.20 / 356.55 | 508 / 1232 |
| runtime-private-scalar-member | 11.95 / 10.95 | 6024 / 6244 | 261.82 / 438.38 | 356 / 1104 |
| logical-functions-256 | 89.07 / 67.26 | 10684 / 10852 | 5.67 / 5.68 | 68316 / 120040 |
| logical-functions-1024 | 261.03 / 251.12 | 25064 / 24364 | 5.44 / 5.41 | 272604 / 477928 |

## Equivalent ordinary workloads

All eight established workloads (startup through composition) produce **byte-identical executables**. The template compile median changes **+1.60%**; paired B/A spans **0.875–1.049**, with A/A **1252–1509 ms**. This does not establish a repeatable regression or speedup. Its work counter remains **144005**, regions remain zero, and peak RSS changes **+120 KiB**. Favorable short-source ratios are startup-sensitive; the runtime-calls compiler pairing includes a **2.319** outlier, retained in the raw record. Runtime noise between identical executables is not an optimization benefit.

Compiler text is **2,188,550 → 2,190,854 bytes**, **+2,304 (+0.105%)**. The effect-sensitive cleanup query uses the existing two cache bytes per AST node instead of doubling the allocation. Constructor defaults use retained semantic edges; no extra syntax graph, string key, specialization replay or IR roundtrip was added.

## Affected O0 output and necessary costs

The focused runtime controls execute **12 million** volatile-bounded iterations and check the result and destruction counts. Their entry executables have correct behavior for these inputs but lack the required PA21 LowIR region/result shape. Thus these rows measure the cost of contract completion, not an optional optimization’s profitability.

Logical-temporary paired runtime B/A is **1.105–1.282**; private-scalar-member B/A is **1.316–2.179**. The median increases and payload growth are real disclosed costs. Each final function has **two** required cleanup regions versus zero at entry. The supplied O0 backend retains their frame/exception metadata overhead even for nonthrowing calls. PA21 requires these structural regions; frontend removal would restore the original contract failures. Backend elimination/encoding policy belongs to PA24 and later optimization stages. No performance improvement is claimed for these changes.

The [first observations](../student.tests/pa21/performance107-startup-sensitive.json), with their [exact driver](../student.tests/pa21/benchmark107_startup_sensitive.py), are preserved. Their 1.2-million-iteration affected runs were too close to startup for a reliable runtime comparison; the final runs lengthen those inputs tenfold. The compiler binaries and other sources are unchanged. A/A and paired observations show substantial ambient variability, so neither unrelated medians nor the best samples are used as a speedup claim.

At **256 / 1024** logical functions, final query work is **10514 / 42002** (41n + 18), regions **512 / 2048** (2n), instructions **10270 / 40990** (40n + 30), and native payload **120040 / 477928** (466n + 744). Each execution checks the evaluated and skipped path and exact destruction count. This supports proportional growth in consumed expressions and produced IR; single-call runtime here is only a startup control.

## Stage-scoped acceptance

Spec §9 applies to PA21/O0. There is no optional optimizer in this increment and no mandated numerical latency/RSS/text ceiling. The explicit work envelope is at most four lazy cleanup booleans per expression plus the existing unwind fact, linear edge traversal and emitted regions, and existing suffix interning per distinct live-state/context/terminal. Fixed-point search, loop cloning and native code transforms were not added. The eight-element array expansion cap is unchanged. Required structural O0 output accounts for the measured affected-code growth; ordinary executables stay identical.

Historical +15%, +16 MiB and 5.5× diagnostic targets remain measurements rather than extra gates under the stage-scoped rules. All earlier evidence remains. Later native optimization and self-hosting measurements are not claimed at PA21. This evidence accepts the completed behavior group only; **15 required failures** still prevent whole-stage acceptance.
