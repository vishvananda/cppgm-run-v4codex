template<class T> struct box {};
extern int state; explicit(state) box(int)->box<int>;
