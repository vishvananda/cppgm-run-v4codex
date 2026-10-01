struct Item{int v;constexpr Item(int x):v(x){}constexpr Item(const Item& x):v(x.v+1){}};
constexpr int f(){Item a[2]={3,4};auto [x,y]=a;return x.v+y.v;}
static_assert(f()==9,"constexpr element constructors");int main(){return f()!=9;}
