# PA29 implementation167 performance and design evidence

Code: `db6b91a0`; PA29/O0. These are required semantic/ABI corrections, with no
new optional optimization or speedup claim. [Manifest](../student.tests/pa29/evidence167/performance-manifest.json)
freezes binary/script hashes, flags, inputs and CPU affinity 0. A is entry
`7719caea`; B is the final implementation. Host linking is outside compiler timing.
Compiler bytes are **4078728 → 4087408**. All results check runtime-dependent inputs.

Final observations: **224 common + 96 affected**, plus six launcher samples.
Common comparisons use four A/A observations then six ABBA blocks per mode.
Affected cases have eight samples per mode and demand shape. `/usr/bin/time`
records peak RSS; a wall clock times each invocation. No compiler build or
correctness suite overlapped final timing; documentation and external scheduling
were uncontrolled. Every outlier remains. An additional **320 preliminary
observations and six launcher samples** precede the dependent-vector ABI fix;
those binaries and measurements are retained and separately labelled.

## Equivalent existing workloads

[All observations](../student.tests/pa29/evidence167/common-performance.json).
Times are median seconds; paired ratios are medians of each ABBA block's B/A
means, followed by the full paired range. RSS is maximum KiB. Every A/B object
and executable is **byte-identical**. Templates demand 2,400 specializations;
fixed loops/calls/memory/floating/exception workloads retain prior checksums.
Pruning adds 1,200 unused ordinary functions. Self-hosting remains PA34-owned.

| Input | Compile A/B s | Paired compile [range] | Compiler RSS A/B | Runtime A/B s | Paired runtime [range] | Text A=B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1722/0.1822 | 0.9963 [0.8927–1.0936] | 29952/29900 | 0.0528/0.0527 | 1.0007 [0.9870–1.0040] | 151633 |
| floating | 0.1681/0.1663 | 0.9902 [0.9613–1.5906] | 29600/29552 | 0.0499/0.0497 | 0.9975 [0.9850–1.0118] | 151474 |
| exceptions | 0.2256/0.2630 | 1.0193 [0.9760–2.0332] | 29928/29800 | 0.2594/0.2651 | 1.0117 [0.9119–1.1501] | 151781 |
| pruning | 0.3260/0.3266 | 1.0131 [0.9899–1.1529] | 35396/35316 | 0.0721/0.0719 | 0.9987 [0.9719–1.0042] | 151633 |

| Input | A/A compiler range s | A/A runtime range s |
|---|---:|---:|
| memory | 0.2597–0.2631 | 0.0525–0.0527 |
| floating | 0.1617–0.1680 | 0.0499–0.0509 |
| exceptions | 0.1635–0.1733 | 0.2511–0.2795 |
| pruning | 0.3283–0.3370 | 0.0713–0.0730 |

Scheduling variation limits precision: byte-identical programs still have timing
spread. The preliminary comparison is retained in
[preliminary common observations](../student.tests/pa29/evidence167/preliminary-common-performance.json).
Neither batch establishes a repeatable runtime change; no runtime gain is claimed.
The final paired compilation results and their full spread disclose overhead
without imposing a new threshold. No repeatable avoidable regression is established.

## Required capability and scaling

[All observations](../student.tests/pa29/evidence167/affected-performance.json).
The layout family demands dependent class layouts, GNU vectors, extended boolean
masks and scalar functions consuming their sizes. The wrapper family validates
unused vector literals and reserved builtins while emitting a demanded inline
call chain. Both perform 20 million runtime-input calls and check independently
calculated sums. Entry compilation fails for all six sources, so it supplies no
semantically equivalent affected baseline and no speedup comparison.

| Family / demands | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| layout600 | 0.1019 [0.1008–0.1055] | 21396 | 0.1010 [0.1006–0.1022] | 1756 | 38321 |
| layout1200 | 0.2012 [0.1986–0.2031] | 35636 | 0.1019 [0.1012–0.1032] | 1760 | 76121 |
| layout2400 | 0.4057 [0.4044–0.4616] | 63200 | 0.1023 [0.1018–0.1123] | 1760 | 151721 |
| wrappers600 | 0.0668 [0.0659–0.0680] | 15808 | 0.2254 [0.2218–0.3318] | 1764 | 38355 |
| wrappers1200 | 0.2060 [0.1996–0.2129] | 24468 | 0.2754 [0.2206–0.3617] | 1760 | 76155 |
| wrappers2400 | 0.2450 [0.2429–0.2491] | 41556 | 0.2223 [0.2213–0.2254] | 1764 | 151755 |

Launcher median 0.0051 s [0.0049–0.4314]; the shortest affected runtime is 19.8× that median.

[Counter evidence](../student.tests/pa29/evidence167/scaling-counters.json) checks
every sample: N body transitions, N class completions for layout, and exactly
130N/32N projected occurrence nodes for layout/wrappers. Their parsed source
regions stay fixed at 3/2. Text and memory grow with demanded output. Wall time
has scheduling spread, so no exact scaling exponent is asserted. Preliminary
[affected observations](../student.tests/pa29/evidence167/preliminary-affected-performance.json)
remain available; the final binary includes the dependent ABI correction.

## Architecture, limits and boundary

`Shape<N>` traces one retained attribute expression through the existing immutable
substitution frame and canonical vector/layout facts to a scalar size in typed
LowIR and direct ELF. `pick<N>` in the substitution control additionally carries a
dependent vector into ABI naming: the PA9 graph's expression edge feeds encoding
and its explicit fact reader/writer. Source-produced LowIR validates and roundtrips;
standalone native execution, MIR, pointer ABI names and telemetry byte equality
are covered by [88 inspections](../student.tests/pa29/evidence167/inspection.json).

An inline wrapper's body is checked once. Its sparse unavailable-builtin marker
prevents emission on use; surrounding operands/statements retain normal checks.
Dormant namespace-inline call edges activate once through the existing keyed
queue. Member construction facts keep their required semantic demand. TU arenas
own syntax, canonical types, queries and body markers; existing function-local
backend state retains its release boundaries. No text transport, cloned graph,
whole-program retry or per-node owning allocation was added.

New optional work/growth budgets are **zero**. Required width/layout validation,
demand edges and dependent ABI encoding have linear work in their actual input
and output. Numeric rounding is bounded by 64 iterations; initializer tails use
one repeated action. Existing constexpr step/depth, native frame/data/alignment
limits and course timeouts remain unchanged. Necessary semantic compiler growth
is reported above; equivalent executables have no code growth. Historical blanket
15% latency/RSS and zero-growth goals remain self-selected diagnostics under spec
§9, preserving their measurements. PA30–34 broad hosted runtime, optimization,
allocation and self-hosting requirements add no PA29 exit gate.

GNU byte-width semantics follow the [GCC vector type attribute](https://gcc.gnu.org/onlinedocs/gcc-15.1.0/gcc/Common-Type-Attributes.html);
lane counts and packed boolean masks follow [Clang vector extensions](https://clang.llvm.org/docs/LanguageExtensions.html#vectors-and-extended-vectors).
Dependent dimension encoding follows the vendor ABI grammar implemented in
[Clang's typed Itanium mangler](https://llvm.googlesource.com/llvm-project/+/666e3326fedfb6a033494c36c36aa95c4124d642/clang/lib/AST/ItaniumMangle.cpp).
No fixture, reference output, comparison rule or required behavior was weakened.
Runtime vector operations remain outside the README scope; the combined required
fixture still needs its separate templated-lambda owner. Independent audit and
all 49 remaining required failures remain open in the compact plan.
