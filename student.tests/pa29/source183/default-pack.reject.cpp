template<class T> struct box {};
template<class... T> box(T... x = 0)->box<int>;
