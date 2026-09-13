// N3485 [temp.inst]: class specialization instantiates member declarations,
// but not member definitions or default arguments. [dcl.fct.default] supplies
// a default only when the corresponding argument is omitted.
int evaluations=0;
int next() { return ++evaluations; }
template<class T> struct Choice {
    T value;
    Choice(int n=T::missing):value(n) {}
    int add(int n=T::missing) const { return value+n; }
    int count(int n=next()) const { return n; }
    int unused() { return T::absent; }
    struct Nested {
        int run(int n=T::missing) { return n+sizeof(T); }
        int unused() { return T::absent; }
    };
};
template<class T> int local(T value) {
    struct Local {
        int run(int n=T::missing) { return n+sizeof(T); }
        int unused() { return T::absent; }
    };
    Local object;
    return object.run(value);
}
template<class T> struct Defined {
    T value;
    Defined(int n):value(n) {}
    int run(int n=sizeof(T));
};
template<class U> int Defined<U>::run(int n) {
    using Value=U;
    Value result=value;
    return result+n;
}
template<class T> struct Convert {
    int value;
    Convert(int n, int extra=sizeof(T)):value(n+extra) {}
};
int main() {
    Choice<int> a(4); Choice<long> b(9);
    if (a.add(3)!=7 || b.add(5)!=14) return 1;
    if (a.count()!=1 || a.count()!=2 || b.count()!=3 || evaluations!=3) return 2;
    Choice<int>::Nested x; Choice<long>::Nested y;
    if (x.run(2)!=2+sizeof(int) || y.run(4)!=4+sizeof(long)) return 3;
    if (local(5)!=5+sizeof(int) || local(7L)!=7+sizeof(long)) return 4;
    Defined<int> d(8); Defined<long> e(11);
    if (d.run()!=8+sizeof(int) || e.run()!=11+sizeof(long)) return 5;
    Convert<int> ci=2; Convert<long> cl=3;
    return ci.value!=2+sizeof(int) || cl.value!=3+sizeof(long);
}
