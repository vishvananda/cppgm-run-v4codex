# PA23 virtual-base oracle correction 125

This revises 20 checked-in LowIR outputs from bundle source revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
The bundle itself is unchanged. The [manifest](../student.tests/pa23/oracles125.json)
records every source/status/oracle hash, including unchanged fixtures, and the
reason for each revision. All 45 sources, exit statuses, comparison rules and
required coverage remain unchanged. No successful result is accepted solely
because this compiler produces it.

## Nonpolymorphic virtual bases (18 outputs)

The previous outputs omit the object layout pointer, use a fixed offset through
references, and in some member calls pass an extra virtual-base argument with
`this`. The [PA23 contract](README.md#assignment-boundary) explicitly requires
references/pointers to recover virtual bases from the object's table and reserves
parameter hidden addresses for by-value parameters. A virtual base may move
when its containing base is embedded in a different most-derived class; its
address cannot be a constant determined from the reference's static type.

The retained [reducer](../student.tests/pa23/reducers/nonpoly-virtual-reference.cpp)
writes 13 to a sibling field and 7 to the shared base. The reference returns 1
instead of the required 0 because its reference read reaches the sibling field.
[N3485](../doc/n3485.txt) §10.1/4,6 requires a single shared virtual subobject;
§8.5.3/5 and §5.2.5/4 make the bound reference and inherited field designate that
same object. §10.3/1 does not make virtual functions a prerequisite for virtual
inheritance. This is also the previously audited
[observation 123](reference-observation123.md).

Correcting the representation changes sizes/alignment, every dependent base and
field offset, allocation sizes, complete/base constructor signatures, VTT and
construction-table contents, vptr stores and reference/pointer projections.
These 18 whole-program outputs were regenerated together after the lifecycle
and ABI consumers were completed. They retain the source actions and observable
semantics, including declaration-only calls, overloads, member pointers,
constructor conversions/templates, placement allocation and forwarding.
Function names and metadata are still subject to the original relaxed comparator.
The manifest names each affected output; unrelated PA23 outputs are untouched.

Two associated corrections are required rather than copying the old output:

- §12.6.2/7,8,10 assigns shared-base initialization to the most-derived
  constructor, in depth-first left-to-right order; §12.4/8 supplies reverse
  destruction. The [copy reducer](../student.tests/pa23/reducers/shared-copy-base.cpp)
  requires `D::V` to keep 9, not be overwritten by `A`'s base-copy entry.
  The reference rejects its valid indirect virtual-base mem-initializer.
  Construction/base-copy controls also verify the rule without that syntax.
  In `100-constructor-prvalue-virtual-base-forwarding.t`, `W` does not initialize
  its virtual scalar fields; §12.6.2/8 leaves them indeterminate. We preserve the
  fixture and its LowIR comparison, but make no claim that its `main` has a
  defined result of zero. Its ordinary-base copy must not silently initialize
  the most-derived object's virtual bases.
- §5.3.4/13 requires skipped initialization and a null result when a nonthrowing
  allocation returns null. §18.6.1.3/2 says standard placement allocation returns
  its pointer argument. The [standard-placement reducer](../student.tests/pa23/reducers/null-placement-standard.cpp)
  gives the null pointer to that function. The reference returns 1 because it
  runs the constructor; our output returns 0. Validation links an address-taken
  standard-library placement function from `<new>`; it does not replace the
  reserved library function. The placement fixture's corrected output includes
  the required null guard and the corrected object size. The class-specific
  placement control passes with both compilers and distinguishes this defect
  from the general allocation path. These citations use the supplied C++11
  draft, including its explicit null-allocation rule.

## Construction RTTI and offset-to-top (two outputs)

`100-virtual-base-constructor-vptr-hidden-target.ref` changes only the secondary
construction segment's RTTI pointer from `V` to the constructor's class `B`.
`200-multi-level-virtual-base-diamond-lifecycle.ref` changes only construction
RTTI/offset-to-top fields and `AB`'s VMI diamond flag, from 0 to 2. Its functions,
VTT order, slot order, lifecycle actions and cleanup order are unchanged.

N3485 §12.7/4,5,6 makes virtual dispatch, `typeid`, and `dynamic_cast` during
construction/destruction observe the class whose constructor/destructor runs.
[Itanium ABI §2.6.4](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#vtable-ctor)
requires construction offsets to locate the complete object's actual virtual
bases, but offset-to-top and RTTI to identify the active base class. Therefore a
`B` segment at complete offset 8 has top offset 0; its `V` view at complete
offset 16 has top offset -8 and `B` RTTI. All `AB` construction views identify
`AB`, including the secondary `B` and shared `V` views. The
[construction reducer](../student.tests/pa23/reducers/construction-view-rtti.cpp)
checks both `typeid(V&) == typeid(A)` and `dynamic_cast<void*>(V*) == this`
inside a secondary `A` constructor. Reference execution returns 2; corrected
execution returns 0.

`AB : A, B`, where both `A` and `B` virtually inherit `V`, has one shared `V`
subobject by §10.1/4. [Itanium ABI §2.9.5](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#rtti-layout)
assigns flag 0x02 to this diamond. The earlier nonvirtual-diamond correction 122
remains unchanged; these are different inheritance properties.

[The explicit observer](../student.tests/pa23/reference125.py) saves commands,
reducer sources, LowIR and execution results in
[reference125.json](../student.tests/pa23/reference125.json). It never writes an
oracle. The corrections remain subject to independent whole-stage audit.
