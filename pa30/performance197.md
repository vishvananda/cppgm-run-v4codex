# PA30 implementation197 performance evidence

Correctness changes only; no optimization or speedup is claimed. Measurements
apply at O0 to PA30's hosted compile workload. The required per-compile limit is
45 seconds. No extra blanket percentage, RSS or code-growth exit gate is imposed.
Prior diagnostic targets/data in performance195/196 remain preserved under spec §9.

## Frozen protocol and equivalent workloads

A is entry `407fcdc0`; B contains `6bf7812b` and `94faedf1`. Evidence binds binary
SHA256 values, exact inputs, flags `-O0 -c --stats` and CPU affinity 2. Host g++
only links compiler-produced objects. Runtime checks consume command-line inputs
and validate computed results. Compilation and execution are measured separately
with wall time and `/usr/bin/time` peak RSS. All observations are retained.

[Common evidence](../student.tests/pa30/evidence197/common-performance.json): four
fixed inherited workloads cover templates, loops, calls, memory, floating point,
exceptions and emission pruning. Each mode uses one AAAA noise block and six ABBA
blocks (224 observations). Every A/B object and executable is byte-identical.
Medians do not establish a speedup or repeatable regression; large paired outliers
and nonstationary timing make the complete spread essential.

| Workload | Compile B/A median [range] | Runtime B/A median [range] | Compile RSS A/B KiB | Object text A=B bytes |
|---|---|---|---|---|
| memory | 0.996 [0.980, 1.948] | 1.004 [0.996, 1.007] | 29,632 / 29,816 | 151,393 |
| floating | 0.982 [0.873, 1.016] | 1.004 [0.995, 1.009] | 29,732 / 30,004 | 151,234 |
| exceptions | 0.998 [0.918, 1.008] | 0.992 [0.970, 1.018] | 29,240 / 29,372 | 151,541 |
| pruning | 0.995 [0.671, 1.035] | 0.997 [0.988, 1.003] | 35,680 / 35,856 | 151,393 |

## Corrected-owner workloads

[Owner evidence](../student.tests/pa30/evidence197/owner-performance.json): six
frozen inputs, eight compile and eight runtime observations each (96), plus 16
launch calibrations. The entry compiler rejects these valid inputs, so final-only
cost is reported; rejection timing is not a performance baseline. Runtime runs
3,000,000 dependent steps from seed 7 and checks both final state and checksum.
Median launch costs are 6.69 ms for compiler help and 4.98 ms for `/bin/true`.

| Family / N | Compile median [range] s | Peak RSS KiB | Runtime median s | Object text bytes |
|---|---|---|---|---|
| lookup128 | 0.073 [0.050, 0.085] | 11,936 | 0.067 | 404 |
| lookup512 | 0.176 [0.173, 0.185] | 25,484 | 0.067 | 404 |
| lookup2048 | 3.011 [1.669, 3.395] | 79,848 | 0.108 | 404 |
| ordering128 | 0.362 [0.334, 0.465] | 30,032 | 0.102 | 391 |
| ordering512 | 1.457 [1.371, 1.686] | 96,460 | 0.108 | 391 |
| ordering2048 | 6.560 [6.182, 10.782] | 361,120 | 0.120 | 391 |

Measured work is exactly linear at all three sizes: namespace lookup = `102N+49`;
ordering-family lookup = `75N+66`, shape construction = `2N+1`, ordering work =
`3N`, ordering hits = `3N+2`, class completions = `10N+1`. Source/semantic work
is indexed and proportional to consumed input; no new retry or traversal exists.
Cache lifetime is the Analyzer/TU; keys are immutable argument identities, and
alias substitution/access is checked before shape equality. The existing exact-ID
fast paths avoid shape construction for identical arguments.

The largest namespace timing increases more than its work counts; a separate
large equivalent control diagnoses this without equating rejection with success.
Ordering scaling takes at most 10.782 s / 361,120 KiB across observed trials;
all measured compilations remain within 45 seconds. Output text stays constant
as undemanded type families grow. This is necessary semantic work, with no
optional transform or added code-growth allowance.

## Large equivalent control

[Equivalent evidence](../student.tests/pa30/evidence197/equivalent-performance.json)
uses 2,048 namespace families with shared using-declarations, accepted by both
compilers. This is a separate fixed control, not a substitution for the corrected
alias workload. One AAAA + six ABBA blocks per mode add 56 observations.
Objects and executables are byte-identical. The linker uses a common input object
basename: an initial setup attempt exposed only GNU linker's injected STT_FILE
basename difference (A.o/B.o), before timed observations; that setup was corrected.

Compile B/A paired median **0.989**
(range 0.971–1.012);
A/A range 0.625–0.646 s;
compile medians A/B 0.634/0.628 s,
peak RSS 76,968/77,136 KiB.
Runtime B/A median **1.002**
(range 0.993–1.222);
A/A range 0.066–0.069 s.
Both object text sizes are 404 bytes.
This equivalent control shows no repeatable regression from the merge changes.

A [follow-up](../student.tests/pa30/evidence197/followup-performance.json) repeats
four compile/runtime trials of each original largest corrected input (16 more
observations), with the same frozen B binary, input and output hashes. Original
observations remain intact. Results:

- lookup2048: compile 0.670 s median [0.668, 0.673], peak RSS 79,684 KiB; runtime median 0.066 s.
- ordering2048: compile 3.251 s median [3.224, 3.269], peak RSS 360,996 KiB; runtime median 0.069 s.

The original slow samples are not stable under repetition with unchanged work
and output. They remain reported; a linear wall-time scaling claim is not made.
Together the series has **404 observations plus 16 launch calibrations**.

## Repaired course workloads

[Hosted evidence](../student.tests/pa30/evidence197/hosted-performance.json): four
successful compilations of each repaired fixture (12 observations). These are
compile-contract objects; hosted runtime correctness remains PA31's separate
requirement. Executable runtime/text is measured on the checked controls above.
The tiny namespace fixture is startup-sensitive; no latency benefit is claimed.

| Fixture | Compile median [range] s | Peak RSS KiB | Object text bytes |
|---|---|---|---|
| 600-chrono-duration-convert-owner | 0.407 [0.389, 0.424] | 19,268 | 0 |
| 600-namespace-same-type-alias-convergence | 0.022 [0.020, 0.024] | 7,608 | 47 |
| 700-hosted-unreachable-inline-callee-export-closure | 2.672 [2.503, 2.715] | 88,636 | 38,771 |

Reproduction scripts: `student.tests/pa27/performance147_common.py`,
`student.tests/pa30/performance197.py`, `equivalent197.py`, `followup197.py`, `hosted197.py`.
Inputs, hashes, flags, phase counters, paired samples and spread accompany the
numbers. No benchmark replaces or weakens course coverage. Self-host performance
is owned by PA34 and is not an extra PA30 acceptance gate.
