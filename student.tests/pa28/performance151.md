# PA28 handoff151 performance

Final code: e58612a4. Frozen entry compiler SHA-256:
`74d142d3f7cac9931ece646a63ad3d71689fe1174fd0d5fb596e1e45d151b06d`.
Final compiler:
`d10d9691602a574ab7c5183c2b6ac71ab657bf693d57d9e2a30a873ff0254686`.

## Protocol and inputs

[Common measurements](evidence151/common-performance.json) retain every wall
time, peak RSS, exit status, phase/work counter, image size, binary/input hash,
flag and host tool version. They reuse the fixed PA26/27 template/call/memory,
FP, exception and unused-function workloads, with 2,400 demanded template bodies.
Their runtime inputs are argc; checked results depend on 3,000,000 loop
iterations or 200,000 throws/destructions. The pruning case adds 1,200 unused
declarations. Compilation and generated-program execution are measured
separately, with host linking outside timing.

Each equivalent A/B pair uses four A/A calibration runs, then six ABBA blocks
for each mode, pinned to CPU 0 with -O0 -c --stats. No compiler build or test
suite ran concurrently with the measurements. Paired ratios use the within-block
B/A means; medians and ranges below retain all samples.

The [new naming workload](evidence151/naming-input.cpp) demands 600 tagged class
specializations and a dependent decay/effect function, then executes 12,000,000
checked calls. A cannot compile this input: its failure is retained. Twelve
compiler and twelve runtime B observations measure newly supported costs;
they are not a failing-A optimization comparison.
[Raw naming data](evidence151/naming-performance.json).

## All four dimensions

Seconds are medians of the twelve measured A/B observations; RSS is maximum KiB.

| Input | Compiler s A/B | Compiler KiB A/B | Runtime s A/B | Text bytes A/B |
|---|---:|---:|---:|---:|
| memory | .2550 / .2523 | 29056 / 29204 | .0533 / .0550 | 151633 / 151633 |
| floating | .1615 / .1646 | 28944 / 29228 | .0504 / .0503 | 151474 / 151474 |
| exceptions | .1564 / .1560 | 28968 / 29284 | .4501 / .4506 | 151781 / 151781 |
| pruning | .2015 / .2513 | 34864 / 35060 | .0588 / .0590 | 151633 / 151633 |

| Input | Paired compiler ratio (range) | Paired runtime ratio (range) | Compiler A/A s | Runtime A/A s |
|---|---:|---:|---:|---:|
| memory | .991 (.801–1.030) | 1.041 (.993–1.138) | .2496–.2538 | .0527–.0607 |
| floating | 1.015 (.815–1.449) | .989 (.952–1.000) | .1683–.2154 | .0491–.0576 |
| exceptions | .996 (.971–1.003) | .996 (.747–1.011) | .1557–.3428 | .2522–.2530 |
| pruning | .997 (.947–1.369) | 1.010 (.999–1.076) | .3035–.3068 | .0531–.0545 |

Naming B costs **.1291 s** compilation (.1252–.1739), **22,264 KiB** compiler
peak, **.1798 s** runtime (.1777–.1872), **1,760 KiB** runtime peak, and
**63,778 executable text bytes**. Its object is 789,136 bytes.

There is substantial scheduling spread and temporal drift. In particular, the
pruning standalone median differs by 24.7%, whereas within-block paired ratios
have median .997. Memory runtime has a measured 4.1% paired increase. Neither
is hidden by an average over workloads. The pre-sparse pruning compiler ratio
was 1.134, also retained. These data do not establish a precise timing gain or
uniform slowdown; no speedup is claimed.

## Generated work and storage

[Object and text comparison](evidence151/comparison.json) proves every common
A/B object is byte-identical and every executable .text hash matches. Thus
actual selected instructions, frames, spills/reloads and calls are unchanged;
this is stronger than comparing IR instruction counts. The new naming object's
hash also stays identical across all three implementation layouts.

All final common work/storage counters, excluding time/RSS, match A, including
Entity size **120 bytes**. The initial implementation enlarged it to 128.
Moving rare tag heads to a flat entity index and placing the callable effect
byte in existing padding removes that avoidable per-entity cost. No optional
runtime transform was added, and common executable/object growth is zero.
Compiler RSS differences are 148–316 KiB (about 0.5–1.1%); raw values remain
available rather than being treated as exact allocation counts.

The new input consumes 6,198 tokens, 69,342 source/projected nodes, 600 class
completions, 601 template body transitions, 6,639 entities, 2,417 ABI graph nodes,
15,048 LowIR instructions and 1,803 native functions. Tags and completed tag
membership/head facts are O(actual published tags); untagged entities require
no tag nodes. Existing query, source occurrence and ABI graph caches own their
facts for the TU; final-name substitution storage is per encoder. Name/tag
sorting is bounded by the actual list size. No global retry or repeated
whole-function optimization scan is introduced.

## Stage acceptance and preserved observations

The inherited blanket 15% latency/RSS and zero-growth diagnostics are not
mandated PA28 gates under spec §9. No limit or coverage rule is weakened.
Existing depth/constant-evaluation, frame/ELF and native work/growth bounds
remain. Required semantic facts have bounded storage/work and no speculative
code growth. PA28 needs correct O0 host ABI behavior; PA32/33 optimizer gains
and PA34 self-hosting remain later-stage acceptance.

All **744 observations** are preserved: 248 before sparse storage, 248 with
sparse heads before effect-byte packing, and 248 final. The two additional
runs followed representation changes, not sample filtering. Earlier PA27
measurements remain untouched. See [inventory](evidence151/inventory.json),
[pre-sparse common](evidence151/pre-sparse-common-performance.json),
[pre-sparse naming](evidence151/pre-sparse-naming-performance.json),
[sparse/unpacked common](evidence151/sparse-unpacked-common-performance.json)
and [sparse/unpacked naming](evidence151/sparse-unpacked-naming-performance.json).

Reproduce with the frozen binaries:

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py OUT_COMMON A B
PERF_CPU=0 python3 student.tests/pa28/performance151.py OUT_NAMING A B
```

The compilation and runtime output checks are part of those scripts. This
evidence accepts the completed naming/attribute group; it does not establish
performance or correctness of the six unfinished runtime/layout cases.
