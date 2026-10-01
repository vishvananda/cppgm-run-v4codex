# Implementation183 — hosted deduction-guide declarations

Entry: `a2ce2670669e0ef392d61b1e1e4aeb6f2e302467` (clean).
Implementation: `054d162c`. Target remains **PA29 full-stage**; this is an
**incomplete implementation handoff**, not an assignment or architecture audit.
Stage base and last-reviewed markers in [plan.md](plan.md) are unchanged.

## Completed owner and extensions

The course's non-template guide declaration previously entered ordinary
function parsing with an absent declarator name and aborted in identifier
lookup. [The parser](../dev/src/syntax/deduction_guide.cpp) now recognizes a
distinct declaration, retains its parameter/result syntax and leaves the class
name in the type index. Complete-class lookahead follows the same rule.
Malformed ordinary function declarations without names now diagnose failure.

The initial case was extended through templated and member guides, nested class
specialization, canonical parameter adjustment, duplicate signatures, defaults,
packs, non-deduced contexts, explicit conditions, exception queries and scope/
access checks. Guides have their own indexed semantic registry, not callable
function entities. Conditions are retained as queries; dependent conditions are
not promises consumed by lowering. Declaring a result specialization does not
complete its class or instantiate its members.

The declaration rules follow the C++17 extension accepted by this PA's fixture:
[N4659 temp.deduct.guide](https://timsong-cpp.github.io/cppwp/n4659/temp.deduct.guide)
requires the corresponding class template, matching result name, same scope/
access and valid parameter clauses; guides do not participate in ordinary name
lookup. [N4659 temp.param/11](https://timsong-cpp.github.io/cppwp/n4659/temp.param#11)
requires guide template parameters without defaults to be deducible. Empty
parameter packs remain possible. Template signature equivalence includes the
normalized template head and dependent result; ordinary signatures discard the
result and adjust parameter cv/array/function types. No fixture or reference was
changed.

Default controls exposed two shared correctness defects. Template parameter
packs now reject defaults at their declaration owner. Source-bound function
defaults reject evaluated prototype-parameter uses while allowing unevaluated
inquiries. A positive `sizeof(parameter)` control then exposed an inherited
substitution defect: parameter names were looked up again before their runtime
objects existed. The shared default owner now retains the existing typed
size/noexcept query and substitutes it independently of body demand. Array
parameter adjustment, references and parameter packs are covered. This follows
C++11 default-argument rules with the adopted
[CWG2082 correction](https://cplusplus.github.io/CWG/issues/2082.html), which
distinguishes potentially evaluated parameter uses from unevaluated operands.
The original N3337 wording predates that correction. No synthesized runtime
parameter object is added.

## Ownership, data flow and complexity

- `syntax/deduction_guide.cpp` owns the single parsed region; delimiter lookahead
  uses the cursor's cached matching positions. `syntax/prediction.cpp` preserves
  template categories during complete-class lookahead.
- `semantic/deduction_guide.cpp` resolves the class identity and forms canonical
  signature/query facts. TU-owned flat indexes use primary identity plus a
  canonical declaration shape; a compact vector retains source, environment,
  template head and conditional facts. No spelling or rendered type is a key.
- `semantic/deduction_parameters.cpp` visits only the signature's reachable
  typed graph, with a visited set. Non-deduced contexts and nested expansions
  have explicit boundaries; there is no candidate substitution or class-layout
  demand merely to test whether a head parameter can be deduced.
- `semantic/default_arguments.cpp` uses the source binding state machine and
  bound declaration identities. A default's additional validation is linear in
  its syntax; the inquiry query is retained once and reused under the existing
  immutable substitution frame. Pack-default rejection belongs to shared
  template parameter creation.
- All new storage belongs to the analyzer/TU and is released there. There is no
  process-global cache, owning node pointer, grammar replay, global retry,
  production text transport or new backend representation.

The inspected `basic` control follows `box<int>` to ordinary aggregate storage,
LowIR and ELF, without a guide symbol. `unevaluated-defaults` demands function
specializations whose default inquiry retains the prototype parameter's type,
substitutes it before body demand, and reaches native code through the ordinary
conversion/lowering path. `member-defaults` composes enclosing class and guide
heads. Direct and explicitly serialized LowIR paths have matching instructions,
symbols and outcomes. AST, MIR, relocations and unwind views are retained in the
[inspection evidence](../student.tests/pa29/evidence183/inspection.json).

## Validation and performance

The [required checks](../student.tests/pa29/evidence183/validation.json) show:
PA1–28 **4538/4538**; PA29 **389/403**; through PA29 **4927/4941**;
file audit passes with the same four inherited warnings. The
[delta](../student.tests/pa29/evidence183/stage-delta.json) proves **15 → 14**
failures, with no new failure and the existing guide fixture repaired.
All **403** stage inputs and **1,707** contract/harness paths are unchanged.
The [53 explicit controls](../student.tests/pa29/evidence183/controls.json) and
**291 inspection commands** pass, including external Clang controls, native
execution, no-demand/symbol checks, typed guide facts, roundtrips and stats-on/
off equality. Inputs live in `student.tests/` and were explicitly executed.

[Performance183](performance183.md) records frozen compilation latency/RSS and
checked executable runtime/text, A/A calibration, ABBA comparisons and final-only
owner scaling. No optional optimization is introduced. Mandated budgets remain
unchanged; the inherited blanket 15%/zero-growth targets remain diagnostic under
spec §9. Historical observations and performance documents are preserved.

## Handoff ledger and boundary

| Disposition | Owner / evidence | Remaining obligation |
|---|---|---|
| Completed implementation group | Hosted guide declaration formation, typed identity, prototype/default validation and substitution; controls and inspection above | Independent review of the committed implementation range remains required. |
| Unfinished implementation | **11** cases in the [remaining ledger](../student.tests/pa29/evidence183/remaining.json) | Extended integer/floating/complex types, vector deduction/lowering, constant-expression compatibility, contextual coroutine syntax, hosted template behavior. |
| Independent contract questions | **3** cases retained from audit182 in the same ledger | Nothrow trait shorthand, invocable-cache expectation, nested-template ABI reference; no acceptance, correction or waiver is inferred. |
| Whole-stage review | [audit182](audit.md) and preserved review markers | Ralph must audit cumulative changes and resolve whole-stage findings before advancement. |

The owner was extended beyond the original crash through both declaration forms,
member specialization, all discovered default/pack defects and positive runtime
substitution controls. None of the 14 remaining failures enters guide formation
or the repaired default inquiry path. Fixing extended scalar/complex operations
requires new canonical numeric representations and native ABI work; the vector
case requires expression/lambda deduction and lowering; coroutine parsing and
suspension require their own deferred-expression owner. Those changes cannot be
completed by extending this declaration registry or its prototype facts. They
remain implementation work, not review questions. General CTAD selection is a
separate consumer of the retained guides; this handoff claims declaration
acceptance, as required by the owning fixture, not newly implemented CTAD calls.
Full PA29 and root through-report success are still required before PA30.
