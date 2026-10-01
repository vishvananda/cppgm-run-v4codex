# Performance185 — bit-integer semantics and ABI

Acceptance is **PA29/O0**, under spec §9. This is required semantic work, with
no added optional optimization pass and no speedup claim. All earlier evidence,
including [Performance184](performance184.md), remains preserved.

## Frozen protocol

A is entry `c6d7a46a`; B is implementation `27012cf5`. Binary SHA-256:

```
A 7cb0fefbc4e09af9dc5c7dabb0d28f81028a475809e3d4db5f652f99828f4f4a
B 5bbd875bcbf039f6b7b57f79f38d6524c88cd53c8ccd1074d5e85d85b1faea68
```

Flags are `-O0 -c --stats`; measurements use CPU 0 affinity, wall time and
`/usr/bin/time` peak RSS. Compilation and executable execution are timed
separately; host linking and correctness checks occur outside timing. The four
inherited inputs use four A/A observations followed by six ABBA blocks per mode.
All samples, spread, outliers, flags, hashes and counters are retained.
Compilation/test/inspection jobs did not overlap measurements. CPU affinity does
not isolate the machine from external contention.

[Common observations](../student.tests/pa29/evidence185/common-performance.json)
contain **224** samples. [Owner observations](../student.tests/pa29/evidence185/owner-performance.json)
contain **48** final-only samples plus **eight** launcher samples. Owner inputs
are rejected by A, so rejection-to-success timing is not a performance ratio.

## Equivalent correct implementations

All four A/B objects are **byte-identical**, including code, data and unwind
information. Their checked runtime inputs cover templates, live loops/calls,
memory, floating point, exceptions and dormant bodies. Ratios below are medians
of the six paired block ratios, followed by their full ranges. RSS is maximum
compiler RSS; executable RSS observations are also retained in the JSON.

| Input | Compile A/B s | Compile B/A [range] | RSS A/B KiB | Runtime A/B s | Runtime B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.3387/0.3324 | 1.0095 [0.9565–1.1404] | 29620/29736 | 0.0885/0.0880 | 1.0005 [0.9372–1.0523] | 151633/151633 |
| floating | 0.3590/0.3703 | 1.0254 [0.9237–1.2005] | 29648/29756 | 0.0853/0.0844 | 0.9802 [0.9491–1.0547] | 151474/151474 |
| exceptions | 0.3605/0.3780 | 0.9397 [0.8161–1.3468] | 29032/29360 | 0.4796/0.4806 | 0.9920 [0.9188–1.0106] | 151781/151781 |
| pruning | 0.4534/0.4343 | 0.9567 [0.7927–1.1970] | 35648/35892 | 0.0891/0.0891 | 0.9822 [0.9115–1.0690] | 151633/151633 |

| Input | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| memory | 0.6008–0.7138 | 0.0866–0.1061 |
| floating | 0.3243–0.3517 | 0.0894–0.0950 |
| exceptions | 0.3272–0.3430 | 0.4734–0.6022 |
| pruning | 0.3837–0.5265 | 0.0827–0.0893 |

Compiler paired medians range from **0.9397 to 1.0254** and every paired range
crosses unity. The memory A/A compilation range (0.6008–0.7138 s) is much higher
than either later median; exceptions include a 0.6668 s A outlier. These are
retained rather than treated as optimizer benefits. The floating paired median
increase and all four small peak-RSS increases are disclosed. This run does not
establish a repeatable avoidable compiler regression; the object identity proves
unchanged generated code on the equivalent workloads. Runtime fluctuations on
those identical objects are noise, not new optimization behavior.

## Newly accepted width demand and live wide arithmetic

Each of N assertions executes its constexpr specialization twice. Its return
precision depends on N, spanning widths 3–128; unsigned wrap must produce 1.
No constexpr-only specialization is emitted. Separately, the generated executable
reads seed 7 from argv and performs **600,000** varying 93-bit recurrence steps,
wrapping multiplication/addition and accumulating remainders. The checksum is
calculated independently in Python and checked by both generated and Clang-built
executables. Eight compiler and eight runtime samples are retained at each size.

| N | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---:|---:|---:|---:|---:|---:|
| 600 | 0.1664 [0.1391–0.2109] | 14344 | 1.0090 [0.9307–1.1454] | 1760 | 1698 |
| 1200 | 0.2624 [0.2323–0.3231] | 22020 | 0.9095 [0.8883–1.0369] | 1760 | 1698 |
| 2400 | 0.6285 [0.4476–0.7616] | 35832 | 0.9106 [0.8501–1.0698] | 1752 | 1698 |

Launcher median is **0.0078 s**, range 0.0073–0.0082 s.

The three owner object and executable hashes are identical, with fixed
**1,698-byte** text. [Scaling assertions](../student.tests/pa29/evidence185/scaling.json)
verify all **24** compiler observations: parsed nodes = 21N+246; N
specializations, body transitions, substitution frames and repeated-execution
cache hits; constant execution steps = 8N; substitution work = 2N+2 and retained
substitution records = 2N. Signature work remains **266**. Source/prepared LowIR
stays **83 instructions / 139 operands**, and native instructions stay **272**.
These counters support bounded demand and emission, not a claim that smaller IR
makes a faster executable. The observed compilation/RSS growth is disclosed as
required frontend work; runtime/text do not grow with constexpr demand.

## Budgets and disposition

Width/type identity and normalization add O(1) work per scalar boundary for the
existing 128-bit capacity. Each needed runtime normalization adds at most two
shifts, with no branch/call or iterative pass; a transient type-qualified value
proof avoids repeating it. Width queries and deduction traverse required typed
edges and use existing frame-owned memoization. No optional transform, new global
cache, unbounded search or source/code multiplication is introduced.

Existing object/frame/data/alignment and constexpr limits remain enforced, as do
mandatory inline limits of **64** nesting levels, **262,144** reserved units per
caller and **4,194,304** per program. The explicit 128-bit scalar precision limit
is documented in the handoff. Arbitrary-precision arithmetic is unimplemented;
unsupported widths are diagnosed instead of silently truncated.

Inherited blanket 15% compiler latency/RSS and zero-growth targets remain
**diagnostic under spec §9**, with their historical measurements preserved.
No mandated limit, correctness rule or course coverage is weakened. Later broad
hosted runtime, optimizer and self-hosting requirements retain their PA30/31,
PA32/33 and PA34 owners rather than becoming extra PA29 gates.

Reproduce from the frozen binaries identified above:

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py OUT/common ENTRY FINAL
PERF_CPU=0 python3 student.tests/pa29/performance185.py OUT/owner ENTRY FINAL
python3 student.tests/pa29/analyze185.py
```
