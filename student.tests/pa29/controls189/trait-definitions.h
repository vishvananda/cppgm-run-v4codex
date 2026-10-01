// Real definitions make the original forward declarations sufficient.
namespace std {
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
