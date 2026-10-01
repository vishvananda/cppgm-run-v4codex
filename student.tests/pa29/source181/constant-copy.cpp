struct S{int x;int a[0];};constexpr S x{7,{}};constexpr S y=x;
static_assert(y.x==7,"constant copy");
template<class T> constexpr unsigned long size(){return sizeof(T);}
static_assert(size<int[0]>()==0,"constant query");
int main(){return y.x!=7;}
