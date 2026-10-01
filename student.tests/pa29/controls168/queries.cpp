struct S { int a; int b; };
template<class T> using Value=decltype((T){.b=3});
static_assert(__is_same(Value<S>,S),"dependent compound literal type");
static_assert(__is_same(decltype((S){.b=2}),S),"prvalue");
template<class T> auto viable(int)->decltype((T){.b=2},int()){return 1;}
template<class> int viable(...){return 2;}
struct Other {int x;};
int main(){return viable<S>(0)!=1 || viable<Other>(0)!=2;}
