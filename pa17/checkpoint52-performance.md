# PA17 accumulated checkpoint performance — loop 52

Final reviewed code: `c43e8eb68db7e9b3f1dd0bbb18f14c92e4fc4b30`. The checkpoint
comparison uses entry `b34c1be9`; the cumulative comparison uses the previous
reviewed code `58789b00`. Both campaigns use the final frozen binary. The
[checkpoint observations](../student.tests/pa17/checkpoint52-performance.json)
and [cumulative observations](../student.tests/pa17/checkpoint52-cumulative-performance.json)
contain full source text, source/binary/backend hashes, every warmup and sample,
wall/CPU time, peak RSS, context switches, output hashes and untimed telemetry.

Build: g++ `-std=gnu++11 -Wall -O3`, `TEST_RUNNER_ENABLE`. Student compilation:
`--emit-lowir -O0`. Linux x86-64, CPU 31 affinity. Each common workload has one
warmup per compiler, four A/A samples and four ABBA blocks. Samples ran serially
without a simultaneous build or test campaign. Untimed `--stats --validate-lowir`
preflights precede timing; the timed compiler runs do not include these flags.
Every common LowIR file is byte-identical across A/B. Native validation uses the
supplied backend at O0, bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`,
SHA-256 `c3bae4acf3243d5a2fd6a15ef00e2d75d11a7d82715b1e4771f55165d0542490`.
Common native files are also byte-identical and every execution returns the
checked successful result. This is the explicit PA17 validation boundary. Own
native code generation and self-hosting are owned by later stages.

| Frozen compiler | SHA-256 | Compiler .text bytes |
|---|---|---:|
| Last review (cumulative A) | `7cdd268e16bc2d71eb64712e0cbc50eea8c5aea30e84bcbf0ddb9fa657f073c2` | 1,720,006 |
| Entry (checkpoint A) | `8acdd8bb8a09068d98617012b23949f63b4dd1dfbe15815007f23bb3a59c6493` | 1,768,518 |
| Final B | `e54376e7412f26025d58faf1f03127adcb6cd8e41f0e45c2b2e938a76bb41e94` | 1,769,670 |

Final compiler text increases 1,152 bytes (0.065%) from entry and 49,664 bytes
(2.89%) from the previous review. These are compiler size costs, separate from
generated-program text.

Compiler tables use milliseconds. A/B medians and sample ranges exclude the
A/A calibration; peak RSS is KiB. The paired column is the median of four
ratios of within-block B/A means, followed by the minimum/maximum block ratio.
It is not the ratio of the separate medians. All outliers remain included.

Checkpoint comparison: entry → final.

| Workload | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Peak RSS A/B KiB |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 110.07 [108.64–112.35] | 110.87 [109.55–114.03] | 1.0080 [0.9990–1.0174] | 108.23–108.91 | 22,400/22,476 |
| common-loop-float-1500 | 152.77 [150.44–154.45] | 153.15 [150.96–209.58] | 1.0016 [0.9985–1.1938] | 152.42–153.22 | 30,260/30,184 |
| common-partials-6000 | 452.19 [446.40–525.74] | 453.90 [449.83–459.90] | 0.9981 [0.9346–1.0129] | 446.72–449.63 | 72,840/72,852 |
| common-loop-float-6000 | 604.44 [597.66–611.03] | 609.31 [604.78–987.72] | 1.1075 [1.0018–1.3941] | 601.15–610.96 | 104,140/104,192 |
| member-qualified-1500 | 290.72 [287.80–407.57] | 293.77 [287.98–295.22] | 0.9971 [0.8452–1.0078] | 288.75–295.47 | 50,628/50,552 |
| member-qualified-6000 | 1214.79 [1206.14–1336.27] | 1209.10 [1205.48–1223.22] | 0.9816 [0.9568–0.9982] | 1216.44–1229.54 | 184,888/184,936 |
| three-head-demand-1500 | 453.59 [449.56–459.81] | 429.10 [425.31–457.59] | 0.9457 [0.9407–0.9770] | 451.16–462.95 | 79,336/78,164 |
| three-head-demand-6000 | 1941.01 [1926.02–1964.47] | 1851.88 [1843.82–1890.11] | 0.9538 [0.9472–0.9671] | 1916.83–1959.01 | 296,284/298,668 |
| wide-member-head-128 | 712.39 [703.22–720.28] | 676.50 [668.12–685.69] | 0.9475 [0.9446–0.9636] | 710.07–716.90 | 126,136/127,168 |
| wide-member-head-512 | 2903.49 [2872.81–2924.33] | 2743.76 [2712.42–2782.32] | 0.9471 [0.9371–0.9512] | 2873.83–2933.89 | 482,068/482,020 |
| friend-access-demand-1500 | 299.47 [294.56–301.99] | 290.41 [285.21–294.86] | 0.9726 [0.9637–0.9774] | 289.84–301.36 | 51,036/51,148 |
| ordinary-friend-demand-1500 | 156.68 [155.24–159.29] | 156.08 [154.03–159.67] | 0.9940 [0.9866–1.0106] | 156.37–160.00 | 29,680/29,528 |
| friend-access-demand-6000 | 1286.15 [1262.87–1384.90] | 1234.16 [1220.05–1274.10] | 0.9535 [0.9262–0.9757] | 1265.49–1272.47 | 186,804/186,800 |
| ordinary-friend-demand-6000 | 659.94 [645.86–675.78] | 659.42 [654.17–785.73] | 1.0060 [0.9905–1.0767] | 640.42–661.85 | 102,316/102,320 |
| current-name-demand-1500 | 149.65 [145.03–152.03] | 144.82 [140.30–146.19] | 0.9680 [0.9563–0.9700] | 150.20–233.86 | 28,616/28,300 |
| parenthesized-value-demand-1500 | 166.02 [162.72–169.57] | 167.71 [164.09–254.39] | 1.0162 [0.9969–1.2609] | 163.87–165.56 | 27,132/27,336 |
| current-name-demand-6000 | 613.92 [602.94–628.77] | 601.66 [595.18–603.59] | 0.9722 [0.9659–0.9949] | 603.72–673.77 | 96,352/96,756 |
| parenthesized-value-demand-6000 | 692.72 [686.38–697.36] | 692.60 [681.53–702.99] | 0.9976 [0.9925–1.0111] | 695.62–745.12 | 92,664/92,700 |
| dormant-member-body-1500 | 108.39 [106.46–112.34] | 105.18 [102.90–115.39] | 0.9657 [0.9615–0.9943] | 108.39–110.73 | 21,952/22,020 |
| dormant-member-body-6000 | 446.41 [431.90–452.45] | 424.17 [419.75–435.47] | 0.9577 [0.9447–0.9665] | 434.59–790.05 | 70,464/70,644 |

The repeated-source fix shows a consistent affected-workload benefit: three-head
demand improves 4.6–5.4% by paired measurements, wide heads improve 5.3%, and
dormant member bodies improve 3.4–4.2%. Every block of these final comparisons
is below 1.0. This supports retaining the removal of duplicate source-name work.
It does not claim a generated-code optimization or a universal frontend speedup.

The checkpoint loop/float-6000 paired result is +10.75%, while separate medians
are 604.44/609.31 ms (+0.81%). Its block ratios are 1.0080, 1.3941, 1.0018 and
1.2069: B wall outliers include 988, 844 and 709 ms while user+system CPU remains
about 590–600 ms. The cumulative run below has large outliers on both sides.
These observations limit wall-time inference; they do not establish a repeatable
compiler-work regression. Parenthesized-value-1500 also has a 1.2609 block but
a +1.62% paired median and a near-neutral 6000-case comparison. No samples were
discarded and no faster campaign was selected to replace these final runs.

Cumulative comparison: previous reviewed code → final.

| Workload | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Peak RSS A/B KiB |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 111.35 [109.81–113.17] | 112.30 [110.74–115.96] | 1.0135 [1.0053–1.0258] | 110.05–112.37 | 22,308/22,500 |
| common-loop-float-1500 | 152.10 [151.61–154.54] | 154.33 [152.52–226.59] | 1.0152 [1.0034–1.2449] | 151.58–156.90 | 29,808/30,196 |
| common-partials-6000 | 453.34 [447.86–530.44] | 453.44 [450.43–458.94] | 1.0012 [0.9270–1.0038] | 445.17–449.83 | 72,744/72,844 |
| common-loop-float-6000 | 611.68 [606.55–1117.92] | 613.16 [608.91–1711.84] | 1.0066 [0.7092–1.8903] | 607.22–610.65 | 104,076/104,148 |
| member-qualified-1500 | 287.66 [284.38–372.90] | 295.92 [292.96–299.88] | 1.0282 [0.8970–1.0412] | 285.36–766.23 | 49,436/50,676 |
| member-qualified-6000 | 1207.00 [1194.01–1344.75] | 1237.34 [1229.45–1251.15] | 0.9874 [0.9678–1.0306] | 1195.96–1207.51 | 181,632/184,956 |

The cumulative member-qualified medians rise about 2.9%/2.5% at 1500/6000;
paired results differ because of the displayed wall-time spread. Peak RSS grows
1,240/3,324 KiB. The accumulated changes add source-head/name/access and friend
identity/demand checks required by the controls. Exposed retained counts and
capacities on this corpus are equal; the final source-check counter records
264,000 node visits for 6,000 distinct source definitions, with no projected
reuses. Those distinct definitions require their own checks. The source audit
found and removed the avoidable projected repetition. No optional transform
or extra generated-code growth is being accepted to explain the remaining cost.
The measurements do not isolate allocator placement as the cause of RSS changes.

Checkpoint RSS also rises 2,384 KiB (0.8%) for three-head-6000 and 1,032 KiB
(0.8%) for wide-head-128, while three-head-1500 falls 1,172 KiB and wide-head-512
is nearly unchanged. These changes are disclosed, not rounded into a memory
saving. The source-check fix adds two TU counters (16 bytes), no per-specialization
retained record. Existing fact/occurrence/entity counters match entry on these
corpora. Peak memory remains proportional to the source and demanded facts in
the measured size pairs; this is scoped evidence, not a bound on every input.

| Source obligation | Final work, small/large | Projected reuses, small/large | Body transitions, small/large | Final compile growth for 4× input |
|---|---:|---:|---:|---:|
| Dormant member body | 431/431 | 1500/6000 | 0/0 | 4.03× |
| Three retained heads | 92/92 | 4500/18000 | 1500/6000 | 4.32× |
| Wide head (128/512 parameters) | 695/2615 | 600/600 | 300/300 | 4.06× |

The constant 431/92 source visits across specialization-count growth demonstrate
sharing at the corrected owner. Dormant bodies remain undemanded. Wide-head
work follows actual source parameters; concrete signature substitution is still
required. No claim is made that this fix eliminates occurrence creation: the
source-check change removes repeated traversal and scratch-index construction.
The [trace](../student.tests/pa17/checkpoint52-trace.json) records the complete
source → semantic facts → typed LowIR → supplied-backend ELF chain, including
late member demand and `sizeof(X)` becoming integer immediates 1 and 4.

Native execution measurements follow. Volatile loop bounds and checked results
keep calls, memory traffic and floating-point work live. All peak native RSS
values are 256 KiB. Text is the sectionless executable payload after the ELF
entry; these runtime programs have no static data.

| Campaign / workload | A median [range] ms | B median [range] ms | Paired B/A [range] | A/A range ms | Native text A/B bytes |
|---|---:|---:|---:|---:|---:|
| Checkpoint / runtime-calls | 358.47 [357.19–359.66] | 358.86 [357.25–360.45] | 1.0009 [0.9997–1.0034] | 357.69–359.62 | 206/206 |
| Checkpoint / runtime-memory | 209.19 [208.94–209.75] | 209.41 [209.01–210.10] | 1.0012 [0.9993–1.0026] | 209.10–210.53 | 434/434 |
| Checkpoint / runtime-floating | 249.47 [248.93–251.14] | 249.11 [248.65–250.14] | 0.9983 [0.9980–0.9989] | 248.78–249.77 | 230/230 |
| Checkpoint / friend-address-runtime | n/a | 181.95 [181.04–184.92] | n/a | n/a | 235 |
| Cumulative / runtime-calls | 358.36 [357.88–363.32] | 359.31 [357.80–361.12] | 0.9993 [0.9964–1.0040] | 357.62–360.65 | 206/206 |
| Cumulative / runtime-memory | 209.85 [209.22–212.31] | 209.89 [209.04–212.02] | 1.0004 [0.9968–1.0040] | 209.93–214.06 | 434/434 |
| Cumulative / runtime-floating | 248.38 [247.94–249.14] | 247.93 [247.84–248.68] | 0.9989 [0.9977–0.9996] | 249.02–250.06 | 230/230 |

The final-only friend-address workload has no correct entry comparator: entry
LowIR validation rejects an undefined function symbol. Final validation and
execution pass. Its six samples follow one warmup; compiler median is 6.285 ms
with 5,836 KiB peak RSS. It measures the required behavior, not a speedup.
The common native-source compiler medians are 5–6 ms and startup-sensitive;
those observations remain in JSON but support no compile-latency claim. Native
common outputs are identical, with runtime ratios close to 1.0. There is no
observed runtime/text tradeoff hidden by faster source checking.

The native trace also inspects actual loops and stack transfers. Calls remain
one per iteration (direct in the call/floating corpora, indirect for the friend
address). The memory loop retains indexed loads/stores. Main frames reserve
72/288/80/88 bytes for calls/memory/floating/friend respectively, with the noted
saved register and floating helper temporaries. These are observed O0 costs,
not inferred allocator spill counts. Common bytes prove these costs unchanged;
the final-only friend case has no valid entry encoding to compare.

No optional generated-code transform changed. Proven integer `sizeof` facts
are consumed directly; unknown facts remain conservative. There is no new
invalidation, fixed point or growth policy. Existing constant evaluation keeps
its 1,000,000-work and 512-depth limits. Array expansion stays capped at eight
(including nested expansion), with counted-loop fallbacks; zero-initialization
and cleanup sharing are unchanged. No ABI, debug-location or floating-point
semantic relaxation is used. The observed frontend improvement removes duplicate
required work without speculative output growth.

PA17/O0 has **no mandated numerical performance ceiling**. The inherited +15%
latency, +16 MiB RSS and 5.5× scaling values are diagnostic targets under
spec.md §9, not additional stage exit gates. Their historical misses remain
recorded. The corrected owner, equivalent common outputs, measured scaling and
explicit semantic costs support current-stage acceptance. All language behavior,
343 course inputs, comparison rules and mandated work/growth limits are retained.
All 37 course failures remain required implementation; performance acceptance
does not waive them.

Preserved earlier evidence: [previous audit](audit-performance.md),
[heads](head-performance.md), [friends](friend-performance.md), and
[source names](name-performance.md), including their archived campaigns. This
audit also preserves the interrupted
[entry-validation campaign](../student.tests/pa17/checkpoint52-performance-before-entry-validation.json),
its completed [resumption](../student.tests/pa17/checkpoint52-performance-before-function-demand.json),
and [intermediate cumulative campaign](../student.tests/pa17/checkpoint52-cumulative-before-function-demand.json).
The first campaign incorrectly expected entry compilation itself to reject the
friend-address program; entry instead emitted invalid LowIR. After adding
validation to that preflight, the resumption reused completed equivalent
observations and measured the final-only case. Those campaigns bind to intermediate
code `1b137ed1`. The two final campaigns above are fresh and bind to `c43e8eb6`.
All binaries and raw files remain under the recorded artifact paths.

Reproduce with `checkpoint52_benchmark.py A B WORK OUT checkpoint` (or
`cumulative`), choosing new output/work paths. Run
`python3 student.tests/pa17/verify_checkpoint52.py` to verify frozen provenance,
all final and complete historical samples, output equality and acceptance records.
