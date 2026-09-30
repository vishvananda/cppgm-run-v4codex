# PA25 final audit141 performance evidence

Reviewed code: `67ea755a273223b7fcc973215e0ca1ec8131689d`.
Correctness repairs carry binding, temporary ownership and storage layout across
phase boundaries. This audit adds no optional optimization. The retained exact-
scale selection is evaluated separately against its correct F80 predecessor.

## Frozen protocol and versions

[performance.py](performance.py) pins `-O0`, compiler/input hashes and executable
images, checks every result, then measures compilation and execution separately.
Each workload/mode has four A/A observations and six ABBA blocks, with wall time
and `/usr/bin/time` peak RSS. Ratios are medians of the six within-block ratios of
B mean time to A mean time; brackets give the full range, not a confidence interval.
Every observation is retained. No build or correctness runner ran during timing.

The final code has **448 observations** across eight workloads. Another **840**
observations of intermediate audit repairs remain intact: **1288** new observations
in total. Intermediate filenames containing `final` are historical labels; only
`cppgm-final-code` and `performance-final-*` identify the committed final code.

| Frozen binary | SHA256 | Role |
|---|---|---|
| `cppgm-entry` | `26ca4cba8f2fe988d34a2a663c6da97ee0b63deddd4c92cc53b96c01a487e48d` | A: entry e6cdc480, correct on shared benchmarks |
| `cppgm-final` | `785d93c5dc489343dd2653bf24a0e52c4a98bb9176ee0f4ed81fe563ee7630aa` | Intermediate EH repair |
| `cppgm-reviewed` | `88d793d695a93b977f2a5cf6f1f5ba463d673de79f2e1c109194fdb44630b828` | Intermediate complete storage/binding repair |
| `cppgm-final-code` | `283e3987a3084961b52aece8957d644cfff63bfa8b322f9d5dd031139b6ac6ad` | B: exact reviewed code, including native clause bounds |

Entry/final pairs are equivalent correct implementations for the seven established
workloads. New reference-binding behavior uses final/final calibration because the
entry implementation is incorrect there. No speedup is inferred from that baseline.
Compiler samples batch 1 invocation for templates, 64 for memory/floating/casts/
allocations, 2 for classes, and 32 for exceptions/binding. Runtime samples batch
three executions. All times below are **batch seconds**, not per-invocation times.

## Final compiler and executable measurements

| Workload | B compile s | Peak compiler KiB A / B | Compile B/A [range] | Compile A/A s |
|---|---:|---:|---|---|
| templates | 0.26354 | 14960 / 15084 | 1.017 [0.991, 1.028] | 0.2746–0.4305 |
| memory | 0.37210 | 6760 / 6896 | 0.953 [0.931, 0.965] | 0.3818–0.4120 |
| floating | 0.36911 | 6880 / 7056 | 0.950 [0.912, 1.030] | 0.4058–0.4279 |
| exceptions | 0.23922 | 7056 / 7184 | 0.959 [0.945, 1.014] | 0.2491–0.2633 |
| classes | 0.29504 | 19808 / 19832 | 0.987 [0.886, 1.187] | 0.3172–0.3369 |
| casts | 0.81032 | 7052 / 7184 | 0.943 [0.869, 1.088] | 0.7527–0.8405 |
| allocations | 0.75765 | 7060 / 7200 | 0.929 [0.861, 1.000] | 0.7592–0.8485 |
| binding | 0.40908 | 7152 / 7152 | 0.997 [0.844, 1.132] | 0.3481–0.4032 |

| Workload | B runtime s | Runtime B/A [range] | Runtime A/A s | Text bytes A / B |
|---|---:|---|---|---:|
| templates | 0.11758 | 1.001 [0.996, 1.005] | 0.1174–0.1183 | 384567 / 384567 |
| memory | 0.15058 | 1.005 [0.991, 1.013] | 0.1515–0.1670 | 521 / 521 |
| floating | 0.14229 | 0.997 [0.990, 1.008] | 0.1415–0.1438 | 362 / 362 |
| exceptions | 2.96652 | 0.997 [0.755, 1.071] | 2.1173–3.8853 | 10652 / 11109 |
| classes | 0.85624 | 1.029 [0.970, 1.167] | 0.7702–0.7953 | 68368 / 68368 |
| casts | 0.18211 | 0.943 [0.873, 1.051] | 0.1658–0.1958 | 1682 / 1682 |
| allocations | 0.55956 | 1.016 [0.914, 1.041] | 0.5385–0.5897 | 5255 / 5544 |
| binding | 2.66565 | 0.995 [0.852, 1.069] | 2.4872–2.9563 | 8193 / 8193 |

Measured runtime peak RSS is 256 KiB for every final workload. Executable file
sizes and all individual RSS observations are in the manifests. Templates demand
4800 specializations across nine TUs and execute 1.92 million calls per run.
Memory and floating execute three million argc-dependent steps. Classes instantiate
400 derived templates and execute eight million calls; casts perform 800,000
adjustments; allocation performs 10,000 new/delete pairs through a secondary base.
Exceptions check 60,000 throws/rethrows, 120,000 destructions and 60,000 catch copies.
Binding performs 20,000 iterations of pointer-reference mutation/rethrow and nested
nullptr-to-member-pointer const-reference catches, checking values and lifetime.

Five existing final images (templates, memory, floating, classes, casts) are byte-
identical to entry. Final wide/statement images were also rebuilt and executed
against the frozen entry using the inherited fixed inputs: both remain identical,
with **977 / 503** text bytes. Their prior calibrated baselines remain preserved;
a single verification execution is not presented as a new timing measurement.

Required EH support grows exception text **10652 -> 11109** (+457, 4.29%) and
allocation text **5255 -> 5544** (+289, 5.50%). The latter includes correctly typed
allocation-failure matching even when the benchmark's allocations succeed. This is
one finite demanded support implementation, not cloning per throw, handler or TU.
Exception/allocation file sizes remain **12880 / 8960** bytes respectively. New
binding temporary storage is bounded by one pointer or two-word member-pointer
slot per applicable handler; payload headers stay 80 bytes and registrations 96.

Host variation is visible even for identical executable pairs. The intermediate
complete-repair exception median was 2.08308 s, with B/A 1.002 [0.992,1.011]; the
final paired spread above and changing absolute time are preserved. Repeated
measurements of identical code are evidence against attributing these shifts to
optimization. No new compiler or runtime speedup is claimed. The final samples
show no avoidable runtime regression attributable to these correctness repairs;
the extra semantic support text and compiler costs are disclosed, not hidden by
node-count reductions or by excluding noisy blocks.

## Legality, profitability and stage acceptance

`native/floating.cpp` retains the local exact-scale selection. Finite nonzero
power-of-two multiplication of binary32/64 inputs is exact in binary80; only the
final declared-precision rounding remains. The MIR proof selects SSE and removes
x87 operand spill/reload traffic. Unknown operands keep F80. Budget: at most two
fixed-size bit tests per multiply, no search/allocation/new IR/frame growth, and
zero native text growth. There is no persistent analysis to invalidate. The
120,120 float/double checks and production/MIR trace were rerun on final code.

The independently rehashed/recomputed correct-F80/correct-scale comparison in
[performance140](performance140.md) has runtime ratio **0.827 [0.824,0.838]**,
median benefit **17.3%**, and text **377 -> 362**. The final floating executable
hash equals that measured optimized image. This historical isolated comparison,
not fewer IR nodes or a new noisy entry/final timing, supports retaining the
optional selection. Required general F80 evaluation costs remain disclosed there.

The rest of the audit adds necessary constant work per declaration, clause or
initializer item, precise sparse facts and bounded handler storage. Selection,
allocation and emission retain their per-function budgets; six-bit CFG propagation
has at most six additions per edge; data and relocation work is linear apart from
the required foreign extent sort. Required PA24 behavioral/MIR bounds still pass.

Under spec section 9, inherited **15% latency/RSS** targets and blanket zero growth
for required semantic work are diagnostic targets, not assignment exit gates.
All observations, including historical misses, remain available. Necessary costs
do not excuse avoidable regressions; the earlier avoidable F80 scaling traffic was
removed only after legality and repeatable runtime benefit were established.
No unprofitable optional transform was added here. Self-hosting is PA34 work;
O1–O3 policies and host object/metadata interoperability belong to later stages.
Correctness, coverage, mandated limits and explicit optional growth budgets remain.

## Preserved manifests

All **21** inherited manifests recorded by validation138–140 were independently
verified: **3080 observations**, all frozen binary/input hashes, **110 executable
images**, and recomputed paired summaries. The review is pinned in validation141.
No measurements or references were revised. New paths below are relative to
`/home/vishvananda/work/private/v4codex/artifacts/pa25-141/`.

| Directory | Observations | performance.json SHA256 |
|---|---:|---|
| `performance-classes` | 168 | `227b1738e96f23a3d280672558fd181823a179a82dd7198c8246572f48362a84` |
| `performance-common` | 168 | `1dc90a1b756aa05879131ab1e6938f5f8158b9c3fea68863cb898a500bccf7f3` |
| `performance-exceptions` | 56 | `0b008332530ec9bf28731bc180c89c010af6154147bd7d3293aa2ac0219dd50f` |
| `performance-final-binding` | 56 | `f9bc382299e39e315f62f58d228d328859e70c7bda0af481207be7fefd91276d` |
| `performance-final-classes` | 168 | `72640343ebe0e80eab2842db40e248d5d80b73987e9252ca83ee4b47514c50b1` |
| `performance-final-common` | 168 | `42f7bcc992c425e2288c7b04d118c8c4be695aca7d61364c5918060a3c119e37` |
| `performance-final-exceptions` | 56 | `7743a07786133b04f91e2beb1f0b3dbf1be57d71e6d0e194709bfb9a40e9f462` |
| `performance-reviewed-binding` | 56 | `ee3fd7974824299b1a3e348766be949e56359b0360f29de182c14d4172c56ed3` |
| `performance-reviewed-classes` | 168 | `36a89355d7b70296a65cb6a8cbabca09318d5306d5184d2998ccb81cda549b66` |
| `performance-reviewed-common` | 168 | `9dd698f5ea21bd50604750c3c2ec59067f121c6f120c402f14584d58663db3ee` |
| `performance-reviewed-exceptions` | 56 | `0c74ba95a089768f90fae89160da4364ff3abb13edbebe1a7d6c8130bf9c9103` |

[Validation141](validation141.json) pins every log, manifest, image verification and final source boundary.
