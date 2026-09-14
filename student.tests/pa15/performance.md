# PA15 scalar-value performance evidence

Frozen implementation: `01b543ed`; baseline: stage entry `8000f3c8`.
[Final observations](performance.json), [preliminary observations](performance-preliminary.json),
[reproducer](benchmark.py), [verification](verify_evidence.py).
Both binaries use g++ C++11, `-Wall -O3`, and the course test runner. Tested
compiler flags are `--emit-lowir -O0`; the supplied native backend uses `-O0`.
All binaries, inputs, outputs, backend and harness versions have SHA-256 records.
Raw artifacts live in `$RALPH_ARTIFACT_DIR/pa15-value/`.

Each campaign freezes outputs before timing, pins one available CPU, warms each
binary, calibrates four A/A observations, and runs two ABBA blocks. New behavior
has six absolute B observations because A rejects it. No build or test suite
ran during timing. Together the campaigns retain **336 invocations**, including
48 warmups (288 timed observations). The initial executable loops lasted only
21–33 ms; the final campaign increases their runtime input tenfold. All initial
observations, including a 478 ms value-workload outlier, remain recorded. The
initial harness is retained as `benchmark-preliminary.py` in the artifact directory.

## Compiler latency, peak RSS and growth

Final medians; compiler text is reported separately below. Small executable-input
compilations take about 6 ms and are dominated by startup; no compiler performance
claim uses those timings.

| Common correct workload | A wall ms | B wall ms | A RSS KiB | B RSS KiB | Paired B/A |
|---|---:|---:|---:|---:|---|
| 1,000 type instances | 71.002 | 71.242 | 17,470 | 17,280 | 1.0111, 1.0086 |
| 4,000 type instances | 278.211 | 278.282 | 53,004 | 52,920 | 1.0077, 1.0066 |
| 12,000 constant assertions | 224.944 | 225.729 | 34,898 | 34,876 | 1.0021, 0.9999 |

The initial 4,000-type paired costs were 1.62% and 2.26%; the repeat is 0.66–0.77%.
The repeat's A/A range is 276–317 ms. These observations do not establish a
compiler speedup. Small additional argument-kind/normalization work is required
for the new semantics; there is no optional transformation to justify or remove.
Compiler `.text` grows **1,424,966 → 1,433,670 bytes**, +8,704 (0.61%).

| Newly correct workload | B wall ms | B RSS KiB |
|---|---:|---:|
| 1,000 value instances, each requested twice through equivalent expressions | 84.292 | 18,472 |
| 4,000 value instances, each requested twice | 330.892 | 58,626 |
| 1,000 dependent default instances | 54.214 | 12,164 |
| 4,000 dependent default instances | 208.089 | 33,636 |

For N value instances: N specializations and N body transitions, 3N+5 type-query
computations, 3N+1 value-query computations, and 11 source binding visits. For N
default instances: N class completions, zero member-body transitions, 2N+4 type
queries and 2N+2 value queries. Fourfold input increases give 3.93×/3.84× median
latency. These counters support the intended source/key ownership on these
workloads; they do not certify every whole-stage demand path.

## Executable runtime and text

Common inputs produce identical LowIR **and identical native bytes** under A/B.
Runtime inputs are volatile and checked results depend on loops, calls, memory
updates and floating arithmetic. These execute student-produced LowIR through
the supplied backend; a student native backend remains PA24 work.

| Workload | A runtime ms | B runtime ms | Native text A/B bytes |
|---|---:|---:|---:|
| Calls, 60 million iterations | 299.824 | 301.022 | 206 / 206 |
| Memory, 40 million updates | 176.931 | 175.778 | 434 / 434 |
| Floating point, 20 million iterations | 208.725 | 209.220 | 230 / 230 |
| New explicit-value template calls, 60 million iterations | unavailable | 317.569 | unavailable / 206 |

Native peak RSS is 256 KiB in these observations. Text measures the executable
payload after the ELF entry, because the supplied backend writes sectionless
ELF; the executable workloads have no static data. Common runtime paired ratios
range from 0.9955 to 1.0057. Timing fluctuations are retained despite byte identity;
no runtime improvement is claimed.

## Stage acceptance

PA15/O0 mandates correctness, canonical identities, bounded lifetimes and
linear/near-linear consumed work. It supplies no numerical latency, RSS or text
ceiling. This increment adds required semantics and no optimizer. The measured
compiler text cost is disclosed, common generated code has no growth, and new
behavior has absolute compiler/native evidence without comparison to an incorrect
baseline. Historical PA14 diagnostic targets do not create new gates. Native
optimization and self-hosting retain their owning stage boundaries; inherited
benchmark sources and evidence are preserved. Whole-stage correctness and audit
remain unfinished as recorded in [the plan](../../pa15/plan.md).
