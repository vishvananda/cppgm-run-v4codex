namespace ordinary { template<class T> struct is_nothrow_default_constructible; }
static_assert(ordinary::is_nothrow_default_constructible<int>::value,"undefined class");
