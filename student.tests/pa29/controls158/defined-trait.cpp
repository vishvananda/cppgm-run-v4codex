namespace ordinary {
 template<class T> struct is_nothrow_default_constructible { static constexpr bool value=__is_nothrow_constructible(T); };
 template<class T> struct is_nothrow_copy_constructible { static constexpr bool value=__is_nothrow_constructible(T,const T&); };
 template<class T> struct is_nothrow_move_constructible { static constexpr bool value=__is_nothrow_constructible(T,T&&); };
}
struct selector{};
struct throwing_default {throwing_default() noexcept(false) {}};
struct throwing_copy {throwing_copy(){} throwing_copy(const throwing_copy&) noexcept(false){}};
static_assert(ordinary::is_nothrow_default_constructible<selector>::value,"");
static_assert(!ordinary::is_nothrow_default_constructible<throwing_default>::value,"");
static_assert(ordinary::is_nothrow_copy_constructible<selector>::value,"");
static_assert(ordinary::is_nothrow_move_constructible<selector>::value,"");
static_assert(!ordinary::is_nothrow_copy_constructible<throwing_copy>::value,"");
int main(){return 0;}
