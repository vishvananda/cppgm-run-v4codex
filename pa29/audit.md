# PA29 checkpoint audit182

Target: **PA29 full-stage**. Phase: **checkpointAudit complete; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `667edd800e4e5eb1b1ef92a3108da3bd96c51708`.
Audit entry: `9211517d1f554f61efa7d6e2e30020dc13171f3f`.
Last reviewed commit: `52070178897f5894edaf2f35d03a734b781979d4`.

Reviewed all **17 commits** from the previous review through the audit code tip:
three accepted implementation handoffs, their intermediate fixes/records, and the
audit fix. The combined range changes **79 implementation/build paths**.
[Range evidence](../student.tests/pa29/evidence182/range.json) retains every commit,
path and combined implementation-diff digest. [Audit178](audit178.md) preserves
the previous record and ledger verbatim. The code fix was validated and committed
first; this records commit makes no further implementation or harness edits.

Interruption revalidation found a clean entry tree, the unchanged previous review
marker, and no inherited build/test process. The interrupted audit is classified
as no progress; the next available action was the complete range review, followed
by the concrete fixes and validation below. No wait or stale lock was used as
completion evidence.

## Findings and owner repairs

1. **Array decomposition discarded source element qualifiers.**
   `binding_object_type` applied ordinary non-reference `auto` deduction to an
   array. `const int a[2]; auto [x,y]=a` therefore made `x` mutable; volatile
   arrays also lost volatile destination access semantics. The reduced
   [array-cv](../student.tests/pa29/source182/array-cv.cpp) assertion fails on entry,
   and [array-cv-write](../student.tests/pa29/source182/array-cv-write.reject.cpp)
   is incorrectly accepted. The shared owner now forms the hidden cv A array
   without removing A's element qualifiers. Ordinary non-array deduction keeps
   its existing rule. Canonical types carry the corrected fact through template
   occurrences, range elements, copy plans, projections and constant evaluation.

   This is the hosted structured-binding extension's
   [N4659 dcl.struct.bind/1–2](https://timsong-cpp.github.io/cppwp/n4659/dcl.struct.bind)
   hidden-array rule and referenced element type, not a claim that structured
   bindings belong to C++11. GCC/Clang outcomes in
   [entry proof](../student.tests/pa29/evidence182/entry-cv.json) corroborate the
   rule. Nine new controls cover cv identity, required rejection, dependent and
   range deduction, class copies, volatile counted loops, and all-handoff effects.
   Entry fails eight; final passes all nine. Zero-sized class copies retain
   constructor/destructor effects independently of their empty representation.

2. **Mandatory expansion had two phase owners across serialization.**
   Source lowering expanded calls and set an in-memory `forced_calls_expanded`
   flag, while the native adapter expanded a newly read Program again. Recursive,
   depth-limited and budget-limited calls survive a valid expansion; reading the
   source view therefore supplied another allowance. The reduced
   [recursive control](../student.tests/pa29/source182/inline-cycle.cpp) produces
   different source and adapter instructions at entry;
   [entry evidence](../student.tests/pa29/evidence182/entry-inline.json) records
   the extra expansion and both disassemblies. The defect concerns phase ownership
   and pipeline budgets even when both executions happen to return the same value.

   Native object preparation is now the sole production expansion owner.
   `--emit-lowir` renders its unprepared typed input, with original attributes;
   source object emission and explicit LowIR use the same transform once. This
   follows the [LowIR force_inline contract](../pa8/lowir.md), which assigns
   expansion to object preparation, and PA29's representative LowIR requirement.
   No new text protocol, metadata spelling, semantic reconstruction or fallback
   compiler is introduced. The existing same-Program guard remains.

   Telemetry follows the native owner: separate preparation time, prepared
   instruction/operand totals, inline work/reservation/decline counters and
   maximum caller reservation. Source lowering reports its original IR.
   The inspection180 assumption that source and prepared views were identical
   was replaced with validation/roundtrip of both views; object instructions,
   relocations, symbols, execution and stats-on/off equality remain required.
   This adds 13 inspection commands and changes no course oracle. New inspection182
   additionally requires exact direct/adapter inline-counter equality, including
   recursion, depth and growth fallbacks. All these checks pass.

No reference correction is needed. All contract inputs, reference sidecars and
comparison/discovery rules are unchanged. The reference bundle remains pinned to
source `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`. The earlier personal
[lifetime179](../student.tests/pa29/lifetime179.md) refinement remains documented
with its original reducer/results; it changed no course coverage. The zero-size
class host-effect disagreement in handoff181 remains disclosed rather than being
used to revise a reference or erase constructor/destructor requirements.

## Every commit reviewed

| Commit | Content and interaction reviewed |
|---|---|
| `cd283a59` | Audit178 records, reviewed marker, retained proof and inherited performance. |
| `a0a67bdf` | Typed local/member bindings and template/range projections; array deduction owner is repaired here. |
| `bcef72ad` | Array copy plans, constants, bounded cleanup and ordinary parameter/parser interactions. |
| `bf487a41` | Narrowing on referred types; completed-copy and default-temporary lifetime controls. |
| `259329ee` | Decomposition handoff, full suites, controls, scaling and performance evidence. |
| `6574d6c8` | Callable group entry scope and unchanged baseline. |
| `639ebd7f` | Static call/subscript selection, receiver evaluation, deduction, queries and typed lowering facts. |
| `a613b21a` | Mandatory inline CFG/value/slot/EH mapping; dual phase ownership is repaired here. |
| `95d43769` | Pre-mutation reservations, reused scratch, per-callee exception analysis and conservative fallbacks. |
| `f613c554` | Static/nonstatic template partial ordering and surrogate conversion ranking. |
| `c0efbf29` | Callable handoff with final/preliminary validation and performance. |
| `42a2bac7` | Zero-extent group plan and 16-failure entry evidence. |
| `e0619de9` | Canonical absent/zero array distinction, typed layout/completeness, lifetimes and native storage. |
| `f69601ef` | Immediate-context zero-bound rejection, nested parameter packs, static bounds and explicit adapter parity. |
| `0e47335f` | Wide object extents checked before packing, preventing 4-GiB wrap to zero. |
| `9211517d` | Zero-extent handoff; 388/403 validation and preserved preliminary/final measurements. |
| `52070178` | Both shared-owner repairs above; integrated reducers, source/adapter budgets, telemetry and audit check tools. |

## Architecture trace and correctness interactions

The nontrivial declaration and demanded template are in
[integrated.cpp](../student.tests/pa29/source182/integrated.cpp): `Z::empty` has a
known zero extent, and demanded `F<7>::operator()` / `F<8>::operator()` contain
array decomposition and mandatory inline calls with normal/throwing exits.

Immutable source buffers feed streaming preprocessing and syntax cursors. The
cooperating Parser/Analyzer publishes source names once and retains the parsed
body. Template instantiation uses canonical argument/frame identities and source
occurrence projections; it does not replay grammar. Non-dependent types and
binding shapes are shared, while actual local object/alias occurrences have their
own facts. Dependent-only publication and existing per-specialization demand
states retain selected constructors, destructors, conversions and function bodies.

`unknown_bound` participates in type hashing/equality, signature shapes and all
rebuilding operations. A known zero is never completed from an initializer as
though it were absent. Completeness is an explicit size-query result; element
alignment and zero bytes survive into typed object layouts. Immediate function
signature substitution owns zero-bound failure separately from valid GNU body
layouts; it visits dependent type edges and actual pack lanes. Parenthesized pack
discovery follows declarator edges without walking nested parameter lists.
Static array completion selects the indexed member definition without broad
class/body demand. These changes retain the earlier default/unevaluated/demand
separation instead of global retries or cache invalidation.

The hidden `source`/copy objects have canonical entity identity. Their binding
aliases store object/member/element and base-adjustment identities, not synthetic
frontend declarations or pretty-printed lookup keys. Shape caching is by canonical
type, and access is checked at each use. The fixed qualifier deduction feeds
both declaration and range owners. Array-copy initialization retains one selected
leaf conversion and its defaults; emission expands at most eight leaves, then
uses a counted loop and a completed-prefix cleanup cursor. Volatile accesses remain
volatile. Zero bytes suppress representation copies only after operand evaluation;
positive element counts still invoke observable class copy/destruction effects.

Static call operators retain receiver evaluation separately from explicit argument
conversions and have no implicit-object ABI parameter. Candidate filtering and
ordering use actual indexed candidates; a neutral static receiver does not hide
a better explicit conversion or user-defined surrogate cost. Selected declaration,
receiver/default/conversion and lifetime facts flow directly into lowering.
No rendered identity or repeated overload resolution is used by the backend.

The useful optimization facts are direct callee identity, mandatory attribute,
fixed boundary, selected conversions and unwind regions. The shared typed LowIR
expander consumes these facts, checks recursion/depth/frame legality and reserves
work before mutation. Immutable original bodies are mapped through compact
value/slot/block IDs; phi inputs name actual split-block exits, and returns retire
callee EH registrations. Unsupported or exhausted requests keep conservative
calls. There is no fixed-point whole-program rescan or optional profitability
search. Attribute expansion is required by the contract even at O0; its bounded
costs and inherited runtime benefits/regressions are disclosed in performance182.

Original IR pools and per-callee cost/return facts live only during preparation;
local mapping tables and scratch die on expansion return. Semantic/type/query
pools belong to each translation unit and are released after typed lowering.
The native selector/encoder constructs and releases one compact Function at a
time; only required symbols, object data, relocations and unwind/linkage facts
survive object emission. Existing arenas and flat indexes retain ownership;
no process-global mutable cache or per-instruction owning node allocation is added.
Production never writes/reparses text; explicit adapters are observation boundaries.

[Machine traces](../student.tests/pa29/evidence182/machine-traces.json) retain
source LowIR, prepared LowIR and selected MIR. Integrated `main` uses a 208-byte
native frame; retained operator bodies use 80 bytes, and the volatile ten-element
copy uses 96 bytes. These include actual homes/spills and cleanup state; smaller IR
is not asserted to imply better runtime. ELF symbol/relocation/unwind inspection
and host executions are retained. The original debug-location remapping property
is checked by inspection180's phi fixture. No unsupported claim of complete DWARF
or later-stage register-allocation quality is made.

## Validation and performance disposition

The [manifest](../student.tests/pa29/evidence182/manifest.json),
[validation](../student.tests/pa29/evidence182/validation.json) and
[source binding](../student.tests/pa29/evidence182/source-binding.json) bind all
471 implementation/build files, symlink targets, tested binary and commands to
`52070178`. Validation preceded the code commit; current commit bytes exactly
match the validated source. Source/fixture edits did not occur afterward.

- `make test-pa29`: **388/403**, exit 2, exactly the entry's **15 failures**.
- Exact prior-through shell command: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4926/4941**, exit 2; only PA29 fails.
- File audit: pass, exit 0, with the same four substantial-header warnings.
- Controls179/180/181: **42/47/43**, all pass; controls182: **9**, all pass.
- Inspections179/180/181/182: **91/264/166/132 commands**, all pass (**653 total**).
- The recursive/depth/growth source and explicit-adapter paths have identical
  code, symbolic relocations, symbol sets, runtime and expansion counters. The
  growth case declines 64 requests and remains within every admitted budget.

[Failure identity](../student.tests/pa29/evidence182/stage-delta.json) contains no
new failures or offsetting personal passes. [Coverage](../student.tests/pa29/evidence182/coverage.json)
preserves all **403 inputs** and **1,707 contract/harness paths** against both
previous-review and entry boundaries. Earlier-stage contract validation is full,
not a subset. No tests, references or comparison rules were weakened.

[Performance182](performance182.md) records all four dimensions at PA29/O0:
**440 new observations** plus eight launchers, and verified review of **2,576
inherited observations** (including all preliminary runs). Seven equivalent
benchmark object pairs are identical; executable code/data/unwind sections also
match. Source-corrected cv arrays have final-only scaling with all 24 compilation
counter checks passing. Noise/outliers and larger-case costs are retained without
an unsupported speedup claim. Mandatory expansion budgets and all existing limits
remain enforced. No optional transform was introduced.

Inherited blanket 15% compiler latency/RSS and zero-growth targets remain
**diagnostic under spec §9**. Earlier mandatory inline costs remain disclosed;
necessary semantic work and constraints owned by later stages create no extra
exit gate. No correctness, coverage or mandated limit is reclassified.

## Remaining work and handoff quality

The [remaining ledger](../student.tests/pa29/evidence182/remaining.json) groups
**15 failures** into extended syntax/types/layout **11**, template demand/hosted
ABI **3**, and legacy trait **1**. Twelve are unfinished implementation; three
remain independent contract questions (legacy nothrow shorthand, default-sensitive
nothrow invocability, nested ABI-tag policy). They remain counted failures.
Extended scalar/vector/complex representation, GNU declaration/expression forms,
contextual operators, char-traits conversion and hosted ABI should proceed as
broad owner groups. A valid standard/contract proof remains necessary for any
future reference correction. Full PA29/through success is required before PA30.

The three handoffs represent useful distinct owners and reduce 22 to 15 course
failures since audit178. However, handling array copy without its exact cv-deduction
rule, and splitting inline preparation from its serialized-budget boundary,
left avoidable follow-up fragmentation. Complete each owner through dependent,
cv/reference, lifetime/exception, ABI and explicit-adapter fallback checks before
handoff. This audit establishes one complete reviewed code baseline and leaves
the assignment unfinished; it does not narrow the full-stage objective.

## Audit ledger

| Checkpoint | Reviewed range | Findings / disposition | Evidence / result |
|---|---|---|---|
| 158 | `2734e5c6..1ab3499d` (three handoffs) | Alias/expression/template storage fixed; deleted-copy reference corrected with clause proof; historical performance targets classified under spec §9. | PA29 317/403, no new failures; PA1–28 4538/4538; file audit/controls pass; four performance dimensions retained. |
| 162 | `1ab3499d..cce8634c` (entry `9662716b`, three handoffs) | Fixed atomic bool RMW, reference snapshots, alignment/native fallback and cv/identity conversions; reviewed complete traits/invocation/atomic ownership range; no reference changes. | PA29 338/403, identical 65 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 221/221 explicit controls; native/LowIR/cache checks and 1,088 performance observations retained. |
| 166 | `cce8634c..f07f7823` (entry `3036bd4e`, three handoffs) | Reviewed all assembly, function-context and evaluation/storage increments; fixed effect invalidation, runtime extents, prvalue materialization and complete query receiver keys/lifetimes; no reference changes. | PA29 350/403, identical 53 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 222 behavioral checks and 222 inspection checks/groups; 832 final performance observations plus 24 launchers, with preliminary evidence retained. |
| 170 | `f07f7823..221d6d0e` (entry `ecb69d94`, three handoffs) | Reviewed all vector/inline, aggregate and block-pointer increments; fixed aggregate pointer dependency ownership and vector query initialization; no reference changes. | PA29 361/403, identical 42 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 376 behavioral and 377 inspection checks; 1,384 final performance observations plus six launchers, with historical evidence preserved. |
| 174 | `221d6d0e..7139ceb5` (entry `914e1a0a`, three handoffs) | Reviewed all intrinsic/fold/closure increments; fixed discarded conversions/lifetimes, unevaluated capture recipes and combined typed ABI identities/grammar; no reference changes. | PA29 377/403, identical 26 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 218 behavioral and 144 inspection checks; four-dimensional final/historical performance evidence retained. |
| 178 | `7139ceb5..667edd80` (entry `e59dcaa1`, three handoffs) | Reviewed all source-invocation, declaration/demand and selection increments; fixed declaration-array bounds, initializer definition/dependency ownership and complete default frames; no reference changes. | PA29 381/403, identical 22 failures and 403 inputs; PA1–28 4538/4538; file audit and controls/inspection pass; 776 final observations plus eight launchers, 816 inherited observations reviewed; mandated limits and coverage preserved. |
| 182 | `667edd80..52070178` (entry `9211517d`, three handoffs) | Reviewed all decomposition, callable/inline and zero-extent increments; fixed array cv deduction and single native preparation ownership; no reference changes. | PA29 388/403, identical 15 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 141 controls and 653 inspection commands pass; 440 new observations plus eight launchers, 2,576 inherited observations verified; budgets and coverage preserved. |
