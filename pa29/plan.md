# PA29 compact plan — implementation169 in progress

Target: **PA29 full-stage**. Phase: **implementation; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `f07f78236eb475648834ed78afbca6c864f64408`.
Previous review: `cce8634c3c835cf6d5e8f4fa5fea0db213959718`.
Entry HEAD: `0c6df4c1e290c8188e95f07732dad0e7ec271b17`.
Implementation169 entry HEAD: `217dc69ff3f741114d2614688b1e4e2eec335980`.
Entry evidence: 357/403, 46 failures; clean checkout and no live build/test.
Previous goal turn: committed progress (implementation168), revalidated at entry.
Current group: Clang block-pointer declarations and indirect invocation (four
required failures). Owner/data flow: retained pointer declarator → canonical
distinct block-pointer type → shared conversions/call facts and template queries
→ typed invocation signature with hidden block argument → LowIR/MIR/ELF. ABI
naming consumes the same type identity. Type and signature interning are O(1)
average; parsing/substitution and call lowering track actual nodes/arguments.
Validation will cover required fixtures, negative types/conversions, templates,
host ABI invocation, serialized LowIR, and A/A+ABBA latency/RSS/runtime/text.
Optional optimization work/growth budgets remain zero. Review markers above
are unchanged; neither this work nor the previous handoff is independently audited.
Implementation code tip: `34a8dd1a` (implementation `344b9c70`, inspection and
benchmark harness `5201692d`, cast-query ownership `34a8dd1a`).
The preceding handoff was verified committed progress. Entry process inspection
found no inherited live build/test. Independent review markers remain intact.

## Completed owner and design alignment

Ordered class aggregate initialization now retains member designators and GNU
compound literals through the existing production pipeline: source nodes →
canonical type/query facts → selected field/conversion/lifetime plans → constant
evaluation or typed LowIR → MIR and direct ELF. Omitted fields, non-first union
members, anonymous union storage, nested arrays, strings, references and bitfields
share this owner. Overloads and SFINAE reject invalid candidates without bypassing
another aggregate match. Definition-time checks, dependent substitution, noexcept
and the PA9 graph/fact adapter retain designator identity. No source replay,
synthetic AST, rendered semantic key, global retry or textual phase transport.

Related constexpr subobject mutation is complete for live local/parameter storage,
including aliases, nested calls, scalar compound assignments, snapshots and trivial
aggregate assignment. A sparse mutable overlay belongs to the activation owning
the storage. Writes invalidate only their ancestor paths and increment the storage
version used by call keys. Reads of scalar references do not freeze enclosing
arrays. Requested aggregate values freeze changed paths into shared immutable TU
values. Frame overlays release on call exit; dead storage cannot be written.
The original `pending165/aggregate-mutation.cpp` now passes explicitly.

Initialization scans clauses and declared fields once; anonymous projections use
the indexed scope and typed storage edge. New query keys include the source cast
kind, target, context, designator name and child queries. Mutation is O(depth)
plus amortized retirement of previous dirty paths. Scalar reads are O(depth);
aggregate snapshots are O(parts + dirty children log dirty children), only when
the aggregate value is consumed. No optimizer pass or optional code growth added.

## Validation and performance

- `make test-pa29`: **357/403**, exit 2; **49 → 46 failures**, exactly the three
  designated-initializer fixtures fixed, no new failures or coverage changes.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4895/4941**, exit 2; only PA29 fails.
- File audit passes with the same four substantial-header warnings. Its initial
  241-line query-function finding was resolved by extracting cast-query formation.
- Explicit controls168: **52/52**; inherited controls167: **45/45**, controls166:
  **21/21**. Original aggregate-mutation reducer: compile/link/run pass.
- **146 inspections** cover validated LowIR and lossless roundtrip, standalone
  execution, MIR, objects, telemetry equality, required fixture execution and
  dependent designator ABI graph/fact roundtrips.
- [Validation](../student.tests/pa29/evidence168/validation.json),
  [coverage](../student.tests/pa29/evidence168/coverage.json),
  [failure delta](../student.tests/pa29/evidence168/stage-delta.json),
  [inspection](../student.tests/pa29/evidence168/inspection.json), and
  [performance168](performance168.md) retain the evidence.

Performance acceptance is PA29/O0. Frozen A/A+ABBA comparisons report compiler
latency/RSS and checked runtime/text together. Aggregate and mutation workloads
scale at 600/1,200/2,400. All preliminary and final observations are retained;
no speedup is claimed. Optional work/growth budgets remain **zero**. Historical
blanket 15%/zero-growth self-selected targets remain diagnostics under spec §9.
Mandated evaluator/native limits, timeouts, correctness and coverage are unchanged.
Broader hosted runtime, optimization and self-hosting retain PA30–34 ownership.

## Remaining implementation and independent review

The [remaining ledger](../student.tests/pa29/evidence168/remaining.json) accounts
for all 46 required failures. Its inherited labels identify ownership, not proven
root causes: extended syntax/types/layout (31), template demand/hosted ABI (13),
legacy traits (1), source-invocation intrinsic operands (1). Unfinished work includes
vendor numeric/value forms, lambdas, folds, bindings, block pointers, zero-length
arrays, template packs/aliases, hosted emission/ABI and source coordinates.

Boundary: the required member-designator/compound-literal group and its related
query, ABI, lifetime and local mutation defects are finished. No remaining required
failure belongs to this initializer group. For example, the remaining hosted
control-flow fixture requires alias statements/conditional explicit syntax;
zero-length array members require a distinct canonical bound/layout contract.
Further progress requires those separate parser/type/demand owners. This is not
a claim of general C99 array/range designators or full C++17/20 support.

Independent review still owes the accumulated implementation167–168 range and
the forward-declared trait and explicitly-false nothrow-invocable oracle questions.
No reference correction, exemption or review waiver was made. Preserve the
source-invocation reducer, alignment placement, dependent offsetof ABI signatures
and class-convertible index reducers with their remaining owners. [Audit166](audit.md)
remains the last independent review. Handoff completion does not complete PA29.

## Handoff ledger

| Turn | Owner / boundary | Required progress | Review status |
|---|---|---|---|
| 166 | Accumulated assembly/function/evaluation audit through `f07f7823` | 350/403; prior stages and file audit pass | Independently reviewed; historical ledger in audit.md. |
| 167 | Vector layout, inline validation/demand and dependent vector ABI; `7db8a253`, `039541f3`, `db6b91a0` | 354/403; no new failures/coverage changes; prior stages/audit pass | Independent review pending; markers preserved. |
| 168 | Aggregate designators/compound literals, query/ABI and activation-owned mutable subobjects; `344b9c70`, `5201692d`, `34a8dd1a` | 357/403; no new failures/coverage changes; prior stages/audit pass | Implementation handoff ready; independent review pending. |
