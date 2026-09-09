# PA11 access and selection checkpoint

Implementation: 165af40e (access/friends), b74a80fa (operators), c7206513
(return conversions). The continuation started clean at 55a21dac, 173/302.
It ends at **234/302**, fixing **61 existing failures with no regressions**.
All four PA11 controls pass. Earlier PAs pass **1025/1025**; the file audit
passes with the existing Analyzer-header declaration-count advisory. Seven
personal programs validate typed LowIR and execute; three negative checks reject.
The full stage remains incomplete; no assignment or reference was changed.

## Ownership and validation

- Declaration access, inheritance edges and using exposure retain provenance.
  Class/function friendships are indexed by canonical entity. Hidden namespace
  functions participate only through valid lookup/ADL visibility. Access is
  checked after selection, preserving inaccessible overload participation.
- Operators use token-indexed names and typed ABI terminals. Relevant member,
  ordinary and associated candidates produce one selected call record, including
  object adjustment, reference category and argument conversions. Prefix/postfix
  identity follows the declared parameter list; postfix zero is a typed implicit
  call argument, not fabricated syntax.
- Lowering consumes those records for calls, returned references and base
  projections. Logical/comma operators retain call side effects in conditions
  and discarded expressions. Materialized temporary storage owns its address,
  value initialization and existing cleanup activation.
- The extended personal operator test caught the return path bypassing a selected
  derived conversion. All b74a80fa measurements remain preserved; c7206513 is the
  final measured implementation. General converting constructors, class argument
  transfer and complete-class parsing ambiguity remain separate owners described
  in [the plan](plan.md).

## Frozen protocol and complete observations

Use [the protocol](../student.tests/pa11/performance-protocol.md): unchanged flags,
one pinned CPU, AAAA calibration then ABBA/ABBA, external wall/RSS measurements,
and separate telemetry. Compiler and runtime compilation are outside execution
timing. No builds/tests run concurrently with timing.

A: 55a21dac, SHA256 `95254072074ac0be229cc9b8aa3a163646fdc311a1b5df4e105d9280faf15387`.
B: c7206513, SHA256 `14fe0b2503b191016f0c2a3bdbbd70c6d518f86432dbf3c615b5381fe4bacbc1`.
Compiler .text: **645,382 → 672,966 bytes (+27,584; 4.27%)**.

Raw final evidence: [common](../student.tests/pa11/selection-common-performance.json),
[follow-up](../student.tests/pa11/selection-repeat-performance.json),
[new behavior](../student.tests/pa11/selection-behavior-performance.json).
Preserved b74a80fa evidence: [initial common](../student.tests/pa11/selection-initial-common-performance.json),
[initial follow-up](../student.tests/pa11/selection-initial-repeat-performance.json),
[initial behavior](../student.tests/pa11/selection-initial-behavior-performance.json).
Embedded pre-rename filenames in the initial follow-up are historical; their
content hashes identify the retained initial common file. Both frozen binaries,
source generators and scratch output sets remain separate. Final artifact hashes,
observation completeness and checked executable results have been verified.

## Compiler measurements

Milliseconds are medians; RSS columns are peak KiB. Both block ratios and the
entire A/A spread are shown, including spikes. All compared outputs satisfy byte
equality or the unchanged course comparator.

| Input | A ms | B ms | B/A blocks | A/A spread | A/B peak KiB |
| --- | ---: | ---: | --- | ---: | --- |
| calls-1 | 391.818 | 395.390 | 1.0093 / 1.2362 | 2.74% | 73168 / 73368 |
| memory-float-1 | 340.804 | 338.398 | 0.9948 / 0.8434 | 1.10% | 67220 / 63916 |
| references-1 | 15.661 | 15.661 | 1.0045 / 1.0120 | 1.47% | 6544 / 6508 |
| template-semantics-1 | 66.171 | 68.095 | 1.0300 / 1.0329 | 1.56% | 12392 / 12544 |
| calls-4 | 1617.750 | 1636.432 | 1.0608 / 0.9159 | 0.68% | 294868 / 281392 |
| memory-float-4 | 1380.179 | 1387.146 | 1.0000 / 1.0109 | 1.36% | 255568 / 256484 |
| references-4 | 49.164 | 49.382 | 1.0149 / 0.9963 | 62.78% | 12880 / 12924 |
| template-semantics-4 | 257.800 | 263.431 | 1.0265 / 1.0347 | 1.77% | 36992 / 36868 |
| references-8000 | 114.007 | 114.494 | 1.0106 / 1.0037 | 1.46% | 24516 / 24716 |

The final full campaign's median changes are −0.71% to +2.91%. The maximum
positive paired peak-RSS difference is 916 KiB. Initial measurements included
+15,680 KiB on memory-float-4 and are retained; peak differences vary between
campaigns. No memory improvement is claimed. Small reference/template groups
below 20× startup are diagnostic: startup is about 5–6 ms.

The final follow-up repeats only the disclosed noisy groups:

| Input | A ms | B ms | B/A blocks | A/A spread | A/B peak KiB |
| --- | ---: | ---: | --- | ---: | --- |
| calls-1 | 394.724 | 397.826 | 1.0056 / 1.0112 | 3.76% | 73136 / 73372 |
| memory-float-1 | 341.882 | 339.246 | 1.1999 / 0.9820 | 1.54% | 67220 / 63728 |
| calls-4 | 1612.604 | 1621.958 | 0.9811 / 1.0297 | 2.23% | 294868 / 281392 |
| references-4 | 48.223 | 48.242 | 1.0007 / 1.0032 | 1.40% | 12876 / 12924 |

One memory-float-1 follow-up observation still takes 0.4811 s wall versus
0.3391 s adjacent, while both use about 0.33 s CPU. The cause of the extra wall
time is not established; all observations remain included. The initial campaign
also had isolated wall spikes and a noisy calibration; its follow-up ratios
were 1.0098/0.9996 (memory-float-1), 1.0245/1.0111 (calls-4) and 1.0195/1.0054
(references-8000). These measurements support a small necessary semantic cost,
not a repeatable speedup or a precise sub-percent claim.

## Executable measurements

The three common native pairs are **byte-identical**, including the checked
results and executable payload sizes. The supplied backend emits sectionless
ELFs; these no-static-data programs use payload after the ELF entry as the
recorded text proxy. It is not an IR-size performance claim.

| Program | A/B median ms | B/A blocks | A/A spread | A/B text proxy bytes |
| --- | --- | --- | ---: | --- |
| calls-long | 481.146 / 480.972 | 1.0056 / 0.9964 | 1.25% | 206 / 206 |
| memory-long | 279.617 / 280.618 | 0.9989 / 1.0102 | 0.72% | 434 / 434 |
| floating-long | 331.346 / 332.089 | 1.0362 / 0.9974 | 0.87% | 230 / 230 |

## Newly correct behavior and scaling

A cannot implement these inputs correctly, so [the fixed generator](../student.tests/pa11/selection_benchmark.py)
uses B-only AAAA observations. Every LowIR is validated and executed before
measurement. Telemetry is a separate invocation.

| Input | Median ms | Wall range ms | Peak KiB | Candidates | Instructions |
| --- | ---: | --- | ---: | ---: | ---: |
| families-1000 | 240.679 | 236.042–242.469 | 44668 | 5001 | 38004 |
| families-4000 | 959.728 | 958.144–961.461 | 165564 | 20001 | 152004 |
| base-depth-8 | 38.857 | 38.614–39.056 | 10268 | 1001 | 5015 |
| base-depth-32 | 42.090 | 42.023–45.072 | 10324 | 1001 | 5015 |

Four times the independent families produces 3.99× wall time and 3.71× peak RSS.
Candidates are 5N+1 and emitted instructions 38N+4, matching the owned work.
At fixed 1,000 ADL queries, base depth 8→32 retains 1,001 candidates and 5,015
instructions. This scales relevant base edges, not source bytes; the short
wall observations are diagnostic. The existing lookup counter does not separately
count ADL type/edge visits, so it is not used as a complete ADL-work claim.

The new runtime checks **48 million** volatile-bounded constructor, operator and
hidden-friend iterations: median **409.573 ms**, range **408.980–410.643 ms**,
peak RSS **256 KiB**, text proxy **236 bytes**, checked exit 0. These observations
measure necessary new behavior without claiming profit over an incorrect A.

## Stage-scoped acceptance

PA11 mandates correct O0 object semantics and proportional work; it mandates no
numeric compiler-time/RSS/text ratio. Historical 1.10× latency, 1.20× RSS +16 MiB,
+128 KiB text, and 4×-input 5.5×-time/5×-RSS targets remain diagnostics, not extra
exit gates. Their observations are preserved above and in the raw files. The
array expansion limit stays eight elements. No optional optimization is proposed;
no runtime speedup is claimed. Correctness, unchanged coverage and explicit
ownership/complexity requirements remain mandatory. Native selection/allocation
and general value transfer stay with their owning later stages.
