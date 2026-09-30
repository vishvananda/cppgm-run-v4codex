# PA25 implementation139 performance evidence

Code commit: `7dfdc6fc`. O0 adds required class runtime and static-initialization
behavior. There is no optional optimization pass and no speedup claim. The
inherited 15% latency/RSS and zero optional text-growth targets remain diagnostic
under [spec section 9](../../spec.md). Required coverage, correctness, compiler
complexity and PA24 native bounds are unchanged.

## Frozen evidence protocol

[performance.py](performance.py) freezes binary/input hashes and `-O0`. Each
workload and mode has four A/A samples followed by six ABBA blocks. Compilation
and execution are measured separately, with `/usr/bin/time` peak RSS. Every
generated program is checked before timing. Ratios divide within-block means;
all observations and paired spread are retained, including outliers. No builds
or correctness test runners ran during measurement.

Entry A: `8ec48c55121ff284ee82238203e17a9e5b482c96dc87cdd3d346ba7d981fff3f`

Exact final B: `82485193297359ed27750a9bcc1daad92208fc0e351c4978512c9833209c9152`

## Equivalent existing workloads

Templates demand 4,800 specializations in nine TUs in one invocation. Memory and
floating compile batches use 64 invocations; those include startup and are
diagnostic for compiler latency. Runtime batches contain three executions with
argc-dependent bounds and checked results; memory/floating execute three million
steps per execution. All A/B executable pairs are byte-identical.

| Workload | B compile batch s | Peak KiB A / B | Compile B/A median [range] | Compile A/A s |
|---|---:|---:|---|---|
| templates | 0.25293 | 15572 / 15692 | 0.998 [0.963–1.016] | 0.250–0.261 |
| memory | 0.35760 | 6756 / 6888 | 0.972 [0.946–0.986] | 0.365–0.722 |
| floating | 0.37139 | 6876 / 7016 | 0.967 [0.936–1.024] | 0.378–0.390 |

| Workload | B runtime batch s | Runtime B/A median [range] | Runtime A/A s | Text bytes A / B |
|---|---:|---|---|---:|
| templates | 0.11811 | 1.004 [0.994–1.013] | 0.117–0.118 | 384567 / 384567 |
| memory | 0.14921 | 0.999 [0.980–1.006] | 0.149–0.150 | 521 / 521 |
| floating | 0.13939 | 1.002 [0.996–1.029] | 0.138–0.139 | 340 / 340 |

The template compiler median is 0.253 s, with a B/A ratio of 0.998. Its peak
RSS rises from 15,572 to 15,692 KiB. Memory compilation has a retained A/A
outlier of 0.722 s; favorable paired ratios do not establish a compiler speedup.
Byte-identical executable output rules out a generated-code regression on these
fixed workloads. Runtime peak RSS is 256 KiB.

## New behavior absolute baselines

The entry compiler cannot link these programs, so comparing its failed work with
a correct compiler would be invalid. Both labels below use the exact final binary.
These ratios are noise calibrations, not optimization evidence.

- `classes`: 400 demanded template-derived classes with constant vptrs; each
  execution performs eight million checked member calls. Compile batches have
  two invocations (median 0.0883 s per invocation).
- `casts`: 400,000 iterations, two dynamic casts each, checking downcast and
  sibling adjustments and summing runtime values. Compile batches have 64 invocations.
- `allocations`: 10,000 allocations/deletions through a secondary base, checking
  values and destructor count. Compile batches have 64 invocations.

| Workload | B compile batch s | Compiler peak KiB | B runtime batch s | Text bytes |
|---|---:|---:|---:|---:|
| classes | 0.17664 | 19560 | 0.50624 | 68368 |
| casts | 0.45809 | 7116 | 0.11864 | 1682 |
| allocations | 0.42549 | 6872 | 0.34795 | 2700 |

| Workload | Compile B/A [range] | Compile A/A s | Runtime B/A [range] | Runtime A/A s |
|---|---|---|---|---|
| classes | 0.981 [0.955–1.121] | 0.172–0.200 | 0.993 [0.987–1.021] | 0.509–0.516 |
| casts | 0.987 [0.865–1.118] | 0.467–0.503 | 0.993 [0.724–1.027] | 0.117–0.152 |
| allocations | 1.021 [1.003–1.047] | 0.405–0.431 | 0.974 [0.919–1.019] | 0.342–0.354 |

Runtime peak RSS is 256 KiB. Each runtime sample batches three executions.
The new runtime traverses inheritance paths, including repeated virtual paths;
it coalesces result identities by address and keeps O(depth) traversal stack.
It does not claim a graph-linear runtime bound. Allocation uses one mapping per
object and releases it on deletion: measured absolute cost is about 11.6 us per
allocation/deletion pair here. These are correct O0 baselines for later measured
runtime improvements, not optional transformations requiring speculative profit.

## Work, ownership and stage acceptance

- Retained definition/fixup demand remains O(symbols + relocations), with a
  deduplicated worklist. Runtime requests are collected on live edges, without
  rescanning or retrying the dependency graph. Discarded weak bodies do not
  demand their allocator. Typed telemetry records runtime functions/instructions/text.
- Support construction is bounded by the finite source ABI role set. Cast support
  adds two fixed functions (209 MIR instructions, 930 native bytes in the trace);
  process primitives are fixed leaf bodies. There is no input-dependent inlining
  or pass iteration, no optimization code-growth budget to spend, and no cold
  support body retained per translation unit.
- A declaration owns its static construction fact. Vptr/field ordering takes
  O(fields log fields), followed by linear data/relocation emission. Mutable
  globals retain runtime reads; named constant destinations establish self relocations.
- Runtime-only LowIR dies after object generation. Function selection temporaries
  die per function; only native bytes and typed fixups enter the linker. Heap
  mapping extents belong to the 16-byte allocation prefix, released by munmap.
- The exact source-to-ELF trace has one template-class completion, three RTTI
  records with eight cache hits, one static-initialization fact, five constant
  fields, and no dynamic initializer. Its separate/direct ELF images match.
- Prior wide/statement benchmarks and historical evidence remain intact.
  Self-hosting is owned by PA34; O1–O3 policies belong to later optimization stages.

No avoidable equivalent-workload regression or unprofitable optional transform
was observed. Necessary class/runtime work is explicitly bounded and measured.
All 4,152 prior checks and PA24 native envelopes pass. The 15 remaining PA25
failures are unfinished implementation, not performance exceptions or waived gates.

## Preserved records

All paths below are relative to `/home/vishvananda/work/private/v4codex/artifacts/pa25-139/`.
The intermediate run predates only the added native parameter-view fact; it is
preserved separately and is not labelled as the final compiler. There are **504**
new observations, plus all historical records linked by [performance138](performance138.md).

| Record | Observations | SHA256 |
|---|---:|---|
| `performance/performance.json` | 168 | `b124994f0bc10b512a7ef0944d0e07cf2f8e579c4610559990eaf40096a1152d` |
| `performance-final/performance.json` | 168 | `6c9417cb8e360f6c10590fe6561007670160b221f86e14acaba904319b5287e1` |
| `performance-classes/performance.json` | 168 | `7e4370837837d0088fb3b6fbbf2052a59ee2061069b6d41c82f0584c7a701510` |

[validation139](validation139.json) pins binaries, checks, exact failures,
source inventory and trace hashes. Every final measurement manifest is rehashed
at handoff. Raw samples are not filtered.
