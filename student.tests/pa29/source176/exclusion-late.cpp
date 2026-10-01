template<class T> struct Late { int used(); int dormant(); };
template<class T> __attribute__((exclude_from_explicit_instantiation)) int Late<T>::used() { return 13; }
template<class T> __attribute__((exclude_from_explicit_instantiation)) int Late<T>::dormant() { return T::missing; }
extern template struct Late<int>;
template struct Late<long>;
int main() { Late<int> a; return a.used()-13; }
