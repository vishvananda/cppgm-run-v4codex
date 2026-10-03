# Backend performance plan: demand and hot-path optimization

Revised 2026-10-03 after the output-demand audit. This implementation campaign
started at `00a29292d2bf69678c6eb7013ca4c73ea627fe4d` on `v4opt`.
Implementation and final validation completed 2026-10-03. Outcome:
**diminishing returns; GCC parity not reached**. The six-hypothesis stopping
window and all decisions are recorded in the ledger.

Retained changes to demand, lowering, constant returns, call-loop slot promotion
and register lifetimes reduce complete-self compile time by **15.2%**. Fresh paired
self/GCC-built runtime improves **2.919x -> 2.636x**; frontend/reference is
**0.915x**, with the frontend guards passing. Frozen object size falls **46.2%
at O0** and **34.7% at O3**. Reference RSS parity remains unmet at **1.350x**.
Final `make test-report` passes **5454/5454**, and `make inception` passes
**423 object comparisons and identical executables**. Normal personal fixtures,
required debug/IR/MIR/link checks and frozen O0–O3 byte comparisons pass.
See the ledger's final outcome for intervals, allocator/counter controls,
remaining size/RSS gaps, the export regression repair and untried hypotheses.

B00–B08 are completed historical experiments in
[backend-perf-experiments.md](backend-perf-experiments.md); use new S-series IDs.
Their early stop exhausted that experiment sequence, not the structural
opportunities identified below.

The order is now **correct emission demand, avoid unnecessary lowering, recover
accessor inlining and subsequent dataflow optimization, then improve register
lifetimes**. Re-profile between these stages. Small instruction-selection and
layout changes follow evidence about the remaining gap.

## Targets and attribution

The primary backend target remains **GCC parity**:
`user_time(self-built compiler) / user_time(GCC-built compiler) <= 1.00`.
Both executables implement the same compiler revision, are built at O3, and
compile the frozen workload at O0. Confirm parity in two independent batches
of at least three paired ABBA blocks, extending sampling to resolve uncertainty.
The reference compiler is a frontend comparison, not the backend target.

Preserve frontend/reference parity and the frontend/GCC ratio already achieved.
Track RSS, compiler executable size and emitted object size as separate goals;
report progress against GCC and the reference without calling runtime parity
size or memory parity. Demand cleanup should reduce actual compiler work as
well as output. Do not improve the backend ratio by slowing its denominator.

Separate three effects in every experiment:

1. **Work performed by the compiler:** unnecessary instantiation, lowering,
   optimization and emission while compiling the frozen source. Removing this
   can speed up both GCC-built and self-built compilers.
2. **Cold output retained in the compiler executable:** unnecessary bodies and
   data can affect size, loading and caches without adding much executed work.
3. **Instructions executed on hot paths:** missed inlining, redundant memory
   operations, spills and weak instruction selection drive the backend ratio.

A demand fix may improve time/RSS/output while leaving the backend ratio flat
or changing its workload mix. Evaluate it under the demand criteria below;
require a backend-ratio improvement for changes claimed as code-generation wins.
Keep absolute self time, seed time and fixed-source kernel controls visible.

## Evidence and baseline

The previous retained change, B02, implements 64-bit constant quotients. Its
final paired self/GCC ratio was **2.93358** [2.90377, 2.93592], with median
self/seed user times of 7.755/2.625 seconds. Frontend/reference was **0.97263**
[0.95855, 0.99647]; frontend/GCC was 0.44805. Frontend RSS remained **1.4142x**
the reference. These are historical measurements, not fixed timing thresholds.
The retained revision passed `make test-report` (5454/5454), `make inception`
(421 matching objects and identical executables), required PA32/PA33 checks,
and frozen O0–O3 seed/self byte comparisons.

### Unnecessary emission survives O3

The diagnostic input is:

```cpp
#include <regex>
int answer() { return 42; }
```

| Optimization | Our distinct function bodies | GCC | Reference | Our object bytes |
| --- | ---: | ---: | ---: | ---: |
| O0 | 231 | 1 | 1 | 203,504 |
| O3 | 60 | 1 | 1 | 69,616 |

Distinct bodies are deduplicated by ELF section, address and nonzero size;
constructor/destructor aliases are not counted as separate code. Some unused
base-entry constructors/destructors have `object_root=yes`. This proves
unnecessary emission on this reducer; it does not prove that every extra
function in the complete workload is dead.

| Frozen workload | Our bodies | GCC bodies | Our object `.text*` bytes | GCC object `.text*` bytes |
| --- | ---: | ---: | ---: | ---: |
| O0 | 8,922 | 3,862 | 1,227,419 | 602,123 |
| O1 | 2,765 | 384 | 1,301,230 | 418,966 |
| O2 | 2,765 | 440 | 1,301,049 | 382,735 |
| O3 | 2,765 | 403 | 1,301,049 | 377,704 |

The reference O0 object has 4,065 bodies, 687,749 executable text bytes and
2,944,688 total bytes; ours has 7,552,664 total bytes. Separate native code,
data, unwind tables, relocations and symbol/string tables before attributing
file growth. More inlining can reduce body count while increasing total text.
Anonymous-namespace/lambda mangling differences also make raw name subtraction
an unreliable dead-code detector.

### Executed work remains the larger runtime lead

A fresh diagnostic, with identical frozen output from all three executables:

| Counter or size | GCC-built compiler | Self-built compiler | Self with scratch section GC |
| --- | ---: | ---: | ---: |
| Retired instructions | 12.124 billion | 37.044 billion | 37.042 billion |
| Cycles | 8.903 billion | 25.977 billion | 25.722 billion |
| Branch misses | 50.37 million | 56.77 million | 56.36 million |
| Executable `.text*` bytes | 4,251,910 | 8,633,685 | 8,523,937 |

These single counter observations guide hypotheses; they are not timing
acceptance evidence. With the current section layout, `--gc-sections` removes
only 1.27% of self text and barely changes executed instructions. This does not
bound all removable code: strong/internal bodies can share a section. It does
show that a linker flag alone is insufficient.

`IdIndex::find` is a concrete hot-path reducer: our 240-byte body has three
call sites to `vector::operator[]`, saves/reloads the probe index and reconstructs
a key from smaller loads. GCC's 126-byte body contains no calls and retains
loop state in registers. The latest profile also identifies string-input
construction, `Ast::edge`, `IdIndex::put`, token cursors and node projection.

Relevant code observations:

- [lowering/symbols.cpp](dev/src/lowering/symbols.cpp) assigns roots to base
  entries and retained members. Trace why a body is demanded separately from
  why it has a particular ABI entry.
- [native/object_demand.cpp](dev/src/native/object_demand.cpp) roots every
  defined global and non-inline weak body. Optimized support pruning in
  [lowir/inline_policy.cpp](dev/src/lowir/inline_policy.cpp) uses different
  function-root rules and also roots all defined globals.
- [native/driver.cpp](dev/src/native/driver.cpp) constructs its workspace and
  emits data before object-demand filtering; the filter applies to hosted
  object emission. Audit the other source/LowIR/native routes explicitly.
- [lowir/optimizer.cpp](dev/src/lowir/optimizer.cpp) uses an immutable summary
  of loops containing calls. Such functions lose scalar-slot promotion,
  memory-value simplification and several cross-block optimizations. A missed
  accessor inline can therefore prevent several later improvements.
- [native/global_placement.cpp](dev/src/native/global_placement.cpp) retains
  values in registers for entire functions and declines functions with EH
  handlers. This is a concrete source of conservative stack placement.

Audit commands, inventories, reducers and counters are under ignored
`obj/backend-demand-audit/`. Prior measurements are under `obj/backend-perf/`.
Recreate missing artifacts from the recorded procedures; commit summaries and
normal fixtures, not generated data. Preserve the original frontend campaign
record in [frontend-perf.md](frontend-perf.md).

## Measurement contract and setup — S00

Freeze the starting seed/self binaries and a coherent self object tree, ordered
link inputs and generated configuration. Record commands, source/header hashes,
producer/binary hashes, GCC version, host library and allocation configuration.
Starting seed SHA-256:
`85a04525b19e7f02264aa64f3a851a1c72a71018e01d3ba1aabb9caf6c9126ae`;
self SHA-256:
`e56b3c4da2224807190a57f631b461dd59a5becc2337ea67a4358b75e39e5309`.
Check these against actual binaries before reusing old measurements.

Use `~/cppgm-extended/benchmarks/self_compile/stable/semantic_overload.cpp`
with its 51-header `include/` closure. Verify `PERF_EPOCH.json`: epoch
`9764b3835e3c6996b6b80803054f80e1cf50f98e`, source SHA-256
`ab00b2e1c3c7463baf9d8e1e7fc754b9cde2c18749568616062011f31e7daba2`,
header closure SHA-256
`7c8a5445f33f04b314de98e6a099de4d75124b4bb032fc97ee5055e56d4827c8`.
Keep canonical GCC 15.2.0 build flags, O3, link order, runner configuration and
host macros fixed. The canonical seed uses glibc and self links jemalloc;
retain this lane and repeat a matched-jemalloc control before final attribution.

Pin serial measurements to an idle physical core; inspect its SMT sibling
(CPU 4 / sibling 48 previously). Do not overlap builds, tests or profiling with
timing. Warm binaries, calibrate A/A at setup and after host conditions change,
and preserve all samples. No post-hoc outlier removal or cache flushing.

Use the mean B user time divided by mean A user time within each ABBA block,
then report the median block ratio. Use a 10,000-resample bootstrap of whole
blocks, seed 340033, for the 95% interval. One block screens; at least three
confirm; small gains require two independent batches and at least six blocks.
Extend backend confirmation to at most ten blocks and frontend non-regression
confirmation to at most twenty, declared before measurement. At persistent
uncertainty, park the candidate; do not change the bound or label noise a loss.
Check host load/A/A calibration before spending the extension budget.

Record both old/new self times and freshly paired old/new self/GCC ratios.
Collect counters separately from timing, alternating baseline/candidate order:
instructions, cycles, branches and misses first; supported cache/TLB event
sets separately. Require adequate event running percentages, and report
absolute counts alongside per-instruction rates. Compare hot-path disassembly
and call counts before treating a counter change as an explanation.

S00 exits when the immutable baseline, inventory, calibration and scratch-link
control reproduce. Add diagnostic-only counts for root reasons, reachable
bodies, materialized/lowered IR, optimization admissions and emitted sections
only where needed to distinguish hypotheses. Instrumentation runs are not
acceptance timings. Setup does not count toward early stopping.

## Ordered structural experiments

Each row is a hypothesis family. Give distinct mechanisms suffix IDs and
predeclare their acceptance criteria before editing; parameter variants remain
one experiment. Keep required semantic validation separate from runtime emission.

| Family | Work and expected mechanism | Criteria to advance beyond the cheap screen |
| --- | --- | --- |
| **S01: correct emission roots** | Trace the unused-header reducer from semantic demand through ABI entries, LowIR roots, globals/aliases and native emission. Remove false roots and edges; reconcile O0/optimized root policies and audit hosted/native routes. | Each change removes its demonstrated false roots at O0–O3 while required exports/uses remain. The family exits when the regex reducer contains only its required function body, explaining any genuinely required initialization. Prove root provenance for remaining unexpected bodies. No weaker diagnostics or required-output loss. |
| **S02: avoid unnecessary lowering and retained data** | Move the proven runtime-demand boundary early enough to avoid materializing dead function IR, data, vtables and downstream work. Share typed demand facts across direct and LowIR replay paths; retain required semantic/constant evaluation. | Show fewer lowered bodies/instructions or retained bytes on the frozen TU and selected header-heavy TUs, with matching direct/replayed objects and preserved semantics. Retain through the demand acceptance lane below, not merely a smaller final symbol table. |
| **S03: simplify before accessor admission** | Revisit B04's constant-return/dead-branch folding, then analyze bounded accessor chains and exception proofs. Remove unreachable expensive paths before costing callees; avoid a blanket increase in inline limits. | The reduced accessor case and actual `IdIndex::find` lose the unintended subscript calls; dynamic calls/instructions fall without uncontrolled text growth. Show a kernel or coherent scratch win, then confirm a complete-self win. |
| **S04: recover dataflow around calls** | Measure functions rejected by the call-in-loop gate after S03. Replace whole-function exclusions with bounded region/local proofs and call-effect/escape information. Recompute admission only when a measured simplification justifies its bounded cost. | A representative loop with a remaining call gains slot promotion or load reuse on proven-safe state. Fewer executed loads/stores and instructions; aliasing, side effects, volatile/atomic accesses, throwing calls and backedges still block unsafe transforms. Complete-self confirmation required. |
| **S05: improve register lifetimes** | On the new profile, replace whole-function reservations with bounded lifetimes/interference where profitable. Start with non-EH loops and calls; handle EH only in a separate proven extension. | Measured spills/reloads and stack traffic fall on real hot paths. Validate phi transfers, joins, backedges, parameter carriers, call clobbers, preserved registers and unwind/debug behavior. A spare-register count alone is insufficient. |
| **S06: remaining hot code** | Re-profile after the structural work. Investigate narrow division/remainder, split loads, address calculations, string/iterator loops, instruction selection or layout only when they explain material remaining cycles. | One concrete mechanism per experiment, reduced normal coverage, dynamic evidence and complete-self acceptance. No generic peephole queue without measured coverage. |

S01 and S02 may overlap in implementation; record a combined experiment if their
contributions cannot be isolated. A late emission-only fix is a useful first
step, but does not close S02 if the discarded bodies were still fully lowered.
For S01, record root class and a predecessor chain to each unexpectedly emitted
body. Do not simply clear all `object_root` flags or discard all weak symbols.
Required roots include externally visible non-inline definitions, explicit instantiation
definitions, required aliases/used sections, live address-taking, initialization,
vtables/RTTI and ABI constructor/destructor entries reachable from real uses.
Preserve required checking of unused non-template definitions and the language's
rules for template instantiation; emission liveness is not semantic validity.

Use normal `.t` C++/LowIR fixtures and existing sidecars under
`student.tests/paN/`, run explicitly. Reduce the hosted-header observation to
small self-contained cases for future course inclusion; keep the real header
case as an integration measurement. Pair dead-demand cases with live controls:
external calls from another TU, function pointers, virtual dispatch/RTTI,
base/complete/deleting destructor variants, static/TLS initialization, aliases
and explicit instantiation. Include required rejection cases so deferred
emission cannot hide diagnostics. Prefer existing object/MIR inspection and
link harnesses over new Python-only regressions; benchmark scripts are separate.

B04 is promising evidence, not a retained fix: it removed the three accessor
calls and showed a provisional 5.58% self gain, but frontend confidence remained
inconclusive. Retest its general mechanism on the new parent with the declared
sampling budget. Do not report the old result as a proven slowdown, a fresh
gain or six different hypotheses through repeated parameter changes.

## Fast iteration and coherent builds

Most ideas should be decided without full inception:

1. **Host producer plus reduced cases.** Build `make -C dev cppgm++ CXX=g++`
   and only the affected tools. Run normal fixtures and inspect O0–O3 output.
   For S01/S02, source-to-object runs are essential: cached LowIR skips the
   very work being changed. Measure phase work/RSS separately from timing.
2. **Cached LowIR, kernels and scratch links.** For backend-only changes,
   replay selected hot TUs using caches keyed by source/include hashes,
   effective macros/flags, producer and IR metadata version. Preserve O3's
   preprocessing configuration even when capturing unoptimized LowIR. Prove
   direct O3 source and replayed LowIR objects equal before using the cache;
   regenerate after semantic/lowering/metadata changes. Time runtime-input
   kernels with checked results, including cold paths and misses.
3. **One complete self generation for a promising candidate.** Rebuild the
   seed and `make -C pa34 cppgm++-self CPPGM_HOST_CXX=g++`. Use isolated
   experiment roots where appropriate. Compare that seed and self on the
   frozen TU; this is the routine performance/reproducibility workload.
   Do not build the inception generation for each candidate.
4. **Full inception only at checkpoints and completion.** The cadence below
   remains independent of the much shorter frozen compilation loop.

For a backend-only hot-object replacement, reuse the supported scratch path:

```sh
make -C pa34 probe-self-link CPPGM_HOST_CXX=g++ \
  SOURCE=../dev/src/support/id_index.cpp \
  PROBE_CXX=../dev/cppgm++ \
  PROBE_CANON_OBJ_ROOT=../obj/backend-perf/structural/checkpoint/selfhost \
  PROBE_OBJ_ROOT_BASE=../obj/backend-perf/structural/S03/probe
```

Populate that checkpoint from a coherent complete self tree first. Each probe
replaces one object; replacements do not accumulate. For a justified small
multi-object probe, preserve an exact ordered link manifest and inspect which
COMDAT definition wins. Keep source/layout/configuration compatible, and keep
mixed-producer artifacts outside canonical PA34 roots. They screen mechanisms
but never establish inception or complete-self acceptance. A broad emission
policy change needs a coherent complete self rebuild before final conclusions.

After any producer change, respect PA34 invalidation and rebuild affected
outputs. Hash the actual producer and object inputs. Restore rejected source
with fresh timestamps or rebuild its objects explicitly; copying an old mtime
previously left stale machine code in a host object. Never reuse that shortcut.

Routine frozen measurement, using immutable candidate snapshots in real runs:

```sh
BACKEND_REF="$HOME/cppgm-extended"
BACKEND_FROZEN="$BACKEND_REF/benchmarks/self_compile/stable"
BACKEND_OUT="$PWD/obj/backend-perf/structural"
BACKEND_CPU=4
mkdir -p "$BACKEND_OUT"
taskset -c "$BACKEND_CPU" python3 "$BACKEND_REF/scripts/run_ab_compile_benchmark.py" \
  --repo-root "$BACKEND_REF" \
  --compiler-a "$PWD/dev/cppgm++" \
  --compiler-b "$PWD/pa34/cppgm++-self" \
  --source "$BACKEND_FROZEN/semantic_overload.cpp" \
  --include "$BACKEND_FROZEN/include" \
  --compiler-arg=-O0 --compiler-arg=-std=gnu++11 \
  --abba-blocks 3 --output-mode exact --timeout-sec 1800 \
  --output-prefix "$BACKEND_OUT/S03-complete-self"
```

Same-revision seed/self outputs must be byte-identical. Every retained demand
or optimizer change also gets frozen O0–O3 equality checks. Legitimate changes
between revisions use behavior/ABI/debug checks and determinism within each
revision, not old object bytes as an oracle. GCC/reference object bytes are
not an equality requirement. Inception does not replace the frozen checks.

## Acceptance criteria and experiment records

Before editing, record the parent revision, mechanism, affected scope, fixtures,
expected IR/machine/counter changes, screen and confirmation budgets, resource
limits and acceptance lane in [backend-perf-experiments.md](backend-perf-experiments.md).

| Lane | Retention criteria, in addition to all correctness/frontend guards |
| --- | --- |
| Demand/output quality | Close the reduced unnecessary-emission defect, preserve live controls, and reproduce the intended reduction on the frozen or predeclared representative TUs. Require at least a 10% reduction in a predeclared whole-TU work/space metric (lowered IR, emitted text/object bytes or peak RSS), or at least a 1% confirmed compile-time gain. Require self-runtime non-regression (95% upper bound <=1.01). A root-correctness fix with no material full-workload gain can be retained separately as correctness/quality only, with no runtime credit. |
| Generated-code performance | Require at least 1% median complete-self user-time improvement over the accepted parent, with 95% interval below 1.00, and an improved freshly paired self/GCC ratio. Confirm gains below 3% in a second batch. Counters/disassembly must support the proposed mechanism. Kernel/probe-only gains do not qualify. |
| Correctness | Necessary semantic/ABI fixes need no speed gain. Add a minimal normal regression and report the performance effects separately. Resolve resulting performance regressions before claiming the campaign's performance gates pass. |

A timing-only cheap screen needs at least a 2% kernel/probe win beyond calibrated
noise. A deterministic removal of false demand can advance without that timing
threshold. A smaller timing screen can advance only with a predeclared coverage
argument for a useful complete-self effect. Avoid mutually masking unrelated
patches; label coupled transformations as a combined experiment and assess
individual and combined effects where practical.

Default limits versus both the accepted parent and campaign start are +3%
compiler executable `.text*`, +5% runtime peak RSS and +10% producer optimization
time/RSS on the selected TU set. Record all O0–O3 emitted sections separately.
Demand fixes should reduce output without unexplained section growth elsewhere.
A larger tradeoff needs a numerical criterion recorded before confirmation and
a measured benefit; frontend/runtime correctness gates still apply. Report
remaining distance to GCC/reference size and memory parity rather than treating
these growth limits as the targets.

Each ledger entry records all raw samples and commands, patches/hashes,
root/body/IR counts, static sections, dynamic counters, absolute seed/self
runtimes, ratios/intervals, RSS, build/optimization cost and validation results.
Distinguish diagnostic, kernel, scratch and coherent complete-self evidence.
Statuses are planned, running, provisional, retained, rejected, inconclusive or
correctness/quality-only. Keep historical rejected entries. Record both the best
runtime candidate and the latest candidate passing every required gate.

## Frontend safeguards and validation cadence

Preserve `user_time(GCC-built cppgm++ -O0) / user_time(g++ -O0)`, the direct
frontend/reference ratio and comparison to the fixed campaign-start seed.
Compare with the accepted parent too; cumulative regressions cannot hide behind
moving baselines. The frontend non-regression margin is 1%: require the 95%
upper bound of candidate/baseline user time <=1.01. Frontend/reference must
remain <=1.00 in confirmed measurements, and freshly paired frontend/GCC ratios
must show no significant regression. More uncertainty means more bounded
sampling, not a looser threshold or evidence that a mechanism failed.

| When | Checks |
| --- | --- |
| Every source experiment | Reduced normal fixtures, live/dead controls and affected assignment behavior/IR/MIR/object checks before timing. |
| Every candidate entering complete-self evaluation | Frontend ABBA screen against the accepted seed and GCC; confirm any suspected regression before retention. For demand changes, also inventory frozen O0–O3 output and compilation work. |
| Every three retained experiments or 60 minutes of implementation, whichever is first | `make test-report CXX=g++ CPPGM_HOST_CXX=g++`; frontend comparisons against GCC, fixed seed and pinned reference; relevant PA32/PA33 debug checks. Start with three blocks, then apply the declared confirmation budget if needed. |
| Every five retained experiments or 20% cumulative self-time improvement since last inception | Full `make inception CXX=g++ CPPGM_HOST_CXX=g++`. Run sooner for a seed/self mismatch or a concrete unresolved ABI/EH/liveness concern. |
| End of the demand phase, before broad hot-path changes | Fresh full report and frontend guards, coherent self build, O0–O3 frozen equality, symbol/link/live-root controls. Existing inception cadence still applies. |
| Final parity or early stop | Fresh full report, full inception, all added normal fixtures and required debug/IR/MIR/link checks, final O0–O3 frozen equality and frontend/backend confirmation on one retained revision. |

The 60-minute timer includes rejected experiments. Finish an active short
measurement before a due checkpoint, and do not start another experiment until
it passes. Rejected screens do not themselves require full inception. If a
checkpoint fails, stop promoting candidates, fix/revert the responsible change,
rerun failed checks and restore a validated checkpoint.

Read [Testing and references](TESTING_AND_REFERENCES.md) and each owning handout
before changing implementation. Use required through reports and debug/link
checks; PA24/PA32 design-specific regression suites are not exit criteria.
Existing tests/references cannot be weakened to accommodate a change.

## Early stopping with structural coverage

Keep the user-requested diminishing-returns option: **N=6** completed distinct
optimization hypotheses, significant individual gain **S=3%**, minimum cumulative
best-self improvement **C=5%**. Start a new window for this campaign; do not
carry B03–B08 into it. Subvariants and repeated measurements are one hypothesis.

Before applying the performance stop, complete the S01 demand-defect work and
record an evidence-based assessment of S02, S03, S04 and S05. Each assessment
needs a representative test or a concrete reason the mechanism does not apply;
listing it as future work is insufficient. Where inlining unlocks dataflow,
include a bounded combined trial rather than rejecting both on isolated screens.
Infrastructure work and mandatory demand/correctness repairs do not consume the
six-experiment performance window.

Early stopping becomes eligible when coverage is satisfied, the confirmed
self/GCC ratio remains above 1.00, none of the last six completed performance
hypotheses produced a retained >=3% self-time improvement, and the best retained
self improved <5% across that window. Confirm the cumulative change directly
with paired timings; do not multiply historical estimates. Rejected valid
experiments count as zero. Confidence-limited results remain marked inconclusive;
invalid measurements do not count as evidence of diminishing returns.

Review the profile, demand inventory and six decisions once before stopping.
An unexplained major demand root or an untested structural family means this
coverage gate has not passed. Do not keep retrying the same variant to manufacture
six failures. If measurements or correctness block progress, report that actual
limitation instead of claiming a performance plateau. A valid early stop still
requires all final correctness and frontend gates on the best retained revision.

## Final delivery

Run all new personal fixtures explicitly, affected contract/through reports,
PA32 LowIR/object roundtrips, PA33 behavior/MIR bounds and required debug checks.
Repeat the branch's conversion-member and overflow-emission regressions,
including overflow LowIR/object comparisons at O0–O3, and constant-quotient
coverage. Then require fresh success on the same retained source:

```sh
make test-report CXX=g++ CPPGM_HOST_CXX=g++
make inception CXX=g++ CPPGM_HOST_CXX=g++
```

Use normal producer dependencies and resource controls, excluding scratch
mixed-producer objects. Compare final seed/self frozen objects at O0–O3.
After builds/tests finish, rerun backend, direct self/start, frontend/GCC,
frontend/reference and fixed-seed confirmations plus the matched-allocator
control. Report remaining false demand, emitted body/text/object counts, compiler
size, RSS and dynamic instructions alongside runtime ratios.

Close with either confirmed GCC parity or the explicit outcome
“diminishing returns; GCC parity not reached,” identifying the contributing
experiments, remaining gap and untried hypotheses. Update this plan and the
ledger without rewriting the historical results. Commit implementation, normal
fixtures and documentation, and push the branch only after final gates pass.
These implementation gates are complete for the final retained revision; the
ledger records the fresh evidence and the explicit early-stop decision.
