# PA22/O0 performance evidence — implementation 115

This handoff completes dependent member-pointer formation, arguments and queries.
It adds no optional optimization pass. The required immediate NTTP call form
consumes a recorded target instead of constructing and unpacking a runtime member
pointer; its costs are measured separately below. The inherited bounded local
proof and its repeatable runtime benefit are documented in
[114's evidence](performance114.md), with that proof's 64-node bound unchanged.

## Protocol and frozen artifacts

[The harness](../student.tests/pa22/benchmark115.py) records source contents,
hashes, commands, backend identity, compiler telemetry, all warmups and all
observations. Each campaign uses one allowed CPU (0), a warmup per lane, four A/A
observations and four ABBA blocks. Compilation and executable timing are separate
wall-time measurements with peak RSS. Flags are `--emit-lowir -O0`; telemetry
and `--validate-lowir` runs produce identical LowIR to plain compilation.
The supplied backend consumes each lane's source-generated LowIR; g++ 15.2.0
links its object. Every executable returns the checked expected result.
Native size is hosted ELF `.text`, including the same startup in both lanes.
Volatile iteration counts and checked accumulated results keep the loops live.
Trivial mains and small compilations are startup-sensitive, not speed evidence.

Frozen binaries remain under `/tmp/pa22-115/`:

| Lane | Source | SHA-256 | Compiler `.text` bytes |
|---|---|---|---:|
| Entry A | `5a21daff` | `cfaa725e1554731684523def17018c801b61d23aca6945d915b7cfbca67eaaf5` | 2255558 |
| Intermediate B | `7336cd8d` | `a9f4cb7d923518f7befde0b5aa8461522790272ffc47db2cec9102524dfe51ad` | 2262214 |
| Final C | `2fd83db0` | `66f8ed6d5d9cd6cae9660303d15e4f280414a7e63b20fc17e505848d8c0895f8` | 2262278 |
| Generic G | B with immediate-target recording disabled | `4046601766303bce3458371ab84466b4a2a07037b78480d343835b8e47209917` | 2261830 |

The generic lane retains correct types, checking, representation and execution.
[Its construction script](../student.tests/pa22/target_variant115.py) replaces
one recording guard in `semantic/member_pointers.cpp`, compiles that source with
the same `-std=gnu++11 -Wall -O3` flags and relinks the same objects.
[Exact build metadata](../student.tests/pa22/target-variant115.json) records
source/variant/binary hashes and commands. Reproduction starts from built
`7336cd8d`; the final parenthesis fix does not affect these benchmark inputs.
All three final target outputs equal B's earlier outputs byte for byte.

## Common correct subset: A versus C

The six fixed inputs are inherited unchanged from 114. All six generated LowIR
outputs and native text sizes are identical between A and C. Unsupported entry
programs are correctness controls, not inputs to speed comparisons.

| Workload | Compiler median ms A/C | Peak RSS KiB A/C | Runtime median ms A/C | Native `.text` A/C |
|---|---:|---:|---:|---:|
| 9,600 template specializations | 705.19 / 733.40 | 109688 / 108472 | 4.19 / 4.19 | 768286 / 768286 |
| Calls | 6.77 / 6.80 | 6132 / 6148 | 123.54 / 123.05 | 430 / 430 |
| Memory loop | 6.41 / 6.36 | 6148 / 6208 | 73.65 / 73.50 | 658 / 658 |
| Floating loop | 6.50 / 6.51 | 6392 / 6360 | 86.45 / 86.48 | 454 / 454 |
| Member-call loop | 6.65 / 6.68 | 6176 / 6172 | 211.81 / 211.99 | 488 / 488 |
| 2,048 member-call functions | 152.11 / 154.60 | 28108 / 28200 | 3.70 / 3.66 | 184632 / 184632 |

| Workload | Paired compiler C/A range | Paired runtime C/A range |
|---|---:|---:|
| Templates | 0.668–1.355 | 0.980–1.025 |
| Calls | 0.968–1.040 | 0.988–1.596 |
| Memory | 0.988–1.186 | 0.989–1.004 |
| Floating | 0.996–1.033 | 0.996–1.002 |
| Member loop | 0.993–1.006 | 0.997–1.137 |
| Member functions | 1.017–1.251 | 0.986–1.002 |

[All final observations](../student.tests/pa22/performance115-common-final.json)
retain the spread and outliers. Template A/A is 687.39–779.94 ms; measured A/C
ranges are 688.74–1364.78 / 692.37–1433.26 ms. Member-function compilation rises
2.49 ms at the median, with 92 KiB more peak RSS. Its completed semantic fact
storage increases 40,960 bytes (one 20-byte record per function); lookup work
increases 6,144 visits while type probes decrease 2,049. This is bounded required
query/checking work, not an optional transform. Compiler text grows 6,720 bytes,
**0.30%**, across the complete behavior group. No general latency gain is claimed.

The [initial common campaign](../student.tests/pa22/performance115-common.json)
is preserved: template medians 1180.80/1079.49 ms and member-function medians
188.41/158.78 ms illustrate why a one-campaign speed claim would be misleading.
All six initial LowIR outputs equal the final ones.

## Immediate NTTP target: G versus C

This comparison isolates a source-known target in a checked 40-million-call
loop and in 512/2,048 demanded specializations. The required
[NTTP call fixture](tests/general/300-ref-qualified-member-function-pointer-nontype-call.ref)
and [NTTP execution fixture](tests/general/300-template-member-pointer-nttp-execute.ref)
consume the known function address directly. The fact belongs to semantic
application and is carried to lowering by an index; lowering performs no lookup
or new proof. G measures the otherwise correct generic representation sequence.

| Workload | Compiler median ms G/C | Peak RSS KiB G/C | Runtime median ms G/C | Native `.text` G/C |
|---|---:|---:|---:|---:|
| NTTP member-call loop | 41.85 / 45.85 | 6148 / 6148 | 222.38 / 233.43 | 447 / 422 |
| 512 specializations | 93.94 / 111.31 | 15240 / 15044 | 42.86 / 47.17 | 30007 / 16696 |
| 2,048 specializations | 306.52 / 257.27 | 43328 / 42344 | 61.31 / 59.45 | 119095 / 65848 |

[The final campaign](../student.tests/pa22/performance115-target-final.json)
was run without overlapping course checks/builds. Loop runtime paired C/G ratios
are **1.177, 0.875, 1.048, 0.982**; A/A spans 210.11–270.72 ms and G/C ranges are
205.02–333.59 / 216.30–264.80 ms. The initial loop medians were
568.10/551.31 ms, with ratios **1.119, 0.881, 0.720, 1.031**; that campaign
overlapped a course report and remains in
[the original observations](../student.tests/pa22/performance115-target.json).
Neither campaign establishes a repeatable runtime benefit. The final loop median
is worse; it is reported rather than described as an optimization win. The tiny
generated mains in the growth workloads are execution checks, not runtime tests.

Compiler paired C/G ranges are 0.919–1.558 (loop), 0.583–1.396 (512) and
0.489–1.051 (2,048). Startup/scheduling spread prevents a latency claim. Text
reduction is repeatable: 25 bytes in the loop, 13,311 / 53,247 bytes in the two
growth inputs. Immediate recording adds 384 compiler text bytes at B; C adds
another 64 bytes for the independent parenthesis correction. This is required
O0 emission with bounded cost; no optional transform is retained on an unsupported
speedup claim, and smaller IR alone is not asserted to improve runtime.

## Budgets and stage acceptance

Canonical member owners reuse an existing Type slot; `Type` does not grow.
Substitution caches are TU-owned and keyed by canonical type/environment;
non-dependent nodes are shared. Access-sensitive member-argument cache keys
include target, lexical query identity, access override and explicit-instantiation
context. Traversals are O(affected type/query edges + required candidates), with
no global retry or unrelated-body demand.

Immediate targets add at most one static fact per application and a constant
amount of lowering work, replacing four generic instructions per call with no
generated growth. At 512→2,048 specializations, body transitions and frames are
exactly 512→2,048; retained source regions stay at 2, region nodes at 41 and
dependence work at 8. Static requests are 512→2,048; final instruction counts
are 5,649→22,545 versus G's 7,697→30,737. Semantic fact storage is identical to
G (487,528→1,929,592 bytes); memory and code scale with demanded specializations.
The existing 64-node local proof budget and conservative generic fallback remain
mandatory bounds. There is no new higher-level search or work/growth budget.

Under spec §9's stage-scoped acceptance, PA22/O0 requires correct behavior and
the checked LowIR shape; it introduces no numeric wall-time/RSS gate. Inherited
+15%, +16 MiB and 5.5× diagnostics remain non-gating, with all historical misses
preserved. Correctness and coverage are unchanged. Necessary semantic checks and
required emission are accounted for above; optional optimization profitability
has not been inferred from noisy timing. Student native selection/allocation and
self-hosting remain later-stage work, not additional gates for this handoff.
