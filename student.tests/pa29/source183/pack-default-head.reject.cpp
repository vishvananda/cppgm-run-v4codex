template<class T> struct box {};
template<class... T=int> box(T...)->box<int>;
