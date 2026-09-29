# PA21/O0 performance evidence (loop 104)

[Raw observations](../student.tests/pa21/performance104.json) preserve frozen
binaries, flags, sources, hashes, CPU affinity, warmups, every wall-time/RSS sample,
paired ratios, output checks and separately collected telemetry. Reproduce with
[benchmark104.py](../student.tests/pa21/benchmark104.py). Entry compiler:
`fa079cde`; final implementation: `511fe9b9`. Both use the course host build
(`g++`, `-std=gnu++11 -Wall -O3`, existing runner entry flags).

Equivalent correct workloads use one warmup each, four A/A calibration samples
and four ABBA blocks. New semantics use a warmup and six final-only samples;
the rejecting entry compiler cannot support an A/B benefit claim. Timing excludes
stats/validation; `/usr/bin/time` records peak RSS. Native execution is timed
separately after compilation and a checked run through the supplied O0 backend.

## Equivalent inherited workloads

All six produce byte-identical LowIR and native binaries. Values are medians.
Compiler size below is ELF `.text`. The supplied sectionless executable has no
separate `.text`; its reported payload proxy includes static data and EH tables.
That native size limitation is explicit, not a claim about isolated instruction
bytes. No own-native-backend optimization is claimed at PA21.

| Workload | Compile A → B, ms | Peak RSS A → B, KiB | Runtime A → B, ms | Native payload B |
|---|---:|---:|---:|---:|
| startup | 6.316 → 6.272 | 5784 → 6120 | 3.953 → 4.005 | 24 B |
| auto-specializations-9600 | 699.684 → 683.351 | 107256 → 107424 | 3.745 → 3.718 | 768062 B |
| runtime-calls | 6.507 → 6.545 | 6032 → 6124 | 124.718 → 125.316 | 206 B |
| runtime-memory | 6.659 → 6.593 | 6048 → 6196 | 73.115 → 72.778 | 434 B |
| runtime-floating | 6.576 → 6.471 | 6228 → 6276 | 86.410 → 86.139 | 230 B |
| runtime-reference-captures | 6.636 → 6.496 | 6028 → 6168 | 24.805 → 24.828 | 188 B |

The heavy-template compiler paired B/A ratios are 1.011, 0.891, 0.969 and 1.055;
A/A spans 660–765 ms. Its peak RSS changes by 168 KiB (0.16%). Live
calls/memory/floating/capture runtime paired ranges are 0.964–1.013 /
0.991–1.002 / 0.919–1.020 / 0.995–1.012. All observations, including scheduling
outliers, remain in the record. Startup-sized timings establish no benefit.
Identical executables cannot attribute runtime noise to this change. No speedup
or repeatable avoidable regression is established. Compiler `.text` grows
20,800 B (0.97%), from 2,134,150 to 2,154,950, for required list semantics,
storage, deduction and checking/emission separation.

## New behavior and scaling

| Workload | Compile ms | Peak RSS KiB | Runtime ms | Native payload |
|---|---:|---:|---:|---:|
| list-specializations-800 | 124.674 | 25288 | 3.367 | 147262 B |
| list-specializations-3200 | 498.680 | 82656 | 3.543 | 588862 B |
| scalar-list-elements-256 | 7.798 | 6412 | 3.211 | 2912 B |
| scalar-list-elements-1024 | 11.200 | 7180 | 3.075 | 11360 B |
| scalar-list-elements-4096 | 26.193 | 10188 | 3.149 | 45152 B |
| class-list-elements-32 | 6.879 | 6188 | 3.067 | 6184 B |
| class-list-elements-128 | 7.706 | 6372 | 3.007 | 22696 B |
| class-list-elements-512 | 11.727 | 7436 | 3.102 | 88744 B |
| runtime-scalar-lists | 6.722 | 6188 | 136.961 | 307 B |
| runtime-class-lists | 6.770 | 6156 | 61.477 | 1664 B |

At 800→3200 specializations, list plans/objects grow exactly 4× and the library
representation count stays one. Semantic expression work is 11,213→44,813;
inherited expression facts are 5,600→22,400, while fixed template expressions
remain nine. Body transitions are exactly 800→3200. LowIR instructions grow
36,805→147,205; latency grows 4.00× and RSS 3.27×. These observations agree
with work proportional to demanded occurrences and reuse of fixed recipes.

Scalar list lengths 256→1024→4096 emit 546→2082→8226 instructions, with one
plan/object/type throughout. Class lengths 32→128→512 emit 284→956→3644
instructions, exactly seven more per additional explicit element. Cleanup suffix
sharing avoids quadratic prefix duplication; complete-array destruction uses a
reverse loop beyond eight elements. Native payload growth includes the supplied
backend's EH support. The 4096-scalar compile range is 25.2–40.6 ms; these short
compilations are scaling diagnostics, not isolated latency benefit claims.

The scalar runtime loop performs 4M calls over changing `i&1` elements and checks
the accumulated 22M result. The class loop performs 1M three-element list calls,
checks 5.5M and zero live objects. Both read volatile trip counts. Their runtime
ranges are 136.3–160.0 ms and 61.3–64.6 ms, well above startup. Other new rows
are checked startup controls; their timings establish no runtime profit.

## Stage-scoped acceptance

Spec §9 at PA21/O0 mandates O(n) or O(n log n) work in consumed/produced facts
and IR, plus the inherited eight-element array expansion cap. This change adds
required semantics, not an optional optimization. No additional numerical gate
is imposed, and there is no profit claim against a compiler rejecting the source.
Avoidable helper/copy emission work was removed before freezing the final binary.
Earlier measurements in [performance102.md](performance102.md) and
[performance103.md](performance103.md) remain intact. Their self-selected ratios
are diagnostic targets under the stage-scoped rule; none replaces required
bounds, correctness or coverage. Self-hosting and own native optimization retain
their later-stage owners. Remaining LowIR contract failures remain required
implementation, regardless of these measurements.
