int conversions=0;
struct Any {
    Any(...) { ++conversions; }
};
int accept(Any) { return conversions; }
namespace example {
template<class T> struct Explicit {
    T value() { return T(3); }
    static T twice(T);
};
template<class T> T Explicit<T>::twice(T n) { return n+n; }
}
template class example::Explicit<int>;
template class ::example::Explicit<long>;
template<class T> struct Unevaluated { static T value; };
template<class T> T Unevaluated<T>::value=T::invalid;
template<class X, class Y> struct Later { typedef Y type; X value; };
template<class N> struct Fallback { N value; };
template<class Renamed, class Other=Fallback<Renamed> > struct Later;
template<class Old, class Default=Old> struct Forward;
template<class New, class Result> struct Forward { typedef Result type; };
template<class V> struct alignas(__alignof(V)) Aligned { char c; };
struct Fixed { static const int value=17; };
template<class Base> struct Derived: Base {};
template<class Base> int constant() { return Derived<Base>::value; }
template<class V> struct Storage { static V value; };
template<class V> V Storage<V>::value;
struct Stored { int n; };
namespace detail { template<class V> struct Set { V value; }; }
template<class V> int elaborated() { typedef struct detail::Set<V> Local; Local x={13}; return x.value; }
int choose(Fallback<long>*) { return 0; }
int choose(Fallback<int>*) { return 1; }
int main() {
    if (sizeof(accept(Unevaluated<int>::value))!=sizeof(int) || conversions!=0) return 1;
    if (accept(7L)!=1) return 2;
    Later<long>::type* selected=0;
    if (choose(selected) || sizeof(Forward<short>::type)!=sizeof(short)) return 3;
    if (alignof(Aligned<double>)!=alignof(double) || constant<Fixed>()!=17) return 4;
    Stored* storage=&Storage<Stored>::value;
    if (storage->n!=0 || elaborated<int>()!=13) return 5;
    example::Explicit<int> a;
    return a.value()!=3 || example::Explicit<long>::twice(4)!=8;
}
