template<class T> struct condition {
  constexpr explicit operator bool() const { return T::missing; }
};
struct value {
  template<class T> explicit(T{}) value(T) {}
  value(...) {}
};
int main() { value v(condition<int>{}); }
