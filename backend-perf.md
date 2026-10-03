# Backend performance plan

Improve the code generated at `-O3` until the self-built compiler matches or
beats the GCC-built compiler on the frozen compilation workload. The primary
target is **`user_time(self) / user_time(gcc-built) <= 1.00`**, confirmed in two
independent batches of at least three paired ABBA blocks. GCC builds the same
compiler source at `-O3`; both resulting executables compile the frozen TU at
`-O0`. This is the backend benchmark defined in `ralph:bench.md`.

Track executable code size, RSS, emitted object size and optimization cost as
secondary metrics. Report runtime parity and code-size parity separately. The
reference compiler can supply observations, but matching its backend ratio
does not satisfy the GCC target.

The campaign below is complete under its diminishing-returns stopping rule.
**GCC parity was not reached.** The final retained revision passes fresh
`make test-report` and `make inception` checks. The original plan and acceptance
criteria follow the outcome; detailed decisions are in
[backend-perf-experiments.md](backend-perf-experiments.md).

## Campaign outcome — 2026-10-03

Retained B02: at O1–O3, select shifts and reciprocal multiplication for 64-bit
constant integer quotients. Preserve signed truncation, negative divisors and
minimum values; zero and signed -1 retain hardware division. The implementation
uses existing wide-multiply instructions and division clobbers. Its normal
C++ fixture is
[310-constant-quotients.t](student.tests/pa33/driver/310-constant-quotients.t),
which checks boundary and pseudo-random inputs against runtime divisors. This
is optimization coverage, not a previously failing correctness regression.

| Final measurement | Result |
| --- | ---: |
| Self/GCC user-time ratio | **2.93358**, 95% interval [2.90377, 2.93592] |
| Starting self/GCC, freshly paired | 3.00000 |
| Within-block backend ratio change | 0.97786 [0.95296, 0.98572], a 2.21% improvement |
| Median self / GCC-built user times | 7.755 s / 2.625 s |
| Frontend / fixed starting seed, 20 blocks | **0.98600** [0.96995, 1.00461]; non-regression guard passes |
| Frontend / reference, 13 blocks | **0.97263** [0.95855, 0.99647]; parity retained |
| Frontend / GCC | 0.44805; no significant regression against the fresh starting lane |
| Matched-jemalloc self/GCC control | 2.86220 [2.75806, 2.99179] |
| Self executable text | 8,633,685 bytes; +1.16% versus starting self, 2.031 times GCC |
| Self runtime peak RSS / starting self | 0.99815; essentially unchanged |
| Frozen O0 object | 7,552,664 bytes; unchanged |

Ratios use paired block means, so they differ from ratios of absolute medians.
B02's initial direct self confirmation showed a 3.34% improvement. The final
batch's direct self ratio was 0.91498, but the GCC-built seed also moved; the
paired backend ratio above avoids crediting that entire change to code generation.
Keep runtime, code size and RSS claims separate: frontend RSS remains about
1.414 times the reference and its O0 object is 2.565 times the reference size.
This campaign did not achieve size or memory parity.

B03–B08 completed six distinct hypotheses without a retained significant gain.
B03 and B04 failed confidence/guard requirements, B06 failed full-workload
acceptance, and B05/B07/B08 failed kernel screens. B07/B08 were repeated after
correcting a stale build artifact; only clean-parent results determine their
outcomes. Best source before and after this window is the same B02 revision;
the direct cumulative check satisfies the less-than-5% rule. Stop reason:
**diminishing returns; GCC parity not reached**, with N=6, S=3%, C=5% unchanged.
The remaining ratio would require about 65.9% less self CPU time at a fixed
GCC runtime. Narrow constant quotients/remainders, register lifetimes and
context-aware accessor inlining are the strongest follow-ups.

Final validation on the retained source:

- `make test-report CXX=g++ CPPGM_HOST_CXX=g++`: **5454/5454**.
- `make inception CXX=g++ CPPGM_HOST_CXX=g++`: **421 matching objects** and
  byte-identical self/inception executables.
- PA32/PA33 contracts, roundtrips, MIR bounds and required debug checks pass.
- The added fixture passes all nine native routes with both seed and self.
  Prior PA1, PA19, conversion-member and overflow fixtures pass explicitly.
- Frozen workload objects match at O0–O3; overflow LowIR and objects also match
  at all four levels. The source and all 51 frozen headers match the manifest.

Final seed SHA-256 is
`85a04525b19e7f02264aa64f3a851a1c72a71018e01d3ba1aabb9caf6c9126ae`;
self/inception SHA-256 is
`e56b3c4da2224807190a57f631b461dd59a5becc2337ea67a4358b75e39e5309`.
Raw commands, samples, counters, source patches and hashes remain under ignored
`obj/backend-perf/`, with final evidence in `obj/backend-perf/final/`.

## Starting point

The frontend work is committed and pushed on `v4opt` as
`52456d749880e798d99cafc692725a75d04fb7f8`. Its measurements and correctness
record are in [frontend-perf.md](frontend-perf.md). Preserve immutable snapshots
of this revision's binaries and build inputs before starting backend edits.

| Existing three-block measurement | GCC-built compiler | Self-built compiler |
| --- | ---: | ---: |
| Median frozen-TU user time | 2.655 s | 8.275 s |
| Median wall time | 3.050 s | 8.765 s |
| Median peak RSS | 515.12 MiB | 510.72 MiB |
| Compiler file bytes | 5,167,408 | 12,808,128 |
| Executable `.text*` bytes | 4,248,326 | 8,534,693 |
| GNU `size` text column | 4,770,890 | 9,595,915 |

The median paired-block ratios are **3.042 user**, **2.815 wall**, and
**2.009 executable text size**. Paired ratios differ from ratios of separately
reported medians. Holding the GCC-built runtime fixed, closing a 3.042 gap
requires about **67% less self-built CPU time**. Re-measure the denominator
throughout; these times are not fixed thresholds.

The frontend optimizations made the self compiler about 29% faster, but the
GCC-built compiler improved more. This explains why the backend ratio widened.
The next campaign must measure generated-code quality as well as absolute
runtime, and must not improve the ratio by slowing the GCC-built compiler.

Baseline binary SHA-256:

- GCC-built: `049c8cabde7e1ad37cc21fa841cbaa26971e03b336c90296ed36074a3ec9a96b`.
- Self-built: `d25168c70621201c2e2147985455526355bd9be8c08b904c571845d11ff25d22`.

Reuse `~/cppgm-extended/benchmarks/self_compile/stable/semantic_overload.cpp`
and its `include/` closure. Verify all 51 headers against `PERF_EPOCH.json`.
The epoch is `9764b3835e3c6996b6b80803054f80e1cf50f98e`, source SHA-256 is
`ab00b2e1c3c7463baf9d8e1e7fc754b9cde2c18749568616062011f31e7daba2`, and header
closure SHA-256 is
`7c8a5445f33f04b314de98e6a099de4d75124b4bb032fc97ee5055e56d4827c8`.

### Initial evidence and hypotheses

A fresh diagnostic on CPU 4 while preparing this plan collected these
user-mode counters. Each event ran 100% of the measurement interval; these
single observations guide investigation and are not acceptance timings.

| Event | GCC-built | Self-built | Self/GCC-built |
| --- | ---: | ---: | ---: |
| Instructions | 12.124 billion | 36.565 billion | 3.016 |
| Cycles | 8.704 billion | 26.933 billion | 3.094 |
| Branches | 2.172 billion | 5.346 billion | 2.462 |
| Branch misses | 50.91 million | 56.64 million | 1.113 |

The instruction excess is a stronger initial lead than branch prediction.
A separate self profile recorded 1,622 samples with no lost samples. Exclusive
hotspots included string construction from `istreambuf_iterator` (6.9%),
`Ast::edge` (4.6%), `Cursor::is` (4.3%), `IdIndex::find` (4.2%), `IdIndex::put`
(3.0%) and `vector<IdIndex::Slot>::resize` (2.5%). Small spelling and token
accessors also remain visible. The profile does not yet prove which missed
optimization causes each cost: compare call sites and disassembly first.
Artifacts are in `obj/backend-perf-plan/`.

Relevant implementation observations:

- `dev/src/lowir/inline_policy.cpp` uses bounded bottom-up admission, call and
  body-size limits, and context-sensitive exception proofs. Its `level` argument
  currently does not change the admission policy.
- `dev/src/native/global_placement.cpp` reserves a register for a retained
  value's entire function, considers at most seven registers, and declines
  functions with handlers. This is safe but potentially expensive in hot loops.
- `dev/src/native/carry.cpp` removes some temporary reloads within bounded
  single-block windows. Cross-block/call liveness has separate conservative
  handling in `placement.cpp` and `parameter_flow.cpp`.
- `dev/src/lowir/optimizer.cpp` already runs inlining, scalar promotion, memory
  simplification, CSE and bounded loop transforms. Diagnose a missed proof or
  interaction before introducing another pass over the same work.

## Measurement contract

Use the canonical GCC 15.2.0 release configuration and the same C++ library,
host macros, target, optimization level, link order and test-runner setting.
Record every build command and executable hash. The existing canonical seed
uses glibc allocation while PA34 links jemalloc when available. Retain this
historical lane and run a matched-allocator control before attributing a gain
to code generation. Lock the control's configuration at setup; explain any
material divergence and do not obtain parity merely by switching allocators.

Pin measurements to an idle physical core and inspect its SMT sibling. CPU 4
and sibling 48 were used here; select them again only after checking load.
Run builds, tests and benchmarks serially. Warm each executable once and use
fresh output prefixes under `obj/backend-perf/`. Keep all samples, including
outliers and failed experiments. Run A/A calibration at setup and after host
conditions change. Do not flush caches or time profiler/telemetry runs as
acceptance results.

For an ABBA block, compute the mean of the two B user times divided by the mean
of the two A user times; report the median block ratio. Use one block only for
screening. Confirmation uses at least three blocks, with a second independent
batch for small gains and final parity. Extend to seven or ten blocks when
noise obscures the decision. Use block-level uncertainty, not individual runs
as if all observations were independent. An inconclusive measurement is not
an accepted improvement.
Record the interval method and random seed in the ledger; a block bootstrap
should resample complete ABBA blocks. Use at least six blocks when a small
gain or the frontend non-regression bound depends on that interval.

Example final or complete-self comparison, from the repository root:

```sh
BACKEND_REF="$HOME/cppgm-extended"
BACKEND_FROZEN="$BACKEND_REF/benchmarks/self_compile/stable"
BACKEND_OUT="$PWD/obj/backend-perf"
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
  --output-prefix "$BACKEND_OUT/E001-complete-self"
```

Use immutable copies in actual experiments. Also compare each candidate self
against the last accepted self directly. Track both runtimes if source changes
alter the GCC-built denominator. Retain a fixed source revision for kernels
and probe inputs so their algorithm and input data stay constant.

Seed/self objects for the same revision must match exactly. An optimization
may legitimately change objects relative to an older revision; those changes
require behavior and IR/ABI/debug validation, rather than an old-byte oracle.
GCC-generated and student-generated program objects need not match each other.

Use the **frozen compilation as the routine host/self target**. A candidate's
host-built and self-built executables compile that one TU, then their objects
are compared and their runtimes paired. This exercises the real compiler
without compiling the repository again as the timing workload. Once the
candidate self executable exists, repeat this short command freely; full
inception is reserved for the checkpoint schedule below. An old self executable
does not validate a changed producer: refresh the self generation at level 3,
or explicitly label a level-2 result as a scratch screen.

## Fast iteration without repeated inception

Use four levels. Most rejected ideas should finish at levels 1 or 2. Record
build and measurement wall time so setup overhead cannot silently dominate.

### 1. Reduced programs and cached LowIR

Build only the host-built producer and affected tools while editing:

```sh
make -C dev cppgm++ CXX=g++
# Add lowiropt or lowir2native to this build when the selected checks use it.
```

Extract a few representative hot operations from the profiles: accessor chains,
index probing/growth, iterator loops and values live around calls/backedges.
Compile unchanged C++ kernels with GCC and the candidate at `-O3`, then time
execution with runtime inputs and a checked result. Keep calls/memory patterns
representative; prevent dead-code elimination without making the entire hot
loop volatile. Include inputs that stress misses, collisions and cold paths.
Use normal C++/LowIR fixtures for correctness; benchmark automation is separate.

Cache pre-optimization LowIR for selected compiler TUs to replay backend ideas
without repeating their preprocessing, parsing and semantic work:

- Preserve the effective macros and hosted includes of their direct `-O3`
  compilations while emitting unoptimized LowIR. In particular, the driver
  normally adds `__OPTIMIZE__=1` at O3; a plain O0 capture can select different
  header code. Retain all PA34 definitions, generated headers and entry flags.
- Before using a cache, prove that direct `-O3 -c source.cpp` and candidate
  `-O3 -c captured.lowir` produce identical objects for the chosen TU. The PA32
  source/LowIR object boundary supports this workflow.
- Key caches by source/header hashes, effective flags, capture producer,
  LowIR format and metadata version. Regenerate for frontend/lowering or
  format changes. Validate direct/replayed output again after relevant edits.
- Bound this cache to selected TUs initially; do not spend the experiment
  budget serializing the whole compiler before demonstrating a useful saving.

Inspect LowIR, MIR and `objdump -drwC` for both generated objects. Collect
`perf stat` counters while executing the kernels, not just while compiling them.
Tiny-program speedups justify a larger trial, never the parity claim.

### 2. Replace a hot object in a scratch compiler

Snapshot the last accepted complete self object tree, its executable, generated
configuration and ordered link inputs under `obj/backend-perf/checkpoint/`.
The repository already has a scratch replacement path:

```sh
make -C pa34 probe-self-link CPPGM_HOST_CXX=g++ \
  SOURCE=../dev/src/support/id_index.cpp \
  PROBE_CXX=../dev/cppgm++ \
  PROBE_CANON_OBJ_ROOT=../obj/backend-perf/checkpoint/selfhost \
  PROBE_OBJ_ROOT_BASE=../obj/backend-perf/E001/probe
```

This compiles one source with the candidate producer at the PA34 O3 settings
and relinks it with the checkpoint's other objects. The example source is an
initial profiling candidate, not a required special case in compiler logic.
Compare the scratch executable with the checkpoint on the full frozen workload.
Use `probe-self-object` alone when only disassembly or a reduced link is needed.

Keep probe sources and included definitions identical to the checkpoint. The
changed backend is the producer of the replacement object, not a reason to
mix incompatible compiler data layouts. Skip this level when a header/ABI
change prevents safe replacement. Verify emitted workload bytes and smoke
tests before timing.

`probe-self-link` replaces **one** object per invocation; earlier replacements
do not accumulate. If profiles justify a small multi-object experiment, add a
manifest-based scratch relink using the exact recorded link order. Record each
object's producer/source hash and inspect which COMDAT/weak definition wins;
an untouched object can otherwise supply the supposedly improved function.

These mixed-producer executables are diagnostic artifacts only. Keep them out
of canonical PA34 directories and never use them as inception evidence.
Checkpoint objects remain immutable until a complete self build replaces the
checkpoint. A scratch speedup is provisional because inlining, layout and
register changes can behave differently across the complete program.

### 3. Complete self build for promising candidates

Only after a cheap screen supports the hypothesis, rebuild the seed and the
single self generation with the candidate. This measures the actual backend
ratio without building the next generation:

```sh
make -C dev cppgm++ CXX=g++
make -C pa34 cppgm++-self CPPGM_HOST_CXX=g++
```

Use isolated roots for retained experimental builds and record any embedded
path/configuration differences. Respect normal producer dependencies: a changed
producer requires all of its affected output objects to be regenerated. Do not
preserve stale canonical objects to shorten a self build.

Confirm the candidate against both the accepted self and its own GCC-built
seed, run focused seed/self byte checks and promote only after the acceptance
criteria below pass. Refresh the scratch checkpoint from this coherent build.
If several changes are tested together, record them as a combined experiment;
do not attribute the combined gain independently to each constituent change.
Use the frozen TU at O0 for each routine timing/equality screen; add O1–O3
equality when the experiment affects those optimization levels. This gives a
fast reproducibility check between full inception milestones.

### 4. Infrequent full validation

Run full inception after **five retained experiments** or **20% cumulative
self-runtime improvement** since the last inception checkpoint, whichever
comes first. Run it earlier for a seed/self mismatch or an ABI, EH or liveness
change whose focused checks leave a concrete correctness concern. Rejected
kernel/probe experiments do not trigger inception.

Always run inception and the full test report on the final retained source,
including when taking the early-stopping option. A full inception run is a
correctness milestone, not the inner timing loop. The starting commit already
passed it; no unchanged full rebuild is needed just to begin this campaign.

### Frontend and test-report checkpoints

Preserve the frontend gains while improving generated-code performance.
Track the original frontend metric
`user_time(GCC-built cppgm++ -O0) / user_time(g++ -O0)` as well as the direct
frontend/reference ratio used by the previous campaign. Freeze the starting
GCC-built seed too, so cumulative drift cannot hide in a moving baseline.

| Interval | Required checks |
| --- | --- |
| Every source experiment | Affected normal fixtures and relevant behavior/IR/MIR checks before timing. |
| Every candidate advancing to a complete-self measurement | One frozen-TU frontend ABBA screen against the last accepted GCC-built seed, plus a paired GCC frontend screen. If either suggests a regression, confirm with at least three blocks before retaining the change. |
| Every **three retained experiments or 60 minutes** of implementation, whichever comes first | Fresh `make test-report CXX=g++ CPPGM_HOST_CXX=g++`; three-block frontend comparisons against GCC, the fixed starting seed and the pinned reference. Run affected PA32/PA33 debug checks too. This checkpoint is independent of the less frequent inception schedule. |
| Final parity or early stop | Full test report, inception, frontend confirmation and final backend measurements on the same retained revision. |

The timer includes time spent on rejected experiments, so a difficult sequence
cannot defer the report indefinitely. Run checkpoints after the active short
measurement finishes and before beginning the next experiment; do not overlap
tests with timings.

Reject a confirmed frontend slowdown over the last accepted or fixed starting
seed. Treat changes below 1% as practically indistinguishable only when the
95% upper bound of the candidate/baseline ratio is at most 1.01; larger
uncertainty calls for more samples, not a larger permitted regression. Also
retain the achieved
**frontend/reference ratio <= 1.00** in confirmed checkpoint measurements.
The frontend/GCC ratio must show no significant regression against its freshly
paired baseline. Review all three comparisons together to distinguish compiler
changes from host/GCC timing drift, rather than comparing raw seconds from
different dates.

If a checkpoint fails, stop advancing the experiment queue, identify the
responsible change among the last three retained candidates, and fix or revert
it. Run the failed checks again and refresh the accepted checkpoint. Track
unconfirmed candidates separately from the last revision passing the full
report; a backend speedup cannot override a frontend or correctness regression.

## Experiment order and acceptance criteria

First collect matching GCC/self disassembly and hot-call counts. Reorder the
following queue when evidence changes; estimated gains are hypotheses.
All optimizations must apply to general IR/source patterns, not symbol names
or the frozen workload's answers.

| ID | Hypothesis and implementation area | Evidence required to advance |
| --- | --- | --- |
| B00 | Establish kernels, cache checks, scratch links and measurement calibration. | Unmodified-producer probe matches the checkpoint's output and timing within noise; direct/replayed LowIR objects match; record fast-loop duration and paired baseline. Infrastructure does not count toward the stall window. |
| B01 | Small accessor/iterator chains fail to inline or simplify effectively. Inspect `lowir/inline_policy.cpp`, `force_inline.cpp`, scalar cleanup and exception admission. | A reduced call chain loses the intended calls/temporary aggregates, preserves exceptions and address-taken definitions, and reduces executed instructions. Show a kernel/probe win before changing broader budgets; bounded work and code-growth gates still apply. |
| B02 | Repeated narrow extensions, address calculations, struct/vector access or constants survive selection. Inspect `native/selection.cpp`, `arithmetic.cpp`, integer encoding and LowIR scalar/CSE passes. | Identify redundant sequences against GCC and eliminate them with a general proof. Preserve signedness, overflow, flags and alias effects; measured instructions/cycles fall on at least two applicable inputs. |
| B03 | Conservative placement spills hot loop/call-crossing values. Inspect `placement.cpp`, `global_placement.cpp`, `parameter_flow.cpp` and `carry.cpp`. | Measured stack loads/stores and frame traffic fall on the identified paths. Progress from bounded live ranges/register reuse before a broad allocator rewrite. Cover joins, backedges, parallel phi copies, call clobbers and preserved registers; retain EH fallback until proven safe. |
| B04 | Inlining exposes memory/aggregate simplifications that remain unproven or run at the wrong point. Inspect `optimizer.cpp`, `slot_promotion.cpp`, `object_splitting.cpp`, `memory_values.cpp` and `local_cse.cpp`. | A replayed hot TU and normal fixture demonstrate eliminated redundant memory traffic with preserved volatile/atomic/escaped-object semantics. Avoid unbounded fixed-point iteration; show reduced dynamic work, not only smaller IR. |
| B05 | Iterator/container loops retain avoidable induction, bounds or loop-invariant work. Inspect `loop_simplify.cpp`, `loop_trip.cpp` and surrounding scalar passes. | Runtime kernels include zero/one/many iterations and calls/aliasing that block hoisting. Demonstrate fewer executed instructions on an actual profiled loop, no speculative trapping work and controlled growth. |
| B06 | Call setup, copies, branch layout or excess emitted code account for the residual gap. Inspect `native/calls.cpp`, `prefix_call.cpp`, `call_policy.cpp`, `control.cpp` and `object_demand.cpp`. | Select one mechanism per experiment from the new profile. Improve measured runtime or cache/branch behavior while preserving ABI, unwind/debug metadata and externally reachable definitions. A static size reduction alone is insufficient for a speed claim. |

For every optimization experiment, specify acceptance criteria **before editing**:

1. Name the hotspot, expected IR/machine change, required normal fixtures and
   the observable counter expected to fall. Record the producer revision and
   last accepted comparison point. A parameter sweep is one hypothesis with
   named variants, not several independent discoveries.
2. A cheap screen must show the intended mechanism and no correctness failure.
   Require at least a 2% kernel/probe timing gain beyond calibrated noise to
   promote on timing alone. A smaller screen may advance if measured coverage
   predicts a worthwhile complete-self gain; state that prediction first.
3. Retain a performance change only when the complete-self comparison gives
   at least **1% median user-time improvement** over the last accepted self,
   with a block-level 95% interval below 1.00 for the new/old ratio. Confirm
   gains below 3% in a second independent batch. If sampling up to ten blocks
   cannot resolve the gain, mark it inconclusive and revert the performance
   patch. Do not keep accumulating unconfirmed tweaks.
4. Also require an improvement in the self/GCC-built ratio and compliance with
   the frontend non-regression checks and checkpoint schedule above. Use
   fixed-source controls if source changes move both runtimes. Re-profile when
   a proxy improves but the full workload does not; reject the proxy-only win.
5. Default resource limits: at most 3% growth in compiler executable `.text*`,
   5% in runtime peak RSS, and 10% in producer optimization time/peak RSS on
   the selected TU set. A larger tradeoff needs a revised, explicit numerical
   criterion recorded before its confirmation run and a demonstrated runtime
   benefit. Record output sections separately from compiler executable size.
6. All focused behavior, IR/MIR bounds, object roundtrips and relevant debug
   checks must pass. Add a reduced normal fixture for an actual discovered
   correctness defect. Label optimization-boundary coverage separately; do
   not imply that a new passing fixture demonstrates a pre-existing bug.

Correctness fixes can be retained without a speed gain, but receive no
performance credit and remain visible in the experiment history.

### Experiment ledger

Maintain `backend-perf-experiments.md` during implementation. Give each
completed hypothesis an immutable ID and link its source patch/commit and raw
artifacts under `obj/backend-perf/<ID>/`. Do not commit generated objects/logs.
Start each entry with the proposed criteria, then append observations and the
decision; keep rejected experiments in the ledger.

| Field | Required record |
| --- | --- |
| Identity | ID, hypothesis, scope, parent accepted revision, patch/commit hash, source/header/producer/binary hashes. |
| Predeclared test | Expected code change and counters; selected fixtures/kernels; time, space and compile-cost limits. |
| Reproduction | Exact build/link/run commands, toolchain/library/allocator, CPU/sibling/load, cache key, replaced object manifest, warmups and ABBA counts. |
| Measurements | All block ratios, medians and interval; absolute self/seed times; self/GCC ratio; counters; executable/section/object sizes; RSS; build/optimizer time. Mark kernel, scratch and complete-self results distinctly. |
| Validation | Fixture results, O0–O3 seed/self output checks, direct/LowIR replay checks, debug/MIR checks; frontend/GCC and frontend/reference results; most recent full test-report and inception checkpoints. |
| Decision | Rejected, inconclusive, provisional, retained, or correctness-only; rationale; best ratio after this experiment; stall-window contribution. |

Keep a compact summary table above the detailed entries with one row per
experiment. A retained change that later fails full validation loses its
performance credit until fixed and remeasured.

## Early stopping for diminishing returns

Enable this option by default for the implementation campaign. Record these
parameters at B00; change them only explicitly, not to turn a disappointing
result into a success:

| Parameter | Default |
| --- | ---: |
| Rolling window of completed optimization hypotheses, `N` | **6** |
| Significant individual complete-self CPU-time improvement, `S` | **3%** |
| Minimum cumulative best-self improvement across the window, `C` | **5%** |

Stopping before parity becomes eligible when **all** of these hold:

1. The best confirmed self/GCC-built ratio is still above 1.00.
2. The last `N = 6` completed optimization hypotheses contain no confirmed
   individual improvement of at least `S = 3%` over the then-best accepted self.
3. The best retained self at the end of that window is less than `C = 5%` faster
   than the best retained self immediately before the window. Confirm this
   cumulative comparison directly with paired timings; do not multiply noisy
   estimates from separate dates.

Rejected performance ideas and correctness-failing ideas count as zero gain.
A correctness-only change also contributes zero. Combine variants of the same
hypothesis into one entry. Setup, repeated measurements and renamed/refactored
versions of an unchanged idea do not add experiments or reset the window.
Measurements invalidated by host interference remain incomplete; resolve them
with bounded retries or report a measurement limitation rather than claiming
evidence of diminishing returns.

This rule allows several smaller wins to continue: six 1–2% gains can exceed
the 5% cumulative threshold even when none individually reaches 3%.

When the trigger fires, do one bounded review of the latest profile and the
six ledger entries, verify the cumulative comparison, then **stop optimization
and finish the final validation gates** on the best retained revision. Record
the stop as **“diminishing returns; GCC parity not reached”**, with the achieved
ratio, remaining gap, stopping parameters, contributing experiment IDs and
highest-value untried hypotheses. Do not label this outcome parity or bypass
correctness because progress stalled. The user can later resume with a revised
budget, threshold or hypothesis set.

## Final validation and delivery

Before committing the final backend implementation, whether at parity or at an
early stop:

1. Run all added personal fixtures explicitly and the affected PA24–PA33
   contracts. PA32 requires LowIR/object roundtrips and debug checks; PA33
   requires behavior, MIR bounds and debug checks. Read the owning handouts
   before each implementation change. Preserve fixtures and reference outputs.
2. Run these root targets on the final retained source with the canonical
   configuration and normal PA34 dependency/resource tracking:

   ```sh
   make test-report CXX=g++ CPPGM_HOST_CXX=g++
   make inception CXX=g++ CPPGM_HOST_CXX=g++
   ```

   Require a fresh complete test report and matching self/inception objects
   and executables. Exclude every scratch mixed-producer object from this run.
   Run PA32/PA33 debug suites and relevant ABI/unwind/link inspections. The
   five pre-existing PA8 textual debug differences documented in the frontend
   report are a baseline observation, not permission for new failures.
3. Compare final seed/self frozen-TU objects at O0/O1/O2/O3. Repeat the normal
   conversion-member and overflow fixtures, including the overflow LowIR/object
   reproducibility checks. Inception alone does not cover the frozen workload.
4. After builds/tests finish, measure final self/GCC-built and new/starting-self
   ratios, matched-runtime controls, RSS and size breakdowns. Require two
   independent three-block-or-longer batches at or below 1.00 to declare runtime
   parity; extend sampling when uncertainty crosses the threshold. An early
   stop reports the measured ratio honestly instead of applying that success gate.
   Repeat the frontend/GCC, fixed-seed and reference comparisons and require
   the frontend non-regression criteria even when stopping early.
5. Update this plan and the ledger with the achieved result, retained/rejected
   changes, validation commands/results, remaining tradeoffs and stopping
   reason. Commit implementation, normal fixtures and documentation; retain
   raw evidence under ignored `obj/`. Publish the final branch only after
   these checks pass. If validation fails, fix/revert and revalidate the best
   retained candidate before delivery.
