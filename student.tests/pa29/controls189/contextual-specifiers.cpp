struct flag {
  bool b;
  constexpr flag(bool v) : b(v) {}
  constexpr explicit operator bool() const { return b; }
};
void quiet() noexcept(flag(true)) {}
void noisy() noexcept(flag(false)) {}
static_assert(noexcept(quiet()), "true noexcept");
static_assert(!noexcept(noisy()), "false noexcept");
struct implicit { explicit(flag(false)) implicit(int) {} };
struct direct { explicit(flag(true)) direct(int) {} };
template<class T> struct friend_value;
class private_condition {
  constexpr explicit operator bool() const { return true; }
  friend struct allowed;
  template<class T> friend struct friend_value;
};
struct allowed {
  explicit(private_condition{}) allowed() {}
  static void f() noexcept(private_condition{}) {}
  static_assert(private_condition{}, "friend assertion");
};
template<class T> struct friend_value {
  explicit(T{}) friend_value() {}
  template<class U> explicit(T{}) friend_value(U) {}
  static void f() noexcept(T{}) {}
  static_assert(T{}, "template friend assertion");
};
int main() {
  implicit i = 1; direct d(1); allowed a; a.f();
  friend_value<private_condition> v, w(3); v.f();
  quiet(); noisy(); return 0;
}
