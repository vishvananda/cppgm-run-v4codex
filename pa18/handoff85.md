# PA18 implementation handoff 85

Entry `f953e42a875bc8f7e3b9882a324db843f0c4191a`: **414/420**, six failures.
This handoff completes discarded-expression demand and validates dormant static
storage publication. It is an implementation handoff, not a whole-stage audit.
The stage-base and last-reviewed markers remain unchanged in [the plan](plan.md).
Final validation and performance evidence are linked there.

## Ownership, data flow and bounds

| Owner | Data flow | Complexity / lifetime | Validation |
|---|---|---|---|
| Semantic expressions and `ExpressionStore` | Parsed built-in form + selected overload + child facts → immutable `discarded_form` bit | O(1) per expression; no syntax rescan or new per-node allocation. `Expression` stays 36 bytes; the packed property fits its existing flag byte | Volatile/nonvolatile lvalues, xvalues, names, calls, member access, built-in and overloaded subscripts, comma and conditional arms |
| `semantic/discarded.cpp`, type-query and conversion owners | Canonical query/context → checked volatile copy recipe or compact failure; source-fixed recipe → concrete temporary, selected constructor, destructor | Expected O(1) NodeId fact lookup; each contextual use owns at most one materialization. Ordinary indexed candidate selection applies. TU-owned recipes and facts are released with the analyzer | Access/deletion/explicit-copy rejection, SFINAE dropping, no eager body demand, fixed/dependent templates, functional and C-style void casts |
| Lowering discard and full-expression lifetime owners | Recorded form/category and conversion → required scalar read or ordinary copy/materialization + cleanup | O(1) classification instead of a second recursive syntax cache; existing linear lowering/lifetime work. No optimizer pass or code cloning | Required volatile access counts, branch suppression, copied-object effects, destruction at full-expression end, checked native execution and cleanup edges |
| Existing template static-storage owner | Declaration/layout demand stays distinct from initializer/storage demand | Existing indexed definition applications and demand queue, unchanged | Dormant invalid/effectful constants and objects; used, explicit, late, namespace, reference and transitive counterparts |

The entry compiler treated parsed subscripts as built-in even after overload
resolution. A conditional with one overloaded-subscript arm could therefore read
its other volatile arm. It also read volatile xvalue members and omitted reads
through built-in data-member pointers. The new semantic bit records the relevant
built-in source form after selection; lowering combines it with the lvalue
category. Ordinary inherited member pointers are covered; no new member-pointer
template surface is introduced.

The same rule requires a temporary copy when the discarded lvalue is a volatile
class. That copy was previously omitted. The implementation now checks the
selected constructor and destructor, retains an unevaluated recipe without body
demand, and materializes exactly one temporary for an evaluated use. A separate
NodeId index keeps this conversion distinct from a cast or comma's incoming
conversion. Existing temporary activation and cleanup paths handle its lifetime.
Fixed template uses reuse source selection; dependent queries return candidate
failure for unavailable/deleted/inaccessible or explicit-only copies.

The first performance run exposed a missed source-recipe publication: 600 fixed
uses repeated copy selection 600 times. Commit `076eccdd` repairs the source cast
and statement paths and publishes the source-form bit consistently on fixed
member/operator facts. Fifteen scaling checks cover five source forms at 1, 37
and 600 uses, requiring exactly one selection, N recipe uses and N distinct
materializations. The initial measurements remain preserved alongside the final
run; the repeated semantic work is fixed, not accepted as a timing allowance.

Connected query validation also found that `void(expr)` in a dependent result
rejected valid discarded operands. Its functional-notation query now uses the
same explicit void conversion and checks as the ordinary cast, including empty
`void()` and scalar controls. No query materializes runtime storage.

## Reference corrections

[The proof](reference-correction85.md) applies N3485 [temp.inst]/1,2,8,10 to the two
unused static definitions, and [expr]/11 with [expr.static.cast]/6 to the discarded
reference-call load. The independent transformer reads entry oracles only and
preserves every other byte. The pinned bundle is unchanged; before/after hashes,
reducers and actual bundle observations accompany the revision.

The bundle executes an unwanted dormant initializer (exit 1), rejects a dormant
invalid initializer and emits one forbidden volatile read after a discarded
reference-returning call. The defined student reducers instead keep the required
call, omit that read and preserve the adjacent required volatile-name read. The
course null-reference reducer is not used as evidence of defined C++ execution.
All course source inputs, status sidecars, fixture paths and comparison rules
remain unchanged. Three entry mismatches are resolved, with no new mismatch.

## Handoff boundary and independent review

**Unfinished implementation:** the three remaining course mismatches concern
class-result ABI conventions: friend alias results, conversion-template object
results and dependent defaulted result types. The inherited class-ellipsis reducer
also remains unfinished at LowIR's scalar-only variadic boundary.

The frozen bundle's reduced ordinary/alias/constructor/float results use direct
object returns, while conversion-template results can be indirect. No uniform
canonical type rule has yet justified changing all calls, definitions and indirect
signatures. Completing that group requires its own coherent boundary model; the
new discard/demand facts neither choose nor repair it. Substituting declaration-
timing or template-spelling heuristics would violate the spec's identity and
lowering requirements. The scalar variadic contract similarly needs a separate
representation decision. These are concrete implementation boundaries, not waived
requirements or questions relabeled as audit work. Further edits to the completed
discard/static owners cannot resolve them.

**Independent review:** verify source-form fidelity through fixed and dependent
queries, compact failure handling, complete context keys, single copy selection
and materialization, default-argument and normal/exceptional cleanup edges, and
both reference proofs. Source `throw`/`catch` execution belongs to PA21; PA18
inspects the emitted cleanup relationships and executes successful lifetimes.
The accumulated audit 82 is still the last review. Handoffs 83–85 remain unaudited;
this third implementation handoff preserves the scheduled review boundary.
Passing controls and performance measurements do not replace that review.
