# PA21 implementation 110 performance evidence

This campaign measures the completed handler/array/static ownership and generated
construction group at PA21/O0. It does not certify stage completion: three
required LowIR comparisons remain. The entry compiler is `d9528e58`; intermediate
code is `bdfdb31b`; final code is `df6e8299`. Frozen executable identities are:

| Compiler | SHA256 | Compiler .text bytes |
|---|---|---:|
| A, entry | `d9e59d64bb1560cf34d86b1236324597e0baeb54ff1bc8cdbbf52966403b1c57` | 2213958 |
| B, before helper sharing | `518f14eed8235f50811e3cd8b3992e3ccf827154e2a90e2d2f392fbb034b0344` | 2221958 |
| C, final | `ac7e58d3cf0c63016f0abf773b5f7cbab104459088aae1b18d6351b1892dc418` | 2222022 |

Final compiler text adds **8064 bytes (0.364%)**. The [primary driver](../student.tests/pa21/benchmark110.py)
freezes source, binary, backend and harness hashes, fixes CPU affinity and flags,
and records one warmup per binary, four A/A samples and four ABBA blocks.
Compilation and execution are timed separately. `/usr/bin/time` supplies peak
RSS; untimed `--stats --validate-lowir` supplies phase/work/IR evidence. Every
executable checks its result before timing and on every measured invocation.
Volatile loop bounds retain calls, memory, floating point and lifetime work.
Short compiler and executable startup controls are labeled as such; no speed
claim is based on them.

The original [A/B observations](../student.tests/pa21/performance110.json),
[template/initializer follow-up](../student.tests/pa21/performance110-supplement.json)
and [prvalue-helper measurements](../student.tests/pa21/performance110-prvalue.json)
are preserved, including unfavorable and noisy samples. The first template A/A
range was **0.700–1.141 s** and A/B paired ratios **0.943–1.268**; the follow-up
ratios were **0.925–1.015**, with unchanged emitted LowIR. This does not establish
a template regression or speedup. Initial variable-initializer controls exercised
an unchanged direct initializer path. The prvalue follow-up explicitly verifies
that final output contains the changed helper entries; the earlier observations
remain useful unaffected-path controls and are not relabeled as helper evidence.

Prvalue measurements found an avoidable cost in B: 256/1024 occurrences created
256/1024 identical zero-argument omitted-constructor helpers. Their host text was
**14877/57885 bytes**, versus **6685/25117** at entry. C shares these by canonical
aggregate type and supplied-prefix arity. An empty-list zero-argument constructor
is fixed by the canonical field type and creates no helper-local argument
identity. Defaults with arguments and all other occurrence-owned recipes keep
the complete plan key. The extra classification is constant work per initializer
action; it introduces no search, fixed point or global invalidation. Prvalue
controls cover self-addresses, two call sites, default temporary identities,
constructor failure, argument failure and throwing temporary cleanup.

Required costs remain distinct from an optimization claim. Retaining empty
copy entries increases B's checked four-million-call loop from **93.72 to
119.76 ms**, with paired B/A **1.260–1.486** and host text **725 → 779 bytes**.
Removing those entries would restore a required comparison failure. The supplied
O0 backend retains these calls; later native inlining/elimination belongs to
PA24 and optimizer stages. Required scalar representation prefixes restore the
course O0 copy form; no small-copy speedup is claimed. The normal list loop
measures only normal-path cost because A's exceptional destruction is incorrect.
It is not a valid optimization baseline for exception behavior.

The final [A/C campaign](../student.tests/pa21/performance110-final.json) and
[prvalue follow-up](../student.tests/pa21/performance110-prvalue-final.json) retain
all observations against the final compiler. Each range below covers the four
paired C/A observations; latency/runtime columns are medians in milliseconds,
RSS is peak KiB. **T** is actual ELF `.text` from the supplied object backend and
host linking, excluding shared runtime code. **P** is the supplied freestanding
backend's sectionless executable payload proxy, which can include static data
and EH tables; it is not mislabeled as `.text`.

| Workload | Compiler ms A → C | RSS KiB A → C | Compiler paired C/A | Runtime ms A → C | Runtime paired C/A | Native bytes A → C |
|---|---:|---:|---:|---:|---:|---:|
| startup | 11.12 → 10.84 | 5800 → 6052 | 0.870–0.970 | 5.20 → 5.14 | 0.956–1.138 | 24 → 24 P |
| auto-specializations-9600 | 1380.15 → 1417.79 | 107844 → 107880 | 0.952–1.012 | 6.96 → 6.45 | 0.903–1.013 | 768062 → 768062 P |
| runtime-calls | 14.01 → 13.24 | 6060 → 6236 | 0.921–0.972 | 193.48 → 183.43 | 0.944–0.978 | 206 → 206 P |
| runtime-memory | 9.40 → 9.53 | 6092 → 6284 | 1.004–1.031 | 115.68 → 118.01 | 0.953–1.050 | 434 → 434 P |
| runtime-floating | 9.09 → 8.72 | 6368 → 6240 | 0.783–0.976 | 129.17 → 127.01 | 0.954–1.014 | 230 → 230 P |
| runtime-reference-captures | 9.74 → 9.23 | 6116 → 6284 | 0.916–1.004 | 38.26 → 38.98 | 0.939–1.028 | 188 → 188 P |
| runtime-class-lists | 11.25 → 12.05 | 6108 → 6252 | 0.985–1.104 | 96.09 → 95.93 | 0.961–1.016 | 1664 → 1584 P |
| runtime-scalar-prefix | 11.88 → 11.74 | 6120 → 6288 | 0.909–1.085 | 140.97 → 135.95 | 0.925–1.012 | 820 → 787 T |
| runtime-empty-copy | 13.00 → 12.91 | 6112 → 6124 | 0.871–1.031 | 107.31 → 132.10 | 1.151–1.339 | 725 → 779 T |
| runtime-omitted-member | 11.41 → 11.36 | 6120 → 6240 | 0.942–1.027 | 78.82 → 79.29 | 0.954–1.067 | 691 → 691 T |
| runtime-list-normal | 14.42 → 14.03 | 6112 → 6276 | 0.911–1.028 | 99.57 → 100.49 | 0.973–1.018 | 774 → 906 T |
| omitted-functions-256 | 48.12 → 47.37 | 9440 → 9488 | 0.967–1.087 | 7.72 → 7.96 | 0.899–1.071 | 15891 → 15891 T |
| omitted-functions-1024 | 145.39 → 148.61 | 18768 → 18820 | 0.890–1.089 | 7.37 → 7.29 | 0.926–1.145 | 61971 → 61971 T |
| runtime-prvalue-helper | 10.17 → 9.98 | 6120 → 6236 | 0.890–1.085 | 75.64 → 91.24 | 1.191–1.226 | 729 → 761 T |
| prvalue-functions-256 | 30.16 → 28.53 | 7868 → 7852 | 0.906–1.004 | 8.35 → 8.88 | 0.963–1.076 | 6685 → 4677 T |
| prvalue-functions-1024 | 86.07 → 79.27 | 13172 → 12552 | 0.908–1.059 | 8.46 → 8.70 | 0.936–1.129 | 25117 → 16965 T |

`startup`, template and function-count executable timings are startup controls,
not throughput measurements. Each `runtime-*` source checks a live loop.
Absolute machine timings changed between campaigns (entry template median
0.72 s originally versus 1.38 s finally); compare within each interleaved campaign,
not across dates. Final template A/A spans **1.327–1.492 s**, paired C/A is
**0.952–1.012**, peak RSS differs by **36 KiB**, and emitted LowIR is identical.
No template performance regression or improvement is established. Short compiler
measurements also include process startup and exhibit noise; their complete
A/A and ABBA samples remain in the records.

The retained empty-copy loop has final paired C/A **1.151–1.339**; the required
prvalue-helper call has **1.191–1.226**. These are disclosed O0 costs, not claimed
optimization wins. Normal list execution has **0.973–1.018**, with text growth
**774 → 906 bytes** for required surviving-element unwind work. Its old exception
path remains invalid, so that comparison is explicitly normal-path-only.
Scalar-prefix execution has paired **0.925–1.012**, which does not establish a
repeatable speed benefit. None of these measurements licenses removing required
entries, protected calls, temporary ownership or destruction.

Final prvalue-helper count is **one** at both 256 and 1024 occurrences. Host text
is **4677/16965 bytes**, exactly **16n + 581**, versus entry **24n + 541** and
intermediate **56n + 541**. Final instructions are **581/2117 = 2n + 69**, versus
entry **5n + 59**. Semantic initializer actions remain **3n**, list objects **2n**,
and expression work **4n + 47**; no semantic coverage was removed. At 1024,
compiler peak RSS is **13172 → 12552 KiB** and paired compile C/A **0.908–1.059**.
Sharing eliminates the measured code/work duplication; it does not remove the
required call boundary or claim a runtime speedup. The one-helper runtime has
the same **761 bytes** as B.

New throwing-destructor work is measured separately because entry skips the
remaining elements and is not a correct baseline. Every final-only executable
throws from the first destroyed element, catches `int`, and verifies zero live
objects and the exact total destruction count. Each compiler/runtime has one
warmup and four retained samples:

| Array extent | Compiler median ms | Peak RSS KiB | Runtime range ms | Host .text bytes | LowIR instructions |
|---|---:|---:|---:|---:|---:|
| 8 | 11.72 | 6200 | 201.27–213.12 | 1498 | 297 |
| 9 | 11.80 | 6124 | 148.09–152.00 | 1046 | 148 |
| 64 | 11.17 | 6204 | 34.23–35.25 | 1082 | 148 |
| 1024 | 10.72 | 6144 | 28.35–29.50 | 1178 | 148 |

Extents 8/9/64 execute 50000/44444/6250 rounds; extent 1024 executes 1000 rounds.
Thus runtime columns describe different checked workloads, not a speed ratio
across extents. Beyond eight elements, the instruction count stays **148** and
serialized LowIR grows only **6000 → 6009 bytes** from extent 9 to 1024. Native
frame/address encoding adds a bounded 132 bytes over that range. The normal loop
counter and exceptional remaining-prefix counter preserve all element lifetimes
without unrolling large arrays. Final-only evidence is cost/scaling evidence,
not an optimization comparison against invalid entry behavior.


Spec §9 acceptance is stage-scoped. All earlier performance evidence, including
[109](performance109.md), remains intact. Historical **+15%, +16 MiB and 5.5×**
figures are diagnostics rather than new exit gates. This change adds required O0
behavior, not an optional optimizer whose runtime benefit could waive its cost.
The measured duplicate-helper growth was avoidable and removed. Other required
call/cleanup costs are disclosed. The binding budgets remain proportional work
in consumed/emitted actions, O(1) expected typed-cache lookup, O(b log b) existing
block ordering, and **at most eight** expanded elements before counted loops.
Function-local address/lifetime caches reset per function; TU semantic recipes
and emission identities are released with the translation unit. No native
backend or self-hosting performance claim is made before those stages.
