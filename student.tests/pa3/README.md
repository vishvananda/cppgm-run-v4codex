# PA3 personal validation

Run explicitly from the repository root:

```sh
python3 student.tests/pa3/check.py
python3 student.tests/pa3/check.py --sanitize
taskset -c 0 python3 student.tests/pa3/benchmark.py
make test-pa3
make test-report-through-pa3
perl scripts/cppgm_file_audit.pl --stage pa3 --paths dev/src
```

`check.py` checks a separate, lazily evaluated Python AST oracle (mathematical
integers, target conversions, 6,000 random trees and all boundary operand pairs)
plus malformed grammar, unconditional token rejection in discarded branches,
phase errors, translated logical lines, literal types and 200,000-deep unary,
parenthesis and conditional expressions. A flat 200,000-operation chain must
retain only two values and one operator. `api.cpp` checks every integral
promotion, macro lookup after table growth and macro state changes, detached
result lifetime, and zero allocation calls after scratch warmup. ASan/UBSan
builds are isolated in `obj/student-pa3/`.

`benchmark.py` freezes tool, flags, implementation/harness hashes and seven
inputs. It independently checks complete output hashes, then measures ordinary
execution (A) against optional telemetry (B), with two A/A calibration pairs
and two ABBA blocks per workload. Every wall time, peak RSS and telemetry sample
is retained in `obj/student-pa3/performance-*`. Each invocation times out at
60 seconds; hashing/warmup is outside measurement. `--baseline` and `--inputs`
support equivalent-output comparisons to future frozen implementations.
No speedup is claimed against the nonfunctional PA3 stub.

The 4/16 MiB repeated workload covers mixed signs/types, character literals,
macro identities and lazy branches. Additional workloads cover a 4 MiB flat
chain, 4 MiB parenthesis nesting, 4 MiB nested conditionals, 200,000 distinct
macro names, and 4 MiB invalid-line recovery. These are frontend benchmarks.
Template instantiation, loops, calls, memory, floating runtime and self-hosting
benchmarks require later stages; PA3 emits decimal values, so generated-program
runtime and text size are N/A. Host-tool text size is reported separately.

Budgets fixed before measurement (N = input bytes, V/O = maximum pending
values/operators): decoded units ≤ 2N+64; literal/number bytes ≤ N; expression
tokens ≤ N; reductions ≤ expression tokens; scalar scratch ≤ 32V+6O+64 bytes
(twofold geometric capacity, 16-byte values and 3-byte operations); scratch
growth events ≤ 128. A flat chain must keep V=2, O=1; repeated 4/16 MiB inputs
must have equal scratch and identifier storage. Fourfold growth must take ≤ 6x
latency. Peak RSS must satisfy both 40N+32 MiB and twice the retained source,
name, spelling and expression capacities plus 32 MiB. Identifier storage and
rehash limits remain the PA1/PA2 envelopes. These are resource limits, not
optimization profitability claims; no additional optimization/code-growth
budget is authorized by the measurements.
