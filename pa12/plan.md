# PA12 implementation plan

Stage base commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Last reviewed commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Target: PA12 full-stage. Phase: implement; incomplete (**250/257**).

## Design/spec alignment and remaining groups

Keep canonical identities, indexed demand, the shared source graph and typed
conversion/lifetime records. Lowering consumes selected facts without source
replay, fake AST, reference delegation or a later native-backend exit gate.

| Owner | Data flow / complexity | Remaining validation |
| --- | --- | --- |
| Full-expression consumption | Source/conversion -> final consumer -> branch destination/cleanup; memoized nodes and immutable object prefixes | Terminal conditional member/return branches, direct class-call branch destinations, return cleanup across loop contexts |
| Initialization and transfer | Selected typed action -> layout/storage operation; once per field/unit | Bit-field constructor instruction order and explicit conversion transfer |
| ABI representation | Class facts -> independent argument/result convention; cached per class | Nontrivial base-copy parameter and empty return padding need representation/identity proofs beyond declaration triviality |

Seven remaining LowIR fixtures, all `tests/general/`:
`300-bit-field-copy-semantics`, `300-direct-object-parameter-passthrough-base-copy`,
`400-conditional-prvalue-member-temporary-lifetime`,
`400-conditional-return-branch-temporary-lifetime`,
`400-direct-init-class-explicit-conversion`, `400-for-iteration-temporary-dtor`,
and `500-direct-class-call-temporary-destination`.

Completed owners include target-typed lists/defaults, delegation/unions, class
value boundaries, references, allocation/aggregates, region/destructor-boundary
separation, and nonvirtual member-pointer formation/application with canonical
indirect signatures. All **13/13** survivor controls pass.
Zero-initialization now follows cached complete canonical type/layout edges:
typed members, skipped references, padding, ABI null values and flattened array
loops. Late-defaulted constructors retain their user-provided initialization
rule. Semantics demands plans; lowering never reconstructs the decision.

## Performance evidence and budgets

[Member](performance.md), [transfer](transfer-performance.md),
[value](value-performance.md), [conversion/reference](conversion-performance.md),
[allocation/aggregate/alias](allocation-performance.md), [list](list-performance.md),
[boundary](boundary-performance.md), [cleanup](cleanup-performance.md),
[destructor](destruction-performance.md) and
[member-pointer](member-pointer-performance.md) and
[zero-initialization](zero-performance.md) evidence retain frozen hashes,
inputs, flags, A/A+ABBA observations, compiler latency/RSS and runtime/text.
Historical misses/outliers remain. Required PA12 representation costs do not
create a positive-runtime gate; avoidable costs were removed and remeasured.

- Local array expansion stays capped at eight total elements. Zero plans cache
  each type once; flattening reuses child plans. Padding expands at most eight
  stores, then uses one bulk operation. Nonzero null arrays use fixed-size loops.
- Full-expression classifiers cache at most three bytes per AST node. Guarded
  scalar calls retain at most one value slot; cleanup suffixes use prefix and
  terminal identities. Scope-bound references do not add temporary guards.
- Destructor suffix duplication is capped at eight actions (28 duplicated tail
  actions); larger classes share one block per suffix. Final destructor runtime
  improves about 20–30%; the eight-field form costs 9.4% compiler time and 176
  native bytes. Required branch/condition costs and all outliers remain disclosed.
- Member-pointer formation/application is constant work, with one 16-byte pair
  slot/copy per materialized value/boundary and a signature per canonical type.
  Common large compiler median rises 1.71%; assignment rises 1.37%, peak RSS 5.9%.
  Compiler text grows 8960 bytes (.94%); common native bytes are identical.
  New function/data-member paths scale linearly; no runtime gain is claimed.
- Zero plans add .67% compiler text. Common native bytes are identical; required
  typed stores cost 3.37% compiler median time, 3.84% peak RSS and 3.92% runtime
  on the boundary workload. Null-array IR is identical in size at 9/1024 elements.
  The frozen campaign completed; required representation costs create no new gate.

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
| `ce2d8363`: member pointers and scalar assignment widths | 249/257 |
| `b5645333`: zero-initialization actions and null representation | 250/257 |

`3da4de09` corrected four bit-field reference retypes under the authorized
exception; [proof and bundle revision](reference-corrections.md) remain.
No fixture, reference or comparison rule changed in the latest groups.

Latest entry: clean `4cefbabe`, freshly checked **248/257**. Now **250/257**:
**two existing failures removed, none added**, unchanged coverage; **189 stage-base
failures removed**. **62** personal source checks pass. Earlier **1327/1327**
passes after the final refinement.
File audit passes with three existing header advisories. Required stage pass is
not claimed. Root reports run serially.
Logs: `/tmp/pa12-zero-stage3.log` (exit 2),
`/tmp/pa12-zero-final-prior.log` (exit 0),
`/tmp/pa12-zero-personal3.log` (exit 0). Both frozen performance campaigns
completed; `8cfcc3fe` records the member-pointer measurements.

Boundary: zero plans establish initialization values and representation, not
whether later code can observe identity or padding. Omitting empty return
storage or changing a nontrivial parameter ABI needs a separate body/transfer
proof. Terminal conditional cleanup needs a final-consumer fact that separates
completed branch destinations from temporaries still used by an enclosing call;
unconditional branch cleanup would regress passing lifetime controls.
