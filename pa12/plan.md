# PA12 implementation plan

Stage base commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Last reviewed commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Target: PA12 full-stage. Phase: implement; incomplete (**255/257**).

## Design/spec alignment and remaining groups

Keep canonical identities, indexed demand, the shared source graph and typed
conversion/lifetime records. Lowering consumes selected facts without source
replay, fake AST, reference delegation or a later native-backend exit gate.

| Owner | Data flow / complexity | Remaining validation |
| --- | --- | --- |
| Storage-unit initialization | Existing transfer-unit facts -> constructor masks/stores; linear per field/unit | `300-bit-field-copy-semantics`; preserve the identical PA11 constructor's course form |
| Parameter representation | Completed copy actions -> representation/identity proof -> independent argument/result ABI; cached per class | `300-direct-object-parameter-passthrough-base-copy`, including typed pointer constants; preserve observing constructors and nontrivial copy effects |
| Conversion materialization | Selected explicit conversion and transfer -> destination, helper demand, lifetime; one record per use | `400-direct-init-class-explicit-conversion`; preserve passing conversion-result elision controls |
| Scalar initialization consumption | Final scalar destination -> branch result store and cleanup; existing conversion/control edges | `500-direct-class-call-temporary-destination`; preserve both reachable branches and enclosing temporary lifetimes |

Completed owners include lists/defaults, delegation/unions, value boundaries,
references, allocation/aggregates, region/destructor-boundary separation,
member-pointer values/signatures, typed zero plans and terminal class returns.
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
[zero](zero-performance.md) and [consumption](consumption-performance.md) retain
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
- Consumption adds 1728 compiler text bytes (.18%). Common native bytes remain
  identical; large compiler median cost rises 1.69% within disclosed VM noise.
  Terminal runtime pairs improve about 1%, with 152 fewer native payload bytes;
  unknown-call runtime has no repeatable winning direction. Work scales linearly.
- The original loop image regressed about 27x. Exact native data relocation,
  preserving instruction positions/opcodes, isolates a writable counter from a
  hot executable cache line and changes B from 4.85777 to .19147 seconds. Both
  raw and relocated observations remain. This is a demonstrated supplied-backend
  layout constraint owned by PA24; no compiler padding workaround was introduced.
  The relocated A/B follow-up is noisy and establishes no loop speed benefit.
- Empty-object zeroing remains and passes the course comparison. The earlier
  assumed need to eliminate that padding was a diagnostic assumption, not a
  mandated gate; no reference, comparison rule or coverage changed.

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

`3da4de09` corrected four bit-field reference retypes under the authorized
exception; [proof and bundle revision](reference-corrections.md) remain.
No fixture, reference or comparison rule changed in the latest groups.

Latest entry: clean `b1e936e8`, freshly checked **250/257**; prior goal turn made
verified progress. Final **253/257**: **three existing failures removed, none
added**, unchanged coverage; **192 stage-base failures removed**. Earlier
**1327/1327**, all **63** personal sources, both explicit LowIR property scripts,
file audit (three existing header advisories) and diff checks pass. Root reports
ran serially. Required stage pass is not claimed.

Logs: `/tmp/pa12-consumption-final-stage.log` (exit 2),
`/tmp/pa12-consumption-final-prior.log` (exit 0),
`/tmp/pa12-consumption-final-personal.log` (exit 0). Explicit properties:
`python3 student.tests/pa12/check_terminal_returns.py` and
`python3 student.tests/pa12/check_zero_initialization.py`.
Both frozen performance campaigns completed; `3789b6a3` preserves the initial
native layout experiment, with the continuation recorded in its JSON.

Boundary: the return destination and no-unwind owners are complete. A scalar
initializer needs its final store before temporary cleanup, so it cannot inherit
the class-return flag blindly. Nonthrowing copy bodies can still have observable
copy effects; the remaining ABI case needs a distinct representation proof.
Storage-unit emission and explicit-conversion materialization likewise need
separate typed actions rather than further changes to return cleanup flags.

Storage entry: clean `9e442405`, fresh **253/257** and all 13 controls. Prior
turn made verified progress. Reuse demanded allocation-unit transfer facts for
constructor unit initialization: evaluate the initializer, read retained bits,
pack the new value, then form the final store address. Owner/data flow is
completed field/unit metadata -> typed constructor store action; constant work
per field, no extra layout scan. Preserve the PA11 path without transfer demand.
Validate shared/split units, volatile fallback, initializer side effects and
copy/move/assignment together, then extend conversion materialization.

Storage validation: **254/257**, all 13 controls, **64** personal sources and
both explicit property scripts pass; earlier **1327/1327** and file audit pass.
One existing failure removed, none added. Logs: `/tmp/pa12-storage-stage2.log`,
`/tmp/pa12-storage-prior.log`, `/tmp/pa12-storage-personal2.log`. Frozen A is
`/tmp/pa12-storage-base-cppgm`; performance measurement remains pending before
handoff. Continue with explicit conversion-result materialization.

Conversion continuation: selected explicit conversion -> retained second
construction in `UserConversion`/`ConversionObject` -> source/destination
identities, helper demand and ordinary cleanup. Constant records per use;
no resolution in lowering. Validate explicit copy/move effects and lifetime,
empty targets, implicit elision controls and rejection/access behavior.

Explicit conversion validation: **255/257**, all 13 controls, **65** personal
sources, both properties, earlier **1327/1327**, file audit and diff checks pass.
Existing failure removed without regressions or coverage changes. Logs:
`/tmp/pa12-explicit-stage1.log`, `/tmp/pa12-explicit-prior.log`,
`/tmp/pa12-explicit-personal1.log`. Performance campaign covers this group and
`c61ecc10` against the frozen storage-entry binary before handoff.

Performance review: the initial complete campaign found a 2.5x explicit-result
runtime cost from mandatory helper calls for nonempty trivial targets. Preserve
that campaign in `storage-helper-performance.json`. The contract permits direct
`copyobj` here; retain source/destination identity but use the existing trivial
storage operation. Empty retained transfers still need their selected call.
Validate and remeasure before accepting the representation cost.

Direct-storage follow-up reduces bytes but still costs about 2.5x at runtime.
Preserve `storage-direct-performance.json`. Nonempty trivial conversion results
can legally initialize the final destination directly; use cached triviality
for that constant-work elision. Empty/nontrivial explicit transfers remain.
The final campaign also measures observable explicit moves as B-only behavior.

Final review shares the constructor field descriptor between unit dispatch and
ordinary bit-field dispatch, avoiding a duplicate metadata lookup per action.
Budgets for these owners: at most one additional conversion-object record per
retained use, two sparse flags, constant queries per use/field, no extra layout
scan; compiler text growth below 4 KiB and common compiler median cost below 5%
are diagnostic review budgets, not new course gates. Array expansion remains 8.
