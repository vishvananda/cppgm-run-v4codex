# Backend performance experiments

B00–B08 below are the completed first campaign. Their results and original
criteria remain historical records. The revised active plan is
[backend-perf.md](backend-perf.md); its S-series campaign starts at
`00a29292d2bf69678c6eb7013ca4c73ea627fe4d`. Planning that campaign does not
change the implementation or reopen old experiment results. Planned families
and the diagnostic evidence are recorded at the end of this ledger.

Historical campaign baseline: `52456d749880e798d99cafc692725a75d04fb7f8`,
branch `v4opt`. Raw artifacts live under ignored `obj/backend-perf/`;
compiler patches and normal fixtures are committed. The following measurement
and stopping settings describe that completed campaign; use the revised plan
for new experiments.

The primary target is complete-self frozen-workload user time divided by the
GCC-built compiler's user time, at most 1.00. Scratch/kernel measurements never
establish parity. Default resource limits are +3% executable text, +5% runtime
RSS, and +10% producer time/RSS. Early stopping is locked at N=6 distinct
hypotheses, S=3% individual gain, C=5% cumulative gain. Infrastructure does not
count. The report/frontend checkpoint timer started 2026-10-02 22:13 UTC;
check every three retained experiments or 60 minutes, including rejected work.
Full inception is due after five retained experiments, 20% cumulative speedup,
a concrete unresolved correctness concern, and at completion.

Measurements use CPU 4 (SMT sibling 48), serial builds/tests/timing, warmups and
paired ABBA blocks. Confirmation uses at least three blocks; small gains use
two batches and at least six blocks for intervals. The interval method is a
10,000-resample percentile bootstrap of whole block ratios, seed 340033; report
median and 95% interval. Preserve all raw samples. Canonical seed uses glibc;
canonical self uses jemalloc. The matched control preloads the system's same
jemalloc into the immutable seed, leaving canonical binaries/configuration
unchanged.

| ID | Hypothesis | Status | Complete-self gain | Stall contribution |
| --- | --- | --- | --- | --- |
| B00 | Baseline, cache and scratch calibration | Passed | — | Excluded |
| B01 | Constant-divisor accessor admission | Rejected: code growth | 1.31% provisional, no credit | Before B02 |
| B02 | Constant quotient instruction selection | Retained | 3.34% | Significant gain; window resets |
| B03 | Unused ordinary scalar reads | Rejected: inconclusive | No confirmed gain | 0; 1/6 |
| B04 | Early constant-return folding | Rejected: frontend uncertainty | 5.58% provisional, no credit | 0; 2/6 |
| B05 | Spare incoming registers | Rejected: kernel screen | No confirmed gain | 0; 3/6 |
| B06 | Canonical integer widening | Rejected: full workload | No confirmed gain | 0; 4/6 |
| B07 | In-place sole-return epilogue | Rejected: kernel screen | No confirmed gain | 0; 5/6 |
| B08 | Compact resolved loop branches | Rejected: kernel screen | No confirmed gain | 0; 6/6 |

## B00 — setup and baseline

Acceptance: an unmodified producer's replacement object and resulting workload
output must match the coherent checkpoint; paired runtime must agree within
noise. Selected source TUs captured at O0 with O3 preprocessing definitions
must replay through O3 to identical direct O3 objects. Record loop costs and
immutable inputs before implementation.

Snapshots: `bin/baseline-seed`, `bin/baseline-self`, `bin/reference`,
`checkpoint/selfhost/`, `checkpoint/generated/`, and `baseline-metadata.json`.
The metadata records source/header hashes, executable hashes and the verified
frozen manifest. Baseline self/GCC paired user ratio from the plan is 3.042;
fresh calibration is pending.

Scratch command (from repository root):

```sh
make -C pa34 probe-self-link CPPGM_HOST_CXX=g++ \
  SOURCE=../dev/src/support/id_index.cpp \
  PROBE_CXX=../obj/backend-perf/bin/baseline-seed \
  PROBE_CANON_OBJ_ROOT=../obj/backend-perf/checkpoint/selfhost \
  PROBE_OBJ_ROOT_BASE=../obj/backend-perf/B00/probe
PERF_CPU=4 python3 obj/backend-perf/run.py B00/probe-calibration \
  obj/backend-perf/bin/baseline-self \
  obj/backend-perf/B00/probe/bin/selfhost/cppgm++-probe 3
```

## B01 — constant-divisor accessor inlining

Predeclared before source edits. Parent: campaign baseline. Disassembly shows
`IdentifierTable::spelling` remains a call inside `syntax::Cursor::is`, with
vector-size division by 24. The optional inliner propagates a costly flag from
all division instructions and blocks those callees in looping/large callers.
Test classifying literal nonzero divisors as eligible for the existing bounded
inlining policy. Do not change body, recursion, exception or growth budgets.

Expected mechanism: remove small accessor calls and expose address/aggregate
simplification; fewer executed calls/instructions on an accessor kernel and
selected syntax/support objects. Boundary fixtures cover variable versus
constant divisors, signed negative inputs, exceptions and address-taken callees.
This is optimization coverage, not a reported correctness defect.

Screen: at least 2% kernel/scratch runtime improvement beyond calibration, plus
visible call elimination. If a small single-object screen misses linked weak
bodies, inspect winners and permit a small manifest-based replacement of the
profiled accessor providers; do not promote on code size alone. Complete-self
acceptance requires at least 1% median improvement with 95% interval below 1.00,
a second batch for gains below 3%, better self/GCC ratio, frontend guards and
default resource limits. Run PA32/PA33 behavior/bounds/debug and relevant inline
fixtures before promotion. Reject/inconclusive variants remain one hypothesis.

B00 scratch calibration passed byte equality in all 12 runs. User block ratios
were 0.99640, 0.98470, 1.00749 (median 0.99640). This establishes roughly 1–2%
single-block noise; small full-workload gains need confirmation.

B01 screen: PA32 contract/object roundtrip/debug and PA33 contract/MIR/debug
all passed, as did the added normal C++ fixture (9/9 native routes). The
accessor kernel removes `Range::view` calls and runs at new/old block ratios
0.54462, 0.62797, 0.62785 (median 0.62785). Kernel screening therefore supports
complete-self promotion. The initial cursor-only probe retains the old linked
weak `Cursor::is`; its timing is not evidence against the mechanism. A coherent
self generation avoids that proxy limitation. Patch: `B01/candidate.patch`.

B01 decision: **rejected**. Complete-self block ratios new/old were 0.93597,
0.98689, 0.99156 (median 0.98689); the paired self/GCC ratio was 3.13546.
Executable text grew from 8,534,693 to 9,369,100 bytes (+9.78%), violating the
predeclared 3% limit. Do not confirm/retain this small provisional runtime gain.
Runtime RSS was flat. The kernel counters fell from 6.168 to 3.994 billion
instructions and 774.0 to 392.0 million branches. The complete self generation
took 12.24 wall seconds (701.31 aggregate user seconds, 299,500 KiB recorded
peak). Frontend screens were 0.94906 versus initial seed and 0.43047 versus GCC;
these noisy one-block screens are not improvement claims. All frozen O0 outputs
matched. Keep binaries/patch/results for investigation; revert the policy.
The added C++ coverage fixture is saved with this rejected experiment, not added
to the retained implementation. Best accepted source remains the baseline.

## B02 — constant integer quotient selection

Predeclared before editing. Parent is the original baseline after reverting B01.
The hot vector-size paths contain 64-bit hardware division by 24/40. Replace
64-bit division by a nonzero literal with bounded shift/reciprocal-multiply
selection at O1–O3, keeping O0 and the inlining policy unchanged. Preserve
signed truncation toward zero, negative divisors and signed minimum values;
retain hardware division for zero and signed -1 (overflow trap). Variable
quotients and remainders retain the established path.

Expected evidence: hardware divide disappears for constant 24/40 and both
signed/unsigned kernels run at least 2% faster, with reduced cycles. Normal
self-checking C++ fixture compares constants with volatile runtime divisors
across boundaries and pseudo-random values; PA24/PA33 contracts, MIR/debug,
source/LowIR roundtrips and frozen seed/self checks must pass. Promote only
with the plan's >=1% complete-self gain and interval/second-batch/frontend
rules. Default text/RSS/producer-cost limits apply without exception. All
reciprocal decisions are derived from operand values, never symbol names.

B00 completed: the scratch compiler itself is byte-identical to baseline
(SHA-256 d25168c…); this is also the A/A calibration. Cache direct/replay
objects match for `support/id_index` and `syntax/cursor`. Observed direct vs
replay wall costs were 0.200/0.019 s and 0.547/0.105 s respectively; these setup
observations overlapped a build and are not acceptance measurements. Effective
capture options include `-g0 -O0 -D__OPTIMIZE__=1`; `-g0` selects the hosted
LowIR driver. Exact commands are in `B00/cache-records.json`; ordered canonical
link inputs are in `B00/link-command.json`.

B02 screen passed: constant-divisor C++ fixture passed all nine native routes;
PA24 and PA33 contract, bounds and debug checks passed. Unsigned quotient
kernel new/old user ratios were 0.23864, 0.23404, 0.23711; signed ratios were
0.47143, 0.46970, 0.46970. Both exceed the 2% promotion threshold. Complete
self generation and frontend screens follow. The reciprocal uses the existing
unsigned wide multiply and existing division clobbers, with no new MIR opcode
or O0 change.

B02 confirmation: complete-self new/old user block ratios 0.96665, 0.95644,
0.97372 give median 0.96665 (3.34% improvement), bootstrap interval
[0.95644, 0.97372]. Paired self/GCC blocks 2.74811, 2.62202, 2.91935 give
median 2.74811. The direct old/new comparison is the reliable gain estimate;
the larger change from the earlier 3.042 ratio also contains denominator/host
variation. Text is 8,633,685 bytes (+1.16%); runtime RSS ratio 1.00295. Selected
replay producer times/RSS were unchanged within measurement resolution;
direct/replayed objects match. Frontend screens are 0.94700 versus fixed seed
and 0.46284 versus GCC. Frozen seed/self objects match at O0/O1/O2/O3. Single
self build cost: 12.58 s wall, 699.27 aggregate user seconds, 300,072 KiB peak.
**Retain B02**, subject to scheduled whole-repository/frontend checkpoints.
This is one retained experiment; most recent full report/inception remains the
starting revision. Preserve the coherent B02 object tree for subsequent probes.

## B03 — remove unused ordinary integer/pointer loads

Predeclared before editing. Parent: retained B02 (patch/source hashes under
`B02/source-manifest.json`). Hot `Cursor::is` loads every copied Token field,
even though only its spelling ID is used. Scalar DCE currently treats every
load as an effect. In the DCE root decision only, allow unused nonvolatile
integer/pointer loads to disappear. Do not widen the shared `discardable`
predicate used by motion/loop transforms; retain volatile/atomic loads, stores,
calls, floating loads and multiple-definition values.

Expected mechanism: eliminate unused field loads and their address chains in
an inlined aggregate-copy kernel and the cached cursor TU. Normal LowIR fixture
must retain used, volatile and atomic loads while meeting a dead-load bound;
normal C++ fixture checks copies and observable volatile/call behavior. Screen
requires >=2% kernel/probe gain plus fewer loads/instructions. Full acceptance
uses the plan's >=1% complete-self improvement, confidence and second-batch
rules, frontend guards and unchanged default resource limits. Run PA32/PA33
contracts, object roundtrips, debug checks and frozen O0–O3 equality.

B02 diagnostic counters: self instructions 37.044 billion (baseline single
observation 36.565 billion), cycles 25.820 billion (baseline 26.933 billion).
The transform trades some instructions for lower-latency arithmetic. Signed
kernel cycles fell 2.295 to 1.067 billion despite instructions rising 0.980
to 1.460 billion. All hardware events ran at 100%; counters are diagnostic.

B03 screen passed: kernel ratios 0.68966, 0.69767, 0.69767 (median 0.69767).
The LowIR fixture confirms ordinary unused loads/address chains disappear while
used, volatile and atomic loads remain. Its first invocation used a numeric
status sidecar; corrected the new fixture to the harness's `EXIT_SUCCESS`
format. Normal C++ fixture passed 9/9 routes. PA32/PA33 contracts, bounds,
roundtrips and debug checks passed. Complete-self confirmation follows.

B03 decision: **inconclusive; reverted**. Initial three-block self median was
0.96566, but an independent six-block old/new host/self sequence failed to
confirm it. One additional block brought the self comparison to the planned
ten-block cap: median 0.97960, 95% interval [0.96566, 1.00317]. Seven paired
frontend blocks gave 0.97974 [0.95993, 1.04562], failing the <=1.01 upper bound.
The simultaneous-in-time ratio comparison (four binaries run serially in
symmetric order) gave backend change 1.01048 [0.94222, 1.03130], not a confirmed
improvement. Text shrank to 8,325,625 bytes, but size/kernel gains do not override
these acceptance failures. All samples are preserved in `self-confirm-1.json`,
`paired-confirm/quad.json` and `paired-final-block/quad.json`. The host's
`schedutil` governor varied frequency; CPU 48 was idle during the sampled core
check. No sample was removed. This is a bounded inconclusive experiment, not a
claim of a measured slowdown or a correctness defect.

The new ordinary-load fixtures and patch remain in `B03/` as experimental
artifacts; they are not added to the retained tree. Best retained source is B02.
Run a full report and frontend checkpoint now, before the next hypothesis.

Paired measurement extension: `quad.py` runs old seed, old self, new self,
new seed, then the reverse order, warming each once. Thus the self and frontend
comparisons each retain ABBA symmetry while sharing host conditions. It records
fresh self/GCC ratios for both revisions and their quotient, executable/input
arguments, output hashes, RSS and all timing samples. Object equality is checked
within each seed/self revision. This avoids comparing backend ratios measured
under different host conditions. Future confirmation should prefer this layout.

Checkpoint on retained B02, 2026-10-02 22:59 UTC: full root `make test-report
CXX=g++ CPPGM_HOST_CXX=g++` passed **5454/5454**. Rebuilt seed hash exactly
matches immutable B02 seed. Six frontend/fixed-seed ABBA blocks have median
0.99902; extend sampling to resolve the 1% upper bound before advancing.

Checkpoint harness correction: the first reference comparison accidentally used
`exact` output mode. Its sole validation error was the expected difference
between two different compilers' objects; both compilers were internally
deterministic. Repeat with `deterministic` mode, as required for the reference
and GCC lanes. Keep the original raw report; do not treat it as a compiler bug.

B02 frontend checkpoint passed. Ten fixed-seed blocks give median 0.99099,
95% interval [0.96756, 1.00599], within the 1.01 non-regression upper bound.
The corrected reference batch gives 0.96071 [0.91367, 0.99636], retaining parity.
Fresh GCC lanes give candidate/GCC 0.45661 [0.43078, 0.51094] and starting/GCC
0.42429 [0.42331, 0.45734]. Those noisy independent lanes do not establish a
regression; the direct fixed-seed paired comparison controls the decision.
All outputs are deterministic; seed/fixed-seed objects remain identical.
Checkpoint completes 2026-10-02 23:06 UTC. Restart the report/frontend timer;
one experiment retained since the initial inception, below its trigger.

## B04 — fold tiny constant-return calls before admission
B04: fold calls to tiny constant-returning inline bodies before optional inlining.
Parent: B02, subject to current frontend/report checkpoint.
Evidence: std::__is_constant_evaluated has a one-instruction return-false body.
The caller's dead assertion path contains division, and the immutable inliner
summary records it before nested expansion/simplification removes that path.
Consequently IdIndex::find retains three vector::operator[] calls despite their
final bodies becoming simple address calculations. Recognize only a direct
call to a one-block, one-instruction integer constant return, with fixed direct
integer/pointer parameters, exact result type, no no_inline/noreturn annotation,
and internal or inline/force-inline body permission. Keep definitions and
address-taken uses; replace only this call's value, with existing scalar
normalization and debug identity. No symbol-name special cases and no extra
whole-program pass. The subsequent existing CFG/EH cleanup and admission use
that fact naturally. Call arguments are still evaluated before the call; do
not fold calls with side effects or observable ABI conversions.

Screen on an invariant-mode accessor loop and the actual id_index.cpp TU:
constant-mode call and unreachable divide disappear, vector accessors inline,
and a kernel or scratch binary improves >=2% with fewer dynamic instructions.
Boundary fixture retains calls with side effects, no_inline, floating/by-address
arguments and externally replaceable bodies, and preserves addressed definitions.
Full criteria remain >=1% complete-self gain with CI below1, improved fresh
self/GCC ratio, frontend guards, default code/RSS/producer-cost limits, PA32/33
contract/debug/roundtrip checks and O0-O3 frozen equality. This differs from B01:
it proves the expensive branch unreachable instead of relaxing division cost.

B04 screen passed. Normal LowIR fixture removes the unreachable division and
inlines `get` into the loop; normal C++ fixture passes 9/9 routes while retaining
argument side effects and an address-taken constant helper. PA32/PA33 contract,
roundtrip, bounds and debug suites pass. Kernel ratios 0.40476, 0.42857, 0.43275
(median 0.42857) support complete-self promotion. Explicit call-site signatures
are conservatively excluded, as are nonmatching typed argument carriers.

B04 initial complete-self confirmation: new/old self block ratios 0.99544,
0.94073, 0.94423 (median 0.94423; 95% interval [0.94073, 0.99544]). Fresh paired
backend medians are old 2.89434, new 2.77444, with median within-block ratio
change 0.95404. Text falls to 8,561,216 bytes from B02's 8,633,685. The actual
`IdIndex::find` now contains no calls to vector subscripting; its old three calls
were eliminated. Replay time/RSS limits pass. Frontend one-block screen was
0.97901, but the mixed confirmation's frontend median was 1.01163 with wide
uncertainty, so extend the direct frontend comparison before retention.

B04 decision: **rejected; frontend confirmation inconclusive**. Ten direct
frontend blocks (six plus four) give median 0.99202 with 95% interval
[0.97388, 1.03183]. Despite the promising self result, this does not meet the
1.01 frontend upper bound. Stop bounded confirmation and restore B02; all
samples, including the earlier mixed and screening batches, remain available.
This is not evidence of a measured frontend slowdown. No performance credit.
Frozen O0–O3 seed/self objects all matched. Diagnostic instructions were
35.416 billion, cycles 24.360 billion, branches 4.996 billion. Save the patch
and optimization coverage fixtures with the rejected experiment. One attempted
final frontend command had a nonexistent baseline path and collected no data;
`frontend-final-valid` is the corrected four-block batch.

## B05 — use unoccupied incoming registers for global values

Predeclared before editing. Parent: retained B02. The global placement pass
excludes R8/R9 whenever a function has any parameters, even if a fixed signature
has at most four direct scalar integer/pointer inputs. In that case these two
registers carry no incoming values. Permit them under this narrow signature
proof; retain whole-function ownership, effect clobbers and EH fallback.
Do not change parameter retention or allocation lifetimes in this experiment.

Expect fewer preserved-register saves/restores and less frame traffic in
small leaf loops. Cover zero/one/many iterations, joins, six arguments, hidden
object returns, calls and exceptions with a normal C++ fixture and the PA24/33
contract/debug/bounds suites. Screen a short leaf reduction with runtime inputs
and check its emitted save/restore instructions. Require >=2% kernel/probe gain
or reject before full self; full promotion uses all default runtime, frontend,
resource and reproducibility criteria. This is a separate register-placement
hypothesis, not a variant of call folding or unused-load elimination.

B05 decision: **rejected at kernel screen**. Preserved-register saves fell
from four to two and the frame from 32 to 16 bytes, but three runtime block
ratios were 1.02597, 0.98734, 1.03896 (median 1.02597). The intended static
mechanism does not yield the required >=2% runtime gain. PA24, PA33 contract,
bounds/debug and the new C++ fixture (9/9 routes) passed. No complete self build
or frontend promotion is needed for this rejected screen. Save patch, binaries
and coverage under B05 and restore B02. This contributes zero performance gain.

## B06 — coalesce canonical integer widening

Predeclared before editing; parent retained B02. The reduction disassembly
loads an unsigned 32-bit value, copies it to a second register to widen it,
then consumes it as 64 bits. Existing loads and arithmetic normalize scalar
registers, but conversion placement does not reuse the dying input register.
Permit last-use coalescing for scalar integer conversions and omit a widening
extension only when the input's actual type matches the declared source type
and its canonical signedness matches sext/zext. Keep memory inputs, cross-sign
operations, wide integers and floating conversions on existing paths.

Screen the same fixed leaf reduction kernel (a different generated-code
mechanism from B05) plus a mixed narrow widening kernel; require fewer executed
moves/extensions and >=2% kernel gain. Cover signed/unsigned byte/word/int
boundaries, calls and joins in a normal C++ fixture; PA24/33 behavior, MIR and
debug checks must pass. Full acceptance, if promoted, uses unchanged backend,
frontend, size, RSS and producer-cost criteria. Variants share this one ID.

B06 screen supports promotion: leaf kernel median 0.98913 (below 2%), but the
mixed signed-byte/unsigned-word widening kernel gives 0.86000 with block ratios
0.86000, 0.81905, 0.91837. Normal fixture passes 9/9 routes and PA24/PA33
contracts, bounds and debug checks pass. Proceed to a coherent self generation,
frontend screens, replay-cost checks and paired full-workload confirmation.

## B07 — emit a sole return epilogue in place

Predeclared before editing. Parent will be the last accepted revision after
B06's confirmation. Hot accessors end in a five-byte jump to the immediately
following shared epilogue. When selected MIR has exactly one Return, use the
encoder's existing in-place epilogue mode instead of sharing an epilogue with
no other return. Leave functions with multiple returns unchanged. No frame,
calling convention or unwinding implementation change is intended.

Expect one fewer executed branch and five fewer code bytes per affected call.
Use a runtime noinline accessor-chain kernel and a normal fixture including
single/multiple returns, recursion and exception cleanup. Require >=2% kernel
speedup, inspect actual epilogue placement, and pass PA24/33 behavior/bounds,
unwind and debug checks before considering a coherent self. Full default
acceptance/size/producer/frontend limits remain unchanged. This is a call-exit
layout hypothesis, separate from register placement and integer conversion.

B06 decision: **rejected**. Complete-self ratios 1.03633, 1.00529, 0.95784
give median 1.00529 (95% interval [0.95784, 1.03633]), missing the required
1% gain. Fresh backend ratio worsens from 2.90577 to 2.97847; within-block
change 1.02904 [1.01192, 1.03094]. Text decreases to 8,582,874 bytes, but the
kernel/size wins do not predict the workload. Frontend screens were 0.97477
versus B02 and 0.42985 versus GCC; mixed frontend median 0.97692. No further
sampling is needed to reject this candidate. Cached/direct source objects and
all O0 workload seed/self objects match. Save patch/fixture and restore B02.
B07 now proceeds with B02 as parent.

B07 decision: **rejected at kernel screen**. The sole return loses its jump
and the existing in-place epilogue passes the normal fixture (9/9), PA24 and
PA33 contract/bounds/debug checks. Kernel block ratios 1.01887, 1.00000,
1.07619 give median 1.01887, failing the >=2% speedup requirement. Preserve the
patch and fixture, restore B02, and give zero credit (fifth hypothesis after B02).

## B08 — compact already-resolved loop branches

Predeclared before editing; parent retained B02. The encoder always uses a
five-byte jump or six-byte conditional branch, even for small backwards loop
edges whose target is already known. At O1–O3 use a two-byte branch only when
the target label has been emitted in the same function and the displacement
fits signed eight bits. Leave forward/unresolved/long edges unchanged; no
relaxation iterations or relocation/CFI rewriting. Keep O0 unchanged.

Expected mechanism: three/four fewer code bytes per applicable branch and
reduced fetch/decode footprint in short loops. Use the fixed reduction kernel
and a dependent integer mixing loop; require >=2% runtime screen gain, inspect
short versus long encodings, and run a normal loop/branch fixture plus PA24/33
behavior, bounds, debug and unwind checks. If promoted, require all default
complete-self, frontend and resource criteria. This encoding-density hypothesis
is distinct from eliminating the single-return jump in B07.

B08 decision: **rejected at kernel screen**. Short backward branches are
emitted as intended, but leaf block ratios 1.19318, 1.12500, 1.08434 (median
1.12500) and mixing-loop ratios 1.03125, 0.96410, 1.02538 (median 1.02538) fail
the >=2% gain criterion. PA24, PA33 behavior/bounds/debug and the normal fixture
(9/9 routes) pass. Save the experiment and restore B02. No complete-self
promotion; sixth zero-credit distinct hypothesis since B02.

## Diminishing-returns audit

B03–B08 are six completed distinct hypotheses: unused-load DCE, early constant
call folding, spare global registers, integer widening, sole-return placement,
and compact loop branches. None earns a confirmed retained >=3% gain. B04's
provisional self gain failed the frontend confidence guard; B03 hit its bounded
confirmation limit. Neither is credited. Best retained source before and after
this window is exactly B02, so the cumulative code change is empty. Run a fresh
paired B02/B02 comparison to verify the <5% cumulative criterion and calibrate
current timing noise; do not turn this into an improvement claim.

Bounded profile/disassembly review: small token/accessor calls, aggregate
copying and loop register traffic remain. `Ast::edge` still contains 32-bit
constant quotient/remainder hardware divisions (including division by 64),
which B02 intentionally did not cover. Extending the arithmetic family to
narrow operations/remainders is the strongest untried follow-up. It is a B02
variant, not a new independent stall hypothesis. Broader register lifetime
analysis and context-aware inlining remain larger follow-ups. With the locked
N=6/S=3%/C=5% rule eligible, stop new optimization after the cumulative check,
then validate and publish the best retained source. GCC parity is not reached.

The direct cumulative-window check is complete: the identical B02 self binary
produced ratios 0.95672, 0.96667, 1.00000 (median 0.96667; 95% interval
[0.95672, 1.00000]). The lower bound exceeds 0.95, satisfying the bounded
<5% improvement criterion. The apparent 3.33% timing difference between
identical binaries is noise/drift, not a gain. This reinforces the importance
of rejecting uncertain small changes. The retained source is byte-identical
to the B02 source manifest. Stop reason: **diminishing returns; GCC parity
not reached**. Final validation and fresh measurements follow.

Final-gate build correction: the first full report passed 5454/5454, but
inception's first new object failed byte comparison. Investigation found a
stale host object, not a source-level defect: restoring `arithmetic.cpp` with
`copy2` retained its earlier timestamp, leaving B06's canonical-widening code
in the seed while self compiled the restored B02 source. Rewriting the same
source with a fresh timestamp and rebuilding yields the exact immutable B02
seed hash 85a04525… again. No new correctness fixture is warranted for this
build-management error. Preserve the failed log and object diff.

This also invalidates B07/B08's claimed B02-parent screens: their old seed
binaries contained the residual B06 selection change. Those two decisions and
the early-stop trigger are provisional until repeated from verified B02.
Preserve old results as `*-stale` identities and rerun both original hypotheses,
without counting repetitions as additional experiments. Rerun final report and
inception afterward; timestamp restoration will use fresh writes henceforth.

B07 clean-parent repeat passes the same fixtures/contracts. Three kernel block
ratios 0.94118, 1.00000, 0.97959 give a marginal 2.04% median gain. This just
crosses the numerical screen but is smaller than current A/A drift. Before
spending a complete-self build, repeat the identical kernel with five times
as many iterations (one billion), keeping all six blocks for the decision.
This is confirmation of B07, not a new hypothesis.

B07 clean-parent confirmation rejects the hypothesis: longer-kernel ratios
1.03386, 0.99229, 0.97948 have median 0.99229. All six clean blocks combined
give median 0.98594, below the required 2% gain, with uncertainty crossing 1.
The earlier stale-parent results are excluded from the decision, retained as
invalid build evidence. Restore verified B02 and rerun B08 next.

B08 clean-parent repeat rejects the hypothesis: leaf ratios 1.16250, 1.15789,
1.13750 and mixing ratios 1.03684, 1.03665, 1.00000. PA24/33 and the normal
fixture pass again. The decisions for B07/B08 now rest exclusively on valid
clean-parent results. The six-hypothesis window and its immutable B02/B02
cumulative comparison are valid; the early stop is confirmed. Restore B02
with fresh source timestamps and rerun all final gates with seed/self hashes
required to match the immutable B02 pair.

Final clean validation: `make test-report CXX=g++ CPPGM_HOST_CXX=g++` passes
5454/5454. `make inception CXX=g++ CPPGM_HOST_CXX=g++` matches all **421 objects**
and the linked compiler. Both canonical binaries match immutable B02 hashes.
PA32 and PA33 contract, roundtrip, MIR-bound and required debug suites pass;
the added constant-quotient fixture passes 9/9 native routes with both seed
and self. The earlier PA1, PA19, conversion-member (seed/self) and overflow
runtime fixtures pass. Raw final logs are under `obj/backend-perf/final/`.
The auxiliary overflow byte-check command was corrected to use the driver's
supported output-file interface (no `-x`, explicit `--emit-lowir -o`); the
normal fixture itself had already passed. No source change resulted.

Final frozen workload objects match at O0/O1/O2/O3. The original overflow
fixture's emitted LowIR and objects also match at all four levels. The frozen
source plus all 51 headers match the epoch manifest. No source changes follow
these correctness gates; the final measurement script uses the exact B02
binaries just validated. Frozen object section sizes/hashes are recorded in
`final/object-sizes.json`; code size is reported separately from runtime.

Final backend confirmation: paired starting/current self/GCC medians are
3.00000 and 2.93358; within-block ratio change 0.97786 with 95% interval
[0.95296, 0.98572]. Current self median user time is 7.755 s versus 8.225 s
for starting self. Median direct self block ratio is 0.91498 [0.91139, 0.98881];
the GCC-built seed also moves (median ratio 0.96014), so do not attribute the
entire direct runtime change to generated code. Initial B02 confirmation's
3.34% gain remains the initial promotion result; final paired ratio improvement
is 2.21%. Final self peak-RSS ratio is 0.99815. Raw data: `final/paired/quad.json`.

Six final direct frontend/fixed-seed blocks have median 0.99405 but interval
[0.98486, 1.05291], including an 8.2% slow block. Retain all samples and extend
this final guard by six blocks after the other serial measurements finish.
Allow at most twenty final frontend blocks if needed; the 1.01 confidence
bound remains unchanged. B02 had already passed its earlier ten-block guard;
this extension establishes fresh final evidence, not a relaxed threshold.

Final reference frontend batch median is 0.98095, but its third block is
1.09627. Extend by six reference blocks too; retain the slow sample and the
same <=1.00 target. GCC frontend candidate median 0.44805 shows no increase
against the fresh starting lane so far. These confirmations do not change
source or repeat correctness tests without a new reason.

Twelve fresh fixed-seed blocks give median 0.98600 [0.96995, 1.03423]. The
median remains favorable, but the confidence guard is unresolved. Take the
remaining eight blocks in the declared twenty-block final budget. All twelve
existing blocks remain included; no outlier removal or threshold change.

Nine reference blocks give median 0.96679 [0.94656, 1.00385], with all samples
included. Add four reference blocks to resolve the small remaining uncertainty
at parity while the fixed-seed lane completes its eight-block extension.
Measurements remain serial on CPU 4.

Final matched-allocator control (jemalloc for both executables) measures
self/GCC 2.86220 [2.75806, 2.99179]. Canonical glibc-seed/jemalloc-self ratio
is 2.93358. Separate batches also contain host variation; do not interpret
their difference as an isolated allocator effect. Both show a large remaining
gap. The final perf sample reports 37.044 billion instructions, 25.901 billion
cycles, 5.346 billion branches and 56.51 million branch misses, all events
running 100% of the interval. The accepted change trades instructions for
lower division latency. Final profile retains string-input construction
(6.77%), IdIndex::find (4.88%), Ast::edge (4.25%) and IdIndex::put (2.96%) as
leads; no new optimization follows the bounded stopping audit.

Size/runtime tradeoffs: self text 8,633,685 bytes (+1.16% versus starting self;
2.031 times GCC's 4,251,910), executable 12,907,536 bytes. Frozen O0 object is
unchanged at 7,552,664 bytes (reference 2,944,688; GCC 3,205,528). O1 object is
3,333,008 bytes; O2/O3 are 3,332,800. Frontend RSS remains about 1.415 times
the reference; this campaign does not claim RSS or object-size parity. Runtime
CPU parity and these secondary size/memory metrics remain separate.

Final fixed-seed frontend guard **passes** at the declared twenty-block cap:
median 0.98600, 95% interval [0.96995, 1.00461], below the 1.01 upper bound.
All twenty blocks, including five ratios above 1.02, remain included. Peak RSS
ratio is 1.00015. The earlier checkpoint also passed independently. Fresh GCC
lanes are candidate 0.44805 [0.43413, 0.45027] and starting compiler 0.43413
[0.43034, 0.45172]; these intervals overlap, and the direct paired fixed-seed
comparison confirms no frontend regression.

Final reference parity guard **passes** with all thirteen blocks: median
0.97263, 95% interval [0.95855, 0.99647], RSS ratio 1.41420. All final timing
outputs are deterministic; same-revision seed/self outputs match. Both frontend
guards and every required correctness gate are now complete on the retained
B02 source. Final result: backend ratio 2.93358; **diminishing returns; GCC
parity not reached**. The outcome and remaining work are summarized in the plan.
No rejected source changes or generated artifacts are included in the commit.

## Structural campaign — started, 2026-10-03

Parent: `00a29292d2bf69678c6eb7013ca4c73ea627fe4d`. Implementation started at
2026-10-03 00:41 UTC; the report/frontend checkpoint timer starts there.
The demand audit under ignored
`obj/backend-demand-audit/` is diagnostic evidence, not a retained optimization.
The two-line unused-regex input emits 231 distinct bodies at O0 and 60 at O3;
GCC/reference each emit one. Fresh frozen-compilation counters show 37.044
billion self instructions versus 12.124 billion GCC-built instructions, with
identical output bytes. The plan records the complete inventory and limitations.

| Family | Hypothesis | Status | Acceptance lane |
| --- | --- | --- | --- |
| S00 | Freeze current artifacts, calibrate timing and reproduce demand/work inventories | Complete | Setup; excluded from stopping window |
| S01 | False emission roots retain unused ABI entries and support bodies | Retained with S02a; final campaign gates pending | Demand correctness/output quality at O0–O3, with live-use controls |
| S02 | Pruning happens after unnecessary lowering and data materialization | S02a retained; generated helpers/data remain partly eager | Demand work/RSS/output reduction with compile-time and self-time guards |
| S03 | Dead paths prevent accessor inlining and subsequent simplification | S03a confirmed; final campaign gates pending | Complete-self runtime and backend-ratio improvement |
| S04 | Calls in loops block optimizations beyond what their effects require | S04a retained with S05a; S04b parked at frontend confidence cap | Complete-self improvement with proven memory/slot safety |
| S05 | Whole-function register reservations cause hot-path stack traffic | S05a retained with S04a; S05b not retained (pressure cancels local EH gain) | Complete-self improvement with ABI/EH/liveness checks |
| S06 | Remaining profile identifies narrower code-generation opportunities | S06a not retained; kernel/instruction win did not confirm in full self | Complete-self improvement supported by measured coverage |

Before source edits, append a concrete entry with suffix ID as needed, parent
revision, mechanism, normal fixtures, numerical acceptance criteria, bounded
sampling budget and resource limits. Apply the revised plan's separate demand
and code-generation acceptance lanes. Preserve frontend parity, the report
checkpoint every three retained changes or 60 minutes, infrequent inception,
and final report/inception plus frozen O0–O3 seed/self equality.

Start a fresh N=6, S=3%, C=5% performance window only for distinct performance
hypotheses. Mandatory demand/correctness repairs and setup do not consume it.
The structural coverage gate must pass before a diminishing-returns stop;
B03–B08 do not carry over. B04's old frontend uncertainty is unresolved
measurement evidence, not proof that accessor simplification was ineffective.

### S00 — baseline and calibration, complete

Verified seed/self/reference hashes against the plan and copied immutable
binaries plus coherent self/generated trees into `obj/backend-perf/structural/`.
Three A/A ABBA blocks on CPU 4 give ratios 1.02222, 1.01601, 0.99265 (median
1.01601). All outputs match. This noise makes small timing screens inconclusive;
use deterministic demand/code evidence first and the declared extended timing
confirmation before retaining a performance claim. Remaining setup: manifest
verification and scratch-link control when advancing a backend experiment.

### S01a — ABI base entry is not an independent root

Predeclared before implementation. Parent is the campaign baseline. Merely
forming C2/D2 for an unused inline derived constructor/destructor currently
sets `object_root=yes` on its base functions. Preserve entry formation and
aliases, but let actual uses, explicit instantiation and other genuine roots
determine emission. Ordinary strong out-of-line definitions remain exports.

Acceptance: the normal `320-unused-base-entry.t` fixture loses both unused
base bodies at O1–O3 and their native definitions at O0–O3; the live-base
control keeps working at all levels through source/object/LowIR routes.
Run affected PA12/PA22/PA32 contracts and inspect the hosted regex reducer
and frozen output. This is one incremental demand repair, not closure of S01
or a runtime improvement claim. Retention still requires the demand lane's
frontend/self guards and final coherent validation; no stall-window credit.

S01a intermediate result: clearing every base root passed the reducer and live
runtime control but failed required PA22/PA23 metadata and one PA32 retained-entry
bound. Refined the repair: preserve standalone LowIR's source ABI exports;
for hosted objects, retain base-entry roots only if reachable before optimization.
A shared typed LowIR demand walk establishes that fact after merged TUs and
initialization/finalization bodies are complete. Ordinary used entries retain
their root metadata even after inlining. No references or tests were changed.
The dead reducer passes native O0–O3 and optimized LowIR O1–O3; the live virtual
base control passes all nine standard PA33 native routes. PA12, PA22, PA23 and
PA32 targets now all pass. S01 remains open: globals and other weak-body roots
still need investigation. Manifest verification also passed for the frozen
source and all 51 headers.

S01a inventory after the conditional-root repair: frozen O0 bodies 8,922 ->
8,535 and file bytes 7,552,664 -> 7,339,656; O3 bodies 2,765 -> 2,455 and
bytes 3,332,800 -> 3,187,632. Regex bodies are now 203 at O0 and 36 at O3.
The normal PA32 fixture passes its standard expectation/lowering/behavior
harness. These are static improvements; no runtime credit or final acceptance
is claimed yet. Logs and the intermediate seed are in `structural/S01a/`.

### S01b — non-inline weak bodies follow demand

Predeclared before editing. The native root rule keeps all non-inline weak
functions, including template instantiations requested only while checking
unused inline code. Optimized support pruning already treats weak functions
as demand-emitted. Use actual references or explicit roots for these bodies
at O0 too, independent of inline profitability hints. Ordinary user-written
weak definitions still supply an externally callable ABI and must carry a
root; retain them at optimized levels as well.

Acceptance: a normal C++ reducer drops the unused non-inline template helper
at native O0–O3, retains its explicit weak export, and links/runs from an
independent consumer TU. Optimized LowIR has only the two declared required
functions. Check direct/replayed O0–O3 object equality, PA27 weak/section
contracts, PA32/PA33 and the frozen/regex inventories. This is another demand
repair, with the same demand-lane guards and no stall-window contribution.

S01b reduced weak-template/export fixture passes O0–O3 with exact direct/LowIR
replay objects. Initial PA27 inspection exposed two additional ABI dependencies:
a used complete constructor keeps its required base companion, and an exported
TLS object supplies its wrapper even without a local wrapper call. Represent
complete-to-base edges during the pre-optimization demand closure and publish
exported TLS wrapper roots explicitly. PA27 now passes 163/163, PA32 and PA33
contracts pass, including native behavior/MIR bounds. Required exported ABI
surfaces remain intact. The unused-header case falls from 203 to 33 O0 bodies;
33 remain at O3. Global support-data roots are the next cause to trace.

S00 scratch control is complete: the unmodified baseline producer's replacement
IdIndex object and the entire relinked executable match the immutable baseline
byte-for-byte. The Make probe correctly rejected the old checkpoint after the
new source module changed its input list; replayed the recorded original link
manifest instead. No mixed source generation was admitted as coherent evidence.
The new PA27 separate-provider fixture and both PA32 expectation fixtures pass
their standard harnesses. Source changes are still provisional pending campaign
frontend/self measurements and final full validation.

### S01c — support data participates in reachability

Predeclared before editing. Rooting every defined global retains otherwise dead
vtables, static function-pointer data and their function graphs. Demand internal
and coalescable data from real consumers, explicit object roots, runtime roles,
required aliases and named sections. Strong globals remain exports. A class's
key definition and explicit instantiation retain its ABI table group. Apply the
same typed reachability to native data emission and optimized support pruning;
serialize all required roots before the LowIR boundary.

Acceptance: a normal unused-polymorphic/static-pointer reducer emits only main
at O0–O3; a separate key-function TU still supplies the vtable to its consumer.
Initialization/TLS/alias/section and live virtual-base controls pass. Direct and
LowIR-replayed objects agree at all levels. Require a materially smaller frozen
output and inspect the full regex remainder; no assumed speed claim from static
counts. Run PA27–PA33 affected contracts and the full report checkpoint before
the next demand phase. Demand-lane frontend/self guards remain required.

S01c now passes the full report (5454/5454), PA27–PA33, exact direct/replayed
objects on the reduced cases, and the separate key-vtable provider control.
The regex reducer is down to three bodies at O0–O3. A new normal explicit-class
static-data provider control initially failed: pruning exposed that semantic
instantiation did not publish this export. Record class-instantiation exports
explicitly for hosted lowering, preserving standalone LowIR's required metadata.
That control now passes. Performance acceptance remains provisional.

### S01d — retained member formation does not imply an independent export

Predeclared before editing. Parent is S01c. Empty destructors retained for ABI
entry formation still become roots even when their only source consumers are
unused inline bodies. Local-class entry formation has the same conflation.
Preserve semantic checking and entry availability, but in hosted emission follow
actual references and explicit class/member instantiation exports. Strong
out-of-line definitions and standalone LowIR keep their required ABI surface.

Acceptance: the normal empty-destructor/local-class reducer and the hosted regex
integration input have only main/answer at O0–O3. Keep live lifecycle/base-entry,
explicit-instantiation, cross-TU, initialization and rejection controls passing.
Require exact direct/replayed objects and affected PA17/PA19/PA22/PA27/PA32/PA33
checks. No timing or stall-window credit until demand-lane guards pass.

S01d closes the original reducer: regex and the normal empty-destructor fixture
each emit exactly one definition at O0–O3, with exact direct/LowIR replay bytes.
Clearing retained roots alone failed three live ABI controls. Added typed
conditional edges from actual source objects and destructor subobject actions,
so effect-free call elimination preserves a live owner's required entries.
PA27 and PA32 then pass; the four new PA32 fixtures pass their normal harness.
S01c's frozen O0 inventory is 4,954 bodies / 783,804 text bytes / 4,272,376 file
bytes, versus baseline 8,922 / 1,227,419 / 7,552,664. O3 is 1,937 bodies /
1,022,687 text bytes / 2,420,296 file bytes. These are stage-specific counts,
not a final candidate speed claim. Local-class policy needed no change for
the normal reducer; keep its existing ABI rules unless a false root is proved.

The fresh S01d full report passes 5454/5454. Its three-block frontend screen
against the fixed seed has user ratios 0.96040, 1.03272, 0.98381 (median 0.98381),
RSS ratio 0.98071. The timing interval is still inconclusive at this sample size;
do not claim accepted frontend non-regression yet. Demand correctness/output
reductions are established, but the materialized IR still includes dead bodies.

### S02a — materialize hosted function bodies from the demand closure

Predeclared before editing. Parent is S01d. Keep semantic checking, symbol/ABI
reservation and initialization facts intact. In a single hosted TU, construct
function IR only as required by roots, data references, ABI edges and already
lowered code. Flush generated helper queues between closure rounds; bound full
graph rescans to 32 rounds, conservatively emitting remaining bodies if needed.
Merged-TU and standalone LowIR presentation keep their established ordering.

Acceptance: at least 10% fewer materialized LowIR instructions on the frozen TU,
plus reductions on the unused-header/normal reducers, without losing diagnostics
or live ABI behavior. Use the existing normal demand fixtures, required rejection
contracts, and direct/LowIR replay comparisons at O0–O3. Record fallback use.
Require demand-lane self/frontend guards and resource limits, affected contracts
and full report at the demand checkpoint. No backend-ratio or early-stop credit
from a late symbol filter; this trial must avoid constructing the dead IR.

S02a deterministic screen passes. Frozen materialized instructions fall from
241,716 to 133,363 (44.8%); IR pool capacity falls from 55,066,624 to 41,697,280
bytes. Regex materialized instructions fall from 7,144 to 270 (remaining helper
IR is conservative), while native output remains its single required body.
The closure takes 19 rounds on frozen and two on regex; neither uses fallback.
The new O0 normal LowIR fixture checks that unused bodies are already absent,
and the unused-inline diagnostic fixture preserves mandatory semantic checking.
All personal demand fixtures and a fresh full report (5454/5454) pass.

Frozen O0–O3 direct objects match replay of unoptimized LowIR with the same
preprocessing configuration. An initial diagnostic replayed already optimized
O1 IR through optimization again and differed; that was an invalid equality
comparison, not a seed/self mismatch. Use O0 capture with `__OPTIMIZE__=1` for
O1–O3 as prescribed by PA32 and the cache contract.

Demand checkpoint frontend batches (three ABBA blocks, all included): fixed
seed ratio 0.95714, 95% interval [0.94632, 0.99378], RSS 0.95496; reference ratio
0.90693 [0.88337, 0.97942], RSS 1.35123. Both runtime guards pass this checkpoint;
RSS parity with the reference remains unmet. Full self acceptance and coherent
frozen seed/self checks are running. Treat S01a–d/S02a as one coupled demand
candidate until that validation completes. These repairs consume no performance
stopping-window slots. The report/frontend checkpoint is fresh as of 01:31 UTC.

The coherent demand self build passes frozen seed/self equality at O0–O3,
PA32/PA33 debug and roundtrip checks, and 27/27 personal native routes. A normal
40-step template-call chain also passes O0 and all native routes, exercising the
conservative fallback (34 rounds). Paired complete-self median change is 0.97037
[0.92350, 0.98184]; fresh old/new backend ratios are 2.90200 and 3.02366.
Absolute median old/new seed times are 2.495/2.325 s and self times 7.240/6.980 s.
The ratio has not improved: this is a reduction in compiler work, not a claimed
code-generation win. Self RSS is 519,846 KiB versus 526,194; self text is
8,647,737 bytes versus 8,633,685 (+0.16%). Diagnostic self instructions fall
37.0442b -> 35.1177b, with 100% counter running time. All samples remain recorded.
Frontend/GCC is 0.41944 [0.40017, 0.42933], below the prior campaign value.
Final demand frozen O0 output is 4,664 bodies / 758,799 text bytes / 4,059,864
file bytes; O3 is 1,744 / 993,191 / 2,268,704. Retention passes the demand lane,
subject to the selected-TU producer-cost check now running and final campaign
validation. The immutable seed/self and coherent object checkpoint are in S02a.

The selected IdIndex/cursor producer check passes: optimization-time ratios
0.97668/0.97842 and peak-RSS ratios 1.00853/0.98102 versus S01d. The coupled
demand candidate is retained under its work/output lane. Generated helper IR,
symbol/signature reservation and some support data remain conservative; this
does not claim that all dead semantic or lowering work has disappeared.

### S03a — constant-return folding before accessor admission

Predeclared before editing, parent the coupled S01/S02 demand candidate. Revisit
B04's general scalar-folding mechanism on this new parent. A direct, permitted
inline/internal one-instruction integer-return body can replace a call's result
without removing argument evaluation. This should remove the dead constant-mode
branch before accessor costing. No symbol-name rules or larger inline limits.
Preserve explicit noinline, replaceable ordinary exports, typed parameter/ABI
constraints, side effects and address-taken definitions.

Screen: normal LowIR and C++ effect/control fixtures, no subscript calls remaining
in actual IdIndex::find, and >=2% runtime-input kernel gain. Recapture hot-TU
LowIR from S02a with exact direct/replay proof. Confirmation: >=1% complete-self
gain, interval below 1, improved fresh backend ratio; a second batch for <3%.
Default text/RSS/producer limits and frontend guards apply. Use three confirmation
blocks initially, at most ten backend/twenty frontend. Run PA32/PA33 contracts,
debug and frozen O0–O3 seed/self checks before retention. This is the first new
performance hypothesis in the structural campaign, not carryover B04 credit.

S03a screen: normal LowIR/effect fixtures, PA32/PA33 contracts/debug checks and
frozen O0–O3 seed/self equality pass. IdIndex::find has no calls. New-parent
kernel ratios are 0.41071, 0.40678, 0.40000. Both hot-TU caches were recaptured
from S02a and match direct objects before and after the change. Host user times
changed markedly during the one-block frontend screen, so those samples remain
diagnostic and fresh A/A plus complete-self/frontend confirmation are running.

CPU 4's fresh A/A ratios were 1.07113, 1.05342, 1.00217: this host epoch is too
noisy to resolve the frontend guard. Its complete-self ratios 0.93978, 1.00590,
0.92017 are inconclusive; seed changes range 0.90207–1.09211, so the apparent
backend gain cannot yet be attributed to generated code. Preserve the entire
CPU 4 batch as diagnostic. Before further confirmation, move to otherwise idle
CPU 2 (sibling 46), recalibrate, and pair both old/new executables there. No
outliers will be removed and this noise is not a failed optimization hypothesis.
The original total sampling caps still apply: at most seven additional backend
blocks and sixteen additional direct frontend blocks for S03a.

### S04a — private scalar slots can cross a loop call

Predeclared before editing. Parent will be S03a if it passes confirmation; retain
that dependency explicitly because accessor inlining changes the remaining
call/slot graph. The existing promotion census already rejects escaped slots,
volatile operations, nonmatching types and unsupported EH merges. Its bounded
SSA construction need not reject the entire function merely for a call in a
loop. Relax only the scalar-slot admission, retaining all legality/work/growth
checks and the other passes' existing call-cycle policy.

Screen on a normal LowIR loop with an unknown call: promote private induction
and accumulator slots, while a C++ runtime control covers escaped state,
volatile effects, call clobbers and throwing calls. Inspect actual hot-TU output
and measure a runtime-input kernel; require >=2% screen gain or a predeclared
coverage-based improvement from eliminating repeated loop memory traffic.
Use the generated-code lane: >=1% complete-self improvement with interval below
1, improved fresh backend ratio, a second batch for <3%, and the standard
frontend/text/RSS/producer guards. Initial three-block confirmation, caps ten
backend/twenty frontend. PA32/PA33/debug plus frozen O0–O3 equality are required.

S03a CPU 2 confirmation: three complete-self ratios 0.94065, 0.93742, 0.94730,
median 0.94065 [0.93742, 0.94730]. Fresh backend medians are parent 2.93006 and
candidate 2.84902; all within-block ratio changes improve. Text falls 0.86% to
8,573,284 bytes. Selected-TU replay optimization time ratios are 1.08460/1.01248
and producer peak-RSS ratios 1.00000/1.07634, inside the declared limits.
Six direct frontend blocks on CPU 2 remain close to the 1% guard, so use the
remaining ten-block extension, preserving all sixteen CPU 2 blocks. The earlier
four CPU 4 frontend blocks remain diagnostic, for twenty total measured blocks.

S03a's bounded frontend confirmation passes: all sixteen CPU 2 blocks give
median 0.99016, 95% interval [0.97403, 1.00431]. The three ratios over 1.025 and
the low 0.92549 sample are all included. The direct comparison used exact O0
output equality in the extension. Together with the 5.94% complete-self gain,
improved paired backend ratio, smaller text and passing correctness/resource
gates, this confirms S03a for retention. Final campaign-wide guards still apply.
Collect fresh counters/profile before choosing the next structural trial.

Alternating S02a/S03a counter runs confirm instructions fall 35.1177b -> 33.5296b
(4.52%) and branches 4.9540b -> 4.6138b (6.87%), at 100% event running time.
The fresh profile has input-iterator string construction at 7.50%, Ast::edge at
5.60%, IdIndex::find at 5.05%, PPTokenCursor::next at 3.23%, and IdIndex::put at
2.65%. Demand walking is 0.64%. This supports proceeding to loop-state/dataflow
and remaining hot accessor/iterator code rather than more late output pruning.
The S04a normal loop currently has three loads, four stores and no phi nodes;
its unknown call prevents promotion despite neither private slot escaping.

S04a screen passes. The normal loop now has two phi nodes and no private
loads/stores; its call remains. Both LowIR fixtures pass their standard harness,
all 45 personal native routes pass, and PA32/PA33 contracts and debug checks
pass. Kernel ratios are 0.76000, 0.83673, 0.77551 (median 0.77551). All three
hot-TU caches (IdIndex, cursor, preprocessor) match direct objects for both
producers. Replay optimization time ratios are 0.88598, 0.98941, 1.04731 and
peak-RSS ratios 1.00000, 1.00370, 1.01678. The screen meets the declared limits;
proceed to frontend screening and a coherent complete-self confirmation.

S04a's full report passes 5454/5454 and its coherent self passes frozen equality
at O0–O3. The first complete-self batch improves runtime, median 0.96325
[0.94925, 0.98248], but the freshly paired backend ratio is inconclusive because
seed ratios range 0.87407–1.05515. Direct frontend confirmation is also unresolved
(ratios 1.01158, 0.95885, 1.07724). Preserve all measurements and use seven more
backend blocks, reaching the ten-block cap, plus six additional frontend blocks.
No retention claim yet. Static inspection also explains a limit: the hottest
input-iterator string constructor has EH, so promotion leaves stack traffic;
its body grows 3,447 -> 3,508 bytes and stack-operand instructions 308 -> 320.
This motivates assessing register placement separately after confirmation.

### S05a — share preserved registers across disjoint ordinary lifetimes

Predeclared before editing. Begin with non-EH functions as planned. The current
allocator reserves one register per cross-block SSA value for the whole function.
Give each candidate a conservative instruction interval that includes phi edge
transfers and closes over intersecting backward CFG edges. Two proven disjoint
intervals may share a register; incoming carriers, fixed clobbers, ordinary
temporary allocation and EH policy remain conservative. Bound closure work and
fall back to whole-function intervals when the budget is exhausted.

Use a normal LowIR program with two call-containing loop phases: behavior,
phi/backedge transfers, and a MIR stack/memory bound must pass. Screen for fewer
spills on the reducer and relevant hot TUs plus >=2% kernel improvement. A count
of available registers alone is not acceptance. Default generated-code criteria,
resource limits, frontend guards and PA33/debug/frozen checks apply. Parent is
S04a if retained; otherwise compare the explicitly coupled S04a+S05a trial to
retained S03a, using the existing S04a measurement to separate contributions.
No candidate builds or profiling overlap the running S04a confirmation.

S04a completes its ten-block backend budget with self ratio 0.96325
[0.94925, 0.98248], but backend-ratio change 1.00282 [0.95380, 1.05458].
It is inconclusive as a standalone code-generation candidate and is not retained
on that evidence. All ten blocks remain in `S04a/paired-combined.json`. Continue
only with the predeclared coupled S04a+S05a trial against retained S03a; do not
reuse S04a's apparent speed gain as a separate accepted improvement.

The S05a normal LowIR fixture executes correctly with the parent, but fails its
predeclared MIR bound: main has a 48-byte frame, versus the <=32-byte target.
This establishes the missing register reuse before building the candidate.

The S04a checkpoint (02:23 UTC) passes the fresh full report and PA32/PA33 debug
checks. Three-block frontend comparisons give candidate/fixed-start 0.93992
[0.90467, 0.98812], candidate/reference 0.91159 [0.89675, 0.95585], and
candidate/GCC 0.41840 [0.40017, 0.43486]. RSS/reference remains 1.35172: runtime
parity does not imply memory parity. This clears the periodic campaign guards
before building S05a, without resolving S04a's parent-relative retention gate.

S05a's normal fixture now passes at 32 frame bytes with three preserved
registers; PA33 contracts/debug and all 45 personal C++ native routes pass.
The runtime kernel isolates an important limit: S05a/S04a is 1.02632 (no
demonstrated win), while the declared coupled S04a+S05a/S03a trial is 0.79798.
Cursor::is loses three additional stack-operand instructions with register
sharing (23 -> 20); Preprocessor::raw's gain belongs to S04a (284 -> 189,
unchanged by S05a). Do not attribute the coupled kernel gain to register reuse.
The coupled trial advances to complete-self evaluation under its predeclared
parent comparison; S05a alone has no runtime retention claim.

Fresh direct/replayed objects match on IdIndex, cursor, preprocessor and
Ast::edge's occurrence TU. Three-block producer optimization-time ratios are
0.99559, 1.06129, 1.02265, 1.01148; peak-RSS ratios stay <=1.04395 and total
replay-driver time <=1.05276. These satisfy the default producer-cost limits.

S05a's complete self build passes frozen O0–O3 seed/self byte equality, and
the full report passes 5454/5454. Complete-self text is 7,965,999 bytes versus
S03a's 8,573,284 and S04a's 8,209,564 (the register-sharing component saves
243,565 bytes). Ast::edge falls from 88 to 44 stack-operand instructions for
the combination. Coherent paired timing and frontend confirmation are running;
these size/static improvements do not replace their acceptance gates.

### S05b — preserve scalar values through hosted exception unwinding

Predeclared before editing; execute after S05a's confirmation. Parent is S05a
if retained. Otherwise keep S03a as the accepted parent and explicitly measure
the S04a+S05b combination, omitting unaccepted register-sharing changes.
The profile's hottest input-iterator string constructor has handlers, so the
current blanket EH exclusion still places every cross-block SSA value on the
stack. Hosted unwinding and the standalone exception runtime have different
register-restoration rules.

The [Itanium EH ABI, section 1.6.3](https://itanium-cxx-abi.github.io/cxx-abi/abi-eh.html)
restores callee-saved registers to their throwing-call state before entering a
landing pad. Local `host_encoding.cpp` uses RAX/RDX for landing arguments;
`layout.cpp` already emits preserved-register CFI. In contrast,
`runtime_encoding.cpp` restores standalone registers from handler registration.
The proposed extension is therefore hosted-only, callee-saved-only, with
whole-function reservations across EH; no interval sharing across exceptional
edges and no standalone relaxation. Keep every existing fixed-clobber/type and
single-definition restriction.

Require a normal C++ control with loop state, post-registration computations,
actual throws, typed catches, destructors and nested resume/rethrow paths.
Run host-EH contracts, PA33 behavior/debug and explicit standalone controls.
Screen a runtime-input throwing/nonthrowing kernel and the actual string-input
hot body: fewer stack loads/stores and >=2% kernel/probe gain. Complete-self
acceptance remains >=1% with interval below one, improved freshly paired backend
ratio, two batches for <3%, and all default text/RSS/producer/frontend guards.
Use three initial blocks, at most ten backend/twenty frontend; frozen O0–O3
equality and full inception are required before retaining this EH extension.

S05a coupled confirmation passes its initial three complete-self blocks:
candidate/S03a 0.94923 [0.94131, 0.95252], with freshly paired backend medians
2.75884 -> 2.69612 and within-block backend change 0.96532 [0.92186, 0.99203].
Median self/seed user times are 6.260/2.335 seconds; parent times are 6.730/2.400.
Self RSS falls from 520,638 to 516,322 KiB. All four direct frontend blocks
(screen plus confirmation) remain within the 1% upper-confidence guard.

Alternating counters separate the components: retired instructions are S03a
33.52930b, S04a 32.92144b, S05a 32.05662b. All events run at 100%. Register
reuse contributes a further 2.63% instruction reduction after promotion, even
though its isolated reduced kernel was inconclusive. Total instruction reduction
is 4.39%, supporting the complete-self gain. The next periodic fixed-start,
reference and GCC frontend checkpoint is running before S05b implementation.

S05a's periodic checkpoint passes (02:34 UTC): frontend/fixed-start 0.91373
[0.91048, 0.94286], frontend/reference 0.94653 [0.91683, 0.95850], and
frontend/GCC 0.42424 [0.42233, 0.43550]. Retain the coupled S04a+S05a change;
best accepted implementation now includes demand cleanup, S03a and this pair.
This is the third retained group. Its >=3% complete-self gain prevents a
diminishing-returns stop in any window that includes it. S05b uses S05a as
its accepted parent. Final campaign gates and publication remain outstanding.

S05b screen passes: the new normal EH fixture passes all nine routes before
and after the change, all 54 personal native routes pass, PA26/PA33 contracts
and PA33 debug checks pass, and explicit private-runtime builds pass O1–O3.
The throwing/nonthrowing runtime-input kernel has ratios 0.81250, 0.82353,
0.77778. The actual input-string constructor loses ten stack-operand instructions
(320 -> 310); its EH structure remains. Four hot-TU caches match direct source
objects. Producer optimization-time ratios are <=0.99670, total replay time
<=0.99219, and peak RSS <=1.03386. Advance to coherent self evaluation; this
screen is not a complete-workload performance claim.

S05b variant 1 passes coherent-self frozen equality at O0–O3, but fails the
size screen: self text grows 7,965,999 -> 8,337,025 bytes (+4.66%, limit +3%).
Do not run confirmation or retain it. A symbol-size diff identifies large
parameter-heavy semantic functions, e.g. declare_object 19,161 -> 34,736 bytes.
These functions use the existing >=5-live-parameter carrier/home policy;
reserving all preserved registers for global values then spills ordinary
temporaries. The selected small hot-TU set missed this pressure case.

Before another confirmation, test one bounded variant: keep the EH exclusion
when >=5 scalar parameters need cross-block/call retention, matching the
existing parameter-placement pressure boundary. The non-EH policy is unchanged.
Add the actual declaration TU to the producer/static screen, preserve the first
variant's patch/binary/results, and require the original +3% complete-self text
limit. This is a policy variant of S05b, not a new stopping-window hypothesis.

Variant 2 restores declare_object to exactly its parent's 19,161 bytes. The
expanded normal six-parameter EH control, all personal native routes, PA26/PA33
contracts/debug and explicit private O1–O3 controls pass. The kernel still gains
16.22% (ratios 0.82353, 0.83784, 0.84000). All five selected TUs now have exact
direct/replay proof, optimization-time ratios <=1.01693, total replay time
<=1.01327, and peak RSS <=1.05238. Repeat the complete-self size/frozen screen
before timing this variant.

S05b variant 2 passes the complete-self size screen: text is 8,112,289 bytes
(+1.84% versus S05a, below the +3% limit and below campaign start). Frozen
O0–O3 seed/self objects match. Begin paired complete-self and frontend timing;
full inception remains mandatory before retaining this hosted-EH extension.

S05b decision: **not retained**. Three complete-self blocks give 0.99466
[0.98289, 1.01473], below the required 1% median gain and not confirmed; fresh
backend change is 1.03841 [0.94058, 1.05429]. Frontend confidence is unresolved.
Alternating 100%-running counter observations explain why the kernel does not
generalize: instructions rise 32.05681b -> 32.36207b (+0.95%), retired load
uops 9.76573b -> 9.98914b (+2.29%), and store uops 5.77189b -> 5.81117b
(+0.68%). Broader pressure outweighs the local EH-loop saving, even after
excluding the worst parameter-heavy cases. Other-host Rust builds were visible;
keep the timing result confidence-limited, not a proven runtime regression.
The valid timing screen and contrary dynamic evidence do not justify extending
confirmation or running inception on this candidate. Preserve both variants and
the normal EH fixture with its experiment artifacts, then restore S05a with
fresh source timestamps. This is one completed performance hypothesis after
the last >=3% retained gain; its two variants do not count separately.

Restoring with fresh timestamps reproduces both S05a seed and self byte for
byte. Fresh CPU-2 A/A block ratios are 1.00325, 1.00320, 0.99520; the host is
slower in absolute time but the paired calibration is stable. The new S05a
profile puts input-string construction at 6.68%, IdIndex::find at 6.60%,
Ast::edge at 4.76%, PPTokenCursor::next at 3.47% and IdIndex::put at 3.29%.
The input-string body still has 129 optimized-IR loads and 13 calls.

### S04b — retain bounded memory-value reuse inside call-containing loops

Predeclared before editing; parent S05a. Scalar promotion is now available, but
memory-value simplification still rejects entire functions with loop calls.
Its own bounded walk already clears mutable facts at unknown/throwing calls,
volatile/atomic operations and EH boundaries, and starts unvisited backedges
and handler entries with empty facts. Remove only that profitability admission;
keep the existing effect/alias/type proofs, 32-cell limit and work budget.
Other passes keep their current call-cycle policy.

The normal LowIR reducer must eliminate repeated reads between calls while
preserving reads after a mutation and all volatile reads. A normal C++ fixture
checks aliasing calls and the resulting values. Screen a runtime-input kernel
for >=2% improvement and inspect real iterator/hot-TU load counts. Confirm a
complete-self gain >=1% with interval below one and improved freshly paired
backend ratio; <3% needs two batches. The five-TU producer set includes the
parameter-heavy declaration file. Default +3% text/+5% runtime RSS/+10% producer
cost, frontend guards, PA32/PA33/debug, and frozen O0–O3 equality apply; start
with three blocks and cap backend/frontends at ten/twenty. This is a distinct
memory-reuse hypothesis, not another scalar-slot-promotion variant.

S04b's normal reducer proves the missing optimization before the change
(four loads versus the required two); afterward its normal harness, all 54
personal native routes, PA32/PA33 contracts and debug checks pass. The synthetic
kernel is neutral (1.00000, 0.98276, 1.01754), so it provides no advancement
credit. The real hot input-string body is materially different: LowIR loads
129 -> 91, native instruction lines 694 -> 616, stack operands 320 -> 248,
with 13 calls retained. All five hot-TU direct/replay proofs pass; producer
optimization-time ratios are <=1.03475 and peak RSS <=1.06371.

Use the plan's alternative actual-workload scratch screen before deciding
whether to build a complete self: replace only the preprocessor object in the
coherent S05a checkpoint. First reproduce its parent link byte for byte. Require
the same >=2% measured gain in three ABBA blocks, checked frozen output, and
inspect COMDAT selection. If a broader probe is justified later, predeclare its
exact object set separately; do not reinterpret the neutral synthetic result
as a gain or relax complete-self acceptance.

S04b's preprocessor-only scratch control reproduces the accepted S05a self
byte for byte. The candidate selects the intended smaller input-string body.
Three frozen ABBA blocks give 1.01201, 0.93973, 0.96199 (median 0.96199,
3.80% gain), with exact output and RSS ratio 1.00291. This passes the cheap
>=2% median screen. Its interval includes one, so it is not complete-self
confirmation. Advance to a coherent complete self build and paired acceptance.

S04b's coherent self passes frozen O0–O3 equality. Text is 7,457,259 bytes
(-6.39% versus S05a); frozen O3 text/object size is 849,557/2,112,648 bytes.
Initial three paired blocks show self ratio 0.94288 [0.90596, 0.99426], but
backend change 1.00836 [0.94319, 1.04929], since the seed also ran faster
in that sequence. Separate frontend confirmation is mixed: screen 0.98586;
three confirmation blocks 0.99457, 1.03986, 1.02980. This does not yet pass
retention. Collect alternating counters, repeat A/A calibration, then use the
remaining seven backend blocks and an initial six-block frontend extension;
retain all earlier samples and keep the original ten/twenty caps.

Alternating S04b counters support the intended mechanism: self instructions
32.05679b -> 31.15182b (-2.82%), loads 9.76572b -> 9.19805b (-5.81%),
stores 5.77183b -> 5.41108b (-6.25%), cycles 22.68433b -> 21.75917b.
Both seeds execute essentially identical instructions (11.67926b), loads and
stores; seed cycles differ by +0.64%. All events run at 100%. The repeat A/A
ratios are 0.98708, 0.99235, 1.03506: no persistent median bias, but measurable
host variation. Keep the bounded extension and every sample.

All ten S04b backend blocks are complete. Across both batches, self ratio is
0.97648 [0.94288, 0.98702], and within-block backend change is 0.96028
[0.94819, 0.98891]. Fresh backend medians are 2.59652 -> 2.50279. The 2.35%
self gain clears the >=1% requirement and the two-batch rule for gains below
3%; frontend confirmation is still pending. This result does not restart the
>=3% significant-gain stopping window if ultimately retained.

S04b decision: **inconclusive; not retained**. All twenty direct frontend
blocks give median 0.99998 [0.99414, 1.03134], RSS ratio 1.00043. The median
is neutral, but the 1.01 upper-confidence guard is not met at the declared
cap. This is not a demonstrated frontend slowdown. Complete-self performance
and dynamic evidence are favorable, but cannot override that safeguard.
Archive the implementation and normal fixtures in S04b, restore S05a with
fresh source timestamps, and check the restored seed/self hashes. S05b and
S04b are now two completed hypotheses since the last >=3% retained gain;
both contribute zero retained improvement. Do not describe measurement
uncertainty as proof of a runtime plateau.

Run a fresh full report and full inception on accepted S05a before S06. This
is an early structural integration checkpoint after demand/liveness changes,
not inception on every rejected experiment. S06a's parent is therefore S05a.

The restored S05a seed and self both match their immutable snapshots. The
fresh full report passes 5454/5454, and full inception passes with 423
matching objects and identical self/inception executables (03:23 UTC).
This is the first full inception for the accumulated S01–S05 changes.
Frontend guards pass at the same checkpoint: fixed-start 0.93504
[0.93090, 0.94400], reference 0.89435 [0.88806, 0.91569], GCC 0.40852
[0.39745, 0.41186]. RSS/reference remains 1.35125; memory parity is unmet.

A read-only demand audit of the unchanged S02a lowering snapshot records
886 explicit-root function bodies in frozen O0 and O3. They occupy 115,276
of 758,799 O0 text bytes and 157,682 of 913,020 accepted S05a O3 bytes.
Many are constructor/destructor ABI entries retained for live source use.
The JSON audit records root classes and ordinary predecessor chains; it does
not establish that all semantic/lowering work or all ABI policy is minimal.
Root retention explains part of the remaining size gap, while most text and
the measured executed-instruction gap remain outside these bodies.

### S06a — select branches through an elided Boolean conversion

Predeclared before editing, to run after S04b's decision and the next structural
validation checkpoint. Parent is S04b if retained, otherwise accepted S05a.
The actual hot input-string body repeatedly contains Compare -> truncation to
u8 -> Branch. Native alias analysis already proves the conversion preserves
0/1 and emits no machine instruction, yet compare/branch selection requires
immediate LowIR adjacency and the branch tests the alias rather than its root.
Consequently these sites still emit set/zero-extension/test after the compare.

Extend selection only across one proven, elided Boolean alias in the same
block, with a single use and single definition; resolve the branch's alias to
its proven root. Keep arbitrary intervening instructions, multi-use values,
mutable values, non-Boolean truncation and wider windows out of scope. Begin
with integer/pointer comparisons; leave floating and i128 conversion fusion
out unless a separate need and proof arise. Existing direct comparisons keep
their current handling. This is distinct from B06's widening coalescence.

Require a normal LowIR fixture that loses set/test materialization, controls
for intervening flag changes, multiple uses, non-Boolean truncation, and a
normal C++ runtime-input branch loop. Run PA24/PA33 behavior/MIR/debug and all
personal native fixtures. Screen with >=2% kernel or a preprocessor-only
coherent scratch win, inspecting the linked hot body. The five-TU producer
cost set, +3% text/+5% runtime RSS/+10% producer limits, frontend guards and
frozen O0–O3 equality remain mandatory. Complete-self acceptance is >=1%
with interval below one and an improved freshly paired backend ratio; gains
below 3% require a second batch. Start with three blocks, cap backend/frontend
at ten/twenty. This declaration provides no performance or early-stop credit.

S06a's parent reducer executes correctly but fails the intended quality bounds:
two compares, one set and one zero-extension instead of a single compare.
The candidate extends only the optimized selector's adjacency proof across
one aliased integer/pointer Boolean conversion, then reads its root's flags.
The normal fixture includes flag-clobber, multi-use and arbitrary-truncation
controls; the C++ fixture checks both branch outcomes and byte truncation.

S06a's screen passes: normal MIR bounds/behavior, all 54 personal C++ routes,
PA24/PA33 contracts and PA33 debug checks pass. Kernel block ratios are
0.88333, 0.86441, 0.86207 (13.56% median improvement). All five selected TUs
match direct source objects to cached LowIR replay. Optimization-time ratios
are <=1.00929 and producer peak RSS <=1.05692. Advance to a coherent complete
self build; the normal kernel alone does not establish a compiler speedup.

In the actual input-string body, decoded native instructions fall 657 -> 618
and set instructions fall 20 -> 7; all 13 calls and 320 stack-operand
instructions remain. These counts use objdump's wide format so byte
continuation rows are not counted as instructions. Earlier S04b line counts
used ordinary objdump output and included such rows; compare like formats.

S06a coherent self builds successfully: text 7,930,758 bytes (-0.44% from
S05a), executable 10,504,520 bytes. Its seed and self frozen objects match
exactly at O0–O3. Start the predeclared frontend/GCC screens and paired
complete-self confirmation; no retention decision yet.

Initial S06a confirmation is promising but unresolved: self ratio 0.97052
[0.94031, 1.01499] and backend change 0.98980 [0.95003, 1.01904]. Direct
frontend blocks are mixed too. Collect alternating self/seed counters and
repeat A/A calibration before deciding the bounded extension. A <3% final
gain still requires the predeclared independent second batch.

S06a counters: self instructions 32.05689b -> 31.42431b (-1.97%), cycles
22.55603b -> 22.40512b; loads/stores are essentially unchanged. Seed
instructions increase only 0.0059%. All events run at 100%. However, CPU-2
A/A ratios 0.96735, 0.98424, 0.99298 show directional drift comparable to
the expected gain. Host clippy/build processes are active; CPU 24 and its
sibling 68 on package 1 are idle in the recorded audit (CPU 8 was busy).

Calibrate CPU 24 before further candidate timing: require the three-block
A/A median within 1% of one and individual ratios within 3%. If it passes,
use that core for remaining confirmation and retain the entire earlier CPU-2
batch as diagnostic evidence of this host epoch, not selected individual
outliers. Do not exceed seven further backend blocks or sixteen further
frontend blocks: the original ten/twenty total budgets still apply.

The first CPU-24 A/A batch fails the predeclared individual-block check
(1.00401, 1.05169, 0.98390); do not advance confirmation on that calibration.
At 03:43 UTC the previously active clippy/build/test processes have exited
and CPU pressure averages are zero. Repeat the same calibration once in
this changed host state, preserving the failed control and original bounds.

Quiet-host CPU-24 A/A gives 0.99571, 1.01735, 1.01927: individual blocks
are now within 3%, but the three-block median misses the 1% check. Extend
this quiet-host control to six blocks once, keeping all six and applying the
same median/individual limits. This increases control precision, not candidate
acceptance limits; do not spend candidate confirmation until it passes. If
the combined control still fails, park this small-gain trial for this epoch.

The six-block quiet-host CPU-24 calibration passes the unchanged limits:
median 1.00653, all individual ratios within 3%. Proceed on CPU 24 with
two independent three-block backend batches and four initial frontend blocks.
The earlier three CPU-2 backend and four frontend blocks remain diagnostic;
all budgets include them. No individual timing samples are removed.

S06a decision: **not retained**. Six CPU-24 complete-self blocks give
1.01038 [0.98438, 1.03555], with backend change 1.00382 [0.99217, 1.03487].
The required >=1% self gain and improved backend ratio are absent. This is
not a statistically demonstrated slowdown. Four CPU-24 frontend blocks are
also inconclusive. The kernel's 13.56% gain and 1.97% instruction reduction
do not establish useful complete-workload performance. Memory traffic is
unchanged, consistent with the limited runtime effect. Preserve all CPU-2
diagnostics, CPU-24 samples, controls, patch and normal fixtures; no extra
confirmation is warranted. This is the third completed hypothesis after
S05a's >=3% gain, with zero retained improvement.

Restore the S05a seed with fresh timestamps and verify its hash. Its immutable
self/checkpoint already passed full inception. Leave the canonical S06a self
as an explicitly stale build until a future complete-self build or final
checkpoint; do not use it as the accepted parent or skip producer invalidation.

### S05c — make unused incoming carriers available in the newly leaf lookup

Predeclared before editing; parent accepted S05a (restored seed verified).
Revisit B05's general R8/R9 admission on this different structural parent:
B05's parent still had three accessor calls in IdIndex::find, so call clobbers
excluded these registers there. S03a removed those calls. The unchanged S05a
profile puts this now-leaf lookup at 6.60% of cycles, making actual applicability
different. B05's old leaf-sum screen remains a rejection, not fresh evidence.

Permit R8/R9 only for a fixed signature with at most four direct scalar
integer/pointer inputs and a non-object result. The owning ABI code shows
that this excludes hidden result inputs and leaves both carriers unoccupied.
Keep all clobber restrictions, EH exclusion and S05a's disjoint-lifetime proof.
Normal LowIR should reduce preserved-register/frame use in a leaf loop; keep
six-argument, hidden-result, call and exception controls in normal fixtures.

Screen an actual IdIndex lookup kernel with checked hits/misses, using one
fixed GCC-built driver linked separately against parent/candidate compiler-
produced IdIndex objects. This is a mixed-producer kernel, not a coherent self
claim. Require >=2% gain and fewer saves/restores, or use the supported
IdIndex-only coherent scratch probe with the same threshold and byte-exact
parent control. No other object replacement without a separate declaration.
If promoted, five-TU direct/replay cost checks, all default text/RSS/producer
and frontend guards, normal fixtures/PA24/PA33/debug, frozen O0–O3 equality
and complete-self acceptance remain mandatory. Start with three blocks, cap
backend/frontend at ten/twenty, and require a second batch for a gain below
3%. Use a calibrated core; current CPU-24 control is recorded under S06a.
This is one new hypothesis under changed hot-path applicability, not multiple
counts for B05 variants or repeated measurements.

S05c decision: **not retained at kernel screen**. PA24/PA33 contracts and
debug checks, both normal LowIR fixtures and all 54 personal C++ routes pass.
The normal leaf loop falls from four preserved registers/frame 32 to two/frame
16. Actual IdIndex::find falls 56 -> 54 decoded instructions and 8 -> 6 stack
operands, retaining zero calls. Its candidate cached LowIR object matches
direct source compilation, and the fixed GCC driver validates hits/misses.

Three CPU-24 kernel ratios are 0.98276, 0.99130, 1.06034: median 0.99130,
only 0.87% improvement, below the >=2% screen. No coherent self build or
full confirmation is justified. Archive the patch, normal coverage and all
samples, then restore S05a. This is the fourth completed hypothesis since
the last significant retained gain, again with zero retained improvement.

### S04c — dominator-scoped pure expressions in call cycles

Predeclared before editing, parent S05a. Remove only the call-cycle gate on
bounded dominance for the first pure-expression CSE pass. Keep the later
edge-fact gate, unique-definition checks, discardable-operation restrictions,
EH rejection, and existing traversal/work budgets. Calls cannot change the
values of immutable SSA operands. This is distinct from S04b memory reuse.

Require a normal LowIR positive reducer and mutable-temporary control, plus
a normal C++ behavior fixture. Screen the five existing producer TUs and
inspect the profiled Cursor::is, IdIndex lookup, and input-string bodies.
If no profiled body improves, stop at the coverage screen. Otherwise require
>=2% on a checked kernel or supported Cursor/preprocessor-only coherent
scratch probe, with byte-exact parent link control. Promoted candidates need
all default producer/text/RSS/frontend guards, PA32/33/debug, frozen equality
and complete-self confirmation (>=1%, interval below one, improved backend
ratio; independent second batch below 3%). Three initial blocks, ten backend
and twenty frontend maximum. This declaration earns no stopping credit.

S04c decision: **not retained at coverage screen**. The normal reducer changes
two multiplies to one across the call loop; its mutable-temporary control
keeps both. PA32 contracts/roundtrips and nine native C++ routes pass. Five-TU
source/replay equality holds; optimization-time ratios <=1.00928 and peak
optimization RSS <=1.06058. Cursor::is remains 86 decoded instructions,
IdIndex::find 56, and the input-string body 657, with unchanged calls and
stack use. The declared profiled-body coverage requirement fails, so no
kernel or complete-self timing is warranted. Archive and restore S05a.
This is the fifth distinct completed hypothesis with zero retained gain.

### S05d — model the converted-compare fixed clobber precisely

Predeclared before editing, parent S05a. parameter_flow.cpp declares RDX
clobbered for every compare followed by an integer conversion, whereas
arithmetic.cpp uses that fixed carrier only in the unoptimized converted-
return path. At optimized levels this can unnecessarily home the third input
of functions with at least five scalar parameters. Retain all actual call,
division, wide arithmetic, atomic and conversion clobbers; restrict the old
compare rule to O0 and ordinary scalar integer/pointer comparisons.

Require a normal five/six-argument reducer showing the third input remains
available after compare/conversion, with division/call controls in the native
contracts. Screen all five fixed producer TUs for native changes; if none
change, reject for absent representative coverage. If any change, inspect
those functions and require >=2% checked kernel or the relevant supported
coherent scratch probe before full self. All default correctness, replay,
producer/text/RSS/frontend and complete-self gates apply if promoted, with
three initial blocks and ten/twenty backend/frontend caps. This is an
incoming-parameter availability hypothesis, distinct from S05c allocation of
unused R8/R9. A small local improvement alone will not justify retention.

Before implementation, the shared first-clobber consumer also shows this
restriction affects straight-line input retention with fewer than five
arguments. The five/six-argument reducer remains sufficient to demonstrate
it. Preserve the conservative comparison rule for floating/i128 paths and
all O0 code; refine only optimized ordinary integer/pointer comparisons.

S05d decision: **not retained at coverage screen**. The normal six-input
reducer retains RDX and reduces preserved registers from two to one, with
frame size 16 unchanged. Its quality bound is <=1 preserved register, not
zero: the Boolean result still occupies a preserved register. Behavior,
PA33 contracts/MIR/debug and nine C++ routes pass. All five representative
native objects are byte-identical between parent and candidate, and all
source/replay pairs match. Producer optimization-time ratios <=1.01658 and
optimization RSS <=1.07390. No kernel or coherent self build is justified
without representative coverage. Archive and restore S05a. This completes
the sixth distinct hypothesis since its significant gain.

### Structural stopping review and final validation

Apply the N=6 early-stop option provisionally, pending final gates. The window
is S05b (EH allocation: no complete-self gain), S04b (memory reuse: 2.35%
self gain but frontend confidence guard unresolved at its sample cap), S06a
(Boolean branch fusion: no complete-self gain), S05c (unused carriers: below
kernel threshold), S04c (pure expression reuse: profiled bodies unchanged),
and S05d (clobber precision: five representative objects unchanged).
None was retained; do not describe the confidence-limited trials as proven
slowdowns. The retained source at both ends of this window is exactly S05a;
confirm its zero cumulative change with a fresh same-binary paired control.

Coverage is satisfied: the unused-ABI demand defect is repaired with negative
and live-use controls; deferred lowering has a bounded fallback and replay
proof; constant-return folding removes the profiled accessor calls; coupled
call-loop slot promotion and disjoint global lifetimes have a confirmed
complete-self gain. The unchanged profile and demand-root inventory show
remaining broad costs in input-string construction, identifier lookup,
syntax traversal and memory traffic. Explicit roots account for ~15% of O0
text and ~17% of O3 text and are classified ABI/export uses, not a demonstrated
major unexplained dead root. This is a bounded campaign stop, not proof that
further code generation or lowering improvements cannot succeed. Preserve
the parked memory-reuse candidate for a quieter future measurement campaign.

Final review found and repaired a correctness regression in S01's weak-root
classification: a non-inline explicit function-template specialization marked
weak was treated as a discardable implicit specialization. The baseline emits
its provider body, while S05a omitted it and a separate consumer failed to
link. Normal PA27 fixture 370-weak-specialization-export reproduces the failed
link and asserts the exported symbol. Explicit specializations now follow the
ordinary weak-export rule; implicit inline/template instances remain demand-
driven. This required repair is not a new performance hypothesis. Repeat all
final gates on the corrected revision, called S05a+export below, and compare
it directly to S05a as well as the campaign start.

The first final report/inception passed (5454 tests, 423 matching objects),
but precedes this repair and is superseded below. The personal validation
orchestrator also initially used PA29's text comparator for the historical
link fixture; its worker succeeded, but comparison lacked text sidecars.
Use the link-program comparator for that existing fixture without modifying
its oracle. Preserve the initial logs as superseded evidence.

Final correctness gates on **S05a+export**: `make test-report CXX=g++
CPPGM_HOST_CXX=g++` passes **5454/5454** and fresh `make inception` passes
**423 object comparisons plus identical self/inception executables**. The
normal personal native programs pass 45 routes with each compiler generation;
PA27 demand/export/link fixtures, PA32 source-demand and LowIR predicates,
PA33 lifetime MIR/behavior and both PA32/PA33 debug suites pass. The historical
conversion-member and overflow fixtures pass with both generations; overflow
LowIR and objects and frozen workload objects match seed/self at O0–O3.
The new weak-specialization provider/consumer also links and runs at all four
levels with both generations. The final validation orchestrator records 75
successful steps in `structural/final/validation.json`.

Final immutable binaries: seed SHA-256
`b140dbba6613515fa42d0897068dcf121409a8c9c20d438007bd66a8a24a2c42`, self
`2dc1f689cd24914fff887bfd047ab4d1e9b7f1c837c96622d7cb610a3420f07f`.
Source hashes, exact argv, environment/library records and validation outputs
are under `obj/backend-perf/structural/final/`. The source snapshot includes
only the retained changes plus the export repair; every rejected trial was
restored before building this revision.

The final export repair leaves all four frozen object files byte-identical
to S05a. Fresh three-block window comparison gives final/S05a self time
**0.99686 [0.99680, 1.00238]**, confirming <5% cumulative retained gain in
the six-hypothesis window. Self peak RSS ratio is 0.99729. Its seed time
ratio is **0.96524 [0.94499, 0.96901]**, satisfying the parent frontend
guard. The backend ratio rises relative to S05a because the denominator is
faster, not because self time increases: 2.58078 -> 2.65672. This required
export fix gets no generated-code performance credit. The same-binary
control is 0.98379 [0.96736, 1.02294]; retain that noise estimate and every
sample. Do not interpret its apparent 1.6% speedup as an optimization.

Untried directions after this bounded stop include coalescing proven adjacent
nonvolatile loads (the split 64-bit key in IdIndex), allocation across EH with
a measured spill-cost decision rather than broad admission, and incremental
lowering-demand traversal instead of repeated closure walks. These require
new proofs, reducers and measurement declarations. The parked S04b candidate
should first receive frontend confirmation under a quieter host epoch; its
2.35% complete-self gain and reduced memory traffic remain useful evidence.

Final timing batches all complete without output mismatches. The initial
six-event counter group runs only 83% of the time and is rejected by the
predeclared >=95% running check. Preserve it as `counters-multiplexed` and
collect memory and branch event sets separately, each with instructions/cycles
and alternating seed/self order. This counter-capacity failure does not
invalidate the separately collected timing samples.

### Final measured outcome — diminishing returns; GCC parity not reached

All final timings use CPU 24, three ABBA blocks per comparison, warmed
executables, frozen source at O0, and the declared block-bootstrap intervals.
Every output-validation check passes. Ratios below are independent paired
estimates; do not divide medians from different batches to reconstruct them.

| Final comparison | Median ratio | 95% interval |
| --- | ---: | ---: |
| Self / campaign-start self | 0.84809 | [0.81448, 0.86233] |
| Backend self/GCC-built, campaign start remeasured | 2.91887 | [2.73371, 2.93763] |
| Backend self/GCC-built, final | 2.63598 | [2.48503, 2.65612] |
| Paired backend-ratio change | 0.90417 | [0.90308, 0.90903] |
| Frontend / fixed campaign-start seed | 0.97024 | [0.93372, 0.97610] |
| Frontend / reference | 0.91522 | [0.90595, 0.94223] |
| Frontend / GCC compiling the frozen source | 0.41632 | [0.40921, 0.42845] |
| Campaign-start frontend / GCC, remeasured | 0.44367 | [0.42487, 0.46620] |
| Backend with jemalloc in both producers | 2.60484 | [2.50605, 2.66807] |

In the four-way paired batch, median self user time is 7.350 -> 6.255 s
and seed time 2.530 -> 2.400 s. The complete self workload improves **15.19%**;
the paired backend ratio improves **9.58%**. Final self peak RSS / start is
0.98457 [0.97885, 0.98585]; frontend RSS / fixed seed is 0.95494. Frontend
RSS / reference remains **1.35045**, and frontend RSS / GCC is 1.02886.
Runtime frontend parity is preserved; reference RSS parity is not achieved.
The fixed-seed and S05a parent frontend intervals both pass the <=1.01 bound.

| Frozen output | Start bodies / text bytes / file bytes | Final bodies / text bytes / file bytes | GCC bodies / text bytes / file bytes |
| --- | --- | --- | --- |
| O0 | 8922 / 1,227,419 / 7,552,664 | 4664 / 758,799 / 4,059,864 | 3862 / 602,123 / 3,205,528 |
| O1 | 2765 / 1,301,230 / 3,333,008 | 1724 / 913,157 / 2,176,152 | 384 / 418,966 / 966,168 |
| O2 | 2765 / 1,301,049 / 3,332,800 | 1724 / 913,020 / 2,175,968 | 440 / 382,735 / 935,768 |
| O3 | 2765 / 1,301,049 / 3,332,800 | 1724 / 913,020 / 2,175,968 | 403 / 377,704 / 924,816 |

The regex-header reducer now emits exactly **one 16-byte function body in a
2,024-byte object at every level**, with exact seed/self objects. Frozen file
size falls **46.25% at O0** and **34.71% at O3**. Final self text is 7,966,047
bytes, down 7.73% from 8,633,685; its file is 10,541,384 bytes. The seed has
4,260,550 text bytes and a 5,180,440-byte file. Distinct body counts deduplicate
aliases by section/address/size.

The final O0 object still exceeds the reference's 2,944,688 bytes. Its section
accounting is 758,799 text, 34,019 data, 237,502 unwind, 636,504 relocations,
252,120 symbol tables, 1,412,857 string tables, 42,388 groups, and 685,675
headers/alignment/other bytes. Reference string tables total 636,340 bytes;
most of the remaining file-size difference is metadata rather than executable
text. ELF string-table sharing is another possible size experiment, without
assuming it would close the execution-time gap. Neither object-size nor RSS
parity is claimed.

Both final hardware-counter groups run at **100%**. Alternating seed/self
memory observations give 11.67928b -> 32.05693b retired instructions (2.74477x),
8.62885b -> 22.48892b cycles (2.60625x), 3.20520b -> 9.76563b loads (3.04681x),
and 1.55362b -> 5.77187b stores (3.71512x). The separate branch group gives
2.09220b -> 4.72268b branches and 49.87984m -> 56.93381m misses. The retained
profile's spill-heavy input-string and traversal paths remain consistent with
these results. Excess executed instructions and memory operations, rather than
allocator choice or cold file bytes alone, explain the larger remaining lead.

Final inventory and the new weak-specialization direct/LowIR-replay checks
also pass for seed and self at O0–O3. The multiplexed counter attempt is kept
separately; its failed status is not overwritten. Successful split-counter
runs and all timing/inventory argv are retained in the final artifact directory.

**Decision:** close this campaign under the predeclared six-hypothesis early
stop. Retain S01/S02 demand and deferred materialization, S03a constant-return
folding, coupled S04a/S05a slot promotion and lifetime sharing, and the final
weak-specialization export repair. GCC parity is not reached. The remaining
hypotheses above and the confidence-limited S04b trial are future work; no
rejected implementation or generated artifact belongs in the commit.
