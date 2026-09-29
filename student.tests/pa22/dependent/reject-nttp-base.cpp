struct A { int x; }; struct D:A{};
template<int D::*P> struct X{};
X<&A::x> invalid;
int main(){return 0;}
