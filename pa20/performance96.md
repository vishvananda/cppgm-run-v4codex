# PA20 aggregate and captureless-callable evidence — handoff 96

[Reproducer](../student.tests/pa20/benchmark96.py),
[all observations](../student.tests/pa20/performance96.json),
[validation](../student.tests/pa20/validation96.json).
Entry is `e59e46fa`; final implementation is `5d2e6a65` (including `beb83901` and `6fbfd99f`).
The frozen binaries remain in `/tmp/pa20-loop96/{entry,final-cache}`. No reference,
source fixture, status sidecar or comparison rule changes in this handoff.

## Protocol and limits

Both builds use `g++ -std=gnu++11 -Wall -O3`, `TEST_RUNNER_ENABLE`; compilation
uses `--emit-lowir -O0`. The supplied native backend is used only by the test
harness, at `-O0`. Its hash, every input, flags, output hashes and telemetry are
recorded. CPU affinity is CPU 1; no concurrent build or correctness run ran
during measurement. Compiler and executable timing are separate.

Fourteen common workloads have one warmup per binary, four A/A observations,
and four ABBA blocks in each dimension. One newly supported default-argument
workload has a warmup and six final-only observations in each dimension.
**572 measured observations and 58 warmups** in the final run are retained
without filtering. The preceding full run against `6fbfd99f` remains in
[before-cache measurements](../student.tests/pa20/performance96-before-cache.json)
with its binary at `/tmp/pa20-loop96/final`; total retained observations/warmups
across the two runs are 1,144/116.
All native executions check their result. Volatile loop limits and dependent
masked sums keep the executable workloads live; specialization executions are
startup-dominated correctness controls, not runtime-profit evidence.

Compiler text is measured as `.text`. The supplied native backend produces a
sectionless ELF: native sizes below are its code/data payload proxy, not an
exact text section. Self-hosting, student encoding, debug and optimization
policies belong to the later stages. No new optimization pass is introduced.

| Binary | SHA-256 | Compiler text bytes |
|---|---|---:|
| Entry | `3971efde7636140fa157dacc70c54d9142d4fbfeb5e67e927534743ca162c1b9` | 2,060,870 |
| Final | `6678cf1f0c551ec5277df1ae913d9cc4e6cf1ef1fe741fae63b0439eb306bbb1` | 2,063,494 |

Text grows **2,624 bytes (0.127%)**.

## Observations and acceptance

Medians are milliseconds; RSS is peak KiB. Both summarize the ABBA observations
(six final observations for new behavior); A/A and warmups remain separately
recorded. A/B means entry/final.

| Workload | Compiler A/B ms | RSS A/B KiB | Runtime A/B ms | Native payload A/B bytes |
|---|---:|---:|---:|---:|
| startup | 19.53 / 18.95 | 5,708 / 5,888 | 10.12 / 10.70 | 24 / 24 |
| auto specializations 2,400 | 707.62 / 732.06 | 30,916 / 31,172 | 5.86 / 5.93 | 192,062 / 192,062 |
| auto specializations 9,600 | 722.15 / 689.36 | 105,768 / 105,872 | 3.97 / 3.88 | 768,062 / 768,062 |
| namespace variables 9,600 | 540.47 / 539.36 | 81,776 / 82,048 | 3.28 / 3.36 | 24 / 24 |
| calls loop | 6.51 / 6.28 | 6,056 / 6,072 | 122.63 / 122.92 | 206 / 206 |
| memory loop | 7.18 / 6.93 | 5,968 / 6,108 | 73.71 / 73.26 | 434 / 434 |
| floating loop | 6.95 / 6.56 | 6,080 / 6,120 | 86.18 / 86.39 | 230 / 230 |
| array members 800 | 165.13 / 160.27 | 31,228 / 31,236 | 3.22 / 3.24 | 153,661 / 176,095 |
| closure entries 800 | 189.13 / 179.36 | 31,660 / 30,076 | 3.45 / 3.49 | 174,462 / 158,462 |
| array members 3,200 | 651.11 / 640.58 | 96,424 / 95,780 | 3.35 / 3.40 | 614,461 / 704,095 |
| closure entries 3,200 | 768.67 / 739.26 | 108,856 / 106,052 | 3.74 / 3.61 | 697,662 / 633,662 |
| pointer closure loop | 6.72 / 6.57 | 6,072 / 6,192 | 54.43 / 45.21 | 258 / 208 |
| immediate closure loop | 6.51 / 6.19 | 6,056 / 6,128 | 45.24 / 45.19 | 192 / 200 |
| array member loop | 6.81 / 6.69 | 5,984 / 6,140 | 36.44 / 110.26 | 285 / 343 |
| new default closure loop | — / 6.79 | — / 6,048 | — / 2,625.97 | — / 284 |

The seven general-control executables are byte-identical. Their runtime
variation cannot establish a code regression. Calls-loop paired runtime ratios
are 1.005, 1.002, 1.005, 1.003. Compiler measurements show substantial noise:
auto-specializations-2400 final samples span 344.08–806.45 ms; 9600's A/A spans
705.37–1,435.63 ms. Array-members-3200 final samples span 627.43–663.64 ms and
its paired ratios are 0.995, 0.925, 0.840, 1.013. Closure-entries-3200 A/A spans
730.26–759.62 ms; final samples span 700.44–798.52 ms. No general compiler
speedup or simple wall-time scaling claim is made. Work-count bounds are
reported separately below. All 15 final LowIR/native outputs are byte-identical
to the preceding before-cache run; both runs' timing and RSS variation is retained.

Pointer conversions now use the demanded parameter-only body entry, avoiding
the extra wrapper call. The affected loop's paired runtime B/A ratios are
**0.796, 0.805, 0.799, 0.859**; paired compiler ratios are
0.882, 1.066, 0.973, 0.993, and payload shrinks 50 bytes. This measured benefit
supports removing that redundant boundary. Immediate-call ratios are
0.980, 1.003, 0.978, 1.026; its mandatory indirect-entry presentation provides
no established runtime benefit and adds eight payload bytes.

**Array helper costs are a regression, not a speedup claim.** The loop's paired
runtime ratios are **3.032, 3.015, 3.022, 3.034** and payload grows 58 bytes.
The entry implementation executed these sources correctly but did not satisfy
the required PA20 LowIR shape. The unchanged
[contract oracle](tests/general/100-aggregate-element-array-member-braced-init.ref)
requires member-array temporary storage, helper calls and representation
copies. These operations explain the measured native cost. At 3,200 templates,
IR instructions decrease from 163,205 to 156,817 while native payload grows
14.6%; a smaller IR is explicitly not treated as runtime evidence. Peak compiler
RSS is 96,424/95,780 KiB in this run versus 97,504/105,640 KiB before the cache.
The extra temporary/operand/signature records remain; RSS layout variability
precludes attributing the difference between runs to the small classifier cache.
Eliminating these calls/copies at PA20/O0 would violate the unchanged comparison
contract. Inlining and native optimization belong to the later stages. This is
a required representation cost, not an optional transform retained despite loss.

The new default-argument source is rejected at entry. Its compiler samples
span 6.40–13.07 ms and runtime spans 2,071.10–2,663.99 ms. The run checks exactly
eight million side-effecting default evaluations and eight million calls.
No comparative gain is claimed; the substantial runtime spread is retained.

Spec §9 stage scoping applies: PA20/O0 mandates no percentage latency, RSS or
native-size gate. Earlier self-selected diagnostics are not additional gates;
all prior evidence and mandated complexity/coverage requirements are preserved.
The array cost is constrained by the current contract; the closure pointer path
shows repeated runtime benefit with lower native size and no added body checking.

## Owners, work bounds and source-to-native trace

- Aggregate helpers consume semantic `InitAction` records, canonical target
  types and declared field identities. Scalar/array-only helpers share the
  complete type/parameter-prefix key. Empty scalar holes use a full parameter
  list, so distinct supplied shapes cannot alias. Class-transfer helpers remain
  owned by their complete plan. A transfer followed by another field falls back
  to ordered destination construction, preserving initializer sequencing.
- Array eligibility is cached by immutable canonical type, including shared
  nested tails. The translation-unit index memoizes success and rejection,
  performs at most one classification per demanded array type, and exposes work
  and hit counts without additional analysis. Completed lookup is O(1) average.
  The 800/3,200 array workloads both classify one type, with 3,202/12,802 hits;
  the index retains one record per demanded canonical array type.
- Each helper lookup/build/call walks only its own fields, O(fields); each
  definition is emitted once. Scalar array members use one representation slot,
  pointer argument and copy each. Existing initialization expansion budget **8**
  is unchanged; large omitted tails keep the bounded loop/zero path. Class array
  elements and volatile elements keep ordinary ordered initialization.
- A closure occurrence/context owns one semantic class, selected call operator,
  conversion and pointer entry. The lexical default owner performs ordinary
  default checking, including fixed names at template definition. Closure prvalues
  initialize their destination directly. `ObjectUse::callable_entry` records an
  immediately invoked closure's ABI entry without replacing its selected language
  declaration or performing lookup in lowering.
- Each demanded entry consumes the same checked body/parameters and gets fresh
  function-local slots/blocks. There are at most **two body entries per closure**
  (object and pointer), plus the small conversion body only if explicitly called
  or addressed. Emission visits each required entry once. Large bodies may thus
  incur up to two body-sized outputs; no recursive inlining or growth transform
  is present. Deferred call edges are activated once; no call-operator emission
  is required solely because the pointer entry is used.
- For n=800/3,200 closures, both binaries report exactly n closures, n template
  body transitions, **2n+1 body checks**, **7n candidates**, and **2n+1 object-use
  records**. Parsing produces 89,725/358,525 nodes in both binaries. Final IR has
  24,005/96,005 instructions versus entry 25,605/102,405. Repeated specialization
  calls and second ABI entries do not repeat semantic body checks.

All records live in existing translation-unit arenas/flat indices; transient
argument vectors and function lowering state retain their existing release
boundaries. Labels and range-variable slots check their actual LowIR function
owner before reuse; no whole-program invalidation/reset is added. Source/AST
sharing, selected declarations and conversions flow directly into typed LowIR.
No fake syntax, source replay, text transport, rendered semantic key, process
cache or host compiler delegation was introduced.

The [trace source](../student.tests/pa20/trace96.cpp),
[inspection harness](../student.tests/pa20/trace96.py), and
[results](../student.tests/pa20/trace96.json) connect two demanded `visit<Bias>`
specializations and one repeated call through shared aggregate helper emission,
per-specialization closure statics, both body entries, distinct range storage,
typed LowIR validation and supplied native encoding. Telemetry/audit output is
byte-identical to ordinary output and the executable exits zero. PA20 owns
source-to-LowIR; the supplied native encoder is a validation boundary.

The 52 new personal controls cover scalar array member representation, omitted
and explicit clauses, strings, enums, nesting, large tails, volatile fields,
class member identity/sequencing, captureless defaults, lexical lookup,
copy/move/deletion, shared statics, both callable entries, reference/class ABI,
labels, switches, cleanup and range composition. They supplement the unchanged
144 course fixtures and the 136 inherited personal controls. This evidence is
for the completed array/callable groups; it does not certify the remaining stage
implementation or waive independent whole-stage review.
