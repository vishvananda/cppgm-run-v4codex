# PA12 implementation plan

Stage base commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Last reviewed commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Target: PA12 full-stage. Phase: implement; incomplete (**256/257**).

## Design/spec alignment and remaining groups

Keep canonical identities, indexed demand, the shared source graph and typed
conversion/lifetime records. Lowering consumes selected facts without source
replay, fake AST, reference delegation or a later native-backend exit gate.

| Owner | Data flow / complexity | Remaining validation |
| --- | --- | --- |
| Parameter representation | Completed copy actions -> identity-preserving transport proof -> independent argument/result ABI; cache per class with a stable body-query dependency | `300-direct-object-parameter-passthrough-base-copy`; typed nonzero pointer constants; observing constructors and cross-TU declarations |
| Scalar initialization consumption | Final scalar destination -> branch result store and cleanup; existing conversion/control edges | `500-direct-class-call-temporary-destination`; both reachable branches and enclosing temporary lifetimes |

Completed owners include lists/defaults, delegation/unions, value boundaries,
references, allocation/aggregates, region/destructor-boundary separation,
member-pointer values/signatures, typed zero plans, terminal class returns,
constructor storage units and explicit conversion-result boundaries.
Return cleanup now belongs to its function even across loops. Terminal branches
finish only their private suffix; nested conditionals feeding an enclosing
transfer/call retain their selector. All **13/13** survivor controls pass.
The scalar transfer proof rejects calls, class subobjects and unknown operations;
it does not strengthen a language exception specification or public ABI.

## Performance evidence and budgets

[Member](performance.md), [transfer](transfer-performance.md),
[value](value-performance.md), [conversion/reference](conversion-performance.md),
[allocation/aggregate/alias](allocation-performance.md), [list](list-performance.md),
[boundary](boundary-performance.md), [cleanup](cleanup-performance.md),
[destructor](destruction-performance.md), [member-pointer](member-pointer-performance.md),
[zero](zero-performance.md), [consumption](consumption-performance.md) and
[storage/conversion](storage-performance.md) and
[parameter](parameter-performance.md) retain
frozen hashes, inputs, flags, A/A+ABBA data, compiler latency/RSS and runtime/text.
Historical misses and outliers remain. Required representation costs and proven
later-backend constraints do not create positive-runtime PA12 exit gates.

- Local array expansion remains capped at eight total elements. Zero plans cache
  each type once, reuse child extents, expand at most eight padding stores, and
  use bulk operations or fixed-size loops beyond those bounds.
- Full-expression classifiers use at most three bytes per AST node. Scalar
  transfer proofs add a lazy byte per AST node, one member flag, and one visit
  per examined node. No call-graph search, source replay or global retry is added.
- Destructor suffixes inline at most eight actions (28 duplicated tail actions),
  then share one block per suffix. Member-pointer pair storage/copy is bounded
  at 16 bytes per value/boundary; signatures use canonical types.
- Consumption adds 1728 compiler text bytes (.18%). Its original loop runtime
  regression is isolated to the supplied backend's code/data cache-line sharing:
  native data relocation preserves instruction positions and removes that cost.
  Both raw and relocated observations remain; placement belongs to PA24.
- Storage/conversion budgets: constant queries per field/use, no extra layout
  scan, two sparse flags, at most one added conversion-object record per retained
  use. Below 4 KiB compiler text growth and 5% common median compile cost are
  diagnostic review budgets, not added course gates. Preserve all observations.
- Two intermediate conversion policies cost about 2.5x at runtime. Direct storage
  reduced bytes but not runtime. Cached trivial-result elision removes that cost:
  a paired helper/elision campaign measures .25780 -> .10575 seconds and
  311 -> 277 native payload bytes. Empty/nontrivial explicit boundaries remain.
- Final storage/conversion text growth is 1920 bytes (.20%); the large common
  compiler median rises .12%, with unchanged common/trivial-explicit native bytes.
  Unit runtime rises .57%; all observations and intermediate costs remain.
- Empty-object zeroing passes the course comparison. The earlier assumed need
  to remove its padding was a diagnostic assumption, not a mandated gate.

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
| `260e0b35`: terminal return branches and function-owned return cleanup | 252/257 |
| `14dac876`: scalar transfer proof and materialization guards | 253/257 |
| `c61ecc10`: constructor storage-unit ordering | 254/257 |
| `8a5a370d`: explicit conversion-result boundaries | 255/257 |
| `b699f183`, `1eae0974`, `2d693f2b`: measured policy correction and shared field facts | 255/257 |

`3da4de09` corrected four bit-field reference retypes under the authorized
exception; [proof and bundle revision](reference-corrections.md) remain.
No fixture, reference, comparison rule or coverage changed in the latest groups.

Latest entry: clean `9e442405`, freshly checked **253/257**; prior turn made
verified progress. Final **255/257**: **two existing failures removed, none
added**; **194 stage-base failures removed**. Earlier **1327/1327**, all **65**
personal sources, both explicit LowIR property scripts, file audit (three existing
header advisories) and diff checks pass. Root reports ran serially. Stage pass
is not claimed.

Logs: `/tmp/pa12-storage-facts-stage.log` (exit 2),
`/tmp/pa12-storage-facts-prior.log` (exit 0),
`/tmp/pa12-storage-facts-personal.log` (exit 0). Explicit properties:
`python3 student.tests/pa12/check_terminal_returns.py` and
`python3 student.tests/pa12/check_zero_initialization.py`.
Performance campaigns preserve partial calibration and every intermediate policy;
final evidence and reproduction commands are in the linked storage report.

Boundary: allocation-unit initialization and explicit result materialization are
complete. The ABI survivor needs an identity-preserving representation proof;
a nonthrowing body alone is insufficient. Inline copy-body facts must also be
available consistently in a TU that merely declares a by-value function, so the
next owner needs a body-query/emission-demand separation, not a lowering flag.
Scalar initialization must perform its final store before temporary cleanup and
cannot inherit class-return consumption blindly. Its reference's inactive arm
also needs a reachability review; changing the condition is not proof that the
original reference is wrong. Continue these two owners without weakening rules.

Parameter entry: clean `eb0a4251`, fresh **255/257**; previous turn made
verified progress. Own a cached representation query per completed class and
separate semantic body demand from emission. Query only an in-class empty copy
body whose sole full-storage base can transfer trivially; consume its checked
actions and source identity. An incremental function cursor discovers queries
without global retries. Validate declaration-only TUs, copy effects/identity,
parameter slot transport and typed integer-to-pointer constants together.

Parameter group validation: **256/257**, all 13 controls, **66** personal
sources, all three explicit property scripts, earlier **1327/1327**, file audit
and diff checks pass. One existing failure removed, none added. Proof rejects
comma-expression escapes and nontrivial copy effects; declaration-only and
linked multi-TU checks agree. Query-only bodies do not receive unrelated scalar
transfer proofs. Frozen A is `/tmp/pa12-parameter-base-cppgm`; performance remains
pending before handoff. Query budgets are one class state/copy ID, one member
flag, incremental entity traversal and linear member/source inspection; 6 KiB
compiler text and 5% common compiler median are diagnostic review budgets.
Continue the scalar-initialization owner before choosing a handoff boundary.

Parameter performance: frozen `f3e7ce93`, +2752 compiler text bytes (.28%);
large common compile median -.22% with mixed/noisy pairs. All non-parameter
native bytes match A. Required object-parameter transport adds 24 native bytes
and about 59% runtime in its focused workload; this is a documented PA12 ABI
cost, not a positive-runtime gate. All observations remain in the linked report.
