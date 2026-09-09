# PA12 parameter and return boundary evidence

Class arguments and results now have independent cached conventions. A class
with a usable trivial copy or move and trivial destruction can use an object
parameter. The result convention also applies the existing sixteen-byte direct
result limit. A selected trivial move at a value boundary uses one `copyobj`;
the existence of an unrelated user copy constructor does not change that move.
Direct class returns share one destination per function.

These changes implement the checked-in PA12 boundary contract, including
`200-trivial-move-does-not-fall-through-to-copy`,
`300-trivial-copy-value-transfer-storage-copy` and the braced-default return
case. Declared constructor calls retain their existing emission policy. No
reference, fixture, comparison rule or coverage changed.

## Frozen experiment

[boundary_benchmark.py](../student.tests/pa12/boundary_benchmark.py) takes A, B,
scratch directory and JSON output. [All observations](../student.tests/pa12/boundary-performance.json)
include hashes, flags, CPU/platform, telemetry, warmups, four A/A measurements
and two ABBA blocks. CPU 0 is pinned; compilation and execution are separate.
Compilation uses `--emit-lowir -O0`. LowIR validation/stats and supplied native
backend construction (`-O0`) run outside compiler timing. Both implementations
produce checked exit 0 on every workload; common LowIR/native bytes match.

- A: `64ecedf7`, `/tmp/pa12-lists-final-cppgm`, SHA-256
  `4284d1f589997a52039b834d25642b0d2139f41343df3146da4ef450c5692eb0`.
- B: `/tmp/pa12-boundary-final-cppgm`, SHA-256
  `0d67bd74b258cff8df51d4e369c96946c937c9058f569b616bb60c636ffdd3b9`.
- Scratch: `/tmp/pa12-boundary-evidence`. Compiler text is 943238/943750 bytes:
  growth of 512 bytes, .054%.

## Compiler and native size

| Workload | A / B median seconds | A / B peak RSS KiB | Paired B/A | Native text A / B |
| --- | ---: | ---: | --- | ---: |
| Common 1000 | .25214 / .25965 | 49484 / 49488 | 1.064 / 1.016 | 168056 / 168056 |
| Common 4000 | 1.01544 / 1.01479 | 184858 / 184792 | .942 / 1.001 | 672056 / 672056 |
| Move 1000 | .31657 / .31653 | 59394 / 59680 | 1.032 / .989 | 178068 / 182068 |
| Move 4000 | 1.31394 / 1.31321 | 219794 / 221110 | 1.012 / .956 | 712068 / 728068 |
| Large 1000 | .28941 / .29403 | 59324 / 59316 | 1.009 / 1.037 | 244068 / 332068 |
| Large 4000 | 1.29046 / 1.20070 | 223890 / 223886 | .929 / .923 | 976068 / 1328068 |

Common A/A ranges are .24728–.24890 and 1.00882–1.05183 seconds. All timing
outliers remain, including .27470 in common 1000 and 1.13546 in common 4000.
The small case's median rises 2.98%; the large case is unchanged. The mixed
paired ratios do not justify a compiler speed claim. Largest affected RSS
growth is .60%. Both cached conventions use the existing per-class special
member analysis; no new traversal over uses or function bodies is added.

## Executable observations and stage acceptance

The runtime loops execute twelve million iterations with volatile bounds and
checked nonconstant results. The move loop also verifies that no copy
constructor runs. The large loop checks both its result and unchanged source.
The namespace programs' roughly 3 ms runtimes are startup dominated.

| Runtime loop | A / B median seconds | Paired B/A | Peak RSS KiB A / B | Text bytes A / B |
| --- | ---: | --- | ---: | ---: |
| Common | .31075 / .31356 | 1.000 / 1.025 | 256 / 256 | 323 / 323 |
| Trivial move | .14253 / .12551 | .878 / .884 | 256 / 256 | 348 / 352 |
| Large object | .40953 / .59173 | 1.446 / 1.434 | 256 / 256 | 411 / 499 |

The move form has a repeatable approximately 12% runtime benefit, at four
additional native bytes. Its work budget is constant per use: consult the
already-selected trivial transfer and emit one bulk transfer, without a new
analysis pass. The shared return slot uses at most one slot per direct class
result function. Cached ABI facts remain bounded per class, and all generated
parameter copies are bounded per actual parameter. Local array unrolling
retains its mandated eight-element total budget.

The large-object boundary is **44.5% slower** and grows runtime text by 88
bytes (21.4%). The function's direct `obj<24x8>` parameter must be materialized
in its local slot, whereas A used a `ptr [pass=by_address]` parameter. The
checked-in PA12 contract requires that direct-object shape; removing its local
payload copy does not satisfy the contract. This is an explicit correctness/
representation cost, not an optimization benefit. The supplied PA8 backend
implements the extra copies; a later native backend or optimizer may improve
them. A positive-runtime gate for this required PA12 representation would be
an unsupported self-imposed gate under the stage-scoped acceptance rules.
All measurements, checks, coverage and mandated limits are preserved.
