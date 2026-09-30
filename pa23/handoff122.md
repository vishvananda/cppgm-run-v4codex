# Implementation handoff 122: nonvirtual polymorphic lifecycle ownership

This is an incomplete **PA23 full-stage implementation handoff**. The remaining
implementation and independent whole-stage audit are still required. The entry
was `26b96f27`; the implementation is `a04a8cae`. Review markers remain at
`f33dd0775073bf5db6665fb2f4f504783159df76`.

## Completed behavior, owners and data flow

- **Class completion and vtable demand** (`semantic/virtuals.cpp`) distinguish
  a reference to a key-owned table from a demand for its local definition.
  An unavailable key definition leaves the table external; it does not demand
  its virtual bodies, deletion entries or RTTI. A later key definition wakes
  precisely its owning table through the existing reverse dependency. Class
  templates retain their instantiation-owned tables. Both states are monotonic;
  definition failure is preserved rather than converted to an external success.
- **Layout** records each physical secondary segment's address point within the
  key-owned group. Alias views share that physical point. The typed class/view
  identity and slot slice remain the source of layout truth. Covariant primary
  slot growth shifts the already recorded secondary points. Offset indexes store
  view IDs, avoiding truncation of a byte displacement into a 32-bit map value.
- **Lowering** emits external group declarations or locally defined groups from
  those facts. Keyless course tables retain separate private view identities.
  Constructor/destructor vptr stores use the recorded group points. A later TU
  replaces a declaration in the existing typed global record; it neither loses
  the definition nor creates a duplicate global. Both TU orders execute correctly.
- **Lifecycle demand** records directly needed destructors even when their class's
  table is external. This prevents a call to an undeclared symbol during a
  nonthrowing-boundary inspection. One common entry-identity decision prevents a
  base-only destructor from reserving duplicate complete/base records. Base-only
  generated entries receive base ABI
  identity directly, independently of inlining. Complete entries remain distinct
  when demanded.
- **Deleting entries** use the required small O0 shape for at most two nontrivial
  prepared subobject actions. A larger suffix, or a source body with statements,
  calls the complete destructor once. The threshold is an explicit work/growth
  bound: source bodies are never cloned, and a deletion path performs each
  destructor and deallocation exactly once, including an escaping exception.
- **Function publication** schedules lifecycle entries after the terminate adapter
  has emitted its runtime declarations. The previous order left declarations
  outside the published function schedule when a destructor required termination
  handling. Emission identity and presentation ordering remain separate.

The new facts have translation-unit ownership and stable entity/view IDs. Table
and function identities live in the output Program and survive merged TU input.
Base aliases publish through the same typed ABI index as real entries. A previous
base definition suppresses a duplicate alias; a declaration-to-definition upgrade
keeps its already published function schedule position. The controls cover this
case as well as covariance in both TU orders.
Each table reference, definition, group and entry publishes once; completing an
external reference does not repeatedly recheck unrelated keys. Added layout and
emission work is O(views + produced rows), with average O(1) indexed lookup.
Temporary group data releases after publication. No grammar replay, semantic
reconstruction through text, source-tree clone, global retry or optimizer was
introduced. Telemetry separately reports defined and external tables.

## Reference correction

The destructor diamond uses **nonvirtual** base-specifiers, so it has two `B0`
subobjects. Its reference VMI flags field incorrectly said zero. The one-field
0→1 correction has a reduced executable, cited N3485 [class.mi]/4 and
[expr.dynamic.cast]/8 rules, ABI layout proof and pinned bundle revision in
[reference-correction122.md](reference-correction122.md). No source fixture,
expected exit status, comparison rule, body or vtable slot was changed. The
apparent missing-resume diff was an ABI-entry naming mismatch during the existing
comparator's nothrow canonicalization; our LowIR had valid resume terminators.
The base-only entry fix resolves it without changing the comparator.

## Validation and evidence

The final sealed results are in
[validation122.json](../student.tests/pa23/validation122.json).
Required PA23 tests improve **17/45 → 20/45**, with **three existing failures
resolved, no new failures, and all 45 fixtures retained**. Earlier PAs pass
**3811/3811**. The through-PA23 report is **3831/3856**; advancement is not allowed.
The required file audit passes with the same three inherited header warnings.

[Explicit lifecycle controls](../student.tests/pa23/controls122.json) pass
**20/20**: secondary-view delete, implicit two/three-base deletion, virtual calls
while destructing, throwing base cleanup and one-time deallocation, key definition
arrival (including inline and template definitions), repeated-base RTTI,
separate objects and merged TUs in both orders, nested secondary views, covariant
primary-slot growth with secondary tables, base declaration/alias upgrades, and
4/16/64-base growth. Every emitted control passes explicit LowIR validation;
telemetry leaves its IR unchanged. Native observations compile our LowIR with the
supplied backend and host-link objects; no host compiler implements source output.
[Standalone execution](../student.tests/pa23/native122.json) passes **19/20**.
The throwing-deletion case reaches a duplicate native object-label error in the
supplied standalone backend. [The reference probe](../student.tests/pa23/backend-limit122.json)
reproduces that error with source-generated reference IR, including after only
private metadata names are disambiguated in scratch. Both original reference IR
and our unmodified IR execute successfully in the hosted backend lane. This is
a recorded supplied-backend limit; no checked-in comparison was altered for it.

[Inherited controls](../student.tests/pa23/controls122-inherited.json) remain
**32/33**, with only the already recorded virtual-member-pointer defect. All
accepted stage outputs are explicitly roundtripped; this is structural evidence,
not a correctness claim for still-failing fixtures. Performance protocol, all
samples, executable checks, compiler latency/RSS, runtime/text and work bounds
are recorded in [performance122.md](performance122.md). Handoff 121's historical
performance evidence and backend-limit observations remain unchanged.

## Concrete remaining boundary

All remaining required failures belong to **shared virtual-subobject layout,
segment-local vbase/vcall rows and virtual-base lifecycle/parameter forwarding**.
These require one canonical shared-subobject owner, separate nonvirtual extents,
complete/base lifecycle actions, typed hidden pointer parameters, and dynamic
projection at references/pointers. The completed nonvirtual group's static
segment offsets cannot supply those facts. Extending static tables or reusing
nonvirtual lifecycle actions would produce incorrect complete-object behavior.
The final-overrider rejection and inherited-slot requirements are retained.

The proposed member-value expansion was investigated but not implemented here.
A virtual member pointer needs consistent formation, static data, conversions,
value proofs and call decoding. Inverse conversions can put a virtual target in
a member pointer whose named owner is **nonpolymorphic**. Existing unknown
nonvirtual member-pointer calls have an established PA22 LowIR contract; an
owner-only decoder or a dispatch wrapper that cannot forward arbitrary variadic
arguments would leave correctness holes. This is a distinct member-value ABI
owner, not another vptr-store or lifecycle fix. The failing executable remains
preserved. It must be resolved with that representation's full data flow in a
subsequent implementation group.

The initial lifecycle scope was extended through deletion exceptions, generated
termination scheduling, nested views, and separate/merged TU ownership because
the same facts supported those fixes. Further work now crosses the shared-base
and member-value representation boundaries above. None is waived or called an
independent-review question merely to end this handoff.

## Independent review remains pending

Review must assess the demand/reference split, key-owned group offsets and
cross-TU publication, lifecycle ABI identities, bounded cleanup policy,
reference proof, semantic/emission ownership and performance evidence. The
inherited pipeline and whole-stage spec obligations also remain review work.
No implementation commit has been independently reviewed in this turn, and no
review marker has advanced. This handoff returns control to Ralph; it does not
certify PA23 or begin PA24.
