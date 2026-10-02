struct Base { protected: static int value() { return 29; } };
template<class T> struct Middle : Base {};
template<class T> struct Child : Middle<T> { int f(); };
template<class T> int Child<T>::f() { return Base::value(); }
int main() { Child<int> c; return c.f()-29; }
