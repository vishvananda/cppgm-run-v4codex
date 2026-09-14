# PA15 specialization performance evidence

Final implementation: `113b6907`; baseline: `d3475a79`. Both use g++ C++11,
`-Wall -O3` and the course runner; measured compiler flags are
`--emit-lowir -O0`. Native execution uses the supplied backend at `-O0`.
Compiler, backend, harness, source, output and native hashes are retained in the
JSON files and checked by [the verifier](verify_specializations.py).

Evidence: [preliminary](specialization-performance-preliminary.json),
[noisy final-binary campaign](specialization-performance-noise.json),
[final repeat](specialization-performance.json),
[reproducer](specialization_benchmark.py). All **756 invocations**, including
108 warmups and 648 timed observations, are preserved. Each common workload
has four A/A observations followed by two wall-time ABBA blocks; new behavior
has six absolute B observations. Each binary is warmed once per workload.
No build or course suite ran during measurement. The first two campaigns pinned
CPU 0; the repeat pinned CPU 31 after observing variable timings on CPU 0.
The first campaign precedes the final static-declaration correction and uses a
separately frozen binary; the latter two use the exact final compiler.

## Compiler latency, peak RSS and code size

Final-repeat medians. Paired ratios show both ABBA blocks; A/A is the observed
wall-time range, not a confidence interval. Small compilations of the executable
inputs take roughly 6 ms and are retained in JSON, but support no latency claim.

| Common correct input | A ms | B ms | A/B RSS KiB | Paired B/A | A/A ms |
|---|---:|---:|---:|---|---|
| types-1000 | 73.249 | 72.842 | 17304/17202 | 0.9799, 1.0405 | 72.7–75.8 |
| values-1000 | 87.065 | 87.384 | 19042/19142 | 1.0093, 1.0109 | 85.4–91.9 |
| types-4000 | 288.116 | 288.528 | 53344/53176 | 0.9967, 1.0008 | 286.9–292.0 |
| values-4000 | 336.870 | 338.677 | 60812/61160 | 1.0166, 0.9962 | 333.2–482.0 |
| defaults-1000 | 54.953 | 54.452 | 12240/12282 | 1.0009, 1.0147 | 55.2–56.4 |
| defaults-4000 | 212.326 | 213.031 | 33754/33812 | 1.0059, 0.9914 | 208.1–279.8 |
| constants | 227.232 | 233.354 | 34876/34984 | 0.9782, 1.1079 | 224.5–227.6 |

The noisy campaign had 4,000-value medians of 344.471/471.919 ms, with
opposing paired ratios 0.8642 and 1.3309; the same final binary repeats at
336.870/338.677 ms. Its A/A range still includes a 482 ms sample. Constants
have one repeat block at 1.1079, while earlier campaigns have paired ratios
1.00099–1.01318. These observations do not establish a repeatable compiler
speedup or the large regressions suggested by individual samples. No sample
was dropped. The small additional selection/kind checks implement required
semantics; there is no optional transform. Compiler `.text` grows
**1,433,670 → 1,447,110 bytes**, **+13,440 (0.94%)**.

| Newly correct input | B ms | B RSS KiB |
|---|---:|---:|
| specialized-classes-1000 | 88.820 | 16224 |
| variable-queries-1000 | 68.868 | 13934 |
| specialized-classes-4000 | 357.406 | 50586 |
| variable-queries-4000 | 268.974 | 40538 |

For N explicit classes, telemetry records exactly N canonical specializations
and N explicit selections, with zero primary class completions. For N constant
variable instances used three times, it records exactly N initializer transitions
and 2N reuses. Fourfold input growth gives 4.02×/3.91× final-repeat latency.
The baseline rejects these inputs; absolute costs are reported without comparing
correct code to failed compilation. This supports the exercised selection/query
ownership and linear produced work, not every unfinished whole-stage path.

## Generated executables

Common inputs have identical LowIR **and native bytes** in every campaign.
Loops consume volatile runtime bounds and check their arithmetic, call, memory
and floating-point results. The specialized runtime control uses an explicit
function definition; the baseline compiles it but its executable returns failure.
That incorrect baseline supplies no timing comparison.

| Workload | A ms | B ms | Native text A/B bytes |
|---|---:|---:|---:|
| runtime-calls | 301.283 | 305.398 | 206/206 |
| runtime-memory | 177.416 | 176.400 | 434/434 |
| runtime-floating | 209.756 | 210.351 | 230/230 |
| runtime-values | 303.046 | 298.476 | 206/206 |
| runtime-specialization | incorrect | 319.682 | 206 |

Native peak RSS is 256 KiB. Text measures the payload after the ELF entry
because the supplied backend writes sectionless executables; these inputs have
no static data. Runtime variation despite byte identity is preserved. No runtime
optimization benefit is claimed. The student native backend remains PA24 work.

## Stage acceptance

PA15/O0 mandates correctness, canonical identities, bounded TU lifetimes and
linear/near-linear consumed work. It supplies no numerical latency/RSS/text
ceiling. This increment has no optional optimizer or growth policy to justify;
common generated code has zero growth, and the required semantic costs are
disclosed with output parity and work counters. Historical diagnostic targets
are preserved in earlier evidence and do not add an exit gate. Native optimization
and self-hosting keep their later-stage ownership. Unfinished source-obligation,
pack, query and initialization work and independent review remain explicitly
recorded in [the plan](../../pa15/plan.md).
