typedef decltype(sizeof(0)) size_t;
int allocated, freed, destroyed, placement, initialized;
struct Tag {};
void *operator new(size_t, Tag) noexcept { ++placement; return 0; }
void *operator new[](size_t, Tag) noexcept { ++placement; return 0; }
int init() { ++initialized; return 7; }
struct S {
    int n;
    S() : n(init()) {}
    ~S() { ++destroyed; }
    static void *operator new(size_t n) { ++allocated; return ::operator new(n); }
    static void operator delete(void *p) noexcept { ++freed; ::operator delete(p); }
    static void operator delete(void *, void *) noexcept { freed = -20; }
};
struct Pointer { S *p; operator S*() const { return p; } };
int main() {
    S *p = new S;
    Pointer q = {p}; delete q;
    if (allocated != 1 || freed != 1 || destroyed != 1 || initialized != 1) return 1;
    const S *r = ::new S(); ::delete r;
    if (allocated != 1 || freed != 1 || destroyed != 2 || initialized != 2) return 2;
    int *scalar = new (Tag()) int(init());
    S *array = ::new (Tag()) S[9];
    if (scalar || array || initialized != 2 || placement != 2) return 3;
    S *nothing = 0; delete nothing; delete[] nothing;
    return destroyed != 2 || freed != 1;
}
