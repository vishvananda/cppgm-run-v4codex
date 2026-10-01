# Performance177 — PA29 / O0

This group implements required scope, branch-demand and lifetime semantics.
Optional optimization work and code-growth budgets are **zero**. No speedup is
claimed. Entry rejects the affected selection syntax, so comparisons use common
correct inputs; affected inputs supply scaling and checked-output evidence.

## Frozen protocol

The [manifest](../student.tests/pa29/evidence177/manifest.json) freezes entry/final
binaries, scripts, hashes and flags. [Common raw evidence](../student.tests/pa29/evidence177/common-performance.json)
retains AAAA calibration plus six ABBA blocks for compiler latency/RSS and
separately measured executable runtime (224 samples). Host linking is outside
both timers. Workloads contain 2,400 demanded templates plus checked loops,
calls, memory, floating point, exceptions or unused functions. Runtime inputs
and checks prevent timing a folded/dead workload. Flags are `-O0 -c --stats`.
All observations, including outliers and phase/work counters, are retained.
No affinity or hardware-counter dependency was added. Builds and correctness
suites completed before timing; external system activity is uncontrolled.

| Workload | Compile A/B median s | Compile paired B/A [range] | Compiler peak RSS A/B KiB | Runtime A/B median s | Runtime paired B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.3620/0.3844 | 1.0360 [0.9183–1.1707] | 29588/29592 | 0.0808/0.0805 | 0.9972 [0.9296–1.0491] | 151633/151633 |
| floating | 0.3454/0.3365 | 0.9325 [0.8522–1.0496] | 29664/29728 | 0.0812/0.0785 | 0.9808 [0.8651–1.0044] | 151474/151474 |
| exceptions | 0.3810/0.3765 | 0.9884 [0.9092–1.0817] | 29572/29628 | 0.5114/0.5609 | 1.0518 [0.7126–1.5261] | 151781/151781 |
| pruning | 0.5220/0.5304 | 0.9947 [0.9435–1.2851] | 35796/35796 | 0.0885/0.0923 | 1.0257 [0.9843–1.2234] | 151633/151633 |

| Workload | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| memory | 0.3468–0.4148 | 0.0965–0.1157 |
| floating | 0.3027–0.3819 | 0.0784–0.0814 |
| exceptions | 0.3285–0.3916 | 0.4278–0.9210 |
| pruning | 0.4723–0.8339 | 0.0875–0.0930 |

All four A/B objects and executables are byte-identical, as verified by hashes.
Compiler paired medians range from 0.9325 to 1.0360; peak compiler RSS grows at
most 0.22%. The wide calibration/block spreads do not establish a repeatable
compiler regression or speedup. Runtime medians include a 5.18% exception and
2.57% pruning increase; identical images and wide A/A spreads rule out changed
generated instructions as their cause. All slower samples remain in the record.

## Affected semantics

[All affected observations](../student.tests/pa29/evidence177/selection-performance.json)
contain eight compiler and eight runtime samples at 600/1200/2400 specializations
(48 samples). Each specialization has a constexpr alias initializer selecting
one of an ordinary if initializer or switch initializer. Runtime executes
24 million calls with a `strtol` seed of 17 and varying loop inputs. Python
independently computes the printed checksum and every run checks it.
Flags are `-std=c++11 -O0 -c --stats`. Entry rejection diagnostics are retained.

| Specializations | Compile median [range] s | Compiler peak RSS KiB | Runtime median [range] s | Runtime peak RSS KiB | Executable text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | 0.3156 [0.2932–0.3464] | 23920 | 0.4181 [0.3488–0.4379] | 1888 | 84760 |
| 1200 | 0.6139 [0.5730–0.6722] | 40416 | 0.4553 [0.4011–0.8278] | 1944 | 169360 |
| 2400 | 0.5802 [0.5374–0.6432] | 73176 | 0.6045 [0.6004–0.6158] | 2172 | 338560 |

Launcher median **0.00358 s**, range **0.00302–0.02875 s**.
[All-sample scaling checks](../student.tests/pa29/evidence177/scaling.json) retain exact linear relationships:

- `parsed_nodes = 15N + 370`.
- `nodes = 167N + 370`.
- `instructions = 22.5N + 43`.
- `text_bytes = 141N - 80`.

Executable text is `141N + 160` bytes. The half-instruction slope averages the
two equally represented constexpr arms. Compiler wall time is non-monotonic
across sizes, so it is not used to prove scaling; the all-sample counters supply
that evidence. Runtime rises at 2,400 functions despite a fixed total call count.
This is an O0 generated-program limitation, without a profiler-based attribution
or an affected speedup claim. The shortest medians exceed 80× median launcher
cost. These results preserve the semantic requirement and expose later optimizer
work rather than imposing an unsupported new percentage gate.

## Acceptance and boundaries

No optional optimizer work/growth was introduced. Semantic work follows actual
statements, canonical declarations and demanded specialization facts; lowering
follows selected statements and lifetime actions. Common executable text is
unchanged. Necessary semantic costs and later optimization work are separate
from the current PA29/O0 acceptance. Timings do not certify whole-stage design.

The inherited unsupported blanket 15% latency/RSS and zero-growth gates remain
diagnostic under spec §9, as documented in [performance176](performance176.md).
No historical measurements were removed. Mandatory capacities remain generated
elements 1,048,576, constexpr steps 1,000,000, call depth 512, packed source-site
capacity 2^31−1, native frame/data capacity 0x70000000 and alignment 4096.
Course timeouts, correctness and coverage are unchanged. Heavy hosted runtime,
optimization and self-hosting remain later-stage work.

Reproduction uses the manifest binaries:

```sh
python3 student.tests/pa27/performance147_common.py OUT_COMMON ENTRY FINAL
python3 student.tests/pa29/performance177.py OUT_AFFECTED ENTRY FINAL
python3 student.tests/pa29/test177.py OUT_CONTROLS FINAL
python3 student.tests/pa29/inspect177.py OUT_INSPECTION
python3 student.tests/pa29/validate177.py OUT_GATES
```
