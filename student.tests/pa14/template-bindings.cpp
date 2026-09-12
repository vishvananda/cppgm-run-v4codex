int choose() { return 0; }
struct FixedBase { int choose() { return 5; } };
template<class T> struct DependentBase { int choose() { return 9; } };
template<class B> struct Derived : B {
    int call() { return choose(); }
    struct Nested : FixedBase { int call() { return choose(); } };
};
template<class B> int local_base() {
    struct Local : B { int call() { return choose(); } };
    Local value;
    return value.call();
}
template<class T> struct CompleteContext {
    struct Nested { int read() { return later; } };
    static const int later = 7;
    unsigned bits : 3;
    unsigned read() { return bits; }
};
template<class T> int conditions(T x) {
    if (int n=x) { int saved=n; return saved; }
    { int n=3; if (n!=3) return 2; }
    switch (int n=0) { case 0: { int k=n+1; return k; } default: return 4; }
}
template<class T> int jump() { goto label; { int n=1; } label: return 0; }
int main() {
    Derived<DependentBase<int> > value;
    Derived<DependentBase<int> >::Nested fixed;
    CompleteContext<int>::Nested nested;
    CompleteContext<int> bits; bits.bits=6;
    if (value.call()!=0 || fixed.call()!=5 || local_base<DependentBase<int> >()!=0) return 1;
    if (nested.read()!=7 || bits.read()!=6) return 2;
    return conditions(4)!=4 || conditions(0)!=1 || jump<int>()!=0;
}
