# PA18 loop 75 performance evidence

Frozen entry `9fa23653` and implementation `6229b49a`, built with
`g++ -std=gnu++11 -Wall -O3` and the test runner enabled. Compiler commands
emit PA18/O0 LowIR; validation, telemetry and native-backend execution are
separate from compiler timing. CPU affinity, binary/input/output hashes, flags,
warmups and every time/RSS observation are retained in the
[first batch](../student.tests/pa18/loop75-performance.json) and
[continuation](../student.tests/pa18/loop75-performance-remaining.json).

Protocol: one warmup per binary, four A/A samples and four ABBA blocks for
equivalent correct inputs; six final-only observations when entry rejects the
new required behavior. No build or validation suite ran concurrently. The first
batch stopped at a preflight assertion because the simple dependent-tag workload
already succeeds at entry. Its LowIR and executable hashes agree, so the
continuation measures it as an equivalent workload. All earlier observations and
the original harness at `/tmp/pa18-loop75/benchmark75-first.py` are preserved;
the continuation skips already measured workloads. This is a classification
correction, not discarded timings or a baseline speedup claim.

## Compiler latency and peak RSS

| Workload | A / B median ms | Paired B/A median (range) | A / B peak RSS KiB |
|---|---:|---:|---:|
| ordering-600 | 107.28 / 107.04 | 1.003 (0.985–1.014) | 15296 / 15216 |
| ordering-2400 | 448.14 / 424.02 | 0.947 (0.875–1.158) | 43144 / 43600 |
| member-head-600 | 172.21 / 166.30 | 1.030 (0.868–1.111) | 22356 / 22840 |
| member-head-2400 | 760.05 / 863.40 | 1.033 (0.948–1.184) | 73568 / 73692 |
| common-loop-float-1500 | 323.77 / 345.59 | 1.012 (1.009–1.247) | 30308 / 30384 |
| runtime-calls | 10.51 / 10.38 | 0.970 (0.868–1.059) | 5952 / 5972 |
| runtime-memory | 12.98 / 13.57 | 0.998 (0.949–1.400) | 5816 / 5888 |
| runtime-floating | 6.50 / 6.35 | 0.976 (0.951–0.987) | 5996 / 6072 |
| list-argument-600 | — / 39.31 | new behavior | — / 11180 |
| dependent-tag-600 | 70.24 / 69.32 | 0.989 (0.969–0.992) | 16300 / 16400 |
| cast-failure-600 | — / 35.57 | new behavior | — / 10836 |
| list-argument-2400 | — / 146.05 | new behavior | — / 26640 |
| dependent-tag-2400 | 272.43 / 273.60 | 1.004 (0.980–1.023) | 47520 / 48404 |
| cast-failure-2400 | — / 130.62 | new behavior | — / 25980 |
| runtime-list-query | — / 6.04 | new behavior | — / 5976 |

Empty-input startup medians are **9.76 / 10.93 ms** in the first batch and **6.07 / 5.85 ms**
in the continuation. Small runtime-source compiler measurements are startup-limited;
the scaled frontend cases dominate startup. Host scheduling spread is substantial:
the first batch includes paired ratios from 0.868 to 1.400. Raw spreads and A/A
calibrations remain available; aggregate medians and paired ratios can differ.
No general compiler speedup is claimed. Equivalent workloads retain byte-identical
LowIR and executables. Maximum measured equivalent RSS growth is **884 KiB**
on dependent-tag-2400; the graph retains the required initializer plans.

Compiler `.text`: **1,897,286 → 1,924,614 bytes**, **+27,328 (1.44%)**.
This implements required semantic facts and consumers, not an optional transform.
No equivalent generated executable grows.

## Checked executable runtime and size

The supplied backend is pinned to bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, invoked separately with `-O0`.
Volatile inputs, loops and checked results prevent folded/dead timing. Every
execution exits zero. Its sectionless ELF payload is a code-size proxy for these
sources without static data, not a claimed `.text` section.

| Workload | A / B median seconds | Paired B/A median (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 1.230452 / 1.213443 | 1.007 (0.984–1.039) | 206 / 206 |
| runtime-memory | 0.670267 / 0.645619 | 0.984 (0.899–0.996) | 434 / 434 |
| runtime-floating | 0.496953 / 0.497408 | 0.999 (0.989–1.003) | 230 / 230 |
| runtime-list-query | — / 0.114408 | new behavior | — / 175 |

Equivalent executables are byte-identical; their observed variation is environmental.
No runtime optimization gain is claimed. The list-query runtime has no correct
entry executable for the same source.

## Graph work and stage acceptance

| Family, 600 → 2400 | Type-query work | List plans | Query edges |
|---|---:|---:|---:|
| list-argument | 3610 → 14410 | 1200 → 4800 | 4210 → 16810 |
| dependent-tag | 606 → 2406 | 1200 → 4800 | 4 → 4 |
| cast-failure | 1206 → 4806 | 0 → 0 | 605 → 2405 |

Fourfold input growth yields time ratios **3.72× / 3.95× / 3.67×** respectively.
Query/initializer work follows demanded types and edges. At 32/128/512 incomplete
classes, completing one class invalidates **three** consumers at every scale;
completion edges are **96/384/1536**. Arrays of **32/4096/1,048,576** elements
retain **four list plans and three fields** each. Thus omitted-tail work does not
grow with the bound. These checks include failed-query completion, not just
successful construction. Inherited completion tests also retain their one-consumer
invalidation bound.

Acceptance is **PA18/O0 LowIR**, spec §9. No mandated numeric compiler/runtime
ceiling or optional transform is introduced. Correctness, coverage and graph
bounds remain gates; all are preserved for this handoff. Historical **+15%,
+16 MiB, 5.5×** targets are unsupported self-imposed diagnostics, not extra
acceptance gates. Their earlier observations remain intact. Required semantic
cost and measured compiler text growth are disclosed; this does not license
avoidable regressions. The common-workload paired results and identical executable
bytes provide no evidence of such a regression. Native optimization and
self-hosting remain PA24–PA34 responsibilities. Current course failures remain
required implementation work, not excused by performance acceptance.
