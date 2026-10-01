template<class T> struct demanded {
  static_assert(sizeof(T) == 0, "demanded definition is ill-formed");
};
template<class T> struct condition {
  constexpr explicit operator bool() const { return sizeof(demanded<T>) != 0; }
};
struct value {
  template<class T> explicit(T{}) value(T) {}
  value(...) {}
};
int main() { value v(condition<int>{}); }
