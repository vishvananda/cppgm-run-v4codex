# Virtual-reference offset defect — observation 123

No contract fixture, expected status, comparison rule or oracle is changed in
this handoff. This observation identifies a correction needed alongside the
remaining virtual-base lifecycle implementation.

The deterministic [reducer](../student.tests/pa23/reducers/nonpoly-virtual-reference.cpp)
has one shared `V` subobject, reached through `B` and `C`. It writes `13` to
`C::guard`, writes `7` to `V::value`, then reads `value` through a `B&`. Its
required source result is zero. The pinned reference bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98` produces a program returning **1**.
The [explicit observer](../student.tests/pa23/observe_reference123.py) records
commands, source/LowIR hashes and execution in
[reference-observation123.json](../student.tests/pa23/reference-observation123.json).
Our same source executes successfully in the layout controls.

This is a language proof, independent of compiler agreement:

- [N3485](../doc/n3485.txt) §10.1 [class.mi]/4 and /6 require one `V`
  subobject shared by both paths. `B` and `C` do not own separate `V` objects.
- §8.5.3 [dcl.init.ref]/5 binds the `B&` parameter to `d`'s `B` subobject.
  §5.2.5 [expr.ref]/4 makes `b.value` designate that object's named inherited
  member. It therefore denotes the same `V::value` initialized to `7`.
- §10.3 [class.virtual]/1 defines polymorphism in terms of virtual functions.
  The absence of virtual functions does not remove the shared-base requirement.
- The PA23 README's Assignment Boundary explicitly requires reference/pointer
  access through the object's table, with no hidden virtual-base argument.

The reference places `B` at offset 0, `C::guard` at offset 4 and the shared
`V::value` at offset 8. Nevertheless `read(B&)` loads offset 4. It deterministically
reads the sibling guard (`13`). This also explains the fixed `+4` read in
`100-virtual-base-parameter-hidden-layout.ref` despite that fixture's complete
object placing its shared base at `+8`.

The new semantic/layout owner gives classes with virtual bases a layout pointer
while retaining the separate C++ polymorphism fact. Its dynamic projections
therefore work for both complete objects and adjusted references. Full oracle
reconciliation is still unfinished: constructor/base-entry signatures and
construction tables must be completed before corrected full-program outputs
can be reviewed. Current failures and all original comparisons remain visible.
