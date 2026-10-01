template<class T> struct Trait {static const bool value=false;};
template<class T> struct Negate {static const bool value=!T::value;};
static_assert(!Negate<Trait<int>>::value,"explicit false remains false");
