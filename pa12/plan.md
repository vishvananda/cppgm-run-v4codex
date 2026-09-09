# PA12 implementation plan

Stage base commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Last reviewed commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Target: PA12 full-stage. Phase: implement; incomplete.

## Design/spec alignment and remaining groups

Keep the shared source graph, canonical TypeId/EntityId facts, demand queue and
own typed LowIR. No source replay, reference delegation or later native gate.
Completed owners: member/ref-qualified selection; delegation and union storage;
implicit/defaulted copy/move actions; class boundaries, destinations and returns;
conditional transfers and local-reference/temporary identities. Lowering consumes
selected facts. Prvalue conditional destinations share existing branch records.

| Owner | Data flow / work bound | Validation and remaining work |
| --- | --- | --- |
| Member and special-member semantics | Canonical declarations -> selected candidates -> typed field/base/unit/storage actions; once per required edge | Qualification, delegation, unions, copy/move/deletion, noalias and bounded arrays pass; inherited bit-field ordering conflict remains |
| Class ABI and destination lowering | Boundary classification + selected transfer + destination/return records -> direct/indirect parameters/calls/results; O(required declarations + expressions + actions) | Direct/indirect calls and returns, forwarding, cv-combined conditional copies, local-reference extension pass. Remaining: aggregate/braced returns/default arguments, explicit class casts and source-specific ABI/emission differences |
| Conversion functions and overloads | Indexed conversion-function candidates -> recorded user/standard sequences -> builtin/member calls | Main remaining semantic owner: scalar/pointer/class conversions, inherited/ref-qualified candidates and class condition declarations |
| Static/array lifetime and cleanup emission | Complete temporary/storage identities -> scope/shutdown/array actions -> shared cleanup suffixes | Static subobject-reference control still rejects; arrays and condition lifetimes remain. Nine full-LowIR lifetime fixtures execute correctly but their comparisons still fail |
| Allocation | Selected allocation/deallocation + storage extent -> construction/destruction and null branches | Scalar/array new/delete, class-specific selection and nonthrowing-null paths remain |

The ordinary bit-field initializer conflict is unchanged: PA11
`400-bit-field-constructor-member-init` and PA12 `300-bit-field-copy-semantics`
prescribe different orders for the identical `Bits` constructor. A general
change regressed earlier checks and was removed. No test-dependent compiler
branch or unproved reference change was added. The direct-object base-copy
fixture also needs ABI-contract investigation; normal nontrivial user copy
constructors select indirect boundaries. Do not copy an isolated reference anomaly.

## Performance evidence

[Member](performance.md), [transfer](transfer-performance.md) and
[value](value-performance.md) evidence preserve frozen binaries, flags, inputs,
A/A/ABBA samples, latency/RSS, runtime/text and work counters. Common LowIR and
native outputs remain byte-identical. Final value workloads have exactly 4x
return/conversion records at 4x input size. Nested branch sharing cuts paired
compiler time 76–78% at depth 64 and 93–94% at depth 256, with unchanged output;
at depth 256 RSS falls 504500 -> 26608 KiB. This corrects repeated semantic work.
No runtime gain is claimed. Compiler text grows 2.24% for the entire value group.

Keep cached class/member facts, two branch transfers per conditional, memoized
cleanup-presence queries and persistent branch suffixes. Array expansion stays
capped at eight elements. The previously unprofitable scalar-only prefix fold
remains removed. Required storage-prefix/whole-object forms retain their measured
supplied-backend cost; they do not create an additional positive-runtime gate.
Initial noisy campaigns and the final repeats are both preserved.

## Handoff ledger

- Stage entry: clean base; **61/257**, 196 failures; earlier **1327/1327**.
- `4e209677`, `55d15ba8`, `8a83045c`: member/delegation/union group, **92/257**;
  31 original failures removed, no new failures. Concurrent root tallies were
  discarded and replaced by serial reports; always run root reports serially.
- `3da4de09`: four bit-field reference retypes corrected under the authorized
  exception; [C++11/LowIR proof and reducers](reference-corrections.md).
- `73664568`, `4ea8394f`: special transfers and measured prefix policy,
  **123/257**; another 31 failures removed, no new failures.
- Current entry verified clean `4ea8394f` and **123/257** in
  `/tmp/pa12-abi-start.log`. `cc2198c9` adds class boundaries/destinations;
  `e140daef` extends conditional and temporary ownership; `a3d40a62` shares
  nested branch conversions instead of rebuilding them.
- Final current checks: **169/257**, 88 failures; **46 entry failures removed,
  no new failures**, unchanged coverage. Earlier report **1327/1327**. File audit
  passes with its existing header advisory. Nineteen personal source checks,
  two focused lifetime/elision controls and nine explicit lifetime-fixture native
  executions pass. The latter nine still fail their required LowIR comparisons.
  No fixtures, sidecars, comparison rules or reference outputs changed this turn.
- Required logs: `/tmp/pa12-value-final-stage.log` (exit 2),
  `/tmp/pa12-value-final-prior.log` (exit 0),
  `/tmp/pa12-value-final-personal.log` (exit 0). Benchmarks and focused checks
  are complete; final cleanup/file audit and commit status are recorded below.
- Concrete boundary: the completed destination model accepts already selected
  class transfers. Remaining conversion functions require their own candidate
  and two-stage conversion records before a destination can consume them.
  Static references additionally need a complete backing-object/shutdown owner
  (including selected subobjects); local full-expression state cannot represent
  that lifetime. Remaining cleanup comparison shapes need an emission-policy
  review, not another scalar/class destination patch. These separate owners make
  further fixes impractical as an extension of this now-tested behavior group.
  PA12 remains incomplete; **108 stage-base failures removed overall**.
- Final file audit and `git diff --check` pass. Implementation and evidence are
  committed; no tests or benchmarks remain running. Working tree is clean at handoff.

- Conversion entry: clean `1a7867fd`; previous goal turn is verified progress.
  Fresh baseline **169/257** in `/tmp/pa12-conversion-start.log`; no live prior work.
  Active owner: canonical conversion-function targets and inherited candidate
  edges -> object/second-standard sequence selection -> existing calls and
  destinations. Bound work by required candidates and base edges; preserve
  local source facts. Validate casts, references, overload ranking, builtins and
  conditions together, then measure common and affected workloads.
- Conversion implementation checkpoint: **201/257**, 32 entry failures removed,
  no new failures; prior **1327/1327**, 27 personal sources pass. Canonical
  target bindings, two standard sequences, selected result transfers, builtin
  candidates, condition declarations, casts and surrogate calls now share the
  conversion owner. Inherited ranking follows [over.match.funcs]/4 while the
  call retains its separate base adjustment. Logs: `/tmp/pa12-conversion-complete-stage.log`,
  `/tmp/pa12-conversion-prior.log`, `/tmp/pa12-conversion-personal2.log`.
  Performance campaign remains pending; no optimization benefit is claimed.
