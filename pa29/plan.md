# PA29 compact plan — checkpoint audit174

Target: **PA29 full-stage**. Phase: **checkpointAudit complete; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `221d6d0e4930da05db2913bdf5f50d808f89c744`.
Audit entry: `914e1a0a07e40884c91c0b967b0421eea9ef0d48`.
Last reviewed commit: `7139ceb5769eea12c6e2e00b54932f09d59c79b5`.

## Reviewed ownership and fixes

The [audit](audit.md) reviews every commit and combined source change across
implementation171–173: builtin template identities/sequences/deduction, retained
fold queries and reduction, and explicit-template closures/capture/ABI. Source
regions remain parsed once; dependent substitution uses canonical arguments and
immutable frames. Typed selections, conversions, layouts, lifetimes and ABI facts
flow directly through LowIR, per-function MIR and ELF. Explicit text tools remain
adapters. The audit fixes three interacting owners:

- Preserve discarded fold forms and selected volatile conversions through query,
  temporary creation, exception facts, cleanup and runtime lowering.
- Exclude declaration-form unevaluated decltype/typeof operands from capture
  recipes, including nested closures; no observable capture copy is invented.
- Preserve the builtin index's converted type, argument-pack grouping and
  parameter-pack substitution identity in ABI facts. The encoder distinguishes
  resolved type arguments from unresolved qualifier argument sequences.

## Validation and performance acceptance

PA29 **377/403**, exactly the entry's **26 failures**; PA1–28 **4538/4538**;
through PA29 **4915/4941**. No new failure is offset by personal passes. All **403**
inputs and **1,707** contract/harness paths remain byte-identical to entry and the
previous review. **218** behavioral controls and **144** inspection checks pass;
the new controls expose nine entry failures and cover integrated host peers,
volatile effects/unwind, AST, LowIR/MIR/native, typed ABI and telemetry equivalence.
File audit passes with four unchanged inherited header warnings.

[Performance174](performance174.md) applies spec §9 to **PA29/O0**, both the full
review range and audit changes. Frozen A/A+ABBA compilation/RSS and checked
runtime/text evidence includes affected owners and scaling, with preliminary and
historical observations retained. No optional optimization or speedup is claimed.
Inherited blanket 15%/zero-growth targets are diagnostic, not additional gates;
mandated generator/evaluator/native limits, correctness and coverage remain.
Heavier runtime, optimizer/allocation and self-hosting remain PA30–34 work.

## Remaining broad work

The [remaining ledger](../student.tests/pa29/evidence174/remaining.json) retains
extended syntax/types/layout **17**, template demand/hosted ABI **7**, legacy trait
**1**, and source-invocation intrinsic operands **1**. These include numeric
representations, structured bindings/control flow, conditional explicit,
deduction guides, zero-length arrays, static receivers, source coordinates and
hosted emission. Through-PA29 success is required before advancing to PA30.

[Audit170](audit170.md) preserves the char-traits analysis, alignment/dependent
offsetof/convertible-index reducers, and both unresolved contract questions:
nothrow default-construction shorthand and nothrow-invocable cache default.
Both remain counted failures; no fixture, reference or comparison rule changed.

The three handoffs have distinct owners. Separating query and ABI followups from
their main owners nevertheless added avoidable fragmentation: discarded-value,
unevaluated-capture and cross-owner ABI gaps survived isolated checks. Future
handoffs should finish direct/dependent/query/discarded uses, lifetime effects
and integrated host linkage together. All three accumulated handoffs are now
reviewed; stage completion remains open.
