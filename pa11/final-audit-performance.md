# PA11 final audit performance evidence

Measured source is `cd09b606`; A is completed-stage `0773e4d8`. The final
action-identity naming cleanup in `9c0a9c4e` produces a byte-identical compiler.
Both frozen compilers use the same GNU C++11/O3 host build and O0 source invocation.
Final B SHA256: `fe662c96e7ed834276169929d4b99e4076331e048867f0c3ceeefb9ea0367280`.
A SHA256: `48ded3f8de96305e55004427cadb840a6128f7353de8dffaf8c8c2512a2fbfbc`.
Compiler `.text`: **782022 → 785798 bytes (+3776, 0.48%)**.

The [frozen protocol](../student.tests/pa11/performance-protocol.md#independent-final-audit)
fixes generators, flags, affinity, four A/A observations and two ABBA blocks.
Compiler timing and native execution are separate; validation, compilation of
native programs and telemetry are outside execution timing. No builds or other
test campaigns ran concurrently. Hardware counters are not a dependency.
All observations, including stalls, remain in the raw records:

- [Initial common campaign](../student.tests/pa11/audit-initial-common-performance.json):
  `b262971d`, frozen hash `a68dcfc04f54128766a178a74dba986e3e2cb200e671239941ca3a5b86810062`;
  retained after the duplicate temporary-zero correction.
- [Final common campaign](../student.tests/pa11/audit-common-performance.json).
- [Final affected families](../student.tests/pa11/audit-affected-performance.json).
- [Noisy-group and longer-runtime follow-up](../student.tests/pa11/audit-followup-performance.json).

Together these retain **336 compiler and 96 runtime observations**, plus
compiler/native startup observations and separate telemetry. The initial and
final common campaigns each contain 108 compiler and 36 runtime observations;
the affected campaign contains 48 and 12, and the follow-up contains 72 and 12.
All source/binary/output hashes and complete observation order are verified by
`student.tests/pa11/audit_verify.py`, including checked native outcomes.
Frozen artifacts are under `$RALPH_ARTIFACT_DIR/pa11-final-audit`; their exact
absolute paths are in the JSON. The checkout compiler matches frozen final B.

## Common compiler and executable behavior

All nine common compiler outputs are **byte-identical**. The three common
native pairs are also **byte-identical**, with text payloads 206/434/230 bytes
for calls/memory/floating respectively. Thus the audit does not introduce a
native code change on these workloads. Actual timing observations still appear
below; differences on identical binaries are not compiler-generated speedups.

| Input | A/B median ms | B/A ABBA blocks | A/A spread | A/B peak KiB |
| --- | ---: | --- | ---: | ---: |
| calls-1 | 401.236 / 401.643 | 0.9964 / 1.0142 | 1.38% | 75176 / 75300 |
| memory-float-1 | 347.693 / 346.071 | 0.9925 / 0.8321 | 1.19% | 68724 / 68732 |
| references-1 | 16.606 / 16.679 | 0.9942 / 1.0036 | 2.28% | 6780 / 6752 |
| template-semantics-1 | 67.586 / 67.766 | 1.0061 / 0.9997 | 1.27% | 12468 / 12584 |
| calls-4 | 1650.741 / 1678.578 | 1.0437 / 1.0082 | 0.64% | 303068 / 302928 |
| memory-float-4 | 1399.045 / 1394.333 | 0.9990 / 0.9634 | 1.40% | 245836 / 245856 |
| references-4 | 50.283 / 50.365 | 1.0042 / 0.9983 | 2.42% | 13556 / 13424 |
| template-semantics-4 | 263.639 / 262.983 | 0.9977 / 0.9742 | 3.58% | 37368 / 37388 |
| references-8000 | 116.645 / 115.655 | 0.9937 / 0.9924 | 40.58% | 25560 / 25536 |

Common compiler median changes range from approximately -0.85% to +1.69%.
Peak RSS changes are at most 124 KiB in this campaign (128 KiB in its follow-up).
Calls-4 repeats at +2.19% median, with 1.0138/1.0512 ABBA ratios and 10.55%
A/A spread. Memory-float-1's initial 0.8321 block and the reference wall spikes
are retained. These observations do not establish a broad speedup or a
repeatable material regression. No added semantic/IR work appears in the common
output paths. Compiler startup medians are about 5.62/5.35 ms; short compiler
inputs remain diagnostic under the inherited 20x-startup criterion.

| Native workload | A/B median ms | B/A ABBA blocks | A/A spread | A/B peak KiB |
| --- | ---: | --- | ---: | ---: |
| calls-long | 477.955 / 490.440 | 1.0458 / 1.0130 | 3.42% | 256 / 256 |
| memory-long | 279.185 / 282.617 | 1.0123 / 1.0183 | 2.27% | 256 / 256 |
| floating-long | 330.976 / 331.507 | 1.0023 / 0.9982 | 0.36% | 256 / 256 |

## Affected compiler work and memory

The completed-stage A is correct on these personal inputs. Both outputs were
validated and executed before timing. Nested omitted initialization changes IR
shape under the handout's exact-zero rule; equivalence is checked through the
source's specified results. No course comparison rule is modified. Joined
literal outputs and native binaries remain byte-identical.

| Input | A/B median ms | B/A ABBA blocks | A/A spread | A/B peak KiB |
| --- | ---: | --- | ---: | ---: |
| nested-initializers-1000 | 248.179 / 76.698 | 0.3055 / 0.3189 | 0.72% | 53040 / 17320 |
| nested-initializers-4000 | 1048.255 / 302.025 | 0.2877 / 0.3396 | 45.06% | 198936 / 56264 |
| joined-literals-500 | 830.459 / 879.158 | 0.8845 / 1.0195 | 4.62% | 178528 / 176492 |
| joined-literals-2000 | 3352.604 / 3367.342 | 1.0044 / 0.9902 | 1.87% | 700492 / 692100 |

The 4000-family nested case reduces median compiler latency from about 1.05 s
to 0.302 s, and peak RSS from 198936 to 56264 KiB. The repeated median is
1.000/0.306 s with 198928/56432 KiB. Although the A-side wall spikes make its
calibration noisy, the large reduction persists in every ABBA pair across both
campaigns. At 1000 families final B is below 20x compiler startup, so that
latency result is diagnostic; the larger input establishes the compiler result.

For 1000/4000 families, A emits 201002/804002 instructions; B emits
13002/52002. B's action counts are 4003/16003, and its IR pool capacities are
2936832/11747328 bytes. These are proportional source/action/IR costs, with
3.94x median B latency for 4x families. The extra semantic action records replace
unbounded lowering reconstruction and eliminate substantially larger IR pools.
The independent 32/million-element checks stay at 13 ordinary or 33 volatile
instructions; 2/4/6 nested dimensions stay at 102/266/430 instructions.

Joined-literal peak RSS falls by about 2 MiB/8 MiB at the two scales, while IR
bytes and capacities remain identical (167855528/671417768 pool bytes).
Source spellings are no longer copied/retained for an unused display view.
Timing noise prevents a precise latency benefit claim: the initial 500-family
median rises 5.86%, while the repeat falls 1.97%; the 2000-family repeat falls
2.28%. The memory/ownership correction is retained without a latency claim.

## Follow-up observations

The same frozen binaries, flags and source/output hashes are used throughout:

| Input | A/B median ms | B/A ABBA blocks | A/A spread | A/B peak KiB |
| --- | ---: | --- | ---: | ---: |
| memory-float-1 | 347.661 / 347.473 | 1.0001 / 0.9963 | 5.17% | 68728 / 68692 |
| calls-4 | 1661.720 / 1698.132 | 1.0138 / 1.0512 | 10.55% | 302944 / 303072 |
| references-8000 | 118.037 / 117.006 | 0.9247 / 1.9947 | 3.56% | 25684 / 25556 |
| nested-initializers-4000 | 999.978 / 305.986 | 0.3040 / 0.2367 | 68.08% | 198928 / 56432 |
| joined-literals-500 | 833.591 / 817.139 | 0.9596 / 0.9010 | 38.40% | 178532 / 176572 |
| joined-literals-2000 | 3396.912 / 3319.345 | 0.9859 / 0.9758 | 13.65% | 700572 / 692124 |

The original observations are not replaced by these repeats. References-8000
still has a 1.9947 block despite near-equal medians. Nested-family and literal
A/A calibrations also contain large stalls. Their causes were not established;
no small or sub-percent timing claim is based on them.

## Executed initialization work and text

The first 120000-iteration runtime is retained as a short diagnostic:
75.402/17.685 ms medians, 0.2271/0.2514 paired ratios. Acceptance uses the longer
2400000-iteration variant with the same per-iteration work and checked checksum.
A volatile loop bound prevents a compile-time trip count, a varying index/value
updates one array, and a separate untouched element must remain zero. The
emitted and encoded initialization work was inspected, not inferred from IR
node counts alone.

Long runtime: **1435.521 → 270.758 ms** median;
B/A ABBA ratios **0.2074/0.1879**, with
**0.265%** A/A spread. Peak RSS is **256/256 KiB**.
Native text payload is **20680 → 319 bytes**. Both executables return zero.
The sectionless ELF has no static data; the established payload-after-entry
metric therefore measures its code. Disassembly shows two 4096-byte `rep stosb`
sequences in B instead of A's unrolled scalar stores, with the same 8208-byte
frame. This is a measured executable benefit (about 79–81% in the paired blocks),
not merely fewer IR nodes. Native allocation/encoding remains the supplied
backend's responsibility; no allocator quality claim is made.

Native startup samples are 3.798, 951.004, 3.711 and 3.464 ms. The isolated
startup stall is retained. Median startup is about 3.75 ms; both long-runtime
medians exceed 20x that value. All long-runtime paired observations and A/A
calibration remain available, rather than discarding a startup outlier or using
the short run as the acceptance measurement.

## Stage-scoped acceptance and reproduction

Correctness fixes are not compared against incorrect implementations: packed
static fields, missing constructor calls, dropped argument effects, access and
default temporary lifetimes use reducers, not speed ratios. Common correct
programs preserve outputs and code. The affected correct initializer workload
shows repeatable compiler/memory/runtime/text benefits; joined-literal ownership
improves memory without an unsupported latency claim. Added compiler text is
0.48%, with bounded action/IR work and no optional transform or higher-level pass.

Historical 1.10x latency, 1.20x RSS plus 16 MiB, 128 KiB compiler-text and
4x-input scaling targets are diagnostic, as already required by spec.md.
Historical misses and initial/final observations are preserved. They neither
permanently fail a corrected implementation nor create new PA11 exit gates.
The implementation's eight-element repetition budget, correctness, stage
coverage and unchanged comparisons remain required. Native optimization,
template body generation and self-hosting stay at their actual later stages.

```sh
python3 student.tests/pa10/benchmark.py delta <A> <B> student.tests/pa11/audit-common-performance.json <scratch>/final-common 0773e4d8
python3 student.tests/pa11/audit_benchmark.py <A> <B> <scratch>/affected student.tests/pa11/audit-affected-performance.json
python3 student.tests/pa11/audit_followup.py student.tests/pa11/audit-common-performance.json student.tests/pa11/audit-affected-performance.json student.tests/pa11/audit-followup-performance.json <scratch>/followup
python3 student.tests/pa11/audit_verify.py
```

Use fresh output paths for a new campaign; the retained measurements and frozen
artifacts must not be overwritten. Required final validation is recorded in
[the independent audit](audit.md#validation-and-handoff-ledger).
