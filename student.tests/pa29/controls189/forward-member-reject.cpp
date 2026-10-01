// Reduced nothrow-shorthand case: no class definition supplies value.
template<class T> struct property;
static_assert(property<int>::value, "a declaration is not a definition");
