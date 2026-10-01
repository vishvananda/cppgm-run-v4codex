# PA29 compact plan — implementation177 handoff

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `221d6d0e4930da05db2913bdf5f50d808f89c744`.
Audit entry: `914e1a0a07e40884c91c0b967b0421eea9ef0d48`.
Last reviewed commit: `7139ceb5769eea12c6e2e00b54932f09d59c79b5`.
Implementation177 entry: `d69d57fc28bfc308f8223e3090fb9e9ab37c4823`.
Implementation code: `eaa24678`, `8288199b`.

## Design/spec alignment and completed group

[Implementation177](implementation177.md) completes selection initializer scope,
condition conversion, constexpr branch selection and runtime demand. Ordinary
and template if/switch forms accept declaration, alias and expression initializers.
The group extends through condition declaration restrictions, class conversion,
constant-evaluation mode, inferred returns, control-flow-limited branches,
discarded-source diagnostics, template/member/storage demand and cleanup on
normal, exceptional and jumping exits.

Parsed source nodes flow into canonical declarations, conversion/selection facts
and lifetime prefixes. Lowering consumes those facts into typed LowIR and the
existing MIR/ELF path. No grammar replay, semantic cloning, textual transport,
global retry or optional optimization was added. New state has TU/function
ownership; work follows actual statements, demanded facts and emitted operations.

## Validation and performance

PA29 **381/403**, **23 → 22 failures**; the existing hosted special-members and
control-flow fixture is fixed, with no new failures or coverage reduction.
PA1–28 **4538/4538**; through PA29 **4919/4941**. All **403** course inputs and
**1,707** tracked contract/harness paths are unchanged. **36** explicit controls
and **59** inspection commands pass, including external LowIR validation,
telemetry equivalence and unwind inspection. File audit passes with the same four
inherited header warnings. [Validation](../student.tests/pa29/evidence177/validation.json),
[coverage](../student.tests/pa29/evidence177/coverage.json), and
[delta](../student.tests/pa29/evidence177/stage-delta.json) retain the exact evidence.

[Performance177](performance177.md) applies spec §9 to **PA29/O0**. Frozen A/A
and six ABBA blocks measure common-input compiler latency/RSS and checked
runtime/text; 600/1200/2400-specialization controls measure the affected syntax.
All samples and work counters are retained. No speedup is claimed for syntax
entry rejects. Optional transform budgets remain zero; historical blanket
15%/zero-growth gates remain diagnostic, with mandated limits preserved.
Common objects/executables are byte-identical. Paired compiler medians are
0.9325–1.0360 and peak RSS grows at most 0.22%; timing noise and the affected
2,400-function runtime increase are disclosed in the report. All affected work
and IR/text counters scale linearly at the three measured sizes.

## Remaining implementation and independent review

The [remaining ledger](../student.tests/pa29/evidence177/remaining.json) retains
extended syntax/types/layout **17**, template demand/hosted ABI **4**, and legacy
trait **1**. Numeric/complex representations, decomposition, coroutine contextual
syntax, deduction guides, zero-length arrays, static receivers, char-traits
conversion shims and required force-inlining remain unfinished implementation.
A successful through-PA29 report remains necessary before PA30.

[Audit174](audit.md) remains the last independent review. [Audit170](audit170.md)
retains char-traits/alignment/dependent-offset/convertible-index reducers and the
nothrow-default-construction and nothrow-invocable contract questions.
[Implementation176](implementation176.md) retains the nested-member ABI-tag
question and reducer. These three contract questions remain counted failures;
no fixture, oracle or comparison rule was corrected or waived. This implementation
requires independent review of correctness, architecture and performance.

## Handoff ledger and boundary

The preceding goal turn was validated implementation176 progress. No live job
required resuming at entry. This turn completed selection initialization, then
extended its shared semantic ownership through lifetime cleanup, constexpr
conditions, inferred returns, jump restrictions and dormant definition demand
when focused controls exposed related defects.

No known required PA29 defect remains in this completed selection-scope group.
Structured bindings need a new decomposition entity/access representation;
coroutine parsing needs its own contextual grammar and eventual coroutine facts.
Numeric layouts, static call receivers and force-inlining likewise have different
owners and representations. Extending selection wrappers or branch-demand state
cannot implement those remaining failures. The existing trait/ABI discrepancies
need independent contract resolution. General C++14 constexpr switch execution
and source-object DWARF support are not claimed by this group or required by its
PA29 fixture. These concrete ownership and contract boundaries end this handoff.
Stage base and Last reviewed markers are preserved; whole-stage completion and
independent review remain open.
