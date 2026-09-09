# PA12 implementation plan

Stage base commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Last reviewed commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Target: PA12 full-stage. Phase: implement; incomplete.

## Design/spec alignment and remaining groups

Retain the shared source graph, canonical TypeId/EntityId keys, demand queue,
recorded conversions/lifetimes and own typed LowIR. Lowering consumes selected
facts; no source replay, reference delegation or later native-performance gate.

| Owner | Data flow / complexity | Validation and remaining work |
| --- | --- | --- |
| Member and special-member semantics | Canonical declarations -> selected member -> field/base/storage actions; once per required edge | Qualification, delegation, unions, copy/move/deletion and bounded arrays pass. Bit-field ordering and rooted-helper/emission differences remain |
| Class values and conversions | Indexed canonical conversion targets/base edges -> object and second-standard sequence -> selected call/result transfer/destination | Conversion, inherited/ref-qualified ranking, builtin, cast, condition, surrogate and reference checks pass. Candidate work follows required overloads; selected records are retained once. Aggregate/braced returns/default arguments, empty-class conversion-result transfers and other ABI/O0 output shapes remain |
| Reference and cleanup ownership | Complete temporary identities -> namespace backing storage and conditional shutdown guards; O(retained objects + branch edges) | Static-reference control and mixed-type/subobject/conversion personal checks pass. Remaining arrays, cleanup-region/continuation shapes, and local subobject-reference generalization require lifetime-owner work |
| Call boundary facts | Source GNU attribute -> validated function EntityId -> LowIR query boundary; no optimization | Stable-prefix probe, positive control and all four rejection controls pass. Survivor runner next fails the indirect member-pointer object-extent control; that needs its representation and call signature |
| Allocation | Canonical scalar/array names -> selected overloads, typed bounds/cookies and lifetime actions -> bounded forward/reverse loops | All required allocation/deletion fixtures pass. Scalar/class/global/placement selection, sized delete, multidimensional arrays, null paths and wide extents have executable personal checks. Work is O(candidates + selected actions), independent of runtime element count |

Unchanged contract questions: PA11 `400-bit-field-constructor-member-init`
and PA12 `300-bit-field-copy-semantics` prescribe different orders for the same
`Bits` constructor; a general change previously regressed PA11 and was removed.
`300-direct-object-parameter-passthrough-base-copy` also needs ABI-contract
investigation. No fixture-dependent behavior or unproved reference revision.

## Performance evidence

[Member](performance.md), [transfer](transfer-performance.md),
[value](value-performance.md) and [conversion/reference](conversion-performance.md)
preserve frozen binaries, flags, inputs, A/A/ABBA samples, compiler latency/RSS,
executable runtime/text and work counters. Common LowIR/native outputs are
byte-identical. Nested branch sharing removed quadratic preparation; conversion
records grow 4000 -> 16000 at fourfold input size. No runtime gain is claimed
for required semantic additions. All three conversion campaigns are preserved.

Keep canonical class/member facts, shared conditional transfer records, memoized
cleanup-presence queries and persistent suffixes. Array expansion stays capped
at eight elements. The unprofitable scalar-only prefix fold remains removed;
mandated storage-prefix/whole-object forms retain their documented supplied-
backend cost without an extra positive-runtime gate. New storage and guards
are bounded by selected objects; query annotations do not request O0 transforms.

## Handoff ledger

- Stage entry: clean base; **61/257**, 196 failures; earlier **1327/1327**.
- `4e209677`, `55d15ba8`, `8a83045c`: members/delegation/unions -> **92/257**.
- `3da4de09`: four reference bit-field retypes corrected under the authorized
  exception; [proof and reducers](reference-corrections.md).
- `73664568`, `4ea8394f`: special transfers and measured prefix policy -> **123/257**.
- `cc2198c9`, `e140daef`, `a3d40a62`, `1a7867fd`: class boundaries/destinations,
  conditional/local-reference ownership and bounded branch sharing -> **169/257**.
- `02f2f154`, `77145afa`, `f9c8e6c3`, `70556b3d`: conversion selection,
  reference storage and query/abort boundaries -> **202/257**, prior **1327/1327**.
  All three measurement campaigns remain in conversion-performance.md; final
  logs are `/tmp/pa12-conversion-final-{stage2,prior2,personal}.log`.
- Allocation entry: clean `70556b3d`, freshly verified **202/257**, 55 failures.
  Current allocation group: **223/257**, **21 entry failures removed, no new
  failures**, unchanged coverage. All required new/delete comparisons pass.
  Earlier **1327/1327**, file audit passes with the same two header advisories.
  Thirty-four personal source checks pass, including six allocation reducers.
- Allocation ownership: source placement/type disambiguation -> canonical
  allocation family -> selected conversion/constructor/destructor/deallocator,
  byte stride and leaf-count cookie -> typed O(1)-size heap loops and partial
  cleanup. Only the first bound is dynamic; the bound runs once. An O(1)
  constant-return proof permits required source-width extent arithmetic;
  otherwise widening precedes multiplication. Constant size overflow rejects.
  Mixed scalar/array runtime entries retain distinct addresses through a tiny
  adapter around each singleton LowIR memory role.
- Validation logs: `/tmp/pa12-allocation-final-stage2.log` (223/257, exit 2),
  `/tmp/pa12-allocation-final-prior1.log` (1327/1327, exit 0),
  `/tmp/pa12-allocation-personal4.log` (exit 0). Root reports run serially.
  Frozen allocation A/A+ABBA and absolute/scaling campaign is in progress.
- Remaining: nine compile rejections belong to aggregate/list/reference
  initialization and destructor alias lookup. Twenty-five LowIR differences
  belong to transfer/helper emission, lifetime continuations, and the two
  inherited bit-field/base-copy contract questions above. The survivor runner
  still reaches member-pointer boundary control 536, which needs its distinct
  representation and indirect signature. No fixtures or references changed.
