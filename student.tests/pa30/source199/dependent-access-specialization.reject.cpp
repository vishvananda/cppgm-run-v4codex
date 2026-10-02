struct Base { protected: static int value() { return 29; } };
template<class T> struct Middle : Base {};
template<> struct Middle<int> {};
template<class T> struct Child : Middle<T> { int f() { return Base::value(); } };
int main() { Child<int> c; return c.f(); }
