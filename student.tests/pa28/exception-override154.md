# PA28 audit154: dynamic exception restrictions on overrides

The entry compiler (`03575afb`, SHA-256 in the retained observations) accepts:

```cpp
struct B { virtual void f() throw(int); };
struct D : B { void f() throw(double); };
```

This is ill-formed by C++11 [except.spec]/5: the overriding declaration must
allow only exceptions allowed by the base declaration. [except.spec]/8 defines
allowance by handler matching; [except.handle]/3 permits identical types,
unambiguous public class bases, permitted pointer/qualification conversions,
and nullptr matching pointer/member-pointer types. It does not permit scalar
promotion from double to int. The same defect accepted an unrestricted override.
The authoritative rules are in [N3485](../../doc/n3485.txt), lines 21504–21520
and 21641–21676. Host compiler agreement was diagnostic evidence only.

`exception-override154.py` retains these reducers and 31 companion controls:
valid subsets, duplicates, empty/nonthrowing specifications, public/private/
protected/ambiguous/shared paths, pointer cv/void/function/deep qualification,
nullptr, dependent class members, multiple bases, implicit destructor sets,
and an undemanded invalid member body. Entry and final outcomes are preserved
in [entry observations](evidence154/entry-override-controls.json) and
[final controls](evidence154/override-controls.json). No course oracle changes.

The semantic virtual-slot owner now invokes `check_exception_override` once
per canonical overriding/base callable pair within class completion. The new
`semantic/exception_override.cpp` owner uses published exception specifications
and canonical TypeIds. Implicit destructor specifications are inspected through
subobject declaration edges; no destructor body or layout is demanded. Matching
ignores access privileges, as exception handling requires public bases even
inside friends or explicit instantiations. A public alternate path to the same
shared virtual base is accepted; ambiguity is checked separately.

Unrestricted and nonthrowing ordinary base declarations retain cheap exits.
Finite sets are sorted/deduplicated, with binary exact lookup before structural
matching. Remaining O(a*b) comparisons are the language-required relation
between the actual and allowed declared exception types, not unrelated global
entities. Public-path results are memoized by class pair during the comparison;
implicit destructor traversal deduplicates class and function identities.
All traversal storage dies with the check. Class-local callable-pair memoization
dies with class completion. Handler compatibility results have a TU-owned flat
cache keyed by the adjusted exception/handler TypeId pair; all class base edges
are fixed before matching, including for the enclosing class still being defined.
The cache therefore needs no declaration-generation invalidation. Work/hit
counters observe the existing query and trigger no additional analysis.
It never caches a miss across a change to its semantic inputs,
clears another owner's cache, reparses grammar, or requests member bodies.

Valid input keeps the same typed signatures, LSDA, code and relocation path.
Frozen A/B evidence includes a valid 400-specialization class-exception workload
and proves identical object/executable bytes; the new work diagnoses previously
accepted invalid declarations. This is a required semantic check, not an
optional optimizer transform or a new performance exit gate.
