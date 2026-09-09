# PA12 member-pointer and scalar-width evidence

`ce2d8363` represents nonvirtual member functions as a function/adjustment pair,
data members as offsets, and records application operands and their indirect
signature by canonical type. Ordinary wide scalar assignment retains the typed
conversion; union-member activation keeps the initializer form. These are
required representation changes, with no optional optimization or speed claim.

[Harness](../student.tests/pa12/member_pointer_benchmark.py) and
[observations](../student.tests/pa12/member-pointer-performance.json) preserve
all hashes, generated inputs, flags, work counts, warmups, four A/A observations
and two ABBA blocks for equivalent correct common/assignment programs. New
member-pointer paths are measured absolutely with a warmup and six observations;
the old binary cannot execute those paths correctly. CPU 0 is pinned.
Compiler latency and peak RSS are separate from execution through the supplied
PA8 `-O0` native backend. Validation and telemetry are outside compiler timing.
Every measured executable returns its checked zero result. Runtime inputs use
volatile bounds, twelve million iterations and checked dynamic values; the
assignment workload also performs observable volatile stores.

A is `4cefbabe`, `/tmp/pa12-member-pointer-base-cppgm`, SHA-256
`dda7927f664ea60214320d6a2135d1edbab5c9000f4403ef811aa9bdd3b35f4e`.
B is `ce2d8363`, `/tmp/pa12-member-pointer-final-cppgm`, SHA-256
`4717657d78309ec69aa89413c513d07d462d5f53eb6bee81273f3d0648e5af41`.
Compiler .text is 950470/959430 bytes: growth 8960 bytes (.94%). Native text uses
the sectionless ELF payload metric retained by previous campaigns.

| Workload / namespaces | Compiler median A/B s | Peak RSS A/B KiB | Paired B/A | Native text A/B |
| --- | ---: | ---: | --- | ---: |
| Common / 1000 | .25079/.24929 | 50164/49748 | .994/.986 | 168056/168056 |
| Common / 4000 | 1.02697/1.04451 | 186216/184960 | .965/1.015 | 672056/672056 |
| Assignment / 1000 | .11602/.11845 | 23896/24924 | 1.021/1.017 | 136072/126072 |
| Assignment / 4000 | .45487/.46112 | 81288/86080 | 1.135/1.018 | 544072/504072 |

Common LowIR and native bytes are identical. Its large compiler median rises
1.71%; assignment compiler medians rise 2.10%/1.37%, with large peak RSS up 5.9%.
All noise and outliers remain, including the 1.135 assignment pair. The common
runtime medians are .30759/.30598 s, paired .905/1.003, with 323 identical native
bytes. Assignment runtime is .07374/.07311 s, paired .995/.986, with 304/296 bytes.
Runtime RSS is 256 KiB; no speed benefit is inferred from these small differences.

| New workload / namespaces | Compiler median s | Peak RSS KiB | Native text |
| --- | ---: | ---: | ---: |
| Function member / 1000 | .47770 | 87476 | 518068 |
| Function member / 4000 | 1.94147 | 335420 | 2072068 |
| Data member / 1000 | .32423 | 58732 | 286068 |
| Data member / 4000 | 1.31149 | 221152 | 1144068 |

Function-member runtime is .69625 s / 685 bytes, data-member runtime .14387 s /
453 bytes; runtime RSS is 256 KiB. These establish absolute costs, not an A/B
benefit. Function-member instructions scale 108004 to 432004 and data-member
instructions 69004 to 276004 for fourfold namespaces. Common source nodes scale
172021 to 688021; assignment instructions scale 23004 to 92004.

Budgets: constant work per formation/application, one 16-byte slot per
materialized function pointer, one bounded 16-byte copy per value boundary,
eight bytes per data pointer, one indirect signature per canonical type, and
one widening operation per required wide assignment. Type and object facts
remain translation-unit-owned, with no body replay or global cache. The local
array expansion limit remains eight total elements. Required semantic costs
are accepted under the stage-scoped spec; they do not impose a positive-runtime
gate. Earlier measurements, coverage and mandated bounds remain intact.
