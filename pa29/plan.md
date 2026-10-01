# PA29 compact plan — implementation169 handoff

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `f07f78236eb475648834ed78afbca6c864f64408`.
Previous review: `cce8634c3c835cf6d5e8f4fa5fea0db213959718`.
Entry HEAD: `217dc69ff3f741114d2614688b1e4e2eec335980`.
Implementation: `3b670e56`; inspection/performance harness: `9772ca29`.
The previous turn was verified committed progress. Entry was clean, 357/403,
with no inherited live build/test. Independent review markers remain unchanged.

## Completed owner and design alignment

The four required Clang block-pointer fixtures now share the ordinary pipeline:
retained `^` declarators → canonical distinct block-pointer/function types →
checked conversions and call facts → typed invocation signature → LowIR/MIR/ELF.
Source, fixed-template and query calls retain the same canonical signature on a
sparse ObjectUse fact. Lowering evaluates the callable once, loads its invocation
entry and supplies the hidden block receiver after any result parameter. The
existing ABI handles scalar/floating/aggregate/reference results, varargs and
exceptions. Ordinary function pointers keep distinct type and signature identity.

Related behavior is complete for pointer-sized storage, cv/reference/array/field
uses, null/value initialization and constant evaluation, aliases, dependent
function types, partial deduction, SFINAE, overloads, conditional/equality/bool
operations, permitted explicit casts, PA9 vendor naming and exception RTTI.
Invalid function children, mismatched signatures, implicit function/void-pointer
conversions, dereference/arithmetic/ordering, arity and argument errors reject.
The constant evaluator's shared scalar-cast path also handles null initialization
of ordinary pointers. Declaration scope/name helpers resolve the audit's size
finding. No source replay, fake syntax, rendered semantic key, global retry or
textual phase transport was introduced.

Type and signature interning use canonical IDs and average O(1) lookup. Parsing
and substitution follow actual nodes; invocation lowering costs O(arguments),
adding one byte projection and one load. Facts use existing TU arenas; call
scratch and MIR keep their existing local lifetimes. Optional optimizer work and
code-growth budgets remain zero. [Design/performance](performance169.md) traces a
declaration and demanded template through the shared pipeline and ownership.

## Validation and performance

- `make test-pa29`: **361/403**, exit 2; **46 → 42 failures**, exactly the four
  block-pointer fixtures fixed, with no new failures or reduced coverage.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4899/4941**, exit 2; only PA29 fails.
- File audit passes with the four inherited substantial-header warnings.
- Explicit controls169: **29/29**; controls168: **52/52**; controls167: **45/45**.
- **60 inspections** cover validated/lossless LowIR, independent native execution,
  MIR, symbols, disassembly, telemetry equality, fixture execution and PA9 facts.
- [Validation](../student.tests/pa29/evidence169/validation.json),
  [unchanged coverage](../student.tests/pa29/evidence169/coverage.json),
  [failure delta](../student.tests/pa29/evidence169/stage-delta.json),
  [inspection](../student.tests/pa29/evidence169/inspection.json), and
  [performance169](performance169.md) retain the evidence.

Performance acceptance is PA29/O0. Frozen A/A+ABBA comparisons contain 224 common
observations; every A/B object/executable and all work counters are identical.
The 48 affected observations cover 600/1,200/2,400 demands and checked runtime;
body transitions are exactly N, checked bodies N+3 and object-use facts N+2.
Compiler latency/RSS and executable runtime/text are reported together. Timing
is noisy; no speedup is claimed. Necessary compiler growth is 4,336 bytes.
Historical blanket 15%/zero-growth self-selected targets remain diagnostics under
spec §9. Mandated limits, timeouts, correctness and coverage remain unchanged.
Broader hosted runtime, optimization and self-hosting retain PA30–34 ownership.

## Remaining implementation and independent review

The [remaining ledger](../student.tests/pa29/evidence169/remaining.json) accounts
for all 42 required failures: extended syntax/types/layout (27), template demand/
hosted ABI (13), legacy traits (1), source-invocation intrinsic operands (1).
These inherited labels describe ownership, not proven root causes. Unfinished
work includes vendor numeric/value forms, lambdas, folds, bindings, zero-length
arrays, conditional explicit/control syntax, packs/aliases, hosted emission/ABI
and source coordinates. No failure or contract question is waived.

Boundary: required block declarations/calls and their related semantic, query,
constant, ABI and RTTI defects are finished. No remaining required failure belongs
to this group. The remaining extended-vector fixture first requires templated
lambda parsing; numeric forms need distinct value representations; pending hosted
emission/source-coordinate issues have separate demand and invocation owners.
Extending block support to literals, capture generation and allocation would need
new closure-lifetime/runtime ownership beyond the required block-pointer surface.
Further related required work cannot be advanced through this completed owner.

Independent review still owes implementation167–169 and the forward-declared
trait/explicitly-false nothrow-invocable oracle questions. The optional Clang-only
block value-catch reducer is retained as an external interoperability observation;
student value catches and cross-compiler reference catches pass. No reference
correction or review waiver was made. Preserve source-invocation, alignment,
dependent offsetof ABI and class-convertible-index reducers with their owners.
The stale aggregate-mutation entry was moved to resolved reducers, matching
implementation168's explicit success. [Audit166](audit.md) remains the last review.

## Handoff ledger

| Turn | Owner / boundary | Required progress | Review status |
|---|---|---|---|
| 166 | Accumulated assembly/function/evaluation audit through `f07f7823` | 350/403; prior stages/audit pass | Independently reviewed; historical ledger in audit.md. |
| 167 | Vector layout, inline validation/demand and dependent vector ABI; `7db8a253`, `039541f3`, `db6b91a0` | 354/403; no coverage/regression change; prior stages/audit pass | Independent review pending. |
| 168 | Aggregate designators/compound literals, query/ABI, activation-owned subobjects; `344b9c70`, `5201692d`, `34a8dd1a` | 357/403; no coverage/regression change; prior stages/audit pass | Independent review pending. |
| 169 | Block-pointer type/storage/query/call/ABI/RTTI group; `3b670e56`, `9772ca29` | 361/403; 46→42 failures; prior stages/audit pass; unchanged coverage | Implementation handoff ready; independent review pending. |
