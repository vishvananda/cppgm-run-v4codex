template<class T> struct box {};
template<class U> box(int)->box<U>;
