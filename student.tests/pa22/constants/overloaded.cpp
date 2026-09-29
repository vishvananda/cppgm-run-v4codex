struct A{int a;constexpr A():a(0){}};struct B{int b;constexpr B(int n):b(n){} constexpr int f()const{return b;} int f(int)const{return 0;}};
struct D:A,B{constexpr D():A(),B(7){}};
constexpr int(D::*p)()const=&B::f;
constexpr D d;
static_assert((d.*p)()==7,"selected overload has converted owner");
template<class T,int(T::*P)()const>int call(T const& t){return (t.*P)();}
int main(){D x;return (x.*p)()!=7 || call<B,&B::f>(x)!=7;}
