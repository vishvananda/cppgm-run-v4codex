# PA12 implementation plan

Stage base commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Last reviewed commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Target: PA12 full-stage. Phase: implement; incomplete.

## Design/spec alignment and remaining groups

Keep the shared source graph, canonical identities, indexed demand and typed
conversion/lifetime records. Lowering consumes selected facts; no source replay,
fake AST, reference delegation or later native-backend performance gate.

| Owner | Data flow / complexity | Validation and remaining work |
| --- | --- | --- |
| Member/special-member semantics | Canonical member -> field/base/storage actions; once per required edge | Delegation, unions and copy/move selection pass. Bit-field ordering and helper/emission differences remain |
| Class values/conversions | Indexed targets/base edges -> selected transfer and destination | List arguments/defaults, aggregate prvalues, helper parameters and empty-base conversion pass. Explicit conversion transfer shape remains |
| Lifetime ownership | Complete objects/branch states -> retained storage and cleanup continuations; O(objects + edges) | Static references, conditional destinations and heap arrays pass. Local subobject references and remaining exception-region/continuation forms need work |
| Call boundary facts | Attribute -> validated EntityId -> LowIR metadata | Stable-prefix and abort controls pass. Survivor control 536 requires member-pointer representation and an indirect signature |
| Parameter/result ABI | Selected trivial copy or move -> independent argument/result convention -> matching caller/callee storage; once per class | Large trivial parameters, trivial moves with a user copy and shared direct return storage pass. Nontrivial base-copy parameter fixture requires a distinct representation proof |
| Allocation | Canonical scalar/array names -> selected functions, extent/cookie and actions -> bounded loops | All required new/delete comparisons pass. Personal checks cover sized delete, null, wide/zero/multidimensional bounds, alias destruction and distinct runtime function addresses |

Target-typed lists now preserve their untyped source through overload selection,
then materialize selected defaults, references and aggregate prvalues. All six
entry compile rejections compile. Twenty-three remaining LowIR differences
concern transfers, helper/ABI shapes and cleanup.
Two inherited contract questions remain: PA11 bit-field constructor order versus
PA12 copy-semantics order, and direct-object base-copy parameter passing. Preserve
all fixtures/comparison rules unless the authorized reference-proof protocol applies.

## Performance evidence and budgets

[Member](performance.md), [transfer](transfer-performance.md),
[value](value-performance.md), [conversion/reference](conversion-performance.md)
and [allocation/aggregate/alias](allocation-performance.md), [list](list-performance.md)
and [boundary](boundary-performance.md) evidence retain frozen binaries, flags, inputs,
A/A/ABBA observations, compiler latency/RSS and executable runtime/text.
No runtime optimization gain is claimed for the new semantic paths.

Heap construction/destruction emits fixed-size loops, independent of the bound;
existing local-array expansion remains capped at eight total elements. A bound
runs once. Widening precedes multiplication unless a completed constant-return
proof permits the required source-width form. Constant size overflow rejects.
Each allocation retains one selected record; each singleton memory role needs
at most one additional two-instruction runtime adapter. Empty-array-constructor
proofs inspect an empty body/no actions and add no generated code. Aggregate
appertainment retains one whole-object conversion per selected initializer.
No speculative pass, global retry or new positive-runtime exit gate is added.

## Handoff ledger

- Stage entry: **61/257**, 196 failures; earlier **1327/1327**.
- `4e209677`, `55d15ba8`, `8a83045c`: members/delegation/unions -> **92/257**.
- `3da4de09`: four bit-field reference retypes corrected under the authorized
  exception; [proof and bundle revision](reference-corrections.md).
- `73664568`, `4ea8394f`: transfers and measured prefix policy -> **123/257**.
- `cc2198c9`, `e140daef`, `a3d40a62`, `1a7867fd`: values/storage -> **169/257**.
- `02f2f154`, `77145afa`, `f9c8e6c3`, `70556b3d`: conversions/references -> **202/257**.
- `d2db8706`: allocation/deletion and bounded heap lifetimes -> **223/257**.
- `ffbc7095`: whole-class aggregate copies preserve source type/callee and
  reference-member storage; named global arrays keep O0 lifetimes -> **225/257**.
- `eed7d552`: destructor aliases now search the object class, then expression context,
  preserving canonical type checks. Implicit allocator declarations are also
  available when taking function addresses. **226/257**: **24 allocation-entry
  failures removed, no new failures**, unchanged coverage; **165 stage-base
  failures removed**. Thirty-eight personal source checks pass.
- Allocation-group earlier tests **1327/1327**, file audit and `git diff --check` pass.
  All three frozen campaigns and nine final allocation output-equivalence
  checks finished; measurements and outliers are retained in the linked evidence.
  Common final compiler medians increase 0.4%/0.8%; native bytes are identical.
  Compiler text grows 20800 bytes (2.32%); no runtime gain is claimed.
- Logs: `/tmp/pa12-allocation-group-final-stage2.log` (226/257, exit 2),
  `/tmp/pa12-allocation-group-final-prior2.log` (1327/1327, exit 0),
  `/tmp/pa12-allocation-group-final-personal2.log` (38 checks, exit 0).
  Root reports ran serially. The survivor still stops at member-pointer control
  536. Stage progress is verified against the entry failure set, not new tests.
  No fixtures/references/comparison rules changed in these groups.
- `64ecedf7`: clean list-group entry `aee24d98`, **226/257**, 31 failures.
  Untyped list -> target-keyed candidate -> per-use conversion/storage -> typed
  constructor or aggregate helper. Aggregate member parameters own the selected
  move and destruction. Reference storage uses conversion-kind-specific facts.
  **230/257**, four old failures removed and no new failures, all six
  old compile rejections compile; 49 personal checks pass. Earlier serial report
  is **1327/1327**; file audit passes (three header advisories).
  List performance evidence records .29%/.45% common latency cost, .21% largest
  common RSS cost and linear new-path counters; no speed gain is claimed.
  Logs: `/tmp/pa12-lists-final-stage.log`, `/tmp/pa12-lists-final-prior.log`,
  `/tmp/pa12-lists-final-personal.log`. An accidental overlapping pair of root
  reports was discarded and rerun serially; its tallies are not evidence.
- Boundary group: separate argument/result ABI facts, selected trivial value
  transfers, empty derived-to-base conversion and one direct return slot per
  function -> **234/257**. Four further entry failures removed, none added.
  Total from this goal-turn entry: **8 removed**, unchanged coverage; **173
  stage-base failures removed**. Fifty personal checks pass, including large
  values and a nontrivial parameter whose constructor/destructor observes `this`.
  Earlier tests **1327/1327**, file audit (three header advisories), and diff
  checks pass. `/tmp/pa12-boundary-final-stage.log`,
  `/tmp/pa12-boundary-final-prior.log`, `/tmp/pa12-boundary-final-personal.log`.
  Frozen evidence shows a 12% trivial-move runtime gain and a required large
  parameter copy costing 44.5% runtime / 88 native bytes; both are retained.
- Handoff boundary: list selection and declaration-based trivial ABI decisions
  are complete. Remaining cleanup cases require full-expression regions and
  separate destructor effect/boundary facts: changing per-call cleanup alone
  would miss temporaries or regress PA11 array lifetimes. The nontrivial
  base-copy ABI case needs a representation/identity proof beyond triviality;
  survivor 536 needs member-pointer parsing, storage and an indirect signature.
  These require separate coordinated owners, rather than further changes to
  the completed list plans or trivial-transfer predicate. Stage remains incomplete.

Cleanup-group entry: clean `3ec8d0ce`, freshly verified **234/257**. The previous
goal turn is verified progress. Owner: full expression -> immutable active
object prefix -> guarded segments and shared unwind suffixes. Selected calls
and destructor facts determine guards; work follows emitted objects/edges.
Validate enclosing/branch temporaries, condition edges, return preservation and
PA11 lexical/array lifetimes together, then measure frozen common/affected paths.

- Cleanup regions: **240/257**, six entry failures removed and none added;
  **1327/1327** earlier, 51 personal checks, file audit and diff checks pass.
  One expression owns guarded segments; successful construction changes its
  active prefix. Branches close segments without ending enclosing lifetimes.
  Conditions, switch expressions, constructor/delegating initializers and
  returns use that owner; throwing destruction first removes its own prefix.
  Classification is memoized (at most three bytes per AST node); work/region
  telemetry counts actual visits and emitted guards. Prefix cleanup caching is
  unchanged. Frozen A=`3ec8d0ce`, B=`/tmp/pa12-cleanup-final-cppgm`; the
  common/branch/condition/switch campaign is running, with no speed claim yet.
  Logs: `/tmp/pa12-cleanup-final-stage.log`,
  `/tmp/pa12-cleanup-final-prior.log`, `/tmp/pa12-cleanup-final-personal.log`.
