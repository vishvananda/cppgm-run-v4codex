# Backend performance experiments

Campaign baseline: `52456d749880e798d99cafc692725a75d04fb7f8`, branch
`v4opt`. Follow [backend-perf.md](backend-perf.md). Raw artifacts live under
ignored `obj/backend-perf/`; compiler patches and normal fixtures are committed.

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
