# PA29 checkpoint audit178

Target: **PA29 full-stage**. Phase: **checkpointAudit complete; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `7139ceb5769eea12c6e2e00b54932f09d59c79b5`.
Audit entry: `e59dcaa11e9f674526962ea1fa28f864469fb081`.
Last reviewed commit: `667edd800e4e5eb1b1ef92a3108da3bd96c51708`.

Reviewed the complete previous-review-to-code-tip range: all **15** accumulated
commits across three accepted handoffs and the audit fix commit, including the
combined changes to **72** implementation/build paths. The
[range record](../student.tests/pa29/evidence178/range.json) lists every commit,
path and combined-diff digest. This is not a latest-handoff review.
[Audit174](audit174.md) preserves the previous findings and ledger verbatim.
The code fix was validated and committed first; this record update changes no code.

## Findings and owner repairs

1. **Lowering indexed declaration arrays using a growing semantic entity count.**
   Source-invocation constant evaluation can append string support entities while
   lowering static initializers. Implementation175 separated their emission queue
   from declaration lifecycle arrays, but `global_finalization` and three symbol
   loops still used the growing count. The reduced
   [source-storage control](../student.tests/pa29/source178/source-storage.cpp)
   requests two distinct file defaults and has a global destructor.
   An entry build with ABI-compatible vector assertions aborts in
   `Procedural::global_finalization`; the
   [stack and assertion proof](../student.tests/pa29/evidence178/bounds-proof.json)
   identify the out-of-range access. All four loops now use the fixed declaration
   symbol extent. Strings retain canonical entity identity and their separate
   typed byte queue; there is no padding with fake declarations or name recovery.
   The final [bounds checks](../student.tests/pa29/evidence178/bounds178.json)
   pass all 16 commands. This is a targeted assertion build, not a full sanitizer run.

2. **Deferred inline initializer checking inherited its requester's context.**
   `sizeof` bound completion and a discarded branch could suppress a required
   initializer's function/default/storage dependencies. A use inside a function
   also incorrectly supplied that function and source-invocation context to a
   static member definition. Move this owner into registered
   `semantic/inline_variable.cpp`. It checks the retained definition in its own
   context, records deduplicated typed dependency edges, and separately demands
   them exactly once when storage or unknown-bound completion requires the
   definition. Active/success/failure states distinguish checking from demand;
   actual dependencies use the existing member/specialization/default queues.

   C++11 N3485 [temp.inst]/1–3 distinguishes member declarations, definitions and
   required initialization effects; [expr.sizeof]/1 requires a complete operand
   type even though its operand is unevaluated. The later inline-variable
   extension retains this distinction in [N4659 temp.inst](https://timsong-cpp.github.io/cppwp/n4659/temp.inst).
   Computing the bound from this initializer needs its definition. The body of
   `init<int>` must therefore be checked and its initialization effects retained;
   an inner `sizeof(dormant<T>())` remains unevaluated. Non-template discarded
   statements still receive semantic checking under
   [N4659 stmt.if](https://timsong-cpp.github.io/cppwp/n4659/stmt.if).
   [Reduced controls](../student.tests/pa29/source178/inline-bound-body.reject.cpp)
   prove required rejection, while effect/default/discard-first/inner-unevaluated
   controls check runtime behavior. [Host observations](../student.tests/pa29/evidence178/bound-contract.json)
   corroborate these rules; they are not the sole proof.

   A preliminary blanket reset emitted constant-only inline objects. Inherited
   inspection176 caught that regression. The final split between definition
   checking and dependency demand preserves its no-storage check. Source builtins
   now observe the initializer's lexical site and empty enclosing function, as
   required by the [builtin contract](https://clang.llvm.org/docs/LanguageExtensions.html#source-location-builtins).

3. **Template defaults could project occurrences before attaching a frame.**
   A default such as `width<T>()` was instantiated before the function body, with
   no canonical substitution frame attached to its occurrence context. Dependent
   `sizeof(T)` and pack queries consequently saw missing arguments. Defaults now
   attach the complete enclosing/canonical frame first, adding an immutable overlay
   for a renamed declaration head; a flat `(specialization, declaring head)` index
   reuses the context. Function bodies reuse the same canonical frame mechanism.
   There is no body instantiation merely to evaluate a default and no grammar replay.
   N3485 [temp.decls]/2 makes each default a separate definition, and [temp.inst]/3
   specifies its demand independently of body demand. See the local
   [standard text](../doc/n3485.txt) and
   [default-frame control](../student.tests/pa29/source178/default-frames.cpp).
   Renamed heads, member templates, late definitions and packs all pass.

[Preliminary disposition](../student.tests/pa29/evidence178/preliminary-disposition.json)
retains the failed approach and its correction. The initial personal inspection
assertion was also narrowed from forbidding a builtin declaration to forbidding
an emitted call; the contract permits declarations. No course comparison changed.
No reference, input, sidecar or discovery rule changed. The bundle remains pinned
to source `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`; no reference correction is claimed.

## Every commit reviewed

| Commit | Content and cross-handoff interaction |
|---|---|
| `235ffa39` | Audit174 records, reviewed marker and retained evidence. |
| `3edfe15e` | Invocation-owned source builtins, contextual defaults and queries. |
| `393b19cc` | Lazy source-address constants and typed support-string emission. |
| `7bb44553` | Separation from declaration lifecycle tables; the remaining growing loop bounds are fixed here. |
| `704538c1` | Validated intrinsics excluded from deferred body demand. |
| `2b7a5132` | Implementation175 validation, performance and source-invocation handoff. |
| `f1b878e5` | Implementation176 declaration/emission ownership plan. |
| `207743ae` | Excluded member demand and inline variable linkage/emission identity. |
| `61756cf8` | Deferred member initializers and shared reference-temporary lifetimes. |
| `1b19e9ac` | Deferred inline array bounds; definition demand crossed unevaluated/discarded contexts incompletely. |
| `d69d57fc` | Implementation176 validation, ABI question and performance handoff. |
| `eaa24678` | Selection initializer scopes, condition conversions, constexpr selection, lifetimes and jumps. |
| `8288199b` | Discarded runtime demand and inferred returns; interaction with deferred initializers checked here. |
| `4a7cf08d` | Implementation177 controls, performance, remaining work and handoff. |
| `e59dcaa1` | Validated implementation177 completion boundary. |
| `667edd80` | Three owner repairs above, reduced/integrated controls, assertions, inspection and measurement tools. |

## Architecture and optimization audit

`lowering/driver.cpp` connects immutable buffers through the preprocessor,
post-token and syntax cursors into cooperating Parser/Analyzer construction.
Deferred regions retain parsed source occurrences; projections refer to source
nodes and compact contexts rather than replaying grammar or cloning semantic
bodies. Types, declarations, argument packs, specialization/query keys and ABI
entities use canonical IDs and flat indexes. Default keys include specialization,
source/default identity, invocation site/evaluation mode and declaration revision
where relevant; renamed heads get parent-linked overlays. The fixes supply the
missing complete frame rather than adding a global cache flush.

The nontrivial declaration is `Box<int>::data` in the
[integrated control](../student.tests/pa29/source178/integrated.cpp). Its exclusion
attribute preserves local definition demand despite the enclosing extern-template
class. `sizeof` completes the array from its initializer, retaining one call to
`initialize<int>`. The default invokes `line` at the initializer's `member.hpp:40`
site; the source constant reaches typed LowIR with no builtin call. Initializer
checking, required body/default dependencies, storage and emission have separate
states and canonical identities. The new edge index is keyed by recipe, typed
reason and target, has TU lifetime, and schedules actual dependencies once.
A constant-only value query computes its fact without demanding storage.

The demanded `work<int>` template selects its true constexpr arm. Its ordinary
if-initializer constructs a Guard, reads the already demanded data through the
excluded member body, saves the return value, destroys the Guard once, and exits.
The dependent invalid discarded arm is not instantiated or lowered. Condition
conversions, selected branch and lifetime prefixes are recorded by semantics;
lowering consumes those typed facts without resolving names or inventing syntax.
Calls are emitted directly to typed LowIR, then to compact per-function MIR and
ELF. Explicit LowIR writing/parsing is confined to the inspection adapters.
Source-string support bytes are drained separately after global initialization;
all declaration lifecycle accesses stop at the declaration snapshot.

The useful facts traced are the constexpr selection and the selected callees'
effect/unwind facts. Branch exclusion is legal after constant evaluation;
non-template discarded statements still require diagnostics. The known scalar
`read`/`line` bodies cannot throw, so the existing direct-callee unwind proof can
omit impossible cleanup edges; unknown/throwing calls retain conservative cleanup.
Inherited exception/jump controls cover observable destruction. There is no new
optional optimization. O0 profitability policy avoids additional speculative
work; optional work/growth budgets are zero. Facts remain valid at their canonical
owners, and publishing a new initializer/default does not invalidate unrelated
lookups or restart pending specializations.

[Machine traces](../student.tests/pa29/evidence178/machine-trace.json) preserve the
actual LowIR/MIR. Integrated `work<int>` uses a 48-byte frame, preserves rbx and
has a Guard slot, condition spill and saved return. The final encoding and unwind
records appear in [inspection178](../student.tests/pa29/evidence178/inspection178.json).
The two source addresses in `source-storage` produce exactly two support globals.
Roundtrips are byte-identical and the explicit adapter objects execute correctly.
Telemetry-on/off objects are identical. Actual spill/frame costs are disclosed;
smaller IR is not treated as proof of runtime profit.

Source/semantic arenas, frame indexes and dependency vectors belong to the TU.
Short-lived traversal vectors end with their operation; no per-node owning
allocation or process-global mutable cache was introduced. `native/driver.cpp`
selects/allocates/encodes one compact Function at a time, releasing transient MIR
before the next function. Cross-function storage is limited to required symbols,
COMDAT, relocations and initialization/linkage facts. Host compilation/linking in
personal checks is observation/runtime support, never production implementation.
Measured work follows source nodes, demanded facts and dependency edges, without
whole-registry retry or unbounded fixed-point optimization.

## Validation and performance disposition

[Final validation](../student.tests/pa29/evidence178/validation.json) and the
[manifest](../student.tests/pa29/evidence178/manifest.json) bind the tested compiler
and all **467** implementation/build files to the reviewed code commit. Validation
ran just before commit, so its recorded HEAD is the entry; the manifest explicitly
binds that tested working source to the new commit. Required commands were rerun
once the new source was included in the hash inventory.

- `make test-pa29`: **381/403**, exit 2; exactly the entry's **22** failures.
- Exact prior-through shell command: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4919/4941**, exit 2; only PA29 fails.
- `perl scripts/cppgm_file_audit.pl --stage pa29 --paths dev/src`: pass; four inherited substantial-header warnings remain.
- Closure/fold controls173/174: **59/35 checks**, all pass.
- Source/declaration/selection controls175/176/177: **17/25/36 cases**, all pass.
- New controls178: **15 student cases and 15 host comparisons**, all pass. Entry fails eight of the 15 student cases; the bounds defect additionally requires assertions to expose reliably.
- Inspections175/176/177/178: **37/48/59/91 commands**, all pass, plus all roundtrip/symbol/telemetry assertions; targeted bounds run: **16** commands pass.

The [exact failure delta](../student.tests/pa29/evidence178/stage-delta.json)
contains no new or removed course failures. [Coverage](../student.tests/pa29/evidence178/coverage.json)
checks all **1,707** contract/harness paths against both review and entry boundaries
and preserves all **403** stage inputs. Personal passes do not offset new failures.
All control/inspection command results are retained beside those records.

[Performance178](performance178.md) applies stage-scoped acceptance to PA29/O0:
**776** final performance observations and eight launcher observations, plus review
of all **816** inherited observations. Frozen A/A and ABBA data cover compiler
latency/RSS, checked runtime and executable text. All 13 paired executable code,
data and unwind sections are identical; compiler paired medians and noise ranges
are reported without a speedup claim. Corrected array semantics use final-only
scaling because entry is incorrect. Initializer computations and demands are N,
with N or 2N dependency edges on the measured declaration/bound inputs. Required
semantic costs and later optimizer work do not add an exit gate. Unsupported
blanket 15%/zero-growth gates remain diagnostic; all mandated capacities, timeouts,
correctness and coverage remain intact.

## Remaining work and handoff quality

The [remaining ledger](../student.tests/pa29/evidence178/remaining.json) groups work
as extended syntax/types/layout **17**, template demand/hosted ABI **4**, and legacy
trait **1**. Continue those broad owners through integrated source-to-ELF checks.
The nothrow-default shorthand, nothrow-invocable default and nested ABI-tag cases
remain three counted contract questions; none is waived or revised. The ABI
extension disagreement lacks a C++11 proof sufficient to change the reference.
Char-trait conversion remains unfinished implementation. Full through-PA29 success
is required before advancing to PA30.

The three handoffs cover distinct useful owner groups, reducing 26 to 22 course
failures since audit174. However, separating string emission from lifecycle loops,
then splitting initializer checks from array-bound demand and discarded-context
handling caused avoidable followup fragmentation. Each owner should finish its
unevaluated/discarded/default/lifetime interactions and integrated host linkage
before handoff. This audit reviews every increment and leaves one code baseline;
it completes the checkpoint audit, not PA29.

## Audit ledger

| Checkpoint | Reviewed range | Findings / disposition | Evidence / result |
|---|---|---|---|
| 158 | `2734e5c6..1ab3499d` (three handoffs) | Alias/expression/template storage fixed; deleted-copy reference corrected with clause proof; historical performance targets classified under spec §9. | PA29 317/403, no new failures; PA1–28 4538/4538; file audit/controls pass; four performance dimensions retained. |
| 162 | `1ab3499d..cce8634c` (entry `9662716b`, three handoffs) | Fixed atomic bool RMW, reference snapshots, alignment/native fallback and cv/identity conversions; reviewed complete traits/invocation/atomic ownership range; no reference changes. | PA29 338/403, identical 65 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 221/221 explicit controls; native/LowIR/cache checks and 1,088 performance observations retained. |
| 166 | `cce8634c..f07f7823` (entry `3036bd4e`, three handoffs) | Reviewed all assembly, function-context and evaluation/storage increments; fixed effect invalidation, runtime extents, prvalue materialization and complete query receiver keys/lifetimes; no reference changes. | PA29 350/403, identical 53 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 222 behavioral checks and 222 inspection checks/groups; 832 final performance observations plus 24 launchers, with preliminary evidence retained. |
| 170 | `f07f7823..221d6d0e` (entry `ecb69d94`, three handoffs) | Reviewed all vector/inline, aggregate and block-pointer increments; fixed aggregate pointer dependency ownership and vector query initialization; no reference changes. | PA29 361/403, identical 42 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 376 behavioral and 377 inspection checks; 1,384 final performance observations plus six launchers, with historical evidence preserved. |
| 174 | `221d6d0e..7139ceb5` (entry `914e1a0a`, three handoffs) | Reviewed all intrinsic/fold/closure increments; fixed discarded conversions/lifetimes, unevaluated capture recipes and combined typed ABI identities/grammar; no reference changes. | PA29 377/403, identical 26 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 218 behavioral and 144 inspection checks; four-dimensional final/historical performance evidence retained. |
| 178 | `7139ceb5..667edd80` (entry `e59dcaa1`, three handoffs) | Reviewed all source-invocation, declaration/demand and selection increments; fixed declaration-array bounds, initializer definition/dependency ownership and complete default frames; no reference changes. | PA29 381/403, identical 22 failures and 403 inputs; PA1–28 4538/4538; file audit and controls/inspection pass; 776 final observations plus eight launchers, 816 inherited observations reviewed; mandated limits and coverage preserved. |
