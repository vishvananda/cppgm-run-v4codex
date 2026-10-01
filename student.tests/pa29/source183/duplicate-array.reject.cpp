template<class T> struct box {};
box(int[])->box<int>;box(int*)->box<int>;
