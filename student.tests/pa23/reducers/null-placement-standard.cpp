typedef decltype(sizeof(0)) size_type;
void* operator new(size_type, void*) noexcept;
int calls;
struct A {};
struct B : virtual A { B() {} B(B&) { ++calls; } };
int main() { B b; B* p = new ((void*)0) B(b); return p != 0 || calls != 0; }
