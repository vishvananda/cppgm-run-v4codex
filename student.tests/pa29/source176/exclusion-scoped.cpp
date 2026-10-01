#if !__has_attribute(exclude_from_explicit_instantiation)
#error missing attribute
#endif
template<class T> struct Scoped {
  [[clang::exclude_from_explicit_instantiation]] int used() { return 7; }
  [[clang::exclude_from_explicit_instantiation]] int unused() { return T::missing; }
};
extern template struct Scoped<int>;
template struct Scoped<long>;
int main() { Scoped<int> a; return a.used()-7; }
