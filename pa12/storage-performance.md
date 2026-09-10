# Storage-unit and explicit-conversion evidence

PA12 `--emit-lowir -O0`: completed constructor unit ordering and explicit
conversion-result boundaries. Final implementation is `2d693f2b`; the course
suite advances from **253/257 to 255/257**, all 13 controls pass, and earlier
assignments remain **1327/1327**. No reference or comparison rule changed.

## Frozen inputs and protocol

[Harness](../student.tests/pa12/storage_benchmark.py),
[final observations](../student.tests/pa12/storage-performance.json) and
[paired elision comparison](../student.tests/pa12/storage-elision-performance.json)
record hashes, flags, CPU affinity, platform, telemetry, every warmup/observation,
compiler latency/peak RSS, native runtime and code size. Runtime loops run
12 million iterations with volatile bounds, varying inputs and checked results.
Compiler timing excludes telemetry/validation; both are checked separately.
The supplied PA8 native backend is used at `-O0`, as required by PA12.

Every common/unit/trivial-explicit workload has warmups, four A/A observations
and two ABBA blocks. Tables use medians from the paired blocks; JSON retains
all samples and paired ratios. Namespace counts 100/400 (common 1000/4000)
measure scaling. Short single-call executable runs check outcomes and size;
they are startup-dominated and do not establish runtime benefits.

Compiler size is ELF `.text`. The supplied backend writes sectionless ELF;
its reported size is the executable payload after entry, not a `.text` section.
RSS in tables is independently measured by `/usr/bin/time`, in KiB.

| Binary | Commit | SHA-256 | Compiler text bytes |
| --- | --- | --- | ---: |
| A | `9e442405` | `b3946580a85465de1e7ba4e012f6fff369a988c468f924c64c7c6323d5f373d3` | 967622 |
| B | `2d693f2b` | `0fba5a741627fc8d1978a5991e56d3d38c95fafdc76bec24819f0f6beaecbd2f` | 969542 |

## Final four-dimensional results

| Compile corpus | Seconds A / B | Peak RSS A / B, KiB | Compiler B/A pairs | Native payload A / B, bytes |
| --- | ---: | ---: | --- | ---: |
| common-1000 | 0.25277 / 0.25564 | 49696 / 49704 | 1.032, 1.008 | 168056 / 168056 |
| common-4000 | 1.02727 / 1.02851 | 184854 / 184880 | 0.942, 1.004 | 672056 / 672056 |
| unit-100 | 0.04436 / 0.04489 | 11832 / 12146 | 1.007, 1.013 | 46668 / 46468 |
| unit-400 | 0.16179 / 0.16257 | 32714 / 33108 | 1.004, 1.003 | 186468 / 185668 |
| explicit-100 | 0.02751 / 0.02772 | 8818 / 8810 | 1.005, 1.010 | 12256 / 12256 |
| explicit-400 | 0.09468 / 0.09504 | 20938 / 20468 | 1.008, 1.001 | 48856 / 48856 |

| Runtime corpus | Compiler seconds A / B | Compiler RSS A / B, KiB | Runtime seconds A / B | Runtime B/A pairs | Native payload A / B, bytes |
| --- | ---: | ---: | ---: | --- | ---: |
| common-runtime | 0.00574 / 0.00575 | 4970 / 4948 | 0.30749 / 0.30847 | 1.001, 0.998 | 323 / 323 |
| unit-runtime | 0.00626 / 0.00630 | 5030 / 5002 | 0.27644 / 0.27803 | 1.005, 1.006 | 633 / 631 |
| explicit-runtime | 0.00595 / 0.00614 | 4946 / 4964 | 0.10513 / 0.10521 | 1.001, 1.000 | 277 / 277 |

Common and trivial-explicit LowIR/native bytes are identical between A and B.
The large common compiler median increases **0.12%**; the first pair contains
an A outlier and is not evidence of a compiler speedup. Its A/A range is
1.02274–1.03768 seconds. The final unit runtime is about **0.57% slower**, with
paired ratios 1.005/1.006; its A/A range is .27495–.27669 seconds. Two fewer
native bytes do not establish a runtime benefit. This small disclosed cost
accompanies required constructor ordering; no unit speedup is claimed.
The explicit-100 startup A/A sample of .02373 seconds remains in the JSON.

## Removed intermediate costs

- [Initial calibration](../student.tests/pa12/storage-performance-initial.json)
  preserves common observations before a benchmark-only unsupported
  `unsigned(n)` spelling stopped generation. The corrected generator uses
  `static_cast<unsigned>(n)`; no fixture changed.
- [Retained helper policy](../student.tests/pa12/storage-helper-performance.json)
  (`8a5a370d`) forced nonempty trivial transfers through a helper: runtime
  .10566 -> .26308 seconds, paired ratios 2.542/2.433; payload 277 -> 311 bytes.
- [Direct storage policy](../student.tests/pa12/storage-direct-performance.json)
  (`b699f183`) reduced that payload to 287 bytes but still measured
  .10524 -> .26235 seconds, ratios 2.497/2.553. Code size alone was insufficient.
- [Selected elision](../student.tests/pa12/storage-elided-performance.json)
  (`1eae0974`) restored byte-identical trivial-explicit output. Its large common
  compiler median was 3.87% slower, with ratios 1.048/1.040; this observation
  remains. Final review also removed a duplicated field-fact query. All twelve
  final benchmark LowIR/native hashes match this predecessor; no causal timing
  claim is made for that small query change.

[A separate paired campaign](../student.tests/pa12/storage-elision-performance.json)
compares frozen `8a5a370d` helper lowering directly with frozen `1eae0974`
elision, using the same correct, dynamically checked trivial-value sources.
Runtime falls **.25780 -> .10575 seconds**, paired ratios **.412/.400**.
Native payload falls **311 -> 277 bytes**. The 400-class compile median falls
**.10087 -> .09491 seconds**, paired ratios **.940/.941**, and compiler peak
RSS falls **22534 -> 20422 KiB**, while the compiler adds only 64 text bytes.
Full observations are retained in that JSON.
The final executable bytes for this workload remain identical.

## Observable explicit transfers and bounded work

`effects-*` uses a move constructor that increments a caller-owned counter.
The reducer requires the retained O0 transfer exactly once; A elides it and
exits 134 on that expectation. This is a difference between permitted elision
policies, not proof that A violates C++11. Accordingly this corpus reports B-only
absolute measurements (warmup and six samples), without an A/B speed claim.

| B-only corpus | Compiler seconds | Peak RSS, KiB | Runtime seconds | Native payload bytes |
| --- | ---: | ---: | ---: | ---: |
| effects-100 | 0.04005 | 11010 | 0.00312 | 22068 |
| effects-400 | 0.14616 | 30246 | 0.00308 | 88068 |
| effects-runtime | 0.00602 | 4974 | 0.15824 | 387 |

Allocation-unit metadata is prepared once per eligible field while preparing
existing transfer actions. Constructor lowering consumes that fact and reuses
one descriptor lookup for dispatch. The unit corpus scales from 400 to 1600
prepared fields and 800 to 3200 transfer actions. There is no extra layout scan.
Volatile fields retain their existing field-wise path; initializer effects
complete before retained bits are read.

Explicit-result policy queries cached empty-class and selected-triviality facts.
Nonempty trivial results reuse the final destination. Empty and nontrivial
explicit transfers retain source/destination identities and normal cleanup.
There is at most one extra conversion-object record per retained use, two
sparse flags overall, and constant extra work per use/field. No source replay,
global retry or call-graph analysis is introduced. Existing array expansion
remains capped at eight elements.

Final compiler text growth is **1920 bytes (.20%)**, within the 4 KiB diagnostic
budget; common compiler medians remain within the 5% diagnostic review budget.
These are review budgets, not additional course exit gates. Required retained
object/transfer costs do not require a positive runtime gain. The avoidable
nonempty trivial materialization cost was removed with paired evidence.

## Reproduction

```sh
python3 student.tests/pa12/storage_benchmark.py /tmp/pa12-storage-base-cppgm /tmp/pa12-storage-facts-cppgm /tmp/pa12-storage-benchmark-facts /tmp/storage-repeat.json
python3 student.tests/pa12/storage_elision_compare.py /tmp/pa12-storage-final-cppgm /tmp/pa12-storage-elided-cppgm /tmp/pa12-storage-benchmark-elided /tmp/pa12-storage-elision-compare /tmp/storage-elision-repeat.json
```

All campaign processes completed. The generated workload sources and native
images are scratch artifacts; the harnesses, hashes and every measurement are
preserved here. Frozen binary paths above identify the exact measured revisions.
