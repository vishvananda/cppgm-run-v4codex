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
template<class T> struct Outside { struct Inner; static int read(); static const int value=8; };
template<class U> struct Outside<U>::Inner { int read(){return value;} };
template<class V> int Outside<V>::read() { return value; }
int nested_specializations();
int main() {
    if (nested_specializations()) return 4;
    Outside<int>::Inner outside;
    if (outside.read()!=8 || Outside<long>::read()!=8) return 3;
    Derived<DependentBase<int> > value;
    Derived<DependentBase<int> >::Nested fixed;
    CompleteContext<int>::Nested nested;
    CompleteContext<int> bits; bits.bits=6;
    if (value.call()!=0 || fixed.call()!=5 || local_base<DependentBase<int> >()!=0) return 1;
    if (nested.read()!=7 || bits.read()!=6) return 2;
    return conditions(4)!=4 || conditions(0)!=1 || jump<int>()!=0;
}
// Nested specialization keeps its enclosing argument environment while using
// the same parsed region for the inner function-template body.
template<class Tag> struct Receiver {
    template<class V> long cast(V value) { return Tag(value); }
    template<class V> Receiver& operator>>(V& value) { value=sizeof(Tag); return *this; }
};
int nested_specializations() {
    Receiver<char> small; Receiver<long> large;
    int a=0,b=0; small>>a; large>>b;
    return a!=1 || b!=8 || small.cast(258)!=2 || large.cast(258)!=258;
}
