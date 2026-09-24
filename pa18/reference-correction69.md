# Scalar pseudo-destructor receiver exception correction

Pinned bundle: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`; entry
`e09162fa8e25361b824859df63ccba7b7f752b13`. The downloaded bundle is unchanged.
The checked `.ref.exit_status` for
`tests/spec/300-scalar-pseudo-destructor-noexcept.t` changes from success to
failure. Its source and empty informational `.ref` remain unchanged. No test
is removed and no comparison rule changes. Rejection is the required behavior.

The [reducer](../student.tests/pa18/noexcept69_reducer.cpp) removes templates:
`value()` returns `int*` and has no exception specification, then
`static_assert(noexcept(value()->~I()), ...)` requires its evaluation not to throw.
The proof is in the checked-in [N3485](../doc/n3485.txt):

- §5.2.4 [expr.pseudo]/1, line 5808: a scalar pseudo-destructor call evaluates
  its receiver. The fact that destruction itself does nothing does not remove
  the receiver's effects.
- §15.4 [except.spec]/12, line 21715: the declaration of `value` permits all
  exceptions; it is also not constexpr.
- §5.3.7 [expr.unary.noexcept]/3, line 6627: that potentially evaluated call
  makes the enclosing `noexcept` false.
- §7 [dcl.dcl]/4, line 7885: asserting that false value is ill-formed.

The original template's `value<int>()` has the same exception specification,
so the proof applies unchanged after substitution. No compiler agreement is
used as proof. [Observation runner](../student.tests/pa18/reference69.py) and
[observations](../student.tests/pa18/loop69-reference.json) retain both source
hashes and both compilers' outputs. The reference incorrectly accepts both.
Personal positive/negative controls separately preserve scalar pseudo-destruction,
nonthrowing and potentially throwing receivers, and throwing class destructors.
