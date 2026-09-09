# PA12 conversion and reference evidence

This measures the required conversion, builtin/surrogate selection, static
reference and call-boundary implementation. It makes no runtime optimization
claim. PA12 uses its own source-generated LowIR and the supplied PA8 native
backend; a later native-backend performance gate does not apply here.

## Reproduction and frozen inputs

Run [conversion_benchmark.py](../student.tests/pa12/conversion_benchmark.py)
with baseline binary, candidate binary, scratch directory and JSON destination.
It pins CPU 0, records platform/tool/input hashes and exact flags, validates
LowIR, executes every generated program, and checks common LowIR and native
bytes for equality. Compilation uses `--emit-lowir -O0`; validation/stats and
native construction occur outside timed compilation. Native flags are `-O0`.

- A: clean `1a7867fd`, `/tmp/pa12-conversion-base-cppgm`, SHA-256
  `330d646fbccf18e0873f161a21d9bfb003b2dd1ce86f8bbf7eed8d27415b2911`.
- B: `77145afa`, `/tmp/pa12-conversion-evidence/conversion-cppgm`, SHA-256
  `1dc3c8737e6c73c28ac665ff8c39ebb3e7777ee5f4e34993e4d821a07c14b064`.
- Final C: `f9c8e6c3`, `/tmp/pa12-conversion-evidence/final-cppgm`, SHA-256
  `86228da122723158ea55c38c1739e2469164a441fac6412767a0a47b2c544219`.

The preserved campaigns are [initial A/B](../student.tests/pa12/conversion-performance.json),
[repeat A/B](../student.tests/pa12/conversion-repeat-performance.json), and
[final A/C](../student.tests/pa12/conversion-final-performance.json).
JSON labels 0/1 designate baseline/candidate in each campaign. Scratch sources,
LowIR and executables remain under `/tmp/pa12-conversion-{final-,}evidence` and
`/tmp/pa12-conversion-evidence-repeat`. The final source hashes are in its JSON.

Common cases use warmups, four A/A observations and two ABBA blocks. Newly
supported cases have no correct A comparison: they use a warmup and six
absolute observations. The runtime workloads perform twelve million calls or
reference reads, with volatile bounds and checked nonconstant checksums.
The 1000/4000-namespace cases measure compilation/size; their approximately
3 ms process runtimes are startup dominated and do not support speed claims.

## Results

Final common compiler observations (seconds are medians; RSS is peak KiB):

| Workload | A / C seconds | A / C RSS | Paired C/A blocks | Native bytes A = C |
| --- | --- | --- | --- | ---: |
| 1000 value namespaces | 0.24835 / 0.25045 | 49600 / 49380 | 0.952 / 1.011 | 168056 |
| 4000 value namespaces | 1.02355 / 1.03312 | 185100 / 185412 | 1.042 / 1.006 | 672056 |
| Common runtime loop | 0.00590 / 0.00593 | 4916 / 4880 | 1.024 / 0.968 | 323 |

Common loop runtime is approximately 0.307/0.305 seconds, with paired ratios
0.998/0.989. Its bytes are identical. The initial 4000-case compiler block
reported 1.080, prompting a repeat: that repeat reported 0.963/1.005. The final
campaign includes both A and C latency outliers; no observation was removed.
Its A/A ranges are 0.24774–0.25298 and 1.01553–1.03112 seconds. The small common
median costs and inconsistent paired outliers do not establish a repeatable
speed change. Peak memory grows about 0.2% on the largest common case.

New final workloads, measured without an unsupported baseline comparison:

| Workload | Compiler median seconds | Peak RSS KiB | Native text bytes |
| --- | ---: | ---: | ---: |
| 1000 conversion namespaces | 0.35252 | 65076 | 252056 |
| 4000 conversion namespaces | 1.42733 | 243620 | 1008056 |
| 1000 static-reference namespaces | 0.28033 | 56028 | 324104 |
| 4000 static-reference namespaces | 1.14935 | 214664 | 1296104 |

Conversion records are **4000/16000**, candidate visits **10001/40001**, and
entities **25002/100002**. Static-reference candidate visits are **2001/8001**
and entities **19002/76002**. These counters support proportional work; timing
alone is not the complexity proof. The new conversion loop runs a median
0.19554 seconds with 410 native bytes; the static-reference loop takes 0.06282
seconds with 528 bytes. Their compiler medians are 0.00608/0.00600 seconds and
peak RSS 4920/4984 KiB. All individual observations and telemetry are retained.

Compiler `.text` grows **833542 -> 897158 bytes**, **63616 bytes (7.63%)**, for
the full semantic/storage/boundary addition. Native text is the supplied
sectionless ELF's executable payload after entry, as defined by the harness.
These are disclosed correctness costs, not evidence of an optional transform's
profitability or a new positive-runtime acceptance gate.

## Owners, work budgets and validation

Conversion declarations are indexed by class and canonical target TypeId.
Lookup visits only required conversion members and base edges. Candidate
selection compares object and second-standard sequences, retaining the chosen
function, access checks, actual base adjustment, result transfer and storage.
Constructors, operators, condition declarations and surrogate calls consume
these facts; lowering performs no overload search. There are no fake syntax
nodes, body clones, global candidate scans or repeated default-body demands.

Storage work is bounded by selected materializations. Each complete temporary
extended by a namespace reference has one backing object; each nontrivial
conditional alternative has at most one eight-byte shutdown guard. Distinct
complete source types retain distinct identities. Finalization visits these
objects once. Scalar conditional references reuse the already materialized
result slot required by the O0 contract. Existing array expansion remains
capped at eight elements. No speculative optimization or code-growth budget
is introduced; query annotations only preserve semantic boundary facts.

Final validation: PA12 **202/257**, earlier **1327/1327**, 28 personal source
checks, explicit abort termination (134), static-reference execution, the
stable-prefix control and four query rejection controls pass. File audit passes
with header-organization advisories. The survivor runner now reaches the
unsupported member-pointer object-extent control. All remaining fixtures and
comparison rules remain required; no reference output changed in this group.
