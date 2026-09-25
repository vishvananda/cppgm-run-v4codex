# PA18 loop 73 performance evidence

Frozen entry `12cbfe83` and implementation `9ebc507c`, both built with
`g++ -std=gnu++11 -Wall -O3` and the test runner enabled. The same binaries
are used throughout. Compiler commands emit PA18/O0 LowIR; validation and
telemetry run separately from timing. CPU affinity, flags, binary/input/output
hashes, every warmup and timing/RSS observation are retained in the
[main observations](../student.tests/pa18/loop73-performance.json) and
[focused observations](../student.tests/pa18/loop73-performance-focused.json).

Protocol: one warmup per binary, four A/A samples, four ABBA blocks for equivalent
correct inputs; six final-only samples when the entry rejects newly required
behavior. The [harness](../student.tests/pa18/benchmark73.py) adds `--focused`
for the final using-hiding and indirect-query workloads. The initial harness is
frozen at `/tmp/pa18-loop73/benchmark73-first.py`; its hash is preserved with the
first observations and in the handoff manifest. No observation was overwritten.
No build or validation suite ran concurrently with either measurement batch.

## Compiler latency and peak RSS

| Workload | A / B median ms | Paired B/A median (range) | A / B peak RSS KiB |
|---|---:|---:|---:|
| ordering-600 | 57.05 / 56.08 | 0.984 (0.970–0.991) | 15248 / 15252 |
| ordering-2400 | 213.86 / 211.99 | 0.991 (0.988–1.097) | 43200 / 43564 |
| member-head-600 | 97.73 / 96.85 | 0.997 (0.979–1.107) | 22844 / 22372 |
| member-head-2400 | 396.56 / 397.38 | 0.996 (0.994–1.007) | 73052 / 74000 |
| common-loop-float-1500 | 155.18 / 155.02 | 0.990 (0.738–1.004) | 30420 / 30296 |
| runtime-calls | 5.82 / 5.84 | 0.997 (0.976–1.026) | 5772 / 5936 |
| runtime-memory | 6.16 / 6.07 | 0.989 (0.965–0.999) | 5704 / 5880 |
| runtime-floating | 5.93 / 6.02 | 0.999 (0.981–1.034) | 5956 / 6088 |
| invocation-600 | — / 133.91 | new behavior | — / 26496 |
| prototype-this-600 | — / 89.57 | new behavior | — / 20504 |
| invocation-2400 | — / 547.28 | new behavior | — / 89136 |
| prototype-this-2400 | — / 361.72 | new behavior | — / 64568 |
| runtime-invocation | — / 5.94 | new behavior | — / 5812 |
| using-hiding-600 | 123.80 / 120.44 | 0.972 (0.894–0.977) | 26868 / 25784 |
| indirect-query-600 | 47.76 / 47.89 | 1.004 (0.986–1.038) | 13240 / 13616 |
| using-hiding-2400 | 518.41 / 504.03 | 0.976 (0.970–0.983) | 89100 / 85756 |
| indirect-query-2400 | 184.45 / 183.06 | 0.994 (0.906–1.004) | 36308 / 36428 |

Empty-input startup medians are **5.55 / 5.58 ms**. Small runtime-source compile
times are startup-limited; scaled frontend workloads dominate startup. Every
common workload produces byte-identical LowIR, and common executable hashes
are identical. There is no claimed general compiler speedup. Common medians
are near parity; maximum observed RSS increase is 948 KiB on member-head-2400.

Using-hiding has paired medians **2.8% / 2.4% lower** at 600 / 2400 classes,
with RSS lower by 1,084 / 3,344 KiB. This is required declaration hiding, with
half as many call candidates and deduced specializations. Normalizing the
signature adds substitution work, but avoids forming the hidden candidates.
Indirect-query medians remain near parity; retaining function-decay conversions
costs 376 / 120 KiB peak RSS in this corpus. These are semantic facts, not an
optional optimization policy.

All spreads remain visible: using-hiding-600 A/A ranges from 121.4 to 189.9 ms;
the common-loop paired range includes 0.738, and ordering/member-head blocks
include ratios above 1.09. These scheduling outliers are preserved, not removed
or used as evidence of a broad gain.

Compiler `.text`: **1,884,742 → 1,892,934 bytes**, **+8,192 bytes (0.435%)**.
No equivalent generated executable grows.

## Checked executable runtime and size

The supplied native backend is pinned to bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, invoked separately with `-O0`.
Volatile runtime inputs, loops and checked results prevent timing folded/dead
work. Every execution returns zero.

| Workload | A / B median seconds | Paired B/A median (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 0.716462 / 0.717604 | 1.004 (1.000–1.007) | 206 / 206 |
| runtime-memory | 0.418087 / 0.417966 | 1.000 (0.997–1.005) | 434 / 434 |
| runtime-floating | 0.494691 / 0.494604 | 1.000 (0.994–1.011) | 230 / 230 |
| runtime-invocation | — / 0.121334 | new behavior | — / 191 |

The backend emits sectionless ELF; payload is a code-size proxy for these
sources without static data, not a claimed `.text` section. Common executables
are byte-identical, so their small runtime variation is environmental. No
runtime optimization benefit or regression is attributed to this frontend work.
The invocation workload has no correct entry executable for the same input.

## Graph work and stage acceptance

| Family, 600 → 2400 | Substitution work | Frames | Type queries | Signature shapes |
|---|---:|---:|---:|---:|
| Invocation | 8412 → 33612 | 5405 → 21605 | 2416 → 9616 | 5 → 5 |
| Prototype `this` | 7808 → 31208 | 4800 → 19200 | 1807 → 7207 | 13 → 13 |
| Using-hiding, final | 13806 → 55206 | 6000 → 24000 | 603 → 2403 | 7 → 7 |
| Indirect query, final | 5403 → 21603 | 2400 → 9600 | 3606 → 14406 | 3 → 3 |

Fourfold source growth produces approximately fourfold work and compiler time:
invocation **4.09×**, prototype `this` **4.04×**, using-hiding **4.18×**, indirect
queries **3.82×**. Prototype object-context counts stay constant (one/two for
the two new families), reflecting shared source facts. Hiding halves candidates
from 1200/4800 to 600/2400; extra normalized shapes are constant in this corpus.
The separate completion controls retain exactly one invalidation each at
32/128/512 dependent classes. No unrelated declaration scan or global retry is
introduced.

Acceptance is **PA18/O0 LowIR**, spec §9. No mandated numerical compiler/runtime
ceiling or optional transform was introduced. Correctness, unchanged coverage
and graph bounds remain gates. Historical **+15%, +16 MiB, 5.5×** targets remain
diagnostics with all prior measurements preserved; they are not extra exit
gates. Native optimization and self-hosting remain PA24–PA34 responsibilities.
