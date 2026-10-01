# PA29 implementation173 performance evidence

Validated implementation: `128e7e39`. Acceptance is **PA29/O0**. Explicit-template
lambda semantics and hosted ABI repairs add no optional optimizer transform.
No speedup is claimed.

[Manifest](../student.tests/pa29/evidence173/performance-manifest.json) freezes
binaries, scripts and environment. Compiler size is **4,166,352 → 4,187,928 bytes**
(+21,576, **0.518%**). Final measurements contain **280** equivalent A/A+ABBA,
**96** new-capability and **8** launcher observations. Two earlier rounds preceded
the trailing-return query and decltype ABI fixes; all **1,128** main observations
and **24** launcher samples across the three binaries remain in the manifest.
Final acceptance uses the exact binary used by all final correctness checks.

Compile and executable times are measured separately; host linking is excluded.
Flags are `-O0 -c --stats`, with `-std=c++11` for closure workloads. Executables
are linked by g++ 15.2.0. Builds and tests finished before each measurement round.
External scheduling and documentation work were uncontrolled; CPU affinity was
unset and every outlier was retained. Timings include process launch. Runtime
inputs use argc; every timed execution checks a known result and returns success.
The common inputs preserve the PA26/27 hashes, loops, calls, memory, floating-point,
exception cleanup and 2,400 demanded template specializations. Self-hosting remains
PA34 work.

## Equivalent workloads

[Common samples](../student.tests/pa29/evidence173/common-performance.json) and
[closure samples](../student.tests/pa29/evidence173/closure-performance.json)
retain four A/A samples followed by six ABBA blocks per workload/mode. Tables use
median seconds and maximum compiler RSS in KiB. Paired values are the median and
full range of each block's mean B/mean A; A/A samples are excluded from A/B medians.
Raw data also retains executable RSS and every phase time/counter.

| Workload | Compile A/B s | Paired B/A [range] | Compiler RSS A/B KiB | Runtime A/B s | Paired B/A [range] | Text bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1720/0.1696 | 0.9714 [0.7099–1.0335] | 29120/29276 | 0.0519/0.0517 | 0.9945 [0.9726–1.0297] | 151633 |
| floating | 0.1718/0.1731 | 0.9857 [0.7120–1.3923] | 29520/29612 | 0.0478/0.0482 | 1.0098 [0.9601–1.0605] | 151474 |
| exceptions | 0.1698/0.1666 | 0.9820 [0.9663–1.5084] | 29388/29500 | 0.2617/0.2672 | 1.0045 [0.7387–1.3674] | 151781 |
| pruning | 0.2138/0.2145 | 1.0527 [0.9746–1.3971] | 35464/35160 | 0.0529/0.0534 | 0.9492 [0.9008–1.0390] | 151633 |
| ordinary1200 | 0.2667/0.2744 | 1.0003 [0.9389–1.1912] | 38536/40492 | 0.3233/0.3253 | 1.0261 [0.9486–1.7077] | 190939 |

| Workload | A/A compile range s | A/A runtime range s |
|---|---:|---:|
| memory | 0.1644–0.1806 | 0.0509–0.0523 |
| floating | 0.1641–0.1715 | 0.0477–0.0541 |
| exceptions | 0.1638–0.1684 | 0.2510–0.3090 |
| pruning | 0.2132–0.2248 | 0.0512–0.0531 |
| ordinary1200 | 0.2608–0.2754 | 0.3180–0.3466 |

All paired ranges cross 1. These samples do not establish a repeatable speed
change or avoidable regression. Common objects and executables are byte-identical.
[Counter comparison](../student.tests/pa29/evidence173/counter-delta.json) finds
all existing common work/storage counters unchanged, with three added zero closure
counters. Ordinary closure work adds one argument-pack record and two lookup steps
for its one source capture recipe; body and specialization counts are unchanged.

The ordinary object grows **936,032 → 1,181,992 bytes** (**26.28%**), while executable
text stays **190,939 bytes**. [ELF evidence](../student.tests/pa29/evidence173/ordinary-elf.json)
shows 1,200 local operators becoming weak definitions, 1,200 → 2,400 COMDAT groups,
and 3,621 → 6,021 sections. This is required inline/template ODR metadata, validated
by host symbol and cross-TU identity checks. It is not optimizer code growth.
Compiler RSS on that input grows **38,536 → 40,492 KiB** (5.08%); source recipes and
host linkage/object sections are necessary semantic costs. No speedup offsets are
assumed, and no extra optimizer or speculative work has been added.

## New capability costs and work bounds

The entry compiler rejects all six inputs, so these are required capability costs,
not equivalent A/B speed comparisons. Each workload/mode has eight final samples.
`closuresN` demands N enclosing specializations, each with one captured generic
lambda called with int, long, then int again. `callsN` demands N distinct non-type
operator specializations from one captured lambda, plus a separate runtime lambda.
Each executable checks all demanded calls and then runs 20 million checked loop
iterations on argc-dependent values.

| Workload | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| closures600 | 0.2095 [0.2057–0.2278] | 34708 | 0.3448 [0.3370–0.4539] | 1428 | 126955 |
| closures1200 | 0.4718 [0.4247–1.0498] | 61944 | 0.3564 [0.3364–0.4272] | 1684 | 254155 |
| closures2400 | 0.9673 [0.8856–1.1422] | 116072 | 0.3486 [0.3358–0.4242] | 1976 | 508555 |
| calls600 | 0.0516 [0.0488–0.0566] | 13728 | 0.1934 [0.1846–0.2206] | 1344 | 40402 |
| calls1200 | 0.0938 [0.0902–0.1344] | 19968 | 0.1881 [0.1832–0.1952] | 1368 | 80602 |
| calls2400 | 0.1801 [0.1739–0.1967] | 32608 | 0.2801 [0.1970–0.3291] | 1480 | 161002 |

Launcher median is **0.00345 s**, full range **0.00327–0.03858 s**.
The shortest capability compile median is **14.9×** launcher median;
the shortest runtime median is **54.5×**. Large scheduling outliers
are retained. No latency exponent or significance claim follows from these runs.

[All-sample assertions](../student.tests/pa29/evidence173/scaling-counters.json),
reproduced by `python3 student.tests/pa29/analyze173.py`, check deterministic work
and text in all final compile observations (excluding elapsed-time and RSS fields):

- `closuresN`: N closures/capture edges, **one** source capture recipe/candidate,
  **3N** specializations/body transitions, **3N+2** body checks, text **212N−245**.
- `callsN`: two closures/capture edges/recipes/candidates, **N+1** specializations
  and body transitions, **N+4** body checks, text **67N+202**.
- Repeated calls reuse concrete operator specializations. Emitted text grows with
  required distinct ABI entries; capture scanning does not multiply by call count.

## Ownership, budgets and acceptance

Source parsing/binding owns canonical lambda head parameters, lexical lookup,
query recipes and nested capture dependencies. Instantiation uses immutable
renaming/enclosing frames and creates closure fields before calls/layout.
Deduction selects ordinary function-template specializations; selected bodies
publish typed calls, conversions, captures, exception and lifetime facts. Pointer
adapters reuse those bodies. LowIR consumes these facts, native MIR/ELF consumes
LowIR, and typed ABI declarations preserve head, placeholder return and direct
versus general decltype identity. There is no synthetic function AST or textual
phase transport, unrelated body demand, process-global cache or global invalidation.

Required work/storage tracks source nodes, capture edges, demanded specializations
and produced IR, plus language-required overload candidates. Source recipes and
adapters use contiguous TU storage and complete-identity flat indexes. Traversal
scratch ends with the operation; semantic records end with the TU; function
lowering ends with its function. All measured existing common counters stay fixed;
new capability counters above bound the changed path. This is evidence for these
inputs, not a universal timing-complexity proof.

Optional optimizer work and code-growth budgets are **zero**. The generator limit
**1,048,576**, constexpr evaluator **1,000,000 steps/512 depth**, native frame/data
**0x70000000**, alignment **4096**, and course timeouts are unchanged. Necessary
semantic and ODR metadata costs are disclosed above; common text stays unchanged.
The inherited blanket 15% latency/RSS and zero-growth gates remain diagnostic under
spec §9; no mandated limit, correctness requirement or coverage was weakened.
Historical [performance172](performance172.md), [performance171](performance171.md)
and [performance170](performance170.md) remain intact. PA30–34 own heavier hosted
runtime, optimization/allocation and self-hosting; these are not PA29 exit gates.

Reproduce measurements with frozen entry/final compilers:

```sh
python3 student.tests/pa27/performance147_common.py OUT_COMMON ENTRY FINAL
python3 student.tests/pa29/performance173.py OUT_CLOSURE ENTRY FINAL
```
