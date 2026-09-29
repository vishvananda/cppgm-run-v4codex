# PA19 constant storage and demand reference revision 92

Entry `065d67835340af14fc3f154f3f314e552d0b3f69`; pinned bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, archive SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
The bundle is unchanged. [reference92.py](../student.tests/pa19/reference92.py)
reconstructs fourteen output revisions from **entry oracles only**, with explicit
source-declaration reconstruction in the structured-bool case. It never reads
student LowIR. The [manifest](../student.tests/pa19/reference92-revisions.json)
records before/after hashes. All 423 source inputs, success/rejection sidecars,
comparison rules and coverage are unchanged. Canonical declaration order is
retained. Independent review of these proofs remains required.

## Automatic scalar arrays (five fixtures)

The five array cases identified in the manifest contain complete constant
initializers of automatic nonvolatile arrays. [PA16's assignment boundary](../pa16/README.md:174)
requires readonly data and one object copy, with separate automatic storage.
The old oracles use individual stores. This is a cumulative contract violation,
not a claim that C++ forbids scalar stores. The transformer extracts all stores
in increasing byte offsets, checks the exact declared extent, checks that removed
address intermediates have no later uses, and replaces only those instructions
with a copy from a readonly image. Every later operation remains untouched.
Two identical images may share data, while destinations remain distinct.

N3485 [dcl.init.aggr]/2,4,7 and [dcl.init.string]/1–2 specify the element values;
[basic.types]/3,9 permits their representation copy, and [intro.object]/6 preserves
object identity. [LowIR copyobj](../pa8/lowir.md#memory-and-addressing) supplies the
required transfer. Reducers `array_identity`, `array_short`, and `array_string`
in [reference_controls92.py](../student.tests/pa19/reference_controls92.py)
check mutation, independent addresses, element values and two readonly copies.
This independently repeats the ownership proof in PA18 revision 83.

## Dormant static definitions (five fixtures)

[N3485 §14.7.1 [temp.inst]/1,2,8,10](../doc/n3485.txt:19603) requires declarations
when a class is completed and forbids implicit instantiation of unnecessary
static member definitions. In the two `npos` fixtures, the member is never used;
its definition becomes a declaration with the same symbol, type and metadata.
In the `sizeof` fixture, the array's declared bound determines both sizeof
operands, whose evaluation is excluded by [expr.sizeof]/1. Its definition becomes
a declaration while `sizes_length` and `observed` keep their required values.

The intermediate-transform fixture only uses `bool_<true>::value` as a constant
expression. [basic.def.odr]/2's constant-value exception requires no object
storage. The structured-bool fixture only uses its template objects and overloads
inside sizeof. Remove its three unnecessary constant definitions, retain the
source declarations of the static reference `from`, `char pick(bool_<false>)`
and `int pick(bool_<true>)`, and retain `main`. The readonly declarations without
out-of-class definitions remain excluded by the existing output convention.
These edits distinguish declaration, constant value, and storage demands.

Reducer `dormant_static` increments a counter in a static initializer: constructing
its owning class must leave that counter zero. `dormant_invalid_static` replaces
the initializer with `T::missing` and must still compile. `demanded_static` needs
the same definition and checks exactly one initialization. `sizeof_static`
checks the array bound without running its effectful initializer. The unchanged
course input need not have observable effects to violate the demand rule.

## Discarded reference result

[N3485 §5 [expr]/11](../doc/n3485.txt:5183) and [expr.static.cast]/6 exclude
lvalue-to-rvalue conversion on the discarded result of a reference-returning
function call. The deleted-return SFINAE oracle calls the right function, then
loads its referent unnecessarily. Remove only that unused load, asserting that
its address comes from the retained call and its result has no use. The original
helper forms a null-derived reference; do not use its native execution as a proof
of defined behavior. `discarded_reference` instead returns a real volatile object,
checks the call's side effect and structurally forbids a referent load. The adjacent
`discarded_id` requires and checks a volatile load. PA18 revision 85 proves the
same distinction independently.

## Constant initialization (two fixtures)

[N3485 §3.6.2 [basic.start.init]/2](../doc/n3485.txt:3841) requires a reference
bound to static storage by a constant expression to be initialized before any
dynamic initialization. In the compound-assignment fixture `_1` binds directly
to `free1`; replace its zero initializer and dynamic assignment by that address.
`constant_reference_order` places an earlier dynamic read before the reference's
source definition and checks the required value and address. The unchanged
operators and their call are still compared. This is the rule of PA18 revision 65.

PA19 explicitly includes constant class-valued member variable-template results
(the variable-template syntax is the course extension to C++11). Their value and
initialization follow the inherited [PA16 constant-object contract](../pa16/README.md:133).
In `static_query_v`, the empty literal object is already represented by its complete
static image. Remove the spurious dynamic helper containing only its address; keep
the global, all source functions, return/copy path and storage identity. N3485
[basic.start.init]/2 and [dcl.constexpr]/9 supply the constant-initialization rule
for the object semantics; C++11 itself does not introduce variable templates.
`constant_member_object` uses a nonempty literal value and an earlier dynamic read,
while variable92's value/address controls check specialization identity. The
empty helper alone is not a runtime failure; it incorrectly represents a static
initialization as a dynamic action under the cumulative constant-object contract.

## Explicit specialization followed by explicit instantiation

[N3485 §14.7.2 [temp.explicit]/5](../doc/n3485.txt:19825) says the later explicit
instantiation has no effect. Remove only its added `object_root=yes`; preserve the
explicit specialization's body, ordinary binding and call.
[LowIR object_root](../pa8/lowir.md:278) denotes a language-required emission root,
including an effective explicit-instantiation definition. A no-effect declaration
cannot introduce one. `specialization_no_effect` checks the selected result and
absence of that metadata. This does not remove the ordinary specialization.

## Validation

Run `python3 student.tests/pa19/reference92.py` to reconstruct all revisions;
run `python3 student.tests/pa19/reference_controls92.py dev/cppgm++ WORK` for thirteen
checked native reducers and structural requirements. Run the unchanged required
stage and through reports to validate every revised oracle and all original
comparisons. Compiler agreement is supporting evidence; the rules and reduced
semantic obligations above are the proof. No reference correction is used for the
remaining sizeof/NTTP widening mismatch: its equivalent O0 forms were reconciled
in the semantic conversion-provenance owner instead.
