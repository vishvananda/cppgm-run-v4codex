struct A { int x; };
template<int A::*P> struct X{};
X<0> invalid;
int main(){return 0;}
