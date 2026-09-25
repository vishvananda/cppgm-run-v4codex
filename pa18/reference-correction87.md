# PA18 canonical class-result boundary correction 87

Pinned bundle: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, archive SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
The bundle is unchanged. [reference87.py](../student.tests/pa18/reference87.py)
reconstructs three revised output oracles **from entry commit `01b47c20` only**;
it never reads student LowIR. [The revision manifest](../student.tests/pa18/reference87-revisions.json)
records original/revised hashes. Inputs, expected success, all other fixtures,
metadata, evaluation order and comparison rules remain unchanged.
[reference87_chain.py](../student.tests/pa18/reference87_chain.py) executes the
historical proof 84 unchanged in a minimal snapshot of the entry references,
then checks its exact composition with this repair. Thus the shared friend
oracle retains all earlier required empty-tag zeroing.

## Defect and reducer

The bundle selects different result boundaries for the same completed function
type, depending on the dependent spelling used before substitution. In the
following reducer, its definition of `make<int>` takes a hidden `%ret` pointer,
but its indirect call to **that same definition** takes just the integer and
expects `obj<4x4>`. The resulting executable faults (signal 11).

```cpp
template<class T> struct Id { typedef T type; };
template<class T> struct A { int n; A(int x) : n(x) {} };
template<class T> typename Id<A<T>>::type make(int x) { return A<T>(x); }
int main() {
  A<int> (*p)(int) = make<int>;
  A<int> a = p(7);
  return a.n != 7;
}
```

Changing only the result spelling to `A<T>` makes both boundaries direct and
execution succeeds. The original friend-alias and defaulted-result fixtures
expose exactly this distinction: replacing only the dependent aliases gives
direct returns; accessing the original specialization through its exact pointer
type gives wrong behavior (friend: exit 1; defaulted-result: signal 11).
The reduced pointer uses preserve deduction, access and the specialized body;
no cast to an incompatible function type is involved.

The third fixture is the conversion-function-template instance returning `A`.
The bundle gives `operator T`/`T=A` an indirect result, whereas a conversion
template returning `A<T>` and the ordinary conversion returning `A` have direct
results for the same small trivial class boundary. It also rejects the valid
explicit call `x.operator A()` to the first instance. The student's explicit
and implicit calls consume the same canonical completed result. The conversion
observations are preserved separately: **the unextended original direct-call
fixtures execute correctly in the bundle**. Their indirect representation by
itself is not a C++ runtime error. The corrected owner is its inconsistent
type-based result classification, evidenced by the pointer reducer, not the
claim that C++ mandates one particular IR spelling for every direct call.

[result87_reduce.py](../student.tests/pa18/result87_reduce.py) records the full
matrix, including unsuccessful explicit/member-pointer observations rather than
presenting them as runtime proofs. [boundary87_observe.py](../student.tests/pa18/boundary87_observe.py)
records aggregate/constructor/access, integer/floating and direct/indirect
controls. These are observations of the executable bundle, not its source.

## Rules and contract proof

* N3485 §7.1.3 [dcl.typedef]/1–2 ([text](../doc/n3485.txt:8103)):
  typedefs and alias declarations denote their associated type; no distinct
  class or function type is created by the alias spelling.
* N3485 §14.5.7 [temp.alias]/2 and §14.8.2 [temp.deduct]: substitution into an
  alias/function template establishes the resulting type. In the reducer the
  result is `A<int>` and the function type is `A<int>(int)`.
* N3485 §5.2.2 [expr.call]/1,4,10 ([text](../doc/n3485.txt:5683)), §4.3 [conv.func]
  and §8.3.5 [dcl.fct]: a call through the exact function pointer type invokes
  the function and obtains its declared result. Merely taking its address
  cannot change the argument positions or result meaning. The wrong-type
  exception in §5.2.10 [expr.reinterpret.cast]/6 does not apply here.
* N3485 §12.3.2 [class.conv.fct]/1 ([text](../doc/n3485.txt:14196)) defines the
  conversion member as a function returning its conversion-type-id. §14.8.2.3
  [temp.deduct.conv] substitutes the selected `T=A`; `A` is the same canonical
  class as in an explicit conversion call or ordinary return declaration.
* N3485 §12.8 [class.copy]/7,9,12 ([text](../doc/n3485.txt:15096)) and §12.4
  [class.dtor]: these result classes have implicit trivial transfers and trivial
  destruction. An ordinary or templated converting constructor does not turn
  them into classes with a nontrivial copy/move or destructor. Their extents
  are 4×4 (`owner<int>`, `owner<long>`, `A`) and 8×8 (`box<double>`).
* [LowIR object ABI conventions](../pa8/lowir.md#object-abi-conventions), direct
  object values and indirect-call signatures require coherent call/definition
  boundaries. Small complete object results have the semantic `obj<...>`
  representation already used by the bundle's concrete signature path.
  [Bulk object copying](../pa8/lowir.md#bulk-object-memory-operations) transfers
  the returned value into the destination without changing source semantics.

The [Itanium ABI type rule](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#non-trivial)
also classifies call triviality from class properties, with trivial types using
the platform ABI; dependent result spelling is not a class property. This is
supporting design evidence, **not an added PA18 native ABI exit requirement**.

## Independent reconstruction

Four affected function definitions in three oracles are made consistent with
their canonical direct result types. Each former `%ret` becomes a local
`obj<extent>` plus its address; all existing construction/copy instructions use
that address, then return the object. Each direct caller receives that object
and copies it to its original destination. Parameter spills, access-sensitive
constructor calls, source effects and field computations are unchanged. Prior
empty-tag zeroing correction 84 is retained byte-for-byte.

Both representations are legal for an isolated direct call. The chosen repair
retains the existing canonical signature classification rather than propagating
the bundle's dependent-spelling exception into unrelated indirect signatures.
There is no new comparator tolerance, removed operation or reduced coverage.
Validation executes every revised oracle and the correct student reducers and
checks the bundle's reported counterexamples. Independent full-stage review
must still assess this proof and bundle revision; this implementation handoff
does not waive that review.
