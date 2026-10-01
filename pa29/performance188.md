# Implementation188 performance evidence

Frozen entry `c1caa0fc` versus final source `bf8bc00f`; the first implementation
`e52f4586` is retained separately. O0, `-c --stats`, the same hashed inputs,
host g++ link, CPU 0 affinity, wall time and `/usr/bin/time` peak RSS. Each
equivalent workload has four A/A samples then six ABBA blocks, separately for
compilation and checked execution. Corrected-only workloads have eight repeats
per mode and size. The 880 observations and 16 startup samples are all retained.

[Final common](../student.tests/pa29/evidence188/common-performance.json),
[final owner](../student.tests/pa29/evidence188/owner-performance.json),
[preliminary common](../student.tests/pa29/evidence188/preliminary-common-performance.json),
[preliminary owner](../student.tests/pa29/evidence188/preliminary-owner-performance.json).
Input hashes, compiler hashes, raw counters, every sample and A/A ranges are in
those records. No outlier was discarded. All generated programs check runtime
inputs/results; the loops cannot become a constant-folded/dead workload.

## Equivalent correct inputs

Times are medians in milliseconds; RSS is maximum KiB. Ratios are B/A paired
block medians with min–max block spread. Text is executable .text bytes.

| Workload | Compile A/B ms | RSS A/B KiB | Compile ratio [spread] | Runtime A/B ms | Runtime ratio [spread] | Text A=B |
|---|---:|---:|---|---:|---|---:|
| memory | 371.29/365.32 | 29876/30140 | 1.020 [0.869, 1.135] | 140.57/137.55 | 1.013 [0.866, 1.146] | 151633 |
| floating | 426.72/352.11 | 29724/29864 | 0.919 [0.691, 1.346] | 92.74/92.25 | 0.993 [0.912, 1.111] | 151474 |
| exceptions | 315.52/311.59 | 30096/30196 | 0.984 [0.938, 1.209] | 488.04/470.57 | 0.992 [0.961, 1.010] | 151781 |
| pruning | 430.18/416.43 | 35572/35716 | 1.001 [0.927, 1.189] | 88.27/89.92 | 1.037 [1.002, 1.444] | 151633 |
| names600 | 113.55/116.71 | 13576/14072 | 1.014 [0.939, 1.071] | 135.38/133.71 | 1.006 [0.931, 1.132] | 700 |
| names1200 | 356.13/374.62 | 20236/20724 | 1.027 [0.860, 1.104] | 162.50/155.35 | 0.961 [0.780, 1.031] | 700 |
| names2400 | 530.56/568.42 | 33548/34384 | 1.027 [0.912, 1.652] | 137.48/134.08 | 0.994 [0.834, 1.348] | 700 |

All seven object and executable A/B pairs are byte-identical. The same images
also survive the source refinement unchanged. Common inputs exercise templates,
memory/loops, floating calls, exceptions and pruning. The names family exercises
complete-class lookup of later ordinary contextual identifiers, comma lists and
enumerators, with 600/1200/2400 unused classes and a checked 3,000,000-step loop.

The preliminary names-family paired compile medians were 1.080, 1.093, 1.115.
The final implementation uses interned IDs for contextual recognition and skips
duplicate declarator indexing. These remove avoidable work. Remaining category
lookups and complete-class entries are necessary to preserve identifier meaning.
Final paired spreads and A/A ranges show substantial wall-time variation. These
samples do not establish a speedup; runtime variation occurs between identical
executables and cannot demonstrate a generated-code regression or improvement.

## Corrected-only capability scaling

The entry compiler rejects every contextual input, so it supplies no correct
A performance oracle. Time is median [min,max] milliseconds; RSS is peak KiB.

| Templates | Compile ms [spread] | RSS | Runtime ms [spread] | Text |
|---:|---|---:|---|---:|
| 600 | 255.16 [205.63, 340.68] | 14604 | 129.47 [124.45, 167.75] | 700 |
| 1200 | 445.55 [409.66, 662.15] | 21648 | 172.66 [162.41, 190.79] | 700 |
| 2400 | 538.00 [482.50, 998.64] | 35432 | 126.36 [124.06, 166.92] | 700 |

All 24 final compiler samples satisfy these existing telemetry counters:
`tokens=45N+139`, `parsed_nodes=nodes=80N+220`, initializer bindings `2N`,
template binding work `38N`, template bindings `8N`, zero specializations/body
transitions, 73 LowIR instructions, 118 operands, four native functions and
104 native instructions. Emitted code/text and images are identical at all N.
This verifies source-proportional retention/binding without instantiating unused
coroutines; it does not claim that actual coroutine runtime is implemented.

## Acceptance and limits

Compiler text is 4,069,174 → 4,075,358 bytes (+6,184, 0.152%).
That growth implements required parsing, lookup and diagnostics. Program text
does not grow. No new optional optimizer, retained optimization body, global
cache or work/growth allowance is introduced. Existing 1,000,000-step/512-depth
constant evaluation, 0x70000000 native frame/data, 4096 alignment and course
timeouts remain. No self-hosting claim is made; PA34 retains that owner.

Under spec §9, correctness costs and later-stage work do not create new exit
gates. Historical blanket 15%/zero-growth targets remain self-selected
diagnostics; no mandated limit, correctness rule or coverage is relaxed.
The refinement and all preliminary evidence remain visible. The measurements
support bounded stage-scoped semantic work, not an optimization-profit claim.
