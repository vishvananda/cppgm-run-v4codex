template<class T> struct box {};
box(const int)->box<int>;box(int)->box<int>;
