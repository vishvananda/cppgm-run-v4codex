namespace ordinary {
template<class T> struct is_nothrow_default_constructible {
  static constexpr bool value = __is_nothrow_constructible(T);
};
template<class T> struct is_nothrow_copy_constructible {
  static constexpr bool value = __is_nothrow_constructible(T, const T&);
};
template<class T> struct is_nothrow_move_constructible {
  static constexpr bool value = __is_nothrow_constructible(T, T&&);
};
}
struct trivial {};
struct throwing_default { throwing_default() noexcept(false) {} };
struct throwing_copy {
  throwing_copy() noexcept {}
  throwing_copy(const throwing_copy&) noexcept(false) {}
};
struct moving {
  moving() noexcept {}
  moving(const moving&) noexcept(false) {}
  moving(moving&&) noexcept {}
};
struct deleted_default { deleted_default() = delete; };
static_assert(ordinary::is_nothrow_default_constructible<trivial>::value, "default");
static_assert(!ordinary::is_nothrow_default_constructible<throwing_default>::value, "throw default");
static_assert(ordinary::is_nothrow_copy_constructible<trivial>::value, "copy");
static_assert(ordinary::is_nothrow_move_constructible<trivial>::value, "move");
static_assert(!ordinary::is_nothrow_copy_constructible<throwing_copy>::value, "throw copy");
static_assert(!ordinary::is_nothrow_copy_constructible<moving>::value, "copy choice");
static_assert(ordinary::is_nothrow_move_constructible<moving>::value, "move choice");
static_assert(!ordinary::is_nothrow_default_constructible<deleted_default>::value, "deleted");
int main() { return 0; }
