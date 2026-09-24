# Loop 71 implementation handoff

Entry `2eb83de5d32f081b028dabfd63f9b57b7ce96d8b`: **353/420**.
Implementation `b6287928`: **367/420**. Fourteen original rejection failures
are repaired; no new failures, fixture changes, reference corrections or
comparison changes. PA18 remains unfinished: **29 status failures and 24 LowIR
mismatches**. This is an implementation handoff, not independent certification.

## Owners, data flow and bounds

| Owner | Completed behavior | Work and lifetime |
|---|---|---|
| `template_call`, `template_type_facts` | Member function specialization composes the retained enclosing frame with its own arguments, including partial explicit arguments. Alias application inside a dependent class retains source scope/declaration identity only when the frame chain contains that class's explicitly recorded lexical frame. Concrete specialization consumes published class/member bindings. Missing concrete bindings still diagnose invariants. | Walks lexical/frame ancestry and indexed bindings, without unrelated declaration searches, copied environments, global retries or text keys. Existing TypeId/QueryId plus complete FrameId caches remain TU-owned. |
| `lookup`, `type_query` | A complete `decltype` base consumes its selected query type rather than looking for an absent terminal identifier. Query calls expand explicit pack arguments through the existing occurrence/frame owner. Callable cv, overload rejection, inherited values and ordinary class layout consume the resulting class identity. | Existing query facts/expansion lanes own work; no parsing replay, fake syntax or additional body demand. Lowering receives the same concrete declarations as ordinary code. |
| `template_signature_shape`, `template_arguments`, `template_checks`, `template_declaration` | Declaration equivalence has a separate structural key. Bound entity/type/parameter identities, operations, qualifiers, argument sequences and explicit template-id presence survive comparison; lexical access context stays on the original semantic query. Renamed out-of-class heads match both source prototypes and concrete member declarations, attaching definitions to the selected function. Canonical NTTP identity uses its normalized type shape. | Each consumed immutable argument/type/query identity gets one cached shape; edges are visited once and interning is average O(1). Temporary vectors release after interning; compact shape keys and the flat cache release with the TU. No query evaluation is triggered for comparison. New work/hit counters expose this cost. |
| Pack substitution inputs | Failed argument substitution remains failure through a pack, including symbolic lane wrapping and alias-head validation. | Linear in consumed pack arguments. These guards do not claim to solve deferred correlated expansions or erased alias obligations. |

Language anchors: checked-in N3485 §14.5.6.1 [temp.over.link]/4–6 for expression
and function-template equivalence; §14.6 [temp.res] and §14.6.4 [temp.dep.res]
for retained lookup; §14.8.2 [temp.deduct] for substitution; §10 [class.derived]
for class-or-decltype bases. Structural comparison never changes the context
used by access checking, candidate selection or constant evaluation.

## Validation

- `make test-pa18`: **367/420**, failures **67 → 53**, no new failures. Every
  fixture and comparison rule is retained. The evidence manifest lists the
  fourteen repaired paths and all remaining failures.
- `make test-report-through-pa17`: **2609/2609**. PA18 file audit passes with
  the same three inherited header advisories. PA18 mandates no additional
  native/debug inspection target; PA9 ABI checks pass in the prior report.
- [Context controls](../student.tests/pa18/context71_controls.py): **29/29**,
  including runtime selection across outer specializations, private aliases,
  class pack sizes, dormant bodies, renamed definitions, dependent NTTP
  overloads, reference identity, and rejection of mismatched definitions and
  invalid/incomplete/final/reference bases. Entry results are preserved too.
- Inherited controls: ordering **64**, substitution **33**, conversion **51**,
  address **63**, deduction **56**, query **58**, audit **56**; all pass.
  Together with the new controls: **410/410**. Three completion-scaling cases
  retain exactly one invalidation when one of 32/128/512 classes completes.
  Four existing dependent-expression ABI checks also pass.
- [Course execution](../student.tests/pa18/context71_course.py): all fourteen
  repaired inputs validate, and thirteen execute successfully. The remaining
  compile-only input, `300-internal-remove-cvref-alias-sfinae`, declares an
  assignment operator without defining it. Both its checked reference and
  student LowIR report an unresolved external during native linking; required
  canonical LowIR comparison passes. No stub or source alteration is used.
- [Performance](performance71.md) preserves A/A calibration, frozen ABBA
  observations, compiler latency/RSS, separate checked runtime/text evidence,
  graph-work scaling and the initial benchmark-classification correction.
  No optional optimization or runtime speedup is claimed.

## Handoff boundary and required unfinished work

The initial frame investigation expanded through symbolic alias application,
concrete member-result access, explicit query argument expansion, `decltype`
bases, declaration equivalence, definition attachment and native execution.
All known defects found in that completed context/identity path are resolved.

Further related failures require retained facts that the current context chain
cannot supply. `400-pack-expansion-size-mismatch-sfinae` tries to expand a source
member default while one pack is still symbolic and its outer pack is concrete;
the current single-lane symbolic representation cannot express that deferred
correlation. It needs an expansion recipe preserving both pack boundaries and
the captured environment, followed by structured mismatch failure at demand.
`300-single-element-detector-idiom-sfinae-false` now reaches its false assertion:
an alias returning a type independent of some arguments has erased their future
validity obligations. `void_t` detector failures share that formation problem.
Fixing these requires changing the retained argument/result model through
substitution, deduction and canonical signatures; another scope fallback or
exception catch would conceal incorrect semantics. These are concrete model
boundaries, not time limits or completed requirements.

Other required implementation remains: lazy nested class definitions, prototype
object facts for `this`, first-declaration result lookup, typed braced query/list
plans and cast/access validity, inherited constructors/explicit member deduction,
and the twenty-four LowIR initialization/result mismatches. The compact plan
continues to target the full stage. Do not advance to PA19.

## Independent review questions

Stage base `94dcb8ad21664137e87d574e878c14a4a047348a` and Last reviewed
`f59e8f67cd8c832361130aef9af1a0337b25c45d` remain unchanged. Review the completed
group's source/concrete frame distinction, completeness and isolation of shape
keys, declaration/body matching, access-context preservation, demand/cache
behavior, and performance evidence. These review obligations are distinct from
the missing implementation above; neither is waived. Ralph owns the independent
audit and whole-stage advancement decision.
