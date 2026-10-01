template<class T> struct box {};
template<class... T> box(T)->box<int>;
