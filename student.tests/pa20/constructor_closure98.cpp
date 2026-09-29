// Reduced deduction proof: an implicit lambda conversion cannot change F.
template<class T> struct is_pointer { static const bool value = false; };
template<class T> struct is_pointer<T*> { static const bool value = true; };
struct Sink {
  int value;
  template<class F> Sink(F f) : value(f(6)) {
    static_assert(!is_pointer<F>::value, "deduction preserves the closure class");
  }
};
int main() { Sink s = [](int n) { return n + 1; }; return s.value != 7; }
