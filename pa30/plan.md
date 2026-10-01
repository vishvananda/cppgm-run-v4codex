# PA30 compact implementation plan — handoff196

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `27029f978e65b78331233123922d342033d5d1f7`.
Target: **PA30 full-stage**. Phase: **implementation handoff, incomplete stage**.
Previous goal turn: progress (validated parser handoff195); entry196 was clean.
Entry196: `c67da7c829f78708b84ef36261a33e81a4896f5f`.
Review markers retain the PA29 boundary; neither implementation handoff is audited.

## Completed group and spec alignment

Handoff195 repaired declaration prediction, split-angle lookahead and elaborated
friend tags (`32727741`); its controls and performance evidence remain preserved.
Implementation196 closes the related **current-instantiation query** group:

| Owner / commit | Data flow and fix | Complexity / lifetime |
|---|---|---|
| `semantic/type_query`, injected type owner / `b7b1436a` | Resolved construction declaration → canonical injected current-instantiation type → dependent query and substitution. Avoid treating the primary declaration as a fixed construction target. | Existing class/source-head cache: O(parameters) first construction, O(1) average reuse; TU Analyzer ownership. |
| `semantic/type_query`, current-instantiation scope / `2f1595b2` | Dependent receiver type → retained current-class scope → indexed qualifier alias → typed member query. Fixes empty symbolic-scope lookup in dot/arrow expressions. | O(lexical depth), cached canonical type comparisons and indexed lookup; query stores type/context once. |

Primary/partial/nested construction, explicit specialization, deleted constructor,
undemanded body, qualified field/function, const receiver, private access and
unrelated qualifier boundaries are covered. No new source registration, container,
backend path, optimization, reference correction or harness/fixture change.
Existing retained graph → semantic facts → typed LowIR → native ELF is preserved.

## Validation and performance

[Validation](../student.tests/pa30/evidence196/validation.json): earlier PAs
**4941/4941 pass**; file audit passes with four inherited substantial-header
warnings. Required PA30 improves **114/153 → 129/153**, **39 → 24 failures**:
**15 fixed existing cases, zero regressions**. All fixture/reference/harness
hashes and expected statuses are preserved in the [delta](../student.tests/pa30/evidence196/stage-delta.json).
Ralph's entry summary reports 114/154; its raw log and a fresh entry run both
show 114/153. The evidence retains every real case. Stage progress is satisfied;
the through-PA30 report still fails at PA30. No advancement is claimed.

[Controls](../student.tests/pa30/evidence196/controls.json): **28 commands pass**,
including negative controls and checked execution through native objects and
explicit LowIR/native adapters. Personal tests were run explicitly.

[Performance196](performance196.md): **332 observations + 16 launchers**,
frozen binaries/flags/inputs, A/A plus six ABBA blocks on four equivalent fixed
workloads, corrected-owner scaling and all 15 newly passing hosted fixtures.
Equivalent A/B objects/executables are byte-identical; no repeatable regression
or speedup is established. Largest repaired hosted compile: **1.379 s / 83,096
KiB**. Corrected 128/512/2048-family query work = **29N+4**, class completions =
**2N**. Mandated **45 s** timeout unchanged. No optional transform or added
growth budget. Inherited blanket percentage/no-growth targets remain diagnostic
under spec §9; historical data, mandated limits and correctness are preserved.

## Remaining implementation groups

| Failures | Owner / required data flow |
|---:|---|
| 2 | `semantic/class_pattern_selection`: chrono/shared-pointer prerequisite patterns → unique partial specialization. |
| 5 | `semantic/callable`, template demand: dependent callable uses → prerequisite facts and demanded bodies. |
| 6 | `semantic/type_builder`, constant evaluation: hosted array expressions → checked integral bounds. |
| 2 | `semantic/lookup`: convergent aliases/using edges → one declaration/type result. |
| 2 | `semantic/access`: random-library nested member use → retained access context. |
| 2 | `semantic/construction`: piecewise tuple arguments → viable constructor and conversions. |
| 1 | `semantic/template_call`: bind member callable → viable overload. |
| 1 | `semantic/exception_specification`: replaceable new redeclaration → compatible exception fact. |
| 1 | Builtin registry / vector semantics/lowering: target vector intrinsic → typed operation. |
| 2 | Body checks / lowering reachability: cross-function local reference and reachable missing return → required rejection. |

These are unfinished implementation, not review questions. Retain canonical
fact keys, indexed lookup, precise demand and bounded candidate/IR work.

## Handoff ledger and review boundary

- Initial scope: 11 construction-query failures. Extended through the shared
  current-instantiation owner to all six qualified-member alias failures,
  including one exposed by the construction repair. Fifteen tests now pass;
  the remaining initially masked shared-pointer failure reaches partial ordering.
- Coherent boundary: no current failure retains either repaired diagnostic.
  The remaining partial-ordering, callable prerequisite, constant-expression,
  access, overload and lowering decisions require facts these query mappings
  cannot establish. Further related query correction lacks a remaining reducer;
  continuing requires a separate owner investigation and implementation group.
- Completed: owner fixes, controls, unchanged coverage/source binding, required
  checks, compiler/runtime/text evidence and **105 evidence consistency checks**.
- Independent review pending: prior split-cache/class-friend scope changes;
  injected type identity for source heads, retained qualifier scope and concrete
  access/substitution; whole-stage demanded-template-to-ELF architecture and
  performance. These review tasks remain separate from the 24 known failures.
  Neither is waived; Last reviewed commit is deliberately unchanged.
- This handoff ends implementation196 only. Ralph schedules more implementation
  and independent audit before advancement. Final evidence/docs commit leaves
  tested implementation unchanged; check clean status after committing.
