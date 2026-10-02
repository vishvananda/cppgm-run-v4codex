# PA30 final audit204 performance evidence

Code: `4a1e92b8`. Whole-stage A is `27029f9`; B is the final compiler.
For hosted fstream/regex, A is implementation203 (`247c383a`), which accepts
both inputs correctly. Frozen hashes, flags, sources and reproduction scripts
are bound in [source-binding](../student.tests/pa30/evidence204/source-binding.json).

## Protocol and stage-scoped acceptance

Current records contain **288 workload observations plus 16 launcher samples**:
224 common A/A+ABBA, 56 hosted A/A+ABBA, and eight final-only affected-image
rebuilds. Each A/B workload/mode has four A/A observations and six ABBA blocks.
Compilation and checked executable execution are separate. Flags are
`-O0 -c --stats`; CPU affinity is 2. All binaries and inputs are frozen before
measurement, and no build, course report or control suite overlaps timing.
The machine is shared; all outliers remain in the raw records. Hardware counters
are not required. There is no optional optimization or speedup claim.

The mandatory PA30 hosted-compile limit remains **45 seconds**. Historical 15%
latency and zero-growth targets are diagnostic under spec §9; neither they nor
inherited plans create additional gates. Correctness, coverage and existing
inline/lane/growth limits remain mandatory. PA31 runtime completion, PA32/33
optimization-level goals and PA34 self-host/inception acceptance remain scoped
exactly as the handout requires.

## Whole-stage fixed compiler and executable workloads

[Raw common observations](../student.tests/pa30/evidence204/common-performance.json)
cover 2,400 demanded templates, live loops/calls, memory, floating point,
exceptions and unused-definition pruning. Each executable consumes runtime argc
and checks its checksum; host g++ only links compiler-produced objects.
All four object **and executable** A/B pairs are byte-identical.

| Workload | Compile A/B ms | B/A median [range] | Runtime A/B ms | B/A median [range] | Compile RSS A/B KiB | Object / executable text bytes |
|---|---|---|---|---|---|---|
| memory | 167.05 / 168.56 | 1.013 [0.995, 1.089] | 52.24 / 52.42 | 1.006 [0.992, 1.089] | 30136 / 30168 | 151393 / 151633 |
| floating | 165.70 / 166.67 | 1.010 [0.999, 1.279] | 49.01 / 49.01 | 1.001 [0.997, 1.005] | 29908 / 30008 | 151234 / 151474 |
| exceptions | 168.25 / 168.02 | 1.005 [1.000, 1.644] | 250.82 / 250.77 | 0.997 [0.940, 1.004] | 30220 / 30272 | 151541 / 151781 |
| pruning | 211.11 / 210.94 | 0.995 [0.981, 2.258] | 52.49 / 52.91 | 1.004 [0.997, 1.009] | 35864 / 35876 | 151393 / 151633 |

| Workload | Compile A/A ms | Runtime A/A ms |
|---|---|---|
| memory | 166.97–298.03 | 52.17–53.08 |
| floating | 164.29–169.81 | 48.79–49.62 |
| exceptions | 166.63–170.68 | 249.57–257.15 |
| pruning | 205.74–212.28 | 52.85–53.09 |

Compiler paired medians range from 0.995 to 1.013. Every paired range crosses
unity. The pruning 2.258 block ratio and 0.676-second B sample are retained,
not discarded; five other blocks are 0.981–1.008. No repeatable avoidable
regression is established. RSS changes are +12 to +100 KiB. Identical executable
bytes establish unchanged generated work more directly than the small noisy
runtime differences. No instruction-count or runtime speedup is credited.

Launcher calibration is retained in the image-binding record: compiler help
9.75–12.82 ms, `/bin/true` 7.24–9.24 ms (eight each, including measurement
wrapper/process overhead). The shortest checked runtime is about 49 ms and the
exception workload about 251 ms. No startup subtraction is applied; those
measurements cannot justify a percent-level generated-code claim.

## Equivalent heavy headers

[Raw hosted observations](../student.tests/pa30/evidence204/hosted-performance.json)
preserve all 56 compiles and object hashes. Objects match in every repetition
and across A/B. These PA30 fixtures are compile-only; executable runtime is
not an applicable acceptance dimension for them.

| Input | Compile A/B s | B/A median [range] | RSS A/B KiB | Text bytes | A/A s |
|---|---|---|---|---|---|
| 600-hosted-fstream-stream-compile | 1.198 / 1.158 | 0.972 [0.873, 1.043] | 68308 / 68412 | 22301 | 1.111–1.142 |
| 700-libstdcxx-regex-compiler-member-alias-call | 2.624 / 2.620 | 1.002 [0.872, 1.260] | 177328 / 177388 | 222302 | 2.677–3.175 |

The final ABBA maximum is **3.411 seconds / 177,388 KiB**. These ratios also
cross unity. Historical absolute costs are not mixed with these contemporary
A/B samples to claim a gain. The required report separately tests every one
of the 153 stage fixtures under the unchanged timeout.

## Affected-image reuse and required semantic costs

[Fresh image bindings](../student.tests/pa30/evidence204/performance-image-binding.json)
rebuild six vector/packed scale points and the two newly accepted random-header
inputs. All eight current objects exactly match implementation203. Thus its
checked executable/runtime/text observations remain applicable; compiler timing
is still labeled historical and these one-shot current costs are not a relative
performance claim. The final-only observations are:

| Input | Wall s | Peak RSS KiB | Object text bytes |
|---|---:|---:|---:|
| vector64 | 0.106 | 10464 | 13114 |
| vector256 | 0.371 | 19456 | 51514 |
| vector1024 | 1.429 | 56368 | 205114 |
| simd64 | 0.265 | 16344 | 111290 |
| simd256 | 0.951 | 44788 | 444218 |
| simd1024 | 4.294 | 152256 | 1775930 |
| 600-random-to-address-qualified-call | 4.198 | 84160 | 19907 |
| 700-hosted-random-mersenne-rshift-compile | 5.315 | 84532 | 21256 |

The largest supplemental hosted cost is **5.315 seconds**, still within 45
seconds. These absolute samples reflect the shared host and do not establish
a regression against separately timed historical runs. The contemporary
equivalent-header ABBA series above is the applicable relative comparison.

[Performance203](performance203.md) retains 512 workload observations plus 16
launchers. Its equivalent vector snapshots add **24N text bytes** and about
**1–2% runtime**, with the full spread and outlier retained. The snapshot owner
captures a complete value before later mutation and preserves volatile lane
accesses; removing that work would break covered semantics. At O0 this is
bounded required lowering, not an unprofitable optional pass. Its current
counter relations remain N specialization/body transitions, **40N+53 LowIR /
41N+72 native instructions** for vectors and **284N+53 / 367N+72** for packed
families. Packed text is **1734N+314 bytes**. All three current scale points
retain those exact counts.

Fixed packed expansion is ≤16 lanes, ordinary vector expansion ≤8 lanes then
a loop, target storage 64 bytes, and comparison fallback ≤32 alternatives.
Forced-inline admission retains depth 64, 262,144 caller and 4,194,304 program
work reservations and valid-call fallback. The new validator adds constant
work per explicit target instruction, zero IR/code growth and no production
phase work. Telemetry does not request extra analyses or alter emitted objects.
The full audit also inspects actual loop/frame traffic, ABI and CFI rather
deducing runtime profit from IR counts.
