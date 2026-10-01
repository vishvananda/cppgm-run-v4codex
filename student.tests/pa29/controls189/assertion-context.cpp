struct choice {
  bool value;
  constexpr choice(bool b) : value(b) {}
  constexpr explicit operator bool() const { return value; }
};
struct integral_choice { constexpr operator int() const { return 4; } };
struct contextual_choice {
  constexpr explicit operator bool() const { return true; }
  constexpr operator int() const { return 0; }
};
struct member { int value; };
int object;
static_assert(choice(true), "explicit bool");
static_assert(integral_choice{}, "implicit int then bool");
static_assert(contextual_choice{}, "explicit bool preferred");
static_assert(1.5, "floating contextual bool");
static_assert(&object, "object pointer");
static_assert(&member::value, "member pointer");
static_assert("text", "string pointer");
static_assert(__builtin_is_constant_evaluated(), "manifest assertion evaluation");
constexpr choice constant(true);
static_assert(constant, "constexpr lvalue receiver");
template<bool B> struct check {
  static_assert(choice(B), "dependent conversion");
  static_assert(choice(true), "fixed conversion");
};
check<true> instantiated;
template<bool B> int function() { static_assert(choice(B), "body conversion"); return 3; }
int main() { return function<true>() == 3 ? 0 : 1; }
