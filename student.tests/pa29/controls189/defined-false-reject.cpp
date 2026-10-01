// Reduced invocable-cache case. The supplied primary defines false for all T.
template<bool B> struct flag { static const bool value = B; };
template<class F, class A> struct property : flag<false> {};
template<class T> struct invert : flag<!T::value> {};
struct callable { int operator()(int) const noexcept { return 0; } };
static_assert(!invert<property<const callable&, const int&>>::value, "false");
