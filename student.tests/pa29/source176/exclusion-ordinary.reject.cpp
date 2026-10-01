template<class T> struct Ordinary {
  __attribute__((exclude_from_explicit_instantiation)) int absent() { return T::missing; }
  int ordinary() { return T::missing; }
};
template struct Ordinary<int>;
