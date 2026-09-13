// N3485 [dcl.fct]/5, [dcl.type.simple], [basic.scope.proto], [temp.inst]/1.
// Callable signatures and raw parameter/body facts have separate owners.
int cv(int*) { return 100; }
int cv(const int*) { return 0; }
int cv(long*) { return 100; }
int cv(const long*) { return 0; }
int twist(int n) { return n+2; }
long twist(long n) { return n+3; }
int destroyed;
int initialized;

template<class T> struct Signature {
    T stored;
    T apply(T values[3], const T offset, T fn(T)) {
        if (sizeof(values)!=sizeof(T*) || sizeof(fn)!=sizeof(T(*)(T))) return 100;
        return fn(values[0])+offset+cv(&offset);
    }
    auto sum(T first, decltype(first) second) -> decltype(first+second) {
        return first+second;
    }
    auto identity(T& value) -> decltype((value)) { return value; }
    T (*get(T value))(T) { stored=value; return twist; }
    T quiet(T value) noexcept(sizeof(T)>0) { return value+1; }
    T fallback(T value=make()) { return value; }
    T unused(T value) { return value.no_such_member; }
private:
    static T make() { return T(7); }
};

template<class T> struct Mixed {
    int stored;
    static int pair(int first, decltype(first) second) { return first+second; }
    int pair(long,long);
};
template<class T> int Mixed<T>::pair(long first,long second) { return stored+first+second; }

template<class T> struct Defaults {
    struct Inner { static T get(T value=make()) { return value; } };
    T stored;
    Defaults(T value=make()) : stored(value) {}
private:
    static T make() { return T(11); }
};

template<class T> struct UnusedDefault {
    T get(T value=make()) { return value; }
    static T make() { return T::missing; }
};

template<class T> struct Initializers {
    int first=++initialized;
    int value=first+make();
    struct Inner { int value=make(); };
private:
    static int make() { return ++initialized; }
};
template<class T> struct OverriddenInitializer {
    int value=T::missing;
    OverriddenInitializer(int supplied) : value(supplied) {}
};

template<class T> int local_signature(T value) {
    struct Local {
        T stored;
        Local(T value) : stored(value) {}
        Local(const Local& other) : stored(other.stored+1) {}
        ~Local() { ++destroyed; }
        auto sum(T first, decltype(first) second) const -> decltype(first+second) {
            return stored+first+second;
        }
        T copy(const T value) { return value+cv(&value); }
    };
    Local first(value); Local second(first);
    return second.sum(2,3)!=value+6 || first.copy(4)!=4;
}

int main() {
    Signature<int> a; Signature<long> b;
    int values[3]={4,5,6}; long other[3]={7,8,9};
    if (a.apply(values,3,twist)!=9 || b.apply(other,5,twist)!=15) return 1;
    if (a.sum(2,3)!=5 || b.sum(3,4)!=7) return 2;
    if (&a.identity(values[0])!=values || &b.identity(other[0])!=other) return 3;
    if (a.get(4)(3)!=5 || a.stored!=4 || b.get(6)(4)!=7 || b.stored!=6) return 4;
    if (a.quiet(5)!=6 || b.quiet(7)!=8 || !noexcept(a.quiet(0))) return 5;
    if (a.fallback()!=7 || b.fallback(9)!=9) return 6;
    if (local_signature<int>(3) || destroyed!=2) return 7;
    if (local_signature<long>(4) || destroyed!=4) return 8;
    Mixed<int> mixed; mixed.stored=4;
    if (Mixed<int>::pair(2,3)!=5 || mixed.pair(2L,3L)!=9) return 9;
    Defaults<int> defaults;
    if (defaults.stored!=11 || Defaults<long>::Inner::get()!=11) return 10;
    UnusedDefault<int> unused;
    if (unused.get(13)!=13) return 11;
    Initializers<int> fields;
    if (fields.first!=1 || fields.value!=3 || initialized!=2) return 12;
    Initializers<long>::Inner nested;
    if (nested.value!=3 || initialized!=3) return 13;
    OverriddenInitializer<int> supplied(17);
    if (supplied.value!=17) return 14;
    return 0;
}
