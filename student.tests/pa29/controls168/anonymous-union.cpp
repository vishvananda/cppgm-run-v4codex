struct S { int tag; union { int a; long b; }; int tail; };
constexpr S s={.b=9};
static_assert(s.tag==0 && s.b==9 && s.tail==0,"anonymous union designation");
template<class T> T make(long x){return (T){.b=x};}
template<class T> using Init=decltype((T){.b=4});
static_assert(__is_same(Init<S>,S),"projected designator type query");
int main(){S t={.b=17};return t.tag!=0 || t.b!=17 || t.tail!=0 || ((S){.b=23}).b!=23 || make<S>(29).b!=29;}
