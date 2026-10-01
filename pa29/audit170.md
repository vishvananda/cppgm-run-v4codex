# PA29 checkpoint audit170

Target: **PA29 full-stage**; accumulated checkpoint audit, not stage completion.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous review: `f07f78236eb475648834ed78afbca6c864f64408`.
Audit entry: `ecb69d94227d60653a7874f9daea63be8a202840`.
Last reviewed commit: `221d6d0e4930da05db2913bdf5f50d808f89c744`.

The complete range contains twelve entry commits across implementation167–169,
plus the audit fix. [Range evidence](../student.tests/pa29/evidence170/range.json)
records every commit, changed path and full/source diff hash. The combined source
review covers all 72 affected implementation/build-list paths and their shared
query, template, ABI, demand and constant-storage interactions. No review boundary
was narrowed. The preceding handoff is verified progress through its committed
implementation and evidence; entry inspection found no inherited live build/test.
The tested fixes were committed before this records-only update.

## Findings and fixes

1. **Aggregate pointer changes escaped constant-call keys.** Storage deduplication
   stopped after the first reference argument. The initializer payload and only
   that addressed subobject were inspected, missing a pointer subsequently written
   into another field. A later mutation of its pointee reused a stale call result.
   `pointer-dependencies.cpp` requires 4+10=14; entry rejects its static assertion.
   `pointer-arithmetic-dependencies.cpp` demonstrates the same gap through array
   pointer arithmetic, without a second argument. The fix indexes address-bearing
   overlays on their actual storage owner. Writes and whole-subobject retirement
   maintain intrusive links in O(1) per changed/retired record. Key construction
   visits those values once through the existing storage/object cycle guards.
   It does not freeze whole scalar arrays or scan unrelated dirty fields.
   Frame exit retires the index with the existing storage lifetime. The retirement
   and constructor controls cover replacement and nested lifetime interactions.
   [N3652, expr.const changes](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3652.html)
   permit reading and modifying objects born during constant evaluation; results
   must follow their current values. This is the C++14 extension used by PA29,
   not a new C++11 mutation rule. Spec §§2,4,5 require complete cache inputs.

2. **Dependent vector-list queries omitted the vector type categories.** Direct
   source initialization recognized vector lanes, but the query initializer used
   by substitution treated vectors as scalar values. Valid `decltype(sizeof(T{1,2}))`
   substitution failed. Both fixed and extended vector types now use their recorded
   canonical lane count in the shared query list planner. Existing conversions,
   narrowing checks, excess-element rejection and SFINAE determine validity;
   no synthetic source, grammar replay or library-name fallback was added.
   The [GNU vector contract](https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html)
   defines scalar lanes, byte widths and braced vector values. PA29 explicitly
   requires vector layout and semantic validation of wrapper literals. Runtime
   vector operations remain outside its scope. Positive queries and three negative
   controls exercise ordinary, substituted and rejected forms.

The integrated reducer combines a vector-query template, a designated compound
literal containing a block pointer, hidden-receiver invocation, and deferred
inline wrappers. Four entry-failing inputs now pass; all twelve final audit
inputs are the same ones recorded in the final entry comparison. Preliminary
probe evidence is retained, including correction of an audit control's initial
hand-calculated expected sum before the final entry/final runs.

No required fixture, reference, sidecar, discovery rule or comparison changed.
[Oracle review](../student.tests/pa29/evidence170/oracle-review.json) revisits both
inherited questions. An ordinary undefined trait requires rejection under N3485
[temp.inst]/7; the original adds declarations in `std`, invoking [namespace.std]
§17.6.4.2.1/1. The explicitly-false ordinary trait makes its negated cache assertion
false under [dcl.dcl] §7/4; the original declares double-underscore names covered
by [global.names] §17.6.4.3.2/1 and [reserved.names] §17.6.4.3/2. These limitations
prevent treating the ordinary reducers as strict proof for changing the original
oracles. Both required failures remain counted. No special library spelling is
implemented, no waiver is granted, and no reference bundle revision is claimed.
The manifest still pins source `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.

## Every commit reviewed

| Commit | Reviewed content and interaction |
|---|---|
| `7719caea` | Prior accumulated audit, reviewed baseline and preserved evidence/coverage. |
| `7db8a253` | Parsed vector attributes, canonical shape/layout, inline validation and precise deferred-use edges; common builtin registry. |
| `039541f3` | Extended/boolean vectors, dependent lane/count validation, packs and deduction; no runtime vector lowering claim. |
| `db6b91a0` | Dependent vector dimension identity through PA9 typed graph, encoder and explicit fact adapters. |
| `0c6df4c1` | Vector/inline validation and frozen performance handoff; four required improvements. |
| `344b9c70` | Designated/compound initialization across source, query, template and ABI owners; activation-owned mutable aggregate storage. |
| `5201692d` | Aggregate inspections, scaling and validation harness; typed ABI controls. |
| `34a8dd1a` | Cast-query construction extracted to its own registered owner; source distinction retained. |
| `217dc69f` | Aggregate handoff, final versus preliminary evidence, three required improvements and remaining ownership. |
| `3b670e56` | Canonical block types, conversions, fixed/query calls, hidden receiver signature, ABI/RTTI and shared declaration helpers. |
| `9772ca29` | Block ABI inspections, host controls, performance and checkpoint harness. |
| `ecb69d94` | Block handoff, four required improvements, all 42 residuals and preserved independent-review boundary. |
| `221d6d0e` | Complete aggregate dependency ownership and shared vector query formation; integrated controls and reproducible audit tooling. |

## Architecture and generated-code trace

`lowering/driver.cpp` retains immutable preprocessor source buffers, streaming
post/syntax cursors, and integrated Parser/Analyzer construction. Attributes and
compound literals are retained parsed nodes, not textual phase transport.
Vector lane/width queries, block function children, qualifiers and aliases use
canonical TypeIds. Expression-query keys retain typed child identities and scope;
source locations remain on source occurrences. Equality and completed lookup use
interned IDs and flat indexes, not rendered signatures or mangled strings.

The nontrivial declaration is `r` in `controls170/integrated.cpp`. Its `Record`
layout includes a block reference and scalar size. `build<Record>` consumes the
selected list conversions and field identities, returns the ABI aggregate, and
`work<3>` calls the recorded block signature. Lowering evaluates the callable
once, loads its entry at offset 16, and passes the receiver before explicit
arguments (after any indirect result). Typed LowIR signatures carry this ABI to
native selection and encoding. No lowering lookup reconstructs the call.

The demanded-template trace is `shape<V>` inside `work<3>`, with `build<Record>`
exercising dependent compound literal formation. Each source body is parsed once;
immutable substitution frames reuse nondependent nodes and compute dependent
queries once per complete key. The corrected vector initializer records the
valid four-lane shape before sizeof produces 16. Invalid list conversions remain
compact failed candidates. The inline-use graph deduplicates owner/target edges;
demand activates only recorded consumers, without retrying unrelated functions.
Unsupported reserved-builtin wrappers stay un-emitted until a real use demands
an error, while ordinary unused-body semantic errors still reject.

Aggregate storage and the dependency index have explicit activation ownership.
Whole assignment unlinks retired descendants; unrelated caches remain warm.
Canonical immutable payloads are shared, and snapshots visit only requested parts.
TU arenas and geometrically grown typed vectors own syntax, queries, layouts,
call facts and ABI graphs. Local argument/planner/activation scratch has bounded
scope; no new per-node owning allocation, shared-pointer graph, global mutable
cache or duplicated semantic tree was introduced. Production uses typed LowIR
and direct ELF emission. `native::compile_image` selects/encodes one function at
a time and releases its MIR/selection scratch before the next function. PA9 and
LowIR readers/writers are explicit inspection adapters only.

[Retained machine evidence](../student.tests/pa29/evidence170/machine-trace.json)
and [83 audit inspections](../student.tests/pa29/evidence170/inspection170.json)
verify validated/lossless LowIR, independent native execution, symbols, MIR,
disassembly and telemetry equality. The integrated `work<3>` uses a 48-byte O0
frame, preserves rbx/r12, loads the block entry once and makes three calls:
aggregate construction, indirect invocation and scalar shape. `shape<V>` has
no frame and returns immediate 16. These are actual encoding inputs/output,
not an inference from smaller IR. Earlier inspections additionally check scalar,
floating, aggregate/reference/variadic block ABI and unwind paths. The optional
Clang value-catch reducer remains a documented external limitation; student value
catches and cross-compiler reference catches pass. PA29 introduces no debug-line
output gate; required later debug/allocator checks retain their stage ownership.

## Legality, profitability, budgets and performance

The useful proof is either a complete constant activation key or a checked vector
shape. A changed pointee version invalidates reuse at the storage dependency owner;
without a valid constant result the ordinary conservative evaluation path remains.
A valid vector sizeof becomes scalar constant 16 after list legality is known.
Neither change speculates across effects, aliasing, lifetimes or exceptions.
Existing ABI conversions and cleanup facts remain authoritative. No new optional
transform or fixed-point search needs a profitability exemption.

[Performance170](performance170.md) reports compiler latency/RSS, executable
runtime and text size together for the full accumulated range and the audit fix.
There are **1,384 final observations**, six launcher samples, frozen A/B binaries,
A/A calibration and ABBA paired spreads. All common and inherited affected A/B
images are byte-identical; every corresponding non-time work counter is equal.
The two audit-only capability families reject on entry and have no correct A
performance oracle. Historical final and preliminary measurements remain intact;
all 912 final handoff observations and their binary/input hashes were reviewed.
Timing noise and cumulative floating/exception compilation variation are disclosed;
no speedup or repeatable avoidable regression is established.

Additional optional work/growth budgets remain zero. Overlay-link maintenance is
O(1), retirement is amortized over created dirty paths, and dependency traversal
follows actual address-bearing values with cycle guards. Scalar-array mutation
still has 22N+24 evaluation steps and 2N dependency work; the new pointer family
has 36N+5 steps and 10N+1 dependency work at N=600/1200/2400. Query list work follows
explicit lanes with repeated zero tails. The 1,000,000-step/512-depth evaluator,
0x70000000 native frame/data limit, 4096 alignment and course timeouts are preserved.
Compiler size grows by 88 audit bytes. Historical blanket 15%/zero-growth targets
remain self-selected diagnostics under spec §9, with measurements preserved;
they add no exit gate. Required semantics, mandated limits and coverage are intact.

## Validation and remaining work

[Validation](../student.tests/pa29/evidence170/validation.json) identifies the final
compiler hash and [unchanged tested source](../student.tests/pa29/evidence170/validated-source.json).
Committing the fixes changed no tested source or binary:

- `make test-pa29`: **361/403**, exit 2; exactly the entry's **42 failures**.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4899/4941**, exit 2; PA29 alone fails.
- File audit passes, with the same four substantial-header warnings and no waiver.
- Controls162–169: **348/348**; new controls170: **28/28**. The four entry-failing
  audit inputs pass, with their newly reachable link/runtime checks.
- Supplementary ordinary trait controls: **2/2**, recorded in [oracle controls](../student.tests/pa29/evidence170/oracle-controls.json).
- Inspections167/168/169/170: **88/146/60/83**, all passing (**377 total**).

[Coverage](../student.tests/pa29/evidence170/coverage.json) hashes all **1,707**
contract input/reference/harness paths against entry and the prior reviewed tip.
All **403** stage inputs remain. [Failure delta](../student.tests/pa29/evidence170/stage-delta.json)
proves no new course failure; extra personal passes do not compensate for any.

The [remaining ledger](../student.tests/pa29/evidence170/remaining.json) preserves
broad owners: extended syntax/types/layout **27**, template demand/hosted ABI **13**,
legacy trait/contract **1**, source-invocation intrinsic operands **1**. These are
ownership labels, not new root-cause claims. Preserve the source-invocation,
alignment, dependent offsetof ABI and class-convertible-index reducers with their
owners. Aggregate mutation is resolved, including the additional alias-key gap
found here. Numeric values, templated lambdas/folds/bindings, conditional explicit,
zero-length arrays, packs/aliases and hosted emission still need implementation.
Full through-PA29 success remains required before advancing to PA30.

The three broad handoffs had distinct owners, but splitting vector ABI followups
and aggregate query ownership from their main changes added avoidable handoff
fragmentation. Missing query and reachable-storage interactions survived until
this audit. Future owner handoffs should cover direct/fixed/dependent/query use,
ABI emission and complete mutable storage dependencies together. This record
reviews every increment and leaves one unambiguous code baseline.

## Audit ledger

| Checkpoint | Reviewed range | Findings / disposition | Evidence / result |
|---|---|---|---|
| 158 | `2734e5c6..1ab3499d` (three handoffs) | Alias/expression/template storage fixed; deleted-copy reference corrected with clause proof; historical performance targets classified under spec §9. | PA29 317/403, no new failures; PA1–28 4538/4538; file audit/controls pass; four performance dimensions retained. |
| 162 | `1ab3499d..cce8634c` (entry `9662716b`, three handoffs) | Fixed atomic bool RMW, reference snapshots, alignment/native fallback and cv/identity conversions; reviewed complete traits/invocation/atomic ownership range; no reference changes. | PA29 338/403, identical 65 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 221/221 explicit controls; native/LowIR/cache checks and 1,088 performance observations retained. |
| 166 | `cce8634c..f07f7823` (entry `3036bd4e`, three handoffs) | Reviewed all assembly, function-context and evaluation/storage increments; fixed effect invalidation, runtime extents, prvalue materialization and complete query receiver keys/lifetimes; no reference changes. | PA29 350/403, identical 53 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 222 behavioral checks and 222 inspection checks/groups; 832 final performance observations plus 24 launchers, with preliminary evidence retained. |
| 170 | `f07f7823..221d6d0e` (entry `ecb69d94`, three handoffs) | Reviewed all vector/inline, aggregate and block-pointer increments; fixed aggregate pointer dependency ownership and vector query initialization; no reference changes. | PA29 361/403, identical 42 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 376 behavioral and 377 inspection checks; 1,384 final performance observations plus six launchers, with historical evidence preserved. |
