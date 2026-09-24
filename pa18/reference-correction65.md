# Constant reference initialization correction

Bundle: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`; checked output at entry
`52d972234f7f4dfdb9ec4c0969f559b9521bdf62`.
Fixture: `tests/spec/500-conversion-function-template-reference-conditional-auto-ref`.
Only its `.ref` changes. Source, success status, coverage and relaxed comparison
rules remain intact. The downloaded bundle is unchanged.

The source's constexpr conversion returns a reference to
`static_storage<wrapper<callable_tag>>::value`, which has static storage duration
and the constant value `{11}`. The true conditional arm determines the same
lvalue. N3485 5.19 [expr.const] permits these constexpr calls and the lvalue
constant expression; 7.1.5 [dcl.constexpr] requires the declared constexpr
reference initializer to be constant. N3485 3.6.2 [basic.start.init]/2
(`doc/n3485.txt:3844`) mandates constant initialization of such a static
reference before any dynamic initialization. This is not optional folding.

The pinned output initializes `selected` to zero and installs its address in
`__cppgm_init`. The [reducer](../student.tests/pa18/constant_reference_reducer.cpp)
places a read in an earlier dynamic initializer. Its required result is 11;
the pinned reference's generated executable fails, while the student's exits
zero. [Recorded source, LowIR and native results](../student.tests/pa18/loop65-reference.json)
come from [the explicit runner](../student.tests/pa18/reference65.py), which
uses the supplied backend only to execute the two separately produced modules.
Compiler agreement is not the proof: the mandatory initialization ordering is.

The corrected output contains an address initializer for `selected`. Its known
reference use names the same backing object, and the obsolete dynamic
initializer is removed. Other functions and aliases are retained. The ordinary
course validator/comparator and original program execution validate the result.

Separate language-version observation: personal pointer-qualification controls
follow N3485 14.8.2.3 [temp.deduct.conv]/7, including its unqualified terminal
deduction example. Modern host compilers differ because
[CWG 2384](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1359r0.html#2384)
later removed that paragraph. This observation changes no course reference.
