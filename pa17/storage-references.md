# Static initialization reference corrections — loop 59

Reference bundle source revision **c2f713cd70d06170632bfde3e75dd6fe1aa44d98**,
bundle SHA-256 `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`,
compiler SHA-256 `e32741e2504f7c75a91abdf5cea0d5fc0b56eb43deb503d2ffbddbcc2f854f70`.
The binary bundle stays pinned. These are local corrections to six checked-in
outputs from entry `7cc89281`; source fixtures, success statuses, 343-case
coverage and comparison rules remain unchanged. The standard proof, rather
than compiler agreement, authorizes the changes.

[Reproducer for the exact oracle edits](../student.tests/pa17/storage_reference_corrections.py)
reads only the frozen original oracles and applies the changes below. It does
not copy compiler output. Run it without arguments to verify the edits.

| Fixture | Incorrect original fact | Correction and rule |
|---|---|---|
| `300-constexpr-static-fn-template-address-pack` | `constexpr table` is zero-filled, then constructed in a dynamic initializer. | Emit its function-address relocation during static initialization. N3485 §3.6.2/2 second bullet requires constant initialization for this constexpr constructor and constant address argument; §5.19/4 makes a function address a constant expression. Preserve the constructor and selected function definitions. |
| `300-rooted-qualified-static-data-member-template-definition` | The namespace reference is zero until a dynamic helper stores its referent address. | Statically bind the reference. Keep the referent's genuinely dynamic constructor. §3.6.2/2 first bullet applies independently of the referent's constructor. |
| `300-namespace-function-template-hides-outer-callable-object` | Same delayed reference binding. | Static address relocation; remove the helper whose only action was that binding. Same rule. |
| `400-alias-nontype-pack-partial-base` | Both base references are bound dynamically. | Static base-address relocations (both layout offsets are zero). §5.19 permits this derived-to-base conversion; §3.6.2/2 first bullet requires static reference initialization. Preserve the empty lifecycle entry already present in this output. |
| `300-function-template-local-static-per-specialization` | An aggregate of two function addresses is dynamically initialized behind a guard. | Static relocations for each distinct specialization, removing only the now-unnecessary initialization guards. §3.6.2/2 third bullet and §6.7/4 require constant initialization before first entering the block. Also zero-initialize the two empty value-initialized arguments, per §8.5/8 and §8.5/6. |
| `300-dependent-hidden-friend-static-member-definition` | An unused static data member is instantiated; a returned value-initialized empty object lacks zero-initialization. | Keep the declaration without instantiating its definition; add the missing zero-initialization. §14.7.1/1,2,8,10 forbids this unused definition instantiation. §8.5/8 requires zero-initialization, including padding under §8.5/6. |

Standard anchors in the checked-in draft:
[§3.6.2](../doc/n3485.txt:3836), [§5.19](../doc/n3485.txt:7170),
[§6.7](../doc/n3485.txt:7715), [§8.5](../doc/n3485.txt:11045),
[§14.7.1](../doc/n3485.txt:19603).

Reduced forms live in [storage controls](../student.tests/pa17/storage_controls.py):
`reference_before_dynamic` observes a reference from an earlier dynamic
initializer, so delaying the binding is observable; `reference_to_dynamic_class`
separates binding from referent construction; `base_reference_offset` checks a
nonzero base offset. `constexpr_function_table` and `local_static_function_table`
reduce the two function-address cases. `unused_effectful_initializer` observes
unwanted initialization, while `unused_invalid_initializer` checks that a
dormant initializer is not instantiated. `empty_value_initialization` inspects
the empty object's byte through unsigned char. Native results supplement the
rules; they are not the proof. Frozen reference observations and a dedicated
reducer script accompany the final evidence.
