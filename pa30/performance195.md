# PA30 implementation195 performance and ownership

Implementation: `32727741`. The [source binding](../student.tests/pa30/evidence195/source-binding.json)
records every tested implementation file and the frozen entry/final compiler hashes.
The unchanged entry is stage base `27029f978e65b78331233123922d342033d5d1f7`.
The [verifier](../student.tests/pa30/verify195.py) checks input/binary identity,
unchanged coverage, successful measurements, ABBA ordering, recomputed ratios,
and equivalent generated objects/executables: **89 checks pass**.

## Protocol and acceptance

All measurements use CPU 0, `-O0 -c --stats`, identical frozen flags and inputs,
wall time via a monotonic clock, and `/usr/bin/time` peak RSS. Compilation and
execution are timed separately; host g++ only links the compiler's objects.
No build, correctness suite or other benchmark overlapped these observations.
The host is shared and unisolated. Nothing is dropped or startup-subtracted.

[Common data](../student.tests/pa30/evidence195/common-performance.json) retains
224 observations: the four fixed inherited template/loop/memory, floating,
exception/call and unused-definition workloads, each with four A/A observations
and six ABBA blocks in each mode. All four object and executable pairs are
byte-identical; each execution checks its runtime-input-dependent result.
[Owner data](../student.tests/pa30/evidence195/owner-performance.json) retains
144 observations and 16 launcher measurements. Three corrected source families
at 128/512/2048 repetitions each have eight compile and eight checked runtime
observations. Entry rejects these inputs; its failures are recorded, never used
as speed baselines. [Hosted data](../student.tests/pa30/evidence195/hosted-performance.json)
adds 36 final-only compilations of the nine repaired real-header fixtures.
Total: **404 observations plus 16 launchers**, with all raw counters and spreads.

The mandated PA30 limit remains **45 seconds per compile**. There is no new
optimization pass, work search or growth allowance. Parser changes add constant
work per already-visited specifier/delimiter and no allocations. The live cursor
owns split-angle metadata and drops it with consumed tokens. Token size remains
**40 bytes** ([representation check](../student.tests/pa30/evidence195/representation.json)).
The parser still builds each region once; probes only inspect categories and
cached delimiter positions. Canonical semantic graphs, demand/fact ownership,
typed LowIR, native selection/allocation and direct ELF remain the shared path.

The inherited blanket 15% latency/zero-growth targets are diagnostic, per
spec §9 and PA29's final audit. This does not change mandated limits, correct
semantics or coverage. No generated-code optimization benefit is claimed.
PA32–33 optimization and PA34 self-host acceptance remain their own stage work.

## Equivalent fixed workloads

Times are medians in milliseconds; RSS is maximum KiB. Ratios are paired B/A
medians with all block extrema. The raw datasets retain A/A spread as well.

| Workload | Compile A → B ms | RSS A → B KiB | Compile ratio [range] | Runtime A → B ms | Runtime ratio [range] | Object text bytes |
|---|---:|---:|---|---:|---|---:|
| memory | 210.42 → 172.86 | 29896 → 30148 | 0.878 [0.768, 1.075] | 52.79 → 53.00 | 1.001 [0.841, 1.052] | 151393 |
| floating | 168.35 → 168.50 | 29836 → 30064 | 0.996 [0.835, 1.021] | 49.32 → 49.44 | 1.002 [0.998, 1.009] | 151234 |
| exceptions | 169.76 → 169.36 | 30052 → 30216 | 0.991 [0.614, 1.008] | 252.93 → 253.30 | 1.004 [0.990, 1.010] | 151541 |
| pruning | 218.21 → 219.58 | 35656 → 35800 | 0.990 [0.632, 1.068] | 53.60 → 53.15 | 0.997 [0.847, 1.017] | 151393 |

Every paired compile/runtime range crosses unity. Compiler RSS changes are
+144 to +252 KiB; unchanged object/executable bytes preclude a generated-code
speed change. The noisy memory compile median does not establish a speedup.
No repeatable avoidable regression is established and no optional transform is
introduced. The fixes perform required grammar classification.

## Corrected owner scaling

| Family / repetitions | Compile median [range] ms | Peak RSS KiB | Runtime median ms | Tokens / max pending |
|---|---:|---:|---:|---:|
| assignment128 | 93.84 [92.96, 107.96] | 14408 | 69.88 | 17778 / 35 |
| assignment512 | 229.72 [217.59, 370.32] | 34648 | 69.66 | 70770 / 35 |
| assignment2048 | 829.82 [816.90, 848.39] | 114180 | 69.69 | 282738 / 35 |
| angles128 | 54.98 [54.02, 55.61] | 13424 | 69.40 | 13042 / 12 |
| angles512 | 200.10 [193.55, 307.53] | 30844 | 69.40 | 51826 / 12 |
| angles2048 | 998.23 [843.14, 1258.28] | 100988 | 71.26 | 206962 / 12 |
| friend128 | 62.46 [55.69, 66.12] | 10616 | 72.84 | 9074 / 25 |
| friend512 | 221.67 [198.63, 246.19] | 20400 | 71.61 | 35954 / 25 |
| friend2048 | 843.08 [777.63, 853.49] | 58828 | 70.17 | 143474 / 25 |

Delimiter work equals produced tokens at every scale. Deferred cursor capacity
is bounded by the local declaration (35/12/25 tokens), independent of repetition
count. These are structural work observations, not a speed comparison against
rejecting entry code. All owner objects contain 377 text bytes; executable text
and hashes are also retained. Runtime loops take external seed `7`, perform
three million state transitions and check independently generated checksums.

compile launcher median 7.25 ms [6.96, 7.78].

runtime launcher median 5.17 ms [5.04, 5.35].

## Newly passing hosted inputs

PA30 requires object emission, not linking these fixtures. Runtime is therefore
covered by the executable controls and fixed/owner benchmarks above. The four
observations per hosted fixture have identical object hashes.

| Fixture | Compile median [range] ms | Peak RSS KiB | Object text bytes |
|---|---:|---:|---:|
| 500-reference-wrapper-smoke | 507.58 [317.62, 518.14] | 23524 | 86 |
| 600-hosted-forward-as-tuple-rvalue-ref-compile | 193.35 [188.47, 198.63] | 21068 | 423 |
| 600-hosted-forward-as-tuple-string-include-order | 817.37 [803.21, 824.36] | 50556 | 19156 |
| 700-hosted-adl-std-get-hidden-friend-compile | 180.28 [179.04, 183.80] | 19532 | 400 |
| 700-hosted-initializer-list-pair-layout-sync | 572.59 [563.38, 589.93] | 41328 | 7464 |
| 700-hosted-map-find-iterator-compile | 910.23 [900.42, 915.64] | 58688 | 22687 |
| 700-hosted-range-for-member-map-compile | 907.58 [890.93, 1150.86] | 58232 | 19485 |
| 700-hosted-std-bind-member-pointer-invocable-compile | 292.77 [291.51, 303.10] | 23576 | 102 |
| 700-hosted-tuple-rref-disabled-copy-ctor | 781.52 [771.61, 820.19] | 49116 | 18847 |

Maximum corrected hosted observation: 1.151 seconds, 58688 KiB. No timeout or budget relaxation.
