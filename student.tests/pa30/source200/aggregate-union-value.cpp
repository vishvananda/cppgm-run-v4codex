union Leaf { void* pointer; };
union Storage { Leaf value; char bytes[8]; };
static_assert(noexcept(((Leaf*)0)->~Leaf()), "declaration query");
struct Owner { Storage storage; Owner() : storage() {} };
int main() { Owner owner; return owner.storage.value.pointer != 0; }
