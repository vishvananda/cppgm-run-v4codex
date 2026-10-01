# PA29 implementation171 performance evidence

Validated code: `846ef3fb`; acceptance is PA29/O0. These are required template semantics, not optional optimizer transforms. No speedup is claimed.

[Manifest](../student.tests/pa29/evidence171/performance-manifest.json) freezes binaries, flags, scripts and environment. Compiler bytes: **4,109,848 → 4,126,688** (+16,840, 0.41%). All observations are preserved: **224** common A/A+ABBA, **144** initial capability and **96** larger generator samples, plus twelve launcher observations.

Host linking is outside measured compilation. All executions check independently computed results using runtime argc; the selection benchmark retains demanded calls and the sequence benchmarks perform 20 million runtime-indexed table accesses. Builds and correctness suites did not overlap timing. Documentation/evidence work and external scheduling were uncontrolled; no outliers were removed.

## Equivalent common workloads

[Raw observations](../student.tests/pa29/evidence171/common-performance.json). Four A/A calibration samples and six ABBA blocks per workload/mode. Times are median seconds; RSS is maximum KiB; paired ranges disclose spread. Every A/B object and executable is byte-identical.

| Workload | Compile A/B s | Compile B/A [range] | RSS A/B KiB | Runtime A/B s | Runtime B/A [range] | Text bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.3415/0.3428 | 0.9831 [0.9033–1.1257] | 28964/29592 | 0.0842/0.0842 | 0.9921 [0.9696–1.0451] | 151633 |
| floating | 0.3442/0.3375 | 0.9844 [0.8389–1.1606] | 28928/29592 | 0.0851/0.0870 | 1.0220 [0.9565–1.0624] | 151474 |
| exceptions | 0.3294/0.3486 | 1.0033 [0.9042–1.1064] | 29248/29596 | 0.5585/0.5653 | 1.0407 [0.8311–1.1147] | 151781 |
| pruning | 0.5191/0.5439 | 1.0217 [0.9281–1.0698] | 35272/35028 | 0.0946/0.0923 | 0.9792 [0.8939–1.1764] | 151633 |

| Workload | A/A compile range s | A/A runtime range s |
|---|---:|---:|
| memory | 0.3065–0.3966 | 0.0801–0.0843 |
| floating | 0.3195–0.3723 | 0.0805–0.0882 |
| exceptions | 0.2883–0.3443 | 0.4626–0.4996 |
| pruning | 0.4627–0.6019 | 0.0874–0.1022 |

Paired compiler medians span 0.9831–1.0217; every paired range crosses 1. Runtime variation occurs despite identical executables. The observed spread does not establish a repeatable runtime change or avoidable compiler regression. [Common work counters](../student.tests/pa29/evidence171/common-counter-delta.json) are unchanged. Correct semantic costs are retained; no optional transform is justified by this noise.

## New capability costs and scaling

[Initial observations](../student.tests/pa29/evidence171/affected-performance.json) and [larger generators](../student.tests/pa29/evidence171/large-generators.json). Eight samples per size and mode. Entry rejects every capability source, so these are final costs, not equivalent A/B speed comparisons. Every input and executable hash is retained.

| Input | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| selection600 | 0.1103 [0.1024–0.1278] | 19448 | 0.1193 [0.1139–0.1229] | 1400 | 42521 |
| selection1200 | 0.2311 [0.1842–0.2502] | 31908 | 0.1170 [0.1152–0.1213] | 1420 | 84521 |
| selection2400 | 0.4031 [0.3681–0.4643] | 57780 | 0.1139 [0.1133–0.1161] | 1660 | 168521 |
| sequence600 | 0.0148 [0.0144–0.0176] | 8660 | 0.1067 [0.1063–0.1144] | 1416 | 484 |
| sequence1200 | 0.0205 [0.0202–0.0214] | 9804 | 0.1067 [0.1064–0.1087] | 1192 | 484 |
| sequence2400 | 0.0326 [0.0320–0.0368] | 12104 | 0.1064 [0.1058–0.1089] | 1196 | 484 |
| integer-pack600 | 0.0155 [0.0149–0.0160] | 8880 | 0.1102 [0.1094–0.1690] | 1416 | 484 |
| integer-pack1200 | 0.0221 [0.0217–0.0228] | 9796 | 0.1104 [0.1090–0.1109] | 1192 | 484 |
| integer-pack2400 | 0.0352 [0.0344–0.0452] | 12696 | 0.1097 [0.1092–0.1108] | 1196 | 484 |
| sequence6000 | 0.0718 [0.0709–0.0761] | 18300 | 0.1092 [0.1068–0.1108] | 1432 | 484 |
| sequence12000 | 0.1472 [0.1407–0.1787] | 29652 | 0.1079 [0.1064–0.1127] | 1456 | 484 |
| sequence24000 | 0.3027 [0.2959–0.3197] | 52108 | 0.1084 [0.1063–0.1124] | 1504 | 484 |
| integer-pack6000 | 0.0775 [0.0753–0.0795] | 19552 | 0.1101 [0.1096–0.1192] | 1432 | 484 |
| integer-pack12000 | 0.1513 [0.1496–0.1546] | 31908 | 0.1098 [0.1092–0.1106] | 1456 | 484 |
| integer-pack24000 | 0.3137 [0.3053–0.3178] | 56884 | 0.1090 [0.1087–0.1106] | 1504 | 484 |

Initial launcher median 0.0055 s; larger-series launcher median 0.0030 s. Initial sequence compilation (0.0148–0.0352 s) was too short for a wall-time scaling claim, so sizes were extended to 6,000/12,000/24,000. The shortest larger compile is 24.0× its launcher median, and the shortest runtime is 35.6×. All initial measurements remain; no timing exponent is claimed.

[All-sample counter assertions](../student.tests/pa29/evidence171/scaling-counters.json) prove the measured work bounds:

- Selection/first-class alias use: query work **2N+12**, type substitution **5N**, class completions/body transitions **N**, expansion lanes **2N**. Text is **70N+521** bytes for N demanded runtime functions.
- `__make_integer_seq`: query work **N+11**, substitution/frames **N+10**, expansion lanes **N**, static-plan work **N+1**. Source parsing stays at **210 nodes**, one body and one class completion.
- `__integer_pack`: query work **2N+12**, substitution **N+12**, frames **N+14**, expansion lanes **N**, static-plan work **N+1**. Source parsing stays at **250 nodes**, one body and one class completion.
- Generator text stays **484 bytes**; the table and symbol/argument records grow with the number of produced elements. Runtime results remain checked at every size.

The source/typed-query/alias-specialization/ABI owners retain one result per complete identity. Builtin heads are created lazily and have fixed size; generated argument storage and temporary vectors grow linearly and have explicit TU/operation lifetimes. No new global cache, full-tree replay or output-dependent lookup is introduced.

## Stage-scoped acceptance

Optional optimizer work and growth budgets are **zero**. Necessary semantic support adds 16,840 compiler bytes; common generated text is unchanged. The existing generator ceiling of **1,048,576 elements**, evaluator 1,000,000-step/512-depth limits, native frame/data 0x70000000 and 4096 alignment limits, and course timeouts are unchanged.

Historical measurements and their misses remain in [performance170](performance170.md) and prior records. Historical blanket 15% latency/RSS and zero-growth targets are diagnostic self-selected targets under spec §9, not extra PA29 exit gates. Mandated limits, correctness and coverage are preserved. Heavy hosted runtime, optimizer/allocation and self-hosting evidence remain PA30–34 work.
