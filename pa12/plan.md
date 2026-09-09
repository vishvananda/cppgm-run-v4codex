# PA12 implementation plan

Stage base commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Last reviewed commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Target: PA12 full-stage. Phase: implement; incomplete (**248/257**).

## Design/spec alignment and remaining groups

Keep the shared source graph, canonical identities, indexed demand and typed
conversion/lifetime records. Lowering consumes selected facts; no source replay,
fake AST, reference delegation or later native-backend performance gate.

| Owner | Data flow / complexity | Remaining validation |
| --- | --- | --- |
| Full-expression consumption | Source/conversion -> final consumer -> branch destination/cleanup; memoized nodes and immutable object prefixes | Terminal conditional member/return branches, direct class-call branch destinations, and return cleanup across loop contexts |
| Initialization and transfer | Selected typed action -> layout/storage operation; once per field/unit | Bit-field constructor instruction order, volatile/union zeroing, explicit conversion transfer, assignment literal widening |
| ABI representation | Class facts -> independent argument/result convention; cached per class | Nontrivial base-copy parameter requires representation/identity proof beyond declaration triviality |
| Member pointers | Formation/application -> object adjustment and function signature -> pair storage and indirect call; constant work per use | Survivor 536 requires nonvirtual member function application and object extent on its indirect signature |

Completed owners include delegation/unions, target-typed list plans and defaults,
class destinations and trivial moves, static/local reference lifetimes, scalar/
array allocation, and full-expression region/destructor-boundary separation.
Nine remaining LowIR fixtures are: `300-bit-field-copy-semantics`,
`300-direct-object-parameter-passthrough-base-copy`,
`300-zero-initialization-object-boundaries`,
`400-conditional-prvalue-member-temporary-lifetime`,
`400-conditional-return-branch-temporary-lifetime`,
`400-direct-init-class-explicit-conversion`, `400-for-iteration-temporary-dtor`,
`500-direct-class-call-temporary-destination`, and
`500-move-constructor-noalias-boundary` (all in `tests/general/`).
Preserve fixtures/comparison rules unless the authorized reference-proof
protocol establishes an error; compiler agreement alone is insufficient.

## Performance evidence and budgets

[Member](performance.md), [transfer](transfer-performance.md),
[value](value-performance.md), [conversion/reference](conversion-performance.md),
[allocation/aggregate/alias](allocation-performance.md), [list](list-performance.md),
[boundary](boundary-performance.md), [cleanup](cleanup-performance.md) and
[destructor](destruction-performance.md) retain frozen hashes, inputs, flags,
A/A+ABBA observations, compiler latency/RSS and executable runtime/text.
Historical misses and outliers remain; required PA12 representation costs do
not create a positive-runtime gate. Avoidable costs were removed and remeasured.

- Local array expansion remains capped at eight total elements; heap lifetimes
  use fixed-size loops. Bounds run once, widening precedes size multiplication
  unless the existing constant-return proof allows source width; overflow rejects.
- Full-expression classifiers cache at most three bytes per AST node. Regions
  follow expression/control edges and successful construction. Each guarded scalar
  call retains at most one value slot; cleanup suffixes are indexed by prefix and
  terminal. Scope-bound reference objects do not add temporary guards.
- Destructor suffixes inline at most eight actions (28 duplicated tail actions).
  Larger classes share one block per suffix action. Constructor nonthrowing facts
  remove redundant handlers without new body analysis or an optimization pass.
- Final common compiler medians rise .24%/.80%, common native bytes are identical,
  and total compiler text growth is 6720 bytes (.71%). Required branch runtime
  costs about 20%; condition overhead falls from about 37% to 7.6% after removing
  a redundant guard. Destructor workloads improve runtime about 20–30%, with
  the eight-field case costing 9.4% compiler time and 176 native bytes. All bounds,
  measurements and stage-scoped acceptance rationale are in the linked evidence.

## Handoff ledger

| Accepted implementation increments | PA12 passing |
| --- | ---: |
| Stage entry | 61/257 |
| `4e209677`, `55d15ba8`, `8a83045c`: members/delegation/unions | 92/257 |
| `73664568`, `4ea8394f`: transfers/prefix policy | 123/257 |
| `cc2198c9`, `e140daef`, `a3d40a62`, `1a7867fd`: values/storage | 169/257 |
| `02f2f154`, `77145afa`, `f9c8e6c3`, `70556b3d`: conversions/references | 202/257 |
| `d2db8706`, `ffbc7095`, `eed7d552`: allocation/aggregates/aliases | 226/257 |
| `64ecedf7`, `3ec8d0ce`: lists and independent value ABI | 234/257 |
| `8bf45f86`: full-expression regions, condition edges, initializer/return ownership | 240/257 |
| `951799ed`: destructor effects, retained boundaries, bounded subobject suffixes | 248/257 |

`3da4de09` corrected four bit-field reference retypes under the authorized
exception; [proof and bundle revision](reference-corrections.md) remain.
`e98cc21e` records initial cleanup measurements, preserved alongside the final run.
No fixtures, references or comparison rules changed in the latest two groups.

Latest entry was clean `3ec8d0ce`, freshly checked **234/257**. Final **248/257**:
**14 existing failures removed, none added**, unchanged coverage; **187 stage-base
failures removed**. Earlier **1327/1327**, all **52** personal source checks,
file audit (three header advisories) and diff checks pass. Root reports ran
serially. All three frozen cleanup/destructor campaigns completed successfully.
Required logs: `/tmp/pa12-destruction-stage2.log` (exit 2),
`/tmp/pa12-destruction-final-prior.log` (exit 0),
`/tmp/pa12-destruction-personal2.log` (exit 0). Survivor 536 still fails at member
pointer application. Required PA12 stage pass is not claimed.

Handoff boundary: the region and destructor-effect owners are complete. The
remaining conditional cases need a final-consumer fact to distinguish a branch
whose selected value is already in its final destination from a branch nested
inside a still-live enclosing call; unconditional branch cleanup would regress
newly passing enclosing-temporary behavior. The remaining ABI case needs an
identity/representation proof, and survivor 536 spans member-pointer formation,
pair storage, object adjustment and an indirect signature. These require separate
coordinated owners, beyond further changes to the completed region/effect flags.
