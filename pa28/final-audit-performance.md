# PA28 final audit154 performance

Final code: `abfa68e7`. Audit entry: `03575afb`.
Frozen A SHA-256: `d472892b7a45a7cf5c62ba272e34d0636b78f6f8b1e8007fd83c036890f69ca9`.
Frozen B SHA-256: `79a205ace278949bd6fa118eeb9e35b1ab59a61007fc21f18746a257b7e5f727`.

## Protocol, correctness and scope

The [common measurements](../student.tests/pa28/evidence154/common-performance.json)
use the fixed PA26/27 inputs: 2,400 demanded function templates with checked
loops/calls/memory, floating point, 200,000 throws, and 1,200 unused declarations.
The [affected workload](../student.tests/pa28/evidence154/override-input.cpp)
demands 400 virtual class specializations with a derived exception type allowed
by the base declaration, and executes 240,000 runtime-controlled virtual calls
and class-exception catches. Both frozen compilers correctly compile and run
these inputs. Invalid overrides are tested separately, never used for a speed
comparison against an incorrect baseline.

Each workload/mode has four A/A calibration observations followed by six ABBA
blocks. Compilation and execution are timed separately; host linking is outside
timing. CPU affinity is 0; compile flags are `-O0 -c --stats`. Compiler, input,
object/executable hashes, host version, flags, wall times, RSS, checks and counters
are retained. No compiler build or correctness suite ran concurrently with the
measured sequence. Workload results depend on argc and are checked before and
during timing. No observations were dropped or wins selected across reruns.

The [launcher calibration](../student.tests/pa28/evidence154/launcher-calibration.json)
measures the same time/taskset launcher with `/usr/bin/true`: 4.58–4.91 ms.
The shortest measured runtime is about 48 ms; compilation and the affected
runtime are substantially longer. A/A calibration and all timing spread are
reported rather than subtracting a guessed startup cost.

## Four dimensions together

Seconds are medians of twelve measured samples per A/B label; RSS is maximum
KiB. Text is executable `.text`; full object sizes/hashes are in the raw data.

| Input | Compile s A/B | Compile KiB A/B | Runtime s A/B | Text bytes A/B |
|---|---:|---:|---:|---:|
| memory | 0.1576 / 0.1576 | 29188 / 29180 | 0.0528 / 0.0526 | 151633 / 151633 |
| floating | 0.1584 / 0.1572 | 29276 / 29252 | 0.0489 / 0.0488 | 151474 / 151474 |
| exceptions | 0.1556 / 0.1565 | 29140 / 29084 | 0.2530 / 0.2544 | 151781 / 151781 |
| pruning | 0.2015 / 0.2008 | 35052 / 35188 | 0.0522 / 0.0523 | 151633 / 151633 |
| overrides | 0.1061 / 0.1087 | 21428 / 21408 | 0.2430 / 0.2418 | 163596 / 163596 |

| Input | Paired compile ratio (range) | Paired runtime ratio (range) | Compile A/A s | Runtime A/A s |
|---|---:|---:|---:|---:|
| memory | 1.021 (0.959–1.069) | 0.998 (0.951–1.059) | 0.1556–0.1671 | 0.0524–0.0532 |
| floating | 0.994 (0.821–1.020) | 0.999 (0.954–1.006) | 0.1593–0.3205 | 0.0485–0.0531 |
| exceptions | 1.005 (0.968–1.035) | 0.996 (0.831–1.018) | 0.1552–0.1824 | 0.2500–0.2550 |
| pruning | 0.998 (0.965–1.697) | 1.002 (0.926–1.007) | 0.1967–0.2056 | 0.0753–0.0759 |
| overrides | 1.010 (0.984–1.039) | 1.004 (0.875–1.524) | 0.1078–0.3256 | 0.2413–0.2439 |

The measured +2.1% memory compile ratio and +1.0% affected compile ratio are
disclosed. Scheduling/temporal spread includes a pruning compile block at 1.697
and an override runtime block at 1.524. A/A itself spans 0.108–0.326 s on the
affected compile workload. The earlier pre-cache pruning runtime ratio was 1.070
(range 1.007–1.453); those samples remain available. These observations do not
establish a precise speedup, a uniform slowdown, or a generated-code regression.

## Work, growth and stage acceptance

[Byte comparisons](../student.tests/pa28/evidence154/comparison.json) prove that
all five equivalent A/B objects and entire executables are identical. Actual
instructions, calls, spills, frames, tables, LSDA, relocations and text size
therefore agree, not only IR node counts. Runtime variation between those
executables cannot be attributed to a code-generation change.

[Counter comparisons](../student.tests/pa28/evidence154/counter-comparison.json)
show unchanged common non-time work/storage counts, plus two new zero-valued
telemetry fields. The affected case adds one canonical base path/subobject
fact: **one handler-match computation and 399 cache hits**. Before caching,
399 extra base-adjustment lookups were visible. The refactor removes that
repeated semantic work without asserting a wall-time optimization gain.

No optional executable transform is added. The new declaration check is required
by [except.spec]/5,8. It uses exact typed-set lookup, necessary structural type
comparisons, cached public-base facts and deduplicated implicit-destructor
edges. Its cache has complete immutable inputs and TU lifetime. It triggers no
body/layout demand and no executable growth. The inherited source/semantic,
native-window/flow, frame and ELF budgets remain as documented in the audit.

Spec §9 stage-scoped acceptance applies to the inherited plans. Blanket 15%
latency/RSS and zero-growth targets are self-selected diagnostics; they do not
become additional PA28 gates. Mandatory limits, correctness, timeouts, coverage
and comparison rules are preserved. This correction adds necessary semantic
work with measured, bounded costs and no avoidable repeated matching remaining.
PA32/33 optimizer profitability and PA34 self-hosting remain later-stage work;
PA28's README explicitly excludes hosted headers and bootstrap builds.

## Historical evidence and reproduction

All **560 audit observations** remain: 224 common + 56 affected before cache
consolidation and the same counts afterward. Six launcher samples are separate.
The [before-cache common](../student.tests/pa28/evidence154/before-cache-common-performance.json)
and [before-cache affected](../student.tests/pa28/evidence154/before-cache-override-performance.json)
files retain every intermediate sample. The rerun followed a code change.

Earlier [naming151](../student.tests/pa28/performance151.md),
[EH/ownership152](../student.tests/pa28/performance152.md) and
[layout153](../student.tests/pa28/performance153.md) evidence remains unchanged.
Its incorrect/unsupported baselines were correctly excluded from optimization
comparisons. Standalone new-feature costs cover tagged templates, dynamic
filters, imported VTT/local statics and virtual-primary/secondary construction.
Those records retain every dimension, the sparse-tag/effect packing correction,
noise/outliers and fixed common-input comparisons. They establish the stage's
implementation history; the fresh equivalent comparisons establish the final
audit correction's costs. No historical diagnostic miss is promoted to a
permanent failure of this corrected implementation.

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py OUT_COMMON A B
PERF_CPU=0 python3 student.tests/pa28/performance154.py OUT_OVERRIDES A B
```
