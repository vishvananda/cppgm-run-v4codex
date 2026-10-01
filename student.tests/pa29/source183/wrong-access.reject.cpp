struct outer {template<class T> struct box {}; private: box(int)->box<int>;};
