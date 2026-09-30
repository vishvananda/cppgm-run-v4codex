# PA27 implementation146 performance evidence

The frozen entry binary (HEAD `5aefb962`, SHA256 `18b276c0…`) and implementation
`bb608e0b` (SHA256 `37d90792…`) use identical `-O0 -c --stats` flags and inputs.
[Common observations](evidence146/common-performance.json) retain 224 measurements
from [performance145.py](performance145.py); its three inherited input hashes
are checked against PA26's manifest. [Naming observations](evidence146/naming-performance.json)
retain 56 measurements from [performance146.py](performance146.py).
Each compiler/runtime row has four A/A samples and six wall-time ABBA blocks.
All observations, binary/input/image hashes, phase/work counters and RSS remain.

Host linking is separate from compiler timing. Every executable is checked before
measurement and every timed run checks its result. `argc` supplies runtime input;
loops, calls, memory, floating point and EH remain live. Self-hosting is PA34's
boundary. The naming input is deliberately self-contained: both binaries compute
the same result; the old symbol spellings are not a correct host interoperability
alternative. This comparison measures the necessary naming cost, not the
performance of a proposed optimization.

| Workload | Compiler median ms A/B | Peak compiler KiB A/B | Runtime median ms A/B | Executable text bytes A/B |
|---|---:|---:|---:|---:|
| 2,400 templates + memory/calls/loops | 333.55 / 328.99 | 28984 / 29124 | 92.33 / 94.89 | 151633 / 151633 |
| 2,400 templates + floating point | 350.57 / 348.08 | 29024 / 29280 | 82.26 / 81.36 | 151474 / 151474 |
| 2,400 templates + exception lifetimes | 160.95 / 156.01 | 29016 / 29196 | 250.40 / 252.32 | 151781 / 151781 |
| Memory input + 1,200 unused local functions | 195.91 / 195.77 | 34816 / 35036 | 51.06 / 51.29 | 151633 / 151633 |
| 1,200 allocator/ostream template signatures | 125.74 / 123.58 | 28604 / 28800 | 45.21 / 45.45 | 112922 / 112922 |

Paired compiler B/A medians (range), in table order: **0.997 (0.922–1.027)**,
**0.974 (0.823–1.049)**, **0.964 (0.901–1.881)**,
**1.007 (0.875–1.074)**, **1.011 (0.965–1.210)**.
Runtime medians (range): **1.005 (0.879–1.146)**,
**0.980 (0.895–0.999)**, **0.995 (0.921–1.025)**,
**1.004 (0.995–1.022)**, **1.008 (0.994–1.020)**.
The spread and A/A samples expose substantial environmental noise, including the
EH compiler outlier; no latency or runtime speedup is claimed. All executable
text sizes are unchanged. Required standard substitutions reduce the naming
object from **654,192 to 543,792 bytes** and its executable from **306,904 to
251,704 bytes**, entirely outside executable text. Common object sizes match.

## Stage-scoped acceptance and budgets

No optional optimizer, search, unrolling or inlining policy was added. The explicit
budgets are structural and proportional to consumed semantic facts/output:

- ABI type/query caches hold one result per canonical type/query. Dependency
  checks reuse the semantic type-dependence cache; prescribed standard-type
  tests inspect a fixed number of canonical components. Mangling work follows
  emitted name bytes. Graph nodes/edges are interned in contiguous TU pools.
- Linkage tests cache each scope/type/entity result; extern-template suppression
  follows lexical class ancestry only. It never retries unrelated bodies.
- Each TU's ABI support cache has at most one `(special-name kind, ABI type)`
  record per requested support object. Lookup is average O(1); the TU releases
  it with lowering. External sharing retains the program-owned linkage index.
- Each hosted TLS entity has at most one wrapper and one optional initializer
  declaration/definition. A dynamic definition owns one guard; a constant
  definition adds no guard/init body. Imported access has a fixed weak-hook
  check, call and address computation. User initializer work is unchanged.
- Closure numbering is one source-order sort, O(C log C), plus constant work per
  closure. The existing signature ordinal remains separate from the local one.

TLS metadata/code and strong undefined references are required correctness
costs, not optional performance transforms. [TLS controls](evidence146/tls-controls.json)
check constant/dynamic initialization, both object orders, O0/O2 and separate
threads. [Linkage controls](evidence146/linkage-controls.json) check static-archive
extraction, ABI near misses and internal VTT identity across TUs.

Under spec §9, inherited blanket percentage/zero-growth diagnostics remain
self-selected diagnostics; no new exit gate or weakened mandated limit is
introduced. Historical145 measurements are preserved. Current observations
show no repeatable avoidable regression and retain all correctness coverage.
Independent whole-stage performance/architecture review remains pending.

Frozen binaries, source inputs, objects and raw command outputs are retained in
`$RALPH_ARTIFACT_DIR/pa27-146/`; generated files and raw logs are not committed.
