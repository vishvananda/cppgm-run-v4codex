# PA18 checkpoint audit 82 performance evidence

Acceptance is **PA18/O0 LowIR**, spec §9. The audit compares frozen A, the
previous reviewed code `82fca940`, with frozen B, audited code `ecc308bc`.
The [raw observations](../student.tests/pa18/loop82-performance.json) preserve
all input text/hashes, binary/backend hashes, flags, telemetry, warmups and
samples. The [harness](../student.tests/pa18/benchmark82.py) is committed with
the code tip. All **33** workloads from handoffs 79–81 are included, plus **seven**
probes of the audit's lookup/query fixes. Historical observations, including
81's failed preflight, intermediate measurements and confirmation, remain intact.

## Protocol and scope

Build flags: `g++ -std=gnu++11 -Wall -O3`, course test runner enabled. Timed
compiler flags: `--emit-lowir -O0`. Separate preflights add `--validate-lowir
--stats`. The harness pins one CPU, warms both binaries, records four A/A samples
and four ABBA blocks per equivalent workload. New behavior uses six final-only
samples; a rejected baseline compilation is never treated as faster correct
work. Compilation, supplied-backend construction and program execution are
separate. Every executable preflight and timed run checks its result.

There are **20 comparable workloads** (11 exact LowIR, nine executed-equivalent)
and **20 newly supported workloads**. Thirty-nine sources include checked native
preflights; the common 1500-function compiler corpus has no `main` and retains
exact LowIR. Runtime inputs use volatile bounds with checked data-dependent
loops, calls, memory and floating arithmetic. The affected result loops perform
24 million iterations. Neither constant-folded nor dead work is timed.

The supplied O0 backend is bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
It is used only by the explicitly invoked harness. Compiler size is actual ELF
`.text`; the sectionless supplied executables use payload bytes as a code-size
proxy, including any static data. Own native optimization, register allocation,
debug encoding and self-hosting remain later-stage obligations.

## Compiler latency and peak RSS

Medians are milliseconds. Ratios are medians of within-block mean B/A wall
times, followed by the full four-block range. RSS is maximum observed KiB.
All observations, including outliers and A/A noise ranges, remain in the raw file.

| Workload | A / B ms | Paired B/A (range) | A / B peak KiB |
|---|---:|---:|---:|
| ordering-600 | 54.83 / 55.62 | 1.005 (1.000–1.008) | 15140 / 15332 |
| ordering-2400 | 213.86 / 216.41 | 1.011 (1.007–1.097) | 43816 / 43904 |
| member-head-600 | 98.19 / 92.33 | 0.945 (0.936–0.953) | 22900 / 21592 |
| member-head-2400 | 392.57 / 371.19 | 0.941 (0.873–0.982) | 73504 / 67656 |
| common-loop-float-1500 | 153.46 / 156.08 | 1.007 (0.996–1.093) | 30232 / 30436 |
| runtime-calls | 6.02 / 5.89 | 0.970 (0.957–0.984) | 5932 / 6036 |
| runtime-memory | 6.31 / 6.07 | 0.959 (0.937–0.980) | 5916 / 6000 |
| runtime-floating | 6.13 / 5.93 | 0.972 (0.964–0.983) | 6080 / 6152 |
| first-signature-600 | — / 66.27 | new behavior | — / 13916 |
| first-member-600 | — / 132.77 | new behavior | — / 27628 |
| qualified-alias-600 | 67.11 / 66.56 | 0.988 (0.978–0.992) | 15984 / 16132 |
| first-signature-2400 | — / 254.92 | new behavior | — / 38172 |
| first-member-2400 | — / 573.34 | new behavior | — / 95500 |
| qualified-alias-2400 | 264.69 / 266.15 | 1.002 (0.993–1.020) | 46104 / 46284 |
| runtime-first-signature | — / 6.18 | new behavior | — / 6088 |
| member-arguments-600 | — / 90.95 | new behavior | — / 19592 |
| member-arguments-2400 | — / 372.11 | new behavior | — / 61248 |
| member-arguments-runtime | — / 6.60 | new behavior | — / 6032 |
| pointer-temporaries-600 | 32.23 / 30.94 | 0.960 (0.958–0.968) | 9968 / 9600 |
| cast-reference-600 | — / 66.85 | new behavior | — / 16492 |
| constant-reference-600 | — / 73.42 | new behavior | — / 16224 |
| pointer-temporaries-2400 | 109.10 / 106.27 | 0.970 (0.962–0.976) | 22904 / 21776 |
| cast-reference-2400 | — / 258.00 | new behavior | — / 48060 |
| constant-reference-2400 | — / 289.11 | new behavior | — / 47732 |
| runtime-pointer-temporary | 6.39 / 6.02 | 0.955 (0.915–0.964) | 5916 / 6064 |
| runtime-cast-reference | — / 5.91 | new behavior | — / 6036 |
| named-result-600 | 67.93 / 60.29 | 0.867 (0.733–0.936) | 14288 / 13372 |
| retained-result-600 | 67.16 / 66.91 | 0.993 (0.980–1.008) | 14612 / 14812 |
| named-result-2400 | 238.75 / 217.53 | 0.925 (0.886–0.947) | 39304 / 35560 |
| retained-result-2400 | 259.94 / 260.95 | 1.007 (0.996–1.081) | 41692 / 42076 |
| runtime-named-result | 5.93 / 5.73 | 0.962 (0.947–0.972) | 5932 / 5944 |
| runtime-known-branch | 6.43 / 6.02 | 0.952 (0.934–0.969) | 5968 / 6040 |
| runtime-retained-result | 6.05 / 5.80 | 0.960 (0.949–0.968) | 5924 / 6104 |
| audit-conversion-lookup-600 | — / 104.35 | new behavior | — / 19864 |
| audit-base-query-600 | — / 69.49 | new behavior | — / 15840 |
| audit-template-query-600 | — / 96.46 | new behavior | — / 20332 |
| audit-conversion-lookup-2400 | — / 421.76 | new behavior | — / 62040 |
| audit-base-query-2400 | — / 272.42 | new behavior | — / 46216 |
| audit-template-query-2400 | — / 400.52 | new behavior | — / 63284 |
| runtime-audit-lookup | — / 6.09 | new behavior | — / 6100 |

Startup medians are **5.59 / 5.37 ms**. Compiler timings for the small runtime
sources are startup-limited diagnostics; no speedup is claimed from those rows.
Scaled equivalent sources dominate startup. Ordering-2400 shows a small observed
cost, **213.86 → 216.41 ms**, paired **1.011**; common-loop is **153.46 → 156.08 ms**,
paired **1.007**, and retained-result-2400 is **259.94 → 260.95 ms**, paired **1.007**.
These costs accompany required signature/query/context tracking across the full
review range. They are disclosed, not called speedups. Exact output is unchanged,
work remains source-proportional, and unchanged-workload peak RSS increases by at
most **384 KiB**. The audit removed per-angle allocation and redundant fact work;
no unprofitable optional pass remains hidden behind required semantic costs.

Member-head and named-result workloads eliminate selected calls and unused inline
bodies. Their 2400-case paired compiler ratios are **0.941 / 0.925**; peak RSS
falls **73504 → 67656 / 39304 → 35560 KiB**. Pointer-temporary code removes a
redundant IR copy at its required reference boundary; its 2400-case paired ratio
is **0.970**, RSS **22904 → 21776 KiB**. Native work must still be examined below.
The 20 baseline-rejected cases have only final costs, not A/B speedup claims.

## Executable runtime and size

Medians are milliseconds; paired ranges retain every block. New-behavior rows
report final runtime with its full observed sample range. Payload sizes are bytes.

| Workload | A / B ms | Paired B/A or B-only range | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 715.10 / 715.75 | 0.998 (0.989–1.001) | 206 / 206 |
| runtime-memory | 418.61 / 417.14 | 0.998 (0.997–0.999) | 434 / 434 |
| runtime-floating | 495.63 / 494.68 | 0.999 (0.996–0.999) | 230 / 230 |
| runtime-first-signature | — / 59.21 | B only (59.05–61.05 ms) | — / 177 |
| member-arguments-runtime | — / 77.97 | B only (77.61–79.18 ms) | — / 219 |
| runtime-pointer-temporary | 102.17 / 102.47 | 1.002 (1.000–1.011) | 239 / 239 |
| runtime-cast-reference | — / 67.06 | B only (66.85–67.33 ms) | — / 215 |
| runtime-named-result | 176.52 / 118.34 | 0.674 (0.672–0.687) | 222 / 196 |
| runtime-known-branch | 170.52 / 118.86 | 0.697 (0.694–0.699) | 268 / 192 |
| runtime-retained-result | 173.67 / 172.45 | 0.996 (0.898–1.003) | 222 / 222 |
| runtime-audit-lookup | — / 140.85 | B only (138.50–147.27 ms) | — / 200 |

The named-result and known-branch loops improve in **all four paired blocks**:
median ratios **0.674 / 0.697**, with payloads **222 → 196 / 268 → 192 bytes**.
This reproduces handoff 81's benefit against the entire accumulated baseline.
The result proves less executed work, not merely smaller IR. Explicit-only
retained-result programs have unchanged LowIR/native bytes and zero summary
inspections. Unaffected call, memory, floating and pointer-temporary programs
also retain identical native bytes; their small runtime variation (including
the pointer-temporary ratio just above one) is measurement noise, not new work.

Compiler `.text` grows **1,945,158 → 1,969,094 bytes**, **+23,936 (1.231%)** across
the entire accumulated range. Relative to entry 81's frozen final compiler,
the audit adds **1,792 bytes (0.091%)**. This is compiler implementation size;
it does not represent growth in programs produced by the optional summary.

## Work, legality, profitability and acceptance

The semantic repairs preserve canonical template, argument, target, naming-scope
and frame identities. Source conversion signatures are indexed at declaration;
source-object queries consume existing base edges. Fixed callee values do not
permit reuse of an obsolete naming class. A target-selected source specialization
projects its template/arguments once per query/frame and reuses the specialization
cache. There is no new body demand, parse replay, global invalidation or retry.
Parser angle scratch has one owner, grows with maximum nesting and is reused.

| Audit source count 600 → 2400 | Query work | Occurrences | Instructions |
|---|---:|---:|---:|
| Conversion lookup with implicit and retained calls | 1805 → 7205 | 37800 → 151200 | 7813 → 31213 |
| Fixed base query | 1804 → 7204 | 22200 → 88800 | 6015 → 24015 |
| Conversion-template query | 1804 → 7204 | 54600 → 218400 | 6013 → 24013 |

Their compiler median scaling is **4.04× / 3.92× / 4.15×** for fourfold source.
Conversion-template body transitions are **600 → 2400**, and named-result
summary requests are **600 → 2400** only where a materialized conversion exists.
The inherited first-member source also retains linear query work,
**7225 → 28825**, including correctly remapped query contexts. The inherited
list/forwarding/nested completion controls and all ten summary/emission budget
programs pass on B. These are structural work proofs; wall-time ratios remain
measurements rather than independent numerical gates.

The existing O0 optimization policy inspects one requested completed conversion
body, one return and at most eight wrappers; it retains at most one result fact.
It declines unknown/effectful/volatile/reference/virtual results and preserves
ordinary calls when proof or budget is unavailable. Receiver effects, second
conversions, storage, selected-arm cleanup and actual function uses are retained.
The transformation adds no executable work or cloned code; deferred emission
visits its leaf list once. There is no fixed-point pass or pipeline growth budget
increase. Constants remain runtime proofs and cannot grant constexpr eligibility.

The repeatable affected runtime improvement and reduced generated payload justify
the bounded proof and compiler implementation cost. Necessary source/query costs
are documented; unchanged correct output and proportional work bound unaffected
costs. No missing behavior is excused by a timing result. PA18 still has **24**
required course failures, which block full-stage completion but do not invalidate
this progress-preserving checkpoint audit.

Spec §9 mandates no numerical PA18/O0 latency/RSS ceiling. Historical **+15%,
+16 MiB and 5.5×** targets remain diagnostic, with their original measurements
preserved in earlier reports. No inherited plan turns them into new exit gates.
Current correctness, coverage, mandated limits, proof/work/growth bounds and
measured profitability remain requirements. On this evidence the accumulated
range meets the current stage's performance acceptance.
