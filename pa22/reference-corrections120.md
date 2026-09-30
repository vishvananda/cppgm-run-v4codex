# PA22 reference correction overlay 120

The independently reproduced layout defect also appears in
`tests/general/100-public-qualified-base-typedef-ambiguous-subobject.ref`.
The source has two distinct `impl` base subobjects, through `inner` and
`trampoline<int>`. Both were placed at offset zero in one byte. The corrected
layout retains the first at zero and places the second at one, reserves two
bytes, and updates `outer::color`'s receiver extent. These are the only three
changed lines. The source, required success status, instructions, call order,
99-case inventory and relaxed comparator are unchanged.

C++11 [intro.object]/6 requires distinct addresses for same-type subobjects
when neither contains the other. The exception for zero-size base subobjects
requires **different types**. [expr.eq]/2 compares object pointers by address.
The checked primary text is [N3485](../doc/n3485.txt), lines 1029–1033 and
6932–6934. PA22 [goal 1](README.md) requires distinct base subobjects with
correct deterministic offsets. A one-byte layout with both `impl` objects at
zero cannot represent those identities. The fix belongs to class layout and
its selected receiver fact, even though the original fixture's constant-return
method does not itself inspect addresses. It is not a fixture-specific storage
optimization or a relaxation of the LowIR shape requirement.

The [reducer harness](../student.tests/pa22/reference120.py) removes the aliases
and method from the same class graph and compares the two `impl*` values.
The required result is unequal. The original supplied compiler and frozen
student entry return 1 (wrong equality); the corrected student returns 0.
The original and corrected fixture IR both validate and execute successfully.
[Evidence](../student.tests/pa22/reference120.json) retains exact source, commands,
binary hashes and both fixture hashes. Compiler agreement is only reproduction;
the object-identity rule and assignment layout contract establish the correction.

Origin is the unchanged `reference-binaries/manifest.tsv`: source revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle
`cppgm-reference-binaries-linux-x86_64-c2f713cd70d0.tar.gz`, SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Overlay 120 is relative to `17bc7a06`, not a replacement/mislabelled bundle.
Overlay 119's four corrections and all their reducers remain preserved and
were executed again against this implementation.

Original fixture SHA-256: `feb47f616bcde7fb9f4dc8a1174d926c34935177587b93282ebd9e1669ca6e27`.
Corrected fixture SHA-256: `704148b6ad2b41c0004700b012203268e6f12f76815419fbfd35a9eb0d42775a`.
