# PA14 checkpoint performance review

This is an O0 semantic implementation checkpoint, **222/314** course tests.
It adds no target optimization pass or student native backend. New template
behavior cannot be compared with the incorrect stage-entry output for a speedup.
Common correct outputs and all three common executables are byte-identical.

## Frozen protocol and retained evidence

All compiler binaries, inputs, outputs and harnesses have SHA-256 identities in
the raw data. Compiler builds use `g++ -std=gnu++11 -Wall -O3`, with the course
test runner enabled. Each workload records its actual mode; LowIR and supplied
native execution use O0. Timing runs omit telemetry and audit validation; a
separate checked run records semantic counters and validates the LowIR.

Each common workload has one warmup per binary, four A/A observations, then
two ABBA blocks. Newly correct workloads have one warmup and six observations
on the working binary. Wall time, process peak RSS, child user/system CPU time
and context switches are retained, including every warmup and outlier. The
benchmark is pinned to the first available CPU (recorded in each JSON). Short
source-read comparisons batch compiles and preserve every constituent sample.

| Evidence | Frozen code | Timed process observations |
| --- | --- | ---: |
| [Initial implementation](../student.tests/pa14/preliminary-performance.json) | A `8af3c149` / B `c48def7d` | 238 |
| [Source-read correction](../student.tests/pa14/graph-fastpath-performance.json) | A / C `7e88952a` | 238 |
| [Isolated B/C comparison](../student.tests/pa14/graph-read-performance.json) | B / C, four affected workloads | 196 |
| [Call-context implementation](../student.tests/pa14/call-context-preliminary-performance.json) | A / D `e0eb788f` | 238 |
| [Final repeat](../student.tests/pa14/performance.json) | A / D, same fixed corpus | 238 |

Total: **1,148 timed processes**. The first D campaign overlapped sanitizer
validation work. It is preserved; the final repeat ran without another build
or validation job. Both contain occasional wall stalls. No observation was
removed or timing threshold used to filter the data. The [verifier](../student.tests/pa14/verify_performance.py)
checks all frozen hashes, observation orders, output equality, sizes, counters
and paired ratios. Artifacts live in `$RALPH_ARTIFACT_DIR/pa14-measurements/`.

## Current compiler cost

The table uses final-run medians over measured observations, excluding warmups.
RSS is the median of each process's peak RSS, in KiB. ABBA columns are the two
paired D/A wall ratios; these are primary timing evidence. A/A ranges show the
four calibration observations and do not bound later scheduling stalls.

| Common workload | A / D wall seconds | A / D peak RSS KiB | A/A wall range seconds | Paired D/A |
| --- | --- | --- | --- | --- |
| calls-1 | .417117 / .430411 | 76732 / 79666 | .410153–.424969 | 1.076, 1.021 |
| memory-float-1 | .359102 / .367094 | 70018 / 69008 | .355756–.361506 | 1.023, 1.024 |
| references-1 | .017265 / .017636 | 6918 / 7340 | .017234–.017864 | 1.024, 1.030 |
| template-semantics-1 | .070329 / .072690 | 12736 / 13492 | .070293–.070809 | 1.031, 1.030 |
| calls-4 | 1.704603 / 1.770984 | 294148 / 305432 | 1.696638–1.795356 | 1.042, 1.034 |
| memory-float-4 | 1.442882 / 1.520519 | 251478 / 266444 | 1.431519–1.451582 | 1.113, 1.053 |
| references-4 | .051648 / .053526 | 14262 / 14882 | .051599–.052632 | 1.039, 1.038 |
| template-semantics-4 | .267886 / .281081 | 37376 / 39592 | .267131–.269329 | 1.584, 1.040 |

The final large-workload median latency increases are about 3.6–5.4%; peak RSS
increases are about 3.8–6.0%. Some paired wall ratios are substantially worse.
For example, the final memory blocks have child-CPU ratios 1.038/1.046 while
wall ratios are 1.113/1.053; the semantic blocks have CPU ratios 1.020/1.038
while wall ratios are 1.584/1.040. In the first D run, memory's second block is
1.172 wall but 1.032 child CPU. These diagnostics support an environmental wait
component; they do not replace the wall observations with a speedup claim.
The short compiler inputs are startup-sensitive and support no fine timing claim.

The structural costs include an eight-byte source/context occurrence index per
parsed node and required canonical specialization/completion queries. They are
bounded by source and demanded facts. The following correction removed measured
avoidable graph-access work. Remaining whole-region semantic checking is an
explicit open architecture requirement in [implementation.md](implementation.md),
not excused as an unavoidable permanent template cost.

## Source-read correction and growth

B read every source node through an out-of-line projected view. C keeps an
inline ordinary-source branch and projects only contextual occurrences. The
isolated comparison uses identical outputs and these paired C/B wall ratios:

| Affected workload | C/B blocks | B / C peak RSS KiB |
| --- | --- | --- |
| calls-4 | .926, .944 | 305450 / 305514 |
| memory-float-4 | 1.027, .926 | 266898 / 266914 |
| template-semantics-4 | .928, .831 | 39512 / 39580 |
| class-instances-1000 | 1.005, .923 | 29238 / 29162 |

Calls and semantic workloads show repeatable benefit, about 5.6–7.4% and
7.2–16.9% respectively. The other two are mixed; their medians are slightly
worse, so no benefit is claimed for them. No extra node allocation or target IR
is introduced. Work is O(1) per read. The review's explicit maintenance budget
for this access change is one source/context branch, no allocation, zero target
IR growth, and at most 64 KiB added compiler `.text` relative to B. This is a
documented local review budget, not a handout limit or a retroactive stage gate.

Compiler `.text`: A **1,023,942**, B **1,057,926**, C **1,114,950**, D
**1,116,230** bytes. B→C costs **57,024 bytes (5.39%)**, including the static
function-address demand correction. The final call-context work adds 1,280 bytes.
Total stage growth is **92,288 bytes (9.01%)**. The source-read benefit justifies
its bounded host-code growth; no generated-code or runtime improvement is claimed.

## Newly correct templates and scaling

These are D-only costs. A lacks required bodies/completion and is not an
equivalent implementation for these workloads.

| Workload | Median compiler seconds / peak RSS KiB | Existing-work counters |
| --- | --- | --- |
| 1000 calls to one specialization | .035853 / 10066 | 25032 parsed nodes; **1** body transition; **24** occurrences |
| 4000 calls to one specialization | .129973 / 26204 | 100032 parsed nodes; **1** body transition; **24** occurrences |
| 250 distinct class specializations | .039846 / 10930 | **250** completions; **8000** occurrences |
| 1000 distinct class specializations | .145117 / 29210 | **1000** completions; **32000** occurrences |

Fourfold input growth changes repeated-call latency by 3.63× and class latency
by 3.64×; RSS changes by 2.60×/2.67×. Repeated calls do not recompute a body.
Independent class work grows exactly fourfold in completion/occurrence counters.
The preliminary class wall scaling was 6.27×, while the later C/D measurements
and work counters do not repeat it. That historical diagnostic miss remains in
the raw data; it is not an additional permanent exit gate.

## Executable runtime and size

The live workloads use volatile bounds and checked results: 96 million call,
64 million memory, 32 million floating-point, and 16 million template iterations.
Native construction is performed by PA8's supplied backend
after validating student-produced LowIR. This is validation, not implementation.
The backend produces sectionless ELF: its size metric is payload after the entry
point, including support/data, rather than a claimed ELF `.text` section size.

| Executable | A / D median runtime seconds | D/A ABBA ratios | A / D payload bytes |
| --- | --- | --- | --- |
| calls | .479496 / .477780 | .991, .998 | 206 / 206 |
| memory | .278994 / .280218 | .989, 1.001 | 434 / 434 |
| floating point | .331389 / .331369 | .997, 1.001 | 230 / 230 |
| template calls/class mutation | unavailable / .081461 | D-only | unavailable / 183 |

The three common executable files are byte-identical, including code and data.
Their small timing differences are variation, not an optimization result.
Their compiler invocations take roughly .0057–.0061 seconds and 4.8–5.2 MiB
peak RSS; these are startup-dominated. The new template executable compiles in
.006256 seconds at 5062 KiB median peak RSS. All generated-program observations
returned the checked success result. Raw executable RSS observations are retained
but this review makes no claim from their coarse 256 KiB reporting floor.

## Stage-scoped acceptance

PA14 owns unoptimized typed LowIR and first-tier templates. It mandates no
numeric compiler latency/RSS threshold or optional optimizer. Later native,
allocation, optimization and self-hosting budgets retain their owning stages.
Historical PA13 diagnostic thresholds and the preliminary scaling miss do not
override the spec's current-stage acceptance. Their evidence remains preserved
in the [PA13 review](../pa13/final-audit-performance.md) and this review's raw data.

Correctness, coverage, comparison rules and the spec's complexity/ownership
requirements remain mandatory. The graph-access regression was addressed;
remaining semantic architecture work is recorded rather than waived. This
performance review does not declare the incomplete PA14 stage complete.
