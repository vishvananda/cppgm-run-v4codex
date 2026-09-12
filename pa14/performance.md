# PA14 checkpoint performance review

Earlier sections preserve the **222/314** and **281/314** checkpoint evidence.
The final section records the **297/314** symbolic-query/binding implementation
and its corrected enclosing-environment cache key. This is an O0 semantic
implementation.
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

## Dependent types and definition owners: current continuation

[Raw evidence](../student.tests/pa14/definition-performance.json), produced by
[the frozen harness](../student.tests/pa14/definition_benchmark.py), compares E
`4fafa38c` (same compiler bytes as D) with F `16e7c163`. The protocol and host/O0
flags above are retained: one warmup, four A/A observations and two ABBA blocks
for every common workload; one warmup and six F-only observations for new
semantics. This campaign ran after builds and sanitizer/course validation had
finished. Every observation, including wall stalls, is preserved. New artifacts
live under `$RALPH_ARTIFACT_DIR/pa14-dependent/performance-work/`.

There are **308 new timed processes**, hence **1,456 total preserved observations**.
The verifier checks all six raw datasets. All 16 common compiler outputs and all
four common executables are byte-identical. The two new definition corpora and
new executable use F only: E lacks required definitions, so its failure cannot
be treated as faster equivalent compilation.

| Compiler workload | E / F median wall seconds | E / F median peak RSS KiB | A/A range seconds | Paired F/E wall ratios |
| --- | --- | --- | --- | --- |
| calls-1 | .429755 / .433671 | 79670 / 79754 | .424769–.430028 | 1.009, .930 |
| memory-float-1 | .364848 / .365697 | 69020 / 69094 | .362317–.374394 | 1.151, .986 |
| calls-4 | 1.747527 / 1.742856 | 305474 / 305520 | 1.730739–1.812201 | .996, .959 |
| memory-float-4 | 1.486482 / 1.490231 | 266660 / 266680 | 1.467547–1.490287 | .968, 1.009 |
| references-4 | .053056 / .053113 | 14986 / 15000 | .052644–.053750 | 1.005, .997 |
| template-semantics-4 | .275220 / .277662 | 38826 / 38932 | .271961–.278068 | 1.017, .620 |
| repeated-template-4000 | .127354 / .127968 | 26246 / 26082 | .125981–.128834 | 1.003, .997 |
| class-instances-250 | .038738 / .039633 | 10908 / 11148 | .038541–.039038 | 1.010, 1.019 |
| class-instances-1000 | .142738 / .144451 | 29084 / 29398 | .140569–.145086 | .789, 1.018 |

Large ordinary medians change by less than 1%; RSS is effectively unchanged.
Class medians increase 1.2–2.3%, with 240–314 KiB higher median peak RSS. Canonical
member-definition provenance/index queries are now part of class creation and
demand. Work remains indexed by the requested owner, with no extra target code.
Default-head binding maps are constructed only for actual defaults crossing
heads; ordinary templates incur no such map population. The .620/.789 paired
results and the 1.151 memory block coexist with near-equal medians: this is noisy
wall evidence, not a claimed speedup or an excuse to discard observations.

Compiler `.text` is E **1,116,230** / F **1,151,238** bytes: **35,008 bytes (3.14%)**
added for required semantics. Total stage growth from A is **127,296 bytes
(12.43%)**. No optional target optimization or inlining policy was added. The
previous source-read change and its explicit local budget are unchanged. No new
numeric gate is imposed on correctness work; the spec still requires indexed
work, dependent-only semantic reuse and bounded fact ownership.

| Newly correct definition workload | F median wall / peak RSS | Class completions / definition applications / occurrences |
| --- | --- | --- |
| 1000 distinct classes | .273597 s / 49774 KiB | 1000 / 3000 / 137000 |
| 4000 distinct classes | 1.168195 s / 179910 KiB | 4000 / 12000 / 548000 |

Each class demands one nested-class, one function and one static-data definition;
repeated internal demands leave **exactly three applications** per concrete owner.
Fourfold source growth yields **4.27×** median wall and **3.61×** RSS. Completion,
definition application, expression and occurrence counters grow exactly fourfold;
lookup work is 53,006 / 212,006. This supports the registry's indexed scaling on
these inputs. It does not establish that remaining whole-region semantic checking
satisfies the dependent-only requirement; that remains explicitly open.

| Executable | E / F median runtime seconds | Paired F/E | E / F payload bytes |
| --- | --- | --- | --- |
| calls, 96M iterations | .477354 / .477684 | .997, 1.000 | 206 / 206 |
| memory, 64M iterations | .279450 / .279393 | 1.007, .999 | 434 / 434 |
| floating point, 32M iterations | .330688 / .331055 | 1.016, 1.002 | 230 / 230 |
| common template, 16M iterations | .081224 / .081335 | 1.001, 1.000 | 183 / 183 |
| out-of-class template member, 32M iterations | unavailable / .163888 | F only | unavailable / 172 |

All runs check live results using volatile bounds. Common runtime differences
are timing variation between identical binaries. The new member executable
compiles in **.006275 s / 5120 KiB** median peak RSS; the common small runtime
sources compile in roughly .0057–.0060 s, dominated by startup. Native payload
uses the supplied sectionless ELF metric described above. No runtime/code-size
optimization benefit is claimed. These costs are recorded alongside compiler
work and code growth, with correctness and remaining PA14 architecture obligations
preserved. The stage remains incomplete at **281/314**.

## Symbolic queries, fixed binding and packed records: 297/314

Entry `e27474c2` is compared with final code `2dc67391`. Intermediate binaries
and observations are retained. The protocol, CPU affinity, flags, exact source
hashes, output hashes and executable hashes are recorded in each file.

| New evidence | Frozen code | Timed process observations |
| --- | --- | ---: |
| [Preliminary queries/binding](../student.tests/pa14/symbolic-preliminary-performance.json) | entry / `af95f4d7` | 378 |
| [Enclosing-context correction](../student.tests/pa14/symbolic-context-performance.json) | entry / `47f5f975` | 378 |
| [Final packed implementation](../student.tests/pa14/symbolic-performance.json) | entry / `2dc67391` | 378 |
| [Direct packing comparison](../student.tests/pa14/packing-performance.json) | `47f5f975` / `2dc67391` | 70 |

Together with the preserved 1,456 historical observations, **2,660 timed
processes verify**. Each common campaign includes two warmups, four A/A samples
and two ABBA blocks; new-only workloads include a warmup and six samples. All
samples, including warmups and outliers, are retained. The preliminary campaign
preceded discovery of an enclosing-specialization cache-key bug; it is not
acceptance evidence for that behavior. A short native diagnostic also overlapped
part of that preliminary campaign. Neither issue is hidden by deleting samples.
The corrected and final campaigns ran separately from builds and tests.

The final campaign verifies **19 byte-identical common compiler outputs** and
**five byte-identical common native executables**. New dependent-query and fixed
binding workloads run only on the final compiler where the entry is incorrect
or unsupported. Compiler timing excludes telemetry and validation; separate
runs collect existing work counters and validate LowIR. Runtime inputs remain
volatile and results are checked. There is no optional target optimization pass,
assembly roundtrip or student native backend in this O0 stage.

### Compiler latency and peak memory

Wall values are observation medians in seconds. RSS is median per-process peak
KiB. Both ABBA ratios are shown; ratios below one favor the final compiler.
These are diagnostic observations, not additional assignment exit gates.

| Common workload | Entry / final wall | Entry / final peak RSS | Two final/entry ABBA ratios |
| --- | --- | --- | --- |
| calls, 14,000 groups | 1.789161 / 1.760971 | 305658 / 305668 | 0.9964, 0.9457 |
| memory/float, 14,000 functions | 1.500998 / 1.501049 | 266648 / 266738 | 0.9763, 1.0104 |
| references, 3,200 links | 0.054091 / 0.053967 | 15032 / 14970 | 0.9955, 0.9962 |
| template semantics, 14,000 calls | 0.280722 / 0.291606 | 38908 / 38928 | 1.0198, 1.5384 |
| repeated specialization, 4,000 calls | 0.129213 / 0.128319 | 26096 / 26402 | 1.0021, 0.9963 |
| class instances, 1,000 | 0.150779 / 0.148181 | 29414 / 29466 | 0.9910, 0.9926 |
| retained definitions, 4,000 owners | 1.161043 / 1.180420 | 179902 / 184500 | 1.0188, 1.0078 |

Spread matters. Large calls span 1.7610–1.9138 s at entry and 1.7517–1.8005 s
at final; their A/A range is 1.7610–1.8463 s. The template-semantics workload has
one final 0.577835 s sample with only 0.22 s user + 0.05 s system CPU; it raises
the second paired ratio to 1.5384. A small compiler startup sample likewise takes
0.321161 s with rounded CPU times of zero. These wall/CPU gaps indicate delay
outside CPU execution; they do not establish extra semantic work. They remain
in the raw data and in the ratios. No general compiler speedup is claimed.

The context-corrected run isolated an avoidable representation cost:
`Expression` had grown from 36 to 40 bytes when null-pointer provenance was
added. [Frozen host-layout measurements](../student.tests/pa14/expression-layout.json)
confirm **36 / 40 / 36 bytes** for entry / unpacked / packed; `Entity` stays
**112 bytes** throughout. Flags now occupy one packed byte, without a new
allocation, semantic pass or additional query. The direct correct/correct
comparison shows:

| Packing workload | Unpacked / packed wall | Unpacked / packed peak RSS | Two packed/unpacked ratios |
| --- | --- | --- | --- |
| template semantics, 14,000 calls | 0.278637 / 0.277368 | 42718 / 39540 | 0.9927, 0.9971 |
| calls, 14,000 groups | 1.786970 / 1.805144 | 309958 / 305730 | 1.0065, 1.0046 |
| dependent queries, 4,000 types | 0.601702 / 0.593799 | 113026 / 111406 | 0.9421, 1.0062 |

Packing removes 3,178 KiB peak RSS in the semantic workload and 4,228 KiB in the
calls workload. Its large-calls A/A range is 1.7539–1.7923 s; the small paired
latency differences do not justify a speed claim. This bounded representation
repair costs **704 compiler text bytes** and restores the existing record budget.
Final compiler text is **1,216,518 bytes**, versus continuation entry
**1,151,238**: +65,280 bytes (+5.67%). Cumulative growth from stage base is
192,576 bytes (+18.81%). This is required frontend/ABI machinery, not an optional
optimization justified by an IR-size claim.

### Work scaling and generated programs

| New workload | 1,000 / 4,000 wall | 1,000 / 4,000 peak RSS | Work evidence |
| --- | --- | --- | --- |
| dependent member-reference queries | 0.146255 / 0.596761 | 31530 / 111300 | Query facts = 3N+6; definition binding work stays 12, with two name facts. |
| fixed names through dependent bases | 0.152969 / 0.621472 | 31294 / 109654 | Definition binding work stays 13, with three name facts. |

Four times the new query input yields **4.08× wall / 3.53× RSS**; fixed-binding
input yields **4.06× wall / 3.50× RSS**. The fixed binding work stays constant
while concrete declarations/bodies grow. This supports the implemented narrow
sharing; it does **not** prove that all nondependent body semantics are shared.
All earlier application/body/occurrence assertions continue to verify.

| Native workload | Entry / final median seconds | Native payload bytes, both |
| --- | --- | ---: |
| calls | 0.478020 / 0.477369 | 206 |
| memory | 0.281050 / 0.280489 | 434 |
| floating point | 0.331783 / 0.331707 | 230 |
| template member/function calls | 0.081322 / 0.081880 | 183 |
| retained member definitions | 0.162502 / 0.162259 | 172 |

The new dependent-query executable is **182 bytes**, runs in median
**0.183470 s** (0.183239–0.184015 s), and checks its computed result. In the
direct packing comparison it is byte-identical, with medians 0.183259 /
0.183835 s. One packed sample is 0.235065 s with 0.18 s user CPU; paired ratios
are 1.1411 and 1.0015. That outlier is preserved. No runtime or native-size gain
is claimed. Native size uses the supplied sectionless ELF payload metric stated
above, including support/data; compiler size uses `.text`.

Stage-scoped acceptance preserves mandated correctness/coverage/ownership and
all historical measurements. Unsupported inherited numeric diagnostic gates
remain diagnostics. There is no optional target optimizer work or code-growth
budget consumed in PA14. The packing repair has bounded constant work, no new
semantic traversals, and a measured memory benefit for its 704-byte compiler
cost. Remaining 17 course failures and the semantic-graph requirements remain
open; these measurements do not declare PA14 complete.

`python3 student.tests/pa14/verify_performance.py` checks frozen hashes, orders,
paired arithmetic, counters and record sizes. Full logs and frozen artifacts
are under `$RALPH_ARTIFACT_DIR/pa14-symbolic/`.
