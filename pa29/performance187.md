# Performance187 — floating GNU complex implementation

Acceptance is **PA29/O0**, under spec §9. This change implements required
semantics; it introduces no optional optimization or speedup claim.

## Frozen measurements and provenance

A is implementation entry `5ef7f89a`; B is `f714397c`, including the final MIR
ABI-reporting repair. The preliminary B was `5c93ddef`. All three frozen
binaries remain bound to their sources in the evidence.

| Binary | SHA-256 |
|---|---|
| Entry A | `a4022c8d9e1a288e70aa4010d1a5d574614d35575f319b56ed088865e0d3c785` |
| Preliminary B | `e10bfd2e14589bceb230eb6c532e3f96adf84c5799c8f62b5f13b41372189793` |
| Final B | `56b1cff876e48842f2328e6a4a297b05cef25a0b1152b697720dc1e4e85148d1` |

Flags are `-O0 -c --stats`; CPU 0 affinity is fixed. Compiler and executable
wall time/peak RSS are measured separately using `/usr/bin/time`. Linking,
initial checksum checks and Clang controls occur outside the timed samples.
Each shared workload/mode uses four A/A samples followed by six ABBA blocks.
All samples, outliers, hashes, counters and inputs are retained.

The [common comparison](../student.tests/pa29/evidence187/common-performance.json)
has **224** observations; the [complex scaling comparison](../student.tests/pa29/evidence187/owner-performance.json)
has **48** observations and **eight** launcher measurements. The corresponding
[preliminary common](../student.tests/pa29/evidence187/preliminary-common-performance.json)
and [preliminary complex](../student.tests/pa29/evidence187/preliminary-owner-performance.json)
data retain another **272** observations and eight launchers: **544 + 16** total.
The inspection-only repair leaves every measured object and executable
byte-identical to the preliminary run. No sample was replaced or discarded.

No build, course-test or control job overlapped timing. External CPU contention
was uncontrolled. A premature read-only evidence aggregation attempt overlapped
the last complex workload for 0.16 s and failed while its JSON was being written;
it changed no measurement. That run and its full spread remain recorded. The
final aggregation ran only after the measurement process exited successfully.

## Equivalent shared workloads: all four dimensions

These are the exact inherited 2,400-specialization sources covering templates,
memory, loops, floating point, exceptions, calls and dormant-body pruning.
All four A/B object pairs and complete executables are **byte-identical**.
Ratios are medians of six paired block ratios, with their full ranges.
RSS below is maximum compiler RSS; executable RSS is retained in the JSON.

| Workload | Compile A/B s | B/A [range] | RSS A/B KiB | Runtime A/B s | B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1673/0.1693 | 1.0166 [0.9927–1.0488] | 29852/29984 | 0.0525/0.0525 | 1.0016 [0.9925–1.1139] | 151633/151633 |
| floating | 0.1674/0.1671 | 0.9967 [0.8530–1.5431] | 29660/29712 | 0.0493/0.0493 | 0.9992 [0.9940–1.0013] | 151474/151474 |
| exceptions | 0.1674/0.1682 | 1.0068 [0.9769–1.6614] | 29960/30052 | 0.2576/0.2594 | 1.0005 [0.9405–1.0656] | 151781/151781 |
| pruning | 0.2120/0.2123 | 0.9647 [0.7806–1.3472] | 35492/35588 | 0.0538/0.0539 | 1.0014 [0.9981–1.0092] | 151633/151633 |

| Workload | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| memory | 0.1630–0.1689 | 0.0520–0.0528 |
| floating | 0.1656–0.1766 | 0.0494–0.0502 |
| exceptions | 0.1680–0.1690 | 0.2514–0.2548 |
| pruning | 0.2556–0.3517 | 0.0520–0.0649 |

Final paired compiler medians span **0.9647–1.0166**, with every paired range
crossing unity. Small maximum-RSS increases of **52–132 KiB** are disclosed.
The preliminary medians spanned **0.9517–1.0305**, also with every range crossing
unity; preliminary compiler RSS increases were **160–244 KiB**. The final
floating/exception compiler blocks at 1.5431/1.6614 and preliminary exception
runtime range expose scheduling noise. The results establish neither a speedup
nor a repeatable avoidable regression. Generated-code quality on the shared
workloads is unchanged, as established by full object/executable identity.

## New complex semantics: demand scaling and checked runtime

Entry rejects these inputs, so rejection-to-success timing is not a valid
performance ratio. Each source assertion executes one complex-returning constexpr
specialization twice and checks both components. A separate live loop reads argv
seed 7, performs **2,000,000** complex rotations/additions, and accumulates both
components. An independent Python integer model computes both sums and the final
pair; all values are exactly representable in double. Clang-built controls and
the final compiler agree. Eight compiler and eight executable samples are kept
per size.

| N | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---:|---:|---:|---:|---:|---:|
| 600 | 0.1103 [0.1085–0.1187] | 13168 | 0.1194 [0.1181–0.1223] | 1760 | 1173 |
| 1200 | 0.2095 [0.2063–0.2148] | 18920 | 0.1208 [0.1198–0.1238] | 1756 | 1173 |
| 2400 | 0.4039 [0.3993–0.4480] | 30012 | 0.1180 [0.1178–0.1204] | 1868 | 1173 |

Launcher median is **0.00515 s**, range **0.00504–0.00560 s**. The checked work
dominates startup. All three object/executable images are identical. Text includes
the executable's own `.text`; the ordinary dynamic libgcc helper dependency is
external, and its execution cost is included in wall time.

[Scaling checks](../student.tests/pa29/evidence187/scaling.json) assert all **24**
final compiler samples: parsed nodes = **24N+272**, source/projected occurrences =
**56N+272**, N specializations/body transitions/constant bodies/execution-cache
hits, and **9N** constant-execution steps. There are **2N−2** interned floating
constants, **N+9** type-query visits and **N+1** value-query visits. Signature work
stays at **14**. Source and prepared IR stay at **155 instructions / 238 operands**;
native instructions stay at **154**, with two emitted functions and **933** object
text bytes (**1,173** linked text bytes). Constant-only specializations are not
emitted. These are demand/complexity facts, not an inference of runtime profit.

## Legality, costs and budgets

Builtin construction stores the two operands independently; it does not compute
`real + i*imag`, which would change zeros/infinities/NaNs. Arithmetic uses typed
scalar operations or ordinary libgcc complex runtime calls for scaled division
and exceptional multiplication. Constant evaluation computes through the
existing real floating store, with the same component types and order. Volatile
component accesses remain explicit; calls preserve argument evaluation. The
standalone adapter's actual frames are **80/96/112 bytes** for the three echo
functions and **400 bytes** for `main`; no allocation improvement is inferred.

The constant pair uses two interned 32-bit IDs in the existing payload. Type
classification adds bounded checks; builtin signatures are at most three per TU,
runtime helper identities at most six per LowIR program. Construction, projection,
conversion and placement do constant work per pair. Complex arithmetic has a
fixed scalar/call expansion with no optimizer multiplier or fixed point.

Existing constexpr execution (**1,000,000 steps**) and storage/alignment/width
limits remain. Existing inline budgets remain **64** levels, **262,144** reserved
units/caller and **4,194,304**/program, with conservative calls on exhaustion.
No optional transformation, work allowance or growth allowance is added here.
No correctness requirement is traded for a timing target.

As established by audit186, inherited blanket 15% latency/RSS and zero-growth
targets are **diagnostic under spec §9**, not extra stage exit gates. Their
measurements remain preserved. Necessary semantic cost and work owned by
PA30/31, PA32/33 or PA34 do not change PA29 acceptance. No mandatory limit,
comparison rule, test coverage or failing requirement is waived.

Reproduce with:

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py /tmp/pa29-187/common-final /tmp/pa29-187/entry-cppgm++ /tmp/pa29-187/handoff-cppgm++
PERF_CPU=0 python3 student.tests/pa29/performance187.py /tmp/pa29-187/owner-final /tmp/pa29-187/entry-cppgm++ /tmp/pa29-187/handoff-cppgm++
python3 student.tests/pa29/analyze187.py /tmp/pa29-187
```
