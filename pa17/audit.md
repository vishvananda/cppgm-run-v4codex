# PA17 checkpoint audit — Ralph loop 56

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `e14b96fa9d4b3376e5922b8ad30093c3c0b0c759`

Target: **PA17 full-stage**, implementation still incomplete. Reviewed the entire
`c43e8eb68db7e9b3f1dd0bbb18f14c92e4fc4b30..e14b96fa` range, including all
three accepted handoffs and both audit fixes: **10 commits and 37 implementation
paths**. Entry `b739e08d` was clean, **324/343 with 19 failures**. The preceding
goal turn was progress, confirmed from committed loop-55 implementation and the
entry checkpoint log. The [previous audit](audit-loop52.md) remains preserved.

The [range record](../student.tests/pa17/checkpoint56-range.json) lists every
commit, touched path and implementation patch hash, including the combined diff.
Reviewed `spec.md`, the assignment README, compact plan, testing/reference rules,
all accumulated implementation patches, all three handoff ledgers and their
benchmark/validation harnesses. Contract fixtures, references, bundle revision,
comparison rules and course coverage are unchanged. No reference correction or
waiver was made.

| Commit | Review and interactions |
|---|---|
| `65b02f17` | Previous audit records; preserved the code baseline, full-stage obligations, historical measurements and stage-scoped acceptance. |
| `0a720134` | Shared exact class/variable partial selection; canonical tuple/default/pack queries; member-partial source heads and lexical frames; qualified argument lists and constant boolean lowering. Found a missing type/template category in qualified-name identity and invalid-type failure mishandling. |
| `3f1f6f0b` | Selection handoff and frozen observations. Checked explicit semantic costs, unchanged controls and the disclosed boolean-branch runtime/text change. Its assertion that no argument/substitution defect remained was too strong; the audit reducers below expose two. |
| `5c5756b7` | Anonymous class injection and storage/access/base paths; constructor/conversion parsing and explicit-condition query/rebinding; array list materialization; bounded selected-union zero stores. Checked concrete versus source pack contexts and typed lowering ownership. |
| `4d1527e7` | Publication handoff, controls, scaling and both original/repeated observations. Preserved noisy samples and scoped later-backend constraints; no numerical diagnostic becomes an exit gate. |
| `deb07c4e` | Current-instantiation dependent-base lookup/access, early inline namespace import, and fixed-position argument expansions. Reviewed with partial selection and retained member publication; source sequences normalize into target arguments only after expansion. |
| `df97733f` | Restores immediate rejection of a known type/value pack mismatch while preserving unresolved suffixes. Reviewed the intermediate permissive change as well as the combined result. |
| `b739e08d` | Qualified handoff, exact 19-case failure set, 439 controls, frozen campaign and repeat. Carried every pending review question from all three increments into this audit. |
| `d37a1049` | Audit fixes for lookup category, cv-preserving failure propagation and expected candidate mismatch; 26 reduced controls and cumulative benchmark harness. |
| `e14b96fa` | Follow-up cache validity: negative type/frame entries require a completed qualifier. Active-class members may still be published; such misses remain uncached. Final checks and complete measurements use this tip. |

The audit fixed these ownership defects:

- A retained dependent name encoded only whether it had an argument list.
  Supporting a member alias template as a template-template argument consequently
  also accepted `typename T::A` when `A` was a template. A function returning
  that invalid bare-template pointer could compile and execute. A compact
  `DependentNameKind` now distinguishes a type, applied template-id and template
  entity in the canonical key. Argument construction supplies the category;
  substitution, source-head normalization and typed ABI views preserve it.
  Lookup enforces the requested category without rendering or reconstructing
  names. N3485 **14.6 [temp.res]/3–4** requires a typename-specifier to denote a
  type; **14.3.3 [temp.arg.template]/1** separately permits class/alias template
  names as template-template arguments.
- Expected qualified lookup failure returned type zero, but the caller applied
  cv-qualification to it. Since the sentinel record has a default fundamental
  descriptor, this manufactured `const int`, `volatile int` or `const volatile
  int` and could select the wrong partial specialization. Substitution now
  propagates failure before qualification. The alias fact records failure before
  checking access on a nonexistent substituted type. Successful aliases still
  perform their definition-owned access checks. This applies the exact-match
  obligation in **14.5.5.1 [temp.class.spec.match]/1–2** and the invalid-type
  rules in **14.8.2 [temp.deduct]/8**.
- The same partial-selection probe threw on non-class qualifiers or an ordinary
  member where a template was required, aborting selection before the primary
  fallback. These category mismatches now return the existing compact failed
  probe result. Hard uses remain diagnosed. Completed dependent-name failures
  share the immutable type/frame cache with successes via a reserved failure
  value; missing bindings and active-class members do not establish negative
  entries. This closes the affected PA17 partial-matching path without claiming
  completion of PA18's broader SFINAE surface or the remaining recursive-query
  protocol. **[temp.class.spec.match]/2** invokes deduction; **[temp.deduct]/8**
  explicitly lists non-class qualifiers and wrong member categories.

The standard citations refer to the checked-in [N3485 text](../doc/n3485.txt).
The [reducers and neighbors](../student.tests/pa17/checkpoint56_controls.py)
cover valid ordinary/applied/template names, inherited template arguments,
const/volatile failures, repeated alias failure, viable fallback, hard rejection,
category use order, pack/default application, conditional constructors and member
variable partials. Entry fails **11/26**; final passes **26/26**, including all
expected native results and normal rejection statuses. All **439 inherited
controls** also pass. No test or comparison was removed or loosened.

Architecture audit: immutable source buffers feed the streaming preprocessor,
post-token cursor and integrated parser/semantic construction in
`lowering/driver.cpp`. Inline namespace import is established at namespace
opening. Parsed template definitions remain source recipes; projected nodes carry
context IDs rather than cloned subtrees or replayed grammar. Identifiers, source
heads, entities, types, argument slices, query IDs and parent-linked frames own
canonical identity. The new name-category discriminator participates in type
interning, substitution, head equivalence and ABI adaptation; rendering remains
a view. Qualifiers and argument sequences are substituted only when dependent.

Selection visits the primary's indexed class/variable partial list. Exact
matching follows candidate shape and emitted pack lanes; winner selection and
verification take O(C) pair comparisons. Immutable candidate-pair ordering facts
are cached; argument-dependent coverage stays local to the match and uses
O(n log n) sorting only when multiple candidates compete. No global template
scan, all-class retry, text key or environment copy was added. Out-of-class
member definitions attach through existing indexed source-head/prototype edges;
selected outer and inner parameter slices remain distinct. Query packs retain
explicit arguments and symbolic counts until their enclosing frames resolve.

Anonymous members use explicit injection edges with direct-name precedence,
separate storage entities and enclosing access paths. Semantic base adjustment
is recorded as a byte offset; lowering adds recorded nested storage offsets
without repeating lookup or overload selection. Explicit conditions are canonical
queries rebound on renamed source heads and evaluated per specialization;
nondependent results are reused. List initialization consumes recorded conversion,
construction and lifetime plans, retaining a concrete pack lane's own frame.
Declaration publication, class completion, body demand, constant execution,
layout and emission remain separate facts. Qualified-member positive caches
publish only after class completion; the new negative substitution cache applies
the same completion constraint. Access facts retain recipe/frame identity.

TU-owned source buffers, interners, node slabs, flat indexes and fact vectors
release with the analyzer. Candidate/argument scratch vectors release at their
owner return; immutable frames share parents. No per-node owning smart pointer,
process-global mutable cache, second semantic tree or retained serialized IR was
introduced. Function lowering consumes selected declarations, conversions,
layouts, lifetime actions and ABI entries directly into typed LowIR; its local
builder state releases per function. The typed program survives for the explicit
requested LowIR writer. The [trace](../student.tests/pa17/checkpoint56-trace.json)
binds a nontrivial member-template declaration and demanded primary/partial,
source/IR/ELF hashes, untimed counters, native result and decoded instructions.
The pinned supplied backend is an explicit validation consumer, as PA17 requires;
own MIR, allocation, ELF writing and self-hosting belong to PA24–PA34.

Optimization audit: recorded nonvolatile boolean constants can select an O0
branch in O(1), without evaluating effects, scanning expressions or adding code.
All other forms retain ordinary lowering. Selected-union aggregate zeroing uses
only a semantic child plan that proves representation zeroing legal; scalar,
volatile, member-pointer and other distinct plans keep their own actions. Small
regions emit at most eight stores and seven internal address operations; larger
regions retain bulk zeroing. Nested array expansion retains the aggregate
8-element budget and counted-loop fallback. Constant execution keeps its
1,000,000-work/512-depth limits. No new optimizer, fixed point, speculative
analysis, ABI/debug relaxation or unbounded growth was introduced. The useful
constant fact in the trace flows from selection through typed lowering to native
encoding; runtime workloads keep live calls, memory and floating-point work.

[Performance evidence](checkpoint56-performance.md) reports compiler latency/RSS
and separate checked executable runtime/text together, with frozen checkpoint and
cumulative A/B binaries, flags, sources, A/A calibration, ABBA pairs/spread and
scaling counters. All historical campaigns and noisy observations remain intact,
as does the explicitly interrupted pre-cache-validity campaign. The boolean
branch's historical runtime regression and union zeroing's measured benefit are
both disclosed; smaller IR alone is not treated as profit. PA17/O0 has no mandated
numerical ceiling. Inherited +15%, +16 MiB and 5.5× diagnostics do not add exit
gates; required semantics, coverage, existing work/growth bounds and avoidance of
unnecessary work remain binding. Later backend constraints do not waive PA17
correctness.

Validation: `make test-pa17` is **324/343**, exactly the entry's **19 failures**.
`make test-report-through-pa16` passes **2266/2266**. The through-PA17 report is
**2590/2609**, with all failures in PA17. File audit passes with the same three
inherited header-division warnings. The
[evidence manifest](../student.tests/pa17/checkpoint56-evidence.json) binds the
code/range, logs and exit statuses, exact failures, 343 course inputs, 465
controls, historical integrity checks, trace and measurements. Run
`python3 student.tests/pa17/verify_checkpoint56.py` to verify it.

All **19 failures remain implementation obligations**, grouped broadly in
[plan.md](plan.md): query/candidate demand and closures (4), static storage and
initialization (9), scalar/object transfer and adjustment (4), and exception
cleanup scheduling (2). All accumulated independent review questions are closed;
that does not mean the stage is complete. Avoidable handoff fragmentation split
selection, publication and qualified-name/argument facts across three increments,
delaying discovery of shared category and failure defects. Future handoffs should
close a broad owner together with its declaration, substitution, demand and
lowering interactions.

| Loop / phase | Reviewed range | Findings and disposition | Validation / remaining work |
|---|---|---|---|
| 56 / checkpointAudit | `c43e8eb6..e14b96fa` (entry `b739e08d`; 10 commits, 37 implementation paths) | Fixed dependent-name categories, cv failure corruption, expected partial mismatch and negative-cache validity; reviewed all three handoffs and their interactions; stage-scoped performance accepted with all observations preserved. | Earlier 2266/2266; PA17 324/343, same 19 failures; file audit pass; 465 controls. Four broad implementation groups remain; no stage advancement. |
