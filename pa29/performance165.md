# PA29 implementation165 performance evidence

Final code: `db0d088961f5f637892c833d0fb8079a8a583892`. Entry: `61e023f39a4c1bafdccc484257ab51df98f87088`.
Level: PA29/O0. This adds required evaluation-mode and initialized-storage semantics; no optional optimization or speedup is claimed.

## Frozen protocol

[Manifest](../student.tests/pa29/evidence165/performance-manifest.json) records compiler hashes, sizes, flags, script hashes, CPU affinity and platform. A is the entry compiler; B is the final compiler (`compiler-C` on disk). Both remain frozen. Compiler bytes: **4057576 → 4074632** (+0.42%).

Common inputs use the inherited checked loops/calls/memory/floating/exception workloads plus 2,400 demanded template specializations; pruning adds 1,200 unused functions. Four A/A samples precede six ABBA blocks for each workload and for compilation and execution separately: 224 observations. `-O0 -c --stats`, CPU 0; host linking is outside compiler timing. Both versions pass every checksum, and their objects **and executables are byte-identical** for all four inputs.

Affected inputs use `-std=c++14 -O0 -c --stats`, 600/1,200/2,400 demanded instances, eight compilation and eight runtime samples each: 96 observations. Every executable checks both demanded results and a 20-million-call loop driven by `argc`. The entry compiler rejects all six inputs for the missing intrinsic; [entry evidence](../student.tests/pa29/evidence165/affected-baseline.json) therefore precludes an affected A/B speedup claim. These are necessary semantic cost and scaling measurements.

All final observations, including outliers, phase counters, hashes, RSS and image sizes: [common](../student.tests/pa29/evidence165/common-performance.json), [affected](../student.tests/pa29/evidence165/affected-performance.json). No course or other compiler tests ran concurrently with either final timing batch.

## Common compiler cost

Latencies are wall seconds; RSS is maximum KiB. The ratio is the median of six paired block ratios, not the ratio of overall medians.

| Input | A/B median s | A/B peak RSS | Paired B/A | Pair range | A/A s range |
|---|---:|---:|---:|---:|---:|
| memory | 0.2515/0.2580 | 29196/29608 | 1.0328 | 0.8330–1.3288 | 0.2512–0.3174 |
| floating | 0.2753/0.2713 | 29592/29800 | 0.9754 | 0.4862–1.0681 | 0.2640–0.2738 |
| exceptions | 0.2716/0.2658 | 29472/29768 | 0.9952 | 0.9103–1.2280 | 0.2328–0.2633 |
| pruning | 0.3292/0.3258 | 35088/35628 | 0.9953 | 0.8368–1.0142 | 0.3215–0.3718 |

## Common executable cost

| Input | A/B median s | Paired B/A | Pair range | A/A s range | A=B text bytes |
|---|---:|---:|---:|---:|---:|
| memory | 0.0730/0.0731 | 0.9997 | 0.9497–1.0441 | 0.0586–0.0648 | 151633 |
| floating | 0.0594/0.0579 | 0.9832 | 0.8888–1.0190 | 0.0524–0.0674 | 151474 |
| exceptions | 0.4556/0.4712 | 1.0420 | 0.9768–1.0798 | 0.4487–0.4534 | 151781 |
| pruning | 0.0675/0.0671 | 0.9627 | 0.8926–1.1002 | 0.0526–0.0772 | 151633 |

Common medians are near parity. RSS grows by 208–540 KiB on these inputs. Wide paired ranges, including an A-side floating compilation outlier, prevent a precise latency claim. Runtime variation cannot represent a generated-code regression here because the executables are identical.

## Affected necessary costs

| Family / demands | Compile median [range] s | Peak RSS KiB | Runtime median [range] s | Text bytes |
|---|---:|---:|---:|---:|
| evaluation600 | 0.1575 [0.1566–0.1590] | 18440 | 0.3379 [0.3364–0.3401] | 95873 |
| evaluation1200 | 0.3000 [0.2845–0.3042] | 29716 | 0.3300 [0.3260–0.3328] | 191273 |
| evaluation2400 | 0.3664 [0.3518–0.6037] | 52224 | 0.1714 [0.1710–0.1804] | 382073 |
| storage600 | 0.2101 [0.2080–0.2138] | 30936 | 0.1342 [0.1327–0.2029] | 112057 |
| storage1200 | 0.4338 [0.4258–0.6750] | 54504 | 0.2698 [0.2682–0.2715] | 223657 |
| storage2400 | 0.8675 [0.8490–0.8876] | 107844 | 0.1358 [0.1344–0.1501] | 446859 |

Launcher median **0.0053 s**, range **0.0048–0.0094 s**. The shortest affected execution is **25.2×** that median; common runtime medians are also above the launcher scale. Compilation and execution are separately measured. Runtime loop work is fixed across demand counts; the roughly twofold timing shifts between batches do not establish a scaling benefit. Scheduling variation is a plausible contributor, but these measurements do not separate it from code-layout effects. All generated code and checksums remain available by the recorded hashes.

`evaluation` executes one template in both required-constant and ordinary contexts. `storage` adds a constexpr class, reference-bound temporary, ordinary array proof and nested constant activation. For demand count N, observed counters are exactly:

| Family | Mode observations | Mode values | Mode activations | Initialized objects | Reference plans |
|---|---:|---:|---:|---:|---:|
| evaluation | N | N | N+1 | N | 0 |
| storage | 4N | 2N | 3N+2 | 3N | N |

Every sample at all three sizes follows these counts. Sparse dependence facts and reference plans grow with demanded work, and the storage family compiler median scales approximately 1:2:4. RSS and text increase with required specializations/storage. Evaluation timing has broader host variation; its counters and image sizes remain linear. Mode lookups are average O(1), cache variants are capped at two per existing evaluation key, and reference rebasing is O(recorded subobject depth). These changes do not traverse unrelated declarations or require a global pass.

## Retained measurements and acceptance

[Preliminary manifest](../student.tests/pa29/evidence165/preliminary-manifest.json), [common observations](../student.tests/pa29/evidence165/preliminary-common.json), [affected observations](../student.tests/pa29/evidence165/preliminary-affected.json) and [entry rejections](../student.tests/pa29/evidence165/preliminary-affected-baseline.json) preserve all 320 earlier samples and six launcher observations from the initial group commit. A later reference-alias control exposed an initialization/storage distinction; the final binary includes its correction. The final storage generator additionally demands nested constant activations, so its text/timing is not an equivalent-input comparison with the preliminary storage generator. No preliminary measurement is discarded or presented as final acceptance.

New optional work and growth budgets are **zero**: this is required semantic evaluation and materialization, not an optional transform. Existing 1,000,000-step/512-depth constexpr limits, native/frame/data/alignment limits and assignment timeouts remain. The common objects show no code-growth regression; new necessary compiler state and affected emitted bytes are disclosed above.

Historical blanket 15% latency/RSS and zero-growth targets remain diagnostics under spec §9, as already classified in [implementation164](performance164.md). They are not mandated PA29 limits: corrected, necessary semantic work is judged at PA29/O0, with equivalent correct outputs and preserved measurements. No correctness or coverage requirement is waived. Broader hosted runtime, optional optimization/allocation and self-hosting remain PA30–34 work. Independent review must still assess ownership, cache completeness and whether any measured cost is avoidable.
