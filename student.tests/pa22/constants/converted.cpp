struct A { int a; constexpr A(int n):a(n){} constexpr int f()const{return a;} };
struct B { int b; constexpr B(int n):b(n){} };
struct D:A,B { constexpr D(int a,int b):A(a),B(b){} };
constexpr D object(7,3);
constexpr int D::*data=&A::a;
constexpr int B::*inverse=static_cast<int B::*>(data);
constexpr int(D::*function)()const=&A::f;
constexpr int(B::*inverse_function)()const=static_cast<int(B::*)()const>(function);
static_assert(object.*data==7,"converted data owner");
static_assert(object.*inverse==7,"inverse data owner");
static_assert((object.*function)()==7,"converted function owner");
static_assert((object.*inverse_function)()==7,"inverse function owner");
int main(){D x(9,4);return x.*inverse!=9 || (x.*inverse_function)()!=9;}
