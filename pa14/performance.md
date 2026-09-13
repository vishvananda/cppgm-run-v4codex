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


## Fixed object receivers and default evaluation storage

The frozen source/code points are A `d6891c36`, B `607752d1` and final C
`70775cfd`. B includes fixed receiver/member facts, pointer legality and repeated
default materialization storage. C consumes conversion-call descriptors by
immutable pointer instead of copying them during lifetime classification.
Implementation ownership and C++11 proofs are in [implementation.md](implementation.md).

| Binary | SHA-256 | Compiler .text bytes |
| --- | --- | ---: |
| A | `8a33268218faa96c18683f3a4d9dcddd79602d3c2d04fd26a0f9051cd21be9c9` | 1,241,990 |
| B | `ea874014083cebcbb3033165979ca76f1b7e1bba9ba9e93d50f3696682069eb4` | 1,253,062 |
| C | `dd0f62e7e1fccb82dfaf4ab9a39a61b82fac84b6c793725bab87fe4f2d95c128` | 1,253,062 |

Compiler text grows **11,072 bytes (0.89%)** from entry; the final view refinement
adds no text. Frozen host layout probes retain Entity/Expression/ObjectUse at
**112/36/36 bytes**. TemporaryState grows **28→32 bytes** to preserve the concrete
cleanup address of each emitted evaluation. Probe headers, commands, class-layout
dumps and hashes are retained under `pa14-object-facts/layouts/`.

`object_benchmark.py` was frozen before timing. Its **644 observations** cover
38 compiler inputs and ten native programs: 35 common compiler/nine native
pairs, plus three correct-B-only compiler cases and one B-only native case.
All 27 preceding compiler inputs are preserved by hash. One warmup per binary,
four A/A observations and two ABBA blocks retain all samples; B-only cases have
one warmup and six observations. Flags are `--emit-lowir -O0` (or the inherited
semantics dump mode), release `g++ -std=gnu++11 -Wall -O3` with TEST_RUNNER_ENABLE,
and the supplied backend at `-O0`. Affinity, host version, binary/backend/input/
output hashes, wall/user/system/RSS/context-switch observations are in the JSON.
No builds or correctness tests ran during timed campaigns.

The first focused repeat completed one case before a harness error passed `-O0`
to semantics mode. Its **14 valid observations** and rejection log remain in
`object-repeat-preliminary-performance.json`; neither the samples nor the old
harness were overwritten. The corrected, separately frozen six-case repeat adds
**84 observations**. The final B/C descriptor-view campaign adds **168** on ten
compiler/two native workloads. Total increment **910**, total retained history
**6,104 verified observations**. Preflight/compiler failures are recorded
separately and never counted as timed successful observations.

Every common A/B LowIR/native output is byte-identical. Final C preflight
preserves all 35 common outputs and all three corrected default outputs; its
measured B/C outputs are also byte-identical. Native figures below use the
supplied sectionless ELF payload after its entry (including support/data), not
an ELF `.text` section. All native sample RSS values are 256 KiB. The small
native loops are diagnostic controls, with no generated-program speedup claim.

### Compiler measurements: A to B

Wall columns are medians in seconds, RSS columns median KiB, and ratios are the
two complete ABBA block means B/A. All warmups, A/A ranges and outliers remain
in `object-performance.json`; no observations are filtered from these medians.

| Workload | A wall s | B wall s | RSS KiB A→B | ABBA B/A |
| --- | ---: | ---: | ---: | --- |
| calls-1 | 0.437722 | 0.438296 | 79920→79876 | 1.0037/1.0032 |
| calls-4 | 1.798453 | 1.796330 | 305716→305644 | 1.0144/0.9861 |
| memory-float-1 | 0.370313 | 0.375243 | 69044→69082 | 1.0140/1.2649 |
| memory-float-4 | 1.509020 | 1.513425 | 266646→266694 | 1.0261/1.0007 |
| template-semantics-1 | 0.073672 | 0.073177 | 13164→13248 | 0.9651/1.0072 |
| template-semantics-4 | 0.279177 | 0.281908 | 38900→38840 | 0.6635/1.0131 |
| query-instances-1000 | 0.145606 | 0.149702 | 31634→31700 | 1.0217/1.0099 |
| query-instances-4000 | 0.604345 | 0.597643 | 111144→111168 | 0.9907/0.9990 |
| binding-instances-1000 | 0.150080 | 0.152177 | 31166→31162 | 1.0231/0.9780 |
| binding-instances-4000 | 0.627091 | 0.614844 | 109986→109960 | 0.9889/0.9760 |
| calls-runtime | 0.006599 | 0.006855 | 5154→5162 | 1.0206/1.0372 |
| memory-runtime | 0.006137 | 0.006104 | 5164→5174 | 0.9895/0.9730 |
| floating-runtime | 0.005736 | 0.005638 | 5334→5366 | 0.9750/1.0074 |
| query-runtime | 0.005806 | 0.005821 | 5100→5150 | 0.9807/1.0039 |
| fixed-instances-1000 | 0.155163 | 0.155442 | 30810→30974 | 0.9873/1.0067 |
| fixed-unused-1000 | 0.099554 | 0.098477 | 20096→20082 | 1.0104/0.9924 |
| fixed-instances-4000 | 0.657629 | 0.655064 | 109086→109122 | 1.0073/0.9516 |
| fixed-unused-4000 | 0.390354 | 0.390293 | 65004→65006 | 0.9785/0.9999 |
| fixed-runtime | 0.006162 | 0.006195 | 5120→5134 | 0.9908/1.0055 |
| call-instances-1000 | 0.105111 | 0.105984 | 23284→23274 | 1.0069/1.0050 |
| call-unused-1000 | 0.075323 | 0.074975 | 15452→15586 | 1.0086/0.9954 |
| call-materializations-1000 | 0.134228 | 0.135363 | 30944→31016 | 1.0698/0.9941 |
| call-instances-4000 | 0.425405 | 0.437754 | 77924→78072 | 1.1007/1.0340 |
| call-unused-4000 | 0.286897 | 0.291075 | 46692→46728 | 0.9877/1.0204 |
| call-materializations-4000 | 0.544552 | 0.578582 | 110376→110438 | 1.0577/1.2607 |
| call-runtime | 0.006033 | 0.005975 | 5130→5220 | 0.9972/0.9972 |
| call-materializations-runtime | 0.006416 | 0.006447 | 5136→5178 | 1.0092/0.9993 |
| object-instances-1000 | 0.164210 | 0.155748 | 34204→33722 | 0.9442/0.9255 |
| object-unused-1000 | 0.086786 | 0.094353 | 18232→19174 | 1.0938/1.0771 |
| object-results-1000 | 0.132720 | 0.126239 | 28420→28460 | 0.7551/0.9632 |
| object-instances-4000 | 0.681021 | 0.649173 | 121038→119426 | 0.9742/0.8994 |
| object-unused-4000 | 0.340358 | 0.372159 | 58372→61448 | 1.0972/1.0505 |
| object-results-4000 | 0.534766 | 0.508689 | 100790→97786 | 0.9553/0.9544 |
| object-runtime | 0.006457 | 0.006515 | 5182→5156 | 0.9974/1.0259 |
| object-results-runtime | 0.006478 | 0.006478 | 5152→5212 | 1.0137/1.0065 |
| default-identities-1000 | — | 0.083873 | 20852 | B only |
| default-identities-4000 | — | 0.330051 | 70854 | B only |
| default-identities-runtime | — | 0.006345 | 5180 | B only |

At 4,000 fixed-receiver specializations, object-use records fall **20,003→8**,
expression work **52,011→12,019**, candidate visits **12,000→4,002**, conversion
selection **44,006→20,012**, and stored conversions **48,007→20,014**. Entity/scope
counts are unchanged; two source call facts serve 8,000 concrete calls. Median
wall improves 4.7%, with 1,612 KiB less peak RSS. The repeat pairs .9402/.9640
and median .684089→.652375 s support a receiver-path improvement.

Class-result receivers preserve one required concrete result object per use:
object-use records **16,003→4,007** at 4,000, candidates **12,006→4,008**, with
3,004 KiB less RSS and a 4.9% median improvement. The 1,000-result initial first
pair (.7551) includes an A outlier; the repeat .9723/.9438 is the more modest
confirmation. Whole-region occurrences remain **77N/46N**, so this does not
claim completion of dependent-only body storage.

Unused definitions now incur required legality checking. At 4,000, B stores
20,000 receiver recipes, checks 8,000 calls and 24,000 conversions, and adds
31.8 ms/3,076 KiB. Eighteen of twenty retained object/pointer reducers are
accepted incorrectly by A and rejected by B/C; the other two preserve prior
rejections. The positive unused-specialization control still demands zero
member bodies. Costs follow required source edges, not speculative instantiation.

The default-identity source has two simultaneously live reference-bound defaults.
A compiles it but its executable exits 1; B/C exit 0. The B-only 4,000-input
median is .330051 s/70,854 KiB. Its native payload is 1,520 bytes and median
.039612 s. These are correctness costs, not a speedup against invalid A output.
The expanded reducer additionally checks destructor identities, conditional
arms, conversion functions, constructor defaults and per-array-element cleanup.

### Native measurements: A to B

The compiler cost of each runtime source appears in the preceding table.

| Workload | A runtime s | B runtime s | RSS KiB A→B | ABBA B/A | Payload bytes A/B |
| --- | ---: | ---: | ---: | --- | ---: |
| calls-runtime | 0.480540 | 0.479946 | 256→256 | 1.0010/1.0015 | 206/206 |
| memory-runtime | 0.281237 | 0.280181 | 256→256 | 0.9920/1.0091 | 434/434 |
| floating-runtime | 0.331884 | 0.332632 | 256→256 | 0.9986/1.0058 | 230/230 |
| query-runtime | 0.183566 | 0.183075 | 256→256 | 1.0056/0.9985 | 182/182 |
| fixed-runtime | 0.229347 | 0.229792 | 256→256 | 1.0015/1.0015 | 268/268 |
| call-runtime | 0.101660 | 0.101559 | 256→256 | 0.9883/1.0036 | 367/367 |
| call-materializations-runtime | 0.052658 | 0.052566 | 256→256 | 0.9959/0.9994 | 1192/1192 |
| object-runtime | 0.041655 | 0.041525 | 256→256 | 0.9939/1.0073 | 696/696 |
| object-results-runtime | 0.112728 | 0.112544 | 256→256 | 0.9833/1.0115 | 1208/1208 |
| default-identities-runtime | — | 0.039612 | 256 | B only | 1520 |

### Follow-up, final views and retained spread

The corrected A/B repeat preserves the original binaries and input hashes:

| Workload | A wall s | B wall s | RSS KiB A→B | ABBA B/A |
| --- | ---: | ---: | ---: | --- |
| memory-float-1 | 0.374112 | 0.374354 | 69040→69110 | 0.9956/1.2795 |
| template-semantics-4 | 0.280307 | 0.282125 | 38852→38928 | 1.0001/1.3915 |
| call-instances-4000 | 0.422844 | 0.428346 | 77956→78088 | 1.1018/1.0075 |
| call-materializations-4000 | 0.545820 | 0.560192 | 110334→110430 | 1.0166/1.2109 |
| object-instances-4000 | 0.684089 | 0.652375 | 120996→119460 | 0.9402/0.9640 |
| object-results-1000 | 0.135303 | 0.127880 | 28358→28416 | 0.9723/0.9438 |

The initial ordinary calls-4 pairs are 1.0144/.9861. Initial memory-float-1 has
B at .574671 s (.29 user + .08 system); template-semantics-4 has A at .562585 s
(.22 + .05). Large scalar calls have B at .512905 s (.32 + .09); class calls have
B at .819264 s (.43 + .12), and the 1,000-result case has A at .199204 s (.09 +
.03). Repetition retains further large wall-only samples rather than selecting
a quieter run. The repeated class-call median .545820→.560192 s also retains a
small CPU increase (.53→.54 s median), prompting review of descriptor copying.

Final B/C view comparison (ratios C/B):

| Workload | B wall s | C wall s | RSS KiB B→C | ABBA C/B |
| --- | ---: | ---: | ---: | --- |
| calls-4 | 1.862341 | 1.843074 | 305732→305718 | 0.9924/0.9923 |
| memory-float-4 | 1.525371 | 1.500249 | 266738→266744 | 0.9749/0.9956 |
| call-instances-4000 | 0.428968 | 0.429029 | 78042→78074 | 0.8460/0.9992 |
| call-materializations-4000 | 0.587705 | 0.568483 | 110350→110390 | 0.9785/0.8626 |
| object-instances-4000 | 0.663039 | 0.652061 | 119476→119454 | 0.9933/0.6932 |
| object-results-4000 | 0.519749 | 0.520450 | 97778→97882 | 1.0032/1.0314 |
| object-unused-4000 | 0.378840 | 0.404254 | 61582→61606 | 1.0194/1.1306 |
| default-identities-4000 | 0.336875 | 0.341317 | 70736→70834 | 1.0054/0.9997 |
| call-materializations-runtime | 0.008731 | 0.008772 | 5142→5168 | 0.9936/1.0000 |
| default-identities-runtime | 0.006557 | 0.006549 | 5144→5180 | 1.0504/0.9907 |

Read-only views remove transient descriptor copies and retain exactly the same
semantic work counters. Calls-4 improves in both pairs (.9924/.9923), with no
code growth; class-call pairs improve .9785/.8626, but the second includes B at
.766554 s (.44 + .11). The object-instance second pair includes B at 1.236648 s
(.51 + .13), so its apparent 30.7% gain is not attributed to this change.
Object-results is mixed/slightly slower (1.0032/1.0314). The unused case regresses
1.0194/1.1306, including C CPU sums .41/.41 s against B .36/.37 in the last block;
its semantic work and output are unchanged, and the changed lowering accessor
is not needed by those unused bodies. The cause of that variation is not
established. No general gain is inferred from the view refinement. Median RSS
deltas in this campaign range from -22 to +104 KiB, with no new semantic records.

The two final native payloads remain 1,192 and 1,520 bytes. Class-call native
medians B/C are .052890/.052478 s (pairs .8294/1.0021); default native medians
.040629/.040136 s (pairs .9962/.9890). The class-call A/A range .052582–1.248936 s
includes a 1.248936 s observation with .05 user time. Identical executable bytes
preclude attributing these runtime differences to compiler output. Every sample
and every earlier historical cost remains retained.

### Stage-scoped acceptance and ownership boundary

At O0 this is required semantic validation/fact sharing and lifetime lowering,
with indexed source facts, constant-time receiver projection, cached expression
effects and work proportional to required semantic argument edges and emitted
objects. Receiver recipes occupy source-proportional storage; concrete class
results/default evaluations own their required storage and cleanup actions.
There is no optional optimizer/native-backend pass or unbounded search here.
The sharing benefit on affected receivers justifies the fixed 0.89% compiler
support-code growth and source checking; the descriptor refinement adds no
compiler text or semantic records and avoids copied immutable facts.

The zero generated-code growth target for fixed-fact reuse is verified on all
common-correct inputs. Extending that target to an invalid default-identity
baseline would be an unsupported gate: C++11 requires the extra overlapping
storage and correct cleanup. The failing reducer, raw costs, coverage and
comparison rules are preserved. PA14/O0 has no mandated numeric compiler-latency
threshold; historical self-imposed gates remain diagnostics under the spec.
This does not dismiss measured regressions, waive mandated bounds or complete
the remaining dependent object/body and finer demand/failure owners.

Final checks pass: PA14 314/314, earlier 1621/1621, through 1935/1935, seventeen
native programs, two standalone reducers, release/ASan/UBSan parity on 331 inputs
plus those reducers, 68 explicit sanitizer rejections, two ABI controls and file
audit with three inherited header advisories. The full-stage architecture goal
remains active. Logs, complete command/status manifests, reduced proofs, layouts,
preflights and frozen binaries live in `$RALPH_ARTIFACT_DIR/pa14-object-facts/`.

## Template-owned fixed fields, prototype scopes and method declarators

This continuation starts at `33b791da` and ends with implementation `24ad2c45`
(expanded reducer coverage `ff744067`). It retains the 6,104 preceding
observations and adds **1,162**, for **7,266 verified process observations**.
Artifacts are under `$RALPH_ARTIFACT_DIR/pa14-dependent-objects/`. The compiler
implements all semantic and LowIR work; the supplied PA8 backend executes its
output. No reference, fixture or comparison rule changed.

### Frozen protocol and provenance

All harnesses were committed before timing. Compiler builds use
`g++ -std=gnu++11 -Wall -O3` with `TEST_RUNNER_ENABLE`; measured compilations use
`--emit-lowir -O0`, with `--stats --validate-lowir` in separate preflights. The
native backend uses `-O0`. CPU affinity is fixed to CPU 0. No task-owned build or
test ran concurrently with timing. A/A calibration, two ABBA blocks and one
warmup per binary retain all fourteen observations per common-correct workload.
Tables use the four observations per binary within the ABBA blocks; paired
ratios use block means. Prototype cases rejected by entry have one warmup plus
six B-only observations, with no optimization-profit comparison.

| Raw campaign in `student.tests/pa14/` | Implementation B | Compiler/native workloads | Observations |
| --- | --- | ---: | ---: |
| [dependent-object-performance.json](../student.tests/pa14/dependent-object-performance.json) | `d2ae9665` | 16/4 | 280 |
| [dependent-object-final-performance.json](../student.tests/pa14/dependent-object-final-performance.json) | `b8ad7f6b` | 16/4 | 280 |
| [dependent-object-repeat-performance.json](../student.tests/pa14/dependent-object-repeat-performance.json) | `e9893b21` | 10/2 | 168 |
| [dependent-object-packed-performance.json](../student.tests/pa14/dependent-object-packed-performance.json) | `918f3971` | 10/2 | 168 |
| [prototype-performance.json](../student.tests/pa14/prototype-performance.json) | `e9893b21` | 3/1, B only | 28 |
| [prototype-packed-performance.json](../student.tests/pa14/prototype-packed-performance.json) | `918f3971` | 3/1, B only | 28 |
| [method-parameter-performance.json](../student.tests/pa14/method-parameter-performance.json) | `24ad2c45` | 10/3 | 182 |
| [prototype-methods-performance.json](../student.tests/pa14/prototype-methods-performance.json) | `24ad2c45` | 3/1, B only | 28 |

The JSON records frozen source/output/binary/backend/harness hashes, complete
wall/RSS/user/system/context-switch samples, flags and CPU. Harness-recorded
HEAD may include a later test-only commit; implementation revisions are listed
above. The final method campaign preflights all sixteen prior compiler inputs
and four native programs, including full-sized inherited controls, and retains
their semantic counters. Every common-correct LowIR and executable is
byte-identical. The prototype native output is identical across its three
working binaries. The verifier checks these hashes, counts, order and equations.

Entry compiler `.text` is **1,253,062 bytes**, SHA-256
`dd0f62e7e1fccb82dfaf4ab9a39a61b82fac84b6c793725bab87fe4f2d95c128`.
Intermediate text sizes are 1,263,238 (`d2ae9665`), 1,265,670 (`b8ad7f6b`),
1,265,350 (`e9893b21`) and 1,265,478 (`918f3971`). Final text is
**1,265,670 bytes**, **+12,608 (+1.006%)**, SHA-256
`f254aebfbdf9534bc1f62ecc23051d183a005c32886ae9525bb6e930affe6458`.
Frozen release and ASan/UBSan binaries are preserved for each implementation.

### Owner, data flow and work/storage bounds

The source method context retains pattern class identity, cv and availability of
`this`. Fixed field facts retain the source declaration's type/category without
a fictitious concrete object. A concrete cache keyed by **(source field EntityId,
concrete object TypeId)** supplies mapped field identity and base adjustment;
object TypeId includes class and cv. Class definition contexts are separate from
out-of-line body contexts. Static status follows candidate declarations and
known parameter shapes; unresolved shapes stay dependent. The actual method
suffix is selected through nested pointer-return declarators. Prototype scopes
bind earlier parameter names before later type queries.

Work is bounded by source syntax, actual overload candidates and required
field/object keys. Source contexts take 8 bytes; concrete member-use records
12 bytes; qualifier pairs 2 bytes. Existing Entity/Expression/ObjectUse sizes
remain **112/36/36 bytes**, verified by the frozen layout probe. The prototype
predicate uses **two bits per parsed source node**, with geometrically grown
vector capacity; reusable scratch retains at most the largest parameter-type
traversal. Ordinary free functions skip template member qualifier work. There
is no optional optimizer or speculative search budget.

For N demanded in-class or out-of-line instances, entity/scope counts stay
equal, expression work falls **24N+4→13N+9**, and object-use records fall
**10N→6N**. There are two source method contexts and **2N** concrete member
records. At N=4,000: expressions **96,004→52,009**, objects **40,000→24,000**,
conversion work **68,000→28,010**, conversion records **80,000→28,013**, and
lookup work **124,027→52,029**. Fixed source expression facts rise **4→15** and
their uses **4N→15N**. Whole-region occurrence storage remains **92N/130N**;
this is a remaining architecture defect, not an accepted dependent-only graph.

For N repeated source field statements across four types, expression work falls
**9N+24→2N+20**, object uses **4N+12→12**; one source context and four concrete
member records suffice. At N=4,000 this is **36,024→8,020** expressions and
**16,012→12** object uses. Occurrences remain **32N+176**. For N unused class
definitions, source checking rises **3N→7N** expressions and **3N→12N** fixed
facts; N contexts demand zero concrete member records, object uses, occurrences,
class completions or member bodies. Fourteen entry-accepted invalid unused
bodies now reject. This validates the source-checking cost separately from reuse.

### Compiler results and retained regressions

Initial field implementation (`d2ae9665`), all cases at scale 4,000:

| Workload | A wall s | B wall s | RSS KiB A→B | ABBA B/A |
| --- | ---: | ---: | ---: | --- |
| member instances | 1.040334 | .989108 | 167312→163504 | .9982/.9422 |
| out-of-line members | 1.200624 | 1.170945 | 181324→175510 | .9835/.9674 |
| unused members | .320184 | .347978 | 60910→64386 | 1.0801/1.0930 |
| repeated fields | .247079 | .228475 | 50054→49184 | .9445/.9138 |
| ordinary calls-4 | 1.756444 | 1.772123 | 305620→305648 | 1.0038/1.0100 |
| memory-float-4 | 1.492083 | 1.475310 | 261386→261388 | .9774/.9990 |

Required prototype scope handling initially adds ordinary-call peak RSS despite
equal semantic entity/scope/work counts. Reusing traversal scratch avoids a real
allocation per declaration, but does not resolve the measured RSS increase.
Packed source classification removes the observed memory/float footprint delta;
the larger ordinary-call delta persists:

| Implementation / control | A wall s | B wall s | RSS KiB A→B | ABBA B/A |
| --- | ---: | ---: | ---: | --- |
| prototype / calls-4 | 1.773835 | 1.795414 | 305664→320350 | 1.0078/1.0172 |
| prototype / memory-float-4 | 1.478803 | 1.488735 | 261384→265776 | 1.0091/.9812 |
| scratch / calls-4 | 1.757762 | 1.773591 | 305606→320512 | .9841/1.0546 |
| scratch / memory-float-4 | 1.591714 | 3.734941 | 261384→265738 | 2.0544/1.2115 |
| packed / calls-4 | 1.874016 | 1.971902 | 305642→320942 | 1.1857/.9891 |
| packed / memory-float-4 | 1.516516 | 1.617590 | 261406→261390 | .8520/1.0668 |
| packed / repeated fields-4000 | .256890 | .240684 | 50002→49008 | .9415/.9331 |

The calls-4 logical classification data requires 325,501 bytes for 1,302,002
parsed nodes, far below the **15,300 KiB** packed campaign RSS delta. The
remaining high-water/allocator cause is **not established**; this delta cannot
be attributed wholesale to necessary semantic storage. It remains an explicit
performance investigation. Neither scratch reuse nor packed storage supports a
general compiler-speed claim.

All outliers remain. In the scratch memory case, B observations include 4.574617 s
with 1.16+.34 CPU seconds, and 7.683436 s with 6.24+1.35 CPU seconds; A also
has 7.240080 s with 5.94+1.27 CPU seconds. These include CPU variation as well as
wall-only stalls, so their cause is not assigned to one external factor. Scratch
repeated-field pairs are .9344/1.1884, with a .356712 s B observation (.18+.04
CPU). Packed member-instance A/A spans 1.022387–1.772402 s; its first ABBA gain
of .7163 includes A at 1.767318 s (.82+.19 CPU). The packed out-of-line median
1.488713→1.193394 s likewise does not establish that entire apparent gain.

Final implementation (`24ad2c45`), new method cases and matched controls:

| Workload | A wall s | B wall s | RSS KiB A→B | ABBA B/A |
| --- | ---: | ---: | ---: | --- |
| method instances-1000 | .246363 | .248496 | 40470→40756 | 1.0038/1.0089 |
| method instances-4000 | 1.023855 | 1.018445 | 145340→145976 | .9953/.9969 |
| member instances-1000 | .236807 | .231914 | 44190→43858 | .9807/.9801 |
| out-of-line members-1000 | .287390 | .279693 | 48308→49402 | .6986/.9868 |
| unused members-1000 | .082713 | .088259 | 18742→19762 | 1.0661/1.0686 |
| repeated fields-1000 | .061380 | .058063 | 16448→16522 | .9398/.9431 |
| ordinary calls-1 | .435626 | .436929 | 80550→83458 | 1.0049/1.0074 |

Method-4000 A/A is 1.016996–1.028738 s; ABBA observations span
1.020607–1.028483 (A), 1.013812–1.030077 (B). The small median improvement is
within that spread. Repeated-1000 A/A is .061932–.062276 s and ABBA spans
.061270–.062586 (A), .057784–.058263 (B). Together with the initial and packed
4,000 cases, this supports a repeatable benefit on repeated fixed field work.
Final out-of-line A includes .512430 s; calls-1 A/A spans .426896–.514450 s.
Their outlier-sensitive ratios do not support a broad speedup claim.

### Native execution and necessary prototype costs

Each native loop has a volatile 3,000,000-iteration bound and checks its result.
The method loop calls both returned function pointers and verifies field updates.
The supplied sectionless ELF's payload after entry is the retained text metric.
Final common-correct generated bytes are unchanged:

| Native workload | A wall s | B wall s | RSS KiB A/B | ABBA B/A | Payload bytes A/B |
| --- | ---: | ---: | ---: | --- | ---: |
| method runtime | .040438 | .040476 | 256/256 | .9961/.9988 | 444/444 |
| member runtime | .036331 | .036395 | 256/256 | 1.0037/1.0003 | 313/313 |
| class call materializations | .051892 | .052074 | 256/256 | 1.0055/.9990 | 1192/1192 |

Method A/A is .040464–.040750 s; the member A/A range is .036286–.036464 s;
class-call A/A is .051887–.052319 s. Identical executable bytes mean runtime
variation is not a generated-code benefit. The complete final preflight also
preserves the default-identity executable's 1,520-byte payload.

The entry rejects the valid prototype-query corpus, so final B-only baselines
are: 1,000 instances **.276395 s / 49,312 KiB**, 4,000 **1.149982 s /
182,718 KiB**. Six-observation wall ranges are .272810–.376954 and
1.143088–1.197902 s. The .376954 observation retains .21+.05 CPU seconds.
Its native loop compiles in **.005984 s / 5,156 KiB** and runs in **.023805 s /
256 KiB**, range .023751–.023915 s, with **266 bytes** of payload. Small runtime
sources are compiler cost baselines, not startup-dominated speed claims. The
valid prototype reducer's entry rejection and corrected native result are frozen.

### Acceptance, validation and concrete boundary

Source fact reuse has a **zero generated-code growth budget** on common-correct
inputs; all campaigns meet it by exact hashes. Its repeatable affected-workload
benefit accompanies a fixed 12,608-byte compiler text increase and the bounded
source/context/key storage above. No native optimization was added or claimed.
Required unused-body checking and previously rejected prototype behavior have
separate correctness proofs and measured costs. O0 has no mandated numeric
compiler wall/RSS gate. Inventing a zero-overhead threshold would contradict
stage-scoped acceptance, but reclassification does not excuse the unexplained
ordinary-call RSS regression or complete the remaining architecture work.

Final implementation passes `make test-pa14` **314/314**, default
`make test-report-through-pa13` **1621/1621**, and
`make test-report-through-pa14 TEST_REPORT_ASSIGNMENT_JOBS=1 TEST_REPORT_SUBTEST_JOBS=1`
**1935/1935**. The preceding default through attempt is retained at **1923/1935**:
twelve unchanged PA1/PA2 lexer cases timed out while workers were observed in
`rq_qos_wait`, with host I/O full-pressure avg60 around 60–62%. The retry changes
only concurrency; coverage, fixtures, comparisons and timeouts are unchanged.
This observation supports an I/O explanation for that test episode, not every
timing outlier above. File audit passes with three inherited header advisories.

Eighteen native programs, **332** course/personal release/ASan/UBSan parity
inputs, **82** explicit rejection controls, two ABI controls and four standalone
reducer native/parity checks pass. The expanded enum/method reducer has frozen
entry/packed/final statuses **0/1/0**, with correct entry/final native execution;
the matching invalid static-use reducer is one of fourteen entry-accepted errors
now rejected. `checks-final.json`, `progress-final.json`, `reducer-parity-final.json`
and the `proofs/` directory preserve commands, statuses, source and output hashes.
Earlier intermediate proofs and sources remain available.

The fixed field/prototype/method group is complete. Substituted receiver types,
dependent declaration/value/storage facts and dependent signatures require
context-keyed typed graph propagation; the source-only fixed index cannot own
them safely. Whole-region projection and separate demand/failure states remain
current-stage owners, alongside the RSS investigation. The full-stage goal
remains active; [plan.md](plan.md) records the boundary and next work.

## Declaration types, substitution frames and query contexts

The continuation entry is `5b1afe54`; final implementation is `5ce59182`.
`declaration_type_benchmark.py` was frozen at `bbb9d729` before either campaign.
The entry SHA is `f254aebfbdf9534bc1f62ecc23051d183a005c32886ae9525bb6e930affe6458`;
final release SHA is `ed043579d0e2c321623061f3204ae19cee11e4d0c472cf5130737ce6b31e98e0`.
Both use g++ C++11/O3 and the same test-runner setting. Sixteen frozen inputs
cover declaration and renamed-member instances, unused definitions, previous
member work, ordinary calls and memory/floating-point work. Five checked native
loops use the supplied PA8 backend. All input/output/binary/harness/backend
hashes, flags, CPU affinity, wall/RSS/CPU/context-switch observations, warmups,
A/A calibration and two ABBA blocks are retained in
`declaration-type-preliminary-performance.json` and
`declaration-type-performance.json`. All inputs and outputs were frozen before
timing; no builds or test campaigns ran concurrently with measurements.

Each campaign adds 294 observations. Together with the 7,266 inherited rows,
**7,854 observations** are retained. The preliminary compiler temporarily
confused body-parameter query ordinals with prototype-only identities and made
fixed queries dependent merely because of their access scope. Reduced invalid
parameter operations and inherited `unknown_call(1)` checking exposed these
regressions; `5ce59182` corrects them. The full final campaign repeats the same
sources, preserving every preliminary sample. No failing required fixture or
comparison rule was changed.

Final compiler medians use the four A and four B observations in the ABBA blocks:

| Workload | A/B wall seconds | A/B peak RSS KiB | ABBA B/A blocks |
| --- | --- | --- | --- |
| Declaration instances 1,000 | .665247 / .649972 | 94,096 / 94,124 | .9682 / 1.0028 |
| Declaration instances 4,000 | 2.945107 / 2.856347 | 362,070 / 362,668 | .9570 / .9874 |
| Renamed members 1,000 | .368930 / .360169 | 61,878 / 63,922 | .9120 / .9814 |
| Renamed members 4,000 | 1.552243 / 1.531868 | 234,724 / 239,446 | .9855 / .9892 |
| Unused definitions 1,000 | .431308 / .457728 | 75,472 / 77,374 | 1.1492 / 1.0543 |
| Unused definitions 4,000 | 1.753455 / 1.865442 | 285,574 / 290,204 | 1.0649 / 1.0647 |
| Prior member instances 1,000 | .233910 / .232585 | 43,772 / 43,998 | .9987 / 1.0012 |
| Prior repeated fields 1,000 | .058848 / .058692 | 16,680 / 16,096 | .9990 / .9924 |
| Ordinary calls 4 | 1.805331 / 1.759348 | 305,864 / 305,796 | .9762 / .9699 |
| Memory/floating source 1 | .371511 / .373368 | 69,706 / 69,818 | .9910 / 1.0105 |

The 4,000-instance declaration workload improves in both final blocks and both
preliminary blocks (.9536/.9737). Renamed-member improvements are smaller but
repeat in both campaigns. Smaller workloads have noise and outliers: final
renamed-1,000 A spans .357482–.432005 s; unused-1,000 B spans .454457–.535821 s.
All samples remain included. Final unused-4,000 A spans 1.748919–1.763862 s and B
1.862148–1.881573 s, showing a repeatable approximately 6.5% source-validation
cost. Five frozen unused function-pointer rejection proofs are entry-accepted
and final-rejected under [expr.call], [expr.ass] and [expr.add]; three existing
access/parameter rejection controls remain rejected. Two complete personal
programs also pass both compilers and native execution. Proof hashes and
commands are retained by `declaration_type_evidence.py`.

| Checked executable | A/B runtime seconds | Identical payload bytes |
| --- | --- | --- |
| Declaration/type loop | .040444 / .040846 | 428 |
| Inherited member loop | .036932 / .036701 | 313 |
| Calls | .478622 / .477609 | 206 |
| Memory | .279846 / .279467 | 434 |
| Floating point | .330091 / .330698 | 230 |

All compiler outputs and native executable hashes are identical across A/B and
both campaigns. Native differences are measurement variation, not optimization
profit. The backend emits sectionless ELF; the established payload-after-entry
metric is used consistently. Compiler text grows 1,265,670→1,274,054 bytes,
**+8,384 (+0.662%)**. Hot Entity/Expression/ObjectUse remain 112/36/36 bytes;
the new substitution frame is 20 bytes, with existing object-context/member-use
records 8/12. `declaration-type-layout.json` freezes the probe and all transitive
headers. Ordinary calls-4 RSS is essentially unchanged against this entry;
it does not explain or discharge the inherited approximately +15 MiB regression
against `33b791da`.

The verifier checks exact work/storage equations at N=1,000 and 4,000. Repeated
declarations have 30 source type-specifier facts, N frames, 6N successful
substituted type records, 57N type uses/hits and 6N+3 substitution work. Renamed
members have ten source facts, 3N frames, 10N records/work, 14N type uses and 9N
hits. Unused definitions have 30N source facts, 3N normalization work, zero frames,
zero concrete type uses and zero occurrences. Frame equality includes the
source parameter head and parent frame; ordinal lookup also checks source
parameter identity. Flat indexes hold complete keys; storage grows with source
facts, frames and demanded dependent type/query/binding facts, never unrelated
declaration products. Occurrences remain 457N/174N in the new instance corpora:
dependent-only body construction remains unfinished current-stage work.

At PA14/O0, this is required semantic ownership with shared substitution, not
an optional optimization pass. The explicit common-correct generated-code
growth budget remains **zero**, verified by hashes. Distinct-key work/storage
bounds above and the larger-scale repeated compile-time benefit justify retaining
the shared caches. Source validation and its measured cost are required by the
handout; five invalid-body proofs substantiate the added coverage. No mandated
numeric compiler latency/RSS/text ceiling exists here. Unsupported inherited
self-selected diagnostic thresholds remain observations rather than exit gates;
all measurements, correctness, required limits and coverage are preserved.

## Member body/default demand and cached source regions

Continuation entry is `ca42e706` (release SHA `ed043579d0e2c321623061f3204ae19cee11e4d0c472cf5130737ce6b31e98e0`).
The final implementation is `e1afac7c`, frozen as `cppgm++-cached-regions-release`
(SHA `4809031b0b8e2cd39a6751deb0a2611f24768b956b012e9ffb05136597514a57`).
All artifacts are under `$RALPH_ARTIFACT_DIR/pa14-regions/`. Three complete
`region_benchmark.py` campaigns preserve 1,008 observations in
`region-preliminary-performance.json`, `region-standard-performance.json` and
`region-performance.json`. A further 70 observations in
`region-cache-performance.json` isolate the source index from the already-correct
region/default implementation. Together with inherited evidence, all **8,932
observations** verify. Binaries, harnesses, inputs and correct outputs were frozen
before each campaign; timings ran without concurrent builds/tests. Every common
case has warmups, four A/A observations and two ABBA blocks. Entry-invalid cases
retain the rejection and six B-only measurements rather than comparing invalid
implementations. All observations and outliers remain in the JSON.

Final ABBA compiler medians and paired block ratios:

| Workload | A/B wall seconds | A/B peak RSS KiB | ABBA B/A |
| --- | --- | --- | --- |
| Unused large body, N=1,000 W=8 | .211428 / .160731 | 43,440 / 31,848 | .7516 / .7603 |
| Unused large body, N=1,000 W=128 | .917265 / .164720 | 198,886 / 31,832 | .1807 / .1790 |
| Unused large body, N=4,000 W=128 | 4.114022 / .672118 | 777,150 / 111,708 | .1635 / .1640 |
| Used large body, N=1,000 W=8 | .294640 / .281533 | 55,658 / 52,300 | 1.0655 / .9548 |
| Used large body, N=1,000 W=128 | 2.538970 / 2.393640 | 373,010 / 369,086 | .9459 / .9704 |
| Unused default, N=1,000 | .065808 / .054553 | 16,686 / 14,862 | .8235 / .8266 |
| Unused default, N=4,000 | .262532 / .208423 | 51,774 / 44,272 | .7931 / .7922 |
| Repeated default, N=1,000 | .031143 / .031350 | 9,178 / 9,456 | .9957 / 1.0094 |
| Repeated default, N=4,000 | .109860 / .109962 | 22,456 / 22,336 | 1.0103 / .9961 |
| Declaration instances 1,000 | .639065 / .618153 | 94,160 / 94,170 | .9709 / .9762 |
| Renamed members 1,000 | .353391 / .346348 | 63,974 / 64,336 | .9804 / 1.0429 |
| Repeated fields 1,000 | .058910 / .059078 | 16,112 / 17,150 | 1.0017 / .9978 |
| Ordinary calls 4 | 1.791887 / 1.768738 | 305,850 / 305,882 | 1.0055 / .9673 |
| Memory/floating source 1 | .370955 / .372537 | 69,690 / 69,784 | .9999 / .7809 |

The large unused-body benefit repeats across all three campaigns. Final N=4,000
A spans 4.068877–4.118932 s and B .670610–.672517 s. The earlier standard-reviewed
implementation slowed the fully used W=128 body: 2.457764→2.533875 s, with ratios
1.0310/1.0422. That result motivated indexing source topology and attributes once.
Final W=128 spans 2.432226–2.606284 s (A) and 2.340152–2.562271 s (B). Smaller
cases retain outliers: used W=8 B reaches .346711 s; renamed-member B .391436 s;
memory/floating A .587595 s. No claim is made from those noisy block differences.
New valid dependent-default inputs have B-only medians .054638 s / 14,592 KiB
(N=1,000) and .210660 s / 43,574 KiB (N=4,000); the latter spans .208794–.252064 s.

The isolated cache comparison uses the standard-reviewed release as A and final
cached release as B, with the same frozen sources and exact output preflight:

| Workload | A/B wall seconds | A/B RSS KiB | ABBA B/A |
| --- | --- | --- | --- |
| Unused large body 4,000 | .703095 / .678476 | 111,658 / 111,108 | .9627 / .9661 |
| Used large body 1,000 | 2.489001 / 2.432524 | 368,874 / 375,588 | .9545 / .9706 |
| Unused default 4,000 | .213462 / .209793 | 44,400 / 44,384 | .9753 / 1.7785 |

For the two body workloads the cache improves both paired blocks. Used-body A/A
spans 2.442122–2.591564 s, ABBA A 2.470659–2.658479 s and B 2.392073–2.470063 s.
Unused-body A/A spans .699134–.704804 s, ABBA A .701182–.707485 s and B
.674190–.683482 s. The unused-default B outlier .547672 s is preserved; its cache
benefit is not established. The isolated used-body peak RSS rises 6,714 KiB
(1.82%), despite unchanged semantic work/record counts and only two source index
records, 1,469 node IDs and five roots. The final full campaign instead has lower
used-body RSS against entry. These observations do not establish the cause of
the isolated RSS increase; no allocator explanation or memory benefit is claimed
for that case. The inherited calls-4 roughly +15 MiB delta against `33b791da`
also remains unexplained. Intermediate campaigns have both A and B around
320,000 KiB, while preliminary/final campaigns have both around 305,000 KiB;
all samples are preserved and the movement is not attributed to this change.

| Checked executable | A/B runtime seconds | Payload bytes |
| --- | --- | --- |
| Region/member loop | .059201 / .059347 | 206, identical |
| Dependent-default/side-effect loop | B-only 2.697394 | 344 |
| Calls | .476324 / .476209 | 206, identical |
| Memory | .278624 / .279180 | 434, identical |
| Floating point | .330484 / .330062 | 230, identical |

The B-only loop spans 2.687025–2.732336 s and checks 12 million live iterations,
default side effects and a checksum. The isolated common loop is .059214/.059399 s
with ratios 1.0074/.9968 and the same 206-byte executable payload. The sectionless
backend uses the established payload-after-entry size metric. All common-correct
LowIR and executable hashes match across every region campaign, including the
cache comparison; B-only outputs also match across the three full campaigns.
No runtime optimization is claimed. Compiler text grows 1,274,054→1,282,758 bytes,
**+8,704 (+0.683%)**; the cache itself adds 2,240 bytes over the standard-reviewed
implementation. Entity/Expression/ObjectUse stay 112/36/36 bytes, substitution
frames 20 bytes and occurrences eight bytes. The per-TU Ast grows 304→352→504
bytes as demand state and source indexes are added. Both historical and final
layout probes and transitive header snapshots are preserved. Requiring unchanged
historical header hashes would forbid the intended Ast ownership extension; the
verifier instead retains frozen historical layouts and checks current live
headers against the new probe. This changes no mandated layout or coverage rule.

Work/storage budgets follow source identities and demanded regions. For the body
corpus, eager occurrences were `(88+11W)N`; final occurrences are `69N` when only
the small body is used and `(65+11W)N` when the large body is used. Both retain
`5N` deferred roots with `N` demands. The source index has two records, five roots
and 65 node IDs for the small-body case, independent of W and N; demanding the
large body indexes `61+11W` nodes once. Unused defaults reduce 34N occurrences
to 21N, with 2N deferred roots and no default analysis. Their shared index has
one region, nineteen nodes and two roots. Repeated defaults retain 28 occurrences,
three indexed regions, two demands and one semantic default computation,
independent of the number of calls. Default runtime evaluation still occurs on
every omitted argument. Flat storage and reusable traversal scratch grow with
indexed source nodes/edges and complete demanded keys; unrequested body contents
are neither walked nor indexed by specialization.

At PA14/O0, the common-correct generated-code growth budget remains **zero**.
There is no mandated numeric compiler latency/RSS/text ceiling. Required default
and region semantics, repeatable body compilation savings, and bounded source
index storage justify the compiler work and text growth; the isolated RSS cost
and noisy cases remain disclosed. Unsupported historical diagnostic gates remain
observations, not extra exit requirements. Correctness, mandated limits, coverage
and all raw measurements are preserved. Dependent-only construction *within*
used bodies and broader typed demand/failure dependencies remain current-stage
work; this region cache does not complete those graph owners.


## Typed body values and dependent bounds (`66fe52c1`)

The frozen `value-query-performance.json` adds **462 observations** to the
8,932 inherited observations. The complete corpus was preflighted before timing:
all 21 prior workloads plus three source/repetition scaling cases, two newly
accepted array cases, and two checked native loops. A is `c78e8d3b` (SHA-256
`4809031b0b8e2cd39a6751deb0a2611f24768b956b012e9ffb05136597514a57`);
B is `66fe52c1` (`2f52c047020bc6b4c959a804863b16ddaf41428bf04198c7685ae34993b05d4b`).
Both use the recorded release flags and `--emit-lowir -O0`; PA8's supplied native
backend runs the compiler's LowIR. Binaries, harness, sources and outputs were
frozen before the isolated campaign. No builds, tests or probes ran during timing.
One warmup per compiler precedes four A/A observations and two ABBA blocks.
Entry-rejected bound inputs receive six B-only observations and no speedup claim.
All samples, CPU affinity, flags, hashes, RSS and context switches remain in JSON.

Compiler values below are medians of ABBA observations, or all six B-only samples.
Very small executable-source compilations expose startup cost and are controls,
not evidence for compiler profitability.

| Compiler workload | A/B wall seconds | A/B peak RSS KiB | Paired B/A |
| --- | --- | --- | --- |
| body-run-1000-8 | 0.161339 / 0.163402 | 31,756 / 31,614 | 1.0044 / 1.2056 |
| body-run-1000-128 | 0.164792 / 0.166778 | 31,868 / 31,870 | 1.0174 / 1.0084 |
| body-run-4000-128 | 0.681140 / 0.687916 | 111,882 / 111,528 | 0.9955 / 1.0241 |
| body-large-1000-8 | 0.286593 / 0.278092 | 52,330 / 53,600 | 0.9806 / 0.6082 |
| body-large-1000-128 | 2.378760 / 2.248171 | 369,076 / 357,102 | 1.0573 / 0.9474 |
| default-unused-1000 | 0.053503 / 0.053246 | 14,790 / 14,818 | 0.9722 / 0.9922 |
| default-repeated-1000 | 0.031615 / 0.031760 | 9,510 / 9,480 | 0.9934 / 1.0173 |
| default-dependent-1000 | 0.054870 / 0.055272 | 14,638 / 14,666 | 1.0133 / 0.9982 |
| default-unused-4000 | 0.210622 / 0.209819 | 44,322 / 44,440 | 0.9901 / 0.9893 |
| default-repeated-4000 | 0.109066 / 0.109270 | 22,370 / 22,324 | 0.9954 / 1.0035 |
| default-dependent-4000 | 0.213492 / 0.214095 | 43,568 / 43,520 | 1.0115 / 1.0008 |
| region-runtime | 0.007232 / 0.007512 | 5,536 / 5,426 | 1.0155 / 1.0403 |
| dependent-default-runtime | 0.005951 / 0.005990 | 5,214 / 5,198 | 1.0038 / 1.0040 |
| declaration-instances-1000 | 0.614259 / 0.612886 | 94,260 / 91,086 | 0.9929 / 1.0002 |
| declaration-outside-1000 | 0.355727 / 0.352581 | 64,272 / 64,190 | 1.0048 / 0.9081 |
| member-repeated-1000 | 0.059492 / 0.059267 | 17,092 / 17,110 | 0.9917 / 0.9954 |
| calls-4 | 1.778973 / 1.784324 | 305,872 / 306,348 | 0.9940 / 1.0336 |
| memory-float-1 | 0.377432 / 0.373781 | 69,676 / 69,660 | 0.9849 / 0.9968 |
| calls-runtime | 0.005917 / 0.005966 | 5,320 / 5,300 | 1.0105 / 1.0160 |
| memory-runtime | 0.005771 / 0.005842 | 5,348 / 5,262 | 0.9922 / 1.0251 |
| floating-runtime | 0.005627 / 0.005721 | 5,308 / 5,326 | 1.0054 / 1.0120 |
| value-offset-1000-8 | 0.203966 / 0.195041 | 42,590 / 41,888 | 0.9597 / 0.9562 |
| value-offset-1000-128 | 2.574320 / 2.446097 | 395,534 / 372,962 | 0.9453 / 0.9402 |
| value-offset-4000-8 | 0.859922 / 0.840842 | 155,094 / 152,206 | 0.8584 / 1.0034 |
| value-bound-1000 | 0.099200 | 23,722 | B only |
| value-bound-4000 | 0.392247 | 79,218 | B only |
| value-runtime | 0.005673 / 0.005726 | 5,314 / 5,204 | 0.9955 / 1.0146 |
| bound-runtime | 0.005921 | 5,148 | B only |

Both N=1,000 offset workloads improve in both paired blocks. W=128 has A/A
2.554534–2.630221 s, ABBA A 2.556135–2.647615 s and B 2.402514–2.464936 s.
Its median falls 2.574320→2.446097 s (4.98%) and peak RSS 395,534→372,962 KiB
(22,572 KiB, 5.71%). The used-body median also falls, but its B outlier 2.790742 s
makes the first pair slower; its A/A range is 2.425298–2.885152 s. That case does
not establish a repeatable timing gain. The N=4,000 offset result likewise has
one nearly unchanged pair and an A outlier 1.092089 s; it supports scaling/work
bounds, not a separate repeatable timing claim. Other costs remain visible:
unused W=128 body medians rise about 1%, ordinary calls rise 0.3%, and the small
used body adds 1,270 KiB RSS. The small-unused-body B outlier .227352 s and calls
B outlier 1.860144 s remain included. No claim relies on removing those samples.

| Checked executable | A/B runtime seconds | Executable payload bytes | Paired B/A |
| --- | --- | --- | --- |
| region-runtime | 0.059342 / 0.060518 | 206 / 206 | 1.0013 / 1.0314 |
| dependent-default-runtime | 2.746361 / 2.700448 | 344 / 344 | 0.9842 / 0.9760 |
| calls-runtime | 0.478612 / 0.477344 | 206 / 206 | 0.9999 / 0.9953 |
| memory-runtime | 0.283686 / 0.279636 | 434 / 434 | 0.9918 / 0.9846 |
| floating-runtime | 0.331501 / 0.331878 | 230 / 230 | 1.0005 / 0.9996 |
| value-runtime | 0.060076 / 0.060115 | 184 / 184 | 0.9984 / 1.0035 |
| bound-runtime | 0.059404 | 194 | B only |

The new loops check twelve million live iterations with volatile runtime counts,
checksums and array updates. All six common-correct executable hashes and all 25
common-correct LowIR hashes match exactly. Generated-code growth is **zero**;
there is no runtime optimization claim. The B-only array loop checks behavior
and establishes its baseline. Payload measurement uses the supplied sectionless
ELF's bytes after its entry point, as in prior campaigns.

The value graph gives sizeof/alignment a fixed result type while retaining
their value dependence. For the used-body corpus, expression work is `4N+2W+3` and
conversion work `3N+2W+2`, replacing the repeated operand checking. The offset
corpus has expression work `3N+4W+1` and conversion work `3N+4W`. Both evaluate
N canonical layout queries with NW uses. Conversion variants are bounded by
four two-bit policies per source operator: these inputs use W variants and 3W
or 2W retained conversion records, independent of N. Array-bound value work is
`3N+1`. Types/queries, frame substitutions, cached constants and conversion
slices are translation-unit owned and indexed by complete typed keys. Expected
nonconstants share the invalid-constant sentinel; active/failed query states do
not restart the query. Full constexpr interpretation is outside PA14.

The first implementation changed constant-widening instructions around layout
queries. The initial LowIR outputs and both native results remain in
`pa14-value-facts/`; they are correctness observations, not a performance
comparison. Shared conversion policy variants correct that difference, with
entry/final byte parity on `value-conversion.t` and `body-values.cpp`. The broader
bound controls cover both conditional arms' type obligations, short-circuit
value evaluation, casts, static constants/enumerators, access, renamed heads,
deduction, multidimensional arrays and parameter adjustment. No course fixture,
reference or comparison changed. Six new invalid-unused-body proofs follow
N3485 [expr.sizeof]/1,6 and fixed operand rules; `value-query-proofs.json` retains
all 22 rejection results and three positive compiler/native proofs.

Compiler text grows 1,282,758→1,294,278 bytes, **11,520 (0.898%)**. The current
transitive-header probe preserves Entity/Expression/ObjectUse at 112/36/36 bytes,
frames at 20, occurrences at eight and Ast at 504; TypeQuery/TypeQueryFact are
48/48 and the sparse queried-value record is eight bytes. Historical layout
snapshots remain checked unchanged. Their old model-header hashes are not an
extra current-layout gate: adding DependentArray and query kinds changes enum
source without changing hot layouts. The new live-header probe establishes
current sizes while retaining every historical measurement.

At PA14/O0 no numeric compiler latency/RSS/text ceiling is mandated. Required
source-time checking and dependent array semantics, complete-key work bounds,
and repeatable offset compilation savings justify the compiler work and text
cost. The explicit budgets are source/key-proportional query storage, at most
four conversion variants per source operation, and zero common-correct generated
growth. No optional optimizer was added. No historical measurement or mandated
limit was removed. The earlier isolated source-cache +6,714 KiB and calls-4
roughly +15 MiB RSS deltas remain unexplained; this campaign does not establish
their cause. Large used regions still allocate occurrences and per-occurrence
fact slots; typed local identity/lifetime overlays and broad demand/failure
edges remain current-stage architecture work.


## Immutable expression properties, concrete uses and local views

The continuation compares frozen entry `5a795af4` with property/use ownership
`c4e4e6f4`, then with local expression views `5ae726e0`. The two complete campaigns
retain **1,120 new observations**, bringing the verified total to **10,514**.
[Initial ownership data](../student.tests/pa14/expression-owner-performance.json)
and [final local-view data](../student.tests/pa14/expression-view-performance.json)
retain all 32 compiler inputs and eight checked executables, binary/source/output
hashes, flags, warmups, A/A calibration, two ABBA blocks, RSS, wall time, context
switches and spreads. Both use the same frozen harness and inputs. Builds, tests,
probes and evidence verification were stopped during each timing campaign.
Medians below use the four samples per binary in the ABBA blocks; calibration
and outliers remain in the raw data and paired means.

The initial store reduced peak memory but regressed the repeated layout-offset
case in both blocks. It was not accepted as an unresolved performance gate:

| Compiler workload | A/B median seconds | A/B peak RSS KiB | Paired B/A |
| --- | --- | --- | --- |
| body-large-1000-128 | 3.501168 / 3.540093 | 357,994 / 321,408 | 0.9677 / 1.0288 |
| value-offset-1000-128 | 2.418760 / 2.551136 | 373,214 / 337,862 | 1.0299 / 1.0630 |
| input-uses-1000-128 | 2.017796 / 1.936081 | 322,480 / 271,000 | 0.9679 / 0.9530 |

The retained diagnostic CPU-clock profile attributed 45.90% of samples to
`IdIndex::get` and 8.98% to `Ast::project_view`. It is diagnostic evidence, not a
replacement timing campaign. Fixed-expression reuse and ordinary expression,
unary, binary and call lowering repeatedly projected the same source/context
edges for separate field reads. Each now retains one stack view during its visit.
Source topology and that occurrence's context are immutable; recursive arena
growth cannot invalidate the copied view. This adds no persistent cache, semantic
rechecking, output transform or invalidation policy. Final paired results follow:

| Compiler workload | A/B median seconds | A/B peak RSS KiB | Paired B/A |
| --- | --- | --- | --- |
| body-run-1000-8 | 0.161517 / 0.153301 | 31,764 / 28,172 | 0.9584 / 0.9387 |
| body-run-1000-128 | 0.164276 / 0.154318 | 32,052 / 28,714 | 0.9389 / 0.9436 |
| body-run-4000-128 | 0.673710 / 0.636663 | 109,112 / 98,838 | 0.9419 / 0.9495 |
| body-large-1000-8 | 0.269335 / 0.248170 | 51,478 / 44,660 | 0.5862 / 0.9249 |
| body-large-1000-128 | 2.198947 / 1.992706 | 356,254 / 321,168 | 0.8889 / 0.9062 |
| default-unused-1000 | 0.053995 / 0.051616 | 14,758 / 13,284 | 0.9734 / 0.2337 |
| default-repeated-1000 | 0.031186 / 0.029899 | 9,328 / 8,930 | 0.9618 / 0.9557 |
| default-dependent-1000 | 0.054848 / 0.052387 | 14,714 / 13,328 | 0.9538 / 0.9481 |
| default-unused-4000 | 0.207285 / 0.198888 | 44,304 / 38,326 | 0.9624 / 0.9544 |
| default-repeated-4000 | 0.109293 / 0.104097 | 22,392 / 20,464 | 0.9552 / 0.9514 |
| default-dependent-4000 | 0.207901 / 0.196971 | 43,648 / 37,978 | 0.9444 / 0.9563 |
| region-runtime | 0.007149 / 0.007066 | 5,588 / 5,552 | 0.9875 / 0.9950 |
| dependent-default-runtime | 0.005850 / 0.005965 | 5,254 / 5,244 | 0.9932 / 4.4638 |
| declaration-instances-1000 | 0.607083 / 0.568326 | 91,082 / 79,706 | 0.9497 / 0.9348 |
| declaration-outside-1000 | 0.342989 / 0.333848 | 64,226 / 56,554 | 0.9778 / 0.9826 |
| member-repeated-1000 | 0.058643 / 0.055488 | 17,304 / 15,074 | 0.9490 / 0.9401 |
| calls-4 | 1.798629 / 1.707297 | 320,468 / 271,046 | 0.9366 / 0.9660 |
| memory-float-1 | 0.370979 / 0.358584 | 69,346 / 62,992 | 0.7318 / 0.9689 |
| calls-runtime | 0.005367 / 0.005438 | 5,172 / 5,212 | 1.0192 / 1.0063 |
| memory-runtime | 0.005736 / 0.005748 | 5,136 / 5,208 | 1.0031 / 1.0111 |
| floating-runtime | 0.005540 / 0.005608 | 5,290 / 5,420 | 1.0171 / 0.9880 |
| value-offset-1000-8 | 0.196707 / 0.175125 | 41,876 / 35,414 | 0.8857 / 0.9004 |
| value-offset-1000-128 | 2.428378 / 2.147875 | 373,240 / 337,888 | 0.8939 / 0.8464 |
| value-offset-4000-8 | 0.825672 / 0.744993 | 152,088 / 126,944 | 0.8220 / 0.9807 |
| value-bound-1000 | 0.094753 / 0.089273 | 23,722 / 21,226 | 0.9384 / 0.9310 |
| value-bound-4000 | 0.390178 / 0.360977 | 80,150 / 70,408 | 0.9207 / 0.9256 |
| value-runtime | 0.005615 / 0.005452 | 5,304 / 5,198 | 0.9760 / 0.9649 |
| bound-runtime | 0.005653 / 0.005680 | 5,216 / 5,388 | 1.0089 / 1.0023 |
| input-uses-1000-8 | 0.182286 / 0.160708 | 37,630 / 33,816 | 0.8759 / 0.8891 |
| input-uses-1000-128 | 2.018831 / 1.799594 | 300,792 / 254,716 | 0.8928 / 0.8902 |
| input-uses-4000-8 | 0.770104 / 0.696235 | 135,188 / 120,594 | 0.9141 / 0.9070 |
| input-runtime | 0.005818 / 0.005775 | 5,176 / 5,276 | 0.9882 / 0.9954 |

The large used-body median improves 9.38%, layout offsets 11.55%, and repeated
call inputs 10.86%; all improve in both paired blocks. Their peak RSS falls by
35,086, 35,352 and 46,076 KiB respectively. These are entry/final comparisons;
cross-campaign elapsed times are not used to isolate the local-view change.
Representative calibration and full ABBA spreads are:

| Compiler workload | A/A seconds | ABBA A seconds | ABBA B seconds |
| --- | --- | --- | --- |
| body-large-1000-128 | 2.243043–2.883895 | 2.179881–2.338324 | 1.978803–2.037215 |
| value-offset-1000-128 | 2.398331–2.573583 | 2.389339–2.618517 | 2.129583–2.152546 |
| input-uses-1000-128 | 1.956701–2.000573 | 1.993306–2.057720 | 1.787284–1.824624 |
| body-large-1000-8 | 0.270014–0.272625 | 0.267143–0.577856 | 0.247512–0.248750 |
| default-unused-1000 | 0.052206–0.053324 | 0.052284–0.395491 | 0.050972–0.053384 |
| memory-float-1 | 0.367585–0.375005 | 0.369417–0.611291 | 0.356631–0.362342 |
| dependent-default-runtime | 0.005773–0.005876 | 0.005769–0.006489 | 0.005698–0.049191 |

The small used-body, unused-default and mixed-memory A outliers remain included;
their unusually low paired ratios are not claimed as the steady-state benefit.
The dependent-default compiler B outlier .049191 s also remains included. Tiny
compiler inputs take about 5–7 ms and are dominated by startup: calls, memory and
bound medians rise by approximately 71, 12 and 27 microseconds; floating compilation
rises by 68 microseconds with mixed paired results. These observations and small
RSS increases remain visible. They do not establish useful speed improvements
or a numerical exit threshold. Longer affected workloads support the acceptance.

| Checked executable | A/B median runtime seconds | Payload bytes A/B | Paired B/A |
| --- | --- | --- | --- |
| region-runtime | 0.059130 / 0.059117 | 206 / 206 | 1.0008 / 0.9986 |
| dependent-default-runtime | 2.658709 / 2.660871 | 344 / 344 | 1.0162 / 0.9603 |
| calls-runtime | 0.476964 / 0.477789 | 206 / 206 | 1.0004 / 1.0018 |
| memory-runtime | 0.279431 / 0.278993 | 434 / 434 | 0.9775 / 0.9997 |
| floating-runtime | 0.330093 / 0.330018 | 230 / 230 | 1.0000 / 0.9986 |
| value-runtime | 0.059907 / 0.059924 | 184 / 184 | 1.0008 / 0.9988 |
| bound-runtime | 0.058876 / 0.058784 | 194 / 194 | 0.9970 / 0.9985 |
| input-runtime | 0.261036 / 0.260819 | 356 / 356 | 0.9954 / 0.9993 |

All 32 common-correct LowIR hashes and all eight executable hashes match exactly
in both campaigns. The new input loop checks twelve million live iterations,
volatile runtime bounds and a checksum; the inherited loops retain calls, memory
and floating work. Generated-code growth is **zero**, with no runtime optimization
claim. Payload follows the earlier sectionless-ELF measurement convention.

The TU-owned expression store retains immutable 24-byte properties, sparse
20-byte uses and a four-byte NodeId-to-use index. A use owns concrete entity and
receiver identities, incoming conversion and ready/evaluated state. Sharing source
properties does not share distinct local objects or lifetime actions. Source call
argument slices produce contextual views through the already-established occurrence
index; materialized argument objects keep concrete slices. Semantics, lowering,
conversion, unwind and cleanup consumers use the same typed accessor.

For N specializations and W repeated source operations, measured property/use
counts are respectively `4N+4W+4` / `N(3W+5)+3W+4` for large used bodies,
`3N+6W+1` / `N(5W+4)+5W+1` for layout offsets, and
`3N+5W+8` / `N(5W+6)+5W+7` for call-input uses. N=1,000 and W=8/128, plus
N=4,000 and W=8, vary source and instance dimensions independently. The retained
call-input edge count is `N+2W`; the former append path required `N+2W+2NW` by
source inspection (the entry did not expose that counter). Expression/conversion
and query work counters remain identical between these equivalent implementations.
The raw expression storage is `4S+20U+24F+44` bytes for S slots, U concrete uses and
F published property records and two sentinels, excluding geometric capacity and the bounded flat
conversion-variant index. Records release with the translation unit; local views
release on return. Whole-region occurrences and dense declaration Fact slots remain.

Compiler text changes 1,294,278→1,305,414 bytes for the first store, then to
**1,303,046 bytes**, a final increase of **8,768 (0.677%)**. The final local-view
change removes 2,368 bytes from the intermediate compiler. Public Entity,
Expression and ObjectUse sizes remain 112/36/36 bytes; frames remain 20, occurrences
eight, Ast 504 and query/type facts 48/48. Analyzer grows 5,576→5,680 bytes per TU.
The current probe verifies all 17 transitive live headers; historical snapshots
and their original measurements remain unchanged. The former live-header equality
to a historical probe is reclassified as snapshot evidence, while the latest probe
continues to check current headers and layouts.

At PA14/O0 no numeric compiler latency/RSS/text ceiling is mandated. Explicit
budgets remain at most four conversion variants per source operation, source/key/
concrete-use-proportional storage, one local view per visit and zero generated
growth. Repeatable compilation savings and lower memory justify the recorded
compiler text and per-TU owner cost. No optional optimizer was added. Compact
source/context occurrence views share one parsed graph; a zero-occurrence-count
gate is not inferred from the spec. Remaining fixed semantic rechecks and typed
demand/failure dependencies still require their own ownership audit. The inherited
source-cache +6,714 KiB and calls-4 roughly +15 MiB RSS deltas remain unexplained;
this campaign does not establish their cause.

Both frozen implementations pass 314 PA14 tests, 1,621 earlier tests and the
1,935-test through report, with unchanged coverage and comparisons. Each new
validation campaign records 338 release/ASan/UBSan parity inputs, 124 rejection
controls, six ABI controls, seven reducer/native checks, the storage/snapshot
control under both builds, and the lifetime control under entry/release/sanitizer.
The final native suite has 24 programs. File audit passes with the same three
inherited header advisories. Command/status manifests retain the initial failed
store adapter build, intermediate binaries, the profile and completed required
checks. The full verifier passes on all **10,514 observations** and these proofs.


## Ordinary member definition ownership (`167f5f43` → `0fc50a95`)

`definition-demand-performance.json` freezes 37 compiler inputs and nine checked
executables. It preserves every source/output from the preceding 32-input corpus,
adds four independent source/key/request scaling cases and one live member-call
loop, and records 644 observations: one warmup each, four A/A samples and two
ABBA blocks per campaign. All binaries, flags, sources and equivalent LowIR/native
outputs were frozen before timing; builds, tests and diagnostic profiling ran
separately. Medians below use the balanced eight ABBA observations. Calibration,
full samples, context switches and paired means remain unfiltered in the JSON.

| Compiler workload | A/B median seconds | A/B median peak RSS KiB | Paired B/A |
| --- | --- | --- | --- |
| body-run-1000-8 | 0.151648 / 0.151488 | 28,236 / 28,224 | 1.1659 / 0.9959 |
| body-run-1000-128 | 0.152086 / 0.152804 | 28,596 / 28,692 | 1.0054 / 0.9999 |
| body-run-4000-128 | 0.629744 / 0.631551 | 96,320 / 98,542 | 1.0029 / 1.2228 |
| body-large-1000-8 | 0.248509 / 0.246399 | 44,366 / 44,504 | 0.9974 / 1.2580 |
| body-large-1000-128 | 1.988715 / 1.977064 | 321,158 / 321,634 | 0.9973 / 0.9935 |
| default-unused-1000 | 0.050941 / 0.051546 | 13,176 / 13,264 | 1.0111 / 1.0086 |
| default-repeated-1000 | 0.029786 / 0.029816 | 8,950 / 8,930 | 1.0109 / 0.9976 |
| default-dependent-1000 | 0.051655 / 0.051862 | 13,358 / 13,140 | 1.0004 / 1.0006 |
| default-unused-4000 | 0.199938 / 0.201299 | 38,440 / 38,112 | 1.0103 / 1.0047 |
| default-repeated-4000 | 0.104265 / 0.104572 | 20,458 / 20,410 | 1.0068 / 1.0047 |
| default-dependent-4000 | 0.198326 / 0.198562 | 37,708 / 37,818 | 1.0034 / 1.0026 |
| region-runtime | 0.007356 / 0.007585 | 5,536 / 5,370 | 1.0436 / 1.0169 |
| dependent-default-runtime | 0.005743 / 0.005899 | 5,286 / 5,174 | 1.0305 / 1.0283 |
| declaration-instances-1000 | 0.569330 / 0.571060 | 83,088 / 83,324 | 0.9686 / 1.0129 |
| declaration-outside-1000 | 0.333151 / 0.314687 | 54,878 / 56,128 | 0.9833 / 0.9436 |
| member-repeated-1000 | 0.054300 / 0.054286 | 15,034 / 14,992 | 1.0031 / 1.0015 |
| calls-4 | 1.709064 / 1.678346 | 270,018 / 269,986 | 1.0003 / 0.9610 |
| memory-float-1 | 0.355550 / 0.356564 | 63,010 / 62,900 | 1.0043 / 1.0023 |
| calls-runtime | 0.005425 / 0.005575 | 5,282 / 5,112 | 1.0257 / 1.0286 |
| memory-runtime | 0.005518 / 0.005591 | 5,162 / 5,138 | 1.0504 / 1.0028 |
| floating-runtime | 0.005437 / 0.005629 | 5,434 / 5,344 | 1.0206 / 1.0487 |
| value-offset-1000-8 | 0.172721 / 0.173763 | 35,402 / 35,394 | 1.0077 / 1.0060 |
| value-offset-1000-128 | 2.128484 / 2.128721 | 337,916 / 337,902 | 0.9942 / 1.0078 |
| value-offset-4000-8 | 0.736564 / 0.739565 | 126,354 / 126,470 | 1.0008 / 1.0061 |
| value-bound-1000 | 0.089521 / 0.089345 | 21,204 / 21,410 | 0.9875 / 1.0084 |
| value-bound-4000 | 0.358014 / 0.357874 | 69,472 / 69,438 | 1.0012 / 1.0088 |
| value-runtime | 0.005615 / 0.005763 | 5,142 / 5,126 | 1.0412 / 1.0117 |
| bound-runtime | 0.005568 / 0.005797 | 5,308 / 5,160 | 1.0434 / 1.0343 |
| input-uses-1000-8 | 0.161319 / 0.162306 | 33,814 / 35,116 | 1.0161 / 0.9942 |
| input-uses-1000-128 | 1.794565 / 1.785652 | 254,748 / 269,954 | 1.0168 / 0.9796 |
| input-uses-4000-8 | 0.693294 / 0.694013 | 120,174 / 120,052 | 0.9451 / 1.0052 |
| input-runtime | 0.006069 / 0.006210 | 5,252 / 5,142 | 1.0194 / 1.0273 |
| demand-uses-1000-8-4 | 0.611172 / 0.387287 | 81,466 / 55,892 | 0.6301 / 0.6203 |
| demand-uses-1000-128-4 | 8.006766 / 3.492325 | 786,884 / 372,090 | 0.4383 / 0.4345 |
| demand-uses-4000-8-4 | 2.540783 / 1.584411 | 307,038 / 207,268 | 0.6285 / 0.6225 |
| demand-uses-1000-8-64 | 2.277459 / 1.999396 | 299,950 / 265,028 | 0.8774 / 0.8778 |
| demand-runtime | 0.006959 / 0.006842 | 5,514 / 5,118 | 0.9961 / 0.9781 |

The four new long-running compiler cases improve 36.63%, 56.38%, 37.64% and
12.21%, each in both paired blocks. Median peak RSS falls by 25,574, 414,794,
99,770 and 34,922 KiB. The smaller native-input compilation takes about 7 ms;
its timing is not the basis for acceptance. Representative full spreads are:

| Compiler workload | A/A seconds | ABBA A seconds | ABBA B seconds |
| --- | --- | --- | --- |
| demand-uses-1000-8-4 | 0.609049–0.612726 | 0.604671–0.654163 | 0.384675–0.391714 |
| demand-uses-1000-128-4 | 7.944603–8.006677 | 7.962301–8.052570 | 3.476860–3.515615 |
| demand-uses-4000-8-4 | 2.521156–2.557174 | 2.512774–2.542494 | 1.581535–1.589882 |
| demand-uses-1000-8-64 | 2.259464–2.271916 | 2.270164–2.283904 | 1.989627–2.005420 |
| body-run-1000-8 | 0.152512–0.156403 | 0.149962–0.151822 | 0.150596–0.200308 |
| body-run-4000-128 | 0.624257–1.054054 | 0.622709–0.637217 | 0.625073–0.915507 |
| body-large-1000-8 | 0.245947–0.253020 | 0.243235–0.250422 | 0.244953–0.380225 |
| input-uses-1000-128 | 1.776835–1.816643 | 1.775106–1.819994 | 1.775199–1.823630 |

The inherited corpus retains small latency increases: unused defaults rise
0.6–1.4 ms, several 5–8 ms compiler inputs rise 73–229 microseconds, and the
small/many-instance offset cases rise 1.0/3.0 ms. These costs are disclosed; no
whole-corpus speedup is claimed. B outliers of .200308 s (body-run-1000-8),
.915507 s (body-run-4000-128) and .380225 s (body-large-1000-8) remain in their
paired means. The call-input-128 median peak RSS rises 15,206 KiB, despite
identical reported semantic work/counts and nearly unchanged latency. This
specific memory cost is investigated below, without discarding its observations.
Other positive RSS deltas include 2,222 KiB for body-run-4000-128, 1,250 KiB for
declaration-outside and 1,302 KiB for input-uses-1000-8.

| Checked executable | A/B median runtime seconds | Payload bytes A/B | Paired B/A |
| --- | --- | --- | --- |
| region-runtime | 0.059315 / 0.059355 | 206 / 206 | 1.0027 / 1.0011 |
| dependent-default-runtime | 2.653447 / 2.639773 | 344 / 344 | 1.0076 / 0.9864 |
| calls-runtime | 0.477130 / 0.476202 | 206 / 206 | 0.9989 / 0.9988 |
| memory-runtime | 0.278873 / 0.279163 | 434 / 434 | 1.0003 / 1.0008 |
| floating-runtime | 0.330419 / 0.330284 | 230 / 230 | 1.0005 / 0.9922 |
| value-runtime | 0.059893 / 0.059967 | 184 / 184 | 1.0008 / 1.0008 |
| bound-runtime | 0.058660 / 0.058570 | 194 / 194 | 1.0007 / 0.9584 |
| input-runtime | 0.260251 / 0.260608 | 356 / 356 | 1.0007 / 1.0026 |
| demand-runtime | 0.155910 / 0.156278 | 261 / 261 | 1.0036 / 1.0021 |

All 37 LowIR hashes and nine executable hashes match exactly. The new native
case executes four calls in each of twelve million iterations with a volatile
bound and a checked checksum. Existing loops retain memory, floating-point and
call behavior. Generated-code growth is **zero** and no runtime optimization is
claimed. Native RSS is 256 KiB in these measurements. Payload retains the earlier
sectionless-ELF convention. Small runtime differences, all recorded in the raw
samples, do not imply changed executable work.

Source prototypes now own normalized signature identities and selected-definition
lists. Concrete members retain those prototype IDs. Application state is keyed
by root specialization/source definition; traversal success or absence is keyed
by concrete member/current source bucket head. Source checking must finish before
selection is published; re-entrant Active applications cannot publish completion.
Late definitions change only the affected head identity. No global invalidation
or complete visible-environment copy is added. Renamed nested aliases use their
recorded declaring head. Matched ordinary definitions register the known member
and substitute retained raw parameter types, preserving body cv/array/function
forms independently from adjusted callable types. Body syntax stays deferred.

For N specializations, K overloads and Q repeated requests per member, source
signature requests/work are K, direct applications/visited definition edges are
N, total requests are N(Q+1), and completed traversal hits are NQ. Entry applied
NK definitions. Required candidate work stays NKQ on both compilers. Measured
occurrences fall from N(53K+17) to N(22K+48). Cases N=1,000/K=8/Q=4,
N=1,000/K=128/Q=4, N=4,000/K=8/Q=4 and N=1,000/K=8/Q=64 separate those dimensions.
Counters are checked by the verifier; they do not substitute for timed evidence.
Distinct class member declarations still require their own concrete identities.

Compiler text grows 1,303,046→1,310,598 bytes, **7,552 bytes (0.580%)**. Per-TU
Analyzer grows 5,680→5,920 bytes; a source prototype grows 16→24, a retained
source definition 24→32, and a concrete MemberFacts record 120→124. Public
Entity/Expression/ObjectUse remain 112/36/36, expression properties/uses 24/20,
frames 20, occurrences eight, Ast 504 and query/type facts 48/48. The new frozen
layout probe checks all 17 transitive live headers. The preceding expression
probe is now historical snapshot evidence; its original source, binary, dump,
header hashes and measurements remain intact. This reclassifies an unsupported
historical live-header equality gate without removing a mandated layout limit.

PA14/O0 has no mandated numeric latency/RSS/text ceiling. Explicit budgets remain
source/key/concrete-use-proportional storage, at most four conversion variants
per source operation, one local view per visit and zero generated growth. The
repeatable savings on affected workloads justify the new source/member owners
and text growth. There is no optional runtime transform or global cache here.
The inherited source-cache +6,714 KiB and calls-4 roughly +15 MiB deltas remain
preserved and unexplained by this campaign; none is silently converted into a
performance claim or a permanent numerical exit gate.

The frozen implementation passes 314 stage tests, 1,621 prior tests and the
1,935-test through report. Validation records 342 release/ASan/UBSan parity
sources, 136 explicit rejection controls, six ABI controls, seven native reducers,
28 native programs, release/sanitizer expression-store checks and the lifetime
control under entry/release/sanitizer. File audit passes with three inherited
header advisories. Twelve reduced unused-definition programs accepted by entry
now reject; the proof cites N3485 [class.mem]/1, [class.mfct]/1–2,
[except.spec]/3–4 and [basic.def.odr]/1, with host agreement as supporting evidence.
No fixture/reference changed. Initial invalid personal pointer comparison,
313/314 alias regression, renamed nested-head regression, intermediate binaries
and command logs are retained in the handoff manifest.


Separate Valgrind 3.26.0 Massif diagnostics of the inherited call-input-128 case
record peak live heap 308,822,014→308,822,038 bytes, a **24-byte** difference;
allocator bookkeeping at those peaks changes 38,802→38,794 bytes. All reported
semantic counters and LowIR output hashes are identical. The profiles expose the
same dominant Ast occurrence-index allocation (67,108,864 bytes) under function
body demand. Instrumented live allocation is not native RSS. This supports an
allocator/page-retention explanation for the native +15,206 KiB RSS delta, but
does not isolate the precise native allocator policy responsible. The measured
RSS cost remains, with both complete profiles and commands in the handoff
manifest. It is not 15 MiB of new live semantic records, and no allocator tuning
or benchmark-specific production path was introduced to suppress it.


## Special-member signatures, injected types and direct application

Continuation entry is `60cf761c`; production changes are `bdbb04d4` and
`5b947a18`, with frozen proofs/harnesses at `f84029d0`. The complete isolated
campaign in `special-signature-performance.json` retains **756 observations**:
44 compiler inputs and ten checked native programs, each with one warmup per
binary, four A/A calibration samples and two ABBA blocks. Together with the
unchanged 11,158 preceding observations, **11,914** observations are retained.
All binaries, flags, sources, equivalent outputs and harnesses were frozen before
timing. Compiler and runtime measurements are separate, pinned to the recorded
CPU; builds, validation and probes were not run concurrently with timing.

A is the entry compiler (SHA `288643cc7b0d9ca819bc7708a6a725a9790f8e1baa0ad8ed716d414fb17647dc`);
B is the final direct-special compiler (SHA `dcce35ef47ad992313d5dadd48385d25b242c52376b47ba5c9b0b6c1b392b71a`).
The two `special-heads` cases instead use the correct frozen injected-head build
(SHA `625ca5e722b23f3619a6acd48abea0505cbb078aec53a9ec7dbacd83a3745195`) as A:
entry rejects the valid nested alias program. This explicitly recorded override
measures direct application between correct implementations. It does not time
rejection as successful compilation. All 37 inherited sources and outputs remain
byte identical to their preceding campaign.

The compiler tables report medians of the four A and four B ABBA observations;
calibration is reported separately. Every warmup, sample, RSS value, context
switch count and outlier remains in the JSON. Spreads are min–max, and paired
ratios use each block's arithmetic means. No outlier is removed.

| Compiler workload | Median seconds A/B | Median RSS KiB A/B | A/A seconds | ABBA range seconds A/B | Paired B/A |
| --- | --- | --- | --- | --- | --- |
| body-run-1000-8 | 0.157616 / 0.157332 | 28,342 / 28,292 | 0.154423–0.198110 | 0.153606–0.193319 / 0.155902–0.158488 | 0.9027 / 1.0020 |
| body-run-1000-128 | 0.157381 / 0.158745 | 28,690 / 28,830 | 0.157798–0.197920 | 0.154597–0.197852 / 0.156563–0.163308 | 0.9099 / 1.0071 |
| body-run-4000-128 | 0.656344 / 0.659578 | 98,594 / 98,846 | 0.657013–0.672506 | 0.634941–0.713083 / 0.632868–0.677204 | 1.0010 / 0.9757 |
| body-large-1000-8 | 0.272327 / 0.255358 | 44,438 / 48,060 | 0.250085–0.296377 | 0.250150–0.359631 / 0.255092–0.260584 | 0.8353 / 0.9496 |
| body-large-1000-128 | 2.092869 / 2.151097 | 321,612 / 321,360 | 2.020567–2.557939 | 2.073115–2.257721 / 2.021408–2.383809 | 1.0256 / 1.0191 |
| default-unused-1000 | 0.052843 / 0.053038 | 13,330 / 13,272 | 0.052395–0.053762 | 0.052606–0.053246 / 0.052833–0.053775 | 1.0118 / 0.9990 |
| default-repeated-1000 | 0.030242 / 0.030401 | 8,930 / 8,946 | 0.030026–0.030911 | 0.029989–0.030787 / 0.029989–0.030774 | 0.9877 / 1.0174 |
| default-dependent-1000 | 0.052065 / 0.052422 | 13,400 / 13,414 | 0.052291–0.053704 | 0.051768–0.052094 / 0.051544–0.052845 | 1.0095 / 1.0025 |
| default-unused-4000 | 0.203845 / 0.204304 | 38,166 / 38,048 | 0.202249–0.441973 | 0.200393–0.246054 / 0.202307–0.206045 | 1.0113 / 0.9073 |
| default-repeated-4000 | 0.104685 / 0.104580 | 20,400 / 20,434 | 0.103070–0.142705 | 0.103820–0.106542 / 0.103843–0.106155 | 1.0019 / 0.9954 |
| default-dependent-4000 | 0.210658 / 0.201907 | 37,830 / 37,868 | 0.205051–0.511231 | 0.201827–0.255198 / 0.201090–0.209238 | 0.8789 / 0.9795 |
| region-runtime | 0.007624 / 0.007654 | 5,474 / 5,556 | 0.007663–0.008298 | 0.007594–0.007794 / 0.007574–0.007682 | 1.0027 / 0.9926 |
| dependent-default-runtime | 0.006238 / 0.006182 | 5,226 / 5,392 | 0.006066–0.006231 | 0.006109–0.006334 / 0.006087–0.006357 | 0.9960 / 0.9951 |
| declaration-instances-1000 | 0.575233 / 0.574736 | 83,334 / 83,312 | 0.574847–0.575936 | 0.574192–0.577683 / 0.572771–0.579147 | 0.9995 / 0.9997 |
| declaration-outside-1000 | 0.317263 / 0.314377 | 56,164 / 56,136 | 0.314028–0.317213 | 0.312749–0.324180 / 0.310515–0.324274 | 0.9957 / 0.9918 |
| member-repeated-1000 | 0.055174 / 0.054381 | 15,044 / 15,048 | 0.054024–0.054724 | 0.054899–0.182479 / 0.054020–0.054480 | 0.9908 / 0.4558 |
| calls-4 | 1.699534 / 1.719451 | 270,020 / 270,032 | 1.709251–1.920076 | 1.694950–1.710048 / 1.695465–1.785815 | 1.0275 / 1.0067 |
| memory-float-1 | 0.362763 / 0.367051 | 63,032 / 63,012 | 0.361960–0.484849 | 0.359315–0.370993 / 0.361214–0.367826 | 1.0101 / 1.0001 |
| calls-runtime | 0.005493 / 0.005521 | 5,198 / 5,176 | 0.005574–0.005947 | 0.005439–0.005613 / 0.005448–0.005606 | 1.0086 / 0.9967 |
| memory-runtime | 0.006113 / 0.005954 | 5,232 / 5,256 | 0.006102–0.006277 | 0.005848–0.006406 / 0.005845–0.006063 | 0.9559 / 0.9907 |
| floating-runtime | 0.005620 / 0.005569 | 5,486 / 5,288 | 0.005628–0.006342 | 0.005530–0.005665 / 0.005509–0.005634 | 1.0020 / 0.9843 |
| value-offset-1000-8 | 0.176547 / 0.176702 | 35,402 / 35,422 | 0.174717–0.176873 | 0.176104–0.178608 / 0.175476–0.192093 | 1.0425 / 0.9949 |
| value-offset-1000-128 | 2.142794 / 2.143584 | 337,822 / 337,938 | 2.132349–2.156443 | 2.121510–2.165040 / 2.125995–2.164439 | 1.0032 / 0.9981 |
| value-offset-4000-8 | 0.753972 / 0.753322 | 126,460 / 126,372 | 0.739694–0.756620 | 0.748210–0.778472 / 0.745818–0.755757 | 0.9975 / 0.9852 |
| value-bound-1000 | 0.091427 / 0.091226 | 21,454 / 21,370 | 0.090336–0.100026 | 0.090917–0.091598 / 0.090791–0.091731 | 0.9966 / 1.0012 |
| value-bound-4000 | 0.366460 / 0.367662 | 69,442 / 69,446 | 0.363229–0.372723 | 0.363076–0.528201 / 0.362866–0.374903 | 1.0186 / 0.8159 |
| value-runtime | 0.005712 / 0.005686 | 5,330 / 5,142 | 0.005774–0.006002 | 0.005648–0.005812 / 0.005637–0.005736 | 0.9970 / 0.9910 |
| bound-runtime | 0.005718 / 0.005671 | 5,184 / 5,254 | 0.005776–0.005895 | 0.005678–0.005794 / 0.005644–0.005702 | 0.9921 / 0.9886 |
| input-uses-1000-8 | 0.168201 / 0.167492 | 35,098 / 35,122 | 0.165649–0.170818 | 0.165102–0.226188 / 0.164705–0.170169 | 0.8518 / 1.0020 |
| input-uses-1000-128 | 1.819844 / 1.886133 | 270,032 / 269,992 | 1.769391–1.825078 | 1.790140–1.831531 / 1.783015–2.001065 | 0.9934 / 1.0871 |
| input-uses-4000-8 | 0.701366 / 0.704496 | 120,050 / 120,088 | 0.696179–0.719975 | 0.697204–0.719538 / 0.693564–0.778027 | 1.0481 / 0.9949 |
| input-runtime | 0.006385 / 0.006480 | 5,214 / 5,370 | 0.006618–0.006674 | 0.006308–0.006426 / 0.006358–0.006721 | 0.9994 / 1.0426 |
| demand-uses-1000-8-4 | 0.392309 / 0.389048 | 55,834 / 55,862 | 0.387850–0.392524 | 0.388577–0.393968 / 0.385930–0.402493 | 1.0079 / 0.9912 |
| demand-uses-1000-128-4 | 3.492002 / 3.492226 | 372,360 / 372,298 | 3.508639–3.540510 | 3.476379–3.532002 / 3.488897–3.521506 | 0.9986 / 1.0017 |
| demand-uses-4000-8-4 | 1.590042 / 1.593490 | 207,148 / 207,254 | 1.587734–1.596380 | 1.574078–1.603782 / 1.586813–1.608328 | 1.0140 / 0.9938 |
| demand-uses-1000-8-64 | 2.001867 / 2.000309 | 265,072 / 264,984 | 1.990666–2.023587 | 1.996510–2.028203 / 1.991706–2.011013 | 0.9970 / 0.9968 |
| demand-runtime | 0.007105 / 0.007040 | 5,312 / 5,330 | 0.006966–0.007304 | 0.007066–0.007214 / 0.006880–0.007125 | 0.9865 / 0.9849 |
| special-uses-1000-8-4 | 0.736660 / 0.513465 | 106,186 / 81,518 | 0.725700–0.731302 | 0.734699–0.753979 / 0.508003–0.516523 | 0.6859 / 0.6994 |
| special-uses-1000-128-4 | 7.473621 / 3.204839 | 789,926 / 385,470 | 7.188911–7.645654 | 7.397737–7.539747 / 3.138208–3.227404 | 0.4257 / 0.4293 |
| special-uses-4000-8-4 | 3.070506 / 2.115285 | 408,810 / 305,840 | 3.043246–3.163439 | 3.058051–3.089953 / 2.111123–2.125002 | 0.6871 / 0.6908 |
| special-uses-1000-8-64 | 3.496290 / 3.249456 | 492,458 / 480,686 | 3.458864–3.579768 | 3.477324–3.986703 / 3.220685–3.303878 | 0.9294 / 0.8741 |
| special-heads-1000 | 1.405064 / 1.316381 | 70,080 / 69,126 | 1.113409–1.650007 | 1.246395–1.523094 / 1.253320–1.405118 | 0.9563 / 0.9411 |
| special-heads-4000 | 3.973118 / 3.758667 | 263,904 / 261,838 | 3.908652–4.192809 | 3.932223–4.063050 / 3.737394–3.866606 | 0.9402 / 0.9569 |
| special-runtime | 0.019396 / 0.019605 | 5,584 / 5,430 | 0.018050–0.019669 | 0.018700–0.020522 / 0.018324–0.019996 | 1.0236 / 0.9651 |

The four constructor N/K/Q cases improve **30.30%, 57.12%, 31.11% and 7.06%**;
both paired blocks improve in every case. Median peak RSS falls by **24,668,
404,456, 102,970 and 11,772 KiB** respectively. The nested-head cases improve
6.31% and 5.40%, with 954 and 2,066 KiB less median peak RSS. Their broad A/A
ranges make the precise latency estimates less certain; their counter reductions
and both paired blocks support the direction. The tiny native-input compiler
measurements are retained but do not establish profitability.

Inherited median latency increases include body-large-1000-128 (+58.2 ms, 2.78%),
calls-4 (+19.9 ms, 1.17%), input-uses-1000-128 (+66.3 ms, 3.64%) and memory-float-1
(+4.3 ms, 1.18%). The input-128 paired blocks disagree (0.9934/1.0871), and its
later B samples reach 2.001065/1.976459 seconds. Calls-4 and input-128 have
identical recorded semantic work. The body-large cases add one symbolic entity,
one scope and 1,109 type probes while eliminating 2,000 repeated source-type uses;
that is bounded work for the now-retained constructor signature/current type.
The large-body timing increase is smaller than its 2.020567–2.557939-second A/A
range; it remains disclosed rather than characterized as a proven slowdown or
an overall speedup. Its semantic preflight time was 765.861→762.077 ms, which is
untimed diagnostic context rather than another performance sample.

Body-large-1000-8 median RSS rises **3,622 KiB**, despite only one additional
entity/scope and unchanged measured record sizes. Its exact native allocation
cause is not isolated. All other inherited positive median RSS deltas are at
most 252 KiB in this campaign. Neither this cost nor the historical +6,714 KiB,
roughly +15 MiB and +15,206 KiB costs is erased or converted into a required
numeric gate. The previous Massif evidence remains as recorded and is not
extrapolated to diagnose this different case. Required source signature facts
are retained; no optional runtime transform or global cache was added.

Timing outliers also affect apparent improvements: body-large-1000-8 has an A
sample of .359631 s; member-repeated-1000 has a paired ratio of .4558. Complete
calibration and ABBA ranges above retain their effects. No whole-corpus speedup
is claimed.

| Checked executable | Median seconds A/B | Payload bytes A/B | A/A seconds | ABBA range seconds A/B | Paired B/A |
| --- | --- | --- | --- | --- | --- |
| region-runtime | 0.058913 / 0.058864 | 206 / 206 | 0.058956–0.108571 | 0.058897–0.058945 / 0.058842–0.059218 | 0.9989 / 1.0022 |
| dependent-default-runtime | 2.742641 / 2.746852 | 344 / 344 | 2.630390–2.677017 | 2.676240–2.796340 / 2.693630–2.905592 | 1.0401 / 0.9845 |
| calls-runtime | 0.481555 / 0.478608 | 206 / 206 | 0.478885–0.500751 | 0.478331–0.490018 / 0.478489–0.479542 | 0.9950 / 0.9882 |
| memory-runtime | 0.279276 / 0.279426 | 434 / 434 | 0.278965–0.280998 | 0.278322–0.280056 / 0.278888–0.284272 | 1.0002 / 1.0089 |
| floating-runtime | 0.330980 / 0.332555 | 230 / 230 | 0.330736–0.331311 | 0.330584–0.331972 / 0.331358–0.345677 | 1.0051 / 1.0215 |
| value-runtime | 0.060010 / 0.061568 | 184 / 184 | 0.059773–0.060814 | 0.059885–0.062249 / 0.060290–0.061881 | 1.0159 / 1.0102 |
| bound-runtime | 0.059079 / 0.059168 | 194 / 194 | 0.059021–0.059413 | 0.058687–0.059480 / 0.058742–0.059616 | 1.0007 / 1.0024 |
| input-runtime | 0.261858 / 0.262098 | 356 / 356 | 0.259595–0.262709 | 0.260563–0.266513 / 0.261451–0.267662 | 0.9923 / 1.0126 |
| demand-runtime | 0.155910 / 0.157020 | 261 / 261 | 0.155580–0.156935 | 0.155634–0.156241 / 0.155904–0.159546 | 1.0097 / 1.0089 |
| special-runtime | 0.340637 / 0.333398 | 400 / 400 | 0.322196–0.332866 | 0.334760–0.348443 / 0.327619–0.336840 | 0.9480 / 1.0044 |

All ten executable hashes match exactly, with **zero generated-code growth**.
The new program executes four constructor/read/destructor scopes during each
of twelve million iterations, using a volatile bound and checked checksum.
The earlier memory, floating-point and call loops remain unchanged. Native peak
RSS is 256 KiB; payload follows the retained sectionless-ELF convention. Runtime
differences (including small increases for floating/value/demand loops) are
recorded in full; identical executables support no changed-work or runtime
optimization claim. Native output is produced using PA8's supplied backend,
separately from this compiler's own LowIR construction.

Thirty-nine compiler outputs have equal raw LowIR hashes. Five new constructor
outputs differ in local slot suffixes but compare equal under the unchanged
course validator and positional canonicalizer. The personal Perl adapter loads
those functions and validates both inputs with student presentation ordering;
it requires exact canonical equality and does not use generated-projection
fallback. PA8 LowIR's comparison contract makes local slot names presentation
and absorbs top-level presentation ordering. The initial byte-only preflight
and the subsequent reference-order preflight were unsupported self-imposed
requirements for two student outputs. Both failed runs, comparison inputs and
harness versions are preserved. No fixture, reference, validation rule or
comparison implementation changed. The adapter, course comparator and contract
hashes are verified in both the proof and performance manifests.

For N specializations, K constructor overloads and Q repeated uses, source
signature requests/work are K+1, selected applications/direct applications and
visited definition edges are 2N, total definition requests are 2N(Q+1), and
completed traversal hits are 2NQ. Entry applies N(K+1) definitions. Required
candidate work stays NKQ. Frames fall from N(K+2) to 3N, and occurrence records
from N(47K+52) to N(17K+82). N=1000/K=8/Q=4, N=1000/K=128/Q=4,
N=4000/K=8/Q=4 and N=1000/K=8/Q=64 vary those dimensions independently.

Nested-head cases compare the correct intermediate to final. Both apply 5N
definitions with 6N frames, 224N occurrences, 12N requests, 5N hits, 7N edges and
four source signature computations. Direct applications rise N→4N. At N=1000,
concrete entities/scopes fall by 4000/3000, lookup work falls 94075→46075 and
source-type substitution records rise 5000→9000. The added raw-parameter facts
replace reconstructed declarations and scopes; the timed benefit and bounded
storage justify those records. Body/default/exception/transfer/ABI decisions
retain their existing semantic owners.

Compiler text grows **1,310,598→1,311,558 bytes: 960 bytes (0.0732%)**. Relative
to the correct injected-head intermediate, direct application adds 512 bytes.
Measured layouts remain unchanged: Entity/Expression/ObjectUse 112/36/36,
expression properties/uses 24/20, frame 20, occurrence eight, Ast 504, query/type
facts 48/48, MemberFacts 124, TemplateDefinition 32, TemplatePrototype 24 and
Analyzer 5920. The current probe checks 17 transitive live headers. The preceding
probe now checks its immutable frozen snapshot, preserving all original hashes,
dumps and measurements; historical live-header equality is not a mandated limit.

PA14/O0 has no mandated numeric compiler latency/RSS/text ceiling. Explicit
budgets remain source/key/concrete-use-proportional storage, at most four O0
conversion variants per source operation, one local view per visit and zero
generated growth. The repeatable constructor savings, lower affected-case RSS
and limited compiler growth justify this completed owner change. Necessary
semantic work and all inherited costs remain visible; this acceptance does not
waive the remaining declaration/lifetime/demand architecture requirements.

Validation passes 314 stage tests, 1621 prior tests, the 1935-test through report,
344 release/ASan/UBSan parity sources, 157 rejection controls, six ABI controls,
seven native reducers, thirty native programs and store/lifetime controls.
File audit passes with three inherited header advisories. Twenty newly rejected
invalid special definitions have reduced C++11 rule proofs; the already-rejected
conversion-noexcept control remains. Both new valid positive controls fail at
entry and execute under the corrected intermediate and final builds. Frozen
logs, initial failures, binaries, layouts and the final check manifest live under
`$RALPH_ARTIFACT_DIR/pa14-special-signatures/`. The cumulative verifier checks all
11,914 observations without changing prior evidence.
