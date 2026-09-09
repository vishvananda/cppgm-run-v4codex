# PA11 implementation

Stage base commit: a97e14d49c7edfc7acc115b974ab667cc90480db
Last reviewed commit: a97e14d49c7edfc7acc115b974ab667cc90480db

Target: PA11 full-stage. Phase: implement. **Incomplete: 194/302; 108 failures.**

## Design and remaining groups

Continue the integrated semantic graph and typed PA10 lowering. EntityId/TypeId
own selection, layout, constructor/destructor actions and demand. Sparse lifetime
uses and immutable lexical tails belong to the TU; materialized temporary states,
projection paths and emitted cleanup interning belong to one lowering function.
Return continuations preserve values and distinguish enclosing loop/switch
context. Eight total array elements is the expansion budget; larger arrays use
counter loops, including partial-construction cleanup. No source reparsing,
fixture logic or host compilation. PA12 value transfer and PA13 dispatch stay separate.

| Owner / remaining group | Required data flow and complexity | Validation |
| --- | --- | --- |
| Scope access, friends and ADL | Preserve access paths, friendship and source-point visibility through canonical lookup/selection; O(relevant declarations + edges + candidates). | private/defaulted/base destructor rejection, positive/negative access and ADL pairs |
| Operator/member bindings and conversions | Give each operator a canonical identity; record selected implicit object and conversion once; O(candidates + recorded conversions). | functors including temporaries, ordinary operators, derived reference/pointer ranking, local-class scope cases |
| Layout and initialization | Typed bit-field/storage/alignment and union/volatile facts feed aggregate cursors and exact zero spans; O(fields + initializer actions + output). | brace elision, static aggregates, empty-base collisions, bit-fields, alignas and zeroinit boundaries |
| Namespace/TLS lifetime and ABI | Separate storage duration, per-thread guards and complete/base entry identities from selected action sequence; O(objects + required helpers). | TLS wrappers, external D1/D2 roots, inheriting constructors and remaining metadata |

The scalar/array lexical cleanup group and all four PA11 controls are complete
for their checked fixtures. Related remaining destructor failures need access
provenance and distinct external ABI entry facts that the current graph lacks.
Adding cleanup calls cannot supply those facts. The next increment must extend
those semantic owners before lowering; full-stage work remains active.

## Performance evidence

[Constructor checkpoint](checkpoint.md), [lifetime checkpoint](lifetime-checkpoint.md)
and the [frozen protocol](../student.tests/pa11/performance-protocol.md) retain all
measurements, including the initial lifetime campaign before the noexcept fix.
Final common and new-behavior campaigns use the frozen 3f590f18 compiler.
No optimization speedup is claimed. Historical numeric timing/RSS/text targets
are diagnostics under the spec's stage-scoped rule; correctness, coverage and
proportional work remain required. Detailed measured costs and spread are in
the linked lifetime checkpoint.

## Handoff ledger

- a97e14d4: initial 43/302; 259 failures. Stage markers preserved above.
- 4113c34d: member ABI/this, cv selection, references, projections: 84/302.
- 17897014 / cd054d80: ordered constructors/defaults/DMI/base initialization,
  namespace startup and frozen evidence: 156/302; 113 original failures fixed.
- 2f886c18: destructor demand, exception specifications, lexical exits,
  namespace fini and shared return/unwind suffixes: 165/302.
- ae8255d4: bounded class arrays, partial unwind, reverse member-array cleanup,
  pseudo-destructors, static member lifetime, temporary member-call storage and
  initializer/expression-statement/return cleanup, inline policy: 172/302.
- 3f590f18: reject noexcept changes after an earlier absent specification:
  **173/302**. This continuation fixes **17 existing failures**, introduces zero
  fixture regressions, and preserves coverage/references/comparison rules.
- `make test-report-through-pa10`: **1025/1025**. File audit passes (declaration
  count advisory on Analyzer header). Five explicit personal programs validate
  typed LowIR and execute with exit 0. `make test-pa11` still exits 2; no later
  assignment is advanced. Evidence and working tree are committed at handoff.
- Continuation at 55a21dac: prior turn is verified progress; clean tree and
  rerun baseline 173/302. Extend access ownership across fields/types/methods,
  base paths, using exposure and constructor/destructor selection, then friend
  namespace identity/hidden visibility and associated-scope lookup. One indexed
  fact per declaration/relation, selected-candidate checks only; validate access
  rejection/permission pairs, friend/ADL fixtures and all earlier PAs.
- Access increment: 194/302, 21 original failures fixed and no regressions.
  Declarations/base edges/using exposure own access; selected functions and
  conversions check privileges. Canonical namespace friend entities feed
  indexed associated scopes; signature hiding runs only for imported families.
  Temporary addresses and zero-offset base paths are reused by typed lowering.
  Six personal programs execute; three access/hidden-name rejections pass.
  Prior checks exposed nested-definition access and explicit-cast view changes;
  both are repaired before continuing. Performance campaign follows the next
  coherent selection increment; no speed claim is made.
