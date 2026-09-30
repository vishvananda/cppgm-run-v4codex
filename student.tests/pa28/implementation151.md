# PA28 naming and attribute handoff151

This increment resolves ten of the sixteen entry failures without changing any
course fixture, reference, harness or comparison rule. It is an implementation
handoff; six runtime/layout failures and the independent PA28 audit remain.

## Ownership and data flow

GNU attribute parsing now retains interned ABI tags in the source arena and a
compact ordered callable-effect value. String arguments are checked while
consuming the token cursor, including concatenated literals. No deferred token
replay is introduced. Source occurrences share the original attribute records.
The semantic attribute owner merges tags using (EntityId, IdentifierId) keys
and retains sparse entity-to-list heads. It preserves class forward-declaration
facts and publishes the pattern's tags when declaring a specialization.

Callable effects live on the canonical entity. ReadNone is stronger than
ReadOnly; neither is inferred from a body. A specialization can precede a
stronger template redeclaration, so final signature publication follows its
canonical pattern edge. This does not repeat body work or invalidate unrelated
specializations. The LowIR call-boundary metadata receives the selected effect,
including when several source TUs merge a declaration and definition.
No-inline/always-inline facts also survive template publication.

The shared typed ABI graph receives tags on class/name components, templates,
constructors, complete/base/deleting destructors, ordinary functions, variables,
RTTI and vtables. Sorting and substitution remain in the existing encoder.
Local-member names retain a typed function context and tagged local component.
The language/ABI rules are documented by the
[Itanium ABI naming grammar](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)
and [GNU ABI attributes](https://gcc.gnu.org/onlinedocs/gcc-13.5.0/gcc/C_002b_002b-Attributes.html).

Template-template parameter names now use canonical parameter ordinals.
Dependent class argument lists flatten the grouping introduced by semantic
pack matching; actual function-template argument packs retain their pack
encoding. Lambda owner prefixes enter substitutions at their grammar position.
Unnamed local classes retain a function-owned ordinal and an unnamed component
fact instead of exposing the compiler's generated lookup identifier. A typedef
linkage name still wins. The standalone ABI reader/writer preserves that fact.

The new __decay type operation uses the existing typed builtin-query owner.
It removes references, applies array/function conversion, then removes top-level
cv. Dependent queries retain argument identities and reuse substitution caches;
lowering emits a vendor type-transform fact. It never reconstructs a return
type from rendered alias text. Personal assertions cover const references,
array references, functions, pointer cv and void.

## Work and lifetime bounds

Parsing adds O(attribute bytes/tags) work. Source tags and semantic tag edges
use contiguous TU-owned pools; flat indexes provide average O(1) completed
membership/head lookup. Untagged entities allocate no tag records. Graph tag
canonicalization costs O(k log k) per tag list. Mangling consumes canonical type,
argument, scope and context identities in proportion to the emitted name.
The existing depth guards and flat substitution tables remain in force.

Local numbering uses function/name identity, with a separate unnamed sequence.
No global declaration search, name-based recovery, semantic-tree copy, text IR
transport, new optimization fixed point, or speculative code growth is added.
Function-local LowIR/MIR and direct ELF ownership are unchanged. These changes
add no new level policy: PA28 measures O0; PA32/33 optimization and PA34
self-hosting acceptance remain at their owning stages.

The entity effect byte occupies existing padding; rare tag heads use the
sparse semantic table. Final telemetry verifies Entity stays at the entry's
120 bytes. Earlier 128-byte measurements are retained in the evidence directory.

## Validation and remaining boundary

The required suite exercises raw object names, imported/exported ABI behavior
and the malformed-tag rejection. The personal cross-object control compiles
the implementation with this compiler, consumes its tagged constructors,
destructor, virtual method, function and variable with host C++, then executes.
LowIR inspection checks ordinary, redeclared, instantiated and late-strengthened
effects; an unannotated mutating function remains conservative. The standalone
unnamed-type control checks the ABI adapter without using a live host-name
oracle. See the final validation manifest for all commands and statuses.

Remaining implementation work is concrete:

* Covariant layout finalization encodes a virtual result row of -24 where the
  fixture requires -32. The shared mangler faithfully encodes the semantic
  BaseAdjustment row; fixing it requires consistent vcall/vbase address-point
  and projection facts, not a spelling change.
* Virtual-base return conditions reference unprovided auxiliary vtable views
  across the host boundary. Lazy template-base RTTI compilation reports an
  unavailable semantic prerequisite. Those require layout/support-object
  ownership and class-demand scheduling work.
* Dynamic exception specifications need host unexpected/filter behavior;
  rethrow and cleanup-resume still need correct outer/lexical lifetime state.

These six failures cross into physical object layout, lazy completeness and
EH/LSDA state. Extending this naming increment into those owners would require
new semantic proofs and host-runtime controls. This is the coherent handoff
boundary, not a waiver or a claim that PA28 is complete.

Independent audit still owes cumulative review of these changes, tag/effect
publication and cache lifetimes, template/local substitution ordering, and the
performance evidence. The stage base and last-reviewed markers are unchanged.
