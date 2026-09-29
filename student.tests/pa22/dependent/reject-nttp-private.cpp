class A { int x; };
template<int A::*P> struct X{};
X<&A::x> invalid;
int main(){return 0;}
