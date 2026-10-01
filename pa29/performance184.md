# Performance184 — explicit cast selection and constant identity

Acceptance is **PA29/O0**, under spec §9. These changes implement required
semantics; they introduce no optional transform and make no speedup claim.
[Performance183](performance183.md) and all inherited observations are preserved.

## Frozen protocol

A is entry `68ee6f8d`; B is implementation `52c00f4d`. Binary SHA-256:

```
A 337588703754eb605fa5680f99b8bdf88425eb80f0c0a7fb7bc776c8999b66dd
B 7cb0fefbc4e09af9dc5c7dabb0d28f81028a475809e3d4db5f652f99828f4f4a
```

Flags are `-O0 -c --stats`. All measurements use CPU 0 affinity. Compiler wall
time/peak RSS and executable wall time/peak RSS are measured separately using
`/usr/bin/time`; host linking and correctness checks are outside timing. Four
A/A samples calibrate noise before six ABBA blocks for each comparable workload.
All observations and outliers are retained; affinity does not eliminate external
CPU contention. Validation and other compiler jobs did not overlap timing.

The [common data](../student.tests/pa29/evidence184/common-performance.json)
contains **224** observations for the inherited fixed 2,400-specialization inputs
with live loops, calls, memory, floating point, exceptions and dormant bodies.
The [owner data](../student.tests/pa29/evidence184/owner-performance.json)
contains **104** observations: 56 for equivalent runtime cv casts, and 48 for
new constexpr casts at three sizes, plus eight launcher measurements. The latter
are final-only because the frozen entry compiler rejects them; no rejection-to-
success timing ratio is claimed. Sources/hashes, expected results, flags and
binary identities are retained. The owner executables read seed 7 from argv and
check six million varying step calls against an independently calculated Python
checksum; Clang-built controls agree.

## Equivalent correct inputs

All five A/B object pairs are **byte-identical**. This proves unchanged generated
code for these workloads; runtime noise does not imply changed optimization
quality. Ratios are medians of six paired block ratios, with their complete
range. RSS is maximum compiler RSS; all runtime RSS observations remain in JSON.

| Input | Compile A/B s | Compile B/A [range] | RSS A/B KiB | Runtime A/B s | Runtime B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.2758/0.2751 | 0.9989 [0.9726–1.0585] | 29608/29416 | 0.0725/0.0730 | 1.0074 [0.9974–1.0588] | 151633/151633 |
| floating | 0.2740/0.2744 | 1.0033 [0.9826–1.1353] | 29608/29672 | 0.0586/0.0584 | 1.0017 [0.9893–1.0098] | 151474/151474 |
| exceptions | 0.1648/0.1648 | 0.9979 [0.9477–1.0131] | 29604/29508 | 0.2517/0.2516 | 1.0046 [0.9902–1.0403] | 151781/151781 |
| pruning | 0.2016/0.2021 | 1.0002 [0.4764–1.0087] | 35740/35580 | 0.0690/0.0723 | 0.9995 [0.8874–1.1507] | 151633/151633 |
| runtime2400 | 0.1777/0.1788 | 0.9889 [0.8839–1.0101] | 33044/32952 | 0.0949/0.0950 | 0.9968 [0.9759–1.0003] | 113060/113060 |

| Input | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| memory | 0.2596–0.3320 | 0.0726–0.0739 |
| floating | 0.2708–0.2761 | 0.0585–0.0612 |
| exceptions | 0.1637–0.2121 | 0.2509–0.2522 |
| pruning | 0.2011–0.2109 | 0.0723–0.0739 |
| runtime2400 | 0.2393–0.2628 | 0.0948–0.0954 |

The common paired compiler medians are 0.9979–1.0033; runtime-cast compilation
has median ratio 0.9889. Every compiler paired range crosses unity. The pruning
block ratio of 0.4764 includes an entry-compiler outlier of 0.6395 s, versus its
0.2016 s median; this is retained, not treated as a speedup. These observations
do not establish a repeatable avoidable compiler regression. RSS changes in
both directions remain disclosed, including the floating workload's increase.

## Newly accepted constexpr ownership

Each of N source assertions calls its constexpr specialization twice. The body
combines a C-style reference base conversion with qualifier removal and a
member-pointer base adjustment. Eight compiler and eight executable observations
are retained per size. `.text` remains **661 bytes**, with identical object
hashes across all sizes. Timing the existing generated loop is separate from
measuring the increased required frontend work.

| N | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---:|---:|---:|---:|---:|---:|
| 600 | 0.0780 [0.0762–0.0799] | 15328 | 0.1001 [0.0990–0.1023] | 1760 | 661 |
| 1200 | 0.1510 [0.1466–0.2507] | 23348 | 0.1012 [0.1003–0.1023] | 1756 | 661 |
| 2400 | 0.3030 [0.2916–0.4879] | 39640 | 0.1014 [0.0988–0.1028] | 1756 | 661 |

Launcher median is 0.0014 s, range 0.0013–0.0020 s.

The [scaling assertions](../student.tests/pa29/evidence184/scaling.json) verify all
**24** compiler observations: parsed nodes = 27N+422; projected nodes = 85N+422;
N specializations and N body transitions; N repeated-execution cache hits;
constant execution work = 9N+10; dependency work = 4N+7; conversion work = 14N+58.
Address work stays **8**, object work **7**, and member constant values **2**.
The existing constant-storage/member identities are shared across specializations.
Source/prepared LowIR remains **91 instructions / 140 operands**. None of the N
constexpr-only specializations contributes an emitted function. Linear required
frontend growth is disclosed alongside memory, executable timing and text; IR
counts are not used as evidence of runtime profit.

## Work budgets and disposition

Each cast visits its two canonical qualification chains and required indexed
base paths, with O(depth) qualifier work and O(1) new temporary storage. Selection
publishes existing typed conversion fields once for the expression/substitution
owner. No new cache, allocation family, fixed-point work or optional code growth
is introduced. Existing constexpr/object-width/frame/data/alignment limits stay
in force, as do mandatory inline limits of **64** nesting levels, **262,144**
reserved units per caller and **4,194,304** per program. Native preparation and
optimization policies are unchanged.

Inherited blanket 15% compiler latency/RSS and zero-growth targets remain
**diagnostic under spec §9**; their historical measurements are preserved. No
mandated limit, correctness rule or coverage is weakened. Later optimizer,
broad hosted runtime and self-hosting requirements retain their PA32/33,
PA30/31 and PA34 owners and do not add PA29 exit gates.

Reproduce with frozen binaries from the evidence manifest:

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py OUT/common ENTRY FINAL
PERF_CPU=0 python3 student.tests/pa29/performance184.py OUT/owner ENTRY FINAL
python3 student.tests/pa29/analyze184.py
```
