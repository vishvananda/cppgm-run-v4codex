template<class T> struct box {};
box(int x,int y=x)->box<int>;
