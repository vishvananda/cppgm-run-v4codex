# PA25 implementation140 performance evidence

Code: `9655db738a8fdaa5e51e03ca7bd359f38e8e2951` supplies correct source
floating evaluation; `e7f66a25` adds exact power-of-two instruction selection.
Exception/runtime correctness is mandatory PA25 work. The small optional
selection is accepted on measured runtime benefit with zero code-growth budget.
The inherited 15% timing/RSS targets are diagnostics under spec section 9,
not mandated stage limits. Historical evidence, native bounds and coverage remain.

## Protocol and frozen versions

[performance.py](performance.py) freezes binary and input hashes and `-O0`.
Each workload/mode has four A/A observations then six ABBA blocks. Compiler
latency and runtime are measured separately; `/usr/bin/time` records peak RSS.
Programs execute with checked argc-dependent results before measurement. Every
observation, including outliers, is retained. No build or correctness runner ran
concurrently with measurement. There are **952 new observations**.

| Version | SHA256 | Meaning |
|---|---|---|
| A | `82485193297359ed27750a9bcc1daad92208fc0e351c4978512c9833209c9152` | Entry, 86/101 |
| B | `a71c438ed1bd2f4b182e74034c4b9bd442119eea96cfd284c963381b186cb0be` | Correct F80 source evaluation, 101/101 |
| C | `26ca4cba8f2fe988d34a2a663c6da97ee0b63deddd4c92cc53b96c01a487e48d` | Final exact-scale selection |

The raw manifests retain an inherited generic `policy` string saying no optional
transform/speedup claim. That string is inaccurate for the B/C scale comparison;
this report identifies the versions and interpretation. The harness wording is
corrected for future runs; historical observations are unchanged.

## Equivalent entry/final workloads

Templates demand 4,800 specializations across nine TUs, one invocation per
compiler sample. Memory and floating compiler samples batch 64 invocations.
Every runtime sample batches three executions; memory/floating perform three
million steps per execution. Templates and memory produce byte-identical A/C
images. Floating images differ but have equivalent checked results on this
workload; A's unrelated calculator rounding defect is not an oracle.

Ratios below are medians of within-block C/A mean ratios, with full ranges.

| Workload | C compile batch s | Compiler KiB A / C | Compile C/A [range] | Compile A/A s |
|---|---:|---:|---|---|
| templates | 0.26196 | 15396 / 15248 | 1.042 [0.952, 1.111] | 0.251–0.252 |
| memory | 0.60454 | 6856 / 6816 | 1.009 [0.695, 1.088] | 0.352–0.363 |
| floating | 0.66153 | 6996 / 6932 | 1.017 [0.966, 1.076] | 0.568–0.668 |

| Workload | C runtime batch s | Runtime C/A [range] | Runtime A/A s | Text bytes A / C |
|---|---:|---|---|---:|
| templates | 0.11711 | 1.001 [0.993, 1.002] | 0.1170–0.1173 | 384567 / 384567 |
| memory | 0.24696 | 1.004 [0.940, 1.047] | 0.273–0.289 | 521 / 521 |
| floating | 0.21801 | 1.049 [1.028, 1.116] | 0.206–0.259 | 340 / 362 |

Final wall times vary substantially from the earlier measurements even for
byte-identical images. These samples establish no compiler speedup. They retain
the measured roughly 4.9% floating runtime cost and 22 extra text bytes versus
entry: general arithmetic now executes the required extended evaluation. The
paired spread and noise calibration limit the precision of that estimate.

## Resolve the avoidable precision cost

The first correct implementation B incurred floating runtime B/A **1.235
[1.229, 1.240]**, batch median **0.17093 s**, text **340 -> 377**. Its compiler
batch median was **0.36697 s**, peak **7060 KiB**, compile B/A **1.015
[0.970, 1.071]**. Treating this whole cost as unavoidable would miss a legal
selection opportunity. Multiplication by a constant power of two is exactly
representable in binary80, so only the declared-precision rounding remains.

The frozen correct-B/correct-C comparison isolates that selection:

| Workload | C compile batch s | Compiler KiB B / C | Compile C/B [range] | Compile B/B s |
|---|---:|---:|---|---|
| templates | 0.26449 | 15492 / 15380 | 1.009 [0.989, 1.041] | 0.257–0.263 |
| memory | 0.35646 | 6956 / 6956 | 1.013 [0.977, 1.033] | 0.360–0.375 |
| floating | 0.34573 | 7084 / 7092 | 1.009 [0.916, 1.036] | 0.338–0.350 |

| Workload | C runtime batch s | Runtime C/B [range] | Runtime B/B s | Text bytes B / C |
|---|---:|---|---|---:|
| templates | 0.11765 | 1.002 [0.993, 1.009] | 0.116–0.141 | 384567 / 384567 |
| memory | 0.14844 | 1.001 [0.982, 1.007] | 0.1484–0.1486 | 521 / 521 |
| floating | 0.14096 | 0.827 [0.824, 0.838] | 0.1705–0.1712 | 377 / 362 |

The affected runtime improves consistently across all six blocks (median
17.3%), and text shrinks by 15 bytes. Compiler/RSS differences are small relative
to observed variation; unaffected executable pairs are byte-identical. The
explicit budget is two fixed-width operand bit tests per multiply, no search,
no allocation, no new IR nodes, no additional frame bytes, and zero native text
growth. The MIR proof selects shorter encoding and removes x87 operand
spill/reload traffic. Unknown operands keep conservative F80 evaluation. There
is no persistent analysis to invalidate. The source-to-ELF scale trace and
120,120 float/double comparisons check legality separately from profitability.

## New behavior absolute baselines

These final C/C comparisons calibrate noise; they do not claim speedups.
Runtime samples contain three executions. `classes` has 400 demanded derived
templates and eight million calls per execution (compile batch 2). `casts`
checks 800,000 adjustments (compile batch 64); `allocations` checks 10,000
new/delete pairs and destruction through a secondary base (compile batch 64).
`exceptions` performs 60,000 throws, base catches, rethrows and by-value copies,
checking 120,000 destructions and 60,000 copies (compile batch 32).

| Workload | Compile batch s | Compiler peak KiB | Runtime batch s | Text bytes |
|---|---:|---:|---:|---:|
| classes | 0.35429 | 19856 | 0.92372 | 68368 |
| casts | 0.74455 | 7100 | 0.17328 | 1682 |
| allocations | 0.79104 | 7100 | 0.67410 | 5255 |
| exceptions | 0.43315 | 7116 | 3.85395 | 10652 |

| Workload | Compile ratio [range] | Compile A/A s | Runtime ratio [range] | Runtime A/A s |
|---|---|---|---|---|
| classes | 0.998 [0.898, 1.050] | 0.367–0.424 | 0.995 [0.952, 1.064] | 0.805–0.848 |
| casts | 0.973 [0.950, 1.025] | 0.810–1.101 | 0.997 [0.872, 1.120] | 0.173–0.232 |
| allocations | 0.987 [0.950, 1.065] | 0.716–0.761 | 1.028 [0.903, 1.091] | 0.584–0.640 |
| exceptions | 1.026 [0.939, 1.101] | 0.394–0.435 | 1.008 [0.936, 1.069] | 3.644–3.827 |

Runtime peak RSS is 256 KiB in every workload. Allocation text grows from the
historical 2700 bytes to 5255 because allocation failure now throws a correctly
typed exception rather than terminating. This mandatory service is demanded
once, not cloned at every allocation or TU. Classes/casts retain their previous
text size. Initial B/B absolute baselines remain in the raw records; all four
images are byte-identical to the corresponding final C/C images.

Exception-clause indexing is linear in blocks/markers. Handler records occupy
96 stack bytes per active registration; payload headers occupy 80 heap bytes,
released after the final handler/destructor. Matching follows hierarchy paths,
with depth-proportional stack, not a claimed linear graph traversal. The typed
runtime has a finite role set; linker demand is indexed and support IR dies
after native construction. These are necessary semantic costs with explicit
ownership. [Implementation140](implementation140.md) traces the data flow.
PA34 owns self-hosting; later stages own O1–O3 policies. No later-stage target
is manufactured into a PA25 exit gate.

## Preserved records

Paths below are relative to `/home/vishvananda/work/private/v4codex/artifacts/pa25-140/`.
Each directory contains `performance.json`, frozen inputs and generated A/B
images. [Validation140](validation140.json) pins and verifies every manifest,
including binary/input and executable hashes. All earlier reports remain intact.

| Directory | Pair | Observations | performance.json SHA256 |
|---|---|---:|---|
| performance-common | A/B | 168 | `e4346c4367f7bba7cc77393d1241fdd13b2d3f3748d655b1b434b64d39b7a1b5` |
| performance-scale | B/C | 168 | `892174006aa50f5ef696241fcadf9c40d61220cdd4d2a5ba6e07ef5364df569d` |
| performance-final | A/C | 168 | `42ddac56bced4f0b9f7259a89621ffed7a411d3a897895f748025e0146b310b1` |
| performance-exceptions | B/B | 56 | `86f6fa0be36bc0302287fcb54450fc94985adfa550b435b8d0ed509e81ddc49b` |
| performance-exceptions-final | C/C | 56 | `6cb3fb6186f456d58d982d8635c99d60382c47f716c1e8b54b4a68c885f25fa3` |
| performance-classes | B/B | 168 | `f2fa977bb563fe878b7be854b8787ac723d2c510980e4d3398ad85e496321697` |
| performance-classes-final | C/C | 168 | `575133b6c94bd94068ee0e66f03740cef1012fecedabe9be7037ad3d42309841` |
