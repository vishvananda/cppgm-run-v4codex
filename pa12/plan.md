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
| Allocation | Selected allocation/deallocation + extent -> construction/destruction and null branches | Scalar/array new/delete, class-specific selection and nonthrowing-null paths remain |

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
- Current entry verified clean `1a7867fd`, **169/257**, 88 failures in
  `/tmp/pa12-conversion-start.log`. `02f2f154` adds conversion functions and shared
  operator selection; `77145afa` retains static references and emits query/abort
  boundaries; `f9c8e6c3` preserves dynamic conversions during static initialization.
- Current final: **202/257**, 55 failures; **33 entry failures removed, no new
  fixture failures**, unchanged coverage. Earlier **1327/1327**; file audit passes.
  Twenty-eight personal source checks, explicit abort termination (134), static-
  reference execution, stable-prefix control and four query rejections pass.
  The survivor runner reaches `536-parameter-object-extent-boundary.cpp`, where
  member-pointer calls remain unsupported. No fixtures/references/comparison
  rules changed this turn. Stage-base progress: **141 original failures removed**.
- Logs: `/tmp/pa12-conversion-final-stage2.log` (exit 2),
  `/tmp/pa12-conversion-final-prior2.log` (exit 0),
  `/tmp/pa12-conversion-final-personal.log` (exit 0). Root reports ran serially.
- Concrete boundary: conversion selection and destination/storage ownership are
  now exercised together. Remaining member-pointer calls need a distinct ABI
  value representation and indirect signature; allocation needs allocation-
  extent/deallocation records; remaining lifetime diffs need exception-region
  and continuation emission, including arrays. These are separate semantic/ABI
  owners, not further conversion-ranking or backing-storage fixes. They remain
  required work for full PA12, including every still-failing output comparison.
- Final file audit and `git diff --check` pass; two header-organization advisories
  remain. All three performance campaigns finished. The final common output is
  byte-identical to A; compiler text cost is 63616 bytes (7.63%). Measurements
  and their noisy observations are preserved in the linked evidence.
