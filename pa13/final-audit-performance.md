# PA13 final performance review

The current implementation is `6c85368f`, including dispatch fixes `3d39b1cd`.
The audit compares frozen stage-completion A (`cf1b9621`) with corrected B.
The original PA12-to-PA13 [stage campaign](../student.tests/pa13/performance.md)
and all 352 historical observations remain intact.

## Frozen experiment and acceptance

- A: SHA-256 `35547adc8e82fb94e30a7d687f6bb94e0d9ca4722639dc41423ee696e2513243`, compiler `.text` 1018502 bytes.
- B: SHA-256 `96225468c9e0a6e3a8e7b72bf52a5c6f7a97c5217e7e5d349343ef8deda01bbf`, compiler `.text` 1023942 bytes.

The [final campaign](../student.tests/pa13/final-audit-performance.json),
[noise repeat](../student.tests/pa13/final-noise-performance.json), and
[scaling follow-up](../student.tests/pa13/final-scaling-performance.json)
retain **550** observations, including warmups, over **33** fixed workloads.
Together with the historical campaign, **902** observations are retained.
JSON records frozen source/output/executable hashes, binary paths, flags,
backend identity, CPU affinity, phase/work telemetry and all wall/RSS samples.
The harnesses are `audit_benchmark.py`, `audit_noise.py`, `audit_scaling.py` and
`audit_verify.py` under `student.tests/pa13/`.

The host build uses the repository's g++ C++11 O3 flags and course runner.
The source compiler uses `--emit-lowir -O0`; the inherited template corpus uses
its supported `--emit-semantics` view. The supplied backend uses O0. Timed
production compilation excludes `--stats` and `--validate-lowir`; separate
telemetry/validation runs collect work counters without changing timed work.
No compiler build, course test or executable verification ran concurrently
with a timing campaign.

Every common workload has warmups, four A/A calibration observations and two
ABBA blocks. Two noisy compiler rows were repeated with four ABBA blocks;
three corrected scaling pairs were repeated with CPU accounting. All common
LowIR/semantic outputs and executable bytes are identical. The five corrected
behavior families have six B-only observations: the independent reducers prove
A incorrect, so no A/B speedup is claimed there. Runtime bounds are volatile,
loop/call/memory/FP work remains live, and every executable checks its result.
Compilation and execution are measured separately; startup is recorded and
short compiler timings for executable sources are not treated as throughput
measurements.

No optional optimization is introduced. The spec mandates correctness,
self-containment, bounded work/growth and evidence at the available stage;
it supplies no numeric PA13 timing/RSS/text ceiling. Inherited 5% latency,
128 KiB text and scaling targets are diagnostic review signals, not additional
exit gates. Their measurements and historical misses remain in prior reports.
The PA12-to-entry text cost was 37,760 bytes; the audit adds **5,440 bytes
(0.534%)**, making the total PA12-to-current cost **43,200 bytes (4.405%)**.
These implement required dispatch, initialization, signature provenance and
internal linkage. Common generated code does not grow.

## Compiler latency and peak memory

Medians and maximum RSS across production samples follow. Paired B/A ratios
are the means within ABBA blocks, not ratios inferred from unrelated runs.
All smaller-scale rows, startup observations and full spreads remain in JSON.

| Common compiler input | A/B median seconds | A/B peak KiB | Paired B/A |
| --- | ---: | ---: | --- |
| calls-4 | 1.746809 / 1.753359 | 308516 / 308472 | 1.0179, 0.9445 |
| memory-float-4 | 1.442354 / 1.459725 | 251460 / 251496 | 1.0330, 0.9967 |
| references-4 | 0.050758 / 0.051116 | 14376 / 14356 | 1.0046, 1.0195 |
| template-semantics-4 | 0.265727 / 0.264858 | 37184 / 37252 | 1.0028, 0.9910 |
| class-4000 | 0.581910 / 0.599491 | 112912 / 112232 | 0.9846, 1.0595 |
| virtual-4000 | 2.532718 / 2.105294 | 314156 / 317444 | 1.0053, 0.8868 |

The initial large virtual row and several runtime rows experienced substantial
wall-time variation. The apparent large virtual speedup is not accepted as a
benefit. The same frozen inputs and binaries give these noise-repeat results:

| Repeated input | A/B median seconds | A/B peak KiB | Four paired B/A ratios |
| --- | ---: | ---: | --- |
| class-4000 | 0.586760 / 0.585101 | 112916 / 112228 | 1.0768, 0.9992, 1.1573, 0.9809 |
| virtual-4000 | 1.603808 / 1.624898 | 314180 / 317504 | 0.8944, 1.0483, 1.0570, 0.9270 |

Repeated class medians change by -0.28%, with identical 0.57 s median child CPU
time. Repeated virtual medians change by +1.32%, with identical 1.56 s median
child CPU time; the A/A wall range is 1.588–1.857 s. Paired ratios straddle one
in both rows. These do not establish a repeatable compiler speedup or sustained
large regression. The required private signature slices and scope cache have
bounded storage cost: virtual peak RSS grows by 3,324 KiB (1.06%) in the repeat.
Class peak RSS decreases by 688 KiB. Smaller common reference and memory/FP
latency increases remain disclosed above; no exact zero-overhead claim is made.

| Corrected compiler input | B median seconds | B wall range seconds | B peak KiB | Instructions |
| --- | ---: | --- | ---: | ---: |
| array-400 | 0.138883 | 0.137159–0.139519 | 31284 | 51602 |
| array-1600 | 0.734401 | 0.544065–1.016923 | 104804 | 206402 |
| destructor-400 | 0.178551 | 0.124656–0.347518 | 27188 | 43600 |
| destructor-1600 | 1.062253 | 0.897417–1.239410 | 99016 | 174400 |
| global-delete-400 | 0.149148 | 0.147712–0.150894 | 33576 | 50800 |
| global-delete-1600 | 0.593350 | 0.592822–0.595785 | 117340 | 203200 |
| conversion-400 | 0.339594 | 0.241559–0.495673 | 31140 | 26000 |
| conversion-1600 | 0.564176 | 0.559222–0.626438 | 110612 | 104000 |
| operator-400 | 0.134663 | 0.133296–0.136497 | 30052 | 23600 |
| operator-1600 | 0.546200 | 0.535861–0.554676 | 105944 | 94400 |

Fourfold source growth produces exactly fourfold slot work and essentially
fourfold IR (array globals add two constant instructions). The noisy initial
array/destructor medians are retained, not promoted to scaling gates. The
CPU-accounted follow-up warms both scales and alternates them four times:

| Corrected family | 400/1600 median seconds | 400/1600 median child CPU seconds | Wall growth |
| --- | ---: | ---: | ---: |
| array | 0.152827 / 0.574985 | 0.140 / 0.560 | 3.762x |
| destructor | 0.127741 / 0.555578 | 0.115 / 0.495 | 4.349x |
| conversion | 0.147463 / 0.567516 | 0.130 / 0.550 | 3.849x |

The structural bounds reviewed in [the audit](audit.md) explain work in terms
of actual slots, signatures, actions and emitted instructions. Array lowering
also emits the same 106 instructions for runtime extents 19 and 1,000,000.
These observations support bounded compiler work; they are not substitutes
for runtime measurement or evidence of optimization profitability.

## Executable runtime and size

The supplied backend produces sectionless ELF. The reported size is executable
payload after the entry point, including support and data, not an isolated
`.text` section. Common outputs have identical payloads and identical bytes.
Timing differences, including the initial floating-point apparent gain and
memory apparent regression, are observational variation of the same programs.

| Common checked executable | A/B median seconds | A/B wall ranges seconds | Payload bytes (both) |
| --- | ---: | --- | ---: |
| class-runtime | 0.087090 / 0.086792 | 0.086595–0.090479 / 0.086437–0.087626 | 248 |
| virtual-runtime | 0.303607 / 0.302561 | 0.298692–0.352507 / 0.295311–0.357206 | 2360 |
| calls-runtime | 0.502891 / 0.478221 | 0.475533–0.534996 / 0.474533–0.534427 | 206 |
| memory-runtime | 0.279852 / 0.290427 | 0.278817–0.330693 / 0.278763–0.319692 | 434 |
| floating-runtime | 0.405856 / 0.358820 | 0.330805–0.541433 / 0.332509–0.405346 | 230 |

| Corrected checked executable | B median seconds | B wall range seconds | B peak KiB | Payload bytes |
| --- | ---: | --- | ---: | ---: |
| array-runtime | 0.609652 | 0.476653–0.684467 | 320000 | 1800 |
| destructor-runtime | 0.540182 | 0.492484–0.828357 | 320000 | 2072 |
| global-delete-runtime | 0.503411 | 0.474251–0.908944 | 320000 | 2312 |
| conversion-runtime | 0.169078 | 0.168767–0.169799 | 256 | 648 |
| operator-runtime | 0.157315 | 0.157120–0.158623 | 256 | 584 |

The three heap rows use 80,000 checked allocation/deallocation iterations;
conversion and operator rows use 12 million iterations. A separate ten-iteration
scalar allocation reducer emits allocation and deallocation calls in student
LowIR. `strace -e trace=mmap,munmap` of the supplied-backend executable records
ten mappings and no unmapping. This explains why repeated allocation samples
retain roughly one page per iteration (320,000 KiB in the large rows), despite
source-generated calls to the required deallocator. The source, LowIR and raw
trace are preserved as `allocator-probe.*` in the audit artifacts. This is an
observed supplied-backend constraint; no student deallocation behavior is
skipped and no new PA24 gate is imposed on PA13.

The original stage runtime evidence remains useful and was independently
hash-verified. The current campaign adds five corrected executable behavior
families and repeats every fixed stage workload. Templates are measured at the available semantic view;
self-hosting and student native encoding remain unavailable until their owning
stages. No optimization is accepted on fewer IR nodes alone. Required semantic
costs, observed noise, common-code identity and bounded work support PA13's
stage-scoped acceptance without weakening correctness or coverage.

Artifacts and full logs are under `$RALPH_ARTIFACT_DIR/pa13-final-audit/`.
`audit_verify.py` verifies every frozen identity, all 550 final observations,
paired calculations, corpus completeness and executed outcomes. The original
`verify_performance.py` verifies the 352 historical observations separately.
