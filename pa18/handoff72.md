# Loop 72 implementation handoff

Entry `ca314a723b7dea27045f57cbdb88c23dbdd65ad3`: **367/420**.
Implementation `57ee1f2e`: **372/420**, five original failures repaired and no
new failures. All 420 course inputs, references and comparison rules remain
unchanged. **48 failures remain: 23 status failures and 25 LowIR mismatches.**
This handoff completes implementation of the group below; independent review
and whole-stage completion remain required.

## Ownership, data flow and bounds

| Owner | Behavior and data flow | Bounds and lifetime |
|---|---|---|
| `template_entities`, `template_call`, `model` | A dependent alias application retains its argument tuple and substituted result. Substitution forms every argument, including erased arguments and defaults, before publishing the concrete type. Deduction, parameter adjustment, conversion deduction and signature comparison use the transparent result. Source normalization preserves free enclosing parameters instead of reopening their lexical bindings. | Immutable TypeId/ArgumentId identities and complete frame/type caches are TU-owned. Alias target projection is cached. Scratch argument vectors release on return; no per-node owning pointers or copied syntax/environment graphs. |
| `template_packs`, new `template_pack_recipe`, `type_query` | Mixed concrete/symbolic packs retain a pattern plus only the consumed pack bindings. The next frame substitutes those captures and checks lengths. Type and expression expansions share this owner. Unequal concrete lengths return candidate failure. A single pack can expose a fixed prefix while retaining its symbolic suffix for partial ordering. | Work follows the consumed pack sequences, dependency/frame edges and produced lanes. Parameter discovery and type/query substitution reuse existing canonical caches. No unrelated declarations, member bodies, global retries or token replay. |
| `template_type_access`, `template_class`, `template_checks` | Alias and default formation preserve source access checks. Candidate access rejection returns a result; class-definition errors retain their hard-error boundary. Structural signature keys retain pack capture meaning with local binder normalization, while alias names remain transparent. | Access recipes stay at their existing source/frame owner. Successful and failed completed facts are cached; incomplete prerequisites keep the existing precise completion mechanism. Three scaling controls still invalidate one consumer among 32/128/512 pending classes. |
| `template_arguments`, `template_binding`, `overload` | Typed template arguments own type/value/template disambiguation, including `template` names, type packs and dependent function types represented by parsed parentheses. Alias applications publish the selected type on their name occurrence; ordinary casts and unevaluated queries consume that fact. | Each argument is interpreted through its source/context cache. The retained parsed region is not reparsed or replaced with fake nodes. Concrete declarations and conversions use ordinary typed LowIR lowering. |

Language anchors: N3485 §14.5.7 [temp.alias]/2 (alias transparency), §14.3
[temp.arg] (argument kinds/ambiguity), §14.5.3 [temp.variadic] (expansion patterns
and equal pack lengths), and §14.8.2 [temp.deduct] (immediate context and access).
[CWG 1558](https://cplusplus.github.io/CWG/issues/1558.html) clarifies that
substitution still applies to dependent alias arguments even when the alias
result omits them; the PA18 detector fixtures require that behavior. There are
**no reference corrections** in this handoff.

## Validation

- `make test-pa18`: **372/420**, original failures **53 → 48**, no new failure
  paths and no coverage/comparison changes. The five repaired course inputs
  also validate and execute successfully through the supplied native backend.
- `make test-report-through-pa17`: **2609/2609**. PA18 file audit passes with
  the same three inherited header advisories. PA18 requires no additional
  native/debug target; the prior report includes the PA9 ABI contract.
- [Alias controls](../student.tests/pa18/alias72_controls.py): **43/43**;
  [pack controls](../student.tests/pa18/pack72_controls.py): **34/34**.
  These cover erased/default/pack arguments, partial detectors, access,
  hard class side effects, dormant bodies, alias adjustment and conversions,
  cv/reference identity, redeclarations, functional casts, template argument
  kinds, mixed/empty/value packs, query expansion, renamed definitions,
  explicit prefixes and nested expansions.
- All **410 inherited semantic controls** pass, for **487/487** total. Three
  completion-scaling controls and six ABI observations pass. ABI observations and frozen compiler/
  executable measurements are recorded in [performance72.md](performance72.md)
  and the [evidence manifest](../student.tests/pa18/loop72-evidence.json).
- Broader checks exposed two PA17 ordering regressions and an array-reference
  conversion regression during development; all were repaired in their owners
  before the final checks. Intermediate logs remain in `/tmp/pa18-loop72`.
  One personal reference-cast test initially parsed as a declaration; adding
  expression parentheses corrected that test. No course input was altered.

## Boundary and required unfinished implementation

The work expanded from erased alias arguments through correlated type/value/query
packs, access, parameter adjustment and conversion deduction, declaration
identity, applied scalar/reference types, explicit pack arguments and dependent
function-type arguments. Known defects found in those paths are resolved.

Further nearby failures now reach distinct owners. The sidecar invocation
fixture gets past its function-type argument but reaches `__builtin_invoke`,
which has no semantic intrinsic owner; implementing it requires recording its
selected callable/object/conversions for both queries and ordinary lowering.
This cannot be supplied by alias substitution. The nested explicit-alias call
now compiles and executes, but its canonical LowIR still differs because the
existing constant-array initializer emits global blobs/copies where the course
expects element initialization. That is an initialization/lowering policy
change shared with other remaining LowIR failures. Runtime agreement does not
waive its required comparison. A separate
[class-ellipsis reducer](../student.tests/pa18/class_ellipsis_pending.cpp)
exposed invalid ordinary variadic LowIR while extending the benchmark. Its
selected callable and pack substitution are correct; representation of a class
argument in the ordinary call/lowering contract remains unfinished. The initial
benchmark observations and source are preserved, and the final pack measurement
uses scalar call arguments. No course coverage or acceptance rule changed.

Other required groups remain: lazy nested class declarations, first-declaration
result lookup and prototype `this` facts; inherited/constructor and member-template
participation; braced query initialization and cast validity; array/constant
initialization, result metadata and class-result/discarded-value lowering.
These require new declaration, invocation or initialization facts beyond the
completed formation owner. They are implementation work, not review questions
or a claim that PA18 is complete. Do not advance to PA19.

## Independent review

Stage base `94dcb8ad21664137e87d574e878c14a4a047348a` and Last reviewed
`f59e8f67cd8c832361130aef9af1a0337b25c45d` are preserved. Handoffs 71 and 72
remain unreviewed. Review alias/capture key completeness, source/concrete frame
composition, transparent signature/ABI projections, access and hard-error
boundaries, argument disambiguation, and measured bounds. These obligations
are separate from the unfinished implementation above; neither is waived.
Ralph owns the independent audit and whole-stage advancement decision.
