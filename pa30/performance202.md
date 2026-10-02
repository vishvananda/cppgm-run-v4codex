# PA30 checkpoint audit202 performance evidence

Reviewed code: `378d1bd83df4dd3f7a9c1693c18afa7e7c7bd61f`. The full-range A binary is audit198's
`4a081cb0`; B is the final audited compiler. The supplemental heavy-header
comparison uses entry `56ecc31c` as A because both versions correctly compile
those repaired inputs. [Bindings](../student.tests/pa30/evidence202/source-binding.json)
identify every binary and measured script. No performance result substitutes
for the required correctness checks.

## Protocol and applicable limits

All measurements use CPU 2 affinity and `-O0 -c --stats`, frozen binaries,
flags and inputs. Equivalent comparisons use an AAAA noise calibration and
six ABBA blocks. Compilation and executable execution are measured separately;
argv/argc, checked checksums, allocation/conversion counts and real loop work
prevent dead or constant-folded timing. Host g++ only links the generated
objects. No compiler build or course report ran during these measurements.

All **576 observations and 32 launcher calibrations** are retained: common
224, conditional-flow scaling 168, converted-bound scaling 48, all 19 repaired
fixtures plus noreturn convergence 80, and supplemental heavy-header A/B 56.
No sample, failed baseline, spread or outlier is discarded. Corrected array
bounds have no valid entry timing baseline: the old compiler rejects them.
Their final-only measurements establish cost and scaling, not a speedup.

The handout's **45-second per-compile limit** remains mandatory. The earlier
blanket 15% latency and zero-growth targets remain diagnostics under spec §9;
no inherited plan adds an exit gate. All performance195–201 documents and raw
measurements remain. No numeric RSS percentage is mandated. Required semantic
checking costs and PA31 hosted runtime, PA32/33 optimization levels and PA34
self-hosting are scoped separately; none excuses an avoidable PA30 regression.

## Equivalent fixed workloads across the full range

These fixed inputs demand 2,400 templates and exercise loops, calls, memory,
floating point, exception cleanup and unused-function pruning. Every A/B object
and executable is byte-identical.

| Workload | Compile A/B median ms | Compile B/A [paired range] | Runtime A/B median ms | Runtime B/A [paired range] | Compile peak RSS A/B KiB | Object / executable text bytes |
|---|---|---|---|---|---|---|
| memory | 282.35 / 283.17 | 1.003 [0.915, 1.011] | 73.06 / 73.00 | 1.001 [0.996, 1.019] | 29644 / 29740 | 151393 / 151633 |
| floating | 282.21 / 283.84 | 1.004 [0.914, 1.016] | 60.85 / 61.12 | 1.007 [1.001, 1.022] | 29864 / 29848 | 151234 / 151474 |
| exceptions | 283.33 / 282.81 | 0.999 [0.995, 1.012] | 365.89 / 368.99 | 0.946 [0.848, 1.167] | 30232 / 30348 | 151541 / 151781 |
| pruning | 215.42 / 215.49 | 1.001 [0.712, 1.009] | 73.44 / 71.29 | 0.966 [0.227, 1.019] | 35792 / 35840 | 151393 / 151633 |

| Workload | Compile A/A ms | Runtime A/A ms |
|---|---|---|
| memory | 281.69–364.97 | 72.87–73.30 |
| floating | 280.91–300.74 | 60.74–61.64 |
| exceptions | 282.32–285.08 | 449.49–495.38 |
| pruning | 206.68–209.11 | 53.34–56.76 |

The compiler paired medians are near unity. Exception runtime has large
environmental variation, despite identical executable bytes. Neither those
outliers nor the lower pruning ratio establish a generated-code speedup.
[All common samples](../student.tests/pa30/evidence202/common-performance.json)
retain the per-block observations and phase counters.

## Constant-join reachability owner

Each namespace demands a nested conditional loop through a template and emits
an ordinary checked transition function. The main loop performs six million
runtime-seeded transitions. Both correct versions emit identical objects and
executables; the new analysis changes acceptance, not program instructions.

| N | Compile A/B median s | Compile B/A [paired range] | Peak RSS A/B KiB | Runtime B median s | Runtime B/A [paired range] | Object / executable text bytes |
|---:|---|---|---|---|---|---|
| 64 | 0.0453 / 0.0447 | 0.986 [0.850, 1.031] | 11640 / 11652 | 0.1425 | 1.000 [0.992, 1.272] | 26618 / 26858 |
| 256 | 0.1566 / 0.1561 | 0.993 [0.826, 1.014] | 24140 / 24268 | 0.1430 | 0.999 [0.987, 1.006] | 105530 / 105770 |
| 1024 | 0.6215 / 0.6180 | 0.939 [0.880, 1.001] | 73084 / 73164 | 0.1424 | 1.000 [0.993, 1.005] | 421178 / 421418 |

Every B repetition reports **N** checked functions, **119N** instruction visits
and **58N** edge/operand visits. These counters agree with the ownership proof:
Pending → Constant → Varying permits at most two updates per integer fact;
only its indexed instruction users are queued. Each block is activated once,
with a single conservative seeding pass for unresolved values. Scratch space
and work are O(instructions + operands + slots + CFG edges), and all scratch
is released after the function. Volatile, escaped, partial and noninteger
storage remains unknown. No code is rewritten, no optional transform is added,
and no work/growth budget is borrowed from later optimization levels.
The low 1024-family paired ratio reflects retained A outliers; no speedup is
claimed. [All owner samples](../student.tests/pa30/evidence202/owner-performance.json).

## Converted allocation bounds: necessary semantic cost

Each independent family demands a class-to-integral bound conversion and an
allocation template. Runtime performs two million allocations, writes/reads,
deallocations and checked conversion calls. The previous entry compiler rejects
these valid sources; its exact outcomes are retained beside the final samples.

| N | Compile median [range] s | Peak RSS KiB | Runtime median [range] s | Object / executable text bytes |
|---:|---|---:|---|---|
| 64 | 0.0400 [0.0398, 0.0420] | 11600 | 0.0713 [0.0707, 0.0722] | 22078 / 22318 |
| 256 | 0.1347 [0.1329, 0.1472] | 23716 | 0.0713 [0.0709, 0.0739] | 87166 / 87406 |
| 1024 | 0.5350 [0.5143, 0.5457] | 72428 | 0.0742 [0.0717, 0.3335] | 347518 / 347758 |

[All converted-bound samples](../student.tests/pa30/evidence202/allocation-performance.json) retain the large runtime outlier too.

Conditional compile launcher: median 7.51 ms [7.11, 8.58].

Conditional runtime launcher: median 5.12 ms [5.00, 5.43].

Allocation compile launcher: median 6.63 ms [6.40, 7.64].

Allocation runtime launcher: median 4.79 ms [4.55, 4.92].

## Heavy headers and corrected outcomes

The final series covers every fixture repaired by implementations199–201,
plus noreturn convergence; all expected rejections produce no object and all
successful objects are deterministic across four trials. PA30 discards hosted
objects, so their execution is not a PA30 exit requirement.

| Fixture | Outcome | Compile median [range] s | Peak RSS KiB | Object text bytes |
|---|---|---|---:|---:|
| 200-local-callable-cross-function-reference-negative | reject | 0.0162 [0.0162, 0.0185] | 7080 | — |
| 400-reachable-missing-return-bad | reject | 0.0168 [0.0158, 0.0208] | 7708 | — |
| 600-hosted-fixed-vector-builtins | emit | 0.0259 [0.0197, 0.0278] | 7924 | 364 |
| 600-hosted-fstream-stream-compile | emit | 2.2256 [1.8886, 3.6819] | 68000 | 22301 |
| 600-hosted-recursive-std-function-string-substr | emit | 1.7293 [1.6529, 2.0047] | 54368 | 24188 |
| 600-noreturn-control-convergence | emit | 0.0206 [0.0205, 0.0216] | 7720 | 275 |
| 600-regex-iterator-difference-alias | emit | 4.5824 [3.7368, 5.2292] | 124284 | 25080 |
| 600-shared-ptr-allocator-shadowing | emit | 1.8294 [1.6572, 2.8008] | 56884 | 7377 |
| 700-hosted-codecvt-wstring-convert-char16-compile | emit | 2.1980 [1.9394, 2.8641] | 67320 | 25386 |
| 700-hosted-function-capturing-lambda-compile | emit | 0.4909 [0.4713, 0.4983] | 24216 | 2159 |
| 700-hosted-function-nullary-base-reentry-compile | emit | 1.7608 [1.5256, 1.8718] | 55556 | 28006 |
| 700-hosted-function-typeid-compare-compile | emit | 0.5871 [0.5569, 0.6189] | 24248 | 1860 |
| 700-hosted-iomanip-setprecision-compile | emit | 2.3971 [2.3106, 2.7851] | 70800 | 22301 |
| 700-hosted-locale-facet-compile | emit | 2.0974 [1.9572, 2.3315] | 65488 | 22301 |
| 700-hosted-map-subscript-piecewise-construct-compile | emit | 1.5129 [1.2263, 1.5721] | 43236 | 7737 |
| 700-hosted-piecewise-pair-index-sequence-alias | emit | 1.6187 [1.5677, 1.8069] | 53816 | 20556 |
| 700-hosted-replaceable-operator-new-dynamic-exception-spec | reject | 0.0631 [0.0602, 0.0829] | 8508 | — |
| 700-hosted-result-of-bind-member-template-callable-compile | emit | 0.6447 [0.6113, 0.6957] | 27524 | 1873 |
| 700-hosted-std-result-of-nested-callable-pack-compile | emit | 0.1406 [0.1381, 0.1450] | 11280 | 519 |
| 700-libstdcxx-regex-compiler-member-alias-call | emit | 5.0340 [4.9202, 5.3903] | 176936 | 222302 |

The equivalent heavy-header A/B series isolates audit repair costs from the
larger historical semantic additions. Both compilers emit identical objects.

| Header workload | Compile A/B median s | B/A [paired range] | A/A range s | Peak RSS A/B KiB | Text bytes |
|---|---|---|---|---|---:|
| 600-hosted-fstream-stream-compile | 1.1472 / 1.1329 | 0.986 [0.938, 1.013] | 1.1086–1.4107 | 68196 / 68116 | 22301 |
| 700-libstdcxx-regex-compiler-member-alias-call | 2.4563 / 2.4937 | 1.011 [0.997, 1.346] | 2.4479–2.6605 | 177132 / 177028 | 222302 |

The regex paired median is 1.011, with a retained 1.346 outlier block; fstream
is 0.986. This small checking cost has bounded work and no memory/text growth
trend. It does not justify dropping correctness checks or adding a percentage
gate. Final hosted samples peak at **5.390 seconds / 177028 KiB**, below the
45-second limit. The baseline A reaches 177132 KiB. See the complete
[hosted series](../student.tests/pa30/evidence202/hosted-performance.json) and
[hosted A/B series](../student.tests/pa30/evidence202/hosted-ab-performance.json).

## Emission and optimization audit

All 27 previously traced objects from implementations199–201 are byte-identical
to current explicit rebuilds, including vector volatility/representation,
aggregate exception cleanup, member-sequence deduction and noreturn/default
contexts. [Object bindings](../student.tests/pa30/evidence202/historical-images.json)
make the earlier runtime/text evidence applicable to those unchanged images.

The current [LowIR/MIR/ELF trace](../student.tests/pa30/evidence202/optimization-trace.json)
repeats the demanded `combine<int>::apply` example: one required always-inline
expansion, 17 actual units against 33 reserved, 64 prepared instructions,
420 standalone text bytes, and preserved volatile/noinline effects and CFI.
Existing caps remain depth 64, 262144 work per caller and 4194304 per program;
unsupported frame semantics, recursion or exhausted budgets retain the call.
Ordinary O0 loop storage and spill costs remain visible in the consumed MIR.
No optional optimization, unbounded search, textual production adapter or
runtime-profit claim was introduced. PA30 still has two required vector failures.
