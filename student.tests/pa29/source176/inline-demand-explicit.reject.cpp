template<class T> struct Lazy { inline static int invalid = T::missing; };
template struct Lazy<int>;
