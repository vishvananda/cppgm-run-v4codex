# PA21 implementation 111 performance evidence

The completed group records a bounded body-effect proof and uses it when choosing
an O0 full-expression unwind continuation. It preserves source exception
specifications and real cleanup ownership. Entry A is `0e5a32dd`; B is
`3028366d`. This is required O0 comparison work; no native optimization or
runtime speedup is claimed. Two unrelated required comparisons remain open.

The [driver](../student.tests/pa21/benchmark111.py) freezes compiler, source,
backend, flags and host identities, pins one CPU, warms each binary, retains
four A/A observations and four ABBA blocks, and measures compilation separately
from execution. Every executable checks its result on every invocation.
Volatile loop bounds preserve calls, memory, floating point and lifetime work.
All samples, including outliers, remain in the
[raw record](../student.tests/pa21/performance111.json).

| Binary | SHA256 | Compiler .text bytes |
|---|---|---:|
| A | `ac7e58d3cf0c63016f0abf773b5f7cbab104459088aae1b18d6351b1892dc418` | 2222022 |
| B | `3e1153ba6b7c12d890ea037e86470e93ea844cbe1b1f5de27a1464a5331965c0` | 2223942 |

Compiler text grows by **1920 bytes (0.086%)**. The table reports medians in
milliseconds, peak RSS in KiB, and the range of four paired B/A observations.
**T** is actual hosted ELF `.text`, excluding shared runtime; **P** is the
sectionless freestanding backend's executable payload proxy, which includes
non-text payload. Startup and function-count executable timings are dominated
by process startup and do not establish runtime profit.

| Workload | Compile ms A → B | RSS KiB A → B | Compile B/A range | Runtime ms A → B | Runtime B/A range | Native bytes A → B |
|---|---:|---:|---:|---:|---:|---:|
| startup | 8.49 → 8.43 | 5764 → 6076 | 0.953–1.000 | 4.30 → 4.18 | 0.830–1.049 | 24 → 24 P |
| auto-specializations-9600 | 1423.90 → 1393.73 | 108700 → 108296 | 0.852–1.179 | 6.51 → 6.33 | 0.944–1.040 | 768062 → 768062 P |
| runtime-calls | 9.81 → 8.71 | 6220 → 6192 | 0.789–1.008 | 216.80 → 219.05 | 0.979–1.027 | 206 → 206 P |
| runtime-memory | 10.04 → 9.88 | 5976 → 6228 | 0.948–1.013 | 134.15 → 128.51 | 0.921–1.006 | 434 → 434 P |
| runtime-floating | 11.39 → 11.10 | 6276 → 6288 | 0.962–1.008 | 148.02 → 148.99 | 0.962–1.122 | 230 → 230 P |
| runtime-leaf-continuation | 13.73 → 13.31 | 6028 → 6224 | 0.945–0.997 | 138.99 → 136.14 | 0.970–1.078 | 772 → 736 T |
| leaf-functions-1024 | 264.69 → 241.03 | 23524 → 21320 | 0.760–0.965 | 10.32 → 10.28 | 0.866–1.101 | 213548 → 176684 T |
| leaf-functions-4096 | 948.22 → 923.72 | 75756 → 66532 | 0.898–1.051 | 4.95 → 5.00 | 0.988–1.024 | 852524 → 705068 T |

The affected four-million-call lifetime workload has paired runtime **0.970–1.078**
and A/A **137.99–190.17 ms**. This does not prove a speedup or a repeatable
regression. Its required O0 continuation reduces text **772 → 736 bytes**.
At 1024 functions, all paired compiler observations favor B; at 4096, the
range crosses one. Therefore no general compiler latency improvement is claimed.
The template workload's **0.852–1.179** pairing and unchanged executable payload
also do not establish a regression or speedup. The calls/memory/floating
controls preserve their generated outputs; their timing spread is disclosed.

The clear effect is bounded storage/output reduction on affected repeated
functions. At 1024/4096 functions, peak compiler RSS decreases **23524 → 21320 /
75756 → 66532 KiB**, and native text decreases **36864 / 147456 bytes**, exactly
**36 bytes per function**. Instructions decrease **34865 → 30769 / 139313 →
122929**, exactly **four per function**. Both versions retain **2048 / 8192**
full-expression regions and **1028 / 4100** checked bodies. Semantic exception
work adds **18** memoized operations at both sizes. Lowering expression work
increases **18456 → 25624 / 73752 → 102424**: the second proof view adds seven
node visits per generated function. The source graphs remain **46334 / 184574**
nodes. These counters establish linear work; they are not runtime profit proxies.

The proof budget is one local classification per completed body, reusing the
existing memoized semantic exception facts, plus at most two unwind-result
visits per source node (declared-spec and body-proof views). The additional
lowering cache is one byte per TU node, allocated only when queried. Positive
body facts use the existing flat index keyed by EntityId and are immutable after
body completion. No body is demanded by the proof, no call-graph fixed point is
introduced, and no active source-handler continuation is rewritten. Unknown
allocation, destruction, aggregate/list initialization and indirect/virtual
call behavior conservatively retains the existing cleanup path. All caches are
released with their TU; full-expression flags reset at its boundary.

Spec §9 acceptance applies at PA21/O0. This proof is necessary to produce the
required empty unwind suffix without omitting cleanup for genuinely throwing
calls. The measured compiler work and text cost are disclosed; emitted code
and memory decrease on the affected scaling workloads. No optional native
optimization is added. Historical **+15%, +16 MiB and 5.5×** diagnostics remain
preserved in earlier reports and do not become additional exit gates. Existing
mandated correctness, comparison, coverage and bounded-work requirements remain
binding. [Performance 110](performance110.md) and all earlier observations are
unchanged; native selection, optimizer and self-hosting work remain later-stage
owners.
