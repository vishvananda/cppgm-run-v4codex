# PA29 handoff155 evidence

This increment implements mandatory hosted input and canonical type-query
semantics. It adds no optional optimizer and claims no runtime speedup. PA29
remains incomplete. These measurements support this implementation handoff,
not whole-stage acceptance or later self-host performance.

## Frozen protocol

A is stage-entry `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`, SHA256
`79a205ace278949bd6fa118eeb9e35b1ab59a61007fc21f18746a257b7e5f727`.
B contains implementation through `6bf1369b`, SHA256
`4e4873df66898a2bb16a06304569616f1fec412178b4ced3d5ab01cd6206d385`.
Build configuration, machine, attempts and exact input/image hashes are retained
in [manifest](../student.tests/pa29/evidence155/manifest.json) and the raw JSON.
The compiler build uses `-std=gnu++11 -Wall -O3`; measured compiler invocations
use `-O0 -c --stats`. `g++` only links emitted host objects. Both compilers use
the same frozen environment. CPU affinity is `taskset -c 0`; no builds or tests
ran concurrently with timed measurements. External scheduling is uncontrolled.

Reproduction, with the frozen binaries in the recorded scratch locations:

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py /tmp/pa29-155/common /tmp/pa29-155/base /tmp/pa29-155/final
PERF_CPU=0 python3 student.tests/pa29/performance155.py /tmp/pa29-155/affected /tmp/pa29-155/base /tmp/pa29-155/final
python3 student.tests/pa29/controls155.py
```

The common harness freezes the inherited memory/call, floating, exception and
pruning inputs, each with 2,400 demanded function templates. Every workload
checks a runtime-input-dependent checksum. Compilation and execution each use
four A/A calibration samples followed by six ABBA blocks. Paired ratios divide
the B mean by the A mean within each block. Peak RSS comes from `/usr/bin/time`;
phase counters observe the same work. [Telemetry controls](../student.tests/pa29/evidence155/telemetry.json)
verify identical object bytes with `--stats` removed.

## Equivalent correct common paths

[Final repeat: all 224 observations](../student.tests/pa29/evidence155/common-performance.json).
Seconds are medians, RSS is maximum KiB. Text is executable `.text*` bytes.

| Input | Compile A / B (s) | Paired compile B/A median [range] | Compile RSS A / B | Runtime A / B (s) | Paired runtime B/A median [range] | Text A = B |
|---|---:|---:|---:|---:|---:|---:|
| Memory/calls | .16027 / .15973 | 1.0068 [.9607, 1.3557] | 29116 / 29608 | .05296 / .05275 | .9935 [.8459, 1.0039] | 151633 |
| Floating | .15576 / .15696 | 1.0086 [1.0004, 1.2795] | 29244 / 29704 | .04909 / .04897 | .9998 [.9982, 1.0013] | 151474 |
| Exceptions | .15630 / .15577 | 1.0067 [.9810, 1.0163] | 28976 / 29736 | .25280 / .25218 | .9993 [.9922, 1.0301] | 151781 |
| Pruning | .19345 / .19360 | .9989 [.7559, 1.0182] | 34640 / 35232 | .05265 / .05235 | .9948 [.9872, 1.0067] | 151633 |

All four entire object files and linked executables are byte-identical A/B.
Runtime changes therefore do not demonstrate a generated-code improvement.
Compiler paired medians show at most 0.9% overhead in this repeat; raw outliers
are substantial and are not discarded. Peak compiler RSS increases 460–760
KiB. These small resource increases accompany required preprocessing/query
capability, not an optional transformation with an unproved payoff.

Repeat A/A compile ranges are .1512–.1635, .1553–.1720, .1551–.1609 and
.1902–.1942 seconds; runtime A/A ranges are .05190–.05271, .04889–.04935,
.25161–.25367 and .05239–.05576. Full A/B ranges remain in the raw evidence.
The [first final-binary run](../student.tests/pa29/evidence155/final-noisy-common-performance.json)
had memory compile A/A 1.57–6.55 seconds, while later samples returned near
.16 seconds. That observation triggered one complete repeat, not selective
sample removal. Both runs and the [earlier implementation common run](../student.tests/pa29/evidence155/before-attribute-common-performance.json)
are preserved. No precise speedup or universal regression bound is inferred.

## New capability costs and scaling

[All 56 observations plus launcher calibration](../student.tests/pa29/evidence155/affected-performance.json).
A correctly rejects these unsupported inputs, so its failure latency is never
a speed baseline. B uses eight compilation and eight execution samples at each
size, plus eight preprocessing samples. The generated sources and their hashes
are fixed by the checked-in harness. Runtime executes 60 million calls dependent
on `argc`, checking checksum 4,230,000,000; compilation demands every template.

| Demanded class/function pairs | Compile median [range] s | Compile RSS KiB | Runtime median [range] s | Runtime RSS KiB | Executable text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | .07504 [.07405, .07676] | 18316 | .26628 [.26519, .27029] | 1760 | 25933 |
| 1200 | .14718 [.14294, .15230] | 29780 | .26760 [.26343, .27395] | 1760 | 51733 |
| 2400 | .30277 [.29434, .30661] | 51616 | .26624 [.26152, .28012] | 1760 | 103333 |

10,000 macro/probe-generated declarations preprocess in .16507 seconds
[.16285, .16809], peak 6996 KiB; output checks every generated declaration count
and the final name. Median launcher overhead is .00453 seconds, well below
compilation and the .266-second runtime workload. The [earlier affected run](../student.tests/pa29/evidence155/before-attribute-affected-performance.json)
used six million iterations; it is preserved but not compared for runtime
speedup. The final workload was lengthened to reduce the launcher fraction.
[Setup correction](../student.tests/pa29/evidence155/benchmark-setup.json)
records a generator grammar typo corrected before any timed observations.

Observed query work is 4211/8411/16811 and candidate work 602/1202/2402.
Body transitions are 600/1200/2400 while retained source regions stay at three.
The new facts track actual specialization demands; measured latency, storage
and emitted code grow with those demands. This is scaling evidence, not a
proof for every remaining hosted template pattern.

## Budgets, ownership and acceptance

Mandatory preprocessing/parsing remains proportional to source and produced
tokens. Immutable builtin tables have fixed size. Shape queries consume one
canonical type; nested-array queries traverse rank; conversion queries follow
actual overload candidates and their dependencies. Interned query facts and
special-member properties are owned by the translation unit and reuse the
existing success/failure state machines. Access is fixed at the definition
owner before storing special-member properties, preventing caller-dependent
cache results. No cross-product scan, speculative body clone or new optimizer
worklist is added. Additional optional optimization work and growth budgets
are **zero**; existing native work/growth limits are unchanged.

Spec §9 requires correct equivalent comparisons and documented mandatory costs.
The inherited blanket 15%/zero-growth diagnostics in [PA28's plan](../pa28/plan.md)
are not PA29 mandates. They do not turn measured required semantics into an
extra exit gate. All historical measurements, correctness requirements and
coverage remain preserved. Self-hosting is explicitly outside PA29; its fixed
later-stage benchmark and requirements are neither claimed nor waived here.

The [77 explicit controls](../student.tests/pa29/evidence155/controls.json) include
negative grammar/access cases and object execution. Four hosted sources also
pass `-c --emit-lowir --validate-lowir`, the LowIR reader and this compiler's
native backend/MIR dump, then execute successfully. [Inspection evidence](../student.tests/pa29/evidence155/inspection.json)
records hashes and typed snippets. The numeric roundtrip canonicalizes textual
f/L suffixes on explicitly typed constants; the types and executed values remain.
[Validation](../student.tests/pa29/evidence155/validation.json) records final
course results, unchanged fixtures and the repaired intermediate regressions.
