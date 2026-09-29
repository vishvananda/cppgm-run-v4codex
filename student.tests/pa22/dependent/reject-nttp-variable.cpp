struct A { int x; };
constexpr int A::*p = &A::x;
template<int A::*P> struct X{};
X<p> invalid;
int main(){return 0;}
