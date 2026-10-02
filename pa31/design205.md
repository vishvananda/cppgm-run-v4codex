# PA31 implementation205 ownership and validation

Entry: `c0566ded7ed123e1eb0696f0ae93b28ceb492fdf`, 78/84 required tests.
Code increments: `ffea90db`, `d5bd5aee`. No new implementation source is added.

## Hosted allocation and ABI entries

`lowering/symbols.cpp` owns the EntityId → SymbolId → ABI name mapping. Allocation
functions without source definitions remain independent external declarations
in hosted LowIR. The standalone allocation/free roles and adapters remain in
that mode only. Hosted call selection and the ELF writer consume the same
ordinary symbol metadata; there is no object-only suppression list or renamed
library helper. User replacement definitions remain owned by their source TU.
The control links two compiler-produced clients against a host-owned array
allocator and verifies both allocations and releases use that replacement.

Complete constructor/destructor entries cannot alias base entries when the
recorded class layout contains virtual bases. Complete entries handle virtual
bases; base entries have distinct VTT/hidden-argument facts. The existing demand
closure emits a separate base entry when needed. Otherwise it exports only the
complete entry. The predicate reads the completed virtual-base count in O(1),
and introduces no new ABI spelling or lookup. The reduced multi-TU control
checks C1/C2 and D1/D2 ownership, including a derived layout with extra storage.

## Braced initializer exception facts

`semantic/exception_expression.cpp` owns nonthrowing facts keyed by canonical
list-plan and source-node IDs. An initializer-list constructor passes its own
braced source to the backing-array conversion. Following that source node's
incoming conversion re-enters the outer plan. Walk the selected conversion
instead: it owns the element expressions, their conversions, and destruction.
Only the self-reference to that same braced source is excluded. Brace-elided
scalar sources still require expression analysis. Constructor defaults and
nested conversions remain visited. Completed plans retain the existing memoized
fact; no global retry, guessed noexcept result or cycle-breaking false cache is
introduced. Work remains proportional to visited plan/source edges, with one
completed exception fact per existing canonical key.

## Inherited zero-argument construction and validity

`semantic/inherited_constructors.cpp` retains a base's zero-argument constructor
when a user-declared derived constructor suppresses implicit default generation.
The derived implicit default still owns the no-user-constructor case; local
signatures hide inherited signatures, and base copy/move constructors remain
excluded. This supplies the hosted standard library's ordinary inheritance
pattern, without naming a library type. The revised inheritance semantics are
part of the C++11 defect-resolution lineage; see [P0136R1, namespace.udecl and
class.inhctor.init](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0136r1.html)
and [N4429's stated C++11 DR intent](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4429.html).
No broader inherited-constructor redesign is claimed by this increment.

The existing typed forwarding record supplies base initialization and defaults;
source bodies are not cloned or reparsed. Declaration enumeration keeps its
existing candidate/default-arity bound, adding at most one zero-argument entry
per applicable base candidate. `semantic/template_checks.cpp` recognizes the
repeated dependent qualifier in `using T::T` as constructor syntax, not a new
name shadowing the template parameter. Ordinary `using B::T` still rejects.
This remains part of the single source-region parameter check.

`semantic/default_initialization_facts.cpp` now applies its monotonic constructor
validity fact to inherited constructors as well. The selected base constructor
initializes that base; other bases and members must be default-initializable
and destructible. The owner checks these facts without demanding member bodies,
so `__is_constructible` can reject deleted, private and uninitialized-reference
subobjects during candidate probing. Existing EntityId-keyed success/failure
states and incomplete-class retries remain intact. Each constructor scans only
its own direct subobjects, once per completed validity fact. No new container,
TU lifetime or invalidation mechanism is introduced.

## Contract correction and evidence boundaries

The two global relocation fixtures already received PC32 and GOTPCREL, but
expected a non-ABI name. [The reducer and ABI proof](reference-correction205.md)
document the four spelling-only sidecar changes. All 84 fixtures, required
counts, rejection classes and run outcomes are preserved.

Explicit controls cover allocation replacement, virtual bases, template
inheritance, access/deletion/traits, throwing list elements and destructors,
nonthrowing lists, and cleanup of partially constructed backing arrays. The
trace rebuilds hosted objects from serialized LowIR solely as an inspection
adapter, validates the public representation, executes rebuilt objects, and
compares native text, named code relocations and function-associated CFI. Production still
uses direct typed data throughout. Inspection compares named text sections and raw FDE payloads keyed by function
section/offset, accounting for serialization order and relocation-derived
augmentation displacements without discarding instructions or CFI.
Initial control mistakes (temporary destruction before the full-expression
boundary and assuming vtable-demanded D2 is absent) were corrected; no course
fixture was changed for them. Initial trace checks overconstrained section/FDE
ordering; byte/record comparisons now use the appropriate identity.

Performance records and final command evidence are linked from `plan.md`.
Independent review must still assess the whole stage and these ownership paths;
the implementation handoff does not advance the review marker.
