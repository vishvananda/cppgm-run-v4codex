# Repeated nonvirtual base RTTI flag correction

The sole changed oracle field is the first `i32` of `D`'s VMI RTTI in
`tests/general/100-diamond-virtual-destructor-slot-merge.ref`: **0 → 1**.
The source, slots, lifecycle bodies, exit status, fixture count and comparison
rules are unchanged. The pinned observed reference bundle remains
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98` (manifest source revision); its
source-to-LowIR implementation emits the incorrect zero flag. This checked-in
oracle correction is revision 122 of that observation, not a bundle replacement.

The reduced source is
[repeated-destructor-base.cpp](../student.tests/pa23/reducers/repeated-destructor-base.cpp).
N3485 §10.1 [class.mi]/4, in [the supplied standard](../doc/n3485.txt), requires a
distinct subobject for each occurrence of a nonvirtual base. `D` therefore has
two distinct `B0` subobjects, one within `B1` and one within `B2`. A virtual
**destructor** does not make a base-specifier virtual. N3485 §5.2.7
[expr.dynamic.cast]/8 requires each public `B0*` to recover its enclosing `D*`.

[Itanium ABI §2.9.5](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#rtti-layout)
assigns VMI flag bit `0x01` to repeated nonvirtual inheritance, including indirect
bases; bit `0x02` instead describes a shared virtual diamond. The source has the
first property and not the second. Consequently the required descriptor flags
are `1`. This is a language-subobject and ABI-layout proof, independent of
compiler agreement. The unchanged LowIR comparator still checks the exact flag.

The reducer is run explicitly by `student.tests/pa23/verify122.py`, including
checked execution through the supplied backend. Reference and student IR flag
observations are recorded in its JSON output. Independent review of this proof
and the implementation remains pending.
