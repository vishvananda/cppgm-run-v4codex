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
| Class values/conversions | Indexed targets/base edges -> object/standard sequence -> selected transfer and destination | Conversion, builtin/surrogate ranking and whole-class aggregate appertainment pass. Braced arguments/defaults, aggregate prvalues and class-valued helper parameters remain |
| Lifetime ownership | Complete objects/branch states -> retained storage and cleanup continuations; O(objects + edges) | Static references, conditional destinations and heap arrays pass. Local subobject references and remaining exception-region/continuation forms need work |
| Call boundary facts | Attribute -> validated EntityId -> LowIR metadata | Stable-prefix and abort controls pass. Survivor control 536 requires member-pointer representation and an indirect signature |
| Allocation | Canonical scalar/array names -> selected functions, extent/cookie and actions -> bounded loops | All required new/delete comparisons pass. Personal checks cover sized delete, null, wide/zero/multidimensional bounds, alias destruction and distinct runtime function addresses |

Target-typed lists now preserve their untyped source through overload selection,
then materialize selected defaults, references and aggregate prvalues. All six
entry compile rejections compile. Twenty-seven remaining LowIR differences
concern transfers, helper/ABI shapes and cleanup.
Two inherited contract questions remain: PA11 bit-field constructor order versus
PA12 copy-semantics order, and direct-object base-copy parameter passing. Preserve
all fixtures/comparison rules unless the authorized reference-proof protocol applies.

## Performance evidence and budgets

[Member](performance.md), [transfer](transfer-performance.md),
[value](value-performance.md), [conversion/reference](conversion-performance.md)
and [allocation/aggregate/alias](allocation-performance.md) and [list](list-performance.md) evidence retain frozen binaries, flags, inputs,
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
- Current entry: clean `70556b3d`, freshly verified **202/257**, 55 failures.
- `d2db8706`: allocation/deletion and bounded heap lifetimes -> **223/257**.
- `ffbc7095`: whole-class aggregate copies preserve source type/callee and
  reference-member storage; named global arrays keep O0 lifetimes -> **225/257**.
- `eed7d552`: destructor aliases now search the object class, then expression context,
  preserving canonical type checks. Implicit allocator declarations are also
  available when taking function addresses. Current **226/257**: **24 entry
  failures removed, no new failures**, unchanged coverage; **165 stage-base
  failures removed**. Thirty-eight personal source checks pass.
- Final earlier tests **1327/1327**, file audit and `git diff --check` pass.
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
- Concrete next boundary: target-keyed list conversions and aggregate helper
  argument ownership require coordinated overload/materialization changes;
  member pointers require a distinct ABI value and call signature. Existing
  whole-object copy or allocation records cannot stand in for those facts.

- List-group entry: clean `aee24d98`, fresh **226/257**, 31 fixture failures.
  Untyped list -> target-keyed candidate -> per-use conversion/storage -> typed
  constructor or aggregate helper. Aggregate member parameters own the selected
  move and destruction. Reference storage uses conversion-kind-specific facts.
  Current **230/257**, four old failures removed and no new failures, all six
  old compile rejections compile; 49 personal checks pass. Earlier serial report
  is **1327/1327**; file audit passes (three header advisories).
  List performance evidence records .29%/.45% common latency cost, .21% largest
  common RSS cost and linear new-path counters; no speed gain is claimed.
  Logs: `/tmp/pa12-lists-stage5.log`, `/tmp/pa12-lists-final-prior.log`,
  `/tmp/pa12-lists-final-personal.log`. An accidental overlapping pair of root
  reports was discarded and rerun serially; its tallies are not evidence.
  Next: parameter/result ABI classification and selected trivial transfer facts,
  then full-expression cleanup regions and survivor member-pointer calls.
