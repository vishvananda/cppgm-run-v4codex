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
int main() {
    if (sizeof(accept(Unevaluated<int>::value))!=sizeof(int) || conversions!=0) return 1;
    if (accept(7L)!=1) return 2;
    example::Explicit<int> a;
    return a.value()!=3 || example::Explicit<long>::twice(4)!=8;
}
