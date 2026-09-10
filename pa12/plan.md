# PA12 implementation plan

Stage base commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Last reviewed commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Target: PA12 full-stage. Phase: validate; required stage checks pass (**257/257**).

## Design/spec alignment and remaining work

Keep canonical identities, indexed demand, the shared source graph and typed
conversion/lifetime records. Lowering consumes selected facts without source
replay, fake AST, reference delegation or a later native-backend exit gate.

| Owner | Data flow / complexity | Validation |
| --- | --- | --- |
| Parameter representation | Checked copy actions and source identity -> cached argument ABI; independent result ABI and body/emission demand; incremental entity cursor | Declaration-only and linked TUs, copy effects/identity/escapes, parameter slots, typed pointer constants pass |
| Scalar initialization | Recorded writes/exposure -> typed final conversion, destination and integral truth proof -> branch store before cleanup; one candidate-initializer walk and constant work per use | Dynamic/constant/volatile/modified conditions, aliases, narrowing and destructor-observed destinations pass |

Implementation groups are complete. Remaining work: final scalar performance
campaign and the root through-PA12 completion report, then final clean audit.

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
| `f3e7ce93`: parameter transport proof, typed pointer constants | 256/257 |
| Scalar initialization consumption (this increment) | 257/257 |

`3da4de09` corrected four bit-field reference retypes under the authorized
exception; [proof and bundle revision](reference-corrections.md) remain.
No fixture, reference, comparison rule or coverage changed in the latest groups.

Current entry: clean `eb0a4251`, freshly checked **255/257**. Parameter
`f3e7ce93` removed one existing failure; scalar consumption removes the last.
Current **257/257**, all **13** controls, earlier **1327/1327**, all **67**
personal sources and three explicit property scripts pass. No coverage reduced.
File audit passes with the same three existing header advisories; diff checks pass.

Logs: `/tmp/pa12-scalar-final-stage.log`,
`/tmp/pa12-scalar-final-prior.log`, `/tmp/pa12-scalar-final-personal.log`
(all exit 0). Explicit properties: `check_parameter_representation.py`,
`check_terminal_returns.py`, `check_zero_initialization.py` under
`student.tests/pa12/`, each run with `python3`.

Parameter evidence (`51fbedef`): +2752 compiler text bytes (.28%); large common
compile median -.22% with mixed/noisy pairs. All non-parameter native bytes
match A. Required object-parameter transport adds 24 native bytes and about 59%
runtime in its focused workload: a documented PA12 ABI cost, not an optional
optimization or positive-runtime gate. One class state/copy ID, one member flag,
an incremental entity cursor and linear member/source inspection bound work.
The 6 KiB text and 5% common median review targets were met.

Scalar diagnostic budgets, set before measurement: at most one consumer record
per automatic scalar initializer and one sparse observation entry per modified
or exposed object; one walk of each candidate initializer; constant work per
use and no new per-node graph. Review compiler text growth above 8 KiB or common
compile median growth above 5%; these are diagnostic targets, not course gates.
Frozen A is `/tmp/pa12-scalar-base-cppgm` (`f3e7ce93` code). Compare dynamic and
known conditions, observed destinations, modified conditions and unaffected code
with A/A+ABBA compiler/RSS and checked native runtime/payload measurements.
