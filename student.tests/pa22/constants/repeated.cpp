struct A {int a;constexpr A(int n):a(n){} constexpr int f()const{return a;}};
struct L:A {constexpr L():A(3){}};
struct R:A {constexpr R():A(7){}};
struct D:L,R {constexpr D():L(),R(){}};
constexpr int(L::*lf)()const=&A::f;
constexpr int(R::*rf)()const=&A::f;
constexpr int(D::*left)()const=lf;
constexpr int(D::*right)()const=rf;
constexpr int L::*ld=&A::a;
constexpr int R::*rd=&A::a;
constexpr int D::*data_left=ld;
constexpr int D::*data_right=rd;
static_assert(left!=right,"[expr.eq]/2 distinct subobjects");
static_assert(data_left!=data_right,"distinct data subobjects");
constexpr D d;
static_assert((d.*left)()==3 && (d.*right)()==7,"function receiver paths");
static_assert(d.*data_left==3 && d.*data_right==7,"data receiver paths");
int main(){D x;return (x.*left)()!=3 || (x.*right)()!=7 || x.*data_left!=3 || x.*data_right!=7;}
