# Final PA7 compiler performance evidence

Final implementation: `9cceb7913` (semantic corrections in `2fd1e8333`, then the
final comma-form ownership fix). All measurements use frozen binaries, flags,
source inputs and output checks. No compiler speedup or generated-code runtime
improvement is claimed.

| Binary | Source | SHA-256 | Host .text bytes |
| --- | --- | --- | --- |
| PA6 baseline | `c02f4ea09` | `70571a477bac3857ec5c3988fe8798fa2a15b7db3cb89419344ee5964a27c075` | 277702 |
| Pre-audit PA7 | `682532c07` / implementation `d5ab80eb4` | `45890eedc6558192b9c60ca5343d005185bf9ceafe78f343912f31c6868664d5` | 349894 |
| Audited PA7 core | `2fd1e8333` | `8cdf089eec0ea09769eec5e858661e665be8c17e41fb92157a32d3e946e6d82c` | 358790 |
| Final PA7 | `9cceb7913` | `4e6db3301eec4318c11776e2bac06bd6b70593e0b2f8bcf2c4cc950cd815c498` | 358790 |

The PA6 baseline was rebuilt independently and matched its saved hash exactly;
the pre-audit PA7 binary also matched the saved final record. The saved
implementation performance verifier was rerun successfully. Frozen binaries
for this review reside under `/tmp/pa7-audit-evidence/` as `dev/cppgm++`, `before`,
`core`, and `final`. Historical paths in JSON are original measurement locations;
the hashes identify the binaries after relocation. g++ 15.2.0 builds use
`-std=gnu++11 -Wall -O3` and the course runner, consistently for both variants.

## Protocol, budgets and retained observations

Each case has two A/A calibration pairs, a B/B pair and two wall-time ABBA
blocks. Inputs/outputs/binaries are hashed; all successful outputs must match.
Phase/work telemetry is measured separately, with its own overhead campaign.
CPU affinity is fixed and recorded. There are no overlapping builds or
correctness suites. User/system CPU time and context switches supplement wall
time in the final campaigns; they do not replace the wall-time criterion.

Unchanged numeric budgets: each accepted paired wall regression <=10% plus A/A
noise; RSS <=20% plus 1 MiB; stage host .text growth <=50%; 4x source <6x wall
and <5x RSS plus 1 MiB; workload wall time >20x the corresponding startup probe.
No bound was raised to accept an observation.

| Record | Scope / disposition | Observations |
| --- | --- | --- |
| [audit-performance.json](../student.tests/pa7/audit-performance.json) | Complete core revision on CPU 0; rejected AST-expression wall/scaling comparison retained. | 454 |
| [audit-repeat-performance.json](../student.tests/pa7/audit-repeat-performance.json) | Same core binary, both AST-expression sizes; one further wall failure retained. | 56 |
| [audit-final-performance.json](../student.tests/pa7/audit-final-performance.json) | Complete final revision, CPU 4: 26 fixed inputs, all identities/output/work checks; one AST-declaration wall failure retained. | 454 |
| [audit-long-performance.json](../student.tests/pa7/audit-long-performance.json) | Final revision, CPU 4, both AST-declaration sizes with 16 independent TUs/run; complete verifier passes. | 56 |
| [audit-delta-performance.json](../student.tests/pa7/audit-delta-performance.json) | Pre-audit versus final PA7, CPU 4, six semantic inputs with 8 TUs/run; complete verifier passes. | 134 |

Total: **1,154 new observations**, with none removed. The earlier implementation's
908 observations remain in `checkpoint-performance.json` and `performance.json`;
they are archival, not the final audit's performance basis.

The first CPU-0 trial has baseline AST-expression runs near 6.35 seconds, then
11.25–13.57 seconds during a slow interval, while B reaches 17.37 seconds.
Its first paired regression is 31.46%. The repeat has one B run at 9.62 seconds
among runs near 6.5 seconds, producing a 24.03% paired regression. Both failures
remain visible and are superseded by final-binary measurements.

On CPU 4 the final AST-declaration comparison has one B run at 2.877 seconds
among runs near 2.15 seconds: a **17.33%** paired failure against **0.20%** A/A
noise. CPU time also rises, so descheduling alone is not established as the
cause. To reduce short-interval variability, a new frozen comparison increases
independent TUs from 4 to 16 for **both** source sizes, with unchanged per-TU source
bytes and flags, longer primary-input lists frozen for both binaries, ABBA ordering and numeric budgets. Its paired regressions are
0.2–1.1%, and 4x source takes 4.09x wall time. That verified longer group
supersedes the short AST-declaration timing for acceptance; the rejected rows
remain in the full record. `verify --recheck=...` verifies both complete records,
requires matching source/binary/host/CPU identities and a larger TU count, and
prints the exact superseded failure. It cannot suppress output, identity, work
or an unrechecked workload's failure.

Other timing variability is disclosed in the tables: A/A noise reaches 28.88%
on the final nested-AST input and 56.40% on the longer small-declaration input.
The latter's actual ABBA regressions are below 1%, so its acceptance does not
depend on that large noise allowance. These data support bounded compiler cost,
not sub-percent precision or a broad speedup claim.

## Outcomes and justified growth

The direct PA7 comparison ranges from **1.13% faster to 5.29% slower**. The large
loop/memory/floating-point case costs **4.91–5.29% more compiler wall time** with
0.50% A/A noise; this is a disclosed regression. The correction records operand
and compound-store conversions that later lowering must consume. Its work is
linear in the expression graph, and RSS on that case falls about 2.5%. Required
correctness work fits the 10% wall budget without using noise as an allowance.
Other PA7 paired wall changes are within 1.7%; maximum paired RSS growth is
5.28%, on the small template input. No optional transform or unbounded search
was added to obtain these results.

Final host .text is **358,790 bytes**, **+29.20%** from PA6 and **+2.54%** from the
pre-audit PA7 binary. Maximum paired inherited RSS growth is 5.72%. Largest
final median RSS is about 202,736 KiB for AST expressions; the largest PA7
median is about 104,938 KiB. These are compiler memory measurements.

In the longer direct PA7 comparison, 4x source costs 4.037x wall for calls,
4.056x for loops/memory/floating point, and 4.073x for template demand. Across
accepted stage groups, maximum wall scaling is 4.971x. All RSS scaling bounds
pass. Ordinary samples exceed startup by at least 51.4x in the full campaign,
251.0x in the longer AST group and 154.0x in the direct PA7 campaign.

Repeated template demand establishes exactly two specializations and argument
packs and five canonical dependence facts at both sizes. At 4x, calls inspect
14,400 candidates for 57,600 expressions; loops analyze 108,000 expressions
and retain 108,000 conversions; templates analyze 72,000 expressions and retain
28,800 conversions. All expression/constant work is <= graph nodes, stored
conversions <=3*nodes and dependence work <= canonical types. Demand processed
matches the deduplicated queue. These counters observe existing work.

Telemetry adds 0.64–1.20% paired wall in the full final campaign and 0.37–1.10%
in the longer direct campaign, with identical output. The latter also observes
about 4.7 MiB additional peak RSS with telemetry; ordinary compiler comparisons
exclude that overhead. `semantic_ms` times declaration consumption;
`frontend_ms` additionally includes member-demand completion.

Generated-program **runtime and text size are N/A**. PA7 emits semantic views,
not executables, and has no LowIR optimizer, MIR, allocator, ELF encoding or
self-hosting surface. No claim about loops, spills, ABI/debug preservation or
native optimization profit is inferred from these compiler measurements.

## Complete accepted stage table

Inherited rows compare PA6 A with final PA7 B. The semantic rows use the same
final binary for A/B to establish absolute cost/noise/scaling. The two
declaration rows use the longer verified record; TUs/run makes that distinction
explicit. Other raw observations, including superseded rows, remain above.

| Input | TUs/run | B wall seconds (range) | B RSS KiB | Paired B/A wall | A/A noise |
| --- | --- | --- | --- | --- | --- |
| ast-classes-1 | 4 | 0.317 (0.316–0.318) | 17122 | 0.999, 0.995 | 3.00% |
| ast-classes-4 | 4 | 0.330 (0.329–0.331) | 17816 | 0.995, 1.002 | 1.13% |
| ast-declarations-1 | 16 | 2.082 (2.067–2.098) | 22688 | 1.006, 1.007 | 56.40% |
| ast-declarations-4 | 16 | 8.515 (8.505–8.545) | 80180 | 1.011, 1.002 | 1.70% |
| ast-expressions-1 | 4 | 1.602 (1.593–1.627) | 64462 | 1.002, 1.011 | 2.00% |
| ast-expressions-4 | 4 | 6.397 (6.388–16.095) | 202736 | 1.002, 1.059 | 0.16% |
| ast-nested-1 | 8 | 0.568 (0.534–0.606) | 8548 | 1.027, 1.132 | 28.88% |
| ast-nested-4 | 8 | 2.821 (2.762–2.891) | 21492 | 1.116, 1.057 | 9.06% |
| ast-procedural-1 | 4 | 0.871 (0.864–0.881) | 33382 | 1.001, 1.012 | 5.09% |
| ast-procedural-4 | 4 | 3.511 (3.487–3.513) | 116508 | 1.010, 1.012 | 3.55% |
| ast-templates-1 | 4 | 0.530 (0.529–0.533) | 21232 | 1.014, 1.006 | 4.29% |
| ast-templates-4 | 4 | 2.138 (2.128–2.141) | 72710 | 1.001, 1.007 | 3.08% |
| semantics-calls-1 | 4 | 0.515 (0.511–0.518) | 29074 | 0.989, 1.002 | 0.97% |
| semantics-calls-4 | 4 | 2.066 (2.060–2.077) | 104938 | 0.979, 1.002 | 0.13% |
| semantics-memory-float-loops-1 | 4 | 0.517 (0.515–0.521) | 28670 | 1.005, 1.005 | 1.52% |
| semantics-memory-float-loops-4 | 4 | 2.071 (2.057–2.079) | 100958 | 0.994, 1.003 | 1.61% |
| semantics-template-demand-1 | 4 | 0.466 (0.462–0.478) | 21836 | 1.016, 0.996 | 1.35% |
| semantics-template-demand-4 | 4 | 1.873 (1.858–1.881) | 75128 | 0.993, 1.004 | 1.25% |
| types-constants-1 | 4 | 0.429 (0.425–0.430) | 24444 | 1.010, 1.019 | 1.89% |
| types-constants-4 | 4 | 1.761 (1.747–1.766) | 82188 | 1.003, 1.000 | 0.43% |
| types-namespaces-1 | 4 | 0.319 (0.314–0.323) | 18526 | 1.015, 1.015 | 3.53% |
| types-namespaces-4 | 4 | 1.345 (1.327–1.356) | 65966 | 1.029, 1.027 | 3.24% |
| types-signatures-1 | 4 | 0.324 (0.320–0.326) | 15088 | 1.035, 1.037 | 3.76% |
| types-signatures-4 | 4 | 1.293 (1.280–1.300) | 48048 | 1.027, 1.032 | 0.62% |
| types-templates-1 | 4 | 0.366 (0.365–0.368) | 17494 | 1.006, 1.022 | 1.37% |
| types-templates-4 | 4 | 1.515 (1.505–1.523) | 58200 | 1.018, 1.025 | 1.51% |

## Direct PA7 before/after table

Both binaries implement PA7; source programs and output hashes match. All six
rows compile eight independent TUs per process.

| Input | TUs/run | B wall seconds (range) | B RSS KiB | Paired B/A wall | A/A noise |
| --- | --- | --- | --- | --- | --- |
| semantics-calls-1 | 8 | 1.016 (1.013–1.024) | 29212 | 1.004, 0.989 | 0.33% |
| semantics-calls-4 | 8 | 4.102 (4.092–4.122) | 104920 | 0.996, 1.004 | 0.27% |
| semantics-memory-float-loops-1 | 8 | 1.013 (1.009–1.022) | 28604 | 1.013, 0.995 | 1.70% |
| semantics-memory-float-loops-4 | 8 | 4.110 (4.086–4.116) | 100952 | 1.053, 1.049 | 0.50% |
| semantics-template-demand-1 | 8 | 0.915 (0.912–0.915) | 21840 | 1.004, 1.009 | 1.21% |
| semantics-template-demand-4 | 8 | 3.726 (3.703–3.808) | 70602 | 1.017, 0.996 | 2.49% |

## Reproduce verification

```sh
python3 student.tests/pa7/benchmark.py verify /tmp/pa7-audit-evidence/dev/cppgm++ /tmp/pa7-audit-evidence/before student.tests/pa7/performance.json
python3 student.tests/pa7/benchmark.py verify /tmp/pa7-audit-evidence/dev/cppgm++ /tmp/pa7-audit-evidence/final student.tests/pa7/audit-final-performance.json --recheck=student.tests/pa7/audit-long-performance.json
python3 student.tests/pa7/benchmark.py verify /tmp/pa7-audit-evidence/before /tmp/pa7-audit-evidence/final student.tests/pa7/audit-delta-performance.json --audit-delta --repeat-factor=2
```

Without `--recheck`, the full final record deliberately reports its retained
17.33% failure. Recheck validates the complete 26-workload coverage and the
longer replacement group's unchanged numeric budgets. To remeasure, use
`measure` with matching options and `taskset -c 4`; retain a new record instead
of overwriting these observations. Compiler source and host/input identities
must be frozen again if any implementation or measurement inputs change.
