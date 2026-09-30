# PA28 loop152 implementation ownership

## Exception lifetime and specification group

Semantic owner: `ExceptionSpecificationFact` keyed by canonical callable EntityId
and declaration occurrence, with the existing specialization/environment fact key.
The retained parsed type-id list feeds adjusted canonical TypeIds once on demand;
ordinary function-template substitution uses its immutable substitution frame.
Class-template members consume projected typed declarations. Nondependent nodes
are shared, with no grammar replay. Allowed types are a sorted unique arena slice;
compatibility compares sets without rendered names. State remains
NotStarted/Active/Success/Failure; unavailable prerequisites remain retryable at
the existing owner. Work is O(k log k) for k declared types, not visible entities.
RTTI demand is recorded through the existing type worklist before lowering.

Lowering owner: active `ExceptionContext` plus persistent live-object prefix.
Handler-only expression continuations are cleanup registrations: they explicitly
retire their expression and catch regions. Catch exit retains its initial outer
live prefix. Shared suffix identity and cached terminals are unchanged. No
whole-function rescanning was added to cleanup construction.

The existing single function-boundary pass consumes the published allowed set.
For host `throw(T...)` it emits an existing typed LowIR `EhFilter` clause and
calls `__cxa_call_unexpected` only on that filter's selector. Same-frame cleanup
resumes can carry permitted exceptions to this boundary, so the selector's miss
edge resumes out. Host `throw()` uses an empty filter (unexpected), whereas
noexcept keeps its terminate boundary. Function parameters retain cleanup at a
dynamic boundary. The private course runtime has no hosted unexpected API and
retains its previous exception-specification boundary; PA28 requires host -c.

Native owner: per-function clause/filter slices, persistent region stacks and
signed host selectors. Filters serialize as zero-terminated ULEB type indices
after the LSDA type-table base; negative actions identify those lists. Positive
catches, cleanups and ancestor action suffixes preserve their existing structure.
Selection/encoding remain linear in consumed clauses and emitted table bytes;
no executable optimization or speculative code growth is introduced. MIR dumps
show the actual filtered type identities and signed LSDA selector. Function
vectors are released after encoding. ELF writes typed relocations directly.

ABI evidence: [Itanium exception ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi-eh.html)
§2.5.3 and [GNU host personality](https://github.com/gcc-mirror/gcc/blob/releases/gcc-12.2.0/libstdc%2B%2B-v3/libsupc%2B%2B/eh_personality.cc)
`check_exception_spec` document the runtime boundary and negative list offset.
No runtime implementation was copied or used to produce compiler output.
C++11 rules: checked-in N3485 [except.spec]/2–9 and [except.unexpected]/1–4.

Validation: all three original PA28 EH failures pass (94/97 overall at this
increment). `exceptions152.py` executes 18 explicit controls covering permitted
scalar/base types, converted unexpected values, ordered local cleanup, nested
rethrow, templates, empty specifications, private cleanup, and invalid type/set
rejections. Two earlier LowIR fixture corrections have independent contract
proof in `reference-corrections152.md`. Performance and full final checks remain
handoff work, separate from implementation correctness checks.

## Imported support ownership and local-static demand

A key-function declaration leaves the vtable and VTT owned by its defining TU.
Host lowering now declares both; it does not define an imported VTT referencing
private auxiliary views. Semantic construction-table dependencies are established
only when the exact table owner transitions to Active definition demand. Its
existing reverse key-definition notification preserves a later local definition.
No unrelated class/member is retried, and imported references do less work.

`constructor_needed` can answer positively from a completed dynamic-class fact:
vptr initialization is necessary. It does not need to drain deferred synthetic
constructor bodies/actions to prove that positive fact. Failure states and the
full action plan remain mandatory for emission and omission decisions. This
fixes local-static initialization of recursive CRTP hierarchies without eagerly
instantiating their unrelated bodies. The RTTI base facts were already correct;
the entry failure was an early constructor-action prerequisite query, not stale
RTTI. Normal and array local statics retain their existing guard/destruction flow.

`ownership152.py` runs nine commands: host-owned virtual-base constructors and
D1 destruction, imported table/VTT symbol ownership, and two compiler-generated
TUs with ordinary/array recursive-template statics and host RTTI cross-casts.
Required stage tests reach 96/97. Earlier PAs remain passing.

## Concrete remaining boundary

The remaining covariant fixture requires a virtual-result row at -32 rather than
-24. The current class model selects only nonvirtual dynamic primary bases;
virtual base allocation always appends their storage. It also represents the
primary table prefix as a contiguous virtual-base list, with vcall rows limited
to secondary views. It therefore lacks nearly-empty virtual primary sharing and
the inherited mixed vcall/vbase prefix needed by Itanium ABI §2.4 and §2.5.3
Category 4. Merely renaming the thunk or inserting a padding word would conceal
that physical-layout mismatch. A coherent fix must change primary selection,
shared virtual-base allocation, prefix identities, base/result projections,
constructor/VTT segments and RTTI together, with mixed host producer/consumer
proofs. This is a distinct layout representation change, beyond the completed
EH and imported-support/demand owners. No partial layout workaround is included.
It remains unfinished implementation, not an independent-review question.
