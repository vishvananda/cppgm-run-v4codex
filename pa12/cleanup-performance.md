# PA12 full-expression cleanup evidence

Commit `8bf45f86` introduced one full-expression region owner and cached
cleanup/unwind classifiers. The source graph remains shared; lowering consumes
selected conversions and lifetime identities. Construction extends an immutable
active prefix; branch terminators close regions without destroying enclosing
temporaries. Scope-bound reference objects use lexical cleanup. Later destructor
refinements and removal of an unnecessary reference-initializer guard are recorded
separately; the measurements below remain the initial implementation evidence.

## Frozen protocol

[Harness](../student.tests/pa12/cleanup_benchmark.py) and
[all initial observations](../student.tests/pa12/cleanup-performance.json) retain
input/compiler/backend hashes, flags, warmups, four A/A observations and two ABBA
blocks. CPU 0 was pinned. Compilation uses `--emit-lowir -O0`; validation,
telemetry and supplied PA8 native backend `-O0` run outside compiler timing.
Correct A/B workloads check equal results; common LowIR/native bytes match.
The switch path checks B alone because A destroys its temporary too late.
Namespace programs measure compiler scaling; their native runtimes are dominated
by startup. Runtime programs use twelve million iterations, volatile bounds and
checked live counts and nonconstant results. No observations were discarded.

A is clean `3ec8d0ce`, `/tmp/pa12-cleanup-base-cppgm`, SHA-256
`0d67bd74b258cff8df51d4e369c96946c937c9058f569b616bb60c636ffdd3b9`.
B is `8bf45f86`, `/tmp/pa12-cleanup-final-cppgm`, SHA-256
`dfd4607a76a31d09da89ae3f5daa4f2bd899b7148bcfc1b16cfff64e54408e55`.
Scratch is `/tmp/pa12-cleanup-evidence`. Compiler text grows 5184 bytes,
943750 to 948934 (0.55%). Native text means sectionless executable payload
following the entry, as in earlier campaigns.

| Workload | Compiler median A/B seconds | Peak RSS A/B KiB | Paired compiler B/A | Native text A/B |
| --- | ---: | ---: | --- | ---: |
| Common 1000 | .25482/.25389 | 49708/50172 | .996/1.038 | 168056/168056 |
| Common 4000 | 1.04976/1.06301 | 184884/186152 | .968/1.011 | 672056/672056 |
| Branch 1000 | .42024/.45156 | 79520/89732 | 1.055/1.371 | 1232592/1462592 |
| Branch 4000 | 1.73749/1.88670 | 286188/303892 | 1.032/1.142 | 4928592/5848592 |
| Condition 1000 | .28315/.28055 | 53164/54692 | .987/.985 | 454592/574592 |
| Condition 4000 | 1.11420/1.16123 | 187444/209716 | .940/1.043 | 1816592/2296592 |
| Switch 1000 (B) | .26753 | 47776 | absolute | 514592 |
| Switch 4000 (B) | 1.12148 | 190832 | absolute | 2056592 |

Common A/A ranges are .25221-.27359 and 1.03969-1.07199 seconds.
Branch 4000 A/A spans 1.72007-1.92672. The large paired branch outlier remains
in the JSON. Common compiler changes do not establish a repeatable speed gain.
Largest observed affected RSS growth is 12.84% (branch 1000); generated guards
and value slots also affect the geometric IR pool capacities.

| Runtime loop | Median A/B seconds | Paired B/A | Native text A/B |
| --- | ---: | --- | ---: |
| Common | .31674/.30826 | .975/.951 | 323/323 |
| Branch | .49198/.58595 | 1.206/1.195 | 1920/2152 |
| Condition | .19798/.27154 | 1.376/1.305 | 1144/1264 |
| Switch (B) | .24943 | absolute | 1200 |

Runtime peak RSS is 256 KiB. Common bytes are identical; apparent timing
improvement is not an implementation speed claim. Branch and condition regressions
are disclosed, not classified as optimization benefits. The condition regression
prompted a follow-up removing a redundant guard for an already scope-owned object.

## Work and growth budgets

Classification caches use at most three bytes per source node and compute each
(node, omitted-result) cleanup fact and each unwind fact once. Emitted segments
track expression/control edges and successful temporary activation. Each guarded
scalar call retains at most one value slot. Shared cleanup suffix lookup keeps
expected constant work per newly needed prefix/terminal pair. No optimization
pass, fixed-point scan or unbounded cloning is introduced. Existing local-array
unrolling remains capped at eight total elements.

At 1000/4000 namespaces, branch classifier visits are 110006/440006, regions
7000/28000 and instructions 136004/544004. Condition visits are 52006/208006
and switch visits 46006/184006; both emit 2000/8000 regions. These counters
establish linear scaling; they do not establish runtime profit. Required region
and value-preservation shapes are PA12 representation costs, with native guard
implementation owned by the supplied backend until PA24. Stage-scoped acceptance
preserves correctness, coverage and bounded work while requiring avoidable costs
to be removed; it does not impose a new positive-runtime gate on required shapes.
