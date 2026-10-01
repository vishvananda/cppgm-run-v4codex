# PA29 implementation172 performance evidence

Validated compiler: `827b4c7c`; stage acceptance is **PA29/O0**. Required fold semantics add no optional optimizer transform. No speedup is claimed.

[Manifest](../student.tests/pa29/evidence172/performance-manifest.json) freezes binaries, scripts and environment. Compiler size is **4,126,688 → 4,166,352 bytes** (+39,664, 0.96%). All **224** common A/A+ABBA observations, **96** capability observations and **8** launcher samples are retained. Compilation and execution are timed separately; host linking is excluded. Correctness suites and builds finished before timing. External scheduling and documentation work were uncontrolled; no outliers were discarded.

## Equivalent common workloads

[Raw observations](../student.tests/pa29/evidence172/common-performance.json). Four A/A samples, then six ABBA blocks per workload/mode. Times are median seconds, RSS is maximum KiB; paired ranges disclose spread. All corresponding objects and executables are byte-identical.

| Workload | Compile A/B s | Paired B/A [range] | RSS A/B KiB | Runtime A/B s | Paired B/A [range] | Text bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1604/0.1604 | 1.0000 [0.9920–1.0074] | 29240/29524 | 0.0508/0.0509 | 1.0010 [0.9855–1.0112] | 151633 |
| floating | 0.1604/0.1612 | 1.0000 [0.9379–1.4381] | 29364/29600 | 0.0475/0.0473 | 0.9981 [0.9842–1.0086] | 151474 |
| exceptions | 0.1611/0.1621 | 1.0093 [0.9797–1.0209] | 29404/29472 | 0.2492/0.2507 | 1.0059 [0.9612–1.0493] | 151781 |
| pruning | 0.2037/0.2010 | 0.9827 [0.9192–1.2881] | 35284/35148 | 0.0511/0.0509 | 1.0025 [0.9856–1.0101] | 151633 |

| Workload | A/A compile range s | A/A runtime range s |
|---|---:|---:|
| memory | 0.1581–0.1800 | 0.0508–0.0512 |
| floating | 0.1583–0.1612 | 0.0472–0.0475 |
| exceptions | 0.1624–0.3848 | 0.2469–0.2502 |
| pruning | 0.2027–0.2055 | 0.0508–0.0509 |

Paired compiler medians range from 0.9827 to 1.0093; every paired range crosses 1. Runtime variation occurs despite identical executables. These observations do not establish a repeatable speed change or avoidable regression. [Every sampled existing work counter is unchanged](../student.tests/pa29/evidence172/common-counter-delta.json); the two new fold counters stay zero.

## New capability costs

[Raw observations](../student.tests/pa29/evidence172/affected-performance.json). Entry rejects these sources, so final costs are not equivalent A/B speed comparisons. Runtime input comes from argc and every timed program checks an independently computed checksum. The runtime family compiles N demanded calls through a fold, then executes a three-element fold for 20 million iterations. The query family forms a constant alias fold of N elements, then performs 20 million checked runtime calls using the resulting constant.

| Workload | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| runtime600 | 0.0488 [0.0484–0.0492] | 13792 | 0.3124 [0.3110–0.3143] | 1388 | 39741 |
| runtime1200 | 0.0929 [0.0908–0.1405] | 20428 | 0.3120 [0.3110–0.3130] | 1416 | 79341 |
| runtime2400 | 0.1810 [0.1771–0.1837] | 33316 | 0.3124 [0.3106–0.3244] | 1412 | 158541 |
| query6000 | 0.0440 [0.0435–0.0448] | 13200 | 0.1438 [0.1433–0.1482] | 1416 | 491 |
| query12000 | 0.0807 [0.0797–0.0825] | 19584 | 0.1438 [0.1432–0.1445] | 1416 | 491 |
| query24000 | 0.1543 [0.1532–0.1562] | 31092 | 0.1438 [0.1435–0.1443] | 1416 | 491 |

Launcher median is **0.00285 s**. The shortest affected compile median is **15.4×** launcher time; the shortest affected runtime is **50.4×**. No timing exponent is claimed. [All-sample counter assertions](../student.tests/pa29/evidence172/scaling-counters.json) establish the measured work bounds:

- Runtime fold: typed fold steps **2N+8**, query work **4N+20**, expansion lanes **N+3**; text **66N+141** bytes for required emitted calls.
- Alias fold: value-scheduler steps **N**, query work **N+8**, expansion lanes **N**; executable text remains **491 bytes**.
- A 24,000-element alias initially exhausted recursive query evaluation. The final explicit work stack completes it with 24,000 scheduled steps and preserves short circuit. This is a correctness/complexity repair, not an optional optimizer.

## Ownership and acceptance

Source `Kind::Fold` retains operator, direction and operands once. Canonical query identities carry the definition lookup and immutable substitution frame. Query expansions retain runtime prvalues; template argument formation separately requires constants. Each reduced operator publishes its selected declaration and conversions. Runtime `FoldStep` records consume those facts, including bound member call pairs, temporary ownership and branch cleanup, and lower directly to typed LowIR/MIR/ELF. No synthetic syntax, textual phase transport, global cache or unrelated body demand is added.

The query and evaluated-step owners use contiguous TU storage and flat identity indexes. Required work/storage is O(N) in expanded operands and emitted operations plus language-required overload candidates; source patterns are parsed once. Type reduction, runtime lowering and constant fold execution use explicit stacks or iterative spines. Temporary traversal vectors die at operation/function exit; published semantic records die with the TU. Existing complete query/mode and constexpr activation keys retain success/failure and validity.

Optional optimizer work and code-growth budgets are **zero**. Existing generator ceiling **1,048,576**, constant-call evaluator **1,000,000-step/512-depth** bounds, native frame/data **0x70000000**, alignment **4096**, and course timeouts remain unchanged. Necessary compiler growth is disclosed above; common generated text is unchanged. No mandated limit or coverage rule was weakened.

Historical blanket 15% latency/RSS and zero-growth gates remain diagnostic, as documented in [performance171](performance171.md) under spec §9. Historical measurements are preserved. Heavier hosted runtime, optimization/allocation and self-hosting remain PA30–34 work.
