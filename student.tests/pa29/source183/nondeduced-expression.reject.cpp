template<class T> struct box {};
template<int N> box(int(&)[N+1])->box<int>;
