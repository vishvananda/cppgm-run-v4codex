// C++11 permits elision of the two source prvalues ([class.copy]/31).
// The lvalue array decomposition still copies each element once.
struct Item{int v;constexpr Item(int x):v(x){}constexpr Item(const Item& x):v(x.v+1){}};
constexpr int f(){Item a[2]={Item(3),Item(4)};auto [x,y]=a;return x.v+y.v;}
static_assert((f()==9 || f()==11),"constexpr element constructors");int main(){return f()!=9;}
