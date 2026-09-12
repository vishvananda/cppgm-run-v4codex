void operator delete(void*) noexcept;
// N3485 [expr.delete]/9: leading :: selects global deallocation.
int destroyed, local_free;
struct B { virtual ~B() { ++destroyed; } };
struct D : B {
    ~D() override { destroyed += 10; }
    static void operator delete(void* p) noexcept { ++local_free; ::operator delete(p); }
};
int main() {
    B* p = new D;
    ::delete p;
    if (destroyed != 11 || local_free != 0) return 1;
    p = new D;
    delete p;
    return destroyed != 22 || local_free != 1;
}
