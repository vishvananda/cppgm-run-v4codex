# PA7 final compiler performance evidence

Final implementation: `e5e777a65`. Final frozen compiler SHA-256:
`45890eedc6558192b9c60ca5343d005185bf9ceafe78f343912f31c6868664d5`.
PA6 baseline (`14f1d402f`) SHA-256:
`70571a477bac3857ec5c3988fe8798fa2a15b7db3cb89419344ee5964a27c075`.

Both full campaigns pass the complete verifier. The checkpoint at `6690406f2`
is retained in [checkpoint-performance.json](../student.tests/pa7/checkpoint-performance.json);
the final record is [performance.json](../student.tests/pa7/performance.json).
Each contains 454 observations: 364 ordinary workload runs, 24 startup probes,
52 separate phase/work runs and 14 telemetry-overhead runs. No timing was
removed. The frozen binaries remain in `/tmp/pa7-evidence/` as `baseline`,
`checkpoint`, and `final`; their source revisions and build flags are recorded.

## Protocol and unchanged budgets

All binaries use `g++ -std=gnu++11 -Wall -O3` with the course runner enabled.
The records identify the host/compiler, fixed CPU affinity, binary/input/output
hashes, repeat counts, every observation, paired ratios and spread. No builds or
correctness suites ran during either timing campaign. The same output is
written and hashed for each variant, including telemetry mode. Each process
runs four independent TUs (eight for the inherited nested-syntax workload).

Each of 26 fixed workloads uses two A/A calibration pairs, a B/B pair and two
ABBA blocks. The twenty inherited PA6 inputs compare PA6 against the final PA7
binary. Six PA7 inputs cover overloads/indirect calls, loops/memory/floating
point, and repeated template declaration demand, each at 1x and 4x size. For
these new inputs, **A and B are the same final binary**: those pairs measure
absolute cost, noise and scaling, and do not establish a speedup over PA6.
All ordinary runs exceed the relevant startup maximum by at least 46.6x.

Budgets set before implementation remain unchanged: inherited wall regression
<=10% plus A/A noise; RSS <=20% plus 1 MiB; host `.text` growth <=50%; 4x input
<6x wall and <5x RSS plus 1 MiB. The final campaign passes each bound in each
applicable pair. No failed budget was relaxed.

## Results and limits

Inherited paired compiler latency ranges from 0.8% faster to 4.8% slower;
maximum paired RSS growth is 5.3%. No broad compiler speedup is claimed. The
small nested-AST input has 9.08% A/A noise, retained and disclosed in the table.
PA7 wall scaling is 3.999x (calls), 3.925x (loops/memory/floating point) and
4.050x (template demand). Maximum wall scaling across the full corpus is 4.176x.
Largest PA7 median RSS is 104,488 KiB; the largest inherited AST workload reaches
202,742 KiB. These are compiler measurements, not generated-program memory.

Final host compiler `.text` is **349,894 bytes**, versus
277,702 bytes for PA6: **+26.00%**, within the feature budget.
Generated-program runtime and text size are **N/A**: PA7 has no executable
output, lowering optimizer, native backend or self-hosting surface. The new
semantic cache is justified by bounded canonical fact reuse; this record makes
no executable optimization or checkpoint-to-final speedup claim.

The template inputs retain exactly two specializations and two argument packs
per TU at both sizes. Final dependence analysis computes five canonical facts
at both sizes (eleven types); substitution reuses closed type subgraphs without
rechecking them. Expression and constant work stay bounded by source graph
nodes. Telemetry's separate ABBA pairs add 0.38% and 1.06% wall time with
identical output. `semantic_ms` times declaration consumption; `frontend_ms`
also includes completion of the member-demand queue, and emission is separate.
Counters observe existing work and do not cause another analysis traversal.

## Full final table

For `semantics-*` rows, the A/B labels refer to the same final binary as
explained above. Other rows compare the frozen PA6 and PA7 binaries.

| Input | Final wall seconds (range) | Final RSS KiB | B/A paired wall | A/A noise |
| --- | --- | --- | --- | --- |
| ast-classes-1 | 0.318 (0.317–0.320) | 17024 | 0.994, 1.006 | 0.53% |
| ast-classes-4 | 0.331 (0.327–0.360) | 17836 | 1.048, 1.000 | 0.56% |
| ast-declarations-1 | 0.531 (0.531–0.536) | 23014 | 0.994, 1.007 | 2.42% |
| ast-declarations-4 | 2.148 (2.140–2.153) | 80090 | 1.007, 0.997 | 1.92% |
| ast-expressions-1 | 1.613 (1.592–1.629) | 64480 | 1.016, 1.008 | 0.33% |
| ast-expressions-4 | 6.391 (6.351–6.418) | 202742 | 0.993, 1.006 | 0.48% |
| ast-nested-1 | 0.265 (0.265–0.267) | 8576 | 1.026, 1.025 | 9.08% |
| ast-nested-4 | 1.036 (1.034–1.053) | 21510 | 1.024, 1.026 | 0.70% |
| ast-procedural-1 | 0.864 (0.861–0.875) | 33358 | 1.008, 1.015 | 0.58% |
| ast-procedural-4 | 3.501 (3.492–3.508) | 116522 | 1.011, 1.007 | 1.18% |
| ast-templates-1 | 0.530 (0.523–0.536) | 21186 | 1.004, 1.012 | 1.60% |
| ast-templates-4 | 2.122 (2.112–2.134) | 72738 | 1.006, 1.002 | 2.99% |
| semantics-calls-1 | 0.514 (0.511–0.518) | 29240 | 0.984, 1.003 | 0.66% |
| semantics-calls-4 | 2.054 (2.039–2.060) | 104488 | 1.002, 1.007 | 0.80% |
| semantics-memory-float-loops-1 | 0.508 (0.507–0.519) | 28906 | 1.002, 1.004 | 0.45% |
| semantics-memory-float-loops-4 | 1.994 (1.987–2.016) | 103586 | 0.999, 1.002 | 1.78% |
| semantics-template-demand-1 | 0.461 (0.460–0.467) | 21730 | 0.995, 0.997 | 0.13% |
| semantics-template-demand-4 | 1.868 (1.859–1.873) | 75210 | 1.005, 1.004 | 1.26% |
| types-constants-1 | 0.421 (0.419–0.426) | 24440 | 1.011, 1.005 | 0.34% |
| types-constants-4 | 1.739 (1.733–1.750) | 82208 | 0.992, 0.999 | 0.37% |
| types-namespaces-1 | 0.319 (0.315–0.321) | 18740 | 1.014, 1.011 | 0.93% |
| types-namespaces-4 | 1.330 (1.326–1.334) | 65822 | 1.022, 1.018 | 1.47% |
| types-signatures-1 | 0.320 (0.320–0.323) | 15070 | 1.036, 1.028 | 0.67% |
| types-signatures-4 | 1.287 (1.282–1.296) | 48172 | 1.029, 1.038 | 1.62% |
| types-templates-1 | 0.369 (0.367–0.370) | 17578 | 1.015, 1.027 | 2.78% |
| types-templates-4 | 1.492 (1.482–1.505) | 58052 | 1.014, 1.011 | 0.75% |

## Recheck commands

```sh
python3 student.tests/pa7/benchmark.py verify /tmp/pa7-evidence/baseline /tmp/pa7-evidence/checkpoint student.tests/pa7/checkpoint-performance.json
python3 student.tests/pa7/benchmark.py verify /tmp/pa7-evidence/baseline /tmp/pa7-evidence/final student.tests/pa7/performance.json
```

The verifier checks the complete fixed corpus and protocol, binary/text/input
identities, all output hashes, telemetry separation, startup dominance, paired
latency/RSS limits and scaling. Source generators are retained in the personal
benchmark harness; no course fixture or discovery rule was changed.
