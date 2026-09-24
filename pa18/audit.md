# PA18 checkpoint audit 70

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Previous Last reviewed commit: `3a883d10a27e41d1b126eaef05eaf0b454de1646`.
Last reviewed commit: `f59e8f67cd8c832361130aef9af1a0337b25c45d`.
Entry: `15b34993c14b10a196c90b876b76d1658da129ba`.

**Checkpoint audit passes; PA18 full-stage implementation remains incomplete.**
All 13 accumulated commits since the previous reviewed code tip, their combined
changes and cross-handoff interactions were reviewed, followed by the audit fix.
The final combined range changes 42 implementation/registration files.
[Evidence](../student.tests/pa18/loop70-evidence.json) records complete commit IDs,
patch/source/fixture hashes, failure sets, commands and results. The first audit
and earlier ledger remain in [audit66.md](audit66.md).

## Accumulated range

| Commits, chronological within each group | Review and interactions |
|---|---|
| `06211ad0` | Prior audit records; retains the reviewed source boundary, historical measurements and mandatory remaining work. |
| `626b5809`, `d5497ec7`, `f2cd086f`, `0b60ca52` | Typed pointer/reference/function NTTPs, source-form restrictions, canonical storage identities, definition demand, lexical frames, ABI substitutions and cross-TU internal linkage. Reviewed the initial implementation and later cache/access correction together. Completed address facts include query, target and access modes; incomplete facts retain query dependencies. |
| `047215cf` | Address handoff, controls and frozen performance; PA9 correction proof independently checked. |
| `3c78f083`, `e0cc21b4`, `bd1d7b4c` | Partial explicit frames, deduction before defaults, non-deduced lists/pack lanes, constexpr temporary identity, isolated direct/base trials and array/reference cv. Reviewed canonical-primary filtering before allocating trial bindings and interactions with address signatures and ordering. |
| `e09162fa` | Deduction handoff and measurements, including preserved pre-filter and prior-suite regression observations; final results supersede those attempts. |
| `f2a9d7a3`, `6073dbc0` | Retained assignment/destructor queries, ordinary single-evaluation lowering, implicit destructor properties, template-id parsing, source ABI and corrected pseudo-destructor rejection. Findings A–C below repair interacting ownership gaps. |
| `15b34993` | Query handoff, controls and frozen observations; all pending review obligations included here. |
| `f59e8f67` | Audit fixes, 56 independent controls, combined architecture trace and benchmark harness. Required checks passed before committing this code tip; this record changes no compiler code. |

## Findings and fixes

**A — builtin assignment candidates lost qualification and ranked the wrong types.**
The new compound-assignment owner decayed class conversion results, erasing
volatile on the modified lvalue. Arithmetic candidates ranked their RHS against
the arithmetic result type, and pointer candidates used the RHS promotion
instead of `ptrdiff_t`. This changed overload participation, not only metadata.
It also offered a class-to-bool/reference plus pointer candidate absent from the
builtin overload set. Reduced controls check ambiguity versus user overloads,
volatile reference/result identity, pointer ranking and effects exactly once.

`builtin_operand_types_value` now preserves lvalue-reference result cv for the
modifying consumer. `assignment_operators` records the promoted RHS parameter
and arithmetic result separately; pointer candidates use the x86-64 `ptrdiff_t`
type. The ordinary nonclass bool/pointer expression remains valid, while class
operands only get the specified builtin candidates. Lowering consumes the selected
conversion once, then converts values for the recorded arithmetic computation.
No semantic search, fabricated source node or extra evaluated operand is added.
Proof: N3485 §13.6 [over.built]/18,21,22 (`doc/n3485.txt:17132`), §5.17
[expr.ass]/7 and §13.3.3 [over.match.best].

**B — scalar pseudo-destruction dropped arrow effects; query arrow checks demanded too much.**
Concrete scalar destructor binding returned before recording the selected arrow
chain. Lowering consequently skipped `operator->` and its temporary cleanup;
`noexcept` could also become true incorrectly. Retained query exception analysis
ignored its existing arrow chain and intermediate result destruction.
`expression` now retains the ordinary object-use record, lowering calls the
ordinary `arrow_object` consumer, and query exception analysis consumes selected
arrow functions/result types. Both concrete and retained controls check throwing,
nonthrowing, nested, reference-return and temporary-return chains; execution checks
call counts and exactly one temporary destructor.

The same audit found that query-only `prepare_arrow` used layout demand and C++
exceptions for expected rejection. It now completes only required declarations,
records declared base paths without laying them out, checks deletion/access as
candidate state, and records incomplete-class query dependencies. Required class
body errors remain hard errors. Missing/private/deleted/nonpointer/recursive arrow
cases discard candidates; completing a forward class allows the affected query
to succeed without retrying unrelated facts. N3485 §5.2.4 [expr.pseudo]/1,
§13.5.6 [over.ref], §5.3.7 [expr.unary.noexcept]/3 and §14.8.2 [temp.deduct]/8
establish the receiver effects and immediate-context boundary.

**C — defaulted-destructor deletion cached caller privileges.**
An out-of-class member's access override leaked into a supposedly
context-independent deletion fact. The compiler accepted destruction of a class
whose member's destructor was inaccessible to that class, or rejected the reverse
case when the owning class was the friend. Two independent out-of-class reducers
and their positive friend variants expose both directions at entry.
`default_destructor_facts` now isolates ordinary and retained-pattern property
computation from naming exemptions and caller access overrides, while keeping
use-site access checks separate. The destructor's declaration remains the complete
cache key; no global epoch, extra context variants or cache flush is needed.
N3485 §12.4 [class.dtor]/5 (`doc/n3485.txt:14281`) specifies access from the
**defaulted destructor**, not its caller.

[Audit controls](../student.tests/pa18/audit70_controls.py): **56/56 pass**;
**34 fail on the frozen entry**. All valid cases run through LowIR validation and
native execution; rejection cases check status, never diagnostic text.

## References and coverage

Only two required oracle files changed in the accumulated review range. Both
corrections retain their input, coverage and comparison rule, and are accepted:

- [PA9 external-entity substitution](reference-correction67.md): reduced typed ABI
  and C++ input; PA9 explicitly requires Itanium substitution order. The ABI's
  [compression rule](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-compression)
  includes names inside expressions and inserts components before composites.
  The reducer's `C` enters slot `S1_`, making the final reference `RS1_`.
  The corrected exact symbol is exercised through the shared encoder.
- [Scalar receiver noexcept](reference-correction69.md): the receiver's declared
  potentially throwing call is evaluated; scalar destruction does not erase it.
  N3485 §5.2.4/1, §5.3.7/3, §15.4/12 and §7/4 require rejecting the assertion.
  The reduced and original inputs again show pinned-reference acceptance and
  student rejection. The empty failed-output file remains informational.

Both records pin bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`; the bundle is
unchanged. Compiler agreement is supplementary, never the proof. Audit 70 changes
no required fixtures, references, harnesses or statuses. The earlier constant
initialization correction is unchanged and was reviewed in audit 66.

## Architecture and optimization trace

[Combined source trace](../student.tests/pa18/audit70_trace.cpp) follows ordinary
`main`, `advance<Proxy,&target>`, partial explicit array deduction and scalar
destruction through the real pipeline. `lowering/driver.cpp` retains immutable
preprocessor source buffers and streams `PostTokenCursor` into the syntax cursor;
`translation_unit(&sem)` constructs source and semantic facts together.
Template instantiation projects source/context occurrences from parsed regions;
it never invokes the parser. Stable TypeId/EntityId/QueryId, canonical argument
packs, immutable parent-linked frames and local overlay bindings own meaning.

The NTTP address conversion records `target`'s canonical storage/declaration and
queues its required definition. Partial deduction selects `length<int,2>` before
the default 5. The assignment records its converted reference and computation;
the destructor records the selected arrow action. Each demanded body has one
monotonic state. Nondependent recipes reuse source facts; unused `dormant` has
no emitted body. The trace has exactly seven function definitions, including
one selected `advance`, `destroy`, `length`, reference conversion and arrow
operator, and executes with checked exit zero.

The address cache includes access mode; successful entries are published only
with complete prerequisites. Substitution uses complete query/frame keys.
Query completion retains class→query→consumer reverse edges and local revisions
from audit 66. Repeated 32/128/512 address uses inspect three candidates total;
completion controls invalidate only the affected consumer. Base deduction walks
explicit edges with a visited set and isolates trial bindings after primary
identity filtering. No rendered type, symbol or serialized graph becomes a
semantic key; no global retry or process-global mutable cache is introduced.

Direct typed LowIR consumes the recorded facts. ABI graph nodes consume entity,
type, argument and source-expression identities; the shared encoder owns one
symbol's substitution table. The LowIR writer is the required PA18 output adapter,
not production phase transport. Full validation is explicit via `--validate-lowir`.
The supplied native backend consumes this LowIR and produces the checked ELF.
Own selection/allocation/object encoding and self-hosting remain PA24–PA34;
this audit makes no claim of implementing those later phases.

Storage: canonical facts, query/arrow records, frames, graph nodes and dense indexes
belong to the TU. Candidate bindings, visited sets and worklists are local scratch.
The driver releases frontend/semantic/lowering scratch at each TU boundary;
only output LowIR, ABI/linkage and lifecycle facts survive to writing. No new
per-node owning allocation or shared-pointer graph is introduced. Work follows
required conversion candidates, type/query operands, base edges and demanded
subobjects; arrow traversal detects repeated selected functions and rejects cycles.

A useful fact is the selected lvalue reference and the receiver's observable
arrow call. Lowering consumes their identities once and preserves effects,
volatile accesses, cleanup, ABI and source locations. These repairs are mandatory
semantics, not optional optimizations. O0 adds no fixed-point pass, inlining,
speculation or code-growth search. Absent proof stays conservative; changed
selection and access prerequisites belong to their semantic owners, and no
cached completed lowering fact is rewritten by an optimization. Existing bounded
array handling is unchanged. [Performance 70](performance70.md) reports the
complete frozen checkpoint/review-range compiler and executable evidence.

## Required validation and remaining work

| Check | Result at the reviewed code tip |
|---|---|
| `make test-pa18` | **353/420**, exit 2; exactly entry's 67 failing paths: 43 status failures, 24 LowIR mismatches. |
| Required prior-through command (`n=18; … make test-report-through-pa$((n - 1))`) | **2609/2609**, exit 0. |
| `perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src` | Pass, exit 0; same three inherited header-division advisories. |
| Coverage/comparison | All 420 source inputs and entry fixture hashes unchanged; no audit changes to course/reference/harness files. |
| Personal semantic controls | **330 inherited + 56 audit**, all pass. |
| Accumulated repaired course checks | **26** pass: preserved native results or explicit compile-only contract, plus the corrected rejection; missing `cast` definition remains compile-only. |
| ABI/linkage/cache | 13 address ABI, four conversion ABI and four query ABI identities; five merged-TU linkage and three repeated-address controls; precise completion controls all pass. |
| Architecture trace | Validated typed LowIR, selected ABI symbols, absent dormant body, checked ELF exit zero. |
| Performance | Stage-scoped PA18/O0 acceptance; see frozen observations and discussion in performance record. |

This checkpoint's exit gate is preservation, not PA18 completion. The unchanged
67 failures remain mandatory work, grouped in [plan.md](plan.md): retained
contexts/packs/expression validity; constructor/explicit deduction; LowIR
initialization/result facts. None is waived by executable agreement. The earlier
three handoffs split closely interacting query, access, ABI and execution work
and repeated evidence packaging; those avoidable boundaries should be consolidated.
Do not advance to PA19 before the root through-PA18 report passes.

| Checkpoint ledger | Range / fixes | Evidence / disposition |
|---|---|---|
| 70, accumulated audit | `3a883d10` → entry `15b34993` → `f59e8f67`; all three handoffs plus assignment, arrow/effect and destructor-property repairs | PA18 353/420, same 67 failures; prior 2609/2609; file audit pass; coverage/comparison preserved; performance accepted in PA18/O0 scope. Reviewed code tip above; full-stage advancement remains pending. |
