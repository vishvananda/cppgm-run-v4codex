# PA13 reference correction

The affected bundle is `cppgm-reference-binaries-linux-x86_64-c2f713cd70d0.tar.gz`,
source revision `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`
([manifest](../reference-binaries/manifest.tsv)). The bundle itself is unchanged.

`400-explicit-virtual-destructor-call-nonvirtual.ref` lowered
`this->~Derived()` inside `Derived::destroy` as a direct D1 call. Explicitly
spelling a destructor does not suppress virtual dispatch. C++11
[12.4/13, class.dtor](https://timsong-cpp.github.io/cppwp/n3337/class.dtor#13)
applies the usual member-function rules and gives both `B_ptr->~B()` (calls D)
and `D_object.B::~B()` (calls B) as examples. The distinction is the explicit
base qualification. [10.3, class.virtual](https://timsong-cpp.github.io/cppwp/n3337/class.virtual)
requires the final overrider; a further-derived object can invoke the inherited
`destroy` body. Its `this` expression does not prove an exact dynamic type.

The defined [reducer](../student.tests/pa13/audit-explicit-destruction.cpp)
allocates objects, calls the unqualified destructor through a base pointer,
and releases storage separately. Destructor side effects require the trace
21, while an explicitly qualified control requires 1. The entry compiler
returns failure; the corrected compiler produces both traces. The extended
case also calls an inherited `destroy` body on a further-derived object.
Compiler agreement is not the proof: the cited language rules determine both
outcomes. The course fixture's automatic object is destroyed twice, so its
`main` is not used as an executable oracle for this correction.

Exactly one call site is corrected: load the vpointer, load the first
(complete-destructor) slot, and call it indirectly with its existing pointer
extent and nonthrowing contract. [LowIR's vtable contract](../pa8/lowir.md)
places D1 before D0. No allocation, deallocation, vtable contents, other
instructions, source fixture, status sidecar, comparison rule or coverage is
changed. The historical fixture filename is retained. This is a manual local
reference edit, not replacement with compiler output.
