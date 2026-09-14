# PA15 pack and literal performance evidence

Baseline `19225b3d`; final implementation `992b1170`. Frozen compilers use
host g++ C++11, `-Wall -O3`, and the course runner. Measured flags are
`--emit-lowir -O0`; executable validation uses the supplied native backend at
`-O0`. This is compiler implementation evidence, not student native-backend work.

[First campaign](/home/vishvananda/work/private/v4codex/artifacts/pa15-packs/performance.json) and [final campaign](/home/vishvananda/work/private/v4codex/artifacts/pa15-packs/performance-final.json) retain all **392 invocations**, including 56 warmups and 336 observations. [Harness](pack_benchmark.py) pins CPU 0, freezes binaries, sources and flags,
checks exact LowIR/native parity before timing, then runs four A/A observations
and two wall-time ABBA blocks for each common input. Newly supported inputs
have six absolute final-compiler observations after a warmup; the baseline
rejects them, so it supplies no performance comparison. No build or test suite
ran during either campaign. Hashes and work counters are checked by
[the verifier](verify_packs.py) and recorded in [the ledger](pack-handoff.json).

## Compiler costs

Final campaign medians and maximum peak RSS. Paired ratios report both ABBA
blocks; A/A is the raw observed range, not a confidence interval.

| Common correct input | A ms | B ms | A/B peak RSS KiB | Paired B/A | A/A ms |
|---|---:|---:|---:|---|---|
| types-1000 | 74.871 | 74.981 | 17656/17488 | 0.9813, 1.0031 | 72.1–76.5 |
| defaults-1000 | 55.981 | 56.784 | 12280/12564 | 1.0244, 1.0129 | 54.9–56.1 |
| types-4000 | 289.665 | 289.841 | 53824/53368 | 1.0014, 0.9012 | 285.1–295.1 |
| defaults-4000 | 211.366 | 215.874 | 33920/33972 | 1.0322, 1.0165 | 209.4–213.5 |
| constants | 225.946 | 226.295 | 34852/35116 | 1.0016, 0.9989 | 224.1–227.4 |

The first campaign exposed avoidable scalar parameter-list work: 4,000 scalar
function instances took 287.961/320.887 ms and default instances
211.166/226.332 ms. Ordinary parameter lists had allocated replacement occurrence
edges; every instantiated AST view also queried expansion indexes. The final
compiler retains unchanged source edges and skips empty expansion indexes.
The repeat removes the scalar-function median regression and most of the
default-heavy overhead. One 4,000-type ABBA block contains a slow baseline
sample, so its 0.9012 ratio is not evidence of a compiler speedup. No observations
were dropped. Remaining default-heavy medians are about 2% higher, with paired
ratios 1.0165–1.0322: pack-kind/default checks add required semantic work,
not an optional transformation. No further larger regression is established.

Compiler `.text` grows **1,447,110 → 1,494,470 bytes**, **+47,360 (3.27%)**,
for the pack, list, query, literal and typed-output machinery. The preliminary
binary had 1,493,702 bytes. Small runtime-input compilations take about 6 ms;
all latency/RSS observations are preserved, but they support no latency claim.

| Newly correct input | B ms | B peak RSS KiB |
|---|---:|---:|
| pack-signatures-1000 | 111.664 | 24000 |
| pack-signatures-4000 | 460.403 | 79336 |
| nested-packs-1000 | 282.982 | 45704 |
| nested-packs-4000 | 1220.860 | 166820 |

Fourfold source growth gives **4.12×** signature and **4.31×** nested-pack
compiler latency. Source discovery is cached once per source/typed pattern;
substitution follows actual arguments and produced lanes. Exact telemetry:

| Family with N unique source tags | Source regions | Discovery work | Expansion lanes | Frames | Body transitions |
|---|---:|---:|---:|---:|---:|
| Explicit pack prefixes + deduction | 3 | 1 | 3N | 9N | N |
| Nested independent expansions | 9 | 18 | 7N | 13N+11 | 3N |

These counts hold at N=1,000 and 4,000. Identity includes each parameter's
pack boundary and each expansion's parent, parameter bindings and lane.
The separately executed partition-growth control checks 64 different partitions
of the same flattened sequence, each reused twice. No partition count cap is
imposed. These are evidence for the exercised owners and produced work, not
certification of the unfinished matching/execution/body-obligation groups.

## Generated executables

Runtime loops read volatile bounds and check call, memory and floating results.
Common inputs have identical LowIR and native bytes; their code growth is zero.
Native peak RSS is 256 KiB for every workload. The supplied backend emits
sectionless executables: text here is the payload after ELF entry, and these
inputs contain no static data.

| Runtime workload | A ms | B ms | Native text A/B bytes | Paired B/A | A/A ms |
|---|---:|---:|---:|---|---|
| runtime-calls | 300.308 | 301.053 | 206/206 | 0.9958, 1.0046 | 299.9–302.0 |
| runtime-memory | 176.652 | 176.931 | 434/434 | 0.9989, 1.0029 | 175.6–176.9 |
| runtime-floating | 208.560 | 208.413 | 230/230 | 1.0009, 0.9996 | 208.4–210.6 |
| runtime-pack-call | unsupported | 313.149 | 217 | absolute only | — |

The pack-forwarding executable adds an ordinary forwarding function at O0;
its cost is reported absolutely. No runtime optimization benefit is claimed.
The first campaign's floating runtime spread is retained despite identical code.

## Stage-scoped acceptance

PA15/O0 requires correctness, canonical complete identities, TU lifetimes and
linear/near-linear consumed/produced work. It specifies **no numerical
latency/RSS/text ceiling** and this increment adds **no optional optimization**.
The avoidable scalar-list cost was removed; new semantic work and compiler
size are disclosed above. Earlier self-selected numerical targets are diagnostic,
not additional exit gates under spec §9; their measurements remain preserved in
[scalar evidence](performance.md) and [selection evidence](specialization-performance.md).
No mandated limit, correctness rule or coverage was weakened. Native optimization,
register allocation and self-hosting retain their later-stage requirements.
Remaining implementation and independent review are recorded separately in
[the plan](../../pa15/plan.md).
