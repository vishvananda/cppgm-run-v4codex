struct A {int a;int f()const{return a;}};struct B {int b;};struct D:A,B{};
constexpr int(D::*d)()const=&A::f;
constexpr int(B::*b)()const=static_cast<int(B::*)()const>(d);
int main(){D x;x.a=7;x.b=3;return (x.*b)()!=7;}
