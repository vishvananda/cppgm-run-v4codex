# PA17 checkpoint audit — Ralph loop 60

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `2290a7bf8b56bc33e6ad975a8ae714a341f55ce5`

Target: **PA17 full-stage**. This checkpoint audit is complete; implementation
still has three course failures and must not advance. Entry `119fa9fe` was clean,
**340/343 with three failures**. The reported check status 2 was make's exit code,
not a two-case failure baseline. The preceding goal turn is classified as
progress from the committed loop-59 storage handoff and its validation; process
inspection found no remaining compiler/test job at entry.

Reviewed **every commit and the combined changes in `e14b96fa..2290a7bf`**:
15 commits, three accepted implementation handoffs, two audit implementation fixes,
one evidence verifier correction and **40 implementation paths**. The [range manifest](../student.tests/pa17/checkpoint60-range.json)
records every commit, changed path and implementation patch hash, including the
combined patch. The [previous audit](audit-loop56.md) remains preserved. Read
`spec.md`, the PA17 handout, compact plan, testing/reference rules, all handoff
records, the accumulated source patches and their neighboring owners, controls,
benchmark harnesses, historical samples, and reference proofs.

| Commit | Review and interaction |
|---|---|
| `69152966` | Previous audit records; preserves the e14b96fa code baseline, previous controls and performance observations. |
| `4e82009f` | Specialized stores, empty subobject transfers, automatic array initialization and canonical base adjustments. Checked their conversion, layout, constant-address and lowering consumers together. |
| `794b150e` | Separates base reachability from layout demand after the fixed-receiver preflight failure. Checked immutable pattern/concrete keys and cached misses, shared tails and deferred layout. |
| `411ad00e` | Transfer handoff and frozen observations, including the failed preflight. Its qualified receiver review question exposed the incomplete call and constexpr paths below. |
| `7d04e8ce` | Active incomplete candidate tuples, default holes, immediate query failures and deleted declarations. Reviewed recursive ADL/default demand with selected class-definition side effects. |
| `1e5f15ab` | Explicit specialization replaces deletion ownership; flag packing restores 120-byte entities. Both the intermediate and final layouts were reviewed. |
| `f284514b` | Fully deduced candidates bypass redundant default work while the ordinary specialization owner still validates its tuple. |
| `7cc89281` | Query handoff, inherited/new controls and all three performance campaigns. Active keys, query failure caching and context restoration were checked across member and partial heads. |
| `8654829c` | Storage plan; preserves the review marker, full-stage scope and reference-proof requirement. |
| `6a1a51a8` | Static definition signatures, immutable read snapshots and function-address queries. Reviewed source/instantiation boundaries, concrete access contexts and query/call consumers. |
| `3c7eeea2` | Six surgical reference corrections; independent standard/contract verification described below. No source, status, coverage or comparator changes. |
| `119fa9fe` | Storage handoff and complete frozen evidence. The entry verifier passes against this checkout; its claim of a closed owner was incomplete at the address-query naming boundary. |
| `1f319dac` | First audit fix: qualified receiver paths and query naming/access provenance, with 30 reducers. Its complete performance campaign is preserved, but later tracing found the constexpr consumer still incomplete. |
| `3ad8f2f4` | Completes constant and query receiver consumption, ABI qualification and deferred emission dependencies. Includes 43 audit controls, final validation harnesses and expanded benchmark inputs. Final compiler changes and frozen benchmark source. |
| `2290a7bf` | Evidence-verifier correction: preserves inherited LowIR required/forbidden patterns, verifies their outputs, and accepts only ancestor campaigns with identical compiler sources and binary hashes. No compiler behavior changes. This is the reviewed code tip. |

## Findings and fixes

**Qualified subobjects.** Candidate viability tested the whole object's direct
path to the declaring base before consuming the explicitly named intermediate
class. It rejected `object.Right::f()` when `Left` and `Right` each contained a
`Base`, although both selected path segments were unambiguous. The ordinary and
retained fixed call paths also attempted that ambiguous direct projection before
overwriting it. They now share `record_member_receiver`; viability checks both
graph segments, and semantic selection records those segments once. Ordinary
lowering consumes completed offsets. Queries record graph paths without demanding
layout. Access, cv/ref ranking, static calls and virtual suppression retain their
own checks. N3485 **5.2.5 [expr.ref]/5** and **11.2 [class.access.base]/5–6** state
these two naming-class boundaries; the whole-object direct path is not the rule.

**Constant evaluation and typed queries.** Runtime lowering already consumed two
recorded adjustments, but constexpr calls reconstructed a receiver by searching
for the declaration's class, selecting the wrong repeated base. Constant calls
and query calls/fields now project through the same recorded edges into canonical
constant subobject addresses. A query previously retained only the final member
name; it now retains the typed explicit qualifier through substitution and
lookup. This fixes both a `static_assert` and a non-type argument using the same
qualified call. The ABI adapter preserves that qualifier through typed expression
nodes. Existing unresolved-name encoding now accepts named template qualifier
levels, without text keys or reparsing. The
[ABI checks](../student.tests/pa17/checkpoint60-abi.json) verify ordinary and nested
template qualifiers against the checked-in Itanium **5.1.6 expression and
unresolved-name grammar**; demangling is supplementary evidence.

**Address-query access.** `decltype(&Derived::f)` checked the declaration owner
instead of the naming class and admitted an otherwise public function through a
private base. Canonical name queries now retain an interned naming scope in their
key; substitution maps it along with the access context. Query facts carry that
scope to selected address/call and target-conversion checks. Signature-only
normalization omits access/naming environments while preserving bound entities
and canonical head types; concrete checking still uses the original query.
Private-member, private-base, friend and public using-declaration controls cover
both single functions and overload families. N3485 **11.2/5–6** supplies the rule.
Entity and query records remain **120 and 48 bytes** respectively.

**Emission after constexpr checking.** A body checked first for constant evaluation
could later be emitted with an undefined callee symbol, because its earlier
unevaluated context had suppressed runtime demand. A TU-owned flat
`(body entity, selected callee)` index now records deferred uses. First emission
demand activates only that body's edges; newly recorded edges of an already
requested body enqueue directly. A deduplicated worklist processes each edge once
and terminates on cycles. The body's entry evaluation depth distinguishes its
potentially evaluated uses from deeper `sizeof`/`decltype` operands. No body scan,
semantic reconstruction, global retry or eager dormant definition is needed.
The controls exercise transitive/member/free-template calls, demand before and
after checking, cycles, unused erroneous definitions and unevaluated operands.
This separates definition checking from emission as spec.md §§4–6 require and
preserves the instantiation boundary in N3485 **14.7.1 [temp.inst]/2–3,8,10**.

The [43 audit reducers and neighbors](../student.tests/pa17/checkpoint60_controls.py)
fail **18/43** at entry and pass **43/43** at the reviewed tip. All **567 inherited
controls** also pass, for **610/610**. Expected successes execute through the
supplied native backend; rejections have ordinary nonzero compiler statuses.
Coverage also includes query publication before/after added defaults or overloads,
member/partial default holes, distinct bound signature names, const receivers,
nonzero offsets, alias qualifiers and repeated queries. No old control was removed
or weakened. Broader PA18 deduction/SFINAE completion is not claimed by this audit.

## Architecture and optimization audit

The [combined trace](../student.tests/pa17/checkpoint60-trace.json) follows
`Both<Right>::add`, `TableOwner<Right>::fn`, the selected `Probe<Right>` partial,
sparse transfer, and `ConstantBoth<0>` through source, typed LowIR and checked ELF.
Immutable source buffers feed the streaming preprocessor/post-token cursor and
integrated parser/analyzer in `lowering/driver.cpp`. Parsed patterns remain shared
source recipes; projected occurrences retain source/context identities rather
than cloned trees or replayed grammar. Canonical identifiers, entities, types,
argument slices, heads and immutable parent-linked frames own semantic identity.

Scope/name and primary/partial indexes limit lookup and selection to relevant
owners. Candidate active keys include the canonical head and incomplete tuple,
including default holes; marks restore on scope exit and are not cached negative
results. Class-definition side effects suspend immediate-context probe modes.
Query results retain structured expected failures; context and naming provenance
participate in canonical identity. Declaration insertion/default controls and
repeated cv/namespace queries verify the relevant cache boundaries. No broad
invalidation, global generation counter or unrelated retry was introduced.

Definition signatures attach to indexed source prototypes. Static definition
application retains its `(specialization, source definition)` monotonic state;
entity-keyed storage demand preserves dormant members. Signature normalization is
memoized and remains separate from concrete access checking. Ordinary O0 reads
snapshot at their source use, instantiated reads after their required demand;
member lowering consumes the snapshot, not a subsequently published entity value.
Selected address targets feed typed initializer/relocation facts. Base graphs
publish before member checks; concrete and retained-pattern identities are distinct.
Graph reachability and layout remain separate cached facts. The new deferred
emission queue follows explicit selected-callee edges only.

All added indexes, vectors and records are analyzer/TU owned. Scratch candidate,
query-child, initializer and signature sequences release at their owner returns.
No per-node owning smart pointer, deep environment copy or process-global mutable
cache was added. Each function lowers once from recorded declarations,
conversions, paths, layouts, lifetime actions and ABI entries into typed LowIR;
function-local builder state releases per function. The typed program survives
for the explicit LowIR writer. The pinned native backend is an authorized
validation consumer; own MIR, register allocation, ELF emission and self-hosting
remain PA24–PA34 responsibilities.

Optimization review covers the entire transfer/query/storage range. Specialized
wide stores preserve target width/sign; empty trivial subobjects retain memberwise
actions, while nontrivial, volatile, reference, union and array behavior keeps its
own plans. Automatic constant-array selection scans explicit initializer nodes,
not expanded bounds, and retains evaluated class materializations. Short wide
arrays use the existing eight-lane limit (at most 24 conversion/address/store
instructions); larger arrays retain bulk/loop fallbacks. Nested expansion budgets
remain shared. Constant evaluation retains its **1,000,000-work / 512-depth**
limits. Selected graph paths replace searches; emission uses a monotonic queue.
No optional optimizer, fixed-point rescan, speculative growth, ABI relaxation or
debug-policy change was introduced. Missing required facts still diagnose rather
than trigger name-based lowering recovery.

The trace follows a selected constant/subobject fact through legally recorded
projections, constant execution or ordinary O0 call emission, final ABI naming
and decoded native instructions. Profitability is evaluated separately in the
[performance report](checkpoint60-performance.md): prior sparse-transfer and
wide-array outcomes, unchanged call/memory/floating controls, static-read cost,
and newly required receiver/emission behavior are all retained. Smaller IR alone
is not evidence of runtime profit.

## References and validation

Independently verified all six corrections in
[storage-references.md](storage-references.md), the reducer sources and pinned
bundle **c2f713cd70d06170632bfde3e75dd6fe1aa44d98**. The correction reproducer derives
only the six explicit edits from frozen original oracles, without copying student
output. N3485 **3.6.2/2**, **5.19/4** and **6.7/4** require constant initialization
of the reference/function-address cases; **8.5/6,8** requires empty-class value
zeroing, including padding; **14.7.1/1–2,8,10** prohibits unused static-member
instantiation. Reduced observations show delayed reference binding, unwanted
initialization, dynamic guards and missing zeroing in the reference. Agreement
with a compiler is not the proof. Preserve the dynamic referent constructor,
selected function definitions, distinct specializations and ordered lifecycle
work. No additional reference correction was made in loop 60.

Final `make test-pa17`: **340/343**, exactly the entry's three failures.
`make test-report-through-pa16`: **2266/2266**.
`make test-report-through-pa17`: **2606/2609**, all failures in PA17.
File audit passes with the same three inherited header-division warnings.
The final evidence verifier also rechecks inherited LowIR-property controls, whose
record schema differs from native/rejection controls. Its correction follows
the compiler freeze; both campaigns remain at `3ad8f2f4` with identical `dev/`
content and compiler hash at the reviewed tip. No timing observation was edited.
All **343 course inputs**, expected statuses, earlier suites and comparison rules
are unchanged; only the six previously proved reference outputs differ from the
last review. The [evidence manifest](../student.tests/pa17/checkpoint60-evidence.json)
binds logs/exit statuses, source digest, full range, exact failures, coverage,
controls, historical integrity, ABI/ELF trace and frozen observations. Run
`python3 student.tests/pa17/verify_checkpoint60.py` to verify it.

The [compact plan](plan.md) keeps two unfinished implementation groups: **closure
entities (one case)** and **exception cleanup scheduling (two cases)**. All
accumulated independent review questions are closed by this audit; these three
implementation failures remain obligations. Avoidable fragmentation split shared
receiver/query/emission facts across the transfer, query and storage handoffs.
The first audit increment also stopped too early at runtime lowering; following
the same facts into constexpr execution exposed the remaining consumers. Future
handoffs should close a broad semantic owner together with its source, query,
constant, emission, ABI and lowering consumers.

| Loop / phase | Reviewed range | Findings and disposition | Validation / remaining work |
|---|---|---|---|
| 60 / checkpointAudit | `e14b96fa..2290a7bf` (entry `119fa9fe`; 15 commits, 40 implementation paths) | Fixed qualified receiver paths, query access/qualifier provenance, constant consumption, ABI qualification and deferred emission dependencies; verified six prior reference corrections and stage-scoped performance evidence. | Earlier 2266/2266; PA17 340/343, same three failures; file audit pass; 610 controls. Closure and cleanup groups remain; no stage advancement. |
