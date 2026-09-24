struct B { int n; int f(){return n;} };
struct M : B {};
template<int>struct D : M {int g(){return M::f();}};
int main(){D<0>x;x.n=7;return x.g()!=7;}
