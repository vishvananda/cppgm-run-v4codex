# PA11 construction completion

PA11 is complete: **302/302**, including all four behavioral controls. The
construction continuation fixes all 29 failures reproduced at 786ed29e, with
zero regressions. Cumulative PA1–PA11 passes **1327/1327**; file audit passes
with its existing Analyzer-header advisory. Seventeen personal programs validate
LowIR and execute; ten rejection tests pass, including braced constructor
narrowing. Source fixtures and comparison coverage are unchanged.

Inherited forwarding, converting temporaries, complete/base ABI demand, TLS,
placement allocation, constructor-array static data and expression/parser
boundaries extend the existing semantic action pipeline. The ownership and
complexity table is in [the plan](plan.md). The seventh narrow reference edit
restores required static reference binding; its reduced ordering program,
C++11 proof and pinned bundle revision are in
[reference corrections](reference-corrections.md).

## Frozen evidence

A: 786ed29e, SHA256
`5ad6bb599da4cadcb0490547195043481054b97c139578dbea77486f39af3cab`.
B: 265440db, SHA256
`48ded3f8de96305e55004427cadb840a6128f7353de8dffaf8c8c2512a2fbfbc`.
Compiler `.text`: **723,590 → 782,022 bytes (+58,432; 8.08%)**.

The [protocol](../student.tests/pa11/performance-protocol.md#construction-completion-campaign)
fixes source generators, flags, CPU affinity and AAAA/ABBA/ABBA order. Compiler
wall/RSS and executable execution are measured separately; telemetry is separate.
No builds or other test campaigns ran concurrently with timing. All nine common
compiler output pairs are byte-identical, as are all three native pairs.

Final raw evidence: [common](../student.tests/pa11/construction-common-performance.json),
[follow-up](../student.tests/pa11/construction-repeat-performance.json),
[new behavior](../student.tests/pa11/construction-behavior-performance.json),
[memory diagnostic](../student.tests/pa11/construction-memory-diagnostic.json).
These retain 108 common compiler, 36 common runtime, eight compiler-startup,
96 follow-up compiler, 32 new-family compiler and four new-runtime observations,
plus separate telemetry. Frozen artifacts, observation completeness and native
outcomes were verified explicitly.

The initial af72ea1d compiler, SHA256
`060a1c6596c93b56a809851d86d47afef0f658676dae6b52e87207911ff1e0f5`,
had 781,766 bytes of compiler text. Its complete
[common](../student.tests/pa11/construction-initial-common-performance.json) and
[behavior](../student.tests/pa11/construction-initial-behavior-performance.json)
campaigns remain intact. Final review added the missing braced-constructor
narrowing check; both campaigns were repeated with the corrected frozen B.
Initial measurements are historical, not substituted for final observations.

Reproduction commands (scratch directories contain frozen inputs and outputs):

```sh
python3 student.tests/pa10/benchmark.py delta /tmp/pa11-construction-base-cppgm /tmp/pa11-construction-reviewed-cppgm student.tests/pa11/construction-common-performance.json /tmp/pa11-construction-reviewed-common 786ed29e
python3 student.tests/pa11/construction_benchmark.py /tmp/pa11-construction-reviewed-cppgm /tmp/pa11-construction-reviewed-behavior student.tests/pa11/construction-behavior-performance.json
python3 student.tests/pa11/selection_repeat.py student.tests/pa11/construction-common-performance.json student.tests/pa11/construction-repeat-performance.json calls-1 memory-float-1 template-semantics-1 calls-4 memory-float-4 references-4 template-semantics-4 references-8000
python3 student.tests/pa10/benchmark.py verify student.tests/pa11/construction-common-performance.json
```

## Common compiler measurements

Wall values are medians. A/A spread is `(max-min)/mean` of the calibration;
both complete ABBA block ratios are shown, including spikes.

| Input | A/B wall ms | B/A blocks | A/A spread | A/B peak KiB |
| --- | ---: | --- | ---: | ---: |
| calls-1 | 402.644 / 417.008 | 0.9905 / 1.1574 | 1.23% | 73720 / 75212 |
| memory-float-1 | 349.504 / 359.615 | 1.0218 / 1.1873 | 3.80% | 67628 / 64884 |
| references-1 | 16.238 / 16.746 | 1.0391 / 1.0263 | 1.15% | 6532 / 6796 |
| template-semantics-1 | 68.111 / 68.538 | 1.0080 / 1.0165 | 16.74% | 12580 / 12596 |
| calls-4 | 1636.920 / 1708.597 | 1.0797 / 0.9999 | 1.86% | 281828 / 301660 |
| memory-float-4 | 1391.637 / 1428.096 | 1.0597 / 1.0198 | 2.39% | 241836 / 245844 |
| references-4 | 48.112 / 50.168 | 1.0525 / 1.0295 | 4.18% | 13008 / 13624 |
| template-semantics-4 | 266.189 / 262.254 | 0.9785 / 0.9648 | 6.15% | 36924 / 36872 |
| references-8000 | 114.330 / 118.175 | 1.0391 / 0.9632 | 62.78% | 24576 / 25816 |

Final common median differences span **−1.48% to +4.38%**. Median compiler
startup is 5.645/5.630 ms; references-1/-4 and template-semantics-1 are below
20× startup and remain diagnostics. Disclosed noisy groups are repeated with
unchanged inputs, binaries and order; no observations are removed:

| Input | A/B wall ms | B/A blocks | A/A spread | A/B peak KiB |
| --- | ---: | --- | ---: | ---: |
| calls-1 | 400.689 / 405.249 | 1.0122 / 1.0223 | 1.68% | 73632 / 75224 |
| memory-float-1 | 342.719 / 343.752 | 1.0111 / 0.9983 | 32.77% | 67548 / 64888 |
| template-semantics-1 | 67.807 / 67.708 | 1.0016 / 0.9892 | 0.88% | 12584 / 12572 |
| calls-4 | 1631.008 / 1680.602 | 1.0099 / 1.0452 | 2.18% | 281840 / 301788 |
| memory-float-4 | 1402.335 / 1416.077 | 1.0217 / 1.0076 | 1.70% | 241816 / 245848 |
| references-4 | 48.211 / 50.981 | 0.2928 / 1.0478 | 2.01% | 12952 / 13528 |
| template-semantics-4 | 259.965 / 262.848 | 1.0238 / 1.0076 | 6.41% | 36900 / 36840 |
| references-8000 | 113.645 / 120.011 | 1.0555 / 1.9843 | 3.77% | 24636 / 25876 |

Reference wall spikes and calibration noise persist in the follow-up. The
measurements do not support a speedup or a precise sub-percent claim. The largest
common/follow-up peak-RSS increase is **19,948 KiB (7.08%)**, on calls-4. Initial
common evidence had a largest increase of 7,072 KiB; it remains preserved.

Separate instrumented A/B calls-4 observations report 281,708/301,936 KiB;
memory-float-4 reports 256,580/245,540 KiB. All shared semantic work counters,
instruction/operand counts and IR pool capacities are identical within each
pair. Both inputs have 210,000 recorded conversions and 336,000 instructions;
their IR capacities remain 73,138,176 and 71,041,024 bytes respectively. This
rules out additional counted semantic/IR work on these inputs, but does not
prove an allocation cause. Required conversion materialization identities,
complete/base storage and emitted-name reservations add owned state; these
diagnostics do not attribute the entire RSS difference to any one record.

## Newly correct behavior and scaling

These are B-only AAAA observations. A lacks inherited/converting/TLS behavior;
its dynamically correct constructor arrays lack the required static IR facts.
Neither is an equivalent speed baseline for those required outcomes. Each
generated input validates LowIR and executes before measurement. Telemetry is
collected in a separate invocation.

| Input | Median ms | Wall range ms | Peak KiB | Instructions |
| --- | ---: | --- | ---: | ---: |
| inherited-1000 | 124.373 | 123.452–125.806 | 27460 | 22002 |
| inherited-4000 | 502.345 | 497.778–506.433 | 93452 | 88002 |
| converting-1000 | 120.970 | 120.080–123.240 | 27076 | 18002 |
| converting-4000 | 487.738 | 484.667–577.730 | 93988 | 72002 |
| constant-arrays-1000 | 144.837 | 144.009–145.389 | 30172 | 19002 |
| constant-arrays-4000 | 588.918 | 587.194–590.127 | 107120 | 76002 |
| tls-1000 | 102.232 | 101.361–102.728 | 23528 | 21002 |
| tls-4000 | 418.394 | 409.987–498.848 | 81688 | 84002 |

Four times the families gives **4.03–4.09× median wall** and **3.40–3.55× peak
RSS**. Retained converting/TLS wall spikes are included in the ranges. The short
TLS input is below 20× compiler startup; its counters provide the work evidence.
For N families, inherited constructors have 2N subobject actions and 22N+2
instructions; converting calls have N materializations and 18N+2 instructions;
constant arrays have 2N cached constructor actions, 4N constant fields and 19N+2
instructions; TLS has N constructor actions and 21N+2 instructions. These
counts track owned declarations/actions and required output, not unrelated
cross products. This is bounded required construction work, not an optimization
profit claim for early static initialization.

The unchanged eight-element expansion budget and compact omitted/string/volatile
ranges retain their [earlier measured bounds](layout-checkpoint.md#new-behavior-and-bounded-ranges).
The final aggregate-cursor and volatile personal programs still validate/execute.

## Generated programs

| Common runtime | A/B median ms | B/A blocks | A/A spread | A/B text bytes |
| --- | ---: | --- | ---: | ---: |
| calls-long | 478.519 / 487.160 | 1.0030 / 1.0157 | 3.88% | 206 / 206 |
| memory-long | 279.483 / 281.138 | 1.0229 / 0.9746 | 5.84% | 434 / 434 |
| floating-long | 332.994 / 332.749 | 0.9891 / 1.0032 | 2.27% | 230 / 230 |

All three A/B executables are **byte-identical** and return zero; each has
256 KiB observed RSS. The unchanged supplied backend emits sectionless ELFs,
so payload after the entry is the text proxy for these programs without static
data. Runtime variation does not reflect different generated instructions.

The additional program checks eight million volatile-bounded iterations through
inherited and converting constructors with a varying checksum. Median execution
is **75.023 ms**, range **74.799–75.377 ms**, peak RSS **256 KiB**, text proxy
**277 bytes**, checked exit zero. There is no correct A speed baseline.

## Stage-scoped acceptance

PA11 requires O0 object semantics and proportional work. Its handout mandates
no numeric compiler-time, RSS or compiler-text ratio. Historical 1.10× latency,
1.20× RSS +16 MiB, +128 KiB text and 4×-input 5.5×-time/5×-RSS targets remain
diagnostics under the spec's stage-scoped rule. The measured calls-4 RSS delta
exceeds 16 MiB; it is disclosed and investigated above, not hidden or promoted
to a new exit gate. All original and final measurements remain preserved.

No optional optimizer is added and no runtime speedup is claimed. The changes
provide required semantic/ABI facts, with proportional work and conservative
dynamic fallback when constructor effects or conversion chains cannot be
represented statically. Native selection/allocation remains owned by later
assignments. Mandated expansion bounds, correctness and coverage are unchanged.
Final required checks, personal checks and evidence verification pass; no PA11
behavior group is deferred.
