template<class T> struct box {};
template<> box(int)->box<int>;
