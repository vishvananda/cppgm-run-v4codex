struct A{int a;};struct B{int b;};struct D:A,B{};
constexpr const D *d=nullptr;
constexpr const B *b=nullptr;
static_assert((B*)d==nullptr && (D*)b==nullptr,"null pointers stay null");
constexpr const int B::*bm=nullptr;
constexpr const int D::*dm=nullptr;
static_assert((int D::*)bm==nullptr && (int B::*)dm==nullptr,"null members stay null");
int main(){const D *p=nullptr; const B *q=nullptr;
 const int B::*r=nullptr;const int D::*s=nullptr;
 return (B*)p==nullptr && (D*)q==nullptr && (int D::*)r==nullptr && (int B::*)s==nullptr?0:1;}
