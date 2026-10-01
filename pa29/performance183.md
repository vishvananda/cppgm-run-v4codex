# Performance183 — guide declarations and default inquiry facts

Acceptance is **PA29/O0**, under spec §9. This change adds required declaration
semantics and fixes default substitution. There is no optional transform and no
speedup claim. [Performance182](performance182.md) and all inherited measurements
remain unchanged.

## Frozen protocol and equivalent workloads

A is entry `a2ce2670`; B is implementation `054d162c`. SHA-256 identities:

```
A 27de74d246ace237d351a2b4b2f59365be53651bff1b99c9d9bccfa829dcd648
B 337588703754eb605fa5680f99b8bdf88425eb80f0c0a7fb7bc776c8999b66dd
```

Flags are `-O0 -c --stats`. Fixed common inputs retain the 2,400-specialization
frontend load with live loops, calls, memory, floating point, exception cleanup
and dormant declarations. Compiler and executable timings are separate, each
with four A/A samples followed by six ABBA blocks. Peak RSS uses `/usr/bin/time`;
host linking and initial correctness checks are outside timing. Every outcome
is checked. Objects are byte-identical in all four A/B pairs, in both runs.

The first common run had substantial uncontrolled scheduling variation; a brief
additional reducer compilation overlapped part of that run. Its **224** samples
are preserved in [first-run data](../student.tests/pa29/evidence183/common-performance.json).
The serial [confirmation](../student.tests/pa29/evidence183/common-confirm-performance.json)
adds **224** samples with compiler and executable affinity fixed to CPU 0; no
other validation job ran concurrently. CPU contention remains uncontrolled.
The **96** owner observations and **eight** launcher samples below are separate.
Nothing was dropped or replaced to obtain the final disposition.

Ratios below are medians of six paired block ratios, with their full range.
RSS is maximum ABBA compiler RSS; raw calibration and runtime RSS remain in JSON.
Text is linked `.text` size. This table uses the pinned confirmation.

| Input | Compile A/B s | Compile B/A [range] | RSS A/B KiB | Runtime A/B s | Runtime B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.2937/0.2937 | 0.9766 [0.6662–1.0311] | 29444/29752 | 0.0732/0.0730 | 0.9929 [0.9790–0.9990] | 151633/151633 |
| floating | 0.2798/0.2814 | 1.0066 [0.8298–1.0840] | 29712/29668 | 0.0556/0.0562 | 1.0035 [0.9755–1.0454] | 151474/151474 |
| exceptions | 0.1649/0.1637 | 0.9922 [0.7065–1.0010] | 29928/30036 | 0.2543/0.2551 | 1.0029 [0.9802–1.0235] | 151781/151781 |
| pruning | 0.2051/0.2056 | 1.0109 [0.9839–1.0211] | 35756/35980 | 0.0523/0.0524 | 1.0017 [0.9920–1.0054] | 151633/151633 |

| Input | Confirmation compile A/A s | Confirmation runtime A/A s | First compile B/A [range] | First runtime B/A [range] |
|---|---:|---:|---:|---:|
| memory | 0.3293–0.4405 | 0.0729–0.0741 | 1.0953 [0.8752–1.2213] | 1.0053 [0.6708–1.0420] |
| floating | 0.1690–0.2696 | 0.0494–0.0587 | 0.8773 [0.6651–1.1134] | 0.9965 [0.9233–1.1085] |
| exceptions | 0.1664–0.2235 | 0.2518–0.2533 | 1.0371 [0.8205–1.2682] | 1.0486 [0.9697–1.3100] |
| pruning | 0.2025–0.2052 | 0.0522–0.0523 | 1.0285 [0.8780–1.2790] | 0.8527 [0.5567–1.3023] |

The confirmation's paired compiler medians range from 0.9766 to 1.0109, and
all compiler paired ranges cross unity. The first memory median of 1.0953 is
not repeated. These observations do not establish a repeatable avoidable
compiler regression or a speedup. Runtime differences on byte-identical objects
do not demonstrate changed generated-code performance. RSS changes, including
increases, remain disclosed; no threshold is used to hide any sample.

## Newly accepted owner inputs

[Owner data](../student.tests/pa29/evidence183/owner-performance.json) freezes all
six generated source texts/hashes and eight compiler plus eight executable
observations per size. [Entry probes](../student.tests/pa29/evidence183/entry-rejections.json)
confirm that A rejects every input, so no rejection-to-success latency ratio is
claimed. Independent Python checksums and Clang executions agree with B. Runtime
reads argv seed 7 and checks 12 million varying step calls; the defaults family
also checks every demanded specialization. These executable loops dominate
launcher time (median 0.0070 s; range 0.0041–0.0117 s).

| Input | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| guides600 | 0.1880 [0.1737–0.2000] | 14408 | 0.2333 [0.2132–0.2530] | 1416 | 591 |
| guides1200 | 0.2106 [0.2011–0.2620] | 21908 | 0.2335 [0.2291–0.2545] | 1416 | 591 |
| guides2400 | 0.4200 [0.3429–0.4631] | 37128 | 0.2091 [0.2046–0.2197] | 1416 | 591 |
| defaults600 | 0.1740 [0.1481–0.2561] | 15176 | 0.2437 [0.2131–0.2658] | 1344 | 48845 |
| defaults1200 | 0.3205 [0.1848–0.4516] | 23636 | 0.2197 [0.2106–0.2434] | 1368 | 97445 |
| defaults2400 | 0.5896 [0.3699–0.8108] | 38732 | 0.2347 [0.2091–0.2732] | 1676 | 194645 |

[Scaling assertions](../student.tests/pa29/evidence183/scaling.json) check all
**48** owner compilations. For N guides: parsed/total nodes = 56N+229, guide
parameters = 2N, declaration specializations = 2N+1, type substitutions = 3N,
query work = 6N+1, query edges = 5N. There are zero template body transitions;
only the three benchmark functions are checked. LowIR remains 70 instructions /
110 operands and native linked text **591 bytes** at every size. Guide vector
capacity is **36,864 / 73,728 / 147,456 bytes** (36-byte records).

For N demanded default specializations: parsed nodes = 15N+270, total projected
nodes = 59N+270, one source default binding, N default facts and N body
transitions. Source and prepared LowIR agree at 17N+71 instructions / 26N+112
operands; linked text is 81N+245 bytes. Those separately demanded O0 function
bodies account for code growth. The default's inquiry is a typed query, not a
replayed parse or a fabricated runtime parameter. Memory, text and timings all
remain reported; IR counts alone are not a runtime-profit claim.

## Budgets and disposition

Guide lookup is indexed by primary identity and canonical signature. Per-guide
work is bounded by its parameter/type/query graph, with visited identities for
deducibility; default validation is linear in its source expression and reuses
source-owned queries under substitution. No global scan, fixed-point optimizer,
extra callable body or generated guide code is added. TU-owned vectors grow
geometrically. Optional transform work and code-growth allowances remain zero.

Existing constexpr, object-width, generated-element, native frame/data and
alignment limits remain enforced. Mandatory inline limits stay **64** nesting
levels, **262,144** reserved units/caller and **4,194,304**/program; this change
does not touch native preparation. Prior mandatory-inline costs remain in
performance180–182. Historical blanket 15% latency/RSS and zero-growth targets
remain diagnostic under spec §9, without weakening mandated limits, correctness
or coverage. General optimizer, broad hosted runtime and self-host costs retain
their PA32/33, PA30/31 and PA34 owners and add no PA29 exit gate.

Reproduce using the frozen binaries named in the manifest:

```sh
python3 student.tests/pa27/performance147_common.py OUT/common ENTRY FINAL
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py OUT/confirm ENTRY FINAL
python3 student.tests/pa29/performance183.py OUT/owner FINAL
python3 student.tests/pa29/analyze183.py OUT
```
