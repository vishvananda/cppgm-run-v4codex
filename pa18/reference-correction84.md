# PA18 empty-value initialization oracle correction 84

The pinned bundle is unchanged: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`,
SHA-256 `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`
([manifest](../reference-binaries/manifest.tsv)). This local revision corrects
three output oracles, inserting five instructions. Source inputs, success status,
coverage, metadata, existing instructions and comparison rules are unchanged.
[reference84.py](../student.tests/pa18/reference84.py) transforms entry oracles
independently of student output; [the revision manifest](../student.tests/pa18/reference84-revisions.json)
records every old/new hash and recovered destination.

[PA11's assignment boundary](../pa11/README.md#assignment-boundary) explicitly
requires “one `zeroinit` operation for value-initialization that already identifies
an exact contiguous nonvolatile, non-union object or subobject span”. PA18 inherits
that boundary. The three fixtures retain a distinct `obj<1x1>` temporary and its
address for `tag()` or `allocation_tag<int>()`, but omit its required zeroing.
An empty class does not exempt this retained value-initialized span.

[N3485 §8.5 [dcl.init]/6,8](../doc/n3485.txt:11048) requires zero-initialization
(including padding) for the selected non-user-provided default constructor;
§5.2.3 [expr.type.conv]/2 establishes value-initialization for `T()`.
The stored one-byte extent follows §9 [class]/4 and §1.8 [intro.object]/5.
The [LowIR bulk operation](../pa8/lowir.md#bulk-object-memory-operations) zeros
exactly this extent. Empty aggregate `T{}` instead has no member initialization
actions under §8.5.1 [dcl.init.aggr]/2 and does not need an empty helper call.

The reduced ADL case is:

```cpp
namespace n {
struct E {};
template<class T> T apply(E, T v) { return v; }
}
int main() { return apply(n::E(), 3) != 3; }
```

The bundle retains `$argobj__1 : obj<1x1>` and its address but omits `zeroinit`.
The same empty object passed by const reference or constructed by placement new
receives zeroing in that bundle. `object84_controls.py` checks all three forms,
plus aggregate initialization, effects, distinct identities and nontrivial
lifetime boundaries. The observation harness records the actual bundle outputs
and executes both the reducer and every corrected fixture through the supplied
native backend. The proof is the cumulative PA11 rule and C++11 initialization
rules, not agreement with another compiler.

This correction concerns required O0 representation. In the original fixtures,
no later operation observes the empty source's padding: its omission need not
produce a runtime difference under the as-if rule. That does not override PA11's
explicit O0 instruction requirement. A trial implementation omitted that zeroing
at the empty by-value boundary; it broke two unchanged earlier contract fixtures
(PA15 aggregate functional braces and PA17 local static specializations). That
trial was removed. Both earlier fixtures and all their zeroing remain intact.

The class-result convention in the friend fixture is deliberately unchanged by
this revision and remains an unfinished implementation/reference investigation.
