void *operator new(unsigned long, void *p) noexcept { return p; }
int destroyed;
struct Other {};
using Alias = Other;
struct S {
    using Nested = S;
    int n;
    S() : n(7) {}
    ~S() { if(n!=7)__builtin_abort(); ++destroyed; }
};
int run(int n) {
    int before=destroyed;
    using Alias=S;
    S s;
    s.~Alias();
    new (&s) S();
    s.~Nested();
    new (&s) S();
    if(destroyed!=before+2)__builtin_abort();
    return n+1;
}
int main() { return run(7)!=8 || destroyed!=3; }
