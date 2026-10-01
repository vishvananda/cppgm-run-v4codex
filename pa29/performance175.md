# Performance175 — PA29 / O0

This change implements source-invocation semantics. No optional optimizer pass or
speedup is claimed. Spec §9 requires correct equivalent comparisons, necessary
semantic costs to be disclosed, and unprofitable optional work to be removed.
There is no valid entry/runtime ratio for the new builtins: the entry compiler
rejects every affected input. The entry failure records remain in the evidence.

## Frozen protocol and equivalent workloads

The [manifest](../student.tests/pa29/evidence175/manifest.json) freezes entry,
preliminary and final binaries. Final code is `704538c1`; compiler SHA-256 is
`3305fd803dccd1ebf63ec54e8ed3f7cf20b97ed1ececebb8e866858b4027bb0f`.
[Common raw samples](../student.tests/pa29/evidence175/common-performance.json)
retain input hashes, flags, host linker version, executable/object hashes, all
phase counters and observations. Each workload has an AAAA calibration and six
ABBA wall-time blocks for compilation and runtime separately (224 observations).
Compiler flags are `-O0 -c --stats`; the inherited benchmark combines 2,400 demanded
templates with checked memory/loop, floating, exception or unused-function work.
Executables validate their results. No hardware counter dependency or affinity
pinning was added; outliers remain included in the paired ranges.

| Workload | Compile A/B median s | Paired B/A median [range] | Compiler peak RSS A/B KiB | Runtime A/B median s | Runtime paired B/A [range] | Executable text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1726/0.1649 | 0.9635 [0.9007–1.0825] | 29260/29784 | 0.0512/0.0514 | 1.0137 [0.9902–1.0701] | 151633/151633 |
| floating | 0.1638/0.1595 | 0.9668 [0.6610–0.9994] | 29472/29740 | 0.0478/0.0475 | 0.9964 [0.9899–1.0127] | 151474/151474 |
| exceptions | 0.1614/0.1617 | 0.9938 [0.5467–1.0804] | 29548/29708 | 0.2513/0.2508 | 0.9912 [0.9687–1.0051] | 151781/151781 |
| pruning | 0.1993/0.1977 | 0.9917 [0.9742–1.4698] | 34952/35368 | 0.0514/0.0511 | 0.9939 [0.9854–1.0031] | 151633/151633 |

All four text sizes are unchanged. These observations do not establish an
avoidable compiler or executable regression. Peak compiler RSS grows by at most
1.8% here, including required query/source-context representation. The small
runtime differences occur with identical text sizes; no runtime profit is
attributed to this frontend change. Initial observations are preserved separately,
not discarded as failed gates or replaced by selected favorable samples.

| Workload | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| memory | 0.1572–0.1654 | 0.0523–0.0531 |
| floating | 0.1631–0.1822 | 0.0477–0.0485 |
| exceptions | 0.1599–0.1698 | 0.2499–0.2551 |
| pruning | 0.1960–0.2031 | 0.0507–0.0516 |

## Required-semantics scaling

[All affected samples](../student.tests/pa29/evidence175/source-performance.json)
retain eight compiler and eight runtime observations at each size (48 total).
`performance175.py` deterministically produces 600/1200/2400 separate calls to
demanded template specializations. Every call consumes a source-location default
and a runtime seed. `strtol(argv[1])`, 24 million call evaluations and a printed
checksum prevent measuring a dead or constant-folded workload. Each executable
is checked against a separately built host program before timing; every timed
run rechecks the same output. Flags are `-std=c++11 -O0 -c --stats`, runtime seed
is `17`, and compilation and execution are measured separately.

| Calls | Compile median [range] s | Compiler peak RSS KiB | Runtime median [range] s | Runtime peak RSS KiB | Executable text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | 0.0639 [0.0625–0.0744] | 14952 | 0.1309 [0.1284–0.1353] | 1656 | 46360 |
| 1200 | 0.1183 [0.1159–0.1237] | 23820 | 0.1303 [0.1291–0.1313] | 1916 | 92560 |
| 2400 | 0.2528 [0.2332–0.3930] | 38784 | 0.4354 [0.4300–0.4446] | 1960 | 184960 |

Launcher median is **0.00313 s**, range
**0.00294–0.00324 s**. Even the shortest affected compiler median
is over 20× launcher median; runtime medians exceed 40×. The 2400-function runtime
is materially slower at the same total call count; the preliminary correct
implementation has the same text and the same observed increase. This is
reported as an O0 generated-code limitation, not hidden behind IR size or a
favorable compiler timing. No additional inlining/allocation pass was introduced.

[All-sample counter assertions](../student.tests/pa29/evidence175/scaling.json)
show **2N+12 sites**, **N specializations**, **N+2 body checks**, **N+11 type queries**,
**N+1 value queries**, and **15N+253 parsed nodes** at all three sizes, in all eight
compiler samples. Native text is **77N+160 bytes**. Work and storage follow actual
source, specialization and call edges. No discarded/reparsed template body or
whole-program retry is added.

## Demand correction, history and acceptance

The preliminary correct implementation allocated file/function support objects
for line-only calls. The final implementation retains compact text identities,
reserving and emitting bytes only when a runtime address is consumed. Explicit
inspection verifies zero support strings for line-only code and constexpr-only
character reads. This fixes a spec demand defect, without adding an optional
optimization. The affected object files shrink by **224 bytes** at each measured
size; executable text is unchanged. Preliminary timings are not paired with the
final timings, so they support no speedup claim.

[Preliminary common](../student.tests/pa29/evidence175/preliminary-common.json) and
[preliminary affected](../student.tests/pa29/evidence175/preliminary-source.json)
measurements retain all 272 observations and the original launcher samples.
Historical [performance174](performance174.md) remains unchanged. Unsupported
blanket **15% latency/RSS** and **zero-growth** targets remain diagnostic,
not additional PA29/O0 gates. Correctness, coverage and mandated limits remain.

Optional transform work and code-growth budgets are **zero**. Source-site work is
linear in actual call/initialization facts; string emission is linear in demanded
bytes. The **2^31−1** source-site capacity protects packed query-context keys.
Existing bounds remain **1,048,576** generated elements, **1,000,000** constexpr
steps and **512** call depth, native frame/data **0x70000000**, alignment **4096**,
and course timeouts. Required semantic costs are distinguished from the optimizer,
allocator, heavy hosted-runtime and self-hosting work owned by PA30–34.

Reproduction (use the manifest's frozen compiler hashes):

```sh
python3 student.tests/pa27/performance147_common.py OUT_COMMON ENTRY FINAL
python3 student.tests/pa29/performance175.py OUT_SOURCE ENTRY FINAL
python3 student.tests/pa29/test175.py OUT_CONTROLS FINAL
python3 student.tests/pa29/inspect175.py OUT_INSPECTION
python3 student.tests/pa29/validate175.py OUT_GATES
```
