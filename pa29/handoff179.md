# Implementation179 handoff ledger

Entry: `cd283a59949c9d56236f6022d018bd3f2057105c`, 381/403, 22 failures.
The completed group is **local direct-member and array decomposition**. It fixes
all four existing structured-binding failures, including templates, GNU-decorated
class-template methods and range-for bodies. The scope was extended through
array copy/move initialization, constants, cv/reference identity, bit-fields,
base members, discarded branches, captures and lifetime/unwind interactions.

## Ownership, data flow and complexity

| Owner | Data flow and retained facts | Work/storage and lifetime |
|---|---|---|
| Syntax | `BindingNames` contains source names on an auto declarator; ordinary parameter-array parsing is unchanged. | One node per name; source syntax parsed once. |
| Semantic declaration | One hidden initialized object; aliases retain object/member/element identities and reference/cv/bit-field facts. Pending names precede the initializer, so self-use cannot fall back to outer declarations. | One projection per resolved alias. Canonical object-type shape caches retain each field sequence once; base graph visited once per shape. Access is checked at each use. Facts live in the existing translation-unit semantic arena. |
| Template and range | Fixed source facts are checked before demand. Dependent occurrences resolve through existing substitution/publication. Range iteration retains one element-copy conversion recipe. | Work follows actual source facts and demanded occurrences, without body replay, registry sweeps or retries. No extra optimization body retention. |
| Initializer | Shared object initialization owns class/reference construction and destruction. Typed `ArrayCopy` retains one selected leaf conversion, including copy/direct mode and default arguments. Narrowing checks compare referred arithmetic types. | One conversion plan per array initialization, independent of extent. Existing conversion/body-demand caches retain constructor recipes. |
| Constant evaluation | Alias addresses project into the hidden object. Array leaves execute their selected conversions; explicit source constructor identity is preserved separately from destination-copy recipes. | Visits actual evaluated elements under the existing one-million-step/depth budgets; temporary evaluator storage follows the evaluation. |
| LowIR/native | Alias uses project into existing storage; no alias slots. Array copy emits up to eight flattened leaves, then a counted loop and completed-prefix cleanup. | Linear emitted IR; one loop/prefix cursor for larger extents. Reverse cleanup handles constructors and default temporary destructors that throw. Existing typed IR, MIR, ABI and ELF owners consume the result. |

No textual phase transport, host/reference delegation, source-spelling lookup,
optional optimizer or architecture replacement was introduced. Telemetry counts
the shape, member and projection arrays already used by semantic analysis.

## Validation and repaired interactions

- Required stage: **385/403**, exactly four original failures removed; no new
  failures. Earlier stages: **4538/4538**. Full through report: **4923/4941**.
- File audit passes with the same four inherited warnings. All 403 inputs and
  all 1,707 tracked contract/harness paths are unchanged across entry and review.
- 42 personal controls pass; 40 exact host outcomes and two documented
  N3485 [temp.res]/8 IFNDR allowances. Host narrowing warnings are errors.
- 91 inspection commands pass: source AST, LowIR roundtrip, serialized-IR object
  adapter, MIR, symbols/relocations/unwind records, links and execution; stats
  on/off objects agree byte-for-byte.
- The array source/destination constexpr-constructor confusion, parameter-array
  parser ambiguity and wide-reference narrowing defects discovered while
  extending the group were fixed in their shared owners.
- The first parameter-lifetime control assumed caller parameter destruction.
  Its original source/results, standard analysis and stronger portable controls
  are preserved in [lifetime179](../student.tests/pa29/lifetime179.md).

[Validation](../student.tests/pa29/evidence179/validation.json),
[failure delta](../student.tests/pa29/evidence179/stage-delta.json),
[coverage](../student.tests/pa29/evidence179/coverage.json),
[controls](../student.tests/pa29/evidence179/controls.json),
[inspection](../student.tests/pa29/evidence179/inspection.json) and
[performance](performance179.md) bind these claims to frozen code and inputs.
Preliminary failures/measurements remain evidence; they are not final passes.

## Remaining implementation and independent review

The [18-case ledger](../student.tests/pa29/evidence179/remaining.json) retains
13 extended syntax/type/layout cases, four template-demand/hosted-ABI cases,
and one legacy-trait case. Fifteen are unfinished implementation; three are
independent contract questions retained from audits170/176/178. They remain
counted failures, with no fixture revision, waiver or comparison change.
In particular, char-traits conversion remains implementation work.

This handoff boundary is the completed local decomposition representation and
its initialization/lifetime pipeline. Remaining required work needs distinct
scalar/vector/complex/BitInt representations, GNU declaration/call rules,
coroutine context, force-inline control flow, library conversion or ABI policy;
it does not consume the new decomposition plans. General tuple-protocol and
namespace-scope decomposition are not implemented by this increment. No claim
of complete C++17 support or full PA29 completion is made. Pass the full current
stage and through report, then resolve the whole-stage audit before advancement.

Commits: `a0a67bdf` typed local/member/range bindings; `bcef72ad` typed array
initialization, constants and bounded cleanup; `bf487a41` shared reference
narrowing repair and final lifetime controls. Documentation/evidence commit
completes the implementation handoff, not independent whole-stage certification.
