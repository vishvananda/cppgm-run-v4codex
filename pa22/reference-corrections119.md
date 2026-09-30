# PA22 reference correction overlay 119

Four `.ref` files are amended by hand with the minimal operations below. The
99 source fixtures, exit statuses, runners and relaxed comparison rules remain
unchanged. This is an oracle correction, not an alternative comparator or a
closed-world specialization of externally callable functions.

Origin: `reference-binaries/manifest.tsv`, source revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle
`cppgm-reference-binaries-linux-x86_64-c2f713cd70d0.tar.gz`, SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
The original compiler hash is
`e32741e2504f7c75a91abdf5cea0d5fc0b56eb43deb503d2ffbddbcc2f854f70`.
The bundle is not replaced or mislabelled; **overlay 119** is the checked-in
fixture revision relative to entry `17603c8a69e76820274b7bf849091d528f6ff261`.
The [reproduction harness](../student.tests/pa22/reference119.py) retrieves the
originals from that commit, records both hashes, validates the corrected outputs,
executes all four original fixture programs using the corrected IR, and exercises
the original and corrected callable bodies with reduced inputs. Its
[results](../student.tests/pa22/reference119.json) preserve commands and outcomes.

## Unknown member receiver adjustment

`general/300-const-member-function-pointer-address-call.ref`: add extraction of
the signed high adjustment word and one byte projection before each indirect
call in `call0` and `call1`. Locals whose zero adjustment was established keep
their original form. Nothing in an external function's parameter declaration
proves that its incoming adjustment is zero.

C++11 [conv.mem]/2 preserves the selected member under base-to-derived
conversion; [expr.static.cast]/12 permits the inverse conversion even when the
base does not itself declare the original member. [expr.mptr.oper]/2–4 permits
application when the object's dynamic type contains that member. See
[N3485](../doc/n3485.txt), lines 5026–5043, 6088–6096 and 6711–6724.
A class with no bases can thus receive a pointer whose member belongs to a
complete derived object and whose receiver displacement is nonzero.

The [callee](../student.tests/pa22/reference119/adjustment-callee.cpp) has the
exact two reference callable bodies; the [caller](../student.tests/pa22/reference119/adjustment-caller.cpp)
passes inverse-converted members of `D : Pad, S` through the `S` subobject.
Those members read `Pad::pad`, initialized to 7; `S::value` is 3. The dynamic
object is a `D`, so this is defined, with required results 7 and 12. Original
reference bodies read the wrong subobject and the check exits 1; corrected
bodies exit 0. The reference compiler emits exactly the original callee IR.
The supplied reference frontend rejects the inverse-conversion caller, so the
harness uses student-generated caller IR with **both** original and corrected
callee bodies. The standard proof, not compiler agreement, establishes validity.
The full source also passes through the student compiler across two TUs.

## Member-pointer truth and typed comparison

`spec/300-member-pointer-parameter-variadic-deduction.ref`: explicitly extract
the low target word before branching. `spec/300-overloaded-member-pointer-function-template-deduction.ref`:
extract that word before `cmp ne i64`. These remain source-sense branches and
comparisons in the same order.

C++11 [conv.bool]/1 converts every null member pointer to false and every
non-null member pointer to true ([N3485](../doc/n3485.txt), lines 5044–5049).
PA22's [Assignment Boundary goal 4](README.md) explicitly requires testing the
target, not treating the adjustment as an independent truth value. The reduced
LowIR caller supplies `1 << 64`: null target, nonzero adjustment. This is an ABI
representation control, not a claim that C++ source can fabricate that value.
The original `mem_fn` branches on all 128 bits (and the supplied O0 backend
rejects that wide branch); corrected IR returns zero.

The [LowIR comparison contract](../pa8/lowir.md), lines 1022–1027, says that a
comparison's type specifies its operands. A loaded `i128` is not an `i64`
operand. The [wide reducer](../student.tests/pa22/reference119/wide-truth.lowir)
exposes the defect without templates. The student validator previously also
accepted it: its compatibility rule for **narrow** integral truth tests omitted
an upper-width bound. That bound is now enforced; the
[narrow control](../student.tests/pa22/reference119/narrow-truth.lowir) still passes.
Explicit truncation both respects target-word truth and makes the comparison
typed. No validation or comparison rule is relaxed.

## Unused static template member

`general/300-structured-bool-conditional-member-pointer-dead-branch.ref`: remove
only the global definition of `bool_<false>::value`. The source never refers to
that member: the conversion returns the template parameter `B`. Both addressed
member functions remain in the output; their potentially evaluated address uses
still require their definitions. The expected inert receiver elision is retained
and implemented separately, rather than changing that expectation to existing
student output.

C++11 [temp.inst]/1 distinguishes class member declarations from definitions;
[temp.inst]/8 states that implicit class instantiation does not instantiate static
data members, and /10 prohibits instantiating members that do not require it.
See [N3485](../doc/n3485.txt), lines 19600–19615 and 19708–19720. The
[reducer](../student.tests/pa22/reference119/unused-static.cpp) has the same
unused declaration/definition and a demanded conversion. The reference emits
the global; the student correctly leaves it undemanded. The personal
`static-member-demand` control takes its address and requires storage, so this
correction does not remove support for real static-member demand.
