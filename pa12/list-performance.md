# PA12 target-typed list evidence

This is required semantic work, with no runtime optimization claim. Selection
retains an untyped source list and caches a target-keyed candidate plan. Each
selected use owns its conversions and temporary. Aggregate helpers consume the
selected field transfers; lowering does not repeat overload resolution.

Run [list_benchmark.py](../student.tests/pa12/list_benchmark.py) with frozen A,
B, scratch directory and JSON destination. The [complete observations](../student.tests/pa12/list-performance.json)
retain flags, platform, CPU affinity, hashes, warmups, outliers and telemetry.
Compilation is `--emit-lowir -O0`; validation/stats and the supplied native
backend's `-O0` construction are outside timed compilation. Every executable
returns the checked result. Common LowIR and native bytes are identical.

- A: `aee24d98`, `/tmp/pa12-lists-base-cppgm`, SHA-256
  `8912589fee00ca2192e1df62ed6e5e944abc8f24270e60ac4e6ed27252997784`.
- B: list implementation, `/tmp/pa12-lists-final-cppgm`, SHA-256
  `4284d1f589997a52039b834d25642b0d2139f41343df3146da4ef450c5692eb0`.
- Scratch: `/tmp/pa12-list-evidence`. Compiler text grows from 917958 to
  943238 bytes: 25280 bytes, 2.75%.

Common cases have warmups, four A/A observations and two ABBA blocks. New cases
have a warmup and six absolute observations; A cannot correctly compile them.

| Workload | A / B compiler median seconds | A / B peak RSS KiB | Paired B/A blocks | Native text bytes |
| --- | ---: | ---: | --- | ---: |
| 1000 common namespaces | .25108 / .25181 | 49266 / 49906 | 1.003 / 1.002 | 168056 |
| 4000 common namespaces | 1.02431 / 1.02891 | 184448 / 184834 | .949 / 1.013 | 672056 |
| 1000 list namespaces | unsupported / .54968 | — / 101610 | absolute only | 610592 |
| 4000 list namespaces | unsupported / 2.30313 | — / 417782 | absolute only | 2440592 |

Common A/A ranges are .25103–.25229 and 1.01806–1.02497 seconds. The 4000-case
A outlier at 1.13719 seconds remains in the observations and explains the first
paired ratio. Common median latency grows .29%/.45%; largest common RSS grows
.21%. These observations do not establish a runtime speed change.

The common twelve-million-iteration loop runs in .30882/.30953 seconds
(paired 1.004/1.002), with identical 323-byte native text. The new loop checks
list overloads, reference ranking, braced defaults and destruction on each
iteration: .38940 seconds, 256 KiB peak RSS, 1304-byte text. Its compiler takes
.00626 seconds / 4988 KiB. The namespace executables' approximately 3 ms
runtimes are startup dominated and are not speed evidence.

At 1000/4000 namespaces, plans are 9000/36000, selected list objects 6000/24000,
candidate visits 16001/64001, conversions 39004/156004 and instructions
105004/420004. Compile latency scales 4.19x, RSS 4.11x and native text 4.00x.
Work follows actual list elements and required candidates. Omitted array tails
use one counted action; the inherited local expansion budget remains eight
total elements. There is no speculative optimizer, source replay, whole-program
retry or optional transform to justify with a speed gain. These semantic costs
meet the stage-scoped evidence protocol; later backend profit gates and
unsupported positive-runtime gates do not apply.
