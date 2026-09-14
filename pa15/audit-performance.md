# PA15 checkpoint performance audit

Final compiler source: `034e3b91`; reviewed code/tooling tip: `538cfcb0` (identical `dev` tree).
Compare the full stage against frozen `8000f3c8`, and the audit fixes against frozen
entry `db0686a3` / compiler `992b1170`. Both use g++ C++11, `-Wall -O3`,
`TEST_RUNNER_ENABLE`; measured compiler flags are `--emit-lowir -O0`.
The supplied native backend runs at `-O0`; it is not a student native compiler.

[Reproducer](../student.tests/pa15/audit_benchmark.py) and
[verifier](../student.tests/pa15/verify_checkpoint.py) retain binary, source, output,
backend and harness SHA-256 identities. Each campaign pins CPU 31, validates and
compares outputs before timing, warms both binaries, then runs four A/A observations
and two wall-time ABBA blocks. New supported behavior has six absolute final-binary
observations because the entry compiler rejects it. No builds or correctness suites
ran during timing. Timed output hashes equal the separate audit/stats preflight,
so telemetry and full validation are excluded from ordinary compiler measurements.

The four new campaigns retain **896 invocations**, including warmups; the seven
historical campaigns retain **1,484**, for **2,380 hash-verified invocations**.
No sample or earlier miss was discarded. Exact campaign hashes and paths are in
[the evidence ledger](../student.tests/pa15/checkpoint-evidence.json).

- [stage-performance](/home/vishvananda/work/private/v4codex/artifacts/pa15-audit/stage-performance.json): initial audit binary `e211b3f8`.
- [checkpoint-performance](/home/vishvananda/work/private/v4codex/artifacts/pa15-audit/checkpoint-performance.json): initial audit binary `e211b3f8`.
- [stage-repeat-performance](/home/vishvananda/work/private/v4codex/artifacts/pa15-audit/stage-repeat-performance.json): final compiler source `034e3b91`.
- [checkpoint-repeat-performance](/home/vishvananda/work/private/v4codex/artifacts/pa15-audit/checkpoint-repeat-performance.json): final compiler source `034e3b91`.

## Compiler latency and memory

Final campaigns below show medians over the timed sequence and maximum observed
process peak RSS in KiB. A/A is the observed wall-time range, not a confidence
interval. Both ABBA paired ratios are retained. Small runtime-source compilations
are about 6 ms and are recorded in JSON; they support no compiler latency claim.

Full accumulated stage, semantically equivalent common inputs:

| Workload | Base / final ms | Base / final peak RSS KiB | Paired final/base | A/A ms |
|---|---:|---:|---|---|
| types-1000 | 70.934 / 72.706 | 17276 / 17548 | 1.0058, 1.0243 | 70.7–73.3 |
| wide-signatures-1000 | 216.257 / 218.089 | 49844 / 50380 | 1.0021, 1.0086 | 213.0–216.8 |
| types-4000 | 279.028 / 286.028 | 53384 / 53896 | 1.0099, 1.0220 | 276.0–279.1 |
| wide-signatures-4000 | 894.189 / 896.430 | 188964 / 190720 | 1.0573, 1.0056 | 892.0–998.4 |
| constants | 226.190 / 225.752 | 34852 / 35132 | 1.0032, 0.9909 | 225.5–228.7 |

Audit-entry comparison for supported and newly supported PA15 behavior:

| Workload | Entry / final ms | Entry / final peak RSS KiB | Paired final/entry | A/A ms |
|---|---:|---:|---|---|
| defaults-1000 | 55.829 / 55.136 | 12560 / 12508 | 0.9819, 1.0034 | 55.4–56.2 |
| values-1000 | 56.029 / 55.991 | 13508 / 13640 | 0.8611, 0.9875 | 55.7–56.3 |
| selected-classes-1000 | 53.950 / 52.811 | 12216 / 12248 | 0.9600, 0.9856 | 52.8–54.2 |
| nested-packs-1000 | 277.691 / 276.173 | 45876 / 45768 | 0.9427, 0.9934 | 276.7–278.2 |
| pack-targets-1000 | unsupported / 135.977 | — / 26988 | absolute only | — |
| pack-selections-1000 | unsupported / 59.870 | — / 12632 | absolute only | — |
| wide-signatures-1000 | 228.324 / 216.131 | 50996 / 50280 | 0.7896, 0.9417 | 226.9–231.0 |
| defaults-4000 | 215.939 / 213.243 | 34212 / 34388 | 0.9879, 0.9708 | 212.4–217.3 |
| values-4000 | 218.534 / 218.867 | 38880 / 38884 | 0.9964, 1.0112 | 218.6–221.0 |
| selected-classes-4000 | 202.130 / 204.124 | 32996 / 32976 | 0.9890, 1.0491 | 200.4–203.5 |
| nested-packs-4000 | 1216.599 / 1209.257 | 166484 / 166420 | 1.0019, 0.9951 | 1189.0–1232.8 |
| pack-targets-4000 | unsupported / 557.826 | — / 90272 | absolute only | — |
| pack-selections-4000 | unsupported / 227.431 | — / 34832 | absolute only | — |
| wide-signatures-4000 | 948.207 / 898.088 | 186512 / 190692 | 0.9565, 0.9056 | 941.4–962.9 |

The first cumulative campaign exposed wide-signature costs of 216.106/227.007 ms
at N=1,000 and 908.403/958.611 ms at N=4,000. Its second size had opposing
paired ratios 1.1492 and 0.9803. Removing temporary expansion vectors, scalar
expansion dispatch and repeated body pack scans brings final cumulative medians
near the base. Against audit entry, the final wide-signature medians decrease
about **5.3%** at both sizes. All four final-entry blocks favor the repair, but
the 0.7896 block includes a slow entry observation; do not interpret it as a
21% speedup. A/A and complete sample sequences remain available above.

The final cumulative scalar-type workload is roughly **2.5%** higher in median
latency than the stage base; paired ratios are 1.0058–1.0243. The new argument-kind,
normalization and expanded-occurrence checks implement required semantics. Source
review removed the avoidable parameter work; there is no optional transform to
retain for an unsupported benefit. Constants are near baseline. Entry comparisons
for values, defaults, classes and nested packs do not establish a new repeatable
regression. Individual blocks remain noisy (including class-4000 at 1.0491).

Peak memory costs are disclosed rather than traded against extra passing tests.
The cumulative large wide-signature peaks are 188,964/190,720 KiB (+0.93%); the
entry repeat has 186,512/190,692 KiB (+2.24%), versus 189,984/189,912 in the first
audit comparison. All measurements remain preserved. Compiler `.text` is
**1,424,966 → 1,495,558 bytes** across PA15 (+70,592, 4.95%), and
**1,494,470 → 1,495,558** for the audit (+1,088, 0.073%). The initial audit
binary had 1,494,598 bytes. No common generated LowIR or native code grows.

## Runtime, text and work bounds

Loops consume volatile runtime bounds and verify call, memory and floating-point
results. Common inputs have exact LowIR and native-byte identity. Native text is
the payload after ELF entry because the supplied backend emits sectionless ELF;
these executable sources contain no static data. Runtime peak RSS is 256 KiB.

| Runtime workload | Entry / final ms | Native text bytes | Paired final/entry | A/A ms |
|---|---:|---:|---|---|
| runtime-calls | 300.314 / 300.298 | 206 / 206 | 0.9913, 1.0023 | 300.1–301.2 |
| runtime-memory | 176.202 / 177.499 | 434 / 434 | 1.0011, 0.9988 | 175.8–176.4 |
| runtime-floating | 208.767 / 209.475 | 230 / 230 | 1.0025, 1.0023 | 208.1–209.1 |
| runtime-pack | 312.052 / 312.567 | 217 / 217 | 0.9883, 1.0157 | 309.4–325.8 |
| runtime-selected-pack | unsupported / 327.849 | 218 | absolute only | — |

Both cumulative campaigns also execute the three common runtime families with
identical native bytes (206/434/230 bytes). No runtime optimization benefit is
claimed from timing noise on identical programs. The newly correct specialized
pack runtime is reported absolutely: its ordinary O0 forwarding function and
strong explicit-specialization identity are required behavior.

At N=1,000 and 4,000, current counters verify:

- Nested packs: nine source regions, 18 discovery visits, 7N lanes, 13N+11 frames,
  3N body transitions. Final fourfold source growth costs 4.38× median latency.
- Target pack signatures: three source regions, one discovery visit, 2N lanes,
  N body transitions; fourfold growth costs 4.10× latency.
- Explicit value-pack selections: N selections, zero primary body transitions;
  separate prefix and complete records yield 2N specialization records. Fourfold
  growth costs 3.80× latency.
- Value reuse: N bodies; defaults: N class completions with zero member bodies;
  explicit class selection: N selections with zero primary completions.

These source/key counts and output sizes support work proportional to actual
arguments, demanded facts and produced IR. There is no fixed pack-partition cap.
The separate 64-partition control reuses each distinct flattened partition twice.

## Stage acceptance and inherited targets

PA15 requires O0 LowIR and provides **no numerical latency, RSS or text ceiling**.
It adds no optional optimization or search/growth policy. Correctness costs and
bounded representation growth above are disclosed; the avoidable scalar costs
were repaired. The 11 required fixture failures remain implementation work and
are not waived by any performance measurement.

Historical numerical targets are diagnostic under spec §9. For example, the
[PA14 source-read review budget](../pa14/performance.md) described 64 KiB of
compiler text for that local change, explicitly not a retroactive stage gate.
It does not constrain the accumulated PA15 semantic implementation. Its no-allocation
and zero-target-growth requirements for the source-read operation remain intact;
all historical measurements and mandated language/complexity limits are preserved.
The earlier PA15 handoffs already used this stage-scoped classification and are
confirmed by this audit, not silently replaced by new numerical targets.

Native allocation/optimization, direct ELF emission and self-hosting are required
at their later owning stages. PA15 cannot produce a valid self-host benchmark or
claim student-backend spill/encoding quality yet. The supplied-backend controls
establish source-generated LowIR execution and code size at the current boundary.
No missing future-stage measurement is made an additional PA15 exit gate.
