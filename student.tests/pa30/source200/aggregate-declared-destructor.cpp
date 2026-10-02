union Leaf { void* pointer; };
union Storage { Leaf value; char bytes[8]; };
// Checking a nondependent declaration establishes destructor validity without
// demanding the unused function body or runtime destruction.
template<class T> void unused() { Leaf value; }
struct Owner { Storage storage; Owner() : storage() {} };
int main() { Owner owner; return owner.storage.value.pointer != 0; }
