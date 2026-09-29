# PA22/O0 performance evidence — implementation 114

The [harness](../student.tests/pa22/benchmark114.py) and
[all observations](../student.tests/pa22/performance114.json) freeze entry
`a8482d76` and implementation `87ee0e07`, flags, input contents and binary hashes.
The [final campaign](../student.tests/pa22/performance114-final.json) repeats the
same protocol after the proof-bound safety guard in `e6316d05`, using a separately
frozen final binary. All six generated LowIR outputs are byte-identical to the
first campaign. The first campaign below remains historical evidence.
Binaries and generated artifacts remain in `/tmp/pa22-114/`; none are committed.
Both lanes execute their own source-generated LowIR through the supplied backend,
then the host linker. All measured programs return the checked expected result.
The benchmark inputs are common, correct subsets of both compilers. Unsupported
entry cases are assessed separately by correctness controls, not speed ratios.

Each lane has a warmup, followed by four A/A observations and four ABBA blocks
on one allowed CPU. Compiler and executable runs are timed separately with
wall time and peak RSS. Native size is actual hosted ELF `.text`, including the
same host startup in both lanes. Telemetry/audit and plain compilation produce
identical LowIR. Small compiler inputs and trivial generated mains are startup
sensitive; their timings are retained without speed claims. Volatile iteration
counts and checked accumulated results keep runtime loops live.

| Workload | Compiler median ms A/B | Peak RSS KiB A/B | Runtime median ms A/B | Native `.text` bytes A/B |
|---|---:|---:|---:|---:|
| 9,600 template specializations | 776.84 / 800.28 | 107908 / 107984 | 10.30 / 10.68 | 768286 / 768286 |
| Calls | 17.96 / 17.27 | 6156 / 6220 | 333.59 / 330.60 | 430 / 430 |
| Memory loop | 9.92 / 9.53 | 6108 / 6188 | 175.31 / 178.88 | 658 / 658 |
| Floating loop | 15.03 / 14.60 | 6296 / 6512 | 141.58 / 148.39 | 454 / 454 |
| Member-call loop | 19.96 / 19.19 | 6140 / 6172 | 492.31 / 543.68 | 460 / 488 |
| 2,048 member-call functions | 368.92 / 253.82 | 32356 / 28252 | 6.14 / 5.42 | 143672 / 184632 |

Compiler `.text` grows 14,848 bytes, **0.66%** (2,240,710 → 2,255,558).
Template compilation's paired B/A range is 0.950–1.292; its 3% median difference
is not a stable general latency result. No general speedup is claimed.

The member loop's paired runtime ratios are **1.044–1.149**, with **28 bytes**
of native growth. Disassembly identifies the supplied backend's extra two-word
loads, stores and stack temporary for the required `i128` load/truncate form.
The frame reservation is unchanged at 112 bytes. The required PA22 representation
also adds 40,960 native bytes across 2,048 functions. Emitting the old object
representation would violate the checked LowIR contract. Scalarizing this
supplied-backend sequence belongs to the student's later native backend; it is
not hidden as a compiler improvement or used to excuse an optional transform.

## Bounded proof experiment

A second frozen compiler disables consumption of the zero-adjustment proof;
its observations remain in the original result. The stronger
[analysis-off experiment](../student.tests/pa22/proof-performance114.json), run
by [this harness](../student.tests/pa22/proof_benchmark114.py), also skips proof
analysis. Both retain the same source access/write checking. Its construction
is reproducible from `87ee0e07`: replace the getter in
`semantic/member_pointer_values.cpp` with `return false`, and return immediately
from `prepare_member_pointer_value`; compile that source with the same
`g++ -std=gnu++11 -Wall -O3` flags and relink the same objects. Frozen source,
exact commands and hashes are retained under `/tmp/pa22-114/` (`no-analysis.*`,
`no-analysis-build.json`, `compiler-D`).

Against this correct generic implementation, the final member loop runs in
**213.48 versus 257.28 ms**. All four paired ratios are **0.812–0.835**;
`.text` decreases **566 → 488 bytes**. The 2,048-function compiler medians are
**161.03 → 155.62 ms**, paired ratios **0.914–0.979**, and peak RSS
**29824 → 28224 KiB**. Its native `.text` decreases **336184 → 184632 bytes**.
Analysis counters distinguish **0 versus 4096** proof visits. The loop establishes
an executable benefit as well as smaller output. Final latency observations
below qualify the initial favorable compiler results.

## Final campaign and acceptance

| Workload | Compiler median ms A/final | Peak RSS KiB A/final | Runtime median ms A/final | Native `.text` bytes A/final |
|---|---:|---:|---:|---:|
| 9,600 template specializations | 696.50 / 714.17 | 109652 / 109692 | 4.70 / 4.84 | 768286 / 768286 |
| Calls | 6.61 / 6.40 | 6116 / 6256 | 124.37 / 123.10 | 430 / 430 |
| Memory loop | 6.66 / 6.45 | 6068 / 6156 | 75.45 / 74.85 | 658 / 658 |
| Floating loop | 6.89 / 6.68 | 6180 / 6392 | 87.42 / 87.38 | 454 / 454 |
| Member-call loop | 6.73 / 6.49 | 6100 / 6272 | 186.14 / 213.75 | 460 / 488 |
| 2,048 member-call functions | 164.74 / 152.49 | 32612 / 28088 | 4.21 / 4.10 | 143672 / 184632 |

The final template paired compiler ratios are **0.994–1.148**, with 40 KiB
additional peak RSS. Member-call runtime ratios are **1.137–1.317**: the required
representation/backend cost persists, rather than disappearing in the resample.
The [required fixture](tests/general/300-member-function-pointer-call.ref)
retains the `load i128`/`trunc i64` sequence responsible for the native traffic;
substituting a narrow load here would break the current comparison contract.

Against analysis-off, the final member loop improves **253.16 → 211.55 ms**, with
all four paired ratios **0.829–0.839** and the same 566 → 488 byte text reduction.
The 2,048-function compiler medians are **156.74 → 154.19 ms**, but paired ratios
span **0.919–1.107**; no repeatable compiler-latency improvement is claimed for
the proof. Peak RSS is **28500 → 28252 KiB**. The final compiler adds 1,152 text
bytes over analysis-off, about **0.05%**, while removing 151,552 generated text
bytes on that workload. The repeated runtime/text benefit, bounded analysis and
small measured compiler cost justify retaining the proof.

## Work budgets and stage acceptance

The proof has a shared **64-expression-node budget** per root demand, monotonic
active/proven/unknown states and a conservative fallback. It considers all
recorded writes and address exposure, never just class layout. It does not
interpret arbitrary calls or scan unrelated functions. Storage and traversal
are O(local member-pointer objects + writes); lowering consumes a completed
entity-keyed fact. Null/base conversions and generic application emit a constant
number of instructions/blocks per operation, with no search or duplication.

The [growth control](../student.tests/pa22/growth114.json) records exactly
**1,024 → 4,096** proof visits for **512 → 2,048** functions, plus IR and memory
measurements. A cyclic write dependency through 200 locals executes correctly
with generic adjustment after proof failure/budget exhaustion. All caches and
write records belong to the semantic TU and are released with it.

PA22 mandates O0 behavior and LowIR shape, not new numeric timing/growth limits.
Inherited +15%, +16 MiB and 5.5× targets remain diagnostic, as already classified
by PA21 under spec §9. Every observation and correctness requirement is retained.
The explicit proof work bound and constant lowering growth apply here; native
selection/allocation and self-hosting remain later-stage responsibilities.
