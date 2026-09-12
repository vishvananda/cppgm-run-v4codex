# PA14 checkpoint performance review

Earlier sections preserve the 222/314, 281/314 and 297/314 checkpoint evidence.
The final section records the 314/314 object-transfer/lifetime implementation.
Each campaign states its own equivalent-output checks. An implementation with
incorrect new behavior is not a speedup baseline. All observations are retained.
This O0 implementation adds no optional optimizer or student native backend.

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

## PA14 transfer/lifetime campaign (314/314)

The [preliminary campaign](../student.tests/pa14/transfer-preliminary-performance.json)
compares entry `2c80bc70` with `6fb57328`. It retains 350 observations, including
warmups and every scheduling outlier. Its cost review exposed preparation of
known-deleted implicit copies. Commit `177f7545` completes that negative fact
before creating scopes, parameters or transfer actions. A separate final run
uses the same [frozen harness](../student.tests/pa14/transfer_benchmark.py), inputs
and entry binary; the preliminary artifacts remain unchanged.

The O0 implementation budgets are explicit: each transfer fact is prepared
once, each reference action emits at most six instructions plus fixed object
setup, cumulative array expansion stays at eight, and deleting-entry sharing
covers at most one nontrivial subobject. Direct late-defaulted construction
requires a proven representation transfer and otherwise retains a call. These
are bounded construction policies, not a new optimization pass. The measured
record sizes remain 112 bytes per declaration and 36 bytes per expression;
[layout evidence](../student.tests/pa14/emission-layout.json) verifies both sides
of moving emission-use flags into existing padding.

The 4,000-specialization preliminary case retained 16,000 transfer actions and
emitted 172,000 instructions versus the entry's 8,000 actions and 148,000
instructions. Six explicit reference instructions per specialization account
for the IR increase. Its wall medians were 1.262162/1.374061 seconds, with paired
ratios 1.2533/1.0681. A B observation at 1.772677 seconds used 1.03 user + .23
system seconds; it remains in the first pair. The binding-4,000 pairs were
1.0538/1.0326, also retained. The template-semantics A stall at .562312 seconds
(.22 user + .05 system) produced a .6887 first-pair ratio; no compiler speedup
is inferred from it. Late-copy native observations include A .168351 and
B .142094 seconds; neither was filtered.


### Deleted-fact comparison (before the volatile-return boundary check)

[Deleted-fact raw data](../student.tests/pa14/transfer-deleted-performance.json) compares
`2c80bc70` with `177f7545`, without concurrent builds or tests. Together the two
new campaigns contain **700 timed process observations**; at this checkpoint the verifier checked
**3,360** across all PA14 campaigns. Fourteen common compiler outputs and four
common executables are byte-identical. Two transfer scaling inputs retain the
same binding operations; their native reducers and the late-copy reducer check
both implementations' results. The return-slot case is B-only because A moves
twice and fails its required count.

Wall/RSS values below are medians of measured observations (warmups remain in
the raw data). RSS is each process's peak in KiB; paired wall ratios include
all observations. A/A ranges do not bound later scheduling stalls.

| Compiler workload | A / B wall s | A / B peak RSS KiB | A/A wall range s | Paired B/A |
| --- | --- | --- | --- | --- |
| calls-4 | 1.790358 / 1.776011 | 305634 / 305624 | 1.755269–1.832074 | 0.9890, 1.0249 |
| memory-float-4 | 1.500108 / 1.505510 | 266758 / 266714 | 1.490982–1.510406 | 0.9964, 0.9916 |
| template-semantics-4 | 0.280988 / 0.278503 | 38932 / 38952 | 0.280179–0.284546 | 1.0014, 1.3474 |
| query-instances-4000 | 0.595981 / 0.600911 | 111340 / 111240 | 0.590655–0.601819 | 1.0013, 1.0283 |
| binding-instances-4000 | 0.626590 / 0.631126 | 109660 / 109358 | 0.617939–0.624691 | 0.9990, 0.9962 |
| transfer-instances-1000 | 0.296639 / 0.301672 | 54372 / 55116 | 0.291527–0.297457 | 1.0139, 1.0066 |
| transfer-instances-4000 | 1.239556 / 1.270937 | 200238 / 205384 | 1.224690–1.240262 | 0.9773, 1.0204 |

| Native workload | A / B runtime s | A / B executable payload bytes | Paired B/A |
| --- | --- | --- | --- |
| calls-runtime | 0.477432 / 0.480184 | 206 / 206 | 1.0097, 1.0032 |
| memory-runtime | 0.281006 / 0.279786 | 434 / 434 | 1.0001, 1.0009 |
| floating-runtime | 0.331571 / 0.330852 | 230 / 230 | 0.9929, 1.0067 |
| query-runtime | 0.183189 / 0.183046 | 182 / 182 | 1.0111, 1.0016 |
| reference-move-runtime | 0.352279 / 0.117849 | 256 / 268 | 0.3345, 0.3337 |
| late-copy-runtime | 0.115526 / 0.100830 | 266 / 222 | 0.8644, 0.8734 |
| return-slot-runtime | — / 0.123003 | — / 472 | Correct B only |

All native processes report 256 KiB peak RSS. Runtime-source compilation is
startup-dominated (roughly 5.6–6.4 ms, 5.0–5.5 MiB RSS); its full compiler
observations remain in the raw data and are not used for a compiler speedup
claim. The supplied backend emits sectionless ELF, so the program size metric
is executable payload after entry, including any support/data there, rather
than an independently recoverable `.text` section.

The reference move is repeatably about **3× faster** for **12 additional payload
bytes**. The prefix now uses an eight-byte transfer followed by the explicit
reference binding action. The late-defaulted copy is **12.7–13.6% faster** in the
final pairs and removes **44 payload bytes** by consuming the proven storage
transfer directly. Both preserve checked results and source-reference identity;
this is executable evidence, not an inference from fewer IR instructions.
The corrected return path costs .123003 seconds and 472 payload bytes; no
speedup is claimed against its incorrect predecessor.

Compiler `.text` grows from **1,216,518 to 1,223,302 bytes**: **6,784 bytes
(0.56%)** this continuation, including 64 bytes for completing known-deleted
facts early. The cumulative increase from the stage base is 199,360 bytes.
Final transfer-4,000 compilation has a 2.5% higher wall median and 5,146 KiB
higher median peak RSS than A. Its paired wall ratios are .9773/1.0204; the
first includes an A stall at 1.343170 seconds. The selected move's extra six
instructions per specialization explain the 148,000→172,000 instruction and
8,231,201→9,184,093 serialized-byte growth. The action budget remains linear;
there is no new pass or repeated scan.

The negative-fact correction removes **4,000 entities, 4,000 scopes and 8,000
unused actions** from the preliminary B at 4,000 specializations. Final B has
92,009 entities, 48,005 scopes and 8,000 transfer actions. Fourfold input gives
4.21× wall and 3.73× RSS; instructions and selected actions grow exactly 4×,
while fixed binding work stays 23. These are work-count improvements; the two
campaigns are not an isolated B/B timing experiment, so no isolated latency or
RSS benefit is claimed for that correction.

The final run retains a template-semantics B stall that produces a **1.3474**
second-pair ratio, and a small-workload A/A stall at **.266296 seconds**. These
observations are neither removed nor converted into a throughput claim. The
preliminary binding-4,000 slowdown is preserved; the final pairs are
.9990/.9962. There is no general compiler speedup claim.

The measured executable gains justify the bounded transfer work and small
compiler/native growth. Necessary O0 reference actions retain their measured
serialization/memory cost. No mandated numeric performance threshold is added
or weakened; historical self-imposed gates remain diagnostics under the
stage-scoped acceptance rule. Correctness, complete coverage, explicit action
budgets and frozen evidence remain required. Broader shared-body/demand-graph
work in the plan remains current PA14 scope.

### Final volatile-boundary binary and evidence

The [final campaign](../student.tests/pa14/transfer-performance.json) compares
entry `2c80bc70` with **`c7507efd`**. The volatile-source eligibility correction
is required by N3485 [class.copy]/31–32; its separate frozen reducer checks one
copy and no move. All 19 benchmark compiler outputs and seven executables are
byte-identical to the preceding B campaigns. The full repeated campaign adds
350 observations, for **1,050 this continuation and 3,710 verified in total**.
No earlier campaign or outlier has been discarded.

| Final compiler workload | A / B wall s | A / B peak RSS KiB | A/A wall range s | Paired B/A |
| --- | --- | --- | --- | --- |
| calls-4 | 1.768822 / 1.771949 | 305638 / 305646 | 1.760518–1.829441 | 1.0062, 1.0011 |
| memory-float-4 | 1.494878 / 1.499050 | 261458 / 261364 | 1.489128–1.500390 | 0.9996, 1.0047 |
| template-semantics-4 | 0.279857 / 0.281473 | 39700 / 39732 | 0.279368–0.281923 | 1.0083, 1.3797 |
| query-instances-4000 | 0.589273 / 0.591730 | 111590 / 111628 | 0.582145–0.605259 | 1.0063, 1.0351 |
| binding-instances-4000 | 0.629085 / 0.632280 | 109648 / 109688 | 0.624333–0.634825 | 1.0017, 1.0033 |
| transfer-instances-1000 | 0.300896 / 0.305805 | 54436 / 54880 | 0.299140–0.303039 | 1.0155, 1.0202 |
| transfer-instances-4000 | 1.238073 / 1.256695 | 200228 / 203474 | 1.227882–1.242997 | 0.9787, 0.9906 |

| Final native workload | A / B runtime s | A / B payload bytes | Paired B/A |
| --- | --- | --- | --- |
| calls-runtime | 0.477830 / 0.479215 | 206 / 206 | 0.9955, 1.0162 |
| memory-runtime | 0.280447 / 0.280135 | 434 / 434 | 0.9985, 0.9971 |
| floating-runtime | 0.332336 / 0.331453 | 230 / 230 | 0.9953, 0.9967 |
| query-runtime | 0.184318 / 0.184529 | 182 / 182 | 1.0446, 0.9943 |
| reference-move-runtime | 0.349302 / 0.119073 | 256 / 268 | 0.3388, 0.3402 |
| late-copy-runtime | 0.116140 / 0.100721 | 266 / 222 | 0.8632, 0.8777 |
| return-slot-runtime | — / 0.123547 | — / 472 | Correct B only |

The final compiler `.text` is unchanged at **1,223,302 bytes** (entry +6,784,
0.56%). The final reference-move pairs are .3388/.3402 (about 3× faster,
+12 payload bytes); late-copy pairs are .8632/.8777 (12.2–13.7% faster,
−44 bytes). All native peak RSS values remain 256 KiB. Runtime-source compiles
remain startup-dominated; their latency/RSS observations are preserved without
claiming a compiler speedup. The return path is still a correct-B-only result.

At 4,000 transfers, final B has a **1.5% higher wall median** and **3,246 KiB
higher median peak RSS** than A. Fourfold input gives **4.11× wall, 3.71× RSS**
and exactly fourfold actions/instructions. The paired wall ratios .9787/.9906
include slower A block observations (notably 1.338612 seconds); they do not
establish a general compiler speedup. This retains the bounded six-instruction
reference action and the earlier measured output-size increase.

Scheduling spread remains visible: A memory-float-1 takes .546021 seconds
(.28 user + .08 system), A template-semantics-1 takes .368702 (.05 + .01),
and B template-semantics-4 takes .490137 (.22 + .06), producing its **1.3797**
second-pair ratio. The smaller template B sample at .098772 seconds is also
retained. These are disclosures, not filtered observations or new exit gates.

The preceding budget/acceptance analysis therefore still applies to the final
binary. Transfer runtime benefits repeat, the compiler/IR growth remains
bounded and disclosed, and the redundant negative-fact preparation is gone.
The verifier checks all hashes, observation orders, paired results, work counts,
record sizes and repeated compiler/native outputs. Final correctness/status
artifacts are under `$RALPH_ARTIFACT_DIR/pa14-transfer/`, including the reduced
volatile-return proof and the full 1,935/1,935 through report.

## Fixed scalar body facts: final evidence (`d19a1ff7`)

A is the frozen continuation-entry `c05778ed` compiler (implementation `c7507efd`);
B is the direct-result fixed-fact implementation `d19a1ff7`. The initial
`b22e683f` binary has two preserved campaigns, including the repeat that motivated
removing its extra result temporary. `body_benchmark.py` is frozen at `d0030349`.
All three campaigns retain one warmup per binary, four A/A observations and two
ABBA blocks per compiler/native command. They add **1,008 observations**, bringing
verified history to **4,718**. No samples or previous gates/measurements were erased.

All **19 compiler outputs and five native outputs are byte-identical** between
A/B and across the three campaigns. Sources, binaries, harness/backend hashes,
flags, CPU affinity, wall/user/system time, peak RSS and context-switch counts
are retained in `student.tests/pa14/body*-performance.json` and
`$RALPH_ARTIFACT_DIR/pa14-body-facts/`. No builds or tests ran during timing.

| Final compiler workload | A / B wall s | A / B RSS KiB | A/A range s | Paired B/A |
| --- | --- | --- | --- | --- |
| calls-1 | 0.664348 / 0.448893 | 79880 / 79882 | 0.432105–0.796763 | 0.6785, 0.6071 |
| calls-4 | 1.758530 / 1.956218 | 305638 / 305592 | 1.757587–2.395504 | 1.0179, 1.0934 |
| memory-float-1 | 0.369888 / 0.370933 | 69210 / 69258 | 0.365098–0.487425 | 1.0094, 0.7956 |
| memory-float-4 | 1.506896 / 1.509456 | 261354 / 261408 | 1.490878–1.536355 | 1.0060, 0.9910 |
| template-semantics-1 | 0.072642 / 0.071997 | 13518 / 13224 | 0.072381–0.091587 | 0.9190, 1.0203 |
| template-semantics-4 | 0.280224 / 0.279449 | 39706 / 39692 | 0.276377–0.289152 | 0.6719, 0.9876 |
| query-instances-1000 | 0.146083 / 0.146953 | 31652 / 31778 | 0.143862–0.152566 | 1.0088, 1.0053 |
| query-instances-4000 | 0.594335 / 0.597662 | 111608 / 111692 | 0.594300–0.598742 | 1.0379, 1.0103 |
| binding-instances-1000 | 0.151066 / 0.150158 | 30904 / 30776 | 0.148782–0.153345 | 1.1843, 0.9957 |
| binding-instances-4000 | 0.623317 / 0.644892 | 109706 / 109982 | 0.620264–0.737100 | 1.0427, 1.0127 |
| fixed-instances-1000 | 0.160099 / 0.151760 | 31612 / 31080 | 0.157769–0.161860 | 0.9495, 0.9238 |
| fixed-unused-1000 | 0.086422 / 0.096511 | 18270 / 19754 | 0.086322–0.091997 | 1.1282, 1.0672 |
| fixed-instances-4000 | 0.670440 / 0.649078 | 111224 / 108870 | 0.657382–0.683033 | 0.9638, 1.0169 |
| fixed-unused-4000 | 0.337981 / 0.390816 | 58784 / 65040 | 0.336771–0.338336 | 1.1843, 1.1212 |

| Final native workload | A / B runtime s | Payload bytes (both) | Paired B/A |
| --- | --- | --- | --- |
| calls-runtime | 0.480416 / 0.480843 | 206 | 1.0002, 0.9996 |
| memory-runtime | 0.280152 / 0.280083 | 434 | 0.9959, 1.0022 |
| floating-runtime | 0.330596 / 0.331071 | 230 | 0.9995, 1.0018 |
| query-runtime | 0.183145 / 0.183776 | 182 | 1.0012, 0.9995 |
| fixed-runtime | 0.228685 / 0.228770 | 268 | 0.9900, 1.0034 |

All native peak RSS observations are 256 KiB. The text metric remains actual
compiler `.text` and the supplied sectionless ELF payload after entry (including
support/data), not an independently recoverable native `.text` section. Compiler
latency for runtime sources remains about 6 ms with roughly 5 MiB peak RSS;
full startup-dominated observations remain in the JSON, without speedup claims.

The fixed-instance corpus establishes **20 source-owned expression facts**.
At N=4,000, expression checking falls from 92,000 to 12,020 operations, conversion
selection from 72,000 to 24,012, and conversion records from 80,000 to 24,014.
Only operand consumption and concrete declaration identity remain contextual.
Fourfold input gives 4.28× B wall and 3.50× B RSS; occurrence nodes remain exactly
76N, so complete dependent-only projection is still an architectural limit.

For 1,000 demanded specializations, final pairs improve **5.1–7.6%**, with 532 KiB
less median RSS. At 4,000, the wall median improves 3.2% and RSS falls 2,354 KiB,
but paired results are mixed (.9638/1.0169). The earlier two 4,000-instance
campaigns had pairs .9337/.9620 and .9473/.9493; the latter repeats the same binary.
These support the work/memory reduction and a benefit on the smaller affected
workload, not a general compiler speedup or a uniform large-workload timing gain.

Unused definitions now receive required operand validation. At 4,000 separate
patterns, B establishes 80,000 expression facts and 56,000 conversion records,
where A established none. This costs **52.8 ms and 6,256 KiB** in final medians.
The twelve retained rejection reducers are accepted by A and rejected by B;
they establish the missing definition-time behavior (PA14 README; N3485
[temp.res]/8, [temp.dep.expr]/1–4 and ordinary expression constraints). The valid
unused corpus still produces the same empty output, making its required checking
cost visible separately from repeated-specialization savings.

Compiler text is **1,227,910 bytes**, entry +4,608 (0.38%); declaration/expression
records remain **112/36 bytes**. The initial B was 1,228,038 bytes. Removing the
extra wrapper temporary reduces text/work but has no isolated B/B speedup claim.

Timing spread and common-workload costs remain disclosed. Final calls-4 has an
11.2% higher B wall median and pairs 1.0179/1.0934, with essentially unchanged RSS.
A itself ranges 1.7576–2.3955 s in calibration; in the second ABBA block A runs
1.7331 and 2.3887 s, while B runs 2.0898 and 2.4169 s. User time also varies
(A 1.38–1.94 s, B 1.69–1.99 s in that block), so a stable cause cannot be assigned
to dispatch alone. The previous calls-4 pairs were 1.0012/1.1908 and 1.0196/1.0572.
The remaining ordinary-path cost merits review as the pending body graph removes
per-occurrence work; these observations are not discarded or called a speedup.

Other retained final spread includes A calls-1 at .7968/.7628/.7666 s, A
memory-float-1 at .4874/.5515 s, A template-semantics-4 at .5555 s (.23 user +
.04 system), and B binding-instances-1000 at .2087 s (.11 + .03). The first
campaign also retains its A query-1000 .4000 s observation, B binding-1000 .2298 s,
and the 4,000-instance A/A range .8822–1.0667 s. The raw files retain every other
observation; no outlier filter is an acceptance rule.

**O0 work/growth budget:** each eligible definition node is checked once; each
operand edge is consumed once per demanded occurrence; the reuse index has one
entry per eligible source node; conversions occupy one source-owned bounded-arity
slice. Eligibility adds no layout, overload, class materialization or global retry
work. Ordinary dispatch is constant work per expression. The generated-code
growth budget for this frontend reuse is **zero**, verified by exact IR/native
equality. No optional optimizer or student native backend was added.

The fixed support-code increase and source-proportional checking are justified
by required semantic fact sharing, twelve closed legality holes, lower repeated
conversion work/memory and the measured affected-workload benefit. The ordinary
path refinement removes identified redundant construction. There is no mandated
numeric compiler threshold at PA14; historical self-imposed gates remain
diagnostics under stage-scoped acceptance. This does not excuse an identified
avoidable regression, waive correctness/coverage or complete the remaining typed
body/demand architecture. All current required checks, native tests and sanitizer
checks pass; `verify_performance.py` validates the full retained history.


## Fixed calls and constructor argument recipes

Frozen entry A is `816c9dc0`; B is `421aa292`, incorporating `ba609e57`
(unified typed selection) and `358a9d5b` (fixed call facts/access). The frozen
`call_benchmark.py` extends all nineteen preceding body workloads with six
call/unused/materialization scaling inputs and two live native loops. One warmup
per binary, four A/A observations and two ABBA blocks give **476 observations**,
bringing retained verified history to **5,194**. Every sample is retained.
Binaries, inputs, flags, output hashes, wall/RSS/user/system/context-switch
measurements and backend identity are in `call-performance.json` and
`$RALPH_ARTIFACT_DIR/pa14-call-facts/`. No builds/tests ran during timing.

All **27 compiler outputs and seven native outputs are byte-identical** between
A/B. The nineteen inherited outputs also equal their preceding campaign's
outputs. This is semantic fact reuse at O0, with no generated-code transform.

| Compiler workload | A / B median wall s | A / B RSS KiB | A/A range s | Paired B/A |
| --- | --- | --- | --- | --- |
| calls-1 | 0.436602 / 0.434862 | 83168 / 83282 | 0.436237–0.445906 | 0.9613, 0.9916 |
| calls-4 | 1.807302 / 1.837008 | 305104 / 305172 | 1.774867–1.859101 | 0.9977, 1.0208 |
| memory-float-1 | 0.384674 / 0.382794 | 72626 / 72608 | 0.380503–0.409509 | 1.0035, 1.0114 |
| memory-float-4 | 1.545971 / 1.548345 | 266964 / 266952 | 1.532602–1.546098 | 1.0164, 0.9863 |
| template-semantics-1 | 0.075020 / 0.075467 | 13528 / 13538 | 0.073632–0.075136 | 1.0062, 0.9976 |
| template-semantics-4 | 0.302856 / 0.294215 | 39498 / 39528 | 0.285281–0.535178 | 1.0153, 0.9333 |
| query-instances-1000 | 0.151627 / 0.149416 | 31680 / 31642 | 0.150481–0.155842 | 1.0128, 1.5285 |
| query-instances-4000 | 0.619438 / 0.604114 | 111338 / 111390 | 0.622641–0.626286 | 0.9855, 0.9976 |
| binding-instances-1000 | 0.155961 / 0.153935 | 31164 / 31076 | 0.154365–0.163514 | 0.9856, 0.9767 |
| binding-instances-4000 | 1.499989 / 1.586472 | 109402 / 110138 | 1.081589–1.498861 | 1.0096, 0.9936 |
| fixed-instances-1000 | 0.313585 / 0.310443 | 30898 / 30936 | 0.292520–0.444456 | 0.9965, 0.9795 |
| fixed-unused-1000 | 0.189404 / 0.199269 | 20176 / 20026 | 0.185436–0.228907 | 1.0083, 1.1880 |
| fixed-instances-4000 | 1.330622 / 1.336693 | 108676 / 108652 | 1.234692–1.350465 | 1.0046, 0.9327 |
| fixed-unused-4000 | 0.844164 / 0.889163 | 65162 / 65032 | 0.790150–1.085663 | 0.9459, 0.9857 |
| call-instances-1000 | 0.263286 / 0.295711 | 23518 / 23238 | 0.243092–0.266715 | 0.9027, 1.0150 |
| call-unused-1000 | 0.122254 / 0.148000 | 14204 / 15476 | 0.120140–0.124236 | 1.2392, 1.2086 |
| call-materializations-1000 | 0.297839 / 0.272505 | 31548 / 31304 | 0.285899–0.304749 | 0.9126, 0.8846 |
| call-instances-4000 | 0.997421 / 0.868713 | 79758 / 78260 | 0.951536–1.000416 | 0.7760, 0.7497 |
| call-unused-4000 | 0.425015 / 0.525732 | 41812 / 46688 | 0.399572–0.454737 | 1.2286, 1.1904 |
| call-materializations-4000 | 1.176891 / 1.096979 | 110950 / 109652 | 1.113925–1.198417 | 0.9113, 0.9045 |

| Native workload | A / B median wall s | Payload bytes (both) | Paired B/A |
| --- | --- | --- | --- |
| calls-runtime | 0.979862 / 0.995758 | 206 | 1.0459, 0.9625 |
| memory-runtime | 0.518311 / 0.566688 | 434 | 1.0040, 1.0369 |
| floating-runtime | 0.664347 / 0.669875 | 230 | 1.0251, 0.9849 |
| query-runtime | 0.384623 / 0.379001 | 182 | 0.9776, 0.9746 |
| fixed-runtime | 0.394640 / 0.394729 | 268 | 1.0130, 0.9947 |
| call-runtime | 0.181317 / 0.159162 | 367 | 0.9532, 1.0018 |
| call-materializations-runtime | 0.086783 / 0.087291 | 1192 | 0.9690, 0.9586 |

Native peak RSS is 256 KiB for every sample. The sectionless native text metric
is still payload after entry, including support/data; compiler size is actual
`.text`. Runtime-source compilation is approximately 10–13 ms and 5 MiB RSS;
these startup-dominated samples remain recorded without speedup claims. Native
outputs are identical, so timing differences are measurement variation, not a
runtime optimization benefit.

At 4,000 demanded scalar-call specializations, candidate work falls
**48,000→4,011**, conversion selection **72,012→12,027**, conversion records
**48,012→12,021** and expression work **60,015→12,022**. Five call facts serve
20,000 uses. Median RSS falls 1,498 KiB. Paired times improve 22.4–25.0%, while
the wall median improves 12.9%; A's ABBA observations range .9929–1.3220 s, so
the precise gain is sensitive to that spread. The 1,000-instance timing is
mixed: B's median is 12.3% higher, with pairs .9027/1.0150. A's calibration is
.2431–.2667 s, while later A samples reach .3488/.3514 s; no uniform small-input
speedup is claimed.

Class materialization pairs improve **8.7–11.5% at 1,000** and **8.9–9.6% at
4,000**. At 4,000, conversion selection also falls 72,012→12,027 and stored
conversions 56,007→32,018; RSS falls 1,298 KiB. Source checking adds only two
constructor recipes and one user-conversion recipe. Concrete entity/scope counts
are identical to A: required objects/lifetimes remain per use. Occurrences remain
50N for scalar calls and 53N for class calls, exposing the remaining full-region
projection cost instead of claiming dependent-only instantiation is complete.

Checking 4,000 unused call patterns costs **100.7 ms and 4,876 KiB** in median
latency/RSS. B validates 20,000 fixed calls, 44,000 required candidates and
60,000 additional conversions; A omitted those decisions. All seventeen reduced
illegal unused definitions are accepted by A and rejected by B. The retained
`rejection-recipes-progress.json` records status, source hashes and diagnostics.
This is required definition-time checking (PA14 README, N3485 [temp.res]/8,
[expr.call], [class.access] and [class.access.base]), with valid unused output
still exactly equal. Defaults additionally preserve their own permitted user
conversion, and native controls check effects and destruction across uses.

Ordinary calls-4 now has pairs .9977/1.0208 and a 1.6% higher B median, with
essentially equal RSS. All historical higher calls-4 medians remain preserved;
this comparison does not erase them or establish a general compiler speedup.
Query-1000 retains a .3159 s B outlier (.12 user + .03 system), compared with
other B samples .1467–.1517 s, producing a second pair of 1.5285 despite a lower
B median. Binding-4000 has a 5.8% higher B median, but pairs 1.0096/.9936 and
substantial drift in A itself (1.0816–1.7241 s). Both binary order and all other
outliers remain in the raw evidence; no sample was filtered or used as a gate.

Compiler `.text` is **1,241,990 bytes**, +14,080 (**1.15%**) over A. Entity and
expression records remain **112/36 bytes**. The added fixed support cost buys
required definition-time legality, one selection owner and shared conversion
recipes, with repeatable affected class-call benefits and lower repeated work
and memory. It adds no optional optimizer or student native backend.

**Work/growth budget:** definition-time call work follows the required candidate
and argument edges once per source call; constructor recipes follow their
parameter/default edges. Each demanded use consumes its argument edges and
allocates only required object/lifetime records. Source indices/recipes are
TU-owned, source-proportional and contain no per-node heap owner or global scan.
Ordinary direct-call selection retains its required-candidate/arity bound.
Generated-code growth budget is **zero**, established by exact IR/native equality.
There is no mandated numeric PA14 compiler threshold; inherited self-selected
gates remain diagnostics under stage-scoped acceptance. Measured required
checking costs do not excuse an identified avoidable regression. Full body
projection and finer demand/failure owners remain current-stage work.
