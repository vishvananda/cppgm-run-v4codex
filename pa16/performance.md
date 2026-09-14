# PA16 scalar handoff performance

Implementation: `7484f22b934b646c3880c3d46a51d62a7b2e094d`.
Frozen entry compiler SHA-256: `bf32d09de557e00965ca3bb81df35e1cfe01947bd1f03ae39344df55584e125a`.
Frozen handoff compiler: `a6e376110c306c5a5b4cf73dfc027fac34966367ef4c155a93a7770577059543`.

The [harness](../student.tests/pa16/benchmark.py) freezes binaries, inputs,
`--emit-lowir -O0` flags and the supplied native backend. It checks LowIR validity,
exact A/B output parity on common inputs, native parity and executable results
before timing. Common workloads have one warmup per binary, four A/A samples and
two ABBA blocks; newly accepted inputs have a recorded entry rejection and six
handoff samples after warmup. No A/B speedup is inferred from rejected inputs.
Compiler and executable timing are separate, pinned to one permitted CPU.
Runtime inputs are volatile and computed results are checked, so loops/calls,
floating work and array updates cannot disappear as dead or constant workloads.

All four campaigns remain under `$RALPH_ARTIFACT_DIR/pa16-scalar/`:

| Record | Disposition |
|---|---|
| `performance.json` | First frozen implementation; selected benchmark parity passes, but nested-class regressions elsewhere were subsequently repaired. |
| `performance-final.json` | Corrected nested-class implementation, before the final empty-initializer repair. |
| `performance-handoff.json` | Current compiler, with correctness jobs running concurrently; preserve the observed scheduling noise. |
| `performance-serial.json` | Current compiler measured after correctness jobs ended; primary handoff evidence. |

Each record retains all raw timings, peak RSS, warmups, output hashes, native text
sizes, telemetry, A/A range and paired ratios. Across the four campaigns there are
**896 timing observations including warmups**. [The manifest](../student.tests/pa16/handoff.json)
records exact paths/hashes; [the verifier](../student.tests/pa16/verify.py) checks
all frozen sources, binaries, output parity, sample order and completeness.

## Current measurements

Compiler medians and maximum observed RSS, primary serial campaign. Paired ratios
are B/A for the two ABBA blocks, including outliers; no samples are discarded.

| Common source workload | A median ms / peak KiB | B median ms / peak KiB | Paired ratios |
|---|---:|---:|---|
| 1,000 repeated scalar calls | 35.17 / 9,292 | 34.31 / 8,920 | 0.952, 0.981 |
| 4,000 repeated scalar calls | 123.97 / 20,716 | 119.73 / 19,728 | 0.965, 0.964 |
| 1,000 template calls | 22.69 / 7,732 | 22.15 / 7,540 | 0.990, 0.966 |
| 4,000 template calls | 74.83 / 14,628 | 71.47 / 13,200 | 0.815, 0.953 |
| 1,000 memory/floating functions | 101.60 / 21,604 | 103.54 / 21,732 | 1.007, 1.023 |
| 4,000 memory/floating functions | 394.26 / 70,496 | 397.68 / 70,708 | 1.000, 0.861 |

The 1,000-function memory/floating A/A range was 100.28–148.14 ms, exposing a
large scheduling outlier. Other paired outliers also preclude claiming general
compiler speedups. Larger source workloads dominate the roughly 6-ms startup
floor visible in tiny executable-source compilations; smaller sizes additionally
serve scaling/counter checks. Common outputs are byte-identical. The largest
observed common RSS increase is 212 KiB, with the constant-call corpora using less
memory after removal of retained per-expression activation values.

| Newly accepted workload | 1,000: median ms / peak KiB | 4,000: median ms / peak KiB | Time scaling |
|---|---:|---:|---:|
| Scalar loops | 42.64 / 8,832 | 155.11 / 19,464 | 3.64x |
| Floating loops | 54.37 / 9,944 | 201.44 / 23,472 | 3.70x |
| Constexpr array reads | 21.02 / 7,748 | 69.16 / 13,800 | 3.29x |

Scalar loop execution steps are exactly 101,000/404,000; floating steps are
87,000/348,000. Each has 1,000/4,000 activation computations and the same count of
completed-call hits. Floating payload counts are 4,005/16,005. Array child indexes
contain exactly 1,000/4,000 entries. No array-bound expansion occurs for omitted
ranges; a separate native control reads a million-element omitted range.

| Native workload | A/B median seconds | A/B text bytes | Paired runtime B/A |
|---|---|---|---|
| Calls, 72 million iterations | 0.3602 / 0.3616 | 206 / 206 | 0.988, 1.025 |
| Memory updates, 48 million iterations | 0.2099 / 0.2096 | 434 / 434 | 0.999, 0.998 |
| Floating calls, 24 million iterations | 0.2491 / 0.2495 | 230 / 230 | 0.993, 1.001 |
| Newly accepted constexpr-local call workload | entry rejected / 1.3710 | unavailable / 296 | no A/B claim |

The three common native executables are byte-identical. The supplied backend
writes sectionless ELF; text here is executable payload after the ELF entry for
these workloads, which have no static data. Compiler `.text` grows from
1,528,326 to 1,544,838 bytes: **16,512 bytes / 1.08%**. This is compiler-owned
semantic functionality, with no added optional optimization or code-growth search.

## Acceptance and limits

PA16/O0 mandates correctness and bounded ownership/work, with no numeric
latency/RSS/text ceiling. Diagnostic targets chosen for this campaign were at
most 15% common compiler latency growth, 16 MiB extra RSS and 5.5x time for 4x
input size. They guide investigation; they are not added exit gates. The final
common measurements and owner counters show no material avoidable regression.
The preliminary 18% call-block ratio is preserved; later campaigns and raw
observations show it is not a stable implementation cost. Earlier PA14/15 local
text/latency targets do not accumulate into a PA16 gate under spec.md's
stage-scoped acceptance rule.

Actual evaluator limits remain **512 active calls and 1,000,000 executed steps per
root**, with limited/unavailable activation results retryable. Call frames own
flat scalar bindings until return; TU arenas own immutable completed values,
floating payloads and array range indexes. Ordinary semantics/lowering remain
linear or near-linear in visited declarations/IR; new array lookup is logarithmic
in explicit initializer children. Native backend, allocation, higher optimization
levels and self-hosting retain their later-stage owners. The inherited 32-byte
automatic-array backing policy is explicitly unfinished PA16 implementation in
[the plan](plan.md), not a waiver of this stage's broader copy requirement.
