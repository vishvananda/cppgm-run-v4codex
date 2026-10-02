# Even-stride pointer-loop correction (implementation 217)

The input `tests/o1/500-effect-free-loop-deleted-twin-backward.t` is unchanged.
Its former reference returned `%begin` unconditionally, although its loop does
not terminate for independently supplied pointers with different residues mod 8.

The reduced reproducer and executable checks are in
[`pointer_congruence.py`](../student.tests/pa32/pointer_congruence.py). Removing
the redundant twin leaves `p = phi(end, next); next = index i8 p, -8`, with an
exit only when `p == begin`. Choose `begin = addr(array)+1` and
`end = addr(array)+16`. There are no dereferences, calls or object accesses in
the loop. After k iterations, `p = end - 8*k`; its residue mod 8 is invariant
even across machine-address wraparound. It can never equal `begin`. Equal
pointers and same-residue finite walks are checked separately.

The proof uses the LowIR contract, not source-C++ forward-progress assumptions:

- [Memory and Addressing](../pa8/lowir.md#memory-and-addressing) defines `index`
  as advancement by the element offset. Plain `index` supplies no stronger
  optimization claim: the fixture has neither `object_bytes` nor a projection
  or alignment fact establishing equal residues.
- [Control-Flow Value Merges](../pa8/lowir.md#control-flow-value-merges) and the branch definitions in
  the LowIR specification select the next block from the predecessor value and
  comparison. Nothing in this IR contract licenses assuming loop termination.
- [PA32 Output Format](README.md#output-format) requires behavior preservation
  for every defined LowIR input. Replacing this loop by an unconditional return
  changes its control flow for the valid pointer inputs above. A C++ rule about
  effect-free source loops is not an additional LowIR promise.

The corrected `.ref` preserves the original loop. The corrected envelope is
the eight-instruction/four-block conservative body plus the usual 10% + one
tolerance (9 instructions, 5 blocks, 3 phis). The erroneous `none(phi)` outcome
is replaced by `has(branch)`, while calls, loads and stores are now prohibited
instead of allowing one each. Equivalent smaller conditional implementations
are allowed. The fixture, success status, structural validator, object-lowering
floor and comparison machinery are unchanged. This restores the missing
nontermination distinction; it does not excuse missing optimization of proved
finite walks, whose other fixtures and personal tests are unchanged.

The affected pinned bundle is source revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`, and
`lowiropt` SHA-256
`6b0a8da0990c1f33c23c3e47553c12bd18b76799235b31add771b34dd2ae59b5`.
This checked-in sidecar correction supersedes that bundle's optimization result
for this case; the bundle is unchanged. Compiler agreement is not the proof.
Timed execution observations supplement the algebraic invariant; they alone
would not prove nontermination. All validation observations are retained with
the implementation-217 evidence.
