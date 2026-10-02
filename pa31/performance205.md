# PA31 implementation205 performance evidence

Frozen A: stage entry `c0566ded`; B: implementation `d5bd5aee`. Binary/source
identities and exact reproduction scripts are in [binding](../student.tests/pa31/evidence205/binding.json).
Flags: `-O0 -c --stats`; CPU affinity 2. Host g++ only links generated objects.
All executable workloads consume runtime input and check live results. No builds,
course suites or control suites overlap timed measurements. The machine is shared.

## Protocol and acceptance

All **544 workload observations and 32 launcher samples** are retained. Equivalent
cases use four A/A samples and six ABBA blocks for compilation and execution
separately. Newly accepted list/inheritance cases have eight final-only samples
per mode/size; entry failures are recorded and never used as speedup baselines.
Compiler wall time, peak RSS, checked runtime and object/executable text size are
reported together. Small timing differences are not credited as optimizations.

This increment adds required semantic and ABI corrections, with no optional
optimization pass, speculative growth or new optimization-level policy. The
existing 45-second hosted compile limit and forced-inline work/growth bounds
remain unchanged. Historical 15% latency/zero-growth targets in inherited plans
remain diagnostics under spec §9; their measurements are preserved. PA32/33
optimization and PA34 self-hosting acceptance retain their later-stage scope.

## Equivalent common, allocation and hosted workloads

[Common raw observations](../student.tests/pa31/evidence205/common-performance.json),
[owner raw observations](../student.tests/pa31/evidence205/owner-performance.json),
[hosted raw observations](../student.tests/pa31/evidence205/hosted-performance.json).
Common inputs retain the earlier fixed hashes and 2,400 demanded templates, with
loops, calls, memory, floating point, exceptions and unused-body pruning. The
allocation sizes demand N template bodies and execute three million transitions.
The stream input constructs, writes, checks and destroys 200,000 ostringstreams.

| Workload | Compile A/B ms | Compile B/A [range] | Runtime A/B ms | Runtime B/A [range] | Compile RSS A/B KiB | Object / executable text A → B |
|---|---|---|---|---|---|---|
| memory | 326.72 / 350.06 | 1.009 [0.968, 1.272] | 107.15 / 109.61 | 1.035 [0.960, 1.054] | 29688 / 29740 | 151393 / 151633 → 151393 / 151633 |
| floating | 329.58 / 337.58 | 1.015 [0.955, 1.131] | 87.16 / 89.91 | 1.023 [0.967, 1.115] | 29904 / 29904 | 151234 / 151474 → 151234 / 151474 |
| exceptions | 433.28 / 481.23 | 1.027 [0.933, 1.903] | 265.55 / 284.15 | 0.998 [0.672, 1.171] | 29680 / 29800 | 151541 / 151781 → 151541 / 151781 |
| pruning | 209.47 / 209.34 | 1.004 [0.976, 1.016] | 52.47 / 52.43 | 0.998 [0.993, 1.004] | 35636 / 35692 | 151393 / 151633 → 151393 / 151633 |
| allocation64 | 36.77 / 36.86 | 1.001 [0.837, 1.075] | 86.14 / 85.63 | 1.011 [0.940, 1.067] | 10824 / 10788 | 12346 / 12586 → 12346 / 12586 |
| allocation256 | 117.79 / 119.08 | 1.013 [0.998, 1.031] | 86.11 / 85.95 | 0.995 [0.966, 1.028] | 20320 / 20368 | 48442 / 48682 → 48442 / 48682 |
| allocation1024 | 468.47 / 468.77 | 0.997 [0.944, 1.097] | 86.26 / 86.55 | 1.006 [0.947, 1.018] | 59484 / 59568 | 192826 / 193066 → 192826 / 193066 |
| stream1 | 1042.13 / 1049.09 | 0.999 [0.992, 1.030] | 117.29 / 117.60 | 1.001 [0.986, 1.028] | 66012 / 66056 | 28652 / 28892 → 28613 / 28853 |

| Workload | Compiler A/A ms | Runtime A/A ms |
|---|---|---|
| memory | 326.31–386.49 | 80.64–104.56 |
| floating | 326.32–403.46 | 70.99–90.57 |
| exceptions | 362.82–417.91 | 1183.03–1275.60 |
| pruning | 205.41–211.28 | 52.35–53.20 |
| allocation64 | 35.58–36.11 | 84.68–85.89 |
| allocation256 | 116.34–119.89 | 85.55–90.80 |
| allocation1024 | 457.81–469.71 | 86.75–91.46 |
| stream1 | 1030.94–1051.29 | 116.90–119.48 |

All common and allocation A/B object and executable pairs are byte-identical.
The stream object/executable text shrinks by **39 bytes**, removing the unwanted
array-allocation adapters. All equivalent compiler and runtime paired ranges
cross unity. Common compiler paired medians are 1.004–1.027; the exception
1.903 block ratio and large shared-host runtime spread remain in the record.
No repeatable avoidable regression is established. No runtime speedup is claimed.

## Newly supported source families

List cases convert a braced temporary through an initializer-list constructor;
inherited cases demand a zero-argument base constructor despite a declared move
constructor. Each family scales source declarations while keeping the runtime
loop at three million checked transitions. The entry compiler crashes on the
list family and rejects inherited construction, so these are absolute costs.

| Family / N | Compiler median [range] s | Peak RSS KiB | Runtime median [range] s | Object / executable text bytes |
|---|---|---:|---|---|
| list64 | 0.0485 [0.0482, 0.0495] | 12276 | 0.1769 [0.1760, 0.6029] | 18874 / 19114 |
| list256 | 0.1297 [0.1277, 0.1311] | 24080 | 0.1753 [0.1749, 0.1770] | 74554 / 74794 |
| list1024 | 0.4765 [0.4729, 0.4837] | 71944 | 0.1752 [0.1746, 0.1767] | 297274 / 297514 |
| inherited64 | 0.0366 [0.0357, 0.0378] | 10912 | 0.0700 [0.0695, 0.0719] | 13242 / 13482 |
| inherited256 | 0.1179 [0.1163, 0.1457] | 21356 | 0.0700 [0.0694, 0.0704] | 52026 / 52266 |
| inherited1024 | 0.4582 [0.4534, 0.6933] | 62688 | 0.0699 [0.0694, 0.0720] | 207162 / 207402 |

Every size retains N template body transitions. List exception work is **22N+3**,
with **2N** list plans, **64N+53** LowIR and **62N+72** native instructions, and
**290N+314** text bytes. Inherited exception work is **6N**, with **2N** constructor
actions, **33N+53** LowIR and **38N+72** native instructions, and **202N+314** text
bytes. The recorded 4× size steps confirm proportional owner work and bounded
memory growth. There is no quadratic retry or additional parsing pass.

The largest affected sample is **0.6933 s / 71,944 KiB** (time and memory maxima
may occur in different samples); hosted stream peaks at **1.0779 s / 66,056 KiB**.
These remain below the inherited 45-second limit. The shortest owner runtime is
about 70 ms. Launcher calibration is 5.790–8.821 ms for compiler help and
4.509–5.287 ms for `/bin/true`, including wrappers. No startup subtraction is used.

Reproduce common observations with `student.tests/pa27/performance147_common.py`.
Use `PERF_CPU=2 PERF_FAMILIES=allocation,list,inherited` with
`student.tests/pa31/performance205.py OUT ENTRY FINAL` for owner measurements,
and `PERF_FAMILIES=stream` for the hosted series. Source, flags, executable
checks, image hashes, phase counters and all individual samples are retained.

The 69-command typed trace and 46-command semantic/ownership controls separately
establish legality and the LowIR/native boundary. Measurement validates costs;
it does not replace required correctness or whole-stage independent review.
