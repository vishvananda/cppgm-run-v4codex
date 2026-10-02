struct Base { protected: static int value() { return 29; } };
struct Other {};
template<class T> struct Child : T { int f() { return Base::value(); } };
int main() { Child<Other> c; return c.f(); }
