# PA30 compact implementation plan — handoff195

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `27029f978e65b78331233123922d342033d5d1f7`.
Target: **PA30 full-stage**. Phase: **implementation handoff, incomplete stage**.
Previous goal turn: progress (PA29 audit completed); entry was clean.
The markers above remain the PA29 boundary; PA30 implementation awaits review.

## Completed group and spec alignment

Implementation `32727741` closes the hosted declaration-parser group, extending
from tuple constructor lookahead into nested angle construction and friend tags:

| Owner | Repaired data flow | Work and lifetime |
|---|---|---|
| `syntax/prediction.cpp` | Member-template `operator=` stays a function declarator; class lookahead stops at its class instead of treating the operator as a variable initializer. | Existing indexed category traversal; constant added decision per token, no grammar replay. |
| `syntax/cursor`, `prediction`, `name_parser` | Cached angle endings distinguish the first half of `>>` from the whole token, preserving type arguments, outer qualifiers and nested braced construction. | One bit in existing token padding; O(1) cache access, released when the cursor consumes the token. |
| `syntax/declarator`, `class_parser` | Elaborated friend tags preserve visible class lookup instead of creating an empty shadowing class scope. | Constant specifier state; semantic friend ownership remains unchanged. |

Tokens still feed one retained graph and the shared semantic → typed LowIR →
native ELF path. No source registry additions, new backend route, optimization,
reference correction, harness change or fixture change. Source-position
information was added to the existing lazy parser rejection diagnostic.

## Validation and performance

[Validation](../student.tests/pa30/evidence195/validation.json): earlier PAs
**4941/4941 pass**; file audit passes with four inherited substantial-header
warnings. PA30 improves **105/153 → 114/153**, **48 → 39 failures**, nine fixed
existing cases and zero regressions. The supplied 105/154 summary disagreed with
its own raw log; fresh entry cases and unchanged hashes establish the baseline.
[Delta](../student.tests/pa30/evidence195/stage-delta.json) retains every case.
The required through-PA30 report still fails at PA30; no advancement is claimed.

[Controls](../student.tests/pa30/evidence195/controls.json): **28 commands pass**,
including three header-free reducers, positive/negative scope/angle boundaries,
checked execution, and explicit LowIR/native adapter execution. Personal inputs
and scripts are in `student.tests/pa30/` and were run explicitly.

[Performance195](performance195.md): **404 observations + 16 launchers**,
frozen binaries/flags/inputs, A/A plus six ABBA blocks for four equivalent fixed
workloads, corrected-owner scaling, and all nine newly passing hosted fixtures.
Equivalent A/B objects/executables are identical; no repeatable regression or
speedup is established. Largest repaired hosted compile: **1.151 s / 58,688 KiB**.
Mandated **45 s** timeout unchanged. Token size **40 → 40 bytes**; lookahead
stays 35/12/25 tokens across 128–2048 repeated owner inputs. No optional transform
or generated-code growth. Inherited blanket percentage/no-growth targets remain
diagnostic under spec §9; mandated budgets and correctness are preserved.

## Remaining implementation groups

Counts below group current diagnostics; deeper root causes remain implementation
work. Exact affected fixtures and messages are preserved in the delta ledger.

| Failures | Owner / next data-flow investigation |
|---:|---|
| 11 | `semantic/type_query`, `query_call`: shared-pointer construction/member queries → completed type/category facts. |
| 5 | `semantic/callable`, template demand: dependent callable uses → prerequisite facts and demanded bodies. |
| 5 | `semantic/dependent_type`: hashtable base/member alias lookup → canonical applied type. |
| 6 | `semantic/type_builder`, constant evaluation: hosted array expressions → checked integral bounds. |
| 2 | `semantic/lookup`: convergent aliases/using edges → one declaration/type result. |
| 2 | `semantic/access`: random-library nested member use → retained access context. |
| 1 | `semantic/class_pattern_selection`: chrono patterns → unique partial specialization. |
| 2 | `semantic/construction`: piecewise tuple arguments → viable constructor and conversions. |
| 1 | `semantic/template_call`: bind member callable → viable overload. |
| 1 | `semantic/exception_specification`: replaceable new redeclaration → compatible exception fact. |
| 1 | Builtin registry / vector semantics/lowering: target vector intrinsic → typed operation. |
| 2 | Semantic body checks / lowering reachability: cross-function local reference and reachable missing return → required rejection. |

These owners must retain indexed lookup, complete canonical fact keys,
per-demand computation and bounded candidate/IR work; new fixes require reducers,
current-suite deltas and earlier-stage validation, not retry-all or name recovery.

## Handoff ledger and review boundary

- Entry/baseline commit: `e7fd16e3`; implementation: `32727741`.
- Completed: parser group above, source/coverage binding, correctness controls,
  required checks and stage-scoped performance evidence (89 consistency checks).
- Boundary: every current hosted parser diagnostic in the entry failure set is
  resolved. The 39 remaining failures reach different semantic/lowering owners.
  Further progress requires their declaration/demand/access/conversion facts;
  the parser's category and delimiter changes cannot establish those facts.
  Shared-pointer query demand is the largest next implementation group.
- Independent review remains pending for this implementation and whole PA30:
  review cursor split-cache reuse, class/friend scope fidelity, and the full
  demanded-template-to-ELF architecture/performance requirements. These are
  review tasks, separate from the known implementation failures above; neither
  is waived and the Last reviewed commit marker is deliberately unchanged.
- This handoff ends implementation195 only. Ralph must schedule further work
  and independent audit before advancing PA30. Final evidence/docs commit leaves
  the tested implementation unchanged; clean status is checked after committing.
