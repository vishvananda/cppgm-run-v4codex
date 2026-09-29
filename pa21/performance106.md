# PA21/O0 source exception implementation evidence (106)

[Raw measurements](../student.tests/pa21/performance106.json) and
[driver](../student.tests/pa21/benchmark106.py) freeze sources, flags, compiler and
both backend hashes. All eight final comparisons use entry `06b2d989` against
`c64e88fb`: one warmup each, four A/A samples and four ABBA blocks, pinned CPU,
separate compiler/executable timing, `/usr/bin/time` peak RSS, checked outputs.
All observations, paired ratios, spreads and separate telemetry are retained.
Compiler build remains the course g++ configuration; measured flags are
`--emit-lowir -O0`. The supplied backend also runs at O0.

## Equivalent unchanged workloads

Times are median milliseconds; RSS is peak KiB. The native backend produces
sectionless ELF, so payload includes static data and EH tables and is explicitly
a proxy rather than isolated text. Every A/B executable is **byte-identical**.

| Workload | Compile A/B ms | RSS A/B KiB | Runtime A/B ms | Payload A/B bytes | Paired compile B/A range |
|---|---:|---:|---:|---:|---:|---:|
| startup | 6.27 / 6.11 | 5788 / 5964 | 3.33 / 3.52 | 24 / 24 | 0.958–1.012 |
| auto-specializations-9600 | 687.01 / 688.09 | 107292 / 107332 | 3.89 / 3.74 | 768062 / 768062 | 0.942–1.076 |
| runtime-calls | 7.62 / 6.94 | 6012 / 6180 | 137.63 / 142.78 | 206 / 206 | 0.773–1.129 |
| runtime-memory | 7.76 / 7.26 | 6024 / 6252 | 82.81 / 81.85 | 434 / 434 | 0.828–1.000 |
| runtime-floating | 8.96 / 7.53 | 6280 / 6320 | 92.92 / 89.89 | 230 / 230 | 0.771–0.905 |
| runtime-reference-captures | 7.77 / 7.61 | 5996 / 6276 | 28.69 / 27.66 | 188 / 188 | 0.729–1.159 |
| runtime-class-lists | 9.12 / 9.37 | 6028 / 6280 | 76.10 / 76.14 | 1664 / 1664 | 0.864–1.096 |
| composition | 10.22 / 10.30 | 6272 / 6544 | 4.48 / 4.21 | 2760 / 2760 | 0.837–1.235 |

The 9600-specialization compile median changes **+0.16%**;
A/A spans **674.77–903.17 ms** and paired results straddle parity.
Small-source compilation is near process-startup cost; its favorable ratios
are not an optimization claim. Runtime differences are noise between identical
executable bytes; every result is checked. The sources cover templates,
calls/loops, memory, floating point, captures, lists and their composition.
No optional native transform or speedup is claimed.

Compiler ELF text is **2,155,654 → 2,188,550 bytes**, +32,896
(+1.53%). Against stage base 2,090,694 bytes, cumulative growth is
97,856 (+4.68%). This pays for required syntax, selected initialization/binding
facts, exception-region/runtime lowering and cleanup continuations. The stage-base
hash is retained for provenance; A/B rows use the turn entry, not the stage base.

## Avoidable cost removed

The [initial run](../student.tests/pa21/performance106-before-catch-default.json)
and [post-catch-repair run](../student.tests/pa21/performance106-before-empty-lifetimes.json)
are preserved, with their prefix and validation snapshots. The latter exposed
**9602 unnecessary empty lifetime records** and a **3.72%** template compile
median increase. Lexical return context alone does not own cleanup; active
source EH contexts already have their own lowering owner. Removing those empty
records restores the entry's zero count on this workload, with identical LowIR.
The final pairs above establish the remaining whole-increment cost; no isolated
speedup is inferred by comparing timing samples from different runs.

## New semantics and work bounds

The entry compiler cannot correctly compile these source exceptions. Their
final-only costs are separate; rejection or incorrect cleanup is never a
performance baseline. At 256/1024 independent try/catch functions, emitted work
is **9493 / 37909 instructions** (37n + 21), with one RTTI record and
**511 / 2047** RTTI cache hits. Compile medians are **32.84 / 132.30 ms**,
peak RSS **10,456 / 23,448 KiB**, and host text **46,430 / 184,670 bytes**.
The four compiler observations per size, input bytes, IR hash, telemetry and
checked execution are retained in the raw record.

The [live-prefix experiment](../student.tests/pa21/performance106-prefix.json)
and [driver](../student.tests/pa21/benchmark106_prefix.py) exercise actual reverse
cleanup with 8/32/128 live nontrivial objects and a thrown exception. Every
execution has the expected destruction count. Instructions are **157 / 421 /
1477** (11n + 69), and full-expression regions **9 / 33 / 129**. Compiler medians
**7.89 / 9.35 / 10.72 ms** are short-input observations, not reliable wall-time
scaling claims. Peak RSS is **6116 / 6240 / 6828 KiB**; host text is
**892 / 2020 / 6724 bytes**. Single-call runtime samples are retained but dominated
by startup.

A separate live loop makes 200,000 calls, throws on 100,000, checks its accumulated
result and all 200,000 destructions. Runtime samples are **221.42, 200.91, 151.46, 203.11 ms**;
compiler samples **7.95, 8.49, 8.36, 6.80 ms**, peak RSS **6204 KiB**,
host text **670 bytes**. Host text excludes shared C++ runtime code.
Volatile iteration limits prevent a dead timing workload. Host executables come
from student-generated LowIR through the pinned object backend; no host compiler
implements the source-to-LowIR path.

## Stage-scoped acceptance

Spec §9 applies to PA21/O0. There is no optional optimization requiring a
profitability budget. Suffix storage/emission is bounded by distinct live-state,
context/terminal edges; iterative suffix construction avoids recursion over the
live-object prefix. Function-owned continuation queues are reset between bodies.
Sorting newly completed blocks costs O(b log b) in emitted blocks; clauses track
actually emitted enclosing handlers. The existing eight-element array expansion
cap is preserved. Work counters and the prefix curve support these bounds;
unchanged ordinary workloads retain their executable bytes.

There is no handout numerical latency/RSS/text ceiling. Historical +15%, +16 MiB
and 5.5× self-selected diagnostics add no gate; all performance102–105 measurements
remain. Necessary O0 exception cost is measured, and the detected avoidable
record growth is removed. Native optimization/runtime representation and
self-hosting remain later-stage work. This evidence covers the completed
increment; 24 required PA21 failures still prevent whole-stage acceptance.
