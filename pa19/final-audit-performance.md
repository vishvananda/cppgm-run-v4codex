# PA19 final audit performance (93)

**Spec Alignment: aligned for PA19/O0.** This audit compares the full stage,
from `e5f4c3ed` to reviewed implementation `228d7fc7`. Compiler sources and the
final binary are unchanged by the audit. No compiler or executable speedup is
claimed. Seven shared workloads have byte-identical LowIR and native output;
two newly supported workloads have final-only costs.

## Frozen experiment

[Runner](../student.tests/pa19/audit93_benchmark.py),
[all raw observations](../student.tests/pa19/audit93-performance.json).
A SHA-256: `0f422d8c85e5da91c8f89188718cfb6394bb880a1cde06cf5183d35ca3d026ca`;
B: `1a181c95ee97d646d41c1009780200049e776d67e1d715840d5526b3d10e1d40`.
The runner asserts those hashes against the retained stage-entry/final evidence,
freezes the backend hash, and checks every source against its previous hash.
Build flags: `g++ -std=gnu++11 -Wall -O3`, `TEST_RUNNER_ENABLE`.
Invocations: `--emit-lowir -O0`; supplied backend: `-O0`, pinned bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.

CPU 31; one warmup per binary, four A/A observations, four wall-time ABBA blocks
for each comparison. New behavior has one B warmup and six B observations.
Compilation and execution are timed separately. Instrumented validation runs
are separate from timings. Every native program must return zero. There are
**324 observations and 34 warmups**, including compiler startup calibration.
No observation was removed. Empty compiler medians are A 9.9 ms and B 9.3 ms;
the A range reaches 27.8 ms. Tiny compiles and frontend executables are
startup-dominated and do not establish a speedup.

## Compiler latency, memory and size

Times are milliseconds; RSS is peak KiB. Ratios are medians of the four paired
block ratios, followed by their full range. Absolute medians need not have the
same ratio when machine speed changes during a run.

| Input | A → B ms | B range | A/A range | Paired B/A (range) | RSS A → B |
|---|---:|---:|---:|---:|---:|
| ordinary-ordering-2400 | 374.9 → 389.5 | 343.0–430.5 | 372.8–815.6 | 1.032 (0.968–1.109) | 40080 → 40492 |
| ordinary-ordering-9600 | 1542.7 → 1685.9 | 1480.1–1925.3 | 1681.2–1863.0 | 0.995 (0.930–1.197) | 144036 → 143768 |
| namespace-variable-2400 | 297.5 → 297.9 | 278.7–345.4 | 287.3–301.4 | 0.971 (0.921–1.021) | 24480 → 24756 |
| namespace-variable-9600 | 1172.3 → 1180.4 | 1047.2–1375.2 | 1140.8–1290.9 | 1.028 (0.989–1.069) | 79472 → 81800 |
| member-variable-2400 | — → 766.7 | 746.1–871.5 | — | final only | — → 67208 |
| member-variable-9600 | — → 1726.8 | 1632.5–1901.9 | — | final only | — → 254648 |
| runtime-calls | 6.6 → 6.6 | 6.3–6.8 | 6.6–6.9 | 0.979 (0.973–1.017) | 5988 → 6084 |
| runtime-memory | 6.9 → 6.8 | 6.7–7.1 | 7.0–7.4 | 0.980 (0.964–1.002) | 5984 → 6060 |
| runtime-floating | 8.8 → 8.6 | 6.9–8.8 | 6.7–7.1 | 0.960 (0.938–0.975) | 6032 → 6248 |

Compiler `.text`: **2,000,006 → 2,005,958 bytes**, +5,952 (+0.298%).
The largest namespace-variable case adds 9,600 initialization conversions and
9,600 type probes for 9,600 values. Parsed nodes (326,447), query work (28,803),
query edges (19,202) and frames (19,200) are unchanged. The second value request
now reaches the entity's completed constant directly; the old initializer-reuse
counter consequently decreases without duplicated evaluation. This is required
initialization checking, including rejection of invalid/deleted/explicit-only
conversions. Its largest paired latency change is +2.8% here, versus +5.5% in
handoff 92; RSS increases 2,328 KiB. The source audit found no extra global pass,
search or retry to remove.

Member-variable 2,400 → 9,600 keys: initializers 2,400 → 9,600; query work
9,605 → 38,405; edges 9,604 → 38,404; frames 24,000 → 96,000; nodes
388,887 → 1,555,287. Each input repeats its assertion and contains an invalid
dormant initializer. The value is computed once per key and the dormant
initializer stays undemanded. RSS grows 3.79×. Wall time grows 2.25× in this run;
neither that ratio nor earlier superlinear timings alone prove complexity.
Source ownership and actual work counts provide the relevant bound.

## Executable runtime and size

All three runtime loops have volatile bounds, live computation and checked
results. They cover calls, integer loops, memory and floating point. A/A noise
and complete ABBA spreads are retained, including a slow A calls observation
that yields the 0.566 block ratio. Identical native bytes and the remaining
spread provide no basis for a generated-code speedup claim.

| Input | A → B ms | B range | A/A range | Paired B/A (range) | Payload A = B |
|---|---:|---:|---:|---:|---:|
| runtime-calls | 124.4 → 125.4 | 123.1–127.5 | 123.7–124.6 | 1.003 (0.566–1.021) | 206 |
| runtime-memory | 74.2 → 74.7 | 72.7–101.5 | 73.3–75.6 | 1.000 (0.988–1.188) | 434 |
| runtime-floating | 86.7 → 87.1 | 86.3–88.9 | 86.1–99.8 | 1.004 (1.000–1.013) | 230 |

The supplied backend emits sectionless ELF. Payload is the executable segment
after its entry, **including static data**, not an exact `.text` section.
Frontend ordering payloads are 134,505 and 537,705 bytes; namespace/member
variable payloads are 24 bytes. Their checked executions have B medians
3.2–7.8 ms and are startup measurements only. All runtime observations, memory
measurements and payload/file sizes remain in the raw record.

## Acceptance and retained evidence

The handoff [91](performance91.md) and [92](performance92.md) measurements,
including incomplete attempts, remain intact. Their broader fixed corpus adds
member aliases, dependent function results, defaulted packs, empty-tail ordering,
class-valued member variables and reference identity. Their final compiler hash
matches this audit's B where applicable. Those records show new semantic costs
separately from comparisons of correct shared behavior. They also retain the
largest-input latency variation and the earlier index profile; no historical
miss is discarded or relabeled as a measured win.

PA19/O0 has no mandated numerical latency, RSS, runtime or code-growth ceiling.
The inherited +15%, +16 MiB and 5.5× thresholds are diagnostic targets under
spec §9, as already classified by PA18. They are not extra exit gates. This
classification preserves measurements, required behavior, coverage, algorithmic
bounds and any later mandated limits. Necessary conversion checks are a measured
semantic cost; no unprofitable optional transform was added in PA19. The audit
reviewed existing bounded forwarding and constant-array lowering separately in
[the architecture record](audit.md). Native allocation/optimization, debug
encoding and self-hosting are owned by PA24/PA32–34 and are not claimed here.
