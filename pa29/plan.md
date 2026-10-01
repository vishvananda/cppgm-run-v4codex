# PA29 compact plan — implementation179

Target: **PA29 full-stage**. Phase: **implementation; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `7139ceb5769eea12c6e2e00b54932f09d59c79b5`.
Audit entry: `e59dcaa11e9f674526962ea1fa28f864469fb081`.
Last reviewed commit: `667edd800e4e5eb1b1ef92a3108da3bd96c51708`.

## Reviewed and repaired

[Audit178](audit.md) reviews every accumulated commit across implementation175–177,
all shared ownership interactions and the combined changes to 72 implementation/
build paths. Code fix `667edd80` was validated and committed before these records.
[Audit174](audit174.md) archives the previous review verbatim.

- Declaration lifecycle loops now stop at their fixed symbol extent; source strings created during lowering use their existing separate typed byte queue.
- Deferred inline initializers own their definition context and record deduplicated typed dependencies. Checking, required bound completion, body/default demand and storage emission remain separate states.
- Template defaults receive complete canonical/enclosing substitution frames before projection; renamed heads use immutable overlays, shared with the body machinery.

The pipeline remains streaming source → integrated semantic graph → typed LowIR
→ per-function MIR → direct ELF. No textual phase transport, grammar replay,
global retry, name recovery or optional optimizer was added. The integrated trace
covers exclusion/extern template, array-bound demand, source defaults, constexpr
selection and Guard cleanup. Allocation/cache lifetimes and budgets are recorded
in the audit and [performance report](performance178.md).

## Validation and evidence

PA29 **381/403**, exactly the same **22** entry failures. PA1–28 **4538/4538**;
through PA29 **4919/4941**. Required prior-through, file audit and progress-preserved
criteria pass; full stage tests still exit 2. All 403 inputs and 1,707 tracked
contract/harness paths are unchanged against both review and entry boundaries.
[Validation](../student.tests/pa29/evidence178/validation.json),
[delta](../student.tests/pa29/evidence178/stage-delta.json),
[coverage](../student.tests/pa29/evidence178/coverage.json) and
[source binding](../student.tests/pa29/evidence178/manifest.json) retain exact evidence.

Closure/fold controls pass 59/35 checks; inherited owner controls pass 17/25/36
cases; 15 new student cases and 15 host comparisons pass. Inspections pass
37/48/59/91 commands and all assertions; targeted bounds checks pass 16 commands.
No reference or comparison change was made. Preliminary failures are preserved.

Performance acceptance is **PA29/O0** under spec §9: 776 final observations plus
eight launchers, all 816 inherited observations reviewed. Equivalent paired images
have identical code/data/unwind sections; timing spread and RSS are disclosed.
Corrected bound semantics use final-only scaling, not speedup over incorrect entry.
Optional transform budgets remain zero. Unsupported inherited blanket 15% and
zero-growth targets remain diagnostic, with mandated limits preserved.

## Broad remaining owners

Continue extended syntax/types/layout **17**, template demand/hosted ABI **4**, and
legacy trait **1**, as itemized in the
[remaining ledger](../student.tests/pa29/evidence178/remaining.json). Three contract
questions remain counted failures; char-trait conversion remains implementation
work. No waiver or fixture correction is claimed. Pass the full through-PA29
report before advancing.

Avoid splitting followups at shared definition/default/discard/lifetime boundaries.
The three owner handoffs were distinct, but the missed interactions caused
avoidable fragmentation. Complete those interactions and integrated host linkage
within each broad owner group before the next handoff.

## Active implementation179

Entry HEAD: `cd283a59949c9d56236f6022d018bd3f2057105c`; baseline 381/403 (22 failures).
Previous turn classification: progress — audit178 committed correctness repairs
and validation evidence; it did not reduce the current failure count.

Owner group: structured declarations, including class/array decomposition,
reference/cv identity, template patterns, range-for and object lifetime. Parse
one source node; establish hidden object and typed projections once in semantics;
lowering consumes object/member/element identities without synthetic syntax.
Work/storage must track actual bindings and required class members, with no
whole-program retries. Preserve dependent-only substitution and existing arenas.
Validate all four course cases plus explicit instantiated/runtime/rejection,
LowIR/object and lifetime controls. Record O0 compiler latency/RSS and checked
runtime/text size with frozen A/A+ABBA comparisons for equivalent inputs;
newly correct cases use final-only measurements. Optional transform budget zero.
Remaining broad owners and independent contract questions above remain open.

Increment 1: parsed binding names, class/reference projections, fixed/dependent
patterns and range-for now pass all four course cases and 14 explicit controls,
including cv/reference decltype, mutable bit-fields, self-use rejection, jumps,
constant evaluation and return/unwind cleanup. File audit passes (four inherited
warnings). Continue array-copy initialization and broader owner interactions;
this commit is not a handoff boundary.

Increment 2: shared typed array-copy plans now cover scalar/class/multidimensional
arrays, moves, direct versus copy initialization, constants and partial cleanup.
Native copies unroll at most eight leaves and otherwise use a counted loop.
Base/member shapes cache by canonical object type; access remains use-scoped.
The source-constructor constexpr recursion and unnamed-array parser ambiguity
found during integration are repaired. Full stage reached 385/403, removing the
four original binding failures with no new failures. Final validation/performance
runs are pending; other owner groups and independent review questions remain open.
