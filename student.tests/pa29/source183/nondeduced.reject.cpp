template<class T> struct box {};
template<class U> box(typename U::type)->box<U>;
