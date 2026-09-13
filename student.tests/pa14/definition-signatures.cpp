// Renamed heads, adjusted parameters and current-instantiation aliases retain
// the same member declaration identity in out-of-class definitions.
template<class T> struct Shape {
    typedef T value_type;
    typedef T (*function)(T);
    struct Nested { typedef T type; type read(type) const; };
    value_type read(const value_type&) const;
    value_type first(value_type values[3]);
    value_type apply(function, value_type);
    int ref(int) &;
    int ref(int) &&;
    auto trailing(value_type x) -> decltype(x);
};
template<class U> typename Shape<U>::value_type Shape<U>::read(const U& x) const { return x+1; }
template<class U> U Shape<U>::first(U* values) { return values[0]; }
template<class U> U Shape<U>::apply(U(*fn)(U),U x) { return fn(x); }
template<class U> int Shape<U>::ref(int x) & { return x+2; }
template<class U> int Shape<U>::ref(int x) && { return x+3; }
template<class U> auto Shape<U>::trailing(U y) -> decltype(y) { return y+4; }
template<class U> typename Shape<U>::Nested::type Shape<U>::Nested::read(typename Shape<U>::Nested::type x) const { return x+5; }
template<class T> struct Outside { struct Nested; };
template<class U> struct Outside<U>::Nested { typedef U type; type read(type) const; };
template<class V> typename Outside<V>::Nested::type Outside<V>::Nested::read(typename Outside<V>::Nested::type x) const { return x+6; }
int twice(int x) { return 2*x; }
int main() {
    Shape<int> a; Shape<long> b; Shape<int>::Nested n; Shape<long>::Nested m;
    int values[3]={7,8,9}; long other[3]={11,12,13};
    if (a.read(2)!=3 || b.read(4)!=5 || a.first(values)!=7 || b.first(other)!=11) return 1;
    if (a.apply(twice,6)!=12 || a.ref(1)!=3 || static_cast<Shape<int>&&>(a).ref(1)!=4) return 2;
    Outside<int>::Nested outside;
    if (outside.read(3)!=9) return 3;
    return a.trailing(1)!=5 || b.trailing(2)!=6 || n.read(3)!=8 || m.read(4)!=9;
}
