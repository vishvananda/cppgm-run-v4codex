# PA29 implementation168 performance and design evidence

Code: `34a8dd1a`; PA29/O0. Required aggregate semantics and evaluation corrections,
with no new optional optimization or speedup claim. The [manifest](../student.tests/pa29/evidence168/performance-manifest.json)
freezes compiler/script hashes, flags, input hashes and CPU affinity 0. A is entry
`0c6df4c1`; B is the final implementation. Host linking is outside compiler timing.

Compiler bytes: **4087408 → 4105424**.

Final observations: **224 common + 96 affected**, plus six launcher samples.
Common comparisons use four A/A observations followed by six ABBA blocks per mode;
affected cases use eight samples per mode/size. Wall clocks measure invocations;
`/usr/bin/time` measures peak RSS. No builds or test suites overlapped final timing;
documentation and external scheduling were uncontrolled. All outliers remain.

Another **320 preliminary observations and six launcher samples** precede the
cast-query ownership extraction required by file audit. Their unchanged binary,
[common observations](../student.tests/pa29/evidence168/preliminary-common-performance.json),
[affected observations](../student.tests/pa29/evidence168/preliminary-affected-performance.json)
and [manifest](../student.tests/pa29/evidence168/preliminary-performance-manifest.json)
are retained. Final acceptance uses the final binary below.

## Equivalent existing workloads

[All observations](../student.tests/pa29/evidence168/common-performance.json).
Times are median seconds; paired ratios are medians of each ABBA block's B/A means,
followed by the full paired range. RSS is maximum KiB. Every A/B object and executable
is **byte-identical**. Fixed inputs include 2,400 demanded template specializations,
loops, calls, memory, floating arithmetic and exception handling. Pruning adds 1,200
unused ordinary functions. Self-hosting remains PA34-owned.

| Input | Compile A/B s | Paired compile [range] | Compiler RSS A/B | Runtime A/B s | Paired runtime [range] | Text A=B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1629/0.1643 | 1.0050 [0.9954–1.0491] | 29668/29912 | 0.0528/0.0527 | 1.0020 [0.9830–1.0277] | 151633 |
| floating | 0.1614/0.1633 | 0.9999 [0.6319–1.0255] | 29084/29164 | 0.0495/0.0492 | 0.9959 [0.9259–1.0263] | 151474 |
| exceptions | 0.1617/0.1640 | 1.0176 [0.5597–1.0223] | 29612/29840 | 0.2512/0.2518 | 1.0033 [0.9951–1.0168] | 151781 |
| pruning | 0.2021/0.2015 | 0.9929 [0.6694–1.0286] | 35208/35404 | 0.0728/0.0729 | 0.9966 [0.9826–1.0144] | 151633 |

| Input | A/A compiler range s | A/A runtime range s |
|---|---:|---:|
| memory | 0.1619–0.1682 | 0.0525–0.0528 |
| floating | 0.1593–0.6421 | 0.0492–0.0494 |
| exceptions | 0.1593–0.1625 | 0.2528–0.2624 |
| pruning | 0.2011–0.2036 | 0.0518–0.0523 |

Byte-identical programs still show scheduling spread. Neither measurement batch
establishes a runtime change, and none is claimed. Compiler paired results and
RSS disclose overhead; no repeatable avoidable regression is established. These
required semantic changes introduce no optional optimizer work.

## Required capability and scaling

[All observations](../student.tests/pa29/evidence168/affected-performance.json).
The designator family instantiates functions initializing an anonymous union's
non-first member and omitted surrounding fields. The mutation family evaluates
an N-element aggregate array: N nested reference calls write its elements, one
copy snapshots the array, and a second loop checks the sum. Both families also
emit N scalar functions and execute 20 million runtime-input calls with independently
calculated checksums. Entry compilation rejects all six sources: it provides no
correct affected baseline, so these measurements are capability costs, not speedups.

| Family / demands | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| designators600 | 0.1117 [0.0844–0.6187] | 19188 | 0.2167 [0.2161–0.2172] | 1760 | 56921 |
| designators1200 | 0.1655 [0.1647–0.1984] | 31164 | 0.1180 [0.1167–0.1197] | 1756 | 113321 |
| designators2400 | 0.3319 [0.3280–0.3422] | 54508 | 0.1164 [0.1162–0.1171] | 1756 | 226121 |
| mutation600 | 0.1398 [0.1383–0.1885] | 26424 | 0.1357 [0.1351–0.1362] | 1760 | 70121 |
| mutation1200 | 0.2754 [0.2725–0.2780] | 44940 | 0.1355 [0.1349–0.1376] | 1756 | 139721 |
| mutation2400 | 0.5584 [0.5562–0.5620] | 82280 | 0.1352 [0.1349–0.1355] | 1756 | 278921 |

Launcher median 0.0049 s [0.0047–0.0061]; shortest affected median runtime is 23.8× launcher median.

[Counter assertions](../student.tests/pa29/evidence168/scaling-counters.json) verify
every compilation sample: N template body transitions for designators; N+1 for
mutation, with exactly 22N+24 evaluation steps, N+7 immutable object work, 2N+7
addresses and 2N dependency work. This directly checks that scalar-reference
calls do not repeatedly freeze the enclosing array. Memory and text grow with
demanded output. Wall-time variation precludes an exact scaling-exponent claim.

## Architecture, limits and handoff

`make<S>` traces a retained compound-literal type and designator through an
immutable substitution frame, indexed anonymous-storage projection and canonical
query facts into selected list/initializer actions and typed LowIR. SFINAE and
noexcept consume those facts. PA9 retains the source expression as typed `InitList`
and `DesignatedInit` nodes; its fact writer/reader roundtrip the `di` member-name
encoding. Neither ABI naming nor lowering reconstructs selection from text.

The local mutation reducer uses an activation-owned sparse overlay keyed by
canonical subobject addresses. Writes dirty only ancestors, and storage versions
participate in complete constant-call keys. Scalar reads walk only the addressed
path. Aggregate snapshots visit actual parts and sort dirty array children;
unchanged values share immutable TU payloads. Whole assignment retires only its
previously dirty descendants. Frame overlays release at call exit, and dead,
const or externally owned storage remains unwritable. Existing constexpr step
and recursion bounds still apply. No global mutable cache or AST clone was added.

[146 inspections](../student.tests/pa29/evidence168/inspection.json) cover LowIR
validation/roundtrip, standalone execution, object disassembly/symbols, MIR facts,
telemetry equality, exact required-fixture execution and PA9 fact adapters. An
initial optional inspection attempted unsupported hosted `-g`/source-line output;
that is not a PA29 gate. The current frontend does not emit those debug views;
required later debug suites retain their stage ownership. No debug behavior was
changed or claimed here. The initial file-audit size finding is retained in
[preliminary validation](../student.tests/pa29/evidence168/preliminary-validation.json)
and resolved by the separate source-cast-query owner.

Optional work/growth budgets are **zero**. Required initialization work is linear
in clauses/fields; mutable writes cost path depth plus amortized dirty-path
retirement; snapshots are linear in their consumed/produced parts plus sorting
changed children. Existing evaluator, native frame/data/alignment and course
limits are preserved. Necessary compiler size growth is disclosed above;
equivalent executables have no growth. Historical blanket 15% latency/RSS and
zero-growth targets remain self-selected diagnostics under spec §9, preserving
all earlier measurements. They create no extra PA29 exit gate.

Omitted-field initialization uses N3485 [dcl.init.aggr]/7 and [dcl.init.list].
Compound literals follow [GNU C++ temporary lifetime rules](https://gcc.gnu.org/onlinedocs/gcc-15.1.0/gcc/Compound-Literals.html).
The C++14 local-mutation extension follows [N3652's constexpr relaxation](https://isocpp.org/files/papers/N3652.html).
Designator encoding follows the [Itanium ABI braced-expression grammar](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangle.expressions).
No course fixture, reference, comparator or coverage rule changed. The compact
plan records all 46 remaining required failures and preserves independent review.
