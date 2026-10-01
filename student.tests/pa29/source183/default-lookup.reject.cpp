template<class T> struct box {};
box(int = missing)->box<int>;
