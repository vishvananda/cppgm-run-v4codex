template<class T> struct box {};
box(int = "bad")->box<int>;
