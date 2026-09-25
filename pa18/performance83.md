# PA18 implementation 83 performance evidence

Acceptance is **PA18/O0 LowIR**, spec §9. Frozen entry `48c864ab`
and final `ac354fad` binaries, source text/hashes, flags, backend
hash, telemetry, warmups and every sample are in
[the raw observations](../student.tests/pa18/loop83-performance.json).
[The harness](../student.tests/pa18/benchmark83.py) pins one CPU, records four
A/A observations and four ABBA blocks, and checks each executable result.
Incorrect/rejected baseline cases receive six final-only observations; they are
never treated as faster correct implementations. Compilation, supplied-backend
construction and executable timing are separate. No optional optimizer is added.

Compiler flags are `--emit-lowir -O0`; separate preflights add `--stats
--validate-lowir`. Build flags are `g++ -std=gnu++11 -Wall -O3`. The backend is
bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, invoked only by the harness.
The runtime sources retain volatile bounds and checked data-dependent loop,
call, memory and floating work. The array loops perform 12 million calls,
initialize distinct mutable arrays and select/mutate elements from runtime input.
Native optimization, own object emission and self-hosting remain later stages.

## Compiler latency and peak RSS

Times are median milliseconds; ratios are paired B/A medians and complete ranges.
RSS is maximum observed KiB. All noise/outlier samples remain in the JSON.

| Workload | A / B ms | Paired ratio (range) | A / B peak KiB |
|---|---:|---:|---:|
| ordering-600 | 57.54 / 57.67 | 0.993 (0.873–1.012) | 15316 / 15308 |
| ordering-2400 | 215.36 / 214.72 | 0.997 (0.828–1.002) | 43832 / 43900 |
| common-loop-float-1500 | 155.60 / 155.67 | 0.996 (0.946–1.162) | 30220 / 30436 |
| runtime-calls | 6.76 / 6.51 | 0.982 (0.932–0.994) | 5996 / 6040 |
| runtime-memory | 6.51 / 6.31 | 0.970 (0.947–0.989) | 5868 / 6064 |
| runtime-floating | 6.23 / 6.05 | 0.962 (0.950–0.983) | 6052 / 6152 |
| wide-array-600 | 52.26 / 54.57 | 1.039 (1.016–1.053) | 13284 / 13736 |
| converted-array-600 | 79.27 / 83.13 | 1.043 (1.028–1.083) | 18240 / 18384 |
| deduced-rows-600 | — / 67.24 | final only | — / 16092 |
| pack-bound-600 | — / 64.00 | final only | — / 14724 |
| wide-array-2400 | 201.20 / 208.87 | 1.037 (1.031–1.230) | 38212 / 37448 |
| converted-array-2400 | 322.06 / 341.18 | 1.077 (1.048–1.122) | 54272 / 55536 |
| deduced-rows-2400 | — / 263.75 | final only | — / 46372 |
| pack-bound-2400 | — / 253.76 | final only | — / 40712 |
| runtime-array-wide | 6.42 / 6.18 | 0.966 (0.943–0.980) | 5968 / 6104 |
| runtime-array-conversion | 6.50 / 6.30 | 0.963 (0.947–0.973) | 5996 / 6044 |

Startup medians are **5.83 / 5.67 ms**.
Small compiler inputs are startup-limited diagnostics, not speedup evidence.

## Executable runtime and size

Times are median milliseconds. The supplied native executables have no section
headers, so payload bytes (including data) are a size proxy, not a claim to have
isolated executable `.text`. The compiler sizes below use actual ELF `.text`.

| Workload | A / B ms | Paired ratio (range) | A / B payload bytes |
|---|---:|---:|---:|
| runtime-calls | 715.18 / 716.69 | 1.001 (0.997–1.003) | 206 / 206 |
| runtime-memory | 417.70 / 415.64 | 0.996 (0.992–0.998) | 434 / 434 |
| runtime-floating | 494.98 / 495.86 | 1.001 (1.000–1.005) | 230 / 230 |
| runtime-array-wide | 80.84 / 86.05 | 1.065 (1.061–1.074) | 284 / 320 |
| runtime-array-conversion | 85.06 / 85.82 | 1.008 (1.006–1.017) | 314 / 320 |

Compiler `.text`: **1,969,094 → 1,972,422 bytes**, **+3,328 (+0.169%)**.

## Work bounds and acceptance

| Source family, 600 → 2400 | Initializer actions | Constant-array checks | Query work | LowIR instructions | Median time scaling |
|---|---:|---:|---:|---:|---:|
| wide-array | 1800 → 7200 | 1800 → 7200 | 607 → 2407 | 7213 → 28813 | 3.83× |
| converted-array | 1800 → 7200 | 1800 → 7200 | 610 → 2410 | 7213 → 28813 | 4.10× |
| deduced-rows | 4200 → 16800 | 4200 → 16800 | 610 → 2410 | 12613 → 50413 | 3.92× |
| pack-bound | 1800 → 7200 | 1800 → 7200 | 1805 → 7205 | 4813 → 19213 | 3.97× |

Bound inference reuses the checked plan, walks explicit element actions once,
and rejects a non-consuming clause. Source shape checks retain fixed conversions
and defer expansion counts; query substitution reads the completed entity by
identity. There is no token replay, expanded-bound walk, global invalidation or
whole-program candidate retry. Initializer plans, source facts and query caches
are TU owned; per-function lowering uses the existing typed action/data path.
The existing eight-lane omitted-element expansion limit and compact zero-data
fallback remain unchanged. Explicit emitted data costs O(output bytes).

The removed small-wide and construction exclusions were contrary to the inherited
PA16 copy requirement. Their cost/size comparison is disclosed as a required
contract repair; it does not justify an optional optimization or relax correctness.
The constant plan checker already proves the initializer before emitting its
image. Removal also eliminates the additional initializer syntax scan. Volatile,
effectful and runtime-address cases retain ordinary execution.

PA18/O0 has no mandated numerical compiler latency/RSS ceiling. Historical +15%,
+16 MiB and 5.5× diagnostic targets remain non-gates; prior measurements are
preserved. Required correctness, coverage, bounded work, and profitability of
optional transforms remain requirements. Existing named-result summary limits
and their previously measured benefit are unchanged; their inspection controls
pass. No new optional transformation needs a growth or profitability allowance.

The unchanged ordering and common compiler workloads retain exact LowIR; call,
memory and floating probes retain identical native bytes. Their paired runtime
ratios are 1.001, 0.996 and 1.001. No speedup or executable regression is inferred
from those noise-scale differences. Common compiler RSS rises by at most 216 KiB.

At 2400 functions, required wide-array materialization costs **201.20 → 208.87 ms**
(paired **1.037**), while converted arrays cost **322.06 → 341.18 ms** (paired
**1.077**). Wide-array RSS falls **38212 → 37448 KiB**; converted-array RSS rises
**54272 → 55536 KiB**, **1264 KiB**. The old wide-scalar shortcut skipped constant
proof/image work completely. Final constant-array work is 7200 actions for both
families, exactly four times the 600-function workload. The additional work
establishes required images; no secondary full initializer scan remains.

The wide-array runtime cost is **80.84 → 86.05 ms**, paired **1.065** in all four
blocks, with **284 → 320 bytes** of payload. Converted-array runtime is
**85.06 → 85.82 ms**, paired **1.008**, payload **314 → 320 bytes**. These are
observed regressions, not gains. The wide-array LowIR differs by replacing four
immediate stores with the required 32-byte readonly image and one 32-byte copy;
the following mutation, indexing, calls and loop remain the same. The image
accounts for most of the 36-byte payload growth. This is necessary representation
work under PA16's inherited rule, performed by the supplied backend, not a new
optional transform that could be removed. Improving native implementation of
that required copy belongs to the later backend stage. No filename/type-width
exception or coverage reduction is used to recover the old numbers.

Four bound/query workloads are final-only: the entry compiler either computes
an incorrect deduced extent or rejects a valid pack-dependent query. Their costs
are measured without treating the wrong result as a fast baseline. All 15
workloads with a main function pass native preflights; the common compiler corpus
retains exact LowIR. On these observations, explicit work bounds and unchanged
fallback semantics, this behavior group meets the stage-scoped acceptance rule.
