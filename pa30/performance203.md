# PA30 implementation203 performance and closure evidence

Code commit: `247c383a`. A is the entry compiler from `404e468e`; B is the
committed implementation. [Source and binary bindings](../student.tests/pa30/evidence203/source-binding.json)
record frozen SHA-256 identities, implementation sources, objects, measurement
scripts, host/compiler versions and CPU information. Both binaries are also
preserved in the writable artifact directory under `implementation203/`.

## Protocol and acceptance

All measurements use CPU 2 affinity and `-O0 -c --stats`. The existing fixed
common inputs retain their historical hashes. Correct equivalent A/B workloads
use four A/A calibration samples and six ABBA blocks for compilation and
execution separately. Generated programs take runtime input and check results
from live loops. Common programs cover 2,400 demanded templates, calls, memory,
floating point, exceptions and unused-function pruning. Vector/SIMD programs
perform three million seeded transitions with an independently computed checksum.
Host g++ links objects; it is not part of their implementation.

**512 observations and 16 launchers** are preserved: common 224, equivalent
heavy headers 56, vector/SIMD scaling 216, newly accepted random headers 16.
No builds or course reports ran during timing. All samples and outliers remain.
The entry compiler rejects the new SIMD/random cases; eight final-only samples
per mode/input establish cost, not a relative speedup. Header contract inputs
are compile-only and have no PA30 executable-runtime acceptance condition.

The mandated **45-second per-compile limit** is unchanged. Historical 15%
latency and zero-text-growth targets remain diagnostic under spec §9; earlier
measurements and mandated correctness/coverage remain intact. This increment
adds required semantics at O0 and no optional optimizer. It does not claim a
speedup. PA31 runtime, PA32/33 optimization and PA34 self-hosting are later-stage
requirements; their scope does not excuse avoidable current-stage regressions.

## Equivalent common workloads

Every A/B object and executable is byte-identical. Runtime variation therefore
cannot establish a generated-code improvement or regression. Near-unity paired
compiler medians have broad outlier ranges; all those observations remain in
[the common record](../student.tests/pa30/evidence203/common-performance.json).

| Workload | Compile A/B median ms | Compile B/A [range] | Runtime A/B median ms | Runtime B/A [range] | Compile peak RSS A/B KiB | Object / executable text bytes |
|---|---|---|---|---|---|---|
| memory | 244.154 / 236.114 | 1.001 [0.949, 1.086] | 52.580 / 52.710 | 0.991 [0.933, 1.009] | 29772 / 29856 | 151393 / 151633 |
| floating | 172.901 / 178.516 | 0.988 [0.907, 1.796] | 50.397 / 50.249 | 1.003 [0.943, 1.012] | 29848 / 30084 | 151234 / 151474 |
| exceptions | 193.981 / 171.958 | 0.988 [0.581, 1.003] | 463.268 / 463.432 | 1.059 [0.894, 1.121] | 30176 / 30308 | 151541 / 151781 |
| pruning | 484.552 / 479.763 | 1.003 [0.818, 1.106] | 93.783 / 91.514 | 1.003 [0.882, 1.414] | 35224 / 35512 | 151393 / 151633 |

| Workload | Compile A/A ms | Runtime A/A ms |
|---|---|---|
| memory | 166.991–257.182 | 52.313–53.191 |
| floating | 169.273–231.893 | 49.194–50.093 |
| exceptions | 171.319–237.207 | 259.854–446.382 |
| pruning | 468.605–581.789 | 86.444–97.439 |

## Vector value snapshots

Each of N independent namespaces demands one template transition. The source
constructs an eight-byte integer vector and extracts two lanes. Both versions
compute the same checksums. The repaired value owner snapshots lvalues before
later operand effects, including intrinsic arguments; already captured values
are reused. Ordinary snapshots are a typed copy, while volatile snapshots read
lanes explicitly. These are bounded O0 representation costs, with no added
alias analysis or optional transform. A complete vector read must retain its
value even when later expressions mutate the source.

| N | Compile A/B median s | Compile B/A [range] | RSS A/B KiB | Runtime A/B median s | Runtime B/A [range] | Object text A/B | Executable text A/B |
|---:|---|---|---|---|---|---|---|
| 64 | 0.034 / 0.035 | 1.010 [0.996, 1.029] | 10664 / 10772 | 0.135 / 0.136 | 1.012 [0.998, 1.018] | 11578 / 13114 | 11818 / 13354 |
| 256 | 0.110 / 0.110 | 0.980 [0.912, 1.081] | 19280 / 19868 | 0.135 / 0.136 | 1.012 [1.008, 1.023] | 45370 / 51514 | 45610 / 51754 |
| 1024 | 0.435 / 0.432 | 1.006 [0.911, 1.039] | 55608 / 56624 | 0.136 / 0.137 | 1.018 [0.999, 1.251] | 180538 / 205114 | 180778 / 205354 |

| N | Compile A/A s | Runtime A/A s |
|---:|---|---|
| 64 | 0.034–0.038 | 0.134–0.136 |
| 256 | 0.107–0.135 | 0.134–0.136 |
| 1024 | 0.429–0.535 | 0.134–0.136 |

The text delta is exactly **24N bytes** (about 13.6% at N=1024), with measured
runtime paired medians 1.012–1.018. The last size retains a 1.251 outlier block.
Compiler paired medians are 0.980–1.010. This modest runtime cost buys correct
value capture; removing capture globally would reintroduce the mutation and
volatile defects covered by the controls. No speculative optimization is
justified or credited by these measurements. Allocation and work remain
linear: **N** specializations/body transitions, **40N+53** LowIR instructions,
**41N+72** native instructions and **200N+314** object text bytes.

## Newly supported packed and architectural operations

Each family combines signed saturation, lane selection and scalar SSE addition.
Runtime results remain live and checked; the entry rejection is retained.
These figures are absolute costs, with eight samples for each mode/size.

| N | Compile median [range] s | Peak RSS KiB | Runtime median [range] s | Object / executable text bytes |
|---:|---|---:|---|---|
| 64 | 0.0778 [0.0762, 0.0826] | 16988 | 0.3751 [0.3731, 0.3766] | 111290 / 111530 |
| 256 | 0.2954 [0.2915, 0.5446] | 43800 | 0.3753 [0.3736, 0.3780] | 444218 / 444458 |
| 1024 | 1.2067 [1.1675, 1.3600] | 152592 | 0.3788 [0.3759, 0.3881] | 1775930 / 1776170 |

Every compile repetition records **N** specializations/body transitions,
**284N+53** LowIR instructions, **367N+72** native instructions and
**1734N+314** object text bytes. The 4× source-size steps have proportional
compiler work and bounded memory growth; runtime performs the same transitions
and stays near 0.375 seconds. Fixed packed expansion is at most 16 lanes.
General vectors unroll at most eight lanes then use one loop; architectural
operations use 64-byte local records. Dynamic comparison fallback has at most
32 alternatives, while known immediate predicates emit one instruction.
No global search, repeated semantic resolution or serialized production phase
was introduced. These are the explicit work/growth bounds; existing forced-
inline depth/work budgets are unchanged. [All scaling observations](../student.tests/pa30/evidence203/vector-performance.json).

Compile launcher median 6.175 ms [6.093, 6.836].

Runtime launcher median 4.781 ms [4.524, 21.176].

Small compile cases are more sensitive to startup; the largest sizes and
work counters support the scaling conclusion. Launcher outliers remain visible.

## Heavy-header budget

Equivalent fstream/regex objects match byte for byte. Large A/A and paired
spreads prevent crediting the lower medians as improvements.

| Header | Compile A/B median s | B/A [range] | A/A s | Peak RSS A/B KiB | Text bytes |
|---|---|---|---|---|---:|
| 600-hosted-fstream-stream-compile | 2.299 / 2.234 | 0.939 [0.910, 1.108] | 2.197–4.141 | 68124 / 68404 | 22301 |
| 700-libstdcxx-regex-compiler-member-alias-call | 5.491 / 4.896 | 0.919 [0.797, 1.095] | 5.393–5.816 | 177020 / 177304 | 222302 |

| Newly accepted header | Compile median [range] s | Peak RSS KiB | Text bytes |
|---|---|---:|---:|
| 600-random-to-address-qualified-call | 1.2937 [1.2485, 1.3865] | 84396 | 19907 |
| 700-hosted-random-mersenne-rshift-compile | 1.2687 [1.2521, 1.3101] | 84872 | 21256 |

Final hosted samples peak at **5.978 seconds / 177304 KiB**, below 45 seconds.
Entry regex reaches 6.695 seconds. The two random cases now emit deterministic
objects and finish in at most 1.387 seconds in this series. See the full
[equivalent series](../student.tests/pa30/evidence203/hosted-ab-performance.json)
and [new-header series](../student.tests/pa30/evidence203/hosted-new-performance.json).
No timeouts, references, comparison rules or required fixtures changed.

## Correctness, inspection and retained diagnostics

Final required reports pass: prior 4941/4941, PA30 153/153, through30 5094/5094,
and file audit (four inherited warnings). All 391 inherited and 48 new control
commands pass. Hardware differential validation passes all 286 cases with two
runtime inputs each. These include true arithmetic, saturation, shifts, masks,
float state and memory effects rather than placeholder results.

The [95-command trace](../student.tests/pa30/evidence203/trace.json) covers seven
source programs, including demanded templates, through canonical facts, typed
LowIR, consumed MIR, native encoding and host objects. Reparsed LowIR executes
through both backends. Stats on/off objects match. Three malformed target-fact
controls reject. The [42 ELF inspection commands](../student.tests/pa30/evidence203/inspection.json)
prove identical instructions/relocations, symbols and CFI after reconstruction.
Five complete object images match; two differ only in string-table ordering.

Intermediate diagnostics are retained separately and are not final passes:
one source-list mistake incorrectly linked the later-stage lowiropt scaffold;
the corrected list registers sources only for actual consumers. Two initial
positive shift samples used an illegal 255-bit immediate; valid 2040-bit samples
and a required-negative 255-bit control now cover both behaviors. Initial trace
assertions assumed floating literal suffixes survived canonical reading and
that integer address literal 1 was a non-pointer. The final check demands stable
canonical text and a typed i32 SSA operand rejection instead. No course check
was relaxed. Earlier green reports before the final immediate/address fixes
are retained but final validation binds the committed compiler above.

[Design203](design203.md) describes the new operation's exact serialized
contract, native scratch ownership and semantic sources. Final evidence
verification binds code, fixtures, binaries, outcomes and all measurements.
This closes implementation evidence; whole-stage independent review remains
Ralph's next step.
