# PA34 hosted trap reducer

`trap.cpp` reduces the PA2 self-tool build failure in GCC 15 `<array>` to an
unused nested class template member calling `__builtin_trap`. The original seed
rejects its non-dependent type query before any member is instantiated.

The [GNU builtin contract](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html#index-__builtin_trap)
specifies `void __builtin_trap(void)` and abnormal termination, explicitly
allowing a call to `abort`. The PA29 registry now gives it that existing typed
intrinsic behavior. Ordinary expressions and retained template queries share
the same declaration and lowering; probes derive from the same registry.
No header/name-of-library recognition or host compilation delegation is involved.

The checked `pa29/tests/preproc/300-has-builtin.ref` previously expected
`identifier builtin_off`. PA29's Required Implementation Surface requires
`__has_builtin` to answer from the implemented registries, explicitly forbidding
a false negative for an implemented builtin. With trap's contract implemented,
the unchanged fixture's true branch is `identifier bad` (the identifier is just
the fixture author's label). Correct that one output token; preserve the input,
success status, coverage and exact comparison. `trap.cpp` is the reduced proof
that probing and actual calls must agree, independently of compiler agreement.
The observed reference bundle remains pinned to source revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`;
this documented checked-output correction does not alter its manifest.

Run `python3 student.tests/pa29/check_trap.py`: validates probe and noexcept,
both ordinary and instantiated member termination at O0/O3, a returning path,
and rejection of wrong arity. The compiler's existing abort lowering records a
no-unwind/no-return call and ends the block with unreachable.
