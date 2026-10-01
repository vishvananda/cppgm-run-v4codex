# Performance186 — accumulated PA29 audit

Acceptance is **PA29/O0**, under spec §9. Required semantics remain mandatory;
this audit adds no optional transform or speedup claim. All earlier measurements,
including preliminary and noisy observations, remain preserved.

## Frozen comparisons and noise protocol

The common comparison covers the **whole accumulated range**: A is the last
reviewed implementation `52070178` (the identical binary frozen at `a2ce2670`),
and B is `2df00585bd10d4e2e068934394dffc8adb0a47ed`. The owner comparison uses audit entry `152396e2`
versus the same B, covering guide declarations/defaults, runtime casts and live
93-bit arithmetic on the exact inherited 2,400-specialization inputs.

| Binary | SHA-256 |
|---|---|
| Reviewed A | `27de74d246ace237d351a2b4b2f59365be53651bff1b99c9d9bccfa829dcd648` |
| Entry A | `5bbd875bcbf039f6b7b57f79f38d6524c88cd53c8ccd1074d5e85d85b1faea68` |
| Final B | `a4022c8d9e1a288e70aa4010d1a5d574614d35575f319b56ed088865e0d3c785` |

Flags are `-O0 -c --stats`; CPU 0 affinity is fixed. Compiler and executable
wall time/peak RSS are measured separately with `/usr/bin/time`. Host linking,
initial output checks and Clang controls occur outside timing. Each equivalent
workload/mode has four A/A samples then six ABBA blocks. Every sample, outlier,
input/hash, binary/hash and compiler counter is retained. No build, test or
inspection job overlapped timing; external CPU contention remains uncontrolled.

[Common data](../student.tests/pa29/evidence186/common-performance.json) retains
**224 observations**; [owner data](../student.tests/pa29/evidence186/owner-performance.json)
retains **272**, plus **eight launcher samples**. All **eight** equivalent
object pairs and complete linked executables are byte-identical. Their workloads
exercise templates, loops, calls, memory, floating point, exceptions and dormant
bodies. Code/data/unwind identity rules out changed generated-code quality on
these comparisons, including the noisy exception timings below.

## All four dimensions

Ratios are medians of six paired block ratios, followed by their full range.
RSS is maximum compiler RSS; executable RSS and all individual samples are in
the JSON. Common rows use reviewed A; owner rows use entry A.

| Workload | Compile A/B s | B/A [range] | RSS A/B KiB | Runtime A/B s | B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1632/0.1651 | 1.0087 [0.8723–1.0179] | 29484/29756 | 0.0525/0.0523 | 1.0001 [0.9873–1.0043] | 151633/151633 |
| floating | 0.1651/0.1662 | 1.0044 [0.9982–1.0198] | 29524/29684 | 0.0495/0.0496 | 1.0046 [0.9949–1.0353] | 151474/151474 |
| exceptions | 0.1667/0.1689 | 1.0102 [0.6832–1.2532] | 29136/29364 | 0.2630/0.3549 | 1.0076 [1.0004–1.3129] | 151781/151781 |
| pruning | 0.2043/0.2067 | 1.0189 [0.9958–1.6877] | 35720/35756 | 0.0524/0.0523 | 0.9991 [0.9957–1.0015] | 151633/151633 |
| guides2400 | 0.2123/0.2120 | 0.9953 [0.9117–1.0050] | 36764/37048 | 0.1465/0.1465 | 1.0020 [0.9978–1.0037] | 591/591 |
| defaults2400 | 0.2215/0.2219 | 1.0005 [0.9974–1.1737] | 38464/38788 | 0.1470/0.1470 | 1.0009 [0.9955–1.0331] | 194645/194645 |
| runtime2400 | 0.1767/0.1757 | 0.9970 [0.9887–1.4784] | 32144/32320 | 0.0947/0.0948 | 1.0014 [0.9948–1.0088] | 113060/113060 |
| widths2400 | 0.2682/0.2671 | 0.9966 [0.8173–1.0055] | 35440/35792 | 0.5882/0.5875 | 0.9988 [0.9945–1.0057] | 1698/1698 |

| Workload | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| memory | 0.1654–0.3037 | 0.0523–0.0526 |
| floating | 0.1625–0.1707 | 0.0493–0.0496 |
| exceptions | 0.1630–0.1655 | 0.4379–0.4465 |
| pruning | 0.2048–0.2066 | 0.0522–0.0527 |
| guides2400 | 0.2116–0.2151 | 0.1458–0.1468 |
| defaults2400 | 0.2202–0.2239 | 0.1466–0.1474 |
| runtime2400 | 0.1746–0.2030 | 0.0942–0.0949 |
| widths2400 | 0.2663–0.4510 | 0.5875–0.5960 |

Compiler paired medians span **0.9953–1.0189**; every compiler paired range
crosses unity. All eight small peak-RSS increases are disclosed. Common exception
runtime has a 1.0076 paired median and an unpaired 0.2630/0.3549 s median split;
its A/A range and retained outliers expose substantial scheduling variation.
The files executed are byte-identical, so these timings do not indicate changed
exception code. The pruning 1.6877 compiler block and final inquiries2400 0.6965 s
outlier remain in the evidence. No repeatable avoidable compiler regression or
speedup is established by these measurements.

## Corrected-only defaults and checked overflow

Entry rejects these inputs, so a rejection-to-success latency ratio would be
invalid. Each of N source assertions executes its constexpr specialization twice
using `decltype(parameter)` in the default. A separate executable reads argv seed
7 and performs **60,000** varying 93-bit multiply/add steps with both overflow
flags and remainders contributing to independently calculated Python checksums.
Clang-built controls and the final compiler agree. Eight compiler and eight
executable observations are retained per size.

| N | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---:|---:|---:|---:|---:|---:|
| 600 | 0.0593 [0.0576–0.0615] | 13636 | 0.0664 [0.0661–0.0681] | 1756 | 4835 |
| 1200 | 0.1090 [0.1078–0.1107] | 19836 | 0.0666 [0.0664–0.0673] | 1764 | 4835 |
| 2400 | 0.2084 [0.2078–0.6965] | 32256 | 0.0664 [0.0662–0.0670] | 1764 | 4835 |

Launcher median is **0.0047 s**, range **0.0044–0.0055 s**. Checked loops dominate
startup. Objects and executables are identical across all three sizes.
[Scaling assertions](../student.tests/pa29/evidence186/scaling.json) verify all
**24** compilations: parsed nodes = 23N+323, source/projected occurrences =
67N+323, N specializations/body transitions/default facts/default demands and
N repeated-execution cache hits. There is one source default binding. Type-query
work = N+3; value-query work = N+1. Source and prepared IR remain **187 instructions
/ 314 operands**; native instructions remain **805** and text **4,835 bytes**.
Required frontend work scales with demand; the constexpr-only bodies are not
emitted. These counters are not a runtime-profit claim.

## Legality, budgets and inherited acceptance

The overflow correction normalizes the stored value to the destination's exact
precision before comparing it to the mathematical result. It adds at most two
shifts per affected intrinsic, preserving volatile stores and existing argument
evaluation. It is necessary semantic work, not an optional optimization. The
existing type-qualified normalized-value proof remains transient and is cleared
by new operations; no extra fixed point, runtime call or code multiplier is added.
Default inquiry substitution uses source/query and complete frame identity.
Its validation visits reachable syntax/query edges; temporary flat worklists and
visited indexes are released after the check. Facts retain TU lifetime.

Existing constexpr (1,000,000 execution steps), object/frame/data/alignment and
scalar-width limits remain enforced. Mandatory inline budgets remain **64**
levels, **262,144** reserved units/caller and **4,194,304**/program, with conservative
calls on unsupported or exhausted expansion. This range adds no optional pass,
work allowance or code-growth allowance. Integrated MIR shows actual frame homes:
`main` **304 bytes**, `step<7>` **256**, `step<93>` **592**. No better register
allocation or runtime efficiency is inferred from IR counts.

[Historical review](../student.tests/pa29/evidence186/historical-review.json)
verifies **1,144 inherited observations plus 24 launchers** across handoffs
183–185, including 183's noisy first run and affinity confirmation. Binary,
input, evidence and record hashes are checked; [source binding](../student.tests/pa29/evidence186/historical-source-binding.json)
checks each validated implementation against its commit. Prior audits and all
performance182-and-earlier evidence remain retained.

Inherited blanket 15% compiler-latency/RSS and zero-growth targets remain
**diagnostic under spec §9**, as already classified by earlier audits. No mandated
limit, correctness condition, coverage or comparison rule is weakened. Necessary
semantic cost and later-stage broad hosted runtime, optimizer and self-hosting
work (PA30/31, PA32/33, PA34) do not add PA29 exit gates. No reference correction
is made and no failing test is waived.

Reproduce with the frozen binary paths in the evidence:

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py OUT/common REVIEWED FINAL
PERF_CPU=0 python3 student.tests/pa29/performance186.py OUT/owner ENTRY FINAL
python3 student.tests/pa29/analyze186.py
```
